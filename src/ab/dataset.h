/********************************************************************************/
/*                                                                              */
/*  Author    : Xavier Carreras Pérez (carreras@lsi.upc.es)                     */
/*  Date      : October 2002                                                    */
/*                                                                              */
/*  dataset.h : defines classes for datasets, including                         */
/*             - bDataset  : binary examples                                    */
/*             - mlDataset : multilabeled examples                              */
/*                                                                              */
/********************************************************************************/


#ifndef __dataset__
#define __dataset__

#include <iostream>
#include "example.h"

using namespace std;


/********************************************************************************/
/*                                                                              */
/*  bDataset:  dataset of binary examples                                       */
/*                                                                              */
/********************************************************************************/

class bDataset 
{
 private:
  
  struct bDatasetNode {
    bExample* ex;          // pointer to the example
    bDatasetNode* next;    // next example, globally
    bDatasetNode* nextC;   // next example of the same class
    
    bDatasetNode() : 
      ex(NULL), next(NULL), nextC(NULL) 
    {}
    bDatasetNode(bExample* e) 
      : ex(e), next(NULL), nextC(NULL) 
    {}
  };
  
    
  bDatasetNode* first;
  bDatasetNode* first_pos;
  bDatasetNode* first_neg;
  bDatasetNode* last;
  bDatasetNode* last_pos;
  bDatasetNode* last_neg;
  int _size;
  int _neg_size;
  int _pos_size;
  int _dimension;

public:

  class iterator {
    private :
      bDatasetNode* n;

    public:
    iterator() 
      : n(NULL)
      {}

    iterator(bDatasetNode* n0) 
      : n(n0)
      {}

    // ~iterator() { cerr << "dataset iterator destroyed\n"; }
    
    bExample* operator->() const { return n->ex; }

    bExample& operator*() const { return *(n->ex); }
    
    iterator& operator++() { n = n->next; return *this; }
    void next_class() { n = n->nextC; }
    iterator operator++(int) { iterator tmp(this->n); n = n->next; return tmp; }
    
    bool operator==(const iterator& rhs) { return n == rhs.n; }
    bool operator!=(const iterator& rhs) { return n != rhs.n; }
    
  };

  static void set_positive_label(int l) {
    bExample::set_positive_label(l);
  }


  bDataset();
  ~bDataset();

  void delete_examples();


  // input
  int read_stream(istream&);
  bool read_one_sample(istream&);
  void add_example(bExample*, bDatasetNode* = NULL); 

  // consultores
  int size() const { return _size; }
  int negative_size() const { return _neg_size; }
  int positive_size() const { return _pos_size; }
  int dimension() const { return _dimension; }
  
  // recorregut
  iterator begin() const { iterator p(first);  return p; }
  iterator begin_pos() const { iterator p(first_pos);  return p; }
  iterator begin_neg() const { iterator p(first_neg);  return p; }
  iterator end() const { iterator p(NULL); return p; }
  
  void     split(int feature, bDataset *ds0, bDataset *ds1, bool copy_ds_nodes);
  void     merge_dataset(bDataset *ds0);

  double   weight_positive_examples() const;
  double   weight_negative_examples() const;
  

  void print(ostream&) const;
  void print_positives(ostream&) const;
  void print_negatives(ostream&) const;

};



/********************************************************************************/
/*                                                                              */
/*  mlDataset:  dataset of binary examples                                      */
/*                                                                              */
/********************************************************************************/

class mlDataset 
{
 private:
  
  struct mlDatasetNode {
    mlExample* ex;          // pointer to the example
    mlDatasetNode* next;    // next example, globally
    
    mlDatasetNode() : 
      ex(NULL), next(NULL) 
    {}
    mlDatasetNode(mlExample* e) 
      : ex(e), next(NULL)
    {}
  };
  
    
  mlDatasetNode* first;
  mlDatasetNode* last;
  int  _size;
  int *_sizes;
  int  _dimension;
  int  _nlabels;

public:

  class iterator {
    private :
      mlDatasetNode* n;

    public:
    iterator() 
      : n(NULL)
      {}

    iterator(mlDatasetNode *n0) 
      : n(n0)
      {}

    // ~iterator() { cerr << "dataset iterator destroyed\n"; }
    
    mlExample* operator->() const { return n->ex; }

    mlExample& operator*() const { return *(n->ex); }
    
    iterator& operator++() { n = n->next; return *this; }
    iterator operator++(int) { iterator tmp(this->n); n = n->next; return tmp; }
    
    bool operator==(const iterator& rhs) { return n == rhs.n; }
    bool operator!=(const iterator& rhs) { return n != rhs.n; }
    
  };


  mlDataset(int nlabels);
  ~mlDataset();

  void delete_examples();


  // input
  void read_stream(istream&);
  void add_example(mlExample*, mlDatasetNode* = NULL); 

  // consultores
  int size() const { return _size; }
  int negative_size(int l) const { return _sizes[2*l]; }
  int positive_size(int l) const { return _sizes[2*l+1]; }
  int nlabels() const { return _nlabels; }
  int dimension() const { return _dimension; }
  
  // recorregut
  iterator begin() const { iterator p(first);  return p; }
  iterator end() const { iterator p(NULL); return p; }
  
  void     split(int feature, mlDataset *ds0, mlDataset *ds1, bool create_ds_nodes);
  
  void print(ostream&) const;
  void print_sizes(ostream&) const;

};



#endif



















