
#  
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without 
# modifications, as long as this notice is preserved.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

# pkginclude_HEADERS = hello.h

UTILS_DIR = ../../../utils/src/lib

INCLUDES = -I$(UTILS_DIR) -I../lib/

DEFS += -DLANG_ENGLISH

lib_LIBRARIES = libswirlcha.a

libswirlcha_a_SOURCES = \
	AnsHeap.C \
	AnsHeap.h \
	AnsStrCounts.C \
	AnswerTree.C \
	AnswerTree.h \
	auxify.C \
	auxify.h \
	Bchart.C \
	Bchart.h \
	BchartSm.C \
	Bst.C \
	Bst.h \
	ChartBase.C \
	ChartBase.h \
	ClassRule.C \
	ClassRule.h \
	CntxArray.C \
	CntxArray.h \
	CombineBests.C \
	CombineBests.h \
	ECArgs.C \
	ECArgs.h \
	ECString.h \
	Edge.C \
	Edge.h \
	EdgeHeap.C \
	EdgeHeap.h \
	edgeSubFns.C \
	edgeSubFns.h \
	EgsFromTree.C \
	ewDciTokStrm.C \
	ewDciTokStrm.h \
	extraMain.C \
	extraMain.h \
	FBinaryArray.C \
	FBinaryArray.h \
	Feat.C \
	Feat.h \
	Feature.C \
	Feature.h \
	FeatureTree.C \
	FeatureTree.h \
	fhSubFns.C \
	fhSubFns.h \
	Field.C \
	Field.h \
	FullHist.C \
	FullHist.h \
	GotIter.C \
	GotIter.h \
	headFinder.C \
	headFinderCh.C \
	headFinderCh.h \
	headFinder.h \
	InputTree.C \
	InputTree.h \
	Item.C \
	Item.h \
	Link.C \
	Link.h \
	MeChart.C \
	MeChart.h \
	Params.C \
	Params.h \
	ParserApi.C \
	ParserApi.h \
	ParseStats.C \
	ParseStats.h \
	SentRep.C \
	SentRep.h \
	Term.C \
	Term.h \
	TimeIt.C \
	TimeIt.h \
	UnitRules.C \
	UnitRules.h \
	utils.C \
	utils.h \
	ValHeap.C \
	ValHeap.h \
	Wrd.h

parseIt: parseIt.o
	g++ parseIt.o -L. -lcha -o parseIt

test: ParserApiTest.o
	g++ ParserApiTest.o -L. -lswirlcha -L../lib -lswirlmain -L$(UTILS_DIR) -lutils -o ParserApiTest
