# Copyright (C) 2006 Mihai Surdeanu <surdeanu@gmail.com>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

EXTRA_DIST = srl-eval.pl

MY_LIB_DIR = ../lib
WORDNET_DIR = $(WNHOME)
ML_DIR = ../ab
PARSER_DIR = ../charniak

INCLUDES = -I$(MY_LIB_DIR) -I$(ML_DIR)

bin_PROGRAMS = \
  convert_treebank swirl_corpus_stats swirl_make_samples \
  swirl_make_binary_samples ab_learner \
  convert_for_test swirl_parse_classify swirl_classify 

swirl_make_samples_SOURCES = swirlMakeSamples.cc
swirl_make_samples_LDADD = \
  -L$(MY_LIB_DIR) -lswirlmain \
  -L$(WORDNET_DIR)/lib -lwn

swirl_make_binary_samples_SOURCES = swirlMakeBinarySamples.cc
swirl_make_binary_samples_LDADD = \
  -L$(MY_LIB_DIR) -lswirlmain \
  -L$(WORDNET_DIR)/lib -lwn

swirl_classify_SOURCES = swirlClassify.cc
swirl_classify_LDADD = \
  -L$(MY_LIB_DIR) -lswirlmain \
  -L$(ML_DIR) -lswirlab \
  -L$(WORDNET_DIR)/lib -lwn 

swirl_parse_classify_SOURCES = swirlParseAndClassify.cc
swirl_parse_classify_LDADD = \
  -L$(MY_LIB_DIR) -lswirlmain \
  -L$(PARSER_DIR) -lswirlcha \
  -L$(ML_DIR) -lswirlab \
  -L$(WORDNET_DIR)/lib -lwn

swirl_corpus_stats_SOURCES = corpusStats.cc
swirl_corpus_stats_LDADD = \
  -L$(MY_LIB_DIR) -lswirlmain \
  -L$(WORDNET_DIR)/lib -lwn

#swirl_corpus_analysis_SOURCES = corpusAnalysis.cc
#swirl_corpus_analysis_LDADD = \
#  -L$(MY_LIB_DIR) -lswirlmain \
#  -L$(WORDNET_DIR)/lib -lwn

convert_treebank_SOURCES = convertToTreebank.cc
convert_treebank_LDADD = \
  -L$(MY_LIB_DIR) -lswirlmain \
  -L$(WORDNET_DIR)/lib -lwn

convert_for_test_SOURCES = convertForTest.cc
convert_for_test_LDADD = \
  -L$(MY_LIB_DIR) -lswirlmain \
  -L$(WORDNET_DIR)/lib -lwn

ab_learner_SOURCES = ab_learner.cc
ab_learner_LDADD = -L$(ML_DIR) -lswirlab

lib:
	cd ../lib; make

