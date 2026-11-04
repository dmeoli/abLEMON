/* -*- mode: C++; indent-tabs-mode: nil; -*-
 *
 * This file is a part of LEMON, a generic C++ optimization library.
 *
 * Copyright (C) 2026 the authors of abLEMON.
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

// KBestFlows against the enumeration of every integer flow of small
// random instances: the costs of the flows it gives are the smallest ones,
// in order, and the flows are feasible and all different; then the limit
// cases (no flow asked, an infeasible problem, fewer flows than asked, a
// given optimal flow, a floating-point type, an arc of infinite capacity)

#include <algorithm>
#include <limits>
#include <set>
#include <string>
#include <vector>

#include <lemon/smart_graph.h>
#include <lemon/cost_scaling.h>
#include <lemon/k_best_flows.h>

#include "test_tools.h"

using namespace lemon;

typedef SmartDigraph Digraph;
typedef Digraph::Node Node;
typedef Digraph::Arc Arc;

struct Small
{
 Digraph g;
 Digraph::ArcMap< int > lower , upper , cost;
 Digraph::NodeMap< int > supply;

 Small() : lower( g , 0 ) , upper( g , 0 ) , cost( g , 0 ) , supply( g , 0 ) {}
 };

// up to 4 nodes and 5 arcs (loops and parallel arcs among them), bounds
// at most 3 apart, costs of both signs, supplies that may leave slack
void randomSmall( Small & p , unsigned int & r )
{
 r = r * 1103515245 + 12345;
 const int n = 1 + ( r >> 16 ) % 4;
 for( int i = 0 ; i != n ; ++i )
  p.g.addNode();
 r = r * 1103515245 + 12345;
 const int m = 1 + ( r >> 16 ) % 5;
 for( int k = 0 ; k != m ; ++k ) {
  r = r * 1103515245 + 12345;
  const Arc a = p.g.addArc( p.g.nodeFromId( ( r >> 8 ) % n ) ,
			    p.g.nodeFromId( ( r >> 20 ) % n ) );
  r = r * 1103515245 + 12345;
  p.lower[ a ] = ( r >> 4 ) % 3 == 0 ? int( ( r >> 8 ) % 2 ) : 0;
  p.upper[ a ] = p.lower[ a ] + int( ( r >> 12 ) % 4 );
  p.cost[ a ] = int( ( r >> 16 ) % 9 ) - 3;
  }
 for( int i = 0 ; i != n ; ++i ) {
  r = r * 1103515245 + 12345;
  p.supply[ p.g.nodeFromId( i ) ] = int( ( r >> 16 ) % 5 ) - 2;
  }
 }

// The costs of all the integer flows of the instance, in order
std::vector< int > allCosts( const Small & p )
{
 const int m = countArcs( p.g ) , n = countNodes( p.g );
 std::vector< int > f( m ) , costs;
 for( int i = 0 ; i != m ; ++i )
  f[ i ] = p.lower[ p.g.arcFromId( i ) ];
 for( ; ; ) {
  std::vector< int > net( n , 0 );
  int c = 0;
  for( int i = 0 ; i != m ; ++i ) {
   const Arc a = p.g.arcFromId( i );
   net[ p.g.id( p.g.source( a ) ) ] += f[ i ];
   net[ p.g.id( p.g.target( a ) ) ] -= f[ i ];
   c += p.cost[ a ] * f[ i ];
   }
  bool ok = true;
  for( int u = 0 ; u != n ; ++u )
   ok = ok && net[ u ] >= p.supply[ p.g.nodeFromId( u ) ];
  if( ok )
   costs.push_back( c );
  int i = 0;
  for( ; i != m ; ++i ) {
   const Arc a = p.g.arcFromId( i );
   if( f[ i ] < p.upper[ a ] ) {
    ++f[ i ];
    break;
    }
   f[ i ] = p.lower[ a ];
   }
  if( i == m )
   break;
  }
 std::sort( costs.begin() , costs.end() );
 return( costs );
 }

void checkAgainstEnumeration( int seeds )
{
 unsigned int r = 1995;
 for( int seed = 0 ; seed != seeds ; ++seed ) {
  Small p;
  randomSmall( p , r );
  const std::string id = "instance " + std::to_string( seed );
  const std::vector< int > all = allCosts( p );
  KBestFlows< Digraph > kb( p.g );
  kb.lowerMap( p.lower ).upperMap( p.upper ).costMap( p.cost ).supplyMap(
    p.supply );
  const int k = 12;
  const int got = kb.run( k );
  check( got == std::min( k , int( all.size() ) ) ,
	 "Wrong number of flows on " + id );
  std::set< std::vector< int > > seen;
  for( int j = 0 ; j != got ; ++j ) {
   check( kb.totalCost( j ) == all[ j ] , "Wrong cost of a flow on " + id );
   std::vector< int > f;
   std::vector< int > net( countNodes( p.g ) , 0 );
   int c = 0;
   for( Digraph::ArcIt a( p.g ) ; a != INVALID ; ++a ) {
    const int x = kb.flow( j , a );
    f.push_back( x );
    check( x >= p.lower[ a ] && x <= p.upper[ a ] ,
	   "Flow out of its bounds on " + id );
    net[ p.g.id( p.g.source( a ) ) ] += x;
    net[ p.g.id( p.g.target( a ) ) ] -= x;
    c += p.cost[ a ] * x;
    }
   for( Digraph::NodeIt u( p.g ) ; u != INVALID ; ++u )
    check( net[ p.g.id( u ) ] >= p.supply[ u ] , "Supply not met on " + id );
   check( c == kb.totalCost( j ) , "Cost not that of the flow on " + id );
   check( seen.insert( f ).second , "A flow given twice on " + id );
   }
  }
 }

void checkLimitCases()
{
 // no flow asked, and an infeasible problem
 {
  Small p;
  const Node u = p.g.addNode() , v = p.g.addNode();
  const Arc a = p.g.addArc( u , v );
  p.upper[ a ] = 1;
  p.supply[ u ] = 2;
  p.supply[ v ] = -2;
  KBestFlows< Digraph > kb( p.g );
  kb.upperMap( p.upper ).supplyMap( p.supply );
  check( kb.run( 0 ) == 0 , "Flows when none is asked" );
  check( kb.run( 5 ) == 0 , "Flows of an infeasible problem" );
  }

 // fewer flows than asked: two parallel arcs that carry 2 units, i.e.,
 // 0 + 2, 1 + 1, 2 + 0, of costs 4, 5, 6
 {
  Small p;
  const Node u = p.g.addNode() , v = p.g.addNode();
  const Arc a = p.g.addArc( u , v ) , b = p.g.addArc( u , v );
  p.upper[ a ] = p.upper[ b ] = 2;
  p.cost[ a ] = 3;
  p.cost[ b ] = 2;
  p.supply[ u ] = 2;
  p.supply[ v ] = -2;
  KBestFlows< Digraph > kb( p.g );
  kb.upperMap( p.upper ).costMap( p.cost ).supplyMap( p.supply );
  check( kb.run( 10 ) == 3 , "Wrong number of flows of two arcs" );
  check( kb.totalCost( 0 ) == 4 && kb.totalCost( 1 ) == 5 &&
	   kb.totalCost( 2 ) == 6 ,
	 "Wrong costs of the flows of two arcs" );
  }

 // the optimal flow of CostScaling given as the first one: the same
 // costs as from that of NetworkSimplex
 {
  unsigned int r = 7;
  Small p;
  randomSmall( p , r );
  CostScaling< Digraph > cs( p.g );
  cs.lowerMap( p.lower ).upperMap( p.upper ).costMap( p.cost ).supplyMap(
    p.supply );
  if( cs.run() == cs.OPTIMAL ) {
   Digraph::ArcMap< int > f( p.g );
   cs.flowMap( f );
   KBestFlows< Digraph > k1( p.g ) , k2( p.g );
   k1.lowerMap( p.lower ).upperMap( p.upper ).costMap( p.cost ).supplyMap(
     p.supply );
   k2.lowerMap( p.lower ).upperMap( p.upper ).costMap( p.cost ).supplyMap(
     p.supply ).initFlow( f );
   const int n1 = k1.run( 8 ) , n2 = k2.run( 8 );
   check( n1 == n2 , "Another number of flows from a given flow" );
   for( int j = 0 ; j != n1 && j != n2 ; ++j )
    check( k1.totalCost( j ) == k2.totalCost( j ) ,
	   "Other costs from a given flow" );
   }
  }

 // a floating-point type, and an arc of infinite capacity on a path that
 // the supply fixes, with a parallel arc of capacity 1: the flows are 2 on
 // the infinite arc, or 1 and 1
 {
  Digraph g;
  const Node u = g.addNode() , v = g.addNode();
  const Arc a = g.addArc( u , v ) , b = g.addArc( u , v );
  Digraph::ArcMap< double > upper( g ) , cost( g );
  Digraph::NodeMap< double > supply( g );
  upper[ a ] = std::numeric_limits< double >::infinity();
  upper[ b ] = 1;
  cost[ a ] = 1.5;
  cost[ b ] = 0.25;
  supply[ u ] = 2;
  supply[ v ] = -2;
  KBestFlows< Digraph , double > kb( g );
  kb.upperMap( upper ).costMap( cost ).supplyMap( supply );
  check( kb.run( 5 ) == 2 , "Wrong number of flows with an infinite arc" );
  check( kb.totalCost( 0 ) == 1.75 && kb.totalCost( 1 ) == 3 ,
	 "Wrong costs of the flows with an infinite arc" );
  check( kb.flow( 0 , a ) == 1 && kb.flow( 0 , b ) == 1 ,
	 "Wrong best flow with an infinite arc" );
  }
 }

int main()
{
 checkAgainstEnumeration( 2000 );
 checkLimitCases();
 return( 0 );
 }
