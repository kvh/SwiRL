/********************************************************************************/
/*                                                                              */
/*  Author    : Xavier Carreras Pérez (carreras@lsi.upc.es)                     */
/*  Date      : November 2002                                                   */
/*                                                                              */
/*  Changes :                                                                   */
/*     June 2004 : mlDataset included here                                      */
/*                                                                              */
/*                                                                              */
/*  dataset.cc                                                                  */
/*                                                                              */
/********************************************************************************/

#include "dataset.h"
#include <cstdlib>

/********************************************************************************/
/*                                                                              */
/*  Class bDataset                                                              */
/*                                                                              */
/********************************************************************************/

// member functions

bDataset::bDataset() {
  first = NULL;
  first_pos = NULL;
  first_neg = NULL;
  last = NULL;
  last_pos = NULL;
  last_neg = NULL;
  _size = 0;
  _pos_size = 0;
  _neg_size = 0;
  _dimension = 0;
}

bDataset::~bDataset() {
  bDataset::bDatasetNode *next;
  while (first!=NULL) {
    next = first->next;
    delete first;
    first = next;
  }
  //  cerr << "dataset destroyed\n";
}

void bDataset::delete_examples() {
  bDataset::bDatasetNode *next;
  while (first!=NULL) {
    next = first->next;
    delete first->ex;
    delete first;
    first = next;
  }
}


void bDataset::add_example(bExample* e, bDatasetNode* node) {

  // si cal, creo node per l'exemple
  if (node == NULL) {
    node = new bDataset::bDatasetNode(e);
    // al crear un node s'inicialitzen els punters a NULL
  }
  else {
    node->next = NULL;
    node->nextC = NULL;
  }

  _size++;
  if (e->dimension() > _dimension) {
    _dimension = e->dimension();
  }

  // l'afegeixo a l'index d'exemples
  if (first == NULL) {
    first = node;
  }
  else {
    last->next = node;
  }
  last = node;
  
  if (e->positive()) {
    _pos_size++;
    // index d'exemples positius
    if (first_pos == NULL) {
      first_pos = node;
    }
    else {
      last_pos->nextC = node;
    }
    last_pos = node;
  }
  else {
    _neg_size++;
    // index d'exemples negatius
    if (first_neg == NULL) {
      first_neg = node;
    }
    else {
      last_neg->nextC = node;
    }
    last_neg = node;
  } 
}

int bDataset::read_stream(istream& in) {
  string line, strcl, feat;
  int cl;
  string::size_type i1;
  bExample* e;
  int example_count = 0;
  while(getline(in, line)) {

    // capturo la classe i l'string de features
    i1 = line.find_first_of(" ", 0);
    strcl = line.substr(0, i1);
    cl = atoi(strcl.c_str());
    if (i1 != string::npos) {
      feat = line.substr(i1+1);
    }
    else {
      feat = "";
    }
    
    // creo l'exemple 
    e = new bExample(cl, feat);
    // l'afegeixo, es crea dins el corresponent bDatasetNode 
    add_example(e);
    example_count ++;
  }

  return example_count;
}

/**
 * Reads just one sample from the stream
 */
bool bDataset::read_one_sample(istream& in) {
  string line, strcl, feat;
  int cl;
  string::size_type i1;
  bExample* e;
  
  if(getline(in, line) == NULL) return false;

  // capturo la classe i l'string de features
  i1 = line.find_first_of(" ", 0);
  strcl = line.substr(0, i1);
  cl = atoi(strcl.c_str());
  if (i1 != string::npos) {
    feat = line.substr(i1+1);
  }
  else {
    feat = "";
  }
    
  // creo l'exemple 
  e = new bExample(cl, feat);
  // l'afegeixo, es crea dins el corresponent bDatasetNode 
  add_example(e);

  return true;
}

// ---------- dataset splitting and merging ---------------------------------

void  bDataset::split(int feature, bDataset *ds0, bDataset *ds1, bool copy_ds_nodes) {
  bDataset::bDatasetNode *node = first;
  bDataset::bDatasetNode *next;
  while (node != NULL) {
    next = node->next;
    if (node->ex->feature_value(feature)) {
      if (copy_ds_nodes) {
	ds1->add_example(node->ex);
      }
      else {
	ds1->add_example(node->ex, node);
      }
    }
    else {
      if (copy_ds_nodes) {
	ds0->add_example(node->ex);
      }
      else {
	ds0->add_example(node->ex, node);
      }
    }
    node = next;
  }
  if (!copy_ds_nodes) {
    first = NULL;
    first_pos = NULL;
    first_neg = NULL;
  }
}


void  bDataset::merge_dataset(bDataset *ds0) {

  // Incloc ds0 a this
  // primer els comptatges
  _size += ds0->_size;
  _pos_size += ds0->_pos_size;
  _neg_size += ds0->_neg_size;
  if (ds0->_dimension > _dimension) {
    _dimension = ds0->_dimension;
  }

  // enllaço els exemples de ds0 a this
  if (last != NULL) {
    last->next = ds0->first;
  }
  else {
    first = ds0->first;
  }
  if (last_pos != NULL) {
    last_pos->nextC = ds0->first_pos;
  }
  else {
    first_pos  = ds0->first_pos;
  }
  if (last_neg != NULL) {
    last_neg->nextC = ds0->first_neg;
  }
  else {
    first_neg = ds0->first_neg;
  }

  // actualitzo els last's
  if (ds0->last != NULL) {
    last = ds0->last;
  }
  if (ds0->last_pos != NULL) {
    last_pos = ds0->last_pos;
  }
  if (ds0->last_neg != NULL) {
    last_neg = ds0->last_neg;
  }

  // desenllaço els els exemples de ds0
  ds0->first = NULL;
  ds0->last = NULL;
  ds0->first_pos = NULL;
  ds0->last_pos = NULL;
  ds0->first_neg = NULL;
  ds0->last_neg = NULL;
  ds0->_size = 0;
  ds0->_pos_size = 0;  
  ds0->_neg_size = 0;  
  ds0->_dimension = 0;  

  // borro ds0
  // cal borrar ??????????????????????
  // delete ds0;
}


// ---------- examples weight -----------------------------------------------

double bDataset::weight_positive_examples() const {
  bDataset::bDatasetNode *node = first_pos;
  double w = 0.0;
  while (node != NULL) {
    w += node->ex->weight();
    node = node->nextC;
  }
  return w;
}

double bDataset::weight_negative_examples() const {
  bDataset::bDatasetNode *node = first_neg;
  double w = 0.0;
  while (node != NULL) {
    w += node->ex->weight();
    node = node->nextC;
  }
  return w;
}


// ---------- functions for printing ----------------------------------------

void bDataset::print(ostream& o) const {
  bDataset::bDatasetNode *h = first;
  int i = 0;
  o << "  %% dataset : " << size() << " examples; " << dimension() << " dimensions;" << endl;
  while (h != NULL) {
    o << "   ex " << i << ":";
    h->ex->print(o);
    o << endl;
    h = h->next;
    i++;
  }
}

void bDataset::print_positives(ostream& o) const {
  bDataset::bDatasetNode *h = first_pos;
  int i = 0;
  while (h != NULL) {
    o << "   ex " << i << ": ";
    h->ex->print(o);
    o << endl;
    h = h->nextC;
    i++;
  }
}

void bDataset::print_negatives(ostream& o) const {
  bDataset::bDatasetNode *h = first_neg;
  int i = 0;
  while (h != NULL) {
    o << "   ex " << i << ": ";
    h->ex->print(o);
    o << endl;
    h = h->nextC;
    i++;
  }
}




/********************************************************************************/
/*                                                                              */
/*  Class mlDataset                                                              */
/*                                                                              */
/********************************************************************************/

// member functions

mlDataset::mlDataset(int nlabels) {
  first = NULL;
  last = NULL;
  _nlabels = nlabels;
  mlOutput::set_nlabels(nlabels);
  _size = 0;
  _sizes = new int[2*nlabels];
  int l; 
  for (l=0;l<2*nlabels;l++) {
    _sizes[l] = 0;
  }
  _dimension = 0;
}

mlDataset::~mlDataset() {
  mlDataset::mlDatasetNode *next;
  delete [] _sizes;
  while (first!=NULL) {
    next = first->next;
    delete first;
    first = next;
  }
  //  cerr << "dataset destroyed\n";
}

void mlDataset::delete_examples() {
  mlDataset::mlDatasetNode *next;
  while (first!=NULL) {
    next = first->next;
    delete first->ex;
    delete first;
    first = next;
  }
}

void mlDataset::add_example(mlExample* e, mlDatasetNode* node) {

  // si cal, creo node per l'exemple
  if (node == NULL) {
    node = new mlDataset::mlDatasetNode(e);
    // al crear un node s'inicialitzen els punters a NULL
  }
  else {
    node->next = NULL;
  }

  _size++;
  if (e->dimension() > _dimension) {
    _dimension = e->dimension();
  }
  int l; 
  for (l=0;l<_nlabels;l++) {
    if (e->belongs_to(l)) {
      _sizes[2*l+1]++;
    }
    else {
      _sizes[2*l]++;
    }
  }
  // l'afegeixo a l'index d'exemples
  if (first == NULL) {
    first = node;
  }
  else {
    last->next = node;
  }
  last = node;

}

void mlDataset::read_stream(istream& in) {
  string line, cl, feat;
  string::size_type i1;
  mlExample* e;
  while(getline(in, line)) {

    // capturo la classe i l'string de features
    i1 = line.find_first_not_of(" ", 0);
    i1 = line.find_first_of(" ", i1);
    cl = line.substr(0, i1);
    if (i1 != string::npos) {
      feat = line.substr(i1+1);
    }
    else {
      feat = "";
    }
    
    // creo l'exemple 
    // cout << "new mlExample; class str ->" << cl << "<- feat str ->" << feat << "<--\n";
    e = new mlExample(cl, feat);
    // l'afegeixo, es crea dins el corresponent mlDatasetNode 
    add_example(e);
  }
}

// ---------- dataset splitting -----------------------------------------

void  mlDataset::split(int feature, mlDataset *ds0, mlDataset *ds1, bool create_ds_nodes) {
  mlDataset::mlDatasetNode *node = first;
  mlDataset::mlDatasetNode *next;
  while (node != NULL) {
    next = node->next;
    if (node->ex->feature_value(feature)) {
      if (create_ds_nodes) {
	ds1->add_example(node->ex);
      }
      else {
	ds1->add_example(node->ex, node);
      }
    }
    else {
      if (create_ds_nodes) {
	ds0->add_example(node->ex);
      }
      else {
	ds0->add_example(node->ex, node);
      }
    }
    node = next;
  }
  if (!create_ds_nodes) {
    first = NULL;
  }
}


// ---------- functions for printing ----------------------------------------

void mlDataset::print(ostream& o) const {
  mlDataset::mlDatasetNode *h = first;
  int i = 0;
  o << "  %% dataset : " << size() << " examples; " << dimension() << " dimensions;" << endl;
  while (h != NULL) {
    o << "   ex " << i << ":";
    h->ex->print(o);
    o << endl;
    h = h->next;
    i++;
  }
}


void mlDataset::print_sizes(ostream& o) const {
  int l;
  for (l=0;l<_nlabels;l++) {
    cout << l << "~[-" << _sizes[2*l] << ":+" << _sizes[2*l+1] << "] ";
  }
}















