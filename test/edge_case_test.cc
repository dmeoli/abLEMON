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

// The algorithms on the smallest graphs: the empty one, one node, a loop,
// parallel arcs and arcs of zero capacity, whose results are known

#include <limits>
#include <vector>

#include <lemon/bellman_ford.h>
#include <lemon/bfs.h>
#include <lemon/capacity_scaling.h>
#include <lemon/circulation.h>
#include <lemon/connectivity.h>
#include <lemon/cost_scaling.h>
#include <lemon/cycle_canceling.h>
#include <lemon/dfs.h>
#include <lemon/dijkstra.h>
#include <lemon/edmonds_karp.h>
#include <lemon/fractional_matching.h>
#include <lemon/hao_orlin.h>
#include <lemon/hartmann_orlin_mmc.h>
#include <lemon/howard_mmc.h>
#include <lemon/karp_mmc.h>
#include <lemon/kruskal.h>
#include <lemon/list_graph.h>
#include <lemon/matching.h>
#include <lemon/min_cost_arborescence.h>
#include <lemon/nagamochi_ibaraki.h>
#include <lemon/network_simplex.h>
#include <lemon/preflow.h>
#include <lemon/suurballe.h>

#include "test_tools.h"

using namespace lemon;

typedef ListDigraph Digraph;
typedef Digraph::ArcMap< int > IntArcMap;
typedef Digraph::NodeMap< int > IntNodeMap;

// a digraph with n nodes and its arcs, each with its lower bound, upper
// bound and cost
struct Case
{
 const char * name;
 int n;
 std::vector< std::pair< int , int > > arcs;
 std::vector< int > lower , upper , cost , supply;
 int type;     // 0 infeasible, 1 optimal, 2 unbounded
 long total;   // the optimal cost
 bool cycle;   // whether there is a cycle
 double mean;  // the minimum mean of a cycle
 int flow;     // the maximum flow from node 0 to node 1
 };

template< class MCF >
void checkMcf( const Case & c , const Digraph & g , const IntArcMap & lo ,
	       const IntArcMap & up , const IntArcMap & cost ,
	       const IntNodeMap & sup )
{
 MCF mcf( g );
 mcf.lowerMap( lo ).upperMap( up ).costMap( cost ).supplyMap( sup );
 int type = mcf.run();
 check( type == c.type , c.name );
 if( type == MCF::OPTIMAL ) {
  check( mcf.template totalCost< long >() == c.total , c.name );
  }
 }

template< class MMC >
void checkMmc( const Case & c , const Digraph & g , const IntArcMap & cost )
{
 MMC mmc( g , cost );
 bool found = mmc.findCycleMean();
 check( found == c.cycle , c.name );
 if( found ) {
  check( mmc.cycleMean() == c.mean , c.name );
  check( mmc.findCycle() && mmc.cycle().length() > 0 , c.name );
  }
 }

void checkDigraph( const Case & c )
{
 Digraph g;
 std::vector< Digraph::Node > v;
 for( int i = 0 ; i < c.n ; ++i ) {
  v.push_back( g.addNode() );
  }
 IntArcMap lo( g ) , up( g ) , cost( g );
 IntNodeMap sup( g , 0 );
 for( std::size_t i = 0 ; i < c.arcs.size() ; ++i ) {
  Digraph::Arc a = g.addArc( v[ c.arcs[ i ].first ] ,
			     v[ c.arcs[ i ].second ] );
  lo[ a ] = c.lower[ i ];
  up[ a ] = c.upper[ i ];
  cost[ a ] = c.cost[ i ];
  }
 for( std::size_t i = 0 ; i < c.supply.size() ; ++i ) {
  sup[ v[ i ] ] = c.supply[ i ];
  }

 checkMcf< NetworkSimplex< Digraph > >( c , g , lo , up , cost , sup );
 checkMcf< CostScaling< Digraph > >( c , g , lo , up , cost , sup );
 checkMcf< CapacityScaling< Digraph > >( c , g , lo , up , cost , sup );
 checkMcf< CycleCanceling< Digraph > >( c , g , lo , up , cost , sup );

 checkMmc< HowardMmc< Digraph , IntArcMap > >( c , g , cost );
 checkMmc< KarpMmc< Digraph , IntArcMap > >( c , g , cost );
 checkMmc< HartmannOrlinMmc< Digraph , IntArcMap > >( c , g , cost );

 Circulation< Digraph , IntArcMap , IntArcMap , IntNodeMap > circ( g , lo ,
								   up ,
								sup );
 check( circ.run() == ( c.type != 0 ) , c.name );

 if( c.n >= 1 ) {
  Bfs< Digraph > bfs( g );
  bfs.run( v[ 0 ] );
  check( bfs.dist( v[ 0 ] ) == 0 , c.name );
  Dfs< Digraph > dfs( g );
  dfs.run( v[ 0 ] );
  check( dfs.reached( v[ 0 ] ) , c.name );
  BellmanFord< Digraph , IntArcMap > bf( g , cost );
  bf.run( v[ 0 ] );
  check( bf.reached( v[ 0 ] ) , c.name );
  MinCostArborescence< Digraph , IntArcMap > mca( g , cost );
  mca.run( v[ 0 ] );
  check( c.n > 1 || mca.arborescenceCost() == 0 , c.name );
  }
 if( c.n >= 2 ) {
  Preflow< Digraph , IntArcMap > pf( g , up , v[ 0 ] , v[ 1 ] );
  pf.run();
  check( pf.flowValue() == c.flow , c.name );
  EdmondsKarp< Digraph , IntArcMap > ek( g , up , v[ 0 ] , v[ 1 ] );
  ek.run();
  check( ek.flowValue() == c.flow , c.name );
  }
 }

void checkDigraphs()
{
 const int inf = std::numeric_limits< int >::max();
 Case cases[] = {
  { "empty digraph" , 0 , {} , {} , {} , {} , {} , 1 , 0 , false , 0 , 0 } ,
  { "one node" , 1 , {} , {} , {} , {} , {} , 1 , 0 , false , 0 , 0 } ,
  // a loop of negative cost is saturated
  { "loop of negative cost" ,
    1 ,
    { { 0 , 0 } } ,
    { 0 } ,
    { 4 } ,
    { -3 } ,
    {} ,
    1 ,
    -12 ,
    true ,
    -3 ,
    0 } ,
  // a loop with a lower bound carries it
  { "loop with a lower bound" ,
    1 ,
    { { 0 , 0 } } ,
    { 2 } ,
    { 4 } ,
    { 5 } ,
    {} ,
    1 ,
    10 ,
    true ,
    5 ,
    0 } ,
  { "loop of negative cost and infinite capacity" ,
    1 ,
    { { 0 , 0 } } ,
    { 0 } ,
    { inf } ,
    { -1 } ,
    {} ,
    2 ,
    0 ,
    true ,
    -1 ,
    0 } ,
  { "parallel arcs" ,
    2 ,
    { { 0 , 1 } , { 0 , 1 } , { 1 , 0 } } ,
    { 0 , 0 , 0 } ,
    { 3 , 0 , 2 } ,
    { 1 , 1 , 1 } ,
    { 2 , -2 } ,
    1 ,
    2 ,
    true ,
    1 ,
    3 } ,
  { "zero capacity" ,
    2 ,
    { { 0 , 1 } } ,
    { 0 } ,
    { 0 } ,
    { 1 } ,
    { 1 , -1 } ,
    0 ,
    0 ,
    false ,
    0 ,
    0 }
  };
 for( const Case & c : cases ) {
  checkDigraph( c );
  }

 // the strongly connected components and a minimum cut of the digraph
 Digraph g;
 check( countStronglyConnectedComponents( g ) == 0 , "empty digraph" );
 Digraph::Node s = g.addNode() , t = g.addNode();
 IntArcMap cap( g );
 cap[ g.addArc( s , t ) ] = 3;
 cap[ g.addArc( s , t ) ] = 0;
 cap[ g.addArc( t , s ) ] = 2;
 check( countStronglyConnectedComponents( g ) == 1 , "parallel arcs" );
 HaoOrlin< Digraph , IntArcMap > ho( g , cap );
 ho.run();
 check( ho.minCutValue() == 2 , "parallel arcs" );
 IntArcMap len( g , 1 );
 Suurballe< Digraph , IntArcMap > su( g , len );
 check( su.run( s , t , 2 ) == 2 , "parallel arcs" );
 }

// an undirected graph with n nodes and its weighted edges
void checkGraph( const char * name , int n ,
		 const std::vector< std::pair< int , int > > & edges ,
		 const std::vector< int > & weight , int matching ,
		 int covered ,
		 int mst , bool conn , bool perfect , int cut )
{
 typedef ListGraph Graph;
 Graph g;
 std::vector< Graph::Node > v;
 for( int i = 0 ; i < n ; ++i ) {
  v.push_back( g.addNode() );
  }
 Graph::EdgeMap< int > w( g );
 for( std::size_t i = 0 ; i < edges.size() ; ++i ) {
  w[ g.addEdge( v[ edges[ i ].first ] , v[ edges[ i ].second ] ) ] =
    weight[ i ];
  }

 MaxMatching< Graph > mm( g );
 mm.run();
 check( mm.matchingSize() == matching , name );
 MaxWeightedPerfectMatching< Graph , Graph::EdgeMap< int > > mwpm( g , w );
 check( mwpm.run() == perfect , name );
 MaxFractionalMatching< Graph > mfm( g );
 mfm.run();
 check( mfm.matchingSize() == covered , name );
 Graph::EdgeMap< bool > tree( g );
 check( kruskal( g , w , tree ) == mst , name );
 check( connected( g ) == conn , name );
 if( n >= 1 ) {
  NagamochiIbaraki< Graph , Graph::EdgeMap< int > > ni( g , w );
  ni.run();
  check( ni.minCutValue() == cut , name );
  }
 }

void checkGraphs()
{
 checkGraph( "empty graph" , 0 , {} , {} , 0 , 0 , 0 , true , true , 0 );
 checkGraph( "one node" , 1 , {} , {} , 0 , 0 , 0 , true , false , 0 );
 // a loop covers its node in a fractional matching only
 checkGraph( "loop" , 1 , { { 0 , 0 } } , { 5 } , 0 , 1 , 0 , true , false ,
	     0 );
 checkGraph( "parallel edges" , 2 , { { 0 , 1 } , { 0 , 1 } } , { 3 , 1 } , 1 ,
	     2 , 1 ,
	     true , true , 4 );
 checkGraph( "two isolated nodes" , 2 , {} , {} , 0 , 0 , 0 , false , false ,
	     0 );
 }

int main()
{
 checkDigraphs();
 checkGraphs();
 return( 0 );
 }
