
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "Constants.h"
#include "CharUtils.h"
#include "Lexicon.h"
#include "Assert.h"

using namespace std;
using namespace srl;

StringMap<int> Lexicon::_wordCounts;
StringMap<int> Lexicon::_lemmaCounts;
StringMap<int> Lexicon::_labelCounts;
StringMap<int> Lexicon::_syntacticLabelCounts;
StringMap<int> Lexicon::_biCounts;

StringMap<int> Lexicon::_tmpNN;
StringMap<int> Lexicon::_tmpNNP;
StringMap<int> Lexicon::_tmpIN;
StringMap<int> Lexicon::_tmpRB;

StringMap<FeatureData *> Lexicon::_features;
int Lexicon::_featureIndex = 1;

StringMap<bool *> Lexicon::_frames;

StringMap<StringMap<int> *> Lexicon::_validLabels;

StringMap<std::vector<VerbParticle> *> Lexicon::_verbParticles;

int Lexicon::maxTokenDistance = 0;

Lexicon::Lexicon()
{
}

int Lexicon::getWordCount(const Char * word)
{
  int count;
  if(_wordCounts.get(word, count) == true){
    return count;
  }

  return 0;
}

void Lexicon::addWordCount(const String & word)
{
  RCIPtr< StringMapEntry<int> > value = _wordCounts.get(word.c_str());
  if(value.operator->() == NULL){
    _wordCounts.set(word.c_str(), 1);
  } else{
    value->setValue(value->getValue() + 1);
  }
}

int Lexicon::getLemmaCount(const Char * word)
{
  int count;
  if(_lemmaCounts.get(word, count) == true){
    return count;
  }

  return 0;
}

void Lexicon::addLemmaCount(const String & word)
{
  RCIPtr< StringMapEntry<int> > value = _lemmaCounts.get(word.c_str());
  if(value.operator->() == NULL){
    _lemmaCounts.set(word.c_str(), 1);
  } else{
    value->setValue(value->getValue() + 1);
  }
}

void Lexicon::addArgLabelCount(const String & label)
{
  RCIPtr< StringMapEntry<int> > value = _labelCounts.get(label.c_str());
  if(value.operator->() == NULL){
    _labelCounts.set(label.c_str(), 1);
  } else{
    value->setValue(value->getValue() + 1);
  }
}

void Lexicon::addSyntacticLabelCount(const String & label)
{
  RCIPtr< StringMapEntry<int> > value = 
    _syntacticLabelCounts.get(label.c_str());
  if(value.operator->() == NULL){
    _syntacticLabelCounts.set(label.c_str(), 1);
  } else{
    value->setValue(value->getValue() + 1);
  }
}

void Lexicon::addBICount(const String & label)
{
  RCIPtr< StringMapEntry<int> > value = 
    _biCounts.get(label.c_str());
  if(value.operator->() == NULL){
    _biCounts.set(label.c_str(), 1);
  } else{
    value->setValue(value->getValue() + 1);
  }
}

class Count {
public:
  String name;
  int count;
};

static void descAppend(vector<Count> & sorted,
		       const String & name,
		       int count)
{
  for(vector<Count>::iterator it = sorted.begin();
      it != sorted.end(); it ++){
    if(count > (* it).count){
      Count c;
      c.name = name;
      c.count = count;
      sorted.insert(it, c);
      return;
    }
  }

  Count c;
  c.name = name;
  c.count = count;
  sorted.push_back(c);
}

void Lexicon::saveBICounts(std::ostream & os)
{
  vector<Count> sc;
  for(StringMap<int>::const_iterator it = _biCounts.begin();
      it != _biCounts.end(); it ++){
    descAppend(sc, (* it).second->getKey(), (* it).second->getValue());
  }
  for(size_t i = 0; i < sc.size(); i ++){
    os << sc[i].name << " " << sc[i].count << endl;
  }
}

static bool goodParticle(const VerbParticle & part)
{
  if(part._count > 0 &&
     part._word != "that" &&
     part._word != "for") 
    return true;
  return false;
}

void Lexicon::saveCounts(const String & path)
{
  string wname = mergeStrings(path, "/word.counts");
  ofstream ws(wname.c_str());
  RVASSERT(ws, "Can not create word count stream!");
  string lname = mergeStrings(path, "/lemma.counts");
  ofstream ls(lname.c_str());
  RVASSERT(ls, "Can not create lemma count stream!");
  string aname = mergeStrings(path, "/args.counts");
  ofstream as(aname.c_str());
  RVASSERT(as, "Can not create arg count stream!");
  string sname = mergeStrings(path, "/label.counts");
  ofstream ss(sname.c_str());
  RVASSERT(ss, "Can not create label count stream!");

  for(StringMap<int>::const_iterator it = _wordCounts.begin();
      it != _wordCounts.end(); it ++){
    ws << (* it).second->getKey() << " " 
       << (* it).second->getValue() << endl;
  }

  for(StringMap<int>::const_iterator it = _lemmaCounts.begin();
      it != _lemmaCounts.end(); it ++){
    ls << (* it).second->getKey() << " " 
       << (* it).second->getValue() << endl;
  }

  vector<Count> lc;
  for(StringMap<int>::const_iterator it = _labelCounts.begin();
      it != _labelCounts.end(); it ++){
    descAppend(lc, (* it).second->getKey(), (* it).second->getValue());
  }
  for(size_t i = 0; i < lc.size(); i ++){
    as << lc[i].name << " " << lc[i].count << endl;
  }

  vector<Count> sc;
  for(StringMap<int>::const_iterator it = _syntacticLabelCounts.begin();
      it != _syntacticLabelCounts.end(); it ++){
    descAppend(sc, (* it).second->getKey(), (* it).second->getValue());
  }
  for(size_t i = 0; i < sc.size(); i ++){
    ss << sc[i].name << " " << sc[i].count << endl;
  }

  string vname = mergeStrings(path, "/valid.labels");
  ofstream vs(vname.c_str());
  RVASSERT(vs, "Can not create valid labels stream!");
  for(StringMap<StringMap<int> *>::const_iterator it = _validLabels.begin();
      it != _validLabels.end(); it ++){
    const Char * arg = (* it).second->getKey();
    StringMap<int> * labels = (* it).second->getValue();
    RVASSERT(labels != NULL, "NULL valid label set for arg " << arg);
    
    vs << arg;
    for(StringMap<int>::const_iterator lit = _syntacticLabelCounts.begin();
      lit != _syntacticLabelCounts.end(); lit ++){
      vs << " " << (* lit).second->getKey();
    }
    vs << endl;
  }

  string pname = mergeStrings(path, "/verb.particles");
  ofstream ps(pname.c_str());
  RVASSERT(ps, "Can not create verb particle stream!");
  for(StringMap<vector<VerbParticle> *>::const_iterator it = 
	_verbParticles.begin();
      it != _verbParticles.end(); it ++){
    const Char * verb = (* it).second->getKey();
    vector<VerbParticle> * particles = (* it).second->getValue();
    RVASSERT(particles != NULL, "NULL particle vector for verb " << verb);

    bool empty = true;
    for(size_t i = 0; i < particles->size(); i ++){
      if(goodParticle((* particles)[i])){
	empty = false;
	break;
      }
    }

    if(! empty){
      ps << verb;
      for(size_t i = 0; i < particles->size(); i ++){
	if(goodParticle((* particles)[i])){
	  ps << " " << (* particles)[i]._word 
	     << " " << (* particles)[i]._count;
	}
      }
      ps << endl;
    }
  }
}

#define MAX_STATS_LINE 1024 

void Lexicon::loadCounts(const String & path)
{
  string wname = mergeStrings(path, "/word.counts");
  ifstream ws(wname.c_str());
  RVASSERT(ws, "Can not open word count stream!");
  string lname = mergeStrings(path, "/lemma.counts");
  ifstream ls(lname.c_str());
  RVASSERT(ls, "Can not open lemma count stream!");

  string sname = mergeStrings(path, "/label.counts");
  ifstream ss(sname.c_str());
  RVASSERT(ss, "Can not open label count stream!");
  

  char line[MAX_STATS_LINE];

  // load word counts
  while(ws.getline(line, MAX_STATS_LINE)){
    vector<String> tokens;
    simpleTokenize(line, tokens, " \t\n\r");
    if(tokens.size() == 2){
      int count = strtol(tokens[1].c_str(), NULL, 10);
      RVASSERT(count > 0, "Invalid count for word " << tokens[0]);
      _wordCounts.set(tokens[0].c_str(), count);
    }
  }

  cerr << "Loaded " << _wordCounts.size() << " words." << endl;

  // load lemma counts
  while(ls.getline(line, MAX_STATS_LINE)){
    vector<String> tokens;
    simpleTokenize(line, tokens, " \t\n\r");
    if(tokens.size() == 2){
      int count = strtol(tokens[1].c_str(), NULL, 10);
      RVASSERT(count > 0, "Invalid count for lemma " << tokens[0]);
      _lemmaCounts.set(tokens[0].c_str(), count);
    }
  }

  cerr << "Loaded " << _lemmaCounts.size() << " lemmas." << endl;

  // load syntactic label counts
  while(ss.getline(line, MAX_STATS_LINE)){
    vector<String> tokens;
    simpleTokenize(line, tokens, " \t\n\r");
    if(tokens.size() == 2){
      int count = strtol(tokens[1].c_str(), NULL, 10);
      RVASSERT(count > 0, "Invalid count for lemma " << tokens[0]);
      _syntacticLabelCounts.set(tokens[0].c_str(), count);
    }
  }
  cerr << "Loaded " << _syntacticLabelCounts.size() 
       << " syntactic labels." << endl;

  string pname = mergeStrings(path, "/verb.particles");
  ifstream ps(pname.c_str());
  RVASSERT(ps, "Can not open verb particles stream!");
  while(ps.getline(line, MAX_STATS_LINE)){
    vector<String> tokens;
    simpleTokenize(line, tokens, " \t\n\r");

    for(size_t i = 1; i < tokens.size() - 1; i += 2){
      int count = strtol(tokens[i + 1].c_str(), NULL, 10);
      RVASSERT(count > 0,
	       "Invalid count " << count << " for particle " << tokens[i]);
      addVerbParticle(tokens[0], tokens[i], count);
    }
  }
}

void Lexicon::loadTmps(const String & path)
{
  string tname = mergeStrings(path, "/words.temporal");
  ifstream is(tname.c_str());
  RVASSERT(is, "Can not open temporal word stream!");

  char line[MAX_STATS_LINE];

  // load lemma counts
  while(is.getline(line, MAX_STATS_LINE)){
    vector<String> tokens;
    simpleTokenize(line, tokens, " \t\n\r");
    if(tokens.size() == 4){
      if(tokens[3] == "IN"){
	_tmpIN.set(tokens[1].c_str(), 1);
      } else if(tokens[3] == "RB"){
	_tmpRB.set(tokens[1].c_str(), 1);
      } else if(tokens[3] == "NN"){
	_tmpNN.set(tokens[1].c_str(), 1);
      } else if(tokens[3] == "NNP"){
	_tmpNNP.set(tokens[1].c_str(), 1);
      }
    }
  }
}

bool Lexicon::isTmpNN(const Char * word)
{
  int count;
  if(_tmpNN.get(word, count) == true) return true;
  return false;
}

bool Lexicon::isTmpNNP(const Char * word)
{
  int count;
  if(_tmpNNP.get(word, count) == true) return true;
  return false;
}

bool Lexicon::isTmpIN(const Char * word)
{
  int count;
  if(_tmpIN.get(word, count) == true) return true;
  return false;
}

bool Lexicon::isTmpRB(const Char * word)
{
  int count;
  if(_tmpRB.get(word, count) == true) return true;
  return false;
}

class SavedFeature {
public:
  SavedFeature() { name = NULL; index = -1; freq = -1; }

  const Char * name;
  int index;
  int freq;
};

class DescByFreq {
public:
  bool operator()(const SavedFeature & d1,
		  const SavedFeature & d2) {
      if(d1.freq > d2.freq){
	return true;
      }
      return false;
  };
};

void Lexicon::saveFeatureLexicon(std::ostream & os)
{
  vector<SavedFeature> orderedFeatures;
  // orderedFeatures.resize(_featureIndex - 1);

  for(StringMap<FeatureData *>::const_iterator it = _features.begin();
      it != _features.end(); it ++){
    int index = (* it).second->getValue()->_index;
    int freq = (* it).second->getValue()->_freq;
    // int position = index - 1;
    // RVASSERT(position < _featureIndex, "Invalid feature index!");
    SavedFeature sv;
    sv.name = (* it).second->getKey();
    sv.index = index;
    sv.freq = freq;
    // orderedFeatures[position] = sv;
    orderedFeatures.push_back(sv);
  }

  DescByFreq dbf;
  sort(orderedFeatures.begin(), orderedFeatures.end(), dbf);

  for(int i = 0; i < (int) orderedFeatures.size(); i ++){
    RVASSERT(orderedFeatures[i].name != NULL, "NULL feature name!");
    os << orderedFeatures[i].name << " " 
       << orderedFeatures[i].index << " "
       << orderedFeatures[i].freq << endl;
  }
}

void Lexicon::loadFeatureLexicon(const String & path)
{
  string fname = mergeStrings(path, "/feature.lexicon");
  ifstream fs(fname.c_str());
  RVASSERT(fs, "Can not open feature dictionary stream!");

  char line[MAX_STATS_LINE];
  int count = 1;

  // load features counts
  while(fs.getline(line, MAX_STATS_LINE)){
    vector<String> tokens;
    simpleTokenize(line, tokens, " \t\n\r");
    RVASSERT(tokens.size() == 3, 
	     "Invalid number of tokens in line: " << line);

    int index = strtol(tokens[1].c_str(), NULL, 10);
    RVASSERT(index > 0, 
	     "Invalid feature index " << index << " in line: " << line);

    int freq = strtol(tokens[2].c_str(), NULL, 10);
    RVASSERT(freq > 0, 
	     "Invalid feature frequency " << freq << " in line: " << line);

    FeatureData * fd = new FeatureData(index, freq);
    _features.set(tokens[0].c_str(), fd);
    count ++;
  }

  // the index of the next feature to be added (unlikely to be used)
  _featureIndex = count;

  cerr << "Read " << count - 1 << " features." << endl;
}

int Lexicon::getFeatureIndex(const String & feature,
			     bool create)
{
  FeatureData * fd;
  if(_features.get(feature.c_str(), fd) == true){
    if(create == true) fd->_freq ++;
    return fd->_index;
  }

  if(create == true){
    int index = _featureIndex;
    FeatureData * fd = new FeatureData(index, 1);
    _features.set(feature.c_str(), fd);
    _featureIndex ++;
    return index;
  }

  return -1;
}

void Lexicon::loadArgFrames(const String & path)
{
  string fname = mergeStrings(path, "/verbs.args");
  ifstream fs(fname.c_str());
  RVASSERT(fs, "Can not open verb frame stream!");

  char line[MAX_STATS_LINE];
  int count = 1;

  // load features counts
  while(fs.getline(line, MAX_STATS_LINE)){
    vector<String> tokens;
    simpleTokenize(line, tokens, " \t\n\r");
    RVASSERT(tokens.size() > 1,  
	     "Invalid number of tokens in line: " << line);

    bool * args = new bool[MAX_NUMBERED_ARGS];
    for(int i = 0; i < MAX_NUMBERED_ARGS; i ++) args[i] = false;
    for(size_t i = 1; i < tokens.size(); i ++){
      if(isdigit(tokens[i][0])){
	int arg = strtol(tokens[i].c_str(), NULL, 10);
	RVASSERT(arg >= 0 && arg < MAX_NUMBERED_ARGS,
		 "Invalid arg number in frame: " << line);
	args[arg] = true;
      }
    }

    _frames.set(tokens[0].c_str(), args);
    count ++;
  }

  cerr << "Read " << count - 1 << " verb frames." << endl;  
}

bool * Lexicon::getFrame(const String & lemma)
{
  bool * frame = NULL;
  if(_frames.get(lemma.c_str(), frame) == true) return frame;
  return NULL;
}

void Lexicon::addArgValidLabel(const String & arg,
			       const String & label)
{
  StringMap<int> * labels = NULL;
  if(_validLabels.get(arg.c_str(), labels) == false){
    labels = new StringMap<int>();
    _validLabels.set(arg.c_str(), labels);
  }
  labels->set(label.c_str(), 1);
}

void Lexicon::addVerbParticle(const String & verb,
			      const String & particle,
			      int count)
{
  vector<VerbParticle> * particles = NULL;
  if(_verbParticles.get(verb.c_str(), particles) == false){
    particles = new vector<VerbParticle>();
    _verbParticles.set(verb.c_str(), particles);
  }
  
  for(size_t i = 0; i < particles->size(); i ++){
    if((* particles)[i]._word == particle){
      (* particles)[i]._count += count;
      return;
    }
  }

  particles->push_back(VerbParticle(particle, count));
}

bool Lexicon::isVerbParticle(const String & verb,
			     const String & particle)
{
  vector<VerbParticle> * particles = NULL;
  if(_verbParticles.get(verb.c_str(), particles) == false) return false;
  for(size_t i = 0; i < particles->size(); i ++)
    if((* particles)[i]._word == particle) return true;
  return false;
}

void Lexicon::addIntervalChecks(vector<String> & feats, 
				const String & prefix,
				int value, 
				int min, 
				int max,
				int increment)
{
  for(int i = min; i <= max; i += increment){
    bool ge = (value > i);
    if(ge == true){
      ostringstream os;
      os << prefix << "ge" << i << "." << ge;
      feats.push_back(os.str());
    }
  }
}

void Lexicon::addGeneralizedValues(vector<String> & feats, 
				   const String & prefix,
				   int value, 
				   int min, 
				   int max)
{
  // boolean flags for great-or-equal
  for(int i = min; i <= max; i ++){
    bool ge = (value >= i);
    if(ge == true){
      ostringstream os;
      os << prefix << "ge" << i << "." << ge;
      feats.push_back(os.str());
    }
  }

  // boolean flags for less-or-equal
  for(int i = max; i >= min; i --){
    bool le = (value <= i);
    if(le == true){
      ostringstream os;
      os << prefix << "le" << i << "." << le;
      feats.push_back(os.str());
    }
  }
}

void Lexicon::addThresholdValue(vector<String> & feats, 
				const String & prefix,
				int value,
				int threshold)
{
  if(value >= threshold){
    ostringstream os;
    os << prefix << "." << threshold;
    feats.push_back(os.str());
  } else {
    ostringstream os;
    os << prefix << "." << value;
    feats.push_back(os.str());
  }
}

void Lexicon::initialize(const String & modelPath, 
			 bool testing)
{
  cerr << "Loading stats..." << endl;
  Lexicon::loadCounts(modelPath);
  Lexicon::loadTmps(modelPath);
  
  if(testing){
    cerr << "Loading feature dictionary..." << endl;
    Lexicon::loadFeatureLexicon(modelPath);

    cerr << "Loading argument frames..." << endl;
    Lexicon::loadArgFrames(modelPath);
  }
}
