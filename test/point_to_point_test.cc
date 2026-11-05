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


// BidirDijkstra and AStar, with heuristics that are zero, exact, half the
// exact one, and admissible but not consistent, against Dijkstra on random
// digraphs, with the paths they give

#include <vector>

#include <lemon/adaptors.h>
#include <lemon/astar.h>
#include <lemon/bidir_dijkstra.h>
#include <lemon/dijkstra.h>
#include <lemon/fib_heap.h>
#include <lemon/list_graph.h>
#include <lemon/maps.h>
#include <lemon/path.h>
#include <lemon/random.h>

#include "test_tools.h"

using namespace lemon;

typedef ListDigraph Digraph;
DIGRAPH_TYPEDEFS( Digraph );

// a path from s to t, whose length is dist
template< class P >
void checkPathOf( const Digraph & g , const IntArcMap & len , const P & p ,
		  Node s , Node t , int dist , const char * who )
{
 check( checkPath( g , p ) , who );
 if( s == t ) {
  check( p.empty() && dist == 0 , who );
  return;
  }
 check( pathSource( g , p ) == s && pathTarget( g , p ) == t , who );
 int total = 0;
 for( typename P::ArcIt a( p ) ; a != INVALID ; ++a ) {
  total += len[ a ];
  }
 check( total == dist , who );
 }

template< class H >
void checkAStar( const Digraph & g , const IntArcMap & len , const H & h ,
		 Node s ,
		 Node t , bool reach , int dist , const char * who )
{
 AStar< Digraph , IntArcMap , H > astar( g , len , h );
 check( astar.run( s , t ) == reach , who );
 if( reach ) {
  check( astar.dist() == dist , who );
  checkPathOf( g , len , astar.path() , s , t , dist , who );
  }
 // the same with a FibHeap in place of the BinHeap [see SetHeap]
 typename AStar< Digraph , IntArcMap , H >::template SetHeap<
   FibHeap< int , IntNodeMap > >::Create fib( g , len , h );
 check( fib.run( s , t ) == reach , who );
 if( reach ) {
  check( fib.dist() == dist , who );
  checkPathOf( g , len , fib.path() , s , t , dist , who );
  }
 }

void checkDigraph( int n , int m , int maxlen )
{
 Digraph g;
 std::vector< Node > v;
 for( int i = 0 ; i < n ; ++i ) {
  v.push_back( g.addNode() );
  }
 IntArcMap len( g );
 for( int i = 0 ; i < m ; ++i ) {
  len[ g.addArc( v[ rnd[ n ] ] , v[ rnd[ n ] ] ) ] = rnd[ maxlen + 1 ];
  }

 BidirDijkstra< Digraph , IntArcMap > bidir( g , len );
 BidirDijkstra< Digraph , IntArcMap >::SetHeap<
   FibHeap< int , IntNodeMap > >::Create fib( g , len );
 for( int q = 0 ; q < 20 ; ++q ) {
  Node s = v[ rnd[ n ] ] , t = v[ rnd[ n ] ];
  Dijkstra< Digraph , IntArcMap > dijkstra( g , len );
  dijkstra.run( s );
  bool reach = dijkstra.reached( t );
  int dist = reach ? dijkstra.dist( t ) : 0;

  check( bidir.run( s , t ) == reach , "BidirDijkstra: reachability" );
  if( reach ) {
   check( bidir.dist() == dist , "BidirDijkstra: distance" );
   checkPathOf( g , len , bidir.path() , s , t , dist , "BidirDijkstra" );
   }
  check( fib.run( s , t ) == reach , "BidirDijkstra with FibHeap" );
  if( reach ) {
   check( fib.dist() == dist , "BidirDijkstra with FibHeap: distance" );
   checkPathOf( g , len , fib.path() , s , t , dist ,
		"BidirDijkstra with FibHeap" );
   }

  // the distances to t, on the reverse digraph, for the heuristics
  ReverseDigraph< const Digraph > rev( g );
  Dijkstra< ReverseDigraph< const Digraph > , IntArcMap > back( rev , len );
  back.run( t );
  IntNodeMap exact( g , 0 ) , half( g , 0 ) , patchy( g , 0 );
  for( NodeIt u( g ) ; u != INVALID ; ++u ) {
   if( back.reached( u ) ) {
    exact[ u ] = back.dist( u );
    half[ u ] = back.dist( u ) / 2;
    patchy[ u ] = rnd.boolean() ? back.dist( u ) : 0;
    }
   }
  checkAStar( g , len , ConstMap< Node , int >( 0 ) , s , t , reach , dist ,
	      "AStar with zero heuristic" );
  checkAStar( g , len , exact , s , t , reach , dist ,
	      "AStar with exact heuristic" );
  checkAStar( g , len , half , s , t , reach , dist ,
	      "AStar with half heuristic" );
  checkAStar( g , len , patchy , s , t , reach , dist ,
	      "AStar with inconsistent heuristic" );
  }
 }

int main()
{
 rnd.seed( 20261103 );
 const int sizes[][ 2 ] = {
  { 1 , 0 } ,   { 1 , 2 } ,    { 2 , 1 } ,    { 5 , 3 } ,
  { 10 , 30 } , { 50 , 120 } , { 50 , 600 } , { 200 , 900 }
  };
 for( const auto & size : sizes ) {
  for( int maxlen : { 0 , 1 , 10 , 1000 } ) {
   for( int k = 0 ; k < 3 ; ++k ) {
    checkDigraph( size[ 0 ] , size[ 1 ] , maxlen );
    }
   }
  }
 return( 0 );
 }
