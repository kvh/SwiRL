/*****************************************************************/
/*                                                               */
/*  Class bAdaBoost                                              */
/*                                                               */
/*****************************************************************/

#ifndef __bAdaBoost__
#define __bAdaBoost__

#include "bABTree.h"
#include "dataset.h"

struct wr_holder {
  bABTree *rule;
  wr_holder  *next;
};

class bAdaBoost {
private:
  // class parameters
  static double epsilon;
  static int    verbose;
  static bool   option_initialize_weights;

  // weakrules linked list
  wr_holder  *first;
  wr_holder  *last;
  wr_holder  *pcl_pointer; // partial classification pointer
  int         nrules;
  int         active; // number of active rules to be used in classification (<=0, all rules)

  // utility gains (NULL when no utility; otherwise array of lenght 2)
  double*     utility;
  
  // output 
  ofstream    *out;

  // stopping criterion
  struct {
    int  n_rounds;
    int  max_depth;
  } SC;

  // auxiliar learning functions
  int stopping_criterion(int nrounds);
  void initialize_weights(bDataset *ds);
  void update_weights(bABTree *wr, double Z, bDataset *ds);
  void add_weak_rule(bABTree *wr);

  // copy constructor forbidden
  bAdaBoost(const bAdaBoost &old_bab); 

public:
  // constructors, destructor and access methods
  bAdaBoost();
  ~bAdaBoost();
  int n_rules();

  // sets the number of rules to be used in classification
  void set_active_rules(int a); 

  // classification methods
  double classify(fvinput *i);

  // partial classification
  void pcl_ini_pointer();
  int  pcl_advance_pointer(int steps);
  double pcl_classify(fvinput *i, double pred, int nrules);

  // learning methods
  void learn(bDataset *ds, int nrounds, int maxdepth);

  void set_utilities(double upos, double uneg);

  // I/O methods
  void set_output(ofstream *os);
  void read_from_stream(ifstream &in);
  void read_from_file(char* file);

  static void set_verbose(int level);
  static void set_epsilon(double eps);
  static void set_initialize_weights(bool b);
};

#endif 




