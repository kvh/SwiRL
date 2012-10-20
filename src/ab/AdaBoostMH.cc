
/*****************************************************************/
/*                                                               */
/*  Class AdaBoostMH                                             */
/*                                                               */
/*****************************************************************/
 
#include "AdaBoostMH.h"
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <cstring>


/*------------------------------------------------------------------------------*\
 *      Class parameters                                                        *
\*------------------------------------------------------------------------------*/

double AdaBoostMH::epsilon = -1.0;
int AdaBoostMH::verbose = 1;
bool AdaBoostMH::option_initialize_weights = true;

void AdaBoostMH::set_epsilon(double eps) {
  epsilon = eps;
}

void AdaBoostMH::set_verbose(int level) {
  verbose = level;
}

void AdaBoostMH::set_initialize_weights(bool b) {
  option_initialize_weights = b;
}

/*------------------------------------------------------------------------------*\
 *      Constructors i Destructors                                              *
\*------------------------------------------------------------------------------*/

AdaBoostMH::AdaBoostMH(int nl) {
  nrules = 0;
  first = NULL;
  last  = NULL;
  out   = NULL;
  pcl_pointer = NULL;
  nlabels = nl;
  mlABTree::set_nlabels(nl);
  
}

AdaBoostMH::~AdaBoostMH() {
  wr_holder *aux;
  while (first!=NULL) {
    aux = first;
    first = first->next;
    delete aux->rule;
    delete aux;
  }
}

/*------------------------------------------------------------------------------*\
 *      Consulta i classificacio                                                *
\*------------------------------------------------------------------------------*/

int AdaBoostMH::n_rules() {
  return nrules;
}


void AdaBoostMH::classify(fvinput *i, double *pred) {
  int l;
  for (l=0; l<nlabels; l++) {
    pred[l] = 0.0;
  }
  wr_holder *wr;
  for (wr=first; wr!=NULL; wr=wr->next) {
    wr->rule->classify(i, pred);
  }
}

void AdaBoostMH::pcl_ini_pointer() {
  pcl_pointer = first;
}

int  AdaBoostMH::pcl_advance_pointer(int steps) {
  if (steps<0) {
    cerr << "AdaBoostMH->pcl_advance_pointer: steps is negative (" << steps << ") !\n";
    exit(-1);
  }
  int i = 0;
  while (i<steps && pcl_pointer!=NULL) {
    pcl_pointer = pcl_pointer->next;
    i++;
  }
  return i;
}

void AdaBoostMH::pcl_classify(fvinput *i, double *pred, int nrules) {
  wr_holder *wr = pcl_pointer;
  while (nrules>0 && wr!=NULL) {
    wr->rule->classify(i, pred);
    wr = wr->next;
    nrules--;
  }
}

/*------------------------------------------------------------------------------*\
 *      Learning                                                                *
\*------------------------------------------------------------------------------*/


void AdaBoostMH::learn(mlDataset *ds, int nrounds, int maxdepth) {
  SC.n_rounds = nrounds;
  SC.max_depth = maxdepth;

  nlabels = ds->nlabels();
  mlABTree::set_nlabels(nlabels);

  mlABTree::set_verbose(verbose);
  if (epsilon == -1.0) {
    mlABTree::set_epsilon(1.0 / (ds->size()*nlabels));
  }
  else {
    mlABTree::set_epsilon(epsilon);
  }
  
  if (option_initialize_weights) {
    initialize_weights(ds);
  }

  int T = 0;
  double Z;
  mlABTree *wr;
  while (!stopping_criterion(T)) {
    if (verbose) {
      cout << "AdaBoostMH: Round " << T << "\n";
    }
    wr = mlABTree::learn(ds, &Z, SC.max_depth);
    if (verbose) {
      cout << "AdaBoostMH: Adding WeakRule.\n";
    }
    add_weak_rule(wr);

    if (verbose) {
      cout << "AdaBoostMH: Updating Weights ... ";
      flush(cout);
    }
    update_weights(wr, Z, ds);
    
    if (verbose > 1) {
      double sw = 0.0;
      mlDataset::iterator ex;     
      int l;
      for(ex=ds->begin(); ex!=ds->end(); ++ex) {
	for (l=0; l<nlabels; l++) {
	  sw += ex->weight(l);
	}
      }
      cout << sw;
    }
    if (verbose) {
      cout << "\n";
    }

    delete wr;
    T++;
  }
}

int AdaBoostMH::stopping_criterion(int nrounds) {
  return (SC.n_rounds <= nrounds);
}

void AdaBoostMH::initialize_weights(mlDataset *ds) {
  mlDataset::iterator ex;     
  double w = 1.0/ (ds->size()*ds->nlabels());
  int l;
  for(ex=ds->begin(); ex!=ds->end(); ++ex) {
    for (l=0; l<nlabels; l++) {
      ex->set_weight(l,w);
    }
  }
}

void AdaBoostMH::update_weights(mlABTree *wr, double Z, mlDataset *ds) {
  double w, margin;
  mlDataset::iterator ex;     
  double out[nlabels];
  int l;

  for(ex=ds->begin(); ex!=ds->end(); ++ex) {
    for (l=0; l<nlabels; l++) {
      out[l] = 0.0;
    }
    wr->classify(&(*ex), out);
    for (l=0; l<nlabels; l++) {
      w = ex->weight(l);
      margin = ex->sign(l) * out[l];
      ex->set_weight(l, (w * exp(-margin)) / Z);
    }
  }
}

void AdaBoostMH::add_weak_rule(mlABTree *wr) {
  if (verbose) {
    wr->print("");
  }

  if (out!=NULL && out->is_open()) {
    *out << "---\n";
    wr->write_to_stream(*out);
  }
}

/*------------------------------------------------------------------------------*\
 *      Operacions I/O                                                          *
\*------------------------------------------------------------------------------*/

void AdaBoostMH::set_output(ofstream *os) {
  out = os;
}

void AdaBoostMH::read_from_file(char* file)
{
  ifstream in(file);
  read_from_stream(in);
}


void AdaBoostMH::read_from_stream(ifstream &in) {
  string token;
  mlABTree *wr;
  wr_holder *wrh;

  if (!in.eof()) {
    in >> token;
  }
  while (!in.eof() && !token.empty()) {
    wr = mlABTree::read_from_stream(in);
    wrh = new wr_holder();
    wrh->rule = wr;
    wrh->next = NULL;
    if (first == NULL) {
      first = wrh;
      last = wrh;
    }
    else {
      last->next = wrh;
      last = wrh;
    }
    nrules++;
    if (! in.eof()) {
      in >> token;
    }
  }
}



/*****************************************************************/
/*                                                               */
/*  Class mlABTree                                               */
/*                                                               */
/*****************************************************************/



double mlABTree::epsilon = 0.0;
int mlABTree::max_depth = 0;
int * mlABTree::used_features = NULL;
int mlABTree::verbose = 1;
int mlABTree::nlabels = 1;


/*------------------------------------------------------------------------------*\
 *      Constructors i Destructors                                              *
\*------------------------------------------------------------------------------*/


mlABTree::mlABTree(double *p0) {
  feature = 0;
  predictions = new double[nlabels];
  int l; 
  for (l=0; l<nlabels; l++) {
    predictions[l] = p0[l];
  }
}

mlABTree::mlABTree(int f0, mlABTree *wrFalse, mlABTree *wrTrue) {
  feature = f0;
  predictions = NULL;
  sons = new mlABTreePtr[2];
  sons[0] = wrFalse;
  sons[1] = wrTrue;
}

mlABTree::~mlABTree() {
  if (feature!=0) {
    delete sons[0];
    delete sons[1];
    delete [] sons;
  }
  else {
    delete [] predictions;
  }
}

/*------------------------------------------------------------------------------*\
 *      Class variables                                                         *
\*------------------------------------------------------------------------------*/

void mlABTree::set_nlabels(int nl) {
  mlABTree::nlabels = nl;
}

void mlABTree::set_epsilon(double eps) {
  mlABTree::epsilon = eps;
}

void mlABTree::set_verbose(int v) {
  mlABTree::verbose = v;
}

/*------------------------------------------------------------------------------*\
 *      Classificació                                                           *
\*------------------------------------------------------------------------------*/

void mlABTree::classify(fvinput *i, double pred[]) {
  if (feature == 0) {
    int l;
    for (l=0; l<nlabels; l++) {
      pred[l] += predictions[l];
    }
  }
  else {
    if (i->feature_value(feature)) {
      sons[1]->classify(i, pred);
    }
    else {
      sons[0]->classify(i, pred);
    }
  }
}


/*------------------------------------------------------------------------------*\
 *      Learning                                                                *
\*------------------------------------------------------------------------------*/


mlABTree* mlABTree::learn(mlDataset *ds, double *Z, int max_depth0) {
  if (max_depth0 >= 0) {
    max_depth = max_depth0;
  }
  else {
    max_depth = int( fabs(float(rand())/float(RAND_MAX+1)) *(-max_depth0+1));
    if (mlABTree::verbose) {
      cout << " mlABTree: Random depth: " << max_depth << "\n";
    }
  }
  mlABTree::used_features = new int[ds->dimension() + 1];
  int i;
  for (i=0;i<=ds->dimension();i++) {
    mlABTree::used_features[i] = 0;
  }
  if (mlABTree::verbose) {
    cout << " mlABTree: Learning Weak Rule; ";
    cout << "DataSet: " << ds->size() << " examples;";
    ds->print_sizes(cout);
    cout << "\n";
  }
  mlABTree *wr = mlABTree::learn_0(ds, Z, 0);
  delete [] mlABTree::used_features;
  return wr;
}


// --------------------


mlABTree* mlABTree::learn_0(mlDataset *ds, double *Z, int depth) {
  int l, c;
  if (mlABTree::stopping_criterion(ds, depth)) {
    double W[2*nlabels];
    for (l=0;l<nlabels;l++) {
      W[2*l]   = 0.0;
      W[2*l+1] = 0.0;
    }
    mlDataset::iterator ex;   
    for(ex=ds->begin(); ex!=ds->end(); ++ex) {
      for (l=0;l<nlabels;l++) {
	c =  (ex->belongs_to(l)) ? 1 : 0;
	W[2*l+c] += ex->weight(l);
      }
    }
    *Z = mlABTree::Zcalculus(W, 1);
    if (mlABTree::verbose>1) {
      cout << " mlABTree[" << depth << "]: new leaf (stopping criterion); Z=" << *Z << ";\n";
    }
    double pred[nlabels];
    mlABTree::Cprediction(0, W, pred);
    return new mlABTree(pred);;
  }
  else {
    double Wfc[4*nlabels];
    int bestf = mlABTree::best_feature(ds, Wfc);
    if (mlABTree::verbose>2) {
      cout << " mlABTree[" << depth << "]: best feature=" << bestf << ";\n";
    }
    if (bestf == 0) {
      *Z = mlABTree::Zcalculus(Wfc, 1);
      if (mlABTree::verbose) {
	cout << " mlABTree[" << depth << "]: new leaf (no more features); Z=" << *Z << ";\n";
      }
      double pred[nlabels];
      mlABTree::Cprediction(0, Wfc, pred);
      return new mlABTree(pred);
    }
    else if (depth == mlABTree::max_depth) {
      double pred[nlabels];
      mlABTree::Cprediction(0, Wfc, pred);
      mlABTree *wr0 = new mlABTree(pred);
      mlABTree::Cprediction(1, Wfc, pred);
      mlABTree *wr1 = new mlABTree(pred);
      *Z = mlABTree::Zcalculus(Wfc, 2);
      if (mlABTree::verbose>2) {
	cout << " mlABTree[" << depth << "]: new leaves (max depth); Z=" << *Z << ";\n";
      }
      return new mlABTree(bestf, wr0, wr1);
    }
    else {
      mlDataset *ds0 = new mlDataset(nlabels);
      mlDataset *ds1 = new mlDataset(nlabels);

      // in the first split (depth==0) the nodes of the dataset are newly created; 
      //  subsequent splits consist of a reindexation of the dataset nodes
      ds->split(bestf, ds0, ds1, depth==0);

      if (mlABTree::verbose>1) {
	cout << " mlABTree[" << depth << "]: new split for feature " << bestf << ": ";
 	cout << "set 0: "; ds0->print_sizes(cout);
	cout << "; set 1: "; ds0->print_sizes(cout); cout << "\n";
      }

      mlABTree::used_features[bestf] = 1;
      double Z0, Z1;
      mlABTree *wr0 = mlABTree::learn_0(ds0, &Z0, depth+1);
      mlABTree *wr1 = mlABTree::learn_0(ds1, &Z1, depth+1);
      mlABTree::used_features[bestf] = 0;

      delete ds0;
      delete ds1;
      *Z = Z0+Z1;      
      return new mlABTree(bestf, wr0, wr1);
    }
  }
}

// --------------------

int mlABTree::stopping_criterion(mlDataset *ds, int depth) {
  int l;
  for (l=0; l<nlabels; l++) {
    if ((ds->positive_size(l)!=0) && (ds->negative_size(l)!=0)) {
      return 0;
    }
  }
  return 1; 
}

// --------------------
typedef double * double_ptr;

int mlABTree::best_feature(mlDataset *ds, double *Wflc) {

  int sizef = ds->dimension() + 1;
  double **feat = new double_ptr[sizef];
  int i, l;

  for(i=0; i<sizef; i++) {
    feat[i] = NULL;
  }

  // omplo l'index
  double wf[nlabels][2];
  for (l=0; l<nlabels; l++) {
    wf[l][0] = 0.0;
    wf[l][1] = 0.0;
  }

  mlDataset::iterator ex;   
  fvinput::const_iterator f;;

  double w[nlabels];
  int c[nlabels];
  for(ex=ds->begin(); ex!=ds->end(); ++ex) {
    for (l=0;l<nlabels;l++) {
      w[l] = ex->weight(l);
      c[l] = (ex->belongs_to(l)) ? 1 : 0;
      wf[l][c[l]] += w[l];
    }
    for(f=ex->begin(); f!=ex->end(); ++f) {
      i = f.label();
      if (!mlABTree::used_features[i]) {
	if (feat[i]==NULL) {
	  feat[i] = new double[2*nlabels];
	  for (l=0;l<nlabels;l++) {
	    feat[i][2*l] = 0.0;
	    feat[i][2*l+1] = 0.0;
	  }
	}
	for (l=0;l<nlabels;l++) {
	  feat[i][2*l+c[l]] += w[l];
	}
      }      
    }
  }


  //recorregut per a trobar el best feature
  int bestf = 0;
  double Z, Zbf=1;
  for(i=0; i<sizef; i++) {
    if (feat[i]!=NULL) {
      for (l=0;l<nlabels;l++) {
	Wflc[2*nlabels + 2*l]    = feat[i][2*l];
	Wflc[2*nlabels + 2*l +1] = feat[i][2*l+1];
	Wflc[2*l]    = wf[l][0] - Wflc[2*nlabels + 2*l];
	Wflc[2*l +1] = wf[l][1] - Wflc[2*nlabels + 2*l +1];
      }
      Z = mlABTree::Zcalculus(Wflc, 2);
      if (!bestf || (Z < Zbf)) {
	bestf = i;
	Zbf = Z;
      }
    }
  }


  if (bestf != 0) {
    for (l=0;l<nlabels;l++) {
      Wflc[2*nlabels + 2*l]    = feat[bestf][2*l];
      Wflc[2*nlabels + 2*l +1] = feat[bestf][2*l+1];
      Wflc[2*l]    = wf[l][0] - Wflc[2*nlabels + 2*l];
      Wflc[2*l +1] = wf[l][1] - Wflc[2*nlabels + 2*l +1];
    }
  }
  else {
    for (l=0;l<nlabels;l++) {
      Wflc[2*nlabels + 2*l]    = 0.0;
      Wflc[2*nlabels + 2*l +1] = 0.0;
      Wflc[2*l]    = wf[l][0];
      Wflc[2*l +1] = wf[l][1];
    }
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


double mlABTree::Zcalculus(double *W, int ndim) {
  int i, l, offset;
  double Z = 0.0;
  for (l=0; l<nlabels; l++) {
    for (i=0; i<ndim; i++) {
      offset = i * 2 *nlabels;
      Z += W[offset+2*l+1] * sqrt((W[offset+2*l]   + mlABTree::epsilon)/(W[offset+2*l+1]+mlABTree::epsilon));
      Z += W[offset+2*l]   * sqrt((W[offset+2*l+1] + mlABTree::epsilon)/(W[offset+2*l ] +mlABTree::epsilon));
    }
  }
  return Z;
}

void mlABTree::Cprediction(int v, double *W, double result[]) {
  int l;
  int offset = v * 2 * nlabels;
  for (l=0; l<nlabels; l++) {
    result [l] = 0.5 * log((W[offset+2*l+1] + mlABTree::epsilon) / (W[offset+2*l] + mlABTree::epsilon));
  }
}


/*------------------------------------------------------------------------------*\
 *      Operacions I/O                                                          *
\*------------------------------------------------------------------------------*/

void mlABTree::print(char *begin) {
  if (!feature) {
    int l;
    for (l=0;l<nlabels;l++) {
      cout << l << ":" << predictions[l] << " ";
    }
    cout << "\n";
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
    cout << begin << &nubegin[lb] << " -+ ";
    nubegin[n-5] = ' ';
    sons[1]->print(nubegin);
    delete []nubegin;
  }
}

void mlABTree::write_to_stream(ofstream &os) {
  if (!feature) {
    os << "-";
    int l;
    for (l=0; l<nlabels; l++) {
      os << " " << predictions[l];
    }
    os << "\n";
  }
  else {
    os << "+ " << feature << "\n";
    sons[0]->write_to_stream(os);
    sons[1]->write_to_stream(os);
  }
}


mlABTree *mlABTree::read_from_stream(istream &is) {
  char c;
  is >> c;
  if (c == '-') {
    double p[nlabels];
    int l;
    for (l=0;l<nlabels;l++) {
      is >> p[l];
    }
    return new mlABTree(p);
  }
  else {
    int f;
    is >> f;
    mlABTree *wr0 = mlABTree::read_from_stream(is);
    mlABTree *wr1 = mlABTree::read_from_stream(is);
    return new mlABTree(f, wr0, wr1);
  }
}










