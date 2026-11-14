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

// readNautyGraph() on complete lines, with and without the header of the
// format, and on lines too short for their graph, which were read past
// their end and now leave the graph empty and the stream failed

#include <sstream>
#include <lemon/smart_graph.h>
#include <lemon/nauty_reader.h>

#include "test_tools.h"

using namespace lemon;

int main()
{
 {
  std::istringstream in( "D~{\n>>graph6<<C]\n" );
  SmartGraph k5 , c4;
  readNautyGraph( k5 , in );
  readNautyGraph( c4 , in );
  check( countNodes( k5 ) == 5 && countEdges( k5 ) == 10 ,
	 "Wrong complete graph" );
  check( countNodes( c4 ) == 4 && countEdges( c4 ) == 4 ,
	 "Wrong cycle read after the header" );
  }

 const char * truncated[] = { "D~\n" , "D\n" , ">>graph6\n" , "~?\n" , "\n" };
 for( int k = 0 ; k < 5 ; ++k ) {
  std::istringstream in( truncated[ k ] );
  SmartGraph g;
  readNautyGraph( g , in );
  check( in.fail() && countNodes( g ) == 0 ,
	 "A line too short read as a graph" );
  }
 return( 0 );
 }
