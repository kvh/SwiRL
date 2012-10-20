
/*****************************************************************/
/*                                                               */
/*  Class bAdaBoost                                              */
/*                                                               */
/*****************************************************************/
 
#include "bAdaBoost.h"
#include <iostream>
#include <cmath>
#include <cstdlib>

/*------------------------------------------------------------------------------*\
 *      Class parameters                                                        *
\*------------------------------------------------------------------------------*/

double bAdaBoost::epsilon = -1.0;
int bAdaBoost::verbose = 1;
bool bAdaBoost::option_initialize_weights = true;

void bAdaBoost::set_epsilon(double eps) {
  epsilon = eps;
}

void bAdaBoost::set_verbose(int level) {
  verbose = level;
}

void bAdaBoost::set_initialize_weights(bool b) {
  option_initialize_weights = b;
}

/*------------------------------------------------------------------------------*\
 *      Constructors i Destructors                                              *
\*------------------------------------------------------------------------------*/

bAdaBoost::bAdaBoost() {
  nrules = 0;
  active = -1;
  first = NULL;
  last  = NULL;
  out   = NULL;
  pcl_pointer = NULL;
  utility = NULL;
}

bAdaBoost::~bAdaBoost() {
  wr_holder *aux;
  while (first!=NULL) {
    aux = first;
    first = first->next;
    delete aux->rule;
    delete aux;
  }
  if (utility != NULL) {
    delete[] utility;
  }
}

/*------------------------------------------------------------------------------*\
 *      Consulta i classificacio                                                *
\*------------------------------------------------------------------------------*/

int bAdaBoost::n_rules() {
  return nrules;
}

void bAdaBoost::set_active_rules (int a) {
  active = a; 
}

double bAdaBoost::classify(fvinput *i) {
  double result = 0.0;
  wr_holder *wr;

  if (active>0 && active<nrules) {
    int n = 0; 
    wr = first; 
    while (n<active && wr!=NULL) {
      result += wr->rule->classify(i);
      n++;
      wr = wr->next; 
    }
  }
  else {
    for (wr=first; wr!=NULL; wr=wr->next) {
      result += wr->rule->classify(i);
    }
  }
  return result;
}

void bAdaBoost::pcl_ini_pointer() {
  pcl_pointer = first;
}

int  bAdaBoost::pcl_advance_pointer(int steps) {
  if (steps<0) {
    cerr << "bAdaBoost->pcl_advance_pointer: steps is negative (" << steps << ") !\n";
    exit(-1);
  }
  int i = 0;
  while (i<steps && pcl_pointer!=NULL) {
    pcl_pointer = pcl_pointer->next;
    i++;
  }
  return i;
}

double bAdaBoost::pcl_classify(fvinput *i, double pred, int nrules) {
  wr_holder *wr = pcl_pointer;
  while (nrules>0 && wr!=NULL) {
    pred += wr->rule->classify(i);
    wr = wr->next;
    nrules--;
  }
  return pred;
}

/*------------------------------------------------------------------------------*\
 *      Learning                                                                *
\*------------------------------------------------------------------------------*/

void bAdaBoost::set_utilities(double upos, double uneg) {
  if (utility == NULL) {
    utility = new double[2];
  }
  utility[0] = uneg;
  utility[1] = upos;
  if (verbose) {
    cout << "bAdaBoost: Using utilities: " << utility[1] << " for positive examples; ";
    cout << utility[0] << " for negative examples;\n";
  }
}

void bAdaBoost::learn(bDataset *ds, int nrounds, int maxdepth) {
  SC.n_rounds = nrounds;
  SC.max_depth = maxdepth;

  bABTree::set_verbose(verbose);
  if (epsilon == -1.0) {
    bABTree::set_epsilon(1.0 / ds->size());
  }
  else {
    bABTree::set_epsilon(epsilon);
  }
  
  if (option_initialize_weights) {
    initialize_weights(ds);
  }

  int T = 0;
  double Z;
  bABTree *wr;
  while (!stopping_criterion(T)) {
    if (verbose == 1) {
      cout << "." << flush; 
      if ((T+1) % 50 == 0) {
	cout << " (" << T+1 << ") " << flush; 
      }
    }
    else if (verbose>1) {
      cout << "bAdaBoost: Round " << T << "\n";
    }
    wr = bABTree::learn(ds, &Z, SC.max_depth);
    if (verbose>1) {
      cout << "bAdaBoost: Adding WeakRule.\n";
    }
    add_weak_rule(wr);

    if (verbose>1) {
      cout << "bAdaBoost: Updating Weights ... ";
      flush(cout);
    }
    update_weights(wr, Z, ds);
    
    if (verbose > 2) {
      double sw = 0.0;
      bDataset::iterator ex;     
      for(ex=ds->begin(); ex!=ds->end(); ++ex) {
	sw += ex->weight();
      }
      cout << sw;
    }
    if (verbose>1) {
      cout << "\n";
    }

    delete wr;
    T++;
  }

  if (verbose==1) {
    cout <<  "\n";
  }

}

int bAdaBoost::stopping_criterion(int nrounds) {
  return (SC.n_rounds <= nrounds);
}

void bAdaBoost::initialize_weights(bDataset *ds) {
  bDataset::iterator ex;     
  if (utility == NULL) {
    double w = 1.0/ds->size();
    for(ex=ds->begin(); ex!=ds->end(); ++ex) {
      ex->set_weight(w);
    }
  }
  else {
    double wp, wn;
    wp = utility[0]*double(ds->negative_size()) +  utility[1]*double(ds->positive_size());
    wn = utility[0]/wp;
    wp = utility[1]/wp;
      

    for(ex=ds->begin_pos(); ex!=ds->end(); ++ex) {
      ex->set_weight(wp);
    }
    for(ex=ds->begin_neg(); ex!=ds->end(); ++ex) {
      ex->set_weight(wn);
    }
  }    
}

void bAdaBoost::update_weights(bABTree *wr, double Z, bDataset *ds) {
  double w, margin;
  bDataset::iterator ex;     
  for(ex=ds->begin(); ex!=ds->end(); ++ex) {
    w = ex->weight();
    margin = - ex->sign() * wr->classify(&(*ex));
    ex->set_weight((w * exp(margin)) / Z);
  }
}

void bAdaBoost::add_weak_rule(bABTree *wr) {
  if (verbose>1) {
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

void bAdaBoost::set_output(ofstream *os) {
  out = os;
}

void bAdaBoost::read_from_file(char* file)
{
  ifstream in(file);
  read_from_stream(in);
}


void bAdaBoost::read_from_stream(ifstream &in) {
  string token;
  bABTree *wr;
  wr_holder *wrh;
  if (!in.eof()) {
    in >> token;
  }
  while (!in.eof() && !token.empty()) {
    wr = bABTree::read_from_stream(in);
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












