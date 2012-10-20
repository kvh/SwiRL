# Copyright (C) 2006 Mihai Surdeanu <surdeanu@gmail.com>
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#

pkginclude_HEADERS = \
  Assert.h \
  RCIPtr.h \
  Wide.h \
  CharArrayEqualFunc.h \
  CharArrayHashFunc.h \
  ClassifiedArg.h \
  HashMap.h \
  StringMap.h \
  Argument.h \
  Classifier.h \
  Morpher.h \
  PathFeatures.h \
  Tree.h \
  Swirl.h 

ML_DIR = ../ab
CHARNIAK_DIR = ../charniak
WORDNET_DIR = $(WNHOME)

INCLUDES = -I. -I$(ML_DIR) -I$(CHARNIAK_DIR) -I$(WORDNET_DIR)/include

DEFS += -DLANG_ENGLISH

lib_LIBRARIES = libswirlmain.a

libswirlmain_a_SOURCES = \
  Assert.h \
  AdaBoostClassifier.cc \
  AdaBoostClassifier.h \
  Analysis.cc \
  Argument.cc \
  Argument.h \
  BankTreeProducer.h \
  BankTreeProducer.cc \
  CharArrayEqualFunc.cc \
  CharArrayEqualFunc.h \
  CharArrayHashFunc.cc \
  CharArrayHashFunc.h \
  CharUtils.cc \
  CharUtils.h \
  Classifier.h \
  ClassifiedArg.h \
  ClassifiedArg.cc \
  Constants.h \
  Edge.h \
  EdgeLexer.cc \
  EdgeLexer.h \
  EdgeParser.h \
  Exception.cc \
  Exception.h \
  For.h \
  HashMap.h \
  Head.h \
  HeadEnglish.h \
  Lexicon.cc \
  Lexicon.h \
  Logger.cc \
  Logger.h \
  Morpher.h \
  Oracle.h \
  Oracle.cc \
  Pair.h \
  Parameters.cc \
  Parameters.h \
  PathFeatures.h \
  RCIPtr.h \
  Score.h \
  StringMap.h \
  Swirl.cc \
  Swirl.h \
  Tree.cc \
  Tree.h \
  TreeClassification.cc \
  TreeProducer.h \
  TreeConvert.cc \
  UnitCandidate.h \
  UnitCandidate.cc \
  Wide.h \
  Word.h \
  Wn.h \
  Wn.cc 




