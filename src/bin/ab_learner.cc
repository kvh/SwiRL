/********************************************************************************/
/*                                                                              */
/*  Author    : Xavier Carreras Pérez (carreras@lsi.upc.es)                     */
/*  Date      : February 2003                                                   */
/*                                                                              */
/*  ablearner : binary real-valued AdaBoost learner                             */
/*                                                                              */
/********************************************************************************/

#include "bAdaBoost.h"
#include "dataset.h"

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <unistd.h>
#include <cstdio>
#include <cmath>

/* Parametres de l'aprenentatge */

// data set file
string dsfile;  
// label of positive examples in data file
int positive_label = 1; 
// file for second dataset (optional);
string ds2file = "";   
// weigths for datasets; -1 value indicates "not used"
double weight_ds1 = -1.0, weight_ds2 = -1.0;  
// number of rounds
int T = 1000;           
// depth of weak rules
int D = 3;            
// epsilon for adaboost
double EPS = -1.0;    
// output file for the learned model
string modelfile = "";
// verbosity level
int verbose = 1;      
// utility for cost learning; indicator and weights
int utility = 0;      
double ut_pos, ut_neg;

void help();
void get_options(int argc, char** argv);

int main(int argc, char *argv[]) {

  get_options(argc, argv);
  
  bAdaBoost::set_verbose(verbose);
  bAdaBoost::set_epsilon(EPS);

  // data set load
  bDataset::set_positive_label(positive_label);

  bDataset *ds = new bDataset;
  ifstream is_ds(dsfile.c_str());
  ds->read_stream(is_ds);
  if (verbose) {
    cout << ds->size() << " examples read.\n";
  }

  // second dataset (optional)
  if (ds2file != "") {
    bDataset *ds2 = new bDataset;
    ifstream is_ds2(ds2file.c_str());
    ds2->read_stream(is_ds);
    if (verbose) {
      cout << ds2->size() << " examples read in second dataset.\n";
    }


    // weighting of datasets
    if (weight_ds1 != -1.0) {
      bAdaBoost::set_initialize_weights(false);
      bDataset::iterator ex;
      double w = weight_ds1/ds->size();
      for (ex = ds->begin(); ex!=ds->end(); ++ex) {
	ex->set_weight(w);
      }
      w = weight_ds2/ds2->size();
      for (ex = ds2->begin(); ex!=ds2->end(); ++ex) {
	ex->set_weight(w);
      }
    }
    ds->merge_dataset(ds2);
    if (verbose) {
      cout << ds->size() << " examples read in merged dataset!\n";
    }    
  }
  
  bAdaBoost *ab = new bAdaBoost;
  ofstream *out = NULL;
  if (modelfile != "") {
    out = new ofstream(modelfile.c_str());
    ab->set_output(out);
  }
  if (utility) {
    ab->set_utilities(ut_pos, ut_neg);
  }
  ab->learn(ds, T, D);
  if (out != NULL) {
    delete out;
  }
}


void get_options(int argc, char** argv) {
  int c;
  extern char *optarg;
  while ((c = getopt(argc, argv, "c:d:w:l:T:D:E:m:v:u:")) != EOF)
    switch (c) {
    case 'c':
      dsfile = optarg;
      break;
    case 'd':
      ds2file = optarg;
      break;
    case 'w':
      if (sscanf(optarg, "%lf:%lf", &weight_ds1, &weight_ds2) == 2) {
	if ((weight_ds1<0.0) || (weight_ds2<0.0) || (fabs(weight_ds1 + weight_ds2 - 1.0) > 0.000001)) {
	  cerr << "ablearner: bad numbers for -w weight values" << weight_ds1 << " " << weight_ds2 << "!\n";
	  help();
	  exit(-1);
	}
      }
      else {
	cerr << "ablearner: bad format for -w weight values!\n";
	help();
	exit(-1);
      }
      break;
    case 'l':
      positive_label = atoi(optarg);
      break;
    case 'D':
      D = atoi(optarg);
      break;
    case 'T':
      T = atoi(optarg);
      break;
    case 'E':
      EPS = atof(optarg);
      break;
    case 'm':
      modelfile = optarg;
      break;
    case 'v':
      verbose = atoi(optarg);
      break;
    case 'u':
      if (sscanf(optarg, "%lf:%lf", &ut_pos, &ut_neg) == 2) {
	utility = 1;
      }
      else {
	cerr << "ablearner: bad format for utility values!\n";
	help();
	exit(-1);
      }
      break;
    case '?':
      help();
      exit(-1);
    }
  if (dsfile=="") {
    cout << "Unspecified data set!\n";
    help();
    exit(-1);
  }
  if (T<=0) {
    cout << "Unspecified number of rounds!\n";
    help();
    exit(-1);
  }
}

void help() {
  cout << "ablearner: binary AdaBoost learner\n";
  cout << "Usage:\n";
  cout << "    -c <file>           DataSet file.\n";  
  cout << "    -d <file>           Second DataSet (optional).\n";
  cout << "    -w w1:w2            weight of first and second datasets,\n"; 
  cout << "                         subject to w1 + w2 = 1.0\n"; 
  cout << "                         not compatible with utilities\n";
  cout << "    -l <int>            Label of positive examples in data file.\n";
  cout << "                          Default: +1.\n";
  cout << "    -T <nrounds>        Number of boosting rounds.\n";
  cout << "    -D <depth>          Depth of the weak rules.\n";
  cout << "                          Default: 0 (stumps).\n";
  cout << "                          Negative depth: random depths in [0,depth].\n";
  cout << "    -E <epsilon>        Smoothing epsilon value.\n"; 
  cout << "                          Default: 1/(number of examples).\n";
  cout << "    -m <file>           Output file for the model.\n"; 
  cout << "    -v <level>          Verbosity level:\n";
  cout << "                          0: no verbose;\n";
  cout << "                          1: verbose (default);\n";
  cout << "                          2: very verbose;\n";
  cout << "    -u u+:u-            Utility gains:\n";
  cout << "                          u+: utility for relevant examples.\n";
  cout << "                          u-: utility for non-relevant examples.\n";
  cout << "\n";
}
