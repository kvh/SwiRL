#ifndef PREDARG_CONSTANTS_H
#define PREDARG_CONSTANTS_H

#define MAX_PROP_LINE (16 * 1024)

#define MAX_PATH_LENGTH 20

/** Max line size in any CoNLL-formatted file */
#define MAX_CONLL_LINE (32 * 1024)

//
// If words/lemmas/preds appear less than these thresholds => mark as unknown 
//
#define UNKNOWN_WORD_THRESHOLD 10
#define UNKNOWN_LEMMA_THRESHOLD 10
#define UNKNOWN_PREDICATE_THRESHOLD 2

#define SIBLING_CONTEXT_LENGTH 3

/** Number of positive examples to consider in training */
#define POSITIVE_EXAMPLES_MAX_COUNT 250000 // 100% of training
// #define POSITIVE_EXAMPLES_MAX_COUNT 125000 // 50% of training
// #define POSITIVE_EXAMPLES_MAX_COUNT 150000 // 60% of training

/** Number of negative examples to consider in training */
#define NEGATIVE_EXAMPLES_MAX_COUNT 1000000 // this fits in 4GB of RAM
//#define NEGATIVE_EXAMPLES_MAX_COUNT 600000 // this fits in 2GB of RAM
//#define NEGATIVE_EXAMPLES_MAX_COUNT 480000 // used for the xval data only!!!

/** Overall number of numbered arguments */
#define MAX_NUMBERED_ARGS 6

#define INCLUDES_PER 0
#define INCLUDES_ORG 1
#define INCLUDES_LOC 2
#define INCLUDES_MISC 3
#define INCLUDES_NN 4
#define INCLUDES_NNP 5
#define INCLUDES_IN 6
#define INCLUDES_RB 7

// How to mark head phrases in the extended CoNLL representation
#define HEAD_PREFIX "^"

/** Use only features that appear > DISCARD_THRESHOLD */
#define DISCARD_THRESHOLD 4

/** Maximum line size for the feature file */
#define MAX_FEATURE_LINE (32 * 1024)

/** Maximum line size for any sample file */
#define MAX_SAMPLE_LINE (128 * 1024)

/** The gamma constant of the softmax function */
#define SOFTMAX_GAMMA 0.1

/**
 * Local beam: how many candidate labels to accept per argument candidate?
 */
#define LOCAL_COUNT_BEAM 10
#define LOCAL_CONF_BEAM 100.00

/**
 * Global beam: how many candidate frames to accept per predicate?
 */
#define GLOBAL_BEAM 15

#endif

