/*
 * Copyright 1999, 2005 Brown University, Providence, RI.
 * 
 *                         All Rights Reserved
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose other than its incorporation into a
 * commercial product is hereby granted without fee, provided that the
 * above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Brown University not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.
 * 
 * BROWN UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR ANY
 * PARTICULAR PURPOSE.  IN NO EVENT SHALL BROWN UNIVERSITY BE LIABLE FOR
 * ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <pthread.h>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <math.h>
#include "GotIter.h"
#include "Wrd.h"
#include "InputTree.h"
#include "Bchart.h"
#include "ECArgs.h"
#include "MeChart.h"
#include "extraMain.h"
#include "AnsHeap.h"
#include "UnitRules.h"
#include "Params.h"
#include "TimeIt.h"
#include "ewDciTokStrm.h"
#include "Link.h"
#include "utils.h"
 
//-----------------------
// Definitions
//-----------------------

/* In order to print out the data in the correct order each
thread has it's own PrintStack which stores the output data
(printStrict) until it is time to print it out in order.
*/
typedef struct printStruct{
  int                sentenceCount;
  int                numDiff;
  vector<InputTree*> trees;
  vector<double>     probs;
  string             name;
} printStruct;
typedef list<printStruct> PrintStack;

//-----------------------
// Prototypes
//-----------------------

static void* mainLoop (void* arg);
static void printSkipped( SentRep *srp, MeChart *chart,PrintStack& pstk, printStruct& ps);
static void workOnPrintStack(PrintStack* printStack);

//-----------------------
// Constants
//-----------------------

static const unsigned int SLEEPTIME = 1;
static const int DEFAULT_NTHREAD = 2;
static const double log600 = log2(600.0);

//-----------------------
// Globals
//-----------------------

/* Syncronize two times for each sentence, one to read it in 
from a common file, and one to write it out to a common file. */
static pthread_mutex_t readlock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t writelock = PTHREAD_MUTEX_INITIALIZER;

int sentenceCount=0; // allow extern'ing for error messages
static int printCount=0;
static ewDciTokStrm* tokStream = NULL;
static istream* nontokStream = NULL;
static Params params;

//------------------------------

static void usage(const char *program) 
{
  cerr << "\n*** Usage information for " << program << " ***\n";

  cerr << "\nDefault use: " << program << " DATA/ [input file]\n";
  cerr << "If no input file supplied, stdin is assumed.\n";

  cerr << "\nRun mode:\n";
  cerr << "-M: language modeling flag\n";
  cerr << "-N: n-best parsing\n"; 

  cerr << "\nPerformance/Quality:\n";
  cerr << "-s: small training corpus flag\n";
  cerr << "-t: number of threads [2]\n";
  cerr << "-T: over-parsing level [210]\n";

  cerr << "\nInput:\n";
  cerr << "-C: case-insensitive flag\n";
  cerr << "-K: pre-tokenized data flag\n";
  cerr << "-l: skip sentences exceeding specified length [100]\n";
  cerr << "-L: language selection (En|Ch) [En]\n";
  cerr << "-n: process every Nth sentence only\n";

  cerr << "\nOutput:\n";
  cerr << "-d: print debug info at specified detail level\n";
  cerr << "-P: pretty-print flag\n";
  cerr << "-S: silent failure flag\n";

  //cerr << "-t: report timings\n"; // this was in comment at start of main but appears deprecated

  cerr << "\nSee README file for additional information.\n\n";
}

//------------------------------

int
main(int argc, char *argv[])
{
  if (argc==1) {
    usage(argv[0]);
    return 1;
  }

  ECArgs args( argc, argv );
  params.init( args );
  int numThreads=DEFAULT_NTHREAD;
  if(args.isset('t')) 
    numThreads = atoi(args.value('t').c_str());

  TimeIt timeIt;
  ECString  path( args.arg( 0 ) );
  generalInit(path);

  ECString flnm = "dummy";
  if(args.nargs()==2) flnm = args.arg(1);
  if(Bchart::tokenize)
    {
      tokStream = new ewDciTokStrm(flnm);
      if(args.nargs() ==1) tokStream->useCin = 1;
    }
  if(args.nargs()==2) nontokStream = new ifstream(args.arg(1).c_str());
  else nontokStream = &cin;

  pthread_t thread[MAXNUMTHREADS];
  int id[MAXNUMTHREADS];
  int i;
  for(i = 0 ; i < numThreads  ; i++){
    id[i]=i;
    pthread_create(&thread[i],0,mainLoop, &id[i]);
  }
  for(i=0; i<numThreads; i++){
    pthread_join(thread[i],0);
  }
  pthread_exit(0);
  return 0;
}

//------------------------------

static void*
mainLoop(void* arg)
{
  int *id = reinterpret_cast<int *>(arg);

  PrintStack printStack;
  for( ; ; )
    {
      SentRep* srp = new SentRep(params.maxSentLen);

      pthread_mutex_lock(&readlock);
      if(Bchart::tokenize)
	*tokStream >> *srp;
      else 
	*nontokStream >> *srp;
      int locCount = sentenceCount++;
      pthread_mutex_unlock(&readlock);

      if( !params.field().in(sentenceCount) ) continue;

      printStruct printS;
      printS.name = srp->getName();
      printS.sentenceCount = locCount;
      printS.numDiff = 0;

      int len = srp->length();
      if (len == 0) break;
      if (len > params.maxSentLen) 
	{
	  ECString msg("skipping sentence longer than specified limit of ");
	  msg += intToString(params.maxSentLen);
	  WARN( msg.c_str() );
	  printSkipped(srp,NULL,printStack,printS);
	  continue;
	}

      // handle input containing reserved word Bchart::HEADWORD_S1; could probably do 
      // better (like undo replacement before printing) but this seems sufficient.
      int i;
      for (i = 0; i < len; ++i) 
	{
	  ECString& w = ((*srp)[i]).lexeme();
	  if (w == Bchart::HEADWORD_S1) 
	    {
	      ECString msg = ECString("Replacing reserved token \"") + Bchart::HEADWORD_S1;
	      msg += "\" at index " + intToString(i) + " of input with token \"^^^\"";
	      WARN( msg.c_str() );
	      w = "^^^";
	    }
	}

      MeChart*	chart = new MeChart( *srp,*id );
       
      chart->parse( );

      Item* topS = chart->topS();
      if(!topS)
	{
	  WARN( "Parse failed: !topS" );
	  printSkipped(srp,chart,printStack,printS);
	  delete chart;
	  continue;
	}
      // compute the outside probabilities on the items so that we can
      // skip doing detailed computations on the really bad ones 
      chart->set_Alphas();

      Bst& bst = chart->findMapParse();
      if( bst.empty())
	{
	  WARN( "Parse failed: chart->findMapParse().empty()" );
	  printSkipped(srp,chart,printStack,printS);
	  delete chart;
	  continue;
	}
      if(Feature::isLM)
	{
	  double lgram = log2(bst.sum());
	  lgram -= (len*log600);
	  double pgram = pow(2,lgram);
	  double iptri =chart->triGram();;
	  double ltri = (log2(iptri)-len*log600);
	  double ptri = pow(2.0,ltri);
	  double pcomb = (0.667 * pgram)+(0.333 * ptri);
	  double lmix = log2(pcomb);
	  if(locCount%10==9)cout << lgram << "\t" << ltri << "\t" << lmix << "\n";
	}
      int numVersions = 0;
      Link diffs(0);
      //cerr << "Need num diff: " << Bchart::Nth << endl;
      for(numVersions = 0 ; ; numVersions++)
	{
	  short pos = 0;
	  Val* v = bst.next(numVersions);
	  if(!v) break;
	  double vp = v->prob();
	  if(vp == 0) break;
	  if(isnan(vp)) break;
	  if(isinf(vp)) break;
	  InputTree* mapparse=inputTreeFromBsts(v,pos,*srp);
	  bool isU;
	  int cnt = 0;
	  diffs.is_unique(mapparse, isU,cnt);
	  if(cnt != len)
	    {
	      cerr << "Bad length parse for: " << *srp << endl;
	      cerr << *mapparse << endl;
	      assert(cnt == len);
	    }
	  if(isU)
	    {
	      printS.probs.push_back(v->prob());
	      printS.trees.push_back(mapparse);
	      printS.numDiff++;
	    }
	  else
	    {
	      delete mapparse;
	    }
	  if(printS.numDiff >= Bchart::Nth) break;
	  if(numVersions > 20000) break;
	}
      if( printS.numDiff == 0)
	{
	  WARN( "Parse failed from 0, inf or nan probabililty" );
	  printSkipped(srp,chart,printStack,printS);
	  delete chart;
	  continue;
	}

      /* put the sentence with which we just finished at the end of the printStack*/
      printStack.push_back(printS);
      workOnPrintStack(&printStack);
      delete chart;
      delete srp;
    }
  while(!printStack.empty()){
    sleep(SLEEPTIME);
    workOnPrintStack(&printStack);
  }

  return 0;
}

//------------------------------

static const ECString& getPOS(Wrd& w, MeChart *chart)
{
  list<float>& wpl = chart->wordPlist(&w, w.loc());      
  list<float>::iterator wpli = wpl.begin();
  float max=-1.0;
  int termInt = (int)max;
  for( ; wpli != wpl.end() ; wpli++)
    {
      int term = (int)(*wpli);
      wpli++;
      // p*(pos|w) = argmax(pos){ p(w|pos) * p(pos) } 
      double prob = *wpli * chart->pT(term); 
      if (prob > max) {
	termInt = term;
	max = prob;
      }
    }
  const Term* nxtTerm = Term::fromInt(termInt);
  return nxtTerm->name();
}

//------------------------------
static void makeFlat(SentRep *srp, MeChart *chart, InputTree*& t)
{
  bool allocated=false;
  if (chart == NULL && srp->length() < MAXSENTLEN) 
    {
      chart = new MeChart( *srp,0);
      allocated=true;
    }

  // 05/30/06 ML: use something short for pretend POS tag
  const ECString UNK="NN"; 
  InputTrees dummy1;
  InputTree* st= new InputTree(0,srp->length(),"","S","",dummy1,NULL,NULL);
  InputTrees dummy2;
  dummy2.push_back(st);
  InputTree* s1 =new InputTree(0,srp->length(),"","S1","",dummy2,NULL,NULL);
  st->parentSet()=s1;
  InputTrees its;
  for (int xx = 0; xx < srp->length(); ++xx)
    {
      Wrd& w = (*srp)[xx];
      const ECString& pos = (chart!=NULL) ? getPOS(w,chart) : UNK;
      InputTree* nt= new InputTree(xx, xx+1, w.lexeme(), pos, "",
				   dummy1,st, NULL);
      its.push_back(nt);
    }
  st->subTrees()=its;
  t=s1;
}

//------------------------------

static void
printSkipped(SentRep *srp, MeChart *chart,PrintStack& printStack,printStruct& printS)
{
  // stderr
  if (!Bchart::silent) 
      cerr << *srp << "\n\n";

  // stdout
  // ML 05/04/06: Ensure every input sentence produces an output parse tree,
  // at least in 1-best mode. The default tree is just a flat S.
  if(Feature::isLM)
    {
      double veryLow=-1000;
      cout << veryLow << "\t" << veryLow << "\t" << veryLow << "\n";
    }
  InputTree* dummy;
  makeFlat(srp,chart,dummy);
  printS.probs.push_back(10e-200);
  printS.trees.push_back(dummy);
  printS.numDiff++;
  printStack.push_back(printS);
  workOnPrintStack(&printStack);
}

//------------------------------

static void
workOnPrintStack(PrintStack* printStack)
{
  int i;
  int numPrinted;
  PrintStack::iterator psi = printStack->begin();
  /* now look at each item from the front of the print stack
     to see if it should be printed now */
  pthread_mutex_lock(&writelock);
  for( numPrinted =0; psi != printStack->end(); numPrinted++ )
    {
      printStruct& pstr=(*psi);
      if(pstr.sentenceCount != printCount) break;
      if(Bchart::Nth > 1) {
	ECString index = pstr.name.empty() ? intToString(sentenceCount)
	  : pstr.name;
	cout << pstr.numDiff << "\t" << index <<"\n";
      }
      printCount++;
      for(i = 0 ; i < pstr.numDiff ; i++)
	{
	  InputTree*  mapparse = pstr.trees[i];
	  assert(mapparse);
	  double logP =log2(pstr.probs[i]);
	  logP -= (mapparse->length()*log600);
	  if (Bchart::Nth > 1) 
	    cout << logP << "\n";
	  else if (!pstr.name.empty())	
	    cout << "<" << pstr.name << "> "; 
	      
	  if (Bchart::prettyPrint) 
	    cout << *mapparse << "\n\n";
	  else
	    {
	      mapparse->printproper(cout);
	      cout << "\n";
	    }
	  delete mapparse;

	}
      psi++;
      cout << endl;
    }
  pthread_mutex_unlock(&writelock);
  for(i = 0 ; i < numPrinted ; i++) printStack->pop_front();
}
