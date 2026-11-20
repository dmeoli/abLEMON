/* -*- mode: C++; indent-tabs-mode: nil; -*-
 *
 * This file is a part of LEMON, a generic C++ optimization library.
 *
 * Copyright (C) 2003-2013
 * Egervary Jeno Kombinatorikus Optimalizalasi Kutatocsoport
 * (Egervary Research Group on Combinatorial Optimization, EGRES).
 *
 * Permission to use, modify and distribute this software is granted
 * provided that this copyright notice appears in all copies. For
 * precise terms see the accompanying LICENSE file.
 *
 * This software is provided "AS IS" with no warranty of any kind,
 * express or implied, and with no claim as to its suitability for any
 * purpose.
 *
 */

#include <string>

#include <lemon/list_graph.h>
#include <lemon/lgf_reader.h>
#include "test_tools.h"

using namespace lemon;

char test_lgf[] = "@nodes\n"
		  "label\n"
		  "0\n"
		  "1\n"
		  "@arcs\n"
		  "     label\n"
		  "0 1  0\n"
		  "1 0  1\n"
		  "@attributes\n"
		  "source 0\n"
		  "target 1\n";

char test_lgf_nomap[] = "@nodes\n"
			"label\n"
			"0\n"
			"1\n"
			"@arcs\n"
			"     -\n"
			"0 1\n";

char test_lgf_bad1[] = "@nodes\n"
		       "label\n"
		       "0\n"
		       "1\n"
		       "@arcs\n"
		       "     - another\n"
		       "0 1\n";

char test_lgf_bad2[] = "@nodes\n"
		       "label\n"
		       "0\n"
		       "1\n"
		       "@arcs\n"
		       "     label -\n"
		       "0 1\n";


int main()
{
 {
  ListDigraph d;
  ListDigraph::Node s , t;
  ListDigraph::ArcMap< int > label( d );
  std::istringstream input( test_lgf );
  digraphReader( d , input )
    .node( "source" , s )
    .node( "target" , t )
    .arcMap( "label" , label )
    .run();
  check( countNodes( d ) == 2 , "There should be 2 nodes" );
  check( countArcs( d ) == 2 , "There should be 2 arcs" );
  }
 {
  ListGraph g;
  ListGraph::Node s , t;
  ListGraph::EdgeMap< int > label( g );
  std::istringstream input( test_lgf );
  graphReader( g , input )
    .node( "source" , s )
    .node( "target" , t )
    .edgeMap( "label" , label )
    .run();
  check( countNodes( g ) == 2 , "There should be 2 nodes" );
  check( countEdges( g ) == 2 , "There should be 2 arcs" );
  }

 {
  ListDigraph d;
  std::istringstream input( test_lgf_nomap );
  digraphReader( d , input ).run();
  check( countNodes( d ) == 2 , "There should be 2 nodes" );
  check( countArcs( d ) == 1 , "There should be 1 arc" );
  }
 {
  ListGraph g;
  std::istringstream input( test_lgf_nomap );
  graphReader( g , input ).run();
  check( countNodes( g ) == 2 , "There should be 2 nodes" );
  check( countEdges( g ) == 1 , "There should be 1 edge" );
  }

 {
  ListDigraph d;
  std::istringstream input( test_lgf_bad1 );
  bool ok = false;
  try {
   digraphReader( d , input ).run();
   }
  catch( FormatError & ) {
   ok = true;
   }
  check( ok , "FormatError exception should have occured" );
  }
 {
  ListGraph g;
  std::istringstream input( test_lgf_bad1 );
  bool ok = false;
  try {
   graphReader( g , input ).run();
   }
  catch( FormatError & ) {
   ok = true;
   }
  check( ok , "FormatError exception should have occured" );
  }

 {
  ListDigraph d;
  std::istringstream input( test_lgf_bad2 );
  bool ok = false;
  try {
   digraphReader( d , input ).run();
   }
  catch( FormatError & ) {
   ok = true;
   }
  check( ok , "FormatError exception should have occured" );
  }
 {
  ListGraph g;
  std::istringstream input( test_lgf_bad2 );
  bool ok = false;
  try {
   graphReader( g , input ).run();
   }
  catch( FormatError & ) {
   ok = true;
   }
  check( ok , "FormatError exception should have occured" );
  }

 // only 0-9, a-f and A-F are hexadecimal digits after \x
 {
  ListDigraph d;
  ListDigraph::NodeMap< std::string > name( d );
  std::istringstream input( "@nodes\nlabel name\n0 \\x4g\n@arcs\n -\n" );
  digraphReader( d , input ).nodeMap( "name" , name ).run();
  check( name[ ListDigraph::NodeIt( d ) ] == std::string( "\x04" ) + "g" ,
	 "Wrong hexadecimal escape" );
  }
 {
  ListDigraph d;
  std::istringstream input( "@nodes\nlabel\n\\xg\n@arcs\n -\n" );
  bool ok = false;
  try {
   digraphReader( d , input ).run();
   }
  catch( FormatError & ) {
   ok = true;
   }
  check( ok , "FormatError exception should have occured" );
  }

 // a label repeated in a node section is an error, and one repeated in an
 // arc or an edge section names no item, which is an error only when an
 // attribute refers to it
 {
  const std::string repeated_arc =
    "@nodes\nlabel\n0\n1\n@arcs\n label\n0 1 5\n1 0 5\n";
  const std::string inputs[] = { "@nodes\nlabel\n0\n0\n@arcs\n -\n" ,
				 repeated_arc + "@attributes\narc 5\n" };
  for( int i = 0 ; i < 2 ; ++i ) {
   ListDigraph d;
   ListDigraph::Arc a;
   std::istringstream input( inputs[ i ] );
   bool ok = false;
   try {
    if( i == 0 )
     digraphReader( d , input ).run();
    else
     digraphReader( d , input ).arc( "arc" , a ).run();
    }
   catch( FormatError & ) {
    ok = true;
    }
   check( ok , "FormatError exception should have occured" );
   ListGraph g;
   ListGraph::Edge e;
   std::istringstream ginput( inputs[ i ] );
   ok = false;
   try {
    if( i == 0 )
     graphReader( g , ginput ).run();
    else
     graphReader( g , ginput ).edge( "arc" , e ).run();
    }
   catch( FormatError & ) {
    ok = true;
    }
   check( ok , "FormatError exception should have occured" );
   }
  ListDigraph d;
  std::istringstream input( repeated_arc );
  digraphReader( d , input ).run();
  check( countArcs( d ) == 2 , "Wrong arcs with a repeated label" );
  }
 }
