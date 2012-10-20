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
#include <time.h>
#include <sys/resource.h>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <math.h>
#include "Field.h"
#include "GotIter.h"
#include "Wrd.h"
#include "InputTree.h"
#include "Bchart.h"
#include "ECArgs.h"
#include "MeChart.h"
#include "headFinder.h"
#include "Params.h"
#include "AnsHeap.h"
#include "TimeIt.h"
#include "extraMain.h"
#include "Link.h"
#include "ClassRule.h"
#include "ScoreTree.h"
 
/* I need to syncronize three times for each sentence,
one to read it in from a common file, one to write it out
to a common file, and once to store how well I did on
the sentence in running total variable.  One a single
processer, each sentence takes about .7 seconds.
*/
pthread_mutex_t readlock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t writelock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t scorelock = PTHREAD_MUTEX_INITIALIZER;

Params 		    params;
bool warnP = false;
int numThreads=1;
int sentenceCount = 0;
int printCount =0;
double  totGram  = 0.0;
double  totMix  = 0.0;
double  totTri  = 0.0;
int  totWords = 0;
bool histPoints[500];
ParseStats  totPst[500];

extern LeftRightGotIter* globalGi[MAXNUMTHREADS];

/* In order to print out the data in the correct order each
thread has it's own PrintStack which stores the output data
(printStrict) until it is time to print it out in order.
*/
typedef struct printStruct{
  int                sentenceCount;
  int                numDiff;
  vector<InputTree*> trees;
  vector<double>     probs;
} printStruct;

typedef list<printStruct> PrintStack;

/* the arguments to the thread function are stored in this struct */
typedef struct loopArg{
  int      id;
  istream* inpt;
  ostream* outpt;
} loopArg;

/* the function called by each thread is "mainLoop" */
void*
mainLoop(void* arg)
{
  loopArg *loopA = (loopArg*)arg;
  istream* testSStream = &cin;
  int id = loopA->id;
  double log600 = log2(600.0);
  PrintStack printStack;
  for( ;  ; )
    {
      InputTree     correct;  
      InputTree*    cuse;

      if( !cin )break;
      cin >> correct;
      if( !cin )break;

      totWords += correct.length()+1;
      int locCount = sentenceCount++;

      cuse = &correct;
      int len = correct.length();
      if(len > params.maxSentLen) continue;
      //cerr << "Len = " << len << endl;
      list<ECString>  wtList;
      correct.make(wtList);
      vector<ECString> poslist;
      correct.makePosList(poslist);
      SentRep sr( wtList );  // used in precision calc

      ScoreTree sc;
      sc.setEquivInts(poslist);
      MeChart*	chart = new MeChart( sr,id );
      if(ChartBase::guided) chart->setGuide(&correct);
      double tmpCrossEnt = chart->parse( );
      Item* topS = chart->topS();
      if(!topS)
	{
	  cout << locCount << "\t" << 0 << endl;
	  //cerr << correct << endl;
	  delete chart;
	  continue;
	}
       
      // compute the outside probabilities on the items so that we can
      // skip doing detailed computations on the really bad ones 

      chart->set_Alphas();

      Bst& bst = chart->findMapParse();
      if( bst.empty()) error( "mapProbs did not return answer");
      double logP =log2(bst.prob());
      logP -= (sr.length()*log600);

      sc.setEquivInts(poslist);
      Val* val = bst.next(0);
      assert(val);

      string warnString = "";
      if(warnP){
	short pos = 0;
	InputTree*  mapparse = inputTreeFromBsts(val,pos,sr);
	assert(mapparse);
	sc.trips.clear();
	ParseStats pSt;
	sc.recordGold(cuse,pSt);
	sc.precisionRecall(mapparse,pSt);
	float newF = pSt.fMeasure();
	if(newF < 1) warnString = "\t!";
      }

      cout << locCount << "\t"<< logP  << warnString << "\n";
      float bestF = -1;
      int i;
      int numVersions = 0;
      Link diffs(0);
      //cerr << "Need num diff: " << Bchart::Nth << endl;
      delete chart;
    }
  return 0;
}

int
main(int argc, char *argv[])
{
   ECArgs args( argc, argv );
   if(args.isset('W')) warnP = true;;

   /* o = basic, but not debugging, output.
      l = length of sentence to be proceeds 0-40 is default
      n = work on each #'th line.
      d = print out debugging info at level #
      t = report timings (requires o)
   */

   int i;
   ChartBase::guided = true;
   params.init( args );
   Bchart::timeFactor = 3;
   //cerr << "Starting wwBCTest " << Feature::sLM << endl;

   ECString  path( args.arg( 0 ) );
   generalInit(path);
   for(int i = 0 ; i < 500 ; i++) histPoints[i] = false;
   histPoints[0] = true;
   if(Bchart::Nth == 50)
     histPoints[1] = histPoints[9] = histPoints[24] = histPoints[49] = true;
   TimeIt timeIt;

   for(i = 0 ; i < MAXNUMTHREADS ; i++) globalGi[i] =NULL;

   pthread_t thread[MAXNUMTHREADS];
   loopArg lA[MAXNUMTHREADS];
   for(i = 0 ; i < numThreads  ; i++){
     lA[i].id = i;
     pthread_create(&thread[i],0,mainLoop, (void*)&lA[i]);
   }
  for(i=0; i<numThreads; i++){
    pthread_join(thread[i],0);
  }

   pthread_exit(0);
   cout.flush();
   return 0;
}


