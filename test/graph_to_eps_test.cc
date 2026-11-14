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

// The bounding box of graphToEps() on graphs whose drawing has no area

#include <sstream>
#include <string>

#include <lemon/graph_to_eps.h>
#include <lemon/list_graph.h>
#include "test_tools.h"

using namespace lemon;

// whether the bounding box of the EPS text has four coordinates of at most
// 10^6 in absolute value
bool boundingBoxOk( const std::string & eps )
{
 std::istringstream is( eps );
 std::string line;
 while( std::getline( is , line ) ) {
  if( line.compare( 0 , 15 , "%%BoundingBox: " ) == 0 ) {
   std::istringstream ls( line.substr( 15 ) );
   long c[ 4 ];
   if( ! ( ls >> c[ 0 ] >> c[ 1 ] >> c[ 2 ] >> c[ 3 ] ) )
    return( false );
   for( int i = 0 ; i < 4 ; ++i ) {
    if( c[ i ] < -1000000 || c[ i ] > 1000000 )
     return( false );
    }
   return( true );
   }
  }
 return( false );
 }

int main()
{
 // a digraph without nodes, and one whose nodes have no size and lie on a
 // horizontal line
 {
  ListDigraph g;
  std::ostringstream os;
  graphToEps( g , os ).run();
  check( boundingBoxOk( os.str() ) , "Wrong bounding box without nodes" );

  ListDigraph::Node u = g.addNode() , v = g.addNode();
  ListDigraph::NodeMap< dim2::Point< double > > coords( g );
  coords[ u ] = dim2::Point< double >( 0 , 0 );
  coords[ v ] = dim2::Point< double >( 5 , 0 );
  std::ostringstream os2;
  graphToEps( g , os2 ).coords( coords ).nodeScale( 0 ).run();
  check( boundingBoxOk( os2.str() ) , "Wrong bounding box on a line" );
  }

 return( 0 );
 }
