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


// A translation unit that imports the module lemon in place of including
// the headers, and uses a graph, a map, an algorithm with its traits and a
// constant of the library

#include <cstdlib>

import lemon;

using namespace lemon;

int main()
{
 ListDigraph g;
 ListDigraph::Node s = g.addNode() , t = g.addNode() , u = g.addNode();
 ListDigraph::ArcMap< int > cap( g ) , cost( g );
 ListDigraph::Arc a = g.addArc( s , t ) , b = g.addArc( s , u ) ,
		  c = g.addArc( u , t );
 cap[ a ] = 1;
 cap[ b ] = 2;
 cap[ c ] = 2;
 cost[ a ] = 4;
 cost[ b ] = 1;
 cost[ c ] = 1;

 NetworkSimplex< ListDigraph > ns( g );
 ns.upperMap( cap ).costMap( cost ).stSupply( s , t , 3 );
 if( ns.run() != NetworkSimplex< ListDigraph >::OPTIMAL ||
     ns.totalCost() != 8 ) {
  return( EXIT_FAILURE );
  }

 Dijkstra< ListDigraph , ListDigraph::ArcMap< int > > dijkstra( g , cost );
 dijkstra.run( s );
 if( dijkstra.dist( t ) != 2 || dijkstra.predArc( s ) != INVALID ) {
  return( EXIT_FAILURE );
  }
 return( EXIT_SUCCESS );
 }
