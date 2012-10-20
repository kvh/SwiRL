
#include <iomanip>
#include <math.h>

#include "Tree.h"
#include "Assert.h"
#include "CharUtils.h"
#include "Head.h"
#include "Lexicon.h"
#include "Logger.h"
#include "AdaBoostClassifier.h"
//#include "SVMClassifier.h"
#include "UnitCandidate.h"
#include "Score.h"

using namespace std;
using namespace srl;

#define DO_CLASSIFICATION_WITH_APPROXIMATE_INFERENCE true

StringMap<Classifier *> Tree::_classifiers;

///////////////////////////////////////////////////////////////////////
//
// These statistics computed when running in ORACLE mode
//
///////////////////////////////////////////////////////////////////////

//
// keeps track of the position of the best candidate in the candidate set
// the candidate is the individual argument (when running greedyClassification)
//   or the full frame (when running rerankingClassification)
//
static vector<int> bestPositions;

//
// keeps track of the oracle scores when using the top N candidates
// the candidate is the individual argument (when running greedyClassification)
//   or the full frame (when running rerankingClassification)
//
static vector<Score> oracleScores;

void Tree::resetOracleStats()
{
  int max = LOCAL_COUNT_BEAM;
  if(DO_CLASSIFICATION_WITH_APPROXIMATE_INFERENCE) max = GLOBAL_BEAM;

  bestPositions.resize(max, 0);
  oracleScores.resize(max);
}

void Tree::printOracleStats(ostream & is)
{
  is << "Positions of the best candidate:\n";
  for(size_t i = 0; i < bestPositions.size(); i ++)
    is << i << ": " << bestPositions[i] << endl;

  is << "Oracle scores:\n";
  for(size_t i = 1; i < oracleScores.size(); i ++){
    is << i << ": P " << oracleScores[i].P() 
       << " R " << oracleScores[i].R() 
       << " F1 " << oracleScores[i].F1()
       << "\t\t(correct " << oracleScores[i].mCorrect
       << " predicted " << oracleScores[i].mPredicted
       << " total " << oracleScores[i].mTotal << ")"
       << endl;
  }
}

///////////////////////////////////////////////////////////////////////
//
// End ORACLE stats
//
///////////////////////////////////////////////////////////////////////

static String merge(const String & s1,
		    const String & s2)
 {
   ostringstream os;
   os << s1 << s2;
   return os.str();
 }

 static String merge(const String & s1,
		     const String & s2,
		     const String & s3)
 {
   ostringstream os;
   os << s1 << s2 << s3;
   return os.str();
 }

 class DescByConf {
 public:
   bool operator()(const ClassifiedArg & d1,
		   const ClassifiedArg & d2) {
       if(d1.conf >= d2.conf){
	 return true;
       }
       return false;
   };
 };

 /**
  * Computes the softmax probability for a classifier confidence value
  */
 static double softmax(double label,
		       const vector<double> & labels)
 {
   double nom = exp(label * SOFTMAX_GAMMA);
   double den = 0.0;
   for(size_t i = 0; i < labels.size(); i ++){
     den += exp(labels[i] * SOFTMAX_GAMMA);
   }
   double prob = nom/den;

   //cerr << "softmax: " << label << " /";
   //for(size_t i = 0; i < labels.size(); i ++) cerr << " " << labels[i];
   //cerr << " = " << prob << "\n";

   return prob;
 }

 bool Tree::
 loadClassifierModels(const char * path)
 {
   static char * labels [] = { 
     // common labels
     "B-A0", "B-A1", "B-A2", "B-A3", "B-A4", "B-A5", 
     "B-AM-ADV", "B-AM-CAU", "B-AM-DIR", "B-AM-DIS", "B-AM-EXT",
     "B-AM-LOC", "B-AM-MNR", "B-AM-MOD", "B-AM-NEG", "B-AM-PNC",
     "B-AM-TMP", "B-C-A0", "B-C-A1", "B-R-A0", "B-R-A1", "B-R-A2",
     "B-R-AM-LOC", "B-R-AM-MNR", "B-R-AM-TMP", 

     // not so common labels
     "B-AA", 
     "B-AM-PRD", "B-AM-REC",
     "B-C-A2", "B-C-A3", "B-C-AM-MNR", 
     "B-R-A3", "B-R-AM-CAU", "B-R-AM-PNC", 

     // the NIL class
     "B-O",

     // "B-X", 
     NULL
   };

   int successCount = 0;
   vector<string> failedClasses;
   for(int i = 0; labels[i] != NULL; i ++){
     Classifier * c = new AdaBoostClassifier(labels[i]);

     ostringstream os;
     os << path << "/" << labels[i] << ".model.ab";

     if(c->initialize(os.str().c_str())){
       //cerr << labels[i] << " ";
       successCount ++;
     } else {
       failedClasses.push_back(labels[i]);
     }

     _classifiers.set(labels[i], c);
   }
   //cerr << endl;

   cerr << "Sucessfully loaded " << successCount << " models.\n";
   if(failedClasses.size() > 0){
     cerr << "WARNING: Failed to load " << failedClasses.size() << " models:";
     for(size_t i = 0; i < failedClasses.size(); i ++)
       cerr << " " << failedClasses[i];
     cerr << "\n";
   }

   RVASSERT(successCount > 0, "No classifiers found. Aborting...");
   return true;
 }

 Classifier * Tree::getClassifier(const String & label)
 {
   Classifier * c = NULL;
   if(_classifiers.get(label.c_str(), c) == true &&
      c->isInitialized()) return c;
   return NULL;
 }

void Tree::fetchGoldArgsForPredicate(const Tree * predicate,
				     std::vector<ClassifiedArg> & goldArgs)
{
  for(list<Argument>::const_iterator it = getGoldArguments().begin();
      it != getGoldArguments().end(); it ++){
    const Argument & g = * it;
    if(g.getVerbPosition() == predicate->getRightPosition() &&
       g.getName() != "V"){ // skip the V args, not relevant
      goldArgs.push_back(ClassifiedArg(this, g.getName(), 1.0));
    }
  }

  for(list<TreePtr>::iterator it = _children.begin();
      it != _children.end(); it ++){
    (* it)->fetchGoldArgsForPredicate(predicate, goldArgs);
  }
}

 void Tree::
 classify(const std::vector< RCIPtr<srl::Tree> > & sentence,
	  int sentenceIndex,
	  bool caseSensitive)
 {
   LOGH << "Started SRL classification for sentence:\n";
   LOGH << * this << "\n\n";

   // detect all predicates in this phrase
   vector<int> predPositions;
   detectPredicates(predPositions);
   LOGH << "Found " << predPositions.size() << " predicates.\n";

   //
   // classify args for each predicate
   //
   for(int predicateIndex = 0; 
       predicateIndex < (int) predPositions.size(); 
       predicateIndex ++){

     int position = predPositions[predicateIndex];
     RVASSERT(position >= 0 && position < (int) sentence.size(),
	     "Invalid predicate position " << position);
     //cerr << "Classifying sentence " << sentenceIndex << " with predicate " << predicateIndex << endl;

     // this is the current predicate
     Tree * predicate = sentence[position].operator->();
     RVASSERT(predicate != NULL && predicate->isPredicate(),
	      "Chosen phrase is not a predicate!");
     LOGH << "Inspecting predicate: " << * predicate;

     // the first S* parent
     const Tree * predParent = predicate->findParentForLluisHeuristic();
     RVASSERT(predParent != NULL, "Found NULL parent for Lluis heuristic!");

     // which argument types are possible for this predicate?
     list<String> possibleArgLabels;
     loadAcceptableArgumentLabels(predicate->getLemma(), possibleArgLabels);

     LOGH << "Arg list is:";
     for(list<String>::const_iterator it = possibleArgLabels.begin();
	 it != possibleArgLabels.end(); it ++)
       LOGH << " " << * it;
     LOGH << endl;

     //
     // this is the full set of candidates for this predicate
     // required by both greedyClassification and rerankingClassification
     //
     vector<Tree *> candidates;
     findCandidatesForPredicate(predicate, predParent, candidates);

     //
     // this is the set of arg labels for all candidates
     // required by both greedyClassification and rerankingClassification
     // candidatesArgs is sorted (both rows and columns) in 
     //   descending order of the classifier confidences
     //
     vector< vector<ClassifiedArg> *> candidatesArgs;
     generateArgsForCandidates(sentence, predicate, 
			       possibleArgLabels, candidates, 
			       caseSensitive, 
			       LOCAL_COUNT_BEAM, LOCAL_CONF_BEAM,
			       candidatesArgs);

     // 
     // if we're running in oracle mode fetch the list of GOLD args
     //
     vector<ClassifiedArg> goldArgs;
     if(getOracleMode()) fetchGoldArgsForPredicate(predicate, goldArgs);

     // classification using global search within many frame candidates
     if(DO_CLASSIFICATION_WITH_APPROXIMATE_INFERENCE){
       rerankingClassification(predicate, candidatesArgs, 
			       GLOBAL_BEAM, goldArgs);
     } 

     // use the greedy strategy for classification
     else {
       greedyClassification(predicate, candidatesArgs, goldArgs);
     }

     // cleanup
     for(size_t i = 0; i < candidatesArgs.size(); i ++)
       delete candidatesArgs[i];

     //
     // expand terminal Bs
     // XXX: you can disable expansion heuristics here!
     //
     expandArguments(sentence, predicate, sentenceIndex, predicateIndex);

   } // finished processing all predicates

   // report classifications
   LOGH << "Tree " << sentenceIndex << " after classification:" 
	<< endl << * this;
 }

 static bool satisfiesDomainConstraints(const vector<ClassifiedArg> & frame,
					const ClassifiedArg & newArg)
 {
   // O is always fine
   if(newArg.label == "O") return true;

   //
   // two arguments in the same frame cannot include each other
   //
   for(size_t i = 0; i < frame.size(); i ++){
     if(frame[i].label == "O") continue;
     Tree * other = frame[i].phrase;
     if(newArg.phrase->includes(other) || other->includes(newArg.phrase)){
       return false;
     }
   }

   //
   // numeric arguments cannot repeat
   //
   if(! startsWith(newArg.label, "AM")){
     for(size_t i = 0; i < frame.size(); i ++){
       const String & other = frame[i].label;
       if(newArg.label == other){
	 return false;
       }
     }
   }

   return true;
 }

class DescendingByProb
{
public:
  bool operator()(const vector<ClassifiedArg> * c1,
		  const vector<ClassifiedArg> * c2) {
    if((* c1)[0].conf > (* c2)[0].conf){
      return true;
    }
    return false;
  };
};

 void Tree::
 generateArgsForCandidates(const std::vector< RCIPtr<srl::Tree> > & sentence,
			   const Tree * predicate,
			   const list<String> & possibleArgLabels, 
			   const std::vector<Tree *> & candidates,
			   bool caseSensitive,
			   int countBeam,
			   double confBeam,
			   vector<vector<ClassifiedArg> *> & allArgs)
 {
   //
   // allArgs stores all possible label assignments for each candidate
   // the labels for each candidate are sorted in descending order of probs
   // allArgs[i] stores the labels for candidates[i]
   //

   for(size_t i = 0; i < candidates.size(); i ++)
     allArgs.push_back(new vector<ClassifiedArg>());

   //
   // fetch the confidences for all possible labels for all candidates
   //
   for(size_t i = 0; i < candidates.size(); i ++){
     candidates[i]->classifyForPredicate(sentence, predicate, 
					 possibleArgLabels, caseSensitive, 
					 countBeam, confBeam,
					 * allArgs[i], NULL);
   }

   //
   // sort the candidate phrases in descending order of their best prob
   //
   DescendingByProb dbp;
   sort(allArgs.begin(), allArgs.end(), dbp);
 }

 void
 Tree::classifyForPredicate(const std::vector< RCIPtr<srl::Tree> > & sentence,
			    const Tree * predicate,
			    const list<String> & allowedLabels, 
			    bool caseSensitive,
			    int countBeam,
			    double confBeam,
			    std::vector<ClassifiedArg> & result,
			    std::vector<int> * savedFeatures)
 {
   vector<ClassifiedArg> output; // stores the output cands before beam

   // this stores all confidences needed by softmax
   vector<double> allLabelConfidences;

   //
   // generate the features for this example
   //
   vector<int> features;
   vector<DebugFeature> debugFeats;
   generateExampleFeatures(sentence, 
			   predicate, 
			   predicate->getRightPosition(),
			   NULL,
			   false, 
			   features,
			   & debugFeats,
			   false, // this param not used in classification!
			   caseSensitive); 

   // save features if needed (needed for reranking)
   if(savedFeatures != NULL){
     for(size_t i = 0; i < features.size(); i ++){
       savedFeatures->push_back(features[i]);
     }
   }

   //
   // traverse all possible arg labels, and classify this phrase
   //
   for(list<String>::const_iterator ait = allowedLabels.begin();
       ait != allowedLabels.end(); ait ++){
     const String & argLabel = (* ait);

     //
     // fetch the classifier for this label
     //
     Classifier * c = Tree::getClassifier("B-" + argLabel);
     if(c == NULL){
       LOGD << "No classifier found for class: " << argLabel << endl;
       continue;
     }
     RVASSERT(c != NULL, "No classifier found for class: " << argLabel);

     //
     // classify for this label
     //
     double conf = c->classify(features);

     LOGD << "For the phrase below confidence in class " << argLabel 
	  << " and predicate " << predicate->getRightPosition()
	  << " is " << conf << ":" << endl << * this;
     LOGD << "The following features are used:";
     for(size_t i = 0; i < debugFeats.size(); i ++){
       LOGD << " " << debugFeats[i]._name << "(" 
	    << debugFeats[i]._index << ")";
     }
     LOGD << endl;

     output.push_back(ClassifiedArg(this, argLabel, conf));
     allLabelConfidences.push_back(conf);

   } // end all possible arg labels  

   //
   // estimate all probs using softmax
   //
   for(size_t i = 0; i < output.size(); i ++)
     // we actually store the log(prob)
     output[i].prob = log(softmax(output[i].conf, allLabelConfidences));

   //
   // sort outputs in descending order of conf => best will be first
   //
   DescByConf dbc;
   sort(output.begin(), output.end(), dbc);

   //
   // keep only as many cands as indicated by the beam
   //
   result.clear();
   for(size_t i = 0; i < output.size() && i < (size_t) countBeam; i ++){
     if(i > 0 && output[0].conf / output[i].conf > confBeam) break;
     result.push_back(output[i]);
   }
 }

/**
 * Verifies if this predicted argument matches a GOLD argument 
 */
static bool matchesGold(const ClassifiedArg & arg,
			const vector<ClassifiedArg> & goldArgs)
{
  for(size_t i = 0; i < goldArgs.size(); i ++){
    if(goldArgs[i] == arg) return true;
  }
  return false;
}

/**
 * Constructs the best frame for one predicate using a greedy strategy
 * @param maxLabelCount Inspects at most this number of labels per phrase
 * @param oracleMode If true it only extracts correct arguments
 */
static void fetchFrameGreedy(const vector<vector<ClassifiedArg> *> & allArgs,
			     const Tree * predicate,
			     vector<ClassifiedArg> & frame,
			     int maxLabelCount,
			     bool oracleMode,
			     const vector<ClassifiedArg> & goldArgs) 
{
  // this keeps tracks of the selections we made in the greedy process
  frame.clear();

  for(size_t i = 0; i < allArgs.size(); i ++){
    const vector<ClassifiedArg> & crtArgs = * allArgs[i];

    for(size_t j = 0; j < crtArgs.size() && j < (size_t) maxLabelCount; j ++){
      const ClassifiedArg & crtArg = crtArgs[j];

      // use conf < 0 as the stop condition (instead of the first O)
      //if(crtArg.conf < 0) break;
      //if(crtArg.label == "O") continue;

      if(satisfiesDomainConstraints(frame, crtArg) &&
	 (oracleMode == false || matchesGold(crtArg, goldArgs))){
	if(crtArg.label != "O") frame.push_back(crtArg);
	break; // no need to inspect any other labels for this phrase
      }
    }
  }
}

void Tree::
greedyClassification(const Tree * predicate,
		     vector<vector<ClassifiedArg> *> & allArgs,
		     const vector<ClassifiedArg> & goldArgs)
{
  bool showGold = false;
  if(showGold){
    cerr << "GOLD FRAME for predicate: " << predicate->getWord() << endl;
    for(size_t i = 0; i < goldArgs.size(); i ++){
      cerr << "\tArg: " << goldArgs[i].label << " " 
	   << goldArgs[i].phrase->getLeftPosition() << " " 
	   << goldArgs[i].phrase->getRightPosition() << endl;
    }
  }

  //
  // the actual classification process
  //
  bool showFrame = false;
  vector<ClassifiedArg> frame;
  fetchFrameGreedy(allArgs, predicate, frame, LOCAL_COUNT_BEAM, 
		   false, goldArgs);
  if(showFrame) cerr << "Predicate: " << predicate->getWord() << endl;
  for(size_t i = 0; i < frame.size(); i ++){
    Argument arg("B", frame[i].label, predicate->getRightPosition());
    arg.setProb(frame[i].prob);
    frame[i].phrase->addArgPrediction(arg);
    if(showFrame) cerr << "\tArg: " << frame[i].label << " " 
		       << frame[i].phrase->getLeftPosition() << " " 
		       << frame[i].phrase->getRightPosition() << endl;
  }

  //
  // oracle mode: compute the position of the best argument
  //
  if(getOracleMode() == true){
    frame.clear();
    for(size_t i = 0; i < allArgs.size(); i ++){
      const vector<ClassifiedArg> & crtArgs = * allArgs[i];
      for(size_t j = 0; j < crtArgs.size(); j ++){
	const ClassifiedArg & crtArg = crtArgs[j];
	if(satisfiesDomainConstraints(frame, crtArg) &&
	   matchesGold(crtArg, goldArgs)){
	  if(crtArg.label != "O") bestPositions[j] ++;
	  break; 
	}
      }
    }
  }

  //
  // oracle mode: upper score when using the top N labels per candidate
  //
  if(getOracleMode() == true){
    for(int i = 1; i < LOCAL_COUNT_BEAM; i ++){
      vector<ClassifiedArg> frame;
      fetchFrameGreedy(allArgs, predicate, frame, i, true, goldArgs);
      oracleScores[i].mTotal += goldArgs.size();
      for(size_t j = 0; j < frame.size(); j ++){
	if(frame[j].label != "O"){
	  oracleScores[i].mPredicted ++;
	  if(matchesGold(frame[j], goldArgs)) oracleScores[i].mCorrect ++;
	}
      }
    }
  }
 }

 void Tree::findCandidatesForPredicate(const Tree * predicate,
				       const Tree * predParent,
				       std::vector<Tree *> & candidates)
 {
   if(! includesPosition(predicate->getRightPosition()) && // NOT contains pred
      isValidArgument(predicate, predParent) && // Lluis' pruning heuristic
      _parent != NULL){
     candidates.push_back(this);
   }

   for(list<TreePtr>::iterator it = _children.begin();
       it != _children.end(); it ++){
     (* it)->findCandidatesForPredicate(predicate, predParent, candidates);
   }
 }

 void Tree::loadAcceptableArgumentLabels(const String & lemma,
					 list<String> & args)
 {
   static char * amLabels [] = { 
     "AM-ADV", "AM-CAU", "AM-DIR", "AM-DIS", "AM-EXT",
     "AM-LOC", "AM-MNR", "AM-MOD", "AM-NEG", "AM-PNC",
     "AM-TMP", "R-AM-LOC", "R-AM-MNR", "R-AM-TMP", "X", NULL
   };

   // the AM args
   for(int i = 0; amLabels[i] != NULL; i ++){
     args.push_back(amLabels[i]);
   }

   // the numeric args + R-* + C-*
   bool * frame = Lexicon::getFrame(lemma);
   // RVASSERT(frame != NULL, "Found NULL frame for verb: " << lemma);
   for(int i = 0; i < MAX_NUMBERED_ARGS; i ++){
     if(frame == NULL || frame[i] == true){
       ostringstream os;
       os << "A" << i;
       args.push_back(os.str());
       args.push_back(merge("R-", os.str()));
       args.push_back(merge("C-", os.str()));
     }
   }

   // the NIL class
   args.push_back("O");
 }

 /** Adds a candidate to the list, 
     maintaining the list in descending order of the candidate log prob
 */
 static void addInOrder(list<UnitCandidate *> & cands,
			UnitCandidate * cand)
 {
   for(list<UnitCandidate *>::iterator it = cands.begin();
       it != cands.end(); it ++){
     if((* it)->mProb < cand->mProb){
       cands.insert(it, cand);
       return;
     }
   }
   cands.push_back(cand);
 }

 static void deleteCandList(list<UnitCandidate *> * cands)
 {
   for(list<UnitCandidate *>::iterator it = cands->begin();
       it != cands->end(); it ++){
     delete * it;
   }
   delete cands;
 }

/** Trims the list to at most maxSize elements */
static void trim(list<UnitCandidate *> & cands,
		 size_t maxSize) {
  while(cands.size() > maxSize){
    UnitCandidate * back = cands.back();
    cands.pop_back();
    delete back;
  }
}

static UnitCandidate * selectBestFrame(const list<UnitCandidate *> & cands,
				       int maxCount)
{
  int pos = 0;
  UnitCandidate * best = NULL;
  for(list<UnitCandidate *>::const_iterator it = cands.begin();
      it != cands.end() && pos < maxCount; it ++, pos ++){
    UnitCandidate * crt = * it;
    if(best == NULL || crt->mScore.F1() > best->mScore.F1()){
      best = crt;
    }
  }
  return best;
}

void Tree::
rerankingClassification(const Tree * predicate,
			vector<vector<ClassifiedArg> *> & allArgs,
			int beam,
			const vector<ClassifiedArg> & goldArgs)
{
  bool showGold = false;
  bool showFrame = false;

  //
  // generate the top beam solutions
  //
  list<UnitCandidate *> * oldCands = new list<UnitCandidate *>();
  list<UnitCandidate *> * newCands = new list<UnitCandidate *>();
  
  for(size_t i = 0; i < allArgs.size(); i ++){
    vector<ClassifiedArg> * crtArgs = allArgs[i];
    for(size_t j = 0; j < crtArgs->size(); j ++){
      const ClassifiedArg & crtArg = (* crtArgs)[j];

      // the partial frames are empty => start them here
      if(oldCands->size() == 0){
	addInOrder(* newCands, new UnitCandidate(crtArg));
      } 
      
      // we continue an existing set of partial frames
      else {
	for(list<UnitCandidate *>::iterator oldIt = oldCands->begin();
	    oldIt != oldCands->end(); oldIt ++){
	  // add the new arg only if domain constraints are Ok
	  if(satisfiesDomainConstraints((* oldIt)->mSequence, crtArg)){
	    UnitCandidate * cand = (* oldIt)->makeCand(crtArg);
	    addInOrder(* newCands, cand);
	  } 
	}
      }
    }

    //
    // newCands stores the new set of cands expanded with the current cand
    // move newCands -> oldCands (this will store the final set of frames) 
    //
    if(newCands->size() > 0){
      // trim the new candidate list to beam elements
      trim(* newCands, beam);

      deleteCandList(oldCands);
      oldCands = newCands;
      newCands = new list<UnitCandidate *>();
    }
  }

  //
  // oracle mode: compute the scores of all candidate frames
  //
  if(getOracleMode() == true){
    for(list<UnitCandidate *>::const_iterator it = oldCands->begin(); 
	it != oldCands->end(); it ++){
      UnitCandidate * unit = * it;
      unit->computeScore(goldArgs);
    }
  }

  //
  // this will store the best frame for this predicate
  //
  UnitCandidate * best = NULL;
  if(oldCands->size() > 0) best = oldCands->front();
  /*
  //
  // Use the code below to compute the true oracle score!
  // Note that the the oracle scores stored in oracleScores use only 
  // the gold props that could be matched on the predicted syntax. Hence
  // these scores are ALWAYS higher than the true oracle scores (by ~5%).
  //
  // BEGIN TRUE ORACLE
  int position = 0;
  for(list<UnitCandidate *>::const_iterator it = oldCands->begin(); 
      it != oldCands->end() && position < GLOBAL_BEAM; it ++, position ++){
    UnitCandidate * unit = * it;
    if(best == NULL || unit->mScore.F1() > best->mScore.F1()){
      best = unit;
    }
  }
  // END TRUE ORACLE
  */

  if(showGold){
    cerr << "GOLD FRAME for predicate: " << predicate->getWord() << endl;
    for(size_t i = 0; i < goldArgs.size(); i ++){
      cerr << "\tArg: " << goldArgs[i].label << " " 
	   << goldArgs[i].phrase->getLeftPosition() << " " 
	   << goldArgs[i].phrase->getRightPosition() << endl;
    }
  }

  if(showFrame && best != NULL){
    cerr << "Predicate: " << predicate->getWord() << endl;
    for(size_t i = 0; i < best->mSequence.size(); i ++){
      if(best->mSequence[i].label != "O"){
	cerr << "\tArg: " << best->mSequence[i].label << " " 
	     << best->mSequence[i].phrase->getLeftPosition() << " " 
	     << best->mSequence[i].phrase->getRightPosition() << endl;
      }
    }
  }

  //
  // create actual args from the best solution
  //
  if(best != NULL){
    for(size_t j = 0; j < best->size(); j ++){
      ClassifiedArg & cand = best->get(j);
      if(cand.label != "O"){
	Argument arg("B", cand.label, predicate->getRightPosition());
	arg.setProb(cand.prob);
	cand.phrase->addArgPrediction(arg);
      } 
    }
  }

  //
  // oracle mode: compute the position of the best frame
  //
  if(getOracleMode() == true){
    int position = 0;
    int bestPosition = 0;
    UnitCandidate * bestFrame = NULL;
    for(list<UnitCandidate *>::const_iterator it = oldCands->begin(); 
	it != oldCands->end(); it ++, position ++){
      UnitCandidate * unit = * it;
      if(bestFrame == NULL || unit->mScore.F1() > bestFrame->mScore.F1()){
	bestFrame = unit;
	bestPosition = position;
      }
    }
    bestPositions[bestPosition] ++;    
  }
  
  //
  // oracle mode: upper score when using the top N frames
  //
  if(getOracleMode() == true){
    for(int i = 1; i < (int) oldCands->size() && i < GLOBAL_BEAM; i ++){
      UnitCandidate * bestFrame = selectBestFrame(* oldCands, i);
      oracleScores[i].mCorrect += bestFrame->mScore.mCorrect;
      oracleScores[i].mTotal += bestFrame->mScore.mTotal;
      oracleScores[i].mPredicted += bestFrame->mScore.mPredicted;
    }
  }
  
  deleteCandList(oldCands);
  deleteCandList(newCands);
}

void Tree::expandArguments(const std::vector< RCIPtr<srl::Tree> > & sentence,
			   const Tree * predicate,
			   int sentenceIndex, 
			   int predicateIndex)
{
  //cerr << "Expanding args in sentence " << sentenceIndex << " with predicate " << predicateIndex << endl;

  if(_parent != NULL){
    for(list<TreePtr>::iterator it = _children.begin();
	it != _children.end(); ){
      Tree * crt = (* it).operator->();
      int position = predicate->getRightPosition();
      Argument barg;
      
      // found a B term
      if(crt->isTerminal() &&
	 crt->includesArgsForVerb(position, barg, false) &&
	 barg.getType() == "B"){
	 // predicate->includedIn(this)){ // XXX: not true in devel!

	// expand this argument until it reaches:
	// end of phrase OR predicate OR another arg for the same verb OR ``
	list<TreePtr>::iterator jt = it;
	Argument iarg;
	for(++ jt; 
	    jt != _children.end() &&
	    (* jt)->isTerminal() &&
	    predicate->includedIn((* jt).operator->()) == false &&
	    (* jt)->includesArgsForVerb(position, iarg, false) == false &&
	    (* jt)->getHeadWord() != "``" &&
	    (* jt)->getHeadWord() != "''";
	    jt ++){
	  (* jt)->addArgPrediction(Argument("I", barg.getName(), position));
	}

	it = jt;
      }

      else {
	it ++;
      }
    }
  }

  for(list<TreePtr>::iterator it = _children.begin();
      it != _children.end(); it ++){
    (* it)->expandArguments(sentence, predicate, sentenceIndex, predicateIndex);
  }
}

bool Tree::includesArgsForVerb(int position, 
			       Argument & arg,
			       bool recursive) const
{
  for(list<Argument>::const_iterator it = _predictedArguments.begin();
      it != _predictedArguments.end(); it ++){
    if((* it).getVerbPosition() == position){
      arg = * it;
      return true;
    }
  }

  if(recursive){
    for(list<TreePtr>::const_iterator it = _children.begin();
	it != _children.end(); it ++){
      if((* it)->includesArgsForVerb(position, arg, recursive)) return true;
    }
  }

  return false;
}

void Tree::dumpCoNLL(OStream & os,
		     const std::vector< RCIPtr<srl::Tree> > & sentence,
		     bool showProbabilities)
{
  // detect all predicates in this phrase
  vector<int> predPositions;
  detectPredicates(predPositions);

  // CoNLL output matrix
  vector< vector<String> > matrix;
  matrix.resize(sentence.size());
  for(size_t row = 0; row < matrix.size(); row ++){
    matrix[row].resize(1 + predPositions.size(), "*");
  }

  // probability matrix:
  //   the probability of each argument set at the same position as the B token
  //   unused entries are set to 0.0
  vector< vector<double> > probMatrix;
  probMatrix.resize(sentence.size());
  for(size_t row = 0; row < probMatrix.size(); row ++){
    probMatrix[row].resize(1 + predPositions.size(), 0.0);
  }

  // populate predicate lemmas
  for(size_t row = 0; row < matrix.size(); row ++){
    if(sentence[row]->isPredicate()) 
      matrix[row][0] = sentence[row]->getLemma();
    else
      matrix[row][0] = "-";
  }

  // set args for each predicate
  for(size_t predIndex = 0; predIndex < predPositions.size(); predIndex ++){
    // dump args in IOB2 notation
    dumpCoNLLColumnIOB(sentence, predPositions[predIndex], 
		       matrix, probMatrix, predIndex + 1);

    // convert IOB2 to CoNLL paren format
    convertIOB2ToParens(matrix, predIndex + 1);

    // some obvious corrections
    // XXX: you can disable expansion heuristics here!
    correctionHeuristics(sentence, matrix, probMatrix, predIndex + 1);
  }

  // dump to stream
  for(size_t row = 0; row < matrix.size(); row ++){
    for(size_t column = 0; column < matrix[row].size(); column ++){
      if(column > 0) os << "\t\t";
      os << matrix[row][column];
      if(column > 0 && showProbabilities)
	os << "\t" << probMatrix[row][column];
    }
    os << endl;
  }
  os << endl;
}

void Tree::dumpCoNLLColumnIOB(const std::vector< RCIPtr<srl::Tree> > & sentence,
			      int predicatePosition, 
			      std::vector< std::vector<String> > & matrix,
			      std::vector< std::vector<double> > & probMatrix,
			      int column) const
{
  bool found = false;
  for(list<Argument>::const_iterator it = _predictedArguments.begin();
      it != _predictedArguments.end(); it ++){
    if((* it).getVerbPosition() == predicatePosition){
      if((* it).getType() == "B"){
	matrix[getLeftPosition()][column] = merge("B-", (* it).getName());
	probMatrix[getLeftPosition()][column] = (* it).getProb();
      } else {
	matrix[getLeftPosition()][column] = merge("I-", (* it).getName());
      }

      for(int i = getLeftPosition() + 1; i <= getRightPosition(); i ++){
	matrix[i][column] = merge("I-", (* it).getName());
      }

      found = true;
      break;
    }
  }

  if(! found){
    for(list<TreePtr>::const_iterator it = _children.begin();
	it != _children.end(); it ++){
      (* it)->dumpCoNLLColumnIOB(sentence, predicatePosition,
				 matrix, probMatrix, column);
    }
  }
}

static bool argContainsFromStart(const String & word,
				 const std::vector< RCIPtr<srl::Tree> > & sentence,
				 std::vector< std::vector<String> > & matrix,
				 int row, 
				 int column) 
{
  for(size_t i = row; i < matrix.size(); i ++){
    if(sentence[i]->getHeadWord() == word) return true;
    if(matrix[i][column].size() > 0 &&
       matrix[i][column][matrix[i][column].size() - 1] == ')') break;
  }
  return false;
}

static bool argContainsFromEnd(const String & word,
			       const std::vector< RCIPtr<srl::Tree> > & sentence,
			       std::vector< std::vector<String> > & matrix,
			       int row, 
			       int column) 
{
  for(int i = row; i >= 0; i --){
    if(sentence[i]->getHeadWord() == word) return true;
    if(matrix[i][column].size() > 0 &&
       matrix[i][column][0] == '(') break;
  }
  return false;
}

void Tree::correctionHeuristics(const std::vector< RCIPtr<srl::Tree> > & sentence,
				std::vector< std::vector<String> > & matrix,
				std::vector< std::vector<double> > & probMatrix,
				int column) const
{
  // Include `` in the argument if '' already included in the same arg
  for(size_t row = 0; row < matrix.size(); row ++){
    if(sentence[row]->getHeadWord() == "``" &&
       row < matrix.size() - 1 &&
       startsWith(matrix[row + 1][column], "(A") &&
       argContainsFromStart("''", sentence, matrix, row + 1, column)){
      int star = matrix[row + 1][column].find_first_of("*");
      RVASSERT(star > 0 && star < (int) matrix[row + 1][column].size(),
	       "Invalid argument label: " << matrix[row + 1][column]);
      matrix[row][column] = 
	matrix[row + 1][column].substr(0, star + 1);
      matrix[row + 1][column] = 
	matrix[row + 1][column].substr(star, 
				       matrix[row + 1][column].size() - star);

      probMatrix[row][column] = probMatrix[row + 1][column];
      probMatrix[row + 1][column] = 0;

      row ++;
    }
  }

  // Include ,'' OR '' in the argument if `` already included in the same arg
  for(size_t row = 0; row < matrix.size(); row ++){
    if(endsWith(matrix[row][column], "*)") &&
       argContainsFromEnd("``", sentence, matrix, row, column)){

      // found , ''
      if(row < matrix.size() - 2 &&
	 sentence[row + 1]->getHeadWord() == "," &&
	 sentence[row + 2]->getHeadWord() == "''"){
	int star = matrix[row][column].find_first_of("*");
	RVASSERT(star >= 0 && star < (int) matrix[row][column].size(),
		 "Invalid argument label: " << matrix[row][column]);
	matrix[row][column] = matrix[row][column].substr(0, star + 1);
	matrix[row + 1][column] = "*";
	matrix[row + 2][column] = "*)";
	row += 2;
      }

      // found ''
      if(row < matrix.size() - 1 &&
	 sentence[row + 1]->getHeadWord() == "''"){
	int star = matrix[row][column].find_first_of("*");
	RVASSERT(star >= 0 && star < (int) matrix[row][column].size(),
		 "Invalid argument label: " << matrix[row][column]);
	matrix[row][column] = matrix[row][column].substr(0, star + 1);
	matrix[row + 1][column] = "*)";
	row ++;
      }
    }
  }
}

void Tree::convertIOB2ToParens(std::vector< std::vector<String> > & matrix,
			       int column) const
{
  for(size_t row = 0; row < matrix.size(); row ++){
    if(matrix[row][column] == "*") continue;

    RVASSERT(matrix[row][column].size() >= 3, 
	     "Invalid arg label: " << matrix[row][column]);
    String type = matrix[row][column].substr(0, 1);
    String name = matrix[row][column].substr(2, matrix[row][column].size() - 2);
    String nextType = "O";
    String nextName;
    if(row < matrix.size() - 1 && matrix[row + 1][column] != "*"){
      RVASSERT(matrix[row + 1][column].size() >= 3, 
	       "Invalid arg label: " << matrix[row + 1][column]);
      nextType = matrix[row + 1][column].substr(0, 1);
      nextName = matrix[row + 1][column].substr(2, matrix[row + 1][column].size() - 2);
    }

    if(type == "B"){
      if(name == "X"){
	matrix[row][column] = "*";
      } else if(nextType == "I" && nextName == name){
	matrix[row][column] = merge("(", name, "*");
      } else {
	matrix[row][column] = merge("(", name, "*)");
      }
    } else if(type == "I"){
      if(name == "X"){
	matrix[row][column] = "*";
      } else if(nextType == "I" && nextName == name){
	matrix[row][column] = "*";
      } else {
	matrix[row][column] = "*)";
      }
    }
  }
}

static void displayString(OStream & os,
			  const String & str,
			  size_t fillSize)
{
  os << str;
  if(fillSize > str.size()){
    for(size_t i = 0; i < fillSize - str.size(); i ++){
      os << " ";
    }
  } else {
    os << " ";
  }
}

#define ARGS_OFFSET 4 // lemma + pred + POS + tree
#define POS_POSITION 0
#define TREE_POSITION 1
#define LEMMA_POSITION 2
#define PRED_POSITION 3

#define MAX_LEMMA_SIZE 16
#define MAX_ARG_SIZE 10
#define MAX_POS_TAG_SIZE 6
#define MAX_TREE_LABEL_SIZE 16

void Tree::dumpExtendedCoNLL(OStream & os,
			     const vector< RCIPtr<srl::Tree> > & sentence)
{
  // detect all predicates in this phrase
  vector<int> predPositions;
  detectPredicates(predPositions);

  // CoNLL output matrix
  vector< vector<String> > matrix;
  matrix.resize(sentence.size());
  for(size_t row = 0; row < matrix.size(); row ++){
    matrix[row].resize(ARGS_OFFSET + predPositions.size(), "*");
  }

  // probability matrix
  vector< vector<double> > probMatrix;
  probMatrix.resize(sentence.size());
  for(size_t row = 0; row < probMatrix.size(); row ++){
    probMatrix[row].resize(ARGS_OFFSET + predPositions.size(), 0.0);
  }

  // the POS tags
  for(size_t row = 0; row < matrix.size(); row ++){
    matrix[row][POS_POSITION] = sentence[row]->getLabel();
  }

  // the tree label
  for(size_t row = 0; row < matrix.size(); row ++){
    matrix[row][TREE_POSITION] = sentence[row]->generateTreeLabel();
  }

  // populate lemmas for all words
  for(size_t row = 0; row < matrix.size(); row ++){
    matrix[row][LEMMA_POSITION] = quotify(sentence[row]->getLemma());
  }

  // mark predicates
  for(size_t row = 0; row < matrix.size(); row ++){
    if(sentence[row]->isPredicate()) 
      matrix[row][PRED_POSITION] = "1";
    else
      matrix[row][PRED_POSITION] = "0";
  }

  // set args for each predicate
  for(size_t predIndex = 0; predIndex < predPositions.size(); predIndex ++){
    // dump args in IOB2 notation
    dumpCoNLLColumnIOB(sentence, predPositions[predIndex], 
		       matrix, probMatrix, predIndex + ARGS_OFFSET);

    // convert IOB2 to CoNLL paren format
    convertIOB2ToParens(matrix, predIndex + ARGS_OFFSET);

    // some obvious corrections
    // XXX: you can disable expansion heuristics here!
    correctionHeuristics(sentence, matrix, probMatrix, predIndex + ARGS_OFFSET);
  }

  // dump to stream
  for(size_t row = 0; row < matrix.size(); row ++){
    for(size_t column = 0; column < matrix[row].size(); column ++){
      int maxSize = 0;
      if(column == LEMMA_POSITION) maxSize = MAX_LEMMA_SIZE;
      else if(column == PRED_POSITION) maxSize = 2;
      else if(column == POS_POSITION) maxSize = MAX_POS_TAG_SIZE;
      else if(column == TREE_POSITION) maxSize = MAX_TREE_LABEL_SIZE;
      else maxSize = MAX_ARG_SIZE; 
      displayString(os, matrix[row][column], maxSize);
    }
    os << endl;
  }
  // one empty line after every sentence
  os << endl;
}

String Tree::generateTreeLabel() const
{
  list<String> labels;

  // The current terminal is head of its phrase
  if(isTerminal() && _parent != NULL && _parent->_head == this)
    labels.push_back("^*");
  else
    labels.push_back("*");

  // get labels to the left of *
  for(const Tree * current = this; 
      current->_parent != NULL;
      current = current->_parent){
    if(current->_leftSibling == NULL){
      String headPrefix = "";
      // the crt parent is a head phrase
      if(current->_parent->_parent != NULL &&
	 current->_parent->_parent->_head == current->_parent)
	headPrefix = HEAD_PREFIX;
      labels.push_front(headPrefix + current->_parent->getLabel());
    }
    else 
      break;
  }

  

  // get labels to the right of *
  for(const Tree * current = this; 
      current->_parent != NULL;
      current = current->_parent){
    if(current->_rightSibling == NULL) 
      labels.push_back(current->_parent->getLabel());
    else 
      break;
  }

  //
  // generate the full label
  //
  ostringstream os;
  bool seenStar = false;
  for(list<String>::const_iterator it = labels.begin();
      it != labels.end(); it ++){
    if(endsWith((* it), "*")){
      os << * it;
      seenStar = true;
    } else if(seenStar == false){
      os << "(";
      os << * it;
    } else{
      os << ")";
    }
  }

  return os.str();
}

//
// Compute some scoring stats
//

StringMap<Stats *> Tree::_stats;

Stats * Tree::getStats(const String & arg)
{
  Stats * s = NULL;
  if(_stats.get(arg.c_str(), s) == false){
    s = new Stats();
    _stats.set(arg.c_str(), s);
  }
  RVASSERT(s != NULL, "Can not find stats for argument: " << arg);
  return s;
}

static bool argsInclude(const list<Argument> & args,
			const Argument & arg)
{
  for(list<Argument>::const_iterator it = args.begin();
      it != args.end(); it ++)
    if(* it == arg) return true;
  return false;
}

void Tree::gatherScoringStats() const
{
  for(list<Argument>::const_iterator it = _predictedArguments.begin();
      it != _predictedArguments.end(); it ++){
    Stats * s = getStats((* it).getLabel());
    if(argsInclude(_arguments, * it)) s->correct ++;
    else s->excess ++;
  }

  for(list<Argument>::const_iterator it = _arguments.begin();
      it != _arguments.end(); it ++){
    Stats * s = getStats((* it).getLabel());
    if(! argsInclude(_predictedArguments, * it)) s->missed ++;
  }

  for(list<TreePtr>::const_iterator it = _children.begin();
      it != _children.end(); it ++){
    (* it)->gatherScoringStats();
  }
}

void Tree::dumpScoringStats(std::ostream & os)
{
  os << "              corr. excess missed     prec.      rec.        F1" 
     << endl
     <<"-----------------------------------------------------------------" 
     << endl;
  
  vector<String> keys;
  for(StringMap<Stats *>::const_iterator it = _stats.begin();
      it != _stats.end(); it ++) keys.push_back((* it).second->getKey());

  sort(keys.begin(), keys.end());

  int correct = 0;
  int missed = 0;
  int excess = 0;

  for(size_t i = 0; i < keys.size(); i ++){
    Stats * s = getStats(keys[i]);

    correct += s->correct;
    missed += s->missed;
    excess += s->excess;

    double prec = 0;
    double rec = 0;
    double f1 = 0;
    if(s->correct > 0){
      prec = ((double) s->correct) / ((double) (s->correct + s->excess));
      rec = ((double) s->correct) / ((double) (s->correct + s->missed));
      f1 = (2 * prec * rec) / (prec + rec);
    }
    os << setw(12) << keys[i] << " " 
       << setw(6) << s->correct << " " 
       << setw(6) << s->excess << " " 
       << setw(6) << s->missed << " " 
       << setw(9) << prec << " " 
       << setw(9) << rec << " " 
       << setw(9) << f1 << endl;
  }

  double totalPrec = 0;
  double totalRec = 0;
  double totalF1 = 0;

  if(correct > 0){
      totalPrec = ((double) correct) / ((double) (correct + excess));
      totalRec = ((double) correct) / ((double) (correct + missed));
      totalF1 = (2 * totalPrec * totalRec) / (totalPrec + totalRec);
    }

  os << "-----------------------------------------------------------------" 
     << endl << setw(12) << "Total" << " "
     << setw(6) << correct << " "
     << setw(6) << excess << " "
     << setw(6) << missed << " "
     << setw(9) << totalPrec << " "
     << setw(9) << totalRec << " " 
     << setw(9) << totalF1 << endl;
}

static bool argIncludedIn(const Argument & arg,
			  const std::list<srl::Argument> & args)
{
  for(list<Argument>::const_iterator it = args.begin();
      it != args.end(); it ++){
    if(* it == arg){
      return true;
    }
  }

  return false;
}

void Tree::gatherListOfMissedArgs(std::list<std::string> & missedArgs,
				  std::list<std::string> & excessArgs) const
{
  for(list<Argument>::const_iterator it = _arguments.begin();
      it != _arguments.end(); it ++){
    if(argIncludedIn(* it, _predictedArguments) == false){
      missedArgs.push_back((* it).getFullName());
    }
  }

  for(list<Argument>::const_iterator it = _predictedArguments.begin();
      it != _predictedArguments.end(); it ++){
    if(argIncludedIn(* it, _arguments) == false){
      excessArgs.push_back((* it).getFullName());
    }
  }

  for(list<TreePtr>::const_iterator it = _children.begin();
      it != _children.end(); it ++){
    (* it)->gatherListOfMissedArgs(missedArgs, excessArgs);
  }
}

