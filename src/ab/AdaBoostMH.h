/********************************************************************************/
/*                                                                              */
/*  Author    : Xavier Carreras Perez (carreras@lsi.upc.es)                     */
/*  Date      : February 2003                                                   */
/*                                                                              */
/*  AdaBoostMH.h :  AdaBoostMH, mlABTree                                        */
/*                                                                              */
/*                                                                              */
/*                                                                              */
/********************************************************************************/


/*****************************************************************/
/*                                                               */
/*  Class AdaBoostMH                                             */
/*                                                               */
/*****************************************************************/

#ifndef __AdaBoostMH__
#define __AdaBoostMH__

#include "dataset.h"
#include <fstream>
#include  <iostream>


// defined below
class mlABTree; 

class AdaBoostMH {
private:
  // class parameters
  static double epsilon;
  static int    verbose;
  static bool   option_initialize_weights;


  struct wr_holder {
    mlABTree *rule;
    wr_holder  *next;
  };


  // weakrules linked list
  wr_holder  *first;
  wr_holder  *last;
  wr_holder  *pcl_pointer; // partial classification pointer
  int         nrules;
  int         nlabels;  

  // output 
  ofstream    *out;

  // stopping criterion
  struct {
    int  n_rounds;
    int  max_depth;
  } SC;

  // auxiliar learning functions
  int  stopping_criterion(int nrounds);
  void initialize_weights(mlDataset *ds);
  void update_weights(mlABTree *wr, double Z, mlDataset *ds);
  void add_weak_rule(mlABTree *wr);

  // copy constructor forbidden
  AdaBoostMH(const AdaBoostMH &old_bab); 

public:
  // constructors, destructor and access methods
  AdaBoostMH(int nl);
  ~AdaBoostMH();
  int n_rules();

  // classification methods
  // Important: pred is an array of predictions, one for each label
  //            the function *assigns* its predicion for each label
  void classify(fvinput *i, double *pred);

  // partial classification
  void pcl_ini_pointer();
  int  pcl_advance_pointer(int steps);
  // Important: pred is an array of predictions, one for each label
  //            the function *adds* its predicion for each label
  void pcl_classify(fvinput *i, double *pred, int nrules);

  // learning methods
  void learn(mlDataset *ds, int nrounds, int maxdepth);


  // I/O methods
  void set_output(ofstream *os);
  void read_from_stream(ifstream &in);
  void read_from_file(char* file);

  static void set_verbose(int level);
  static void set_epsilon(double eps);
  static void set_initialize_weights(bool b);
};



/*****************************************************************/
/*                                                               */
/*  Class mlABTree                                               */
/*                                                               */
/*****************************************************************/


class mlABTree {

private:
  // binary tree structure
  int         feature;        // 0 when leaf
  mlABTree  **sons;           // when no leaf 
  double     *predictions;    // when leaf  (array of predicitons, one for each class)

  // learning parameters
  static int       nlabels;
  static double    epsilon;
  static int       max_depth;
  static int      *used_features; 
  static int       verbose;

  // auxiliar learning functions
  static mlABTree* learn_0(mlDataset *ds, double *Z, int depth);
  static int       stopping_criterion(mlDataset *ds, int depth);
  // W is W[2][nlabels][2]
  static int       best_feature(mlDataset *ds, double *W);

  // W is W[ndim][nlabels][2]
  static double    Zcalculus(double *W, int ndim);
  // W is W[v][nlabels][2]; result is result[nlabels][2]
  static void      Cprediction(int v, double *W, double result[]);

  // copy constructor forbidden
  mlABTree(const mlABTree &wr0);

public:
  // class parameters
  static void set_nlabels(int nl);
  static void set_verbose(int level);
  static void set_epsilon(double eps);

  // Constructors and destructor

  //  p0 is an array of predicions, one for each label
  mlABTree(double *p0);
  mlABTree(int f, mlABTree *wrFalse, mlABTree *wrTrue);
  ~mlABTree();

  // Classification
  // Important: pred is an array of predictions, one for each label
  //            the function *adds* its predicion for each label
  void classify(fvinput *i, double *pred);

  //  I/O operations
  void print(char *carry);
  void write_to_stream(ofstream &os);
  static mlABTree* read_from_stream(istream &is);

  // learning
  static mlABTree* learn(mlDataset *ds, double *Z, int max_depth0);
};

typedef mlABTree * mlABTreePtr;

#endif 
