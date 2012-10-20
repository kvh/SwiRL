/*
 * Copyright 1999, 2005 Brown University, Providence, RI.
 * 
 *                         All Rights Reserved
 * 
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose other than its incorporation into a
 * commercial product is hereby granted without fee, provided that the
 * above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Brown University not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.
 * 
 * BROWN UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR ANY
 * PARTICULAR PURPOSE.  IN NO EVENT SHALL BROWN UNIVERSITY BE LIABLE FOR
 * ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef CHARTBASE_H
#define CHARTBASE_H

#include "Edge.h"
#include "Item.h"
#include "SentRep.h"
#include "Feature.h"
#include <vector>

class InputTree;

class           ChartBase
{
public:
  ChartBase(SentRep& sentence,int id);
    virtual ~ChartBase();

    enum Err { OK, OVERFLW, FAILURE };

    // parsing functions, what the class is all about.
    virtual double  parse() = 0;

    // extracting information about the parse.
    void            set_Alphas();
    const Items&    items( int i, int j ) const
		    {   return regs[ i ][ j ];   }
    int             edgeCount() const    { return ruleiCounts_; }
    int             poppedEdgeCount() const    { return poppedEdgeCount_; }
    int             poppedEdgeCountAtS() const    { return poppedEdgeCountAtS_; }
    int             totEdgeCountAtS() const    { return totEdgeCountAtS_; }
    Item*           addtochart(const Term* trm);
    // printing information about the parse.
    const Item*     mapProbs();
    static bool finalPunc(const char* wrd);

    Item*           topS() { return get_S(); }
    int    thrdid;
    static int&	    ruleCountTimeout()  {   return ruleiCountTimeout_;   }
    static const double
		    badParse;	// error return value for parse(), crossEntropy
    SentRep&        sentence_;
    vector<vector<int> > extPos;
    int             effEnd(int pos);
    static float endFactor;
    static float midFactor;
    static int      numItemsToDelete[MAXNUMTHREADS];
    static int      itemsToDeletesize[MAXNUMTHREADS];
    static vector<Item*>    itemsToDelete[MAXNUMTHREADS];
    static bool     guided;
    void            setGuide(InputTree* tree);
protected:
    Item           *get_S() const;  
    Items           regs[MAXSENTLEN][MAXSENTLEN];
    vector<short>   guide[MAXSENTLEN][MAXSENTLEN];
    bool            inGuide(int st, int ed, int trm);
    bool            inGuide(Edge* e);
    list<Edge*>     waitingEdges[2][MAXSENTLEN];
    double          crossEntropy_;
    int             wrd_count_;
    int             poppedEdgeCount_;
    int             totEdgeCountAtS_;
    int             poppedEdgeCountAtS_;
    int             ruleiCounts_; // keeps track of how many edges have been
                                // created --- used to time out the parse
    Item*           pretermItems[4000];
    int             pretermNum;
    int		    endPos;
    static int      ruleiCountTimeout_ ; //how many rulei's before we time out.
    static int      poppedTimeout_;
    float           endFactorComp(Edge* dnrl);

private:
    void            free_chart_items(Items& itms);
    void            free_chart_itm(Item * itm);
    void            free_edges(list<Edge*>& edges);
};


#endif	/* ! CHARTBASE_H */
