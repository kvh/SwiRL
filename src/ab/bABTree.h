/********************************************************************************/
/*                                                                              */
/*  Author    : Xavier Carreras Pérez (carreras@lsi.upc.es)                     */
/*  Date      : February 2003                                                   */
/*                                                                              */
/*  bABTree.h :                                                                 */
/*                                                                              */
/*                                                                              */
/*                                                                              */
/********************************************************************************/

#ifndef __bABTree__
#define __bABTree__


#include "dataset.h"

#include <fstream>
#include  <iostream>

class bABTree {

private:
  // binary tree structure
  int      feature;        // 0 when leaf
  bABTree  **sons;         // when no leaf 
  double   prediction;     // when leaf

  // learning parameters
  static double epsilon;
  static int    max_depth;
  static int   *used_features; 
  static int    verbose;

  // auxiliar learning functions
  static bABTree* learn_0(bDataset *ds, double *Z, int depth);
  static int stopping_criterion(bDataset *ds, int depth);
  static int best_feature(bDataset *ds, double W[2][2]);
  static double Zcalculus(double W[][2], int ndim);
  static double Cprediction(int v, double W[][2]);

  // copy constructor forbidden
  bABTree(const bABTree &wr0);

public:
  // class parameters
  static void set_verbose(int level);
  static void set_epsilon(double eps);

  // Constructors and destructor
  bABTree(double p0);
  bABTree(int f, bABTree *wrFalse, bABTree *wrTrue);
  ~bABTree();

  // Classification
  double classify(fvinput *i);

  //  I/O operations
  void print(char *carry);
  void write_to_stream(ofstream &os);
  static bABTree* read_from_stream(istream &is);

  // learning
  static bABTree* learn(bDataset *ds, double *Z, int max_depth0);
};

typedef bABTree * bABTreePtr;

#endif
