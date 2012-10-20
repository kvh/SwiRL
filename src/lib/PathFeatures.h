/**
 * This stores info about the well-known path feature
 */

#ifndef PATH_FEATURES_H
#define PATH_FEATURES_H

#include <vector>
#include "Wide.h"

namespace srl {
  
/**
 * Set of basic features extracted from the full predicate-argument path
 */
class PathFeatures {
 public:
  PathFeatures() {
    clauseCount = 0;
    upClauseCount = 0;
    downClauseCount = 0;   
    vpCount = 0;
    upVpCount = 0;
    downVpCount = 0;
    length = 0;
    tooLong = false;
    subsumptionCount = 0;
    largeSubsumption = false;
  }

 public:
  /** 
   * List of paths between arg and pred
   * The first path is the specific path a la Gildea
   * The others are generalized paths
   */
  std::vector<String> paths;

  int clauseCount;
  int upClauseCount;
  int downClauseCount;

  int vpCount;
  int upVpCount;
  int downVpCount;

  int length;
  bool tooLong;

  int subsumptionCount;
  bool largeSubsumption;
};

/**
 * Set of basic features extracted from the flat predicate-argument path
 */
class FlatPathFeatures {
 public:
  FlatPathFeatures() {
    clauseCount = 0;
    vpCount = 0;
    length = 0;
  }

 public:
  String path;
  int clauseCount;
  int vpCount;
  int length;
};

}

#endif
