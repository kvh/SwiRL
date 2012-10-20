/*****************************************************************/
/*                                                               */
/*  Class bABTree                                             */
/*                                                               */
/*****************************************************************/

#include  <cstring>
#include  <cmath>
#include  <cstdlib>

#include "bABTree.h"

double bABTree::epsilon = 0.0;
int bABTree::max_depth = 0;
int * bABTree::used_features = NULL;
int bABTree::verbose = 1;


/*------------------------------------------------------------------------------*\
 *      Constructors i Destructors                                              *
\*------------------------------------------------------------------------------*/


bABTree::bABTree(double p0) {
  feature = 0;
  prediction = p0;
}

bABTree::bABTree(int f0, bABTree *wrFalse, bABTree *wrTrue) {
  feature = f0;
  sons = new bABTreePtr[2];
  sons[0] = wrFalse;
  sons[1] = wrTrue;
}

bABTree::~bABTree() {
  if (feature!=0) {
    delete sons[0];
    delete sons[1];
    delete [] sons;
  }
}

/*------------------------------------------------------------------------------*\
 *      Class variables                                                         *
\*------------------------------------------------------------------------------*/

void bABTree::set_epsilon(double eps) {
  bABTree::epsilon = eps;
}

void bABTree::set_verbose(int v) {
  bABTree::verbose = v;
}

/*------------------------------------------------------------------------------*\
 *      Classificació                                                           *
\*------------------------------------------------------------------------------*/

double bABTree::classify(fvinput *i) {
  if (feature == 0) {
    return prediction;
  }
  if (i->feature_value(feature)) {
    return sons[1]->classify(i);
  }
  else {
    return sons[0]->classify(i);
  }
}


/*------------------------------------------------------------------------------*\
 *      Learning                                                                *
\*------------------------------------------------------------------------------*/


bABTree* bABTree::learn(bDataset *ds, double *Z, int max_depth0) {
  if (max_depth0 >= 0) {
    max_depth = max_depth0;
  }
  else {
    max_depth = int( fabs(float(rand())/float(RAND_MAX+1)) *(-max_depth0+1));
    if (bABTree::verbose>1) {
      cout << " bABTree: Random depth: " << max_depth << "\n";
    }
  }
  bABTree::used_features = new int[ds->dimension() + 1];
  int i;
  for (i=0;i<=ds->dimension();i++) {
    bABTree::used_features[i] = 0;
  }
  if (bABTree::verbose>1) {
    cout << " bABTree: Learning Weak Rule; ";
    cout << "DataSet: [-" << ds->negative_size() << ",+" << ds->positive_size() << "];\n"; 
  }
  bABTree *wr = bABTree::learn_0(ds, Z, 0);
  delete [] bABTree::used_features;
  return wr;
}

bABTree* bABTree::learn_0(bDataset *ds, double *Z, int depth) {
  if (bABTree::stopping_criterion(ds, depth)) {
    double W[1][2];
    W[0][0] = ds->weight_negative_examples();
    W[0][1] = ds->weight_positive_examples();
    *Z = bABTree::Zcalculus(W, 1);
    if (bABTree::verbose>2) {
      cout << " bABTree[" << depth << "]: new leaf (stopping criterion); Z=" << *Z << ";\n";
    }
    return new bABTree(bABTree::Cprediction(0, W));
  }
  else {
    double Wfc[2][2];
    int bestf = bABTree::best_feature(ds, Wfc);
    if (bABTree::verbose>3) {
      cout << " bABTree[" << depth << "]: best feature=" << bestf << "; W0- = " << Wfc[0][0] << "; W0+ = " <<  Wfc[0][1];
      cout << "; W1- = " << Wfc[1][0] << "; W1+ = " << Wfc[1][1] << ";\n";
    }
    if (bestf == 0) {
      *Z = bABTree::Zcalculus(Wfc, 1);
      if (bABTree::verbose>1) {
	cout << " bABTree[" << depth << "]: new leaf (no more features); Z=" << *Z << ";\n";
      }
      return new bABTree(bABTree::Cprediction(0, Wfc));
    }
    else if (depth == bABTree::max_depth) {
      bABTree *wr0 = new bABTree(bABTree::Cprediction(0, Wfc));
      bABTree *wr1 = new bABTree(bABTree::Cprediction(1, Wfc));
      *Z = bABTree::Zcalculus(Wfc, 2);
      if (bABTree::verbose>3) {
	cout << " bABTree[" << depth << "]: new leaves (max depth); Z=" << *Z << ";\n";
      }
      return new bABTree(bestf, wr0, wr1);
    }
    else {
      bDataset *ds0 = new bDataset();
      bDataset *ds1 = new bDataset();

      // in the first split (depth==0) the nodes of the dataset are copied; 
      //  subsequent splits consist of a reindexation of the dataset nodes
      ds->split(bestf, ds0, ds1, depth==0);

      if (bABTree::verbose>2) {
	cout << " bABTree[" << depth << "]: new split for feature " << bestf << ": ";
 	cout << "set 0: [-" << ds0->negative_size() << ",+" << ds0->positive_size() << "]; "; 
	cout << "set 1: [-" << ds1->negative_size() << ",+" << ds1->positive_size() << "];\n";
      }

      bABTree::used_features[bestf] = 1;
      double Z0, Z1;
      bABTree *wr0 = bABTree::learn_0(ds0, &Z0, depth+1);
      bABTree *wr1 = bABTree::learn_0(ds1, &Z1, depth+1);
      bABTree::used_features[bestf] = 0;

      delete ds0;
      delete ds1;
      *Z = Z0+Z1;      
      return new bABTree(bestf, wr0, wr1);
    }
  }
}

int bABTree::stopping_criterion(bDataset *ds, int depth) {
   return ((ds->positive_size()==0) || (ds->negative_size()==0));
}

typedef double * double_ptr;

int bABTree::best_feature(bDataset *ds, double Wfc[2][2]) {

  int sizef = ds->dimension() + 1;
  double **feat = new double_ptr[sizef];
  int i, c;
  
  for(i=0; i<sizef; i++) {
    feat[i] = NULL;
  }

  // omplo l'index
  double w=0.0;
  double wf[2] = {0.0, 0.0};

  bDataset::iterator ex;   
  fvinput::const_iterator f;
  
  for(ex=ds->begin(); ex!=ds->end(); ++ex) {
    w = ex->weight();
    c = (ex->positive()) ? 1 : 0;
    wf[c] += w;
    for(f=ex->begin(); f!=ex->end(); ++f) {
      i = f.label();
      if (!bABTree::used_features[i]) {
	if (feat[i]==NULL) {
	  feat[i] = new double[2];
	  feat[i][0] = 0.0;
	  feat[i][1] = 0.0;
	}
	feat[i][c] += w;
      }      
    }
    
  }


  //recorregut per a trobar el best feature
  int bestf = 0;
  double Z, Zbf;
  for(i=0; i<sizef; i++) {
    if (feat[i]!=NULL) {
      Wfc[1][0] = feat[i][0];
      Wfc[1][1] = feat[i][1];
      Wfc[0][0] = wf[0] - Wfc[1][0];
      Wfc[0][1] = wf[1] - Wfc[1][1];
      Z = bABTree::Zcalculus(Wfc, 2);
      if (!bestf || (Z < Zbf)) {
	bestf = i;
	Zbf = Z;
      }
    }
  }

  if (bestf != 0) {
    Wfc[1][0] = feat[bestf][0];
    Wfc[1][1] = feat[bestf][1];
    Wfc[0][0] = wf[0] - Wfc[1][0];
    Wfc[0][1] = wf[1] - Wfc[1][1];
  }
  else {
    Wfc[1][0] = 0.0;
    Wfc[1][1] = 0.0;
    Wfc[0][0] = wf[0];
    Wfc[0][1] = wf[1];    
  }

  // borro l'index;
  for(i=0; i<sizef; i++) {
    if (feat[i]!=NULL) {
      delete [] feat[i];
    }
  }
  delete [] feat;

  return bestf;
}  

double bABTree::Zcalculus(double W[][2], int ndim) {
  int i;
  double Z = 0.0;
  for (i=0; i<ndim; i++) {
    Z += W[i][1] * sqrt((W[i][0]+bABTree::epsilon)/(W[i][1]+bABTree::epsilon));
    Z += W[i][0] * sqrt((W[i][1]+bABTree::epsilon)/(W[i][0]+bABTree::epsilon));
  }
  return Z;
}

inline double bABTree::Cprediction(int v, double W[][2]) {
  return 0.5 * log((W[v][1] + bABTree::epsilon) / (W[v][0] + bABTree::epsilon));
}


/*------------------------------------------------------------------------------*\
 *      Operacions I/O                                                          *
\*------------------------------------------------------------------------------*/

void bABTree::print(char *begin) {
  if (!feature) {
    cout << prediction << "\n";
  }
  else {
    cout << feature << " -- ";
    int lf = int(log(double(feature))/log(10.0)) +1;
    int lb = strlen(begin);
    int n = lb + lf + 5;
    char *nubegin = new char[n];
    strcpy(nubegin, begin);
    int i;
    for(i=lb;i<n;i++) {
      nubegin[i] = ' ';
    }
    nubegin[n-1] = '\0';
    nubegin[n-4] = '|';
    sons[0]->print(nubegin);
    nubegin[n-4] = ' ';
    nubegin[n-5] = '\0';
    cout << begin << &nubegin[lb] << " -- ";
    nubegin[n-5] = ' ';
    sons[1]->print(nubegin);
    delete []nubegin;
  }
}

void bABTree::write_to_stream(ofstream &os) {
  if (!feature) {
    os << "- " << prediction << "\n";
  }
  else {
    os << "+ " << feature << "\n";
    sons[0]->write_to_stream(os);
    sons[1]->write_to_stream(os);
  }
}

bABTree *bABTree::read_from_stream(istream &is) {
  char c;
  is >> c;
  if (c == '-') {
    double p;
    is >> p;
    return new bABTree(p);
  }
  else {
    int f;
    is >> f;
    bABTree *wr0 = bABTree::read_from_stream(is);
    bABTree *wr1 = bABTree::read_from_stream(is);
    return new bABTree(f, wr0, wr1);
  }
}
