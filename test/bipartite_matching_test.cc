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


// HopcroftKarp against MaxMatching, and MinCostAssignment against
// NetworkSimplex, on random bipartite graphs, with the certificates of
// both: the vertex cover and the potentials

#include <vector>

#include <lemon/bipartite_matching.h>
#include <lemon/list_graph.h>
#include <lemon/matching.h>
#include <lemon/network_simplex.h>
#include <lemon/random.h>
#include <lemon/smart_graph.h>

#include "test_tools.h"

using namespace lemon;

typedef ListBpGraph BpGraph;
BPGRAPH_TYPEDEFS( BpGraph );

// a random bipartite graph with nr red and nb blue nodes, each pair joined
// with probability p, possibly more than once, and costs in [-20, 50]
void randomGraph( BpGraph & g , IntEdgeMap & cost , int nr , int nb ,
		  double p )
{
 std::vector< RedNode > red;
 std::vector< BlueNode > blue;
 for( int i = 0 ; i < nr ; ++i ) {
  red.push_back( g.addRedNode() );
  }
 for( int i = 0 ; i < nb ; ++i ) {
  blue.push_back( g.addBlueNode() );
  }
 for( int i = 0 ; i < nr ; ++i ) {
  for( int j = 0 ; j < nb ; ++j ) {
   while( rnd() < p ) {
    cost[ g.addEdge( red[ i ] , blue[ j ] ) ] = rnd[ 71 ] - 20;
    if( rnd() < .8 ) {
     break;
     }
    }
   }
  }
 }

// a matching map that is a matching of g, of the given size
template< class MM >
void checkMatching( const BpGraph & g , const MM & m , int size )
{
 int n = 0;
 for( NodeIt v( g ) ; v != INVALID ; ++v ) {
  if( m[ v ] != INVALID ) {
   check( g.u( m[ v ] ) == v || g.v( m[ v ] ) == v , "wrong matching" );
   Node w = g.oppositeNode( v , m[ v ] );
   check( m[ w ] == m[ v ] , "wrong matching" );
   ++n;
   }
  }
 check( n == 2 * size , "wrong size of the matching" );
 }

void checkHopcroftKarp( const BpGraph & g )
{
 MaxMatching< BpGraph > general( g );
 general.run();
 for( int greedy = 0 ; greedy < 2 ; ++greedy ) {
  HopcroftKarp< BpGraph > hk( g );
  if( greedy ) {
   hk.greedyInit();
   }
  else {
   hk.init();
   }
  hk.start();
  check( hk.matchingSize() == general.matchingSize() ,
	 "HopcroftKarp is not maximum" );
  checkMatching( g , hk.matchingMap() , hk.matchingSize() );
  int cover = 0;
  for( NodeIt v( g ) ; v != INVALID ; ++v ) {
   cover += hk.cover( v );
   if( hk.matching( v ) != INVALID ) {
    check( hk.mate( v ) == g.oppositeNode( v , hk.matching( v ) ) ,
	   "wrong mate" );
    }
   }
  check( cover == hk.matchingSize() , "the cover is not minimum" );
  for( EdgeIt e( g ) ; e != INVALID ; ++e ) {
   check( hk.cover( g.u( e ) ) || hk.cover( g.v( e ) ) ,
	  "the cover misses an edge" );
   }
  }
 }

void checkAssignment( const BpGraph & g , const IntEdgeMap & cost )
{
 // the same problem as a minimum cost flow: a unit from each red node to
 // a sink, through the blue nodes
 SmartDigraph d;
 BpGraph::NodeMap< SmartDigraph::Node > copy( g );
 SmartDigraph::Node sink = d.addNode();
 SmartDigraph::ArcMap< int > cap( d ) , dcost( d );
 SmartDigraph::NodeMap< int > supply( d , 0 );
 int nr = 0;
 for( NodeIt v( g ) ; v != INVALID ; ++v ) {
  copy[ v ] = d.addNode();
  if( g.red( v ) ) {
   supply[ copy[ v ] ] = 1;
   ++nr;
   }
  else {
   SmartDigraph::Arc a = d.addArc( copy[ v ] , sink );
   cap[ a ] = 1;
   dcost[ a ] = 0;
   }
  }
 supply[ sink ] = -nr;
 for( EdgeIt e( g ) ; e != INVALID ; ++e ) {
  SmartDigraph::Arc a =
    d.addArc( copy[ g.redNode( e ) ] , copy[ g.blueNode( e ) ] );
  cap[ a ] = 1;
  dcost[ a ] = cost[ e ];
  }
 NetworkSimplex< SmartDigraph > ns( d );
 ns.upperMap( cap ).costMap( dcost ).supplyMap( supply );
 bool feasible = ns.run() == NetworkSimplex< SmartDigraph >::OPTIMAL;

 MinCostAssignment< BpGraph , IntEdgeMap > mca( g , cost );
 check( mca.run() == feasible , "MinCostAssignment: wrong feasibility" );
 if( ! feasible ) {
  return;
  }
 check( mca.totalCost() == ns.totalCost() , "MinCostAssignment: not optimal" );
 checkMatching( g , mca.matchingMap() , nr );
 int total = 0;
 for( EdgeIt e( g ) ; e != INVALID ; ++e ) {
  int slack = cost[ e ] - mca.potential( g.blueNode( e ) ) +
	      mca.potential( g.redNode( e ) );
  check( slack >= 0 , "MinCostAssignment: a reduced cost is negative" );
  if( mca.matching( e ) ) {
   check( slack == 0 , "MinCostAssignment: a matched edge has slack" );
   total += cost[ e ];
   }
  }
 check( total == mca.totalCost() , "MinCostAssignment: wrong total cost" );
 // the unmatched blue nodes have the same potential, the largest one
 bool first = true;
 int free = 0 , most = 0;
 for( BlueNodeIt b( g ) ; b != INVALID ; ++b ) {
  if( first || mca.potential( b ) > most ) {
   most = mca.potential( b );
   }
  first = false;
  }
 for( BlueNodeIt b( g ) ; b != INVALID ; ++b ) {
  if( mca.matching( b ) == INVALID ) {
   check( mca.potential( b ) == most ,
	  "MinCostAssignment: an unmatched blue node is not the highest" );
   ++free;
   }
  }
 check( free == countBlueNodes( g ) - nr , "MinCostAssignment: wrong size" );
 }

int main()
{
 rnd.seed( 20261103 );
 const int sizes[][ 2 ] = { { 0 , 0 } ,   { 1 , 0 } ,  { 0 , 1 } ,   { 1 ,
   1 } ,
			    { 3 , 5 } ,   { 5 , 3 } ,  { 10 , 10 } , { 20 ,
			    35 } ,
			    { 40 , 25 } , { 60 , 60 } };
 const double densities[] = { .02 , .1 , .3 , .7 };
 for( const auto & size : sizes ) {
  for( double p : densities ) {
   for( int k = 0 ; k < 3 ; ++k ) {
    BpGraph g;
    IntEdgeMap cost( g );
    randomGraph( g , cost , size[ 0 ] , size[ 1 ] , p );
    checkHopcroftKarp( g );
    checkAssignment( g , cost );
    }
   }
  }
 return( 0 );
 }
