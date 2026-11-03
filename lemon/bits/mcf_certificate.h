/* -*- mode: C++; indent-tabs-mode: nil; -*-
 *
 * This file is a part of LEMON, a generic C++ optimization library.
 *
 * Copyright (C) 2026 the authors of lemon-ng.
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

#ifndef LEMON_BITS_MCF_CERTIFICATE_H
 #define LEMON_BITS_MCF_CERTIFICATE_H

#include <algorithm>
#include <cmath>
#include <limits>
#include <type_traits>
#include <vector>

#include <lemon/preflow.h>
#include <lemon/smart_graph.h>
#include <lemon/tolerance.h>

//\ingroup min_cost_flow_algs
//\file
//\brief The certificates of an infeasible and of an unbounded minimum
//cost flow problem, which the four minimum cost flow algorithms give.

namespace lemon
{

namespace _mcf_bits
{

// A minimum cost flow problem as a list of arcs, which each algorithm
// copies out of its own data: the arcs go from source[ i ] to
// target[ i ] between lower[ i ] and upper[ i ], the latter being
// infinite if inf[ i ], and the nodes are 0, ..., n - 1.
template< typename V >
struct McfData
{
 int n;
 std::vector< int > source , target;
 std::vector< V > lower , upper , supply;
 std::vector< char > inf;
 std::vector< int > ref;

 McfData() : n( 0 ) {}

 void addArc( int s , int t , V l , V u , bool i , int r = -1 ) {
  ref.push_back( r );
  source.push_back( s );
  target.push_back( t );
  lower.push_back( l );
  upper.push_back( u );
  inf.push_back( i ? 1 : 0 );
  }
 };

// Whether x is larger than zero by more than the round-off of a sum
// whose terms have absolute values that sum to scale: exactly for an
// integer type, beyond the epsilon of Tolerance otherwise.
template< typename V >
bool positiveBeyond( long double x , long double , std::true_type )
{
 return( x > 0 );
 }

template< typename V >
bool positiveBeyond( long double x , long double scale , std::false_type )
{
 return( x > static_cast< long double >( Tolerance< V >::defaultEpsilon() ) *
	   ( 1 + scale ) );
 }

template< typename V >
bool positiveBeyond( long double x , long double scale )
{
 return( positiveBeyond< V >(
   x , scale ,
   std::integral_constant< bool ,
			   std::numeric_limits< V >::is_integer >() ) );
 }

// A set N' of nodes that proves the problem infeasible. Under GEQ the
// supply of N' exceeds what can leave it, i.e.,
//
//   sum_{ v in N' } sup( v ) > sum_{ uv out of N' } upper( uv )
//                            - sum_{ uv into N' } lower( uv ) ,
//
// and under LEQ (leq) the demand of N' exceeds what can enter it, i.e.,
// the same with the arcs taken backwards and the supplies negated. No arc
// out of N' (into it under LEQ) has an infinite upper bound. The set
// comes from a minimum cut of the flow that takes the supplies, net of
// the lower bounds, from a source to the nodes and the demands from the
// nodes to a sink (Gale, 1957; Hoffman, 1960): if it falls short of the
// supplies, the side of the source is such a set. It is empty if no set
// is found (e.g., the problem is feasible), and it is checked before it
// is given, so that what round-off in floating point makes a mistake of
// is not given at all.
template< typename V >
std::vector< int > infeasibleCut( const McfData< V > & d , bool leq )
{
 typedef SmartDigraph::Node Node;
 typedef SmartDigraph::Arc Arc;
 const int n = d.n , m = int( d.source.size() );
 std::vector< int > cut;
 if( n == 0 )
  return( cut );

 // Under LEQ, the arcs backwards and the supplies negated
 const std::vector< int > & from = leq ? d.target : d.source;
 const std::vector< int > & to = leq ? d.source : d.target;
 std::vector< long double > sup( n );
 for( int u = 0 ; u != n ; ++u )
  sup[ u ] = leq ? -static_cast< long double >( d.supply[ u ] )
		 : static_cast< long double >( d.supply[ u ] );
 for( int i = 0 ; i != m ; ++i ) {
  sup[ from[ i ] ] -= d.lower[ i ];
  sup[ to[ i ] ] += d.lower[ i ];
  }

 long double pos = 0;
 for( int u = 0 ; u != n ; ++u )
  if( sup[ u ] > 0 )
   pos += sup[ u ];
 // A capacity that no minimum cut below pos may contain, in place of
 // an infinite one
 const long double MAXV =
   static_cast< long double >( ( std::numeric_limits< V >::max )() );
 const V big = static_cast< V >( pos + 1 < MAXV ? pos + 1 : MAXV );

 SmartDigraph g;
 g.reserveNode( n + 2 );
 for( int u = 0 ; u != n + 2 ; ++u )
  g.addNode();
 const Node s = g.nodeFromId( n ) , t = g.nodeFromId( n + 1 );
 SmartDigraph::ArcMap< V > cap( g );
 for( int i = 0 ; i != m ; ++i ) {
  if( from[ i ] == to[ i ] )
   continue;
  const V c = d.inf[ i ] ? big : static_cast< V >( d.upper[ i ] -
						   d.lower[ i ] );
  if( c > 0 ) {
   const Arc a = g.addArc( g.nodeFromId( from[ i ] ) ,
			   g.nodeFromId( to[ i ] ) );
   cap.set( a , c );
   }
  }
 for( int u = 0 ; u != n ; ++u ) {
  if( sup[ u ] > 0 )
   cap.set( g.addArc( s , g.nodeFromId( u ) ) , static_cast< V >( sup[ u ] ) );
  else if( sup[ u ] < 0 )
   cap.set( g.addArc( g.nodeFromId( u ) , t ) ,
	    static_cast< V >( -sup[ u ] ) );
  }

 Preflow< SmartDigraph , SmartDigraph::ArcMap< V > > pf( g , cap , s , t );
 pf.runMinCut();
 std::vector< char > in( n , 0 );
 for( int u = 0 ; u != n ; ++u )
  if( pf.minCut( g.nodeFromId( u ) ) ) {
   in[ u ] = 1;
   cut.push_back( u );
   }

 // The check of the inequality, on the data as they were given
 long double gap = 0 , scale = 0;
 for( int u = 0 ; u != n ; ++u )
  if( in[ u ] ) {
   const long double x =
     leq ? -static_cast< long double >( d.supply[ u ] ) : d.supply[ u ];
   gap += x;
   scale += std::abs( x );
   }
 for( int i = 0 ; i != m ; ++i ) {
  if( in[ from[ i ] ] && ! in[ to[ i ] ] ) {
   if( d.inf[ i ] )
    return( std::vector< int >() );
   gap -= d.upper[ i ];
   scale += std::abs( static_cast< long double >( d.upper[ i ] ) );
   }
  else if( ! in[ from[ i ] ] && in[ to[ i ] ] ) {
   gap += d.lower[ i ];
   scale += std::abs( static_cast< long double >( d.lower[ i ] ) );
   }
  }
 if( cut.empty() || ! positiveBeyond< V >( gap , scale ) )
  cut.clear();
 return( cut );
 }

// A directed cycle of negative cost made of arcs of infinite upper bound,
// as the indices of its arcs in the direction of the flow, which proves
// a feasible problem unbounded: Bellman-Ford from a virtual node joined
// to all the others, on those arcs only, and the cycle of predecessors
// that a node still improved after as many rounds as there are nodes
// lies on. It is empty if there is none, and in floating point if its
// cost is not negative beyond the round-off.
template< typename V , typename CV >
std::vector< int > negativeCycle( const McfData< V > & d , const CV & cost )
{
 typedef typename CV::value_type Cost;
 const int n = d.n , m = int( d.source.size() );
 std::vector< int > cycle;
 const Tolerance< Cost > tol;
 std::vector< Cost > dist( n , Cost( 0 ) );
 std::vector< int > pred( n , -1 ) , mark( n );
 int v = -1;
 for( int round = 0 ; v < 0 ; ++round ) {
  bool changed = false;
  for( int i = 0 ; i != m ; ++i ) {
   if( ! d.inf[ i ] )
    continue;
   const int a = d.source[ i ] , b = d.target[ i ];
   if( tol.less( dist[ a ] + cost[ i ] , dist[ b ] ) ) {
    dist[ b ] = dist[ a ] + cost[ i ];
    pred[ b ] = i;
    changed = true;
    }
   }
  if( ! changed || round > 4 * n + 4 )
   return( cycle );
  // A cycle of the predecessors, which every node has at most one of,
  // is one of negative cost (Tarjan, 1983)
  std::fill( mark.begin() , mark.end() , -1 );
  for( int r = 0 ; r != n && v < 0 ; ++r ) {
   int u = r;
   while( u >= 0 && mark[ u ] < 0 ) {
    mark[ u ] = r;
    u = pred[ u ] < 0 ? -1 : d.source[ pred[ u ] ];
    }
   if( u >= 0 && mark[ u ] == r )
    v = u;
   }
  }

 long double c = 0 , scale = 0;
 for( int u = v ; ; ) {
  const int i = pred[ u ];
  cycle.push_back( i );
  c += cost[ i ];
  scale += std::abs( static_cast< long double >( cost[ i ] ) );
  u = d.source[ i ];
  if( u == v )
   break;
  }
 std::reverse( cycle.begin() , cycle.end() );
 if( ! positiveBeyond< Cost >( -c , scale ) )
  cycle.clear();
 return( cycle );
 }

// The problem as it was given to CostScaling, CapacityScaling or
// CycleCanceling, out of their residual network: the forward arcs of the
// digraph, ref being the index of each in it, those in inf_upper having
// an infinite upper bound that init() replaced by a finite one.
template< typename MCF >
McfData< typename MCF::Value > residualData( const MCF & mcf ,
					     const std::vector< int > &
					     inf_upper )
{
 typedef typename MCF::Value Value;
 const Value MAX = ( std::numeric_limits< Value >::max )();
 std::vector< char > was_inf( mcf._res_arc_num , 0 );
 for( std::size_t k = 0 ; k != inf_upper.size() ; ++k )
  was_inf[ inf_upper[ k ] ] = 1;
 McfData< Value > d;
 d.n = mcf._root;
 d.supply.assign( mcf._supply.begin() , mcf._supply.begin() + d.n );
 for( int u = 0 ; u != d.n ; ++u )
  for( int j = mcf._first_out[ u ] ; j != mcf._first_out[ u + 1 ] ; ++j ) {
   if( ! mcf._forward[ j ] || mcf._target[ j ] == mcf._root )
    continue;
   d.addArc( u , mcf._target[ j ] ,
	     mcf._has_lower ? mcf._lower[ j ] : Value( 0 ) , mcf._upper[ j ] ,
	     was_inf[ j ] || mcf._upper[ j ] >= MAX , j );
   }
 return( d );
 }

// The cycle of negativeCycle() as arcs of the digraph of MCF, given its
// residual data and the costs of its residual arcs.
template< typename MCF , typename CV >
std::vector< typename MCF::Digraph::Arc >
residualCycle( const MCF & mcf , const McfData< typename MCF::Value > & d ,
	       const CV & res_cost )
{
 typedef typename MCF::Digraph::Arc Arc;
 std::vector< typename CV::value_type > cost( d.ref.size() );
 for( std::size_t i = 0 ; i != cost.size() ; ++i )
  cost[ i ] = res_cost[ d.ref[ i ] ];
 const std::vector< int > ids = negativeCycle( d , cost );
 std::vector< Arc > arc_of( mcf._res_arc_num , INVALID ) , cycle;
 for( typename MCF::Digraph::ArcIt a( mcf._graph ) ; a != INVALID ; ++a )
  arc_of[ mcf._arc_idf[ a ] ] = a;
 for( std::size_t i = 0 ; i != ids.size() ; ++i )
  cycle.push_back( arc_of[ d.ref[ ids[ i ] ] ] );
 return( cycle );
 }

// The set of infeasibleCut() as nodes of the digraph of MCF, which has
// the GEQ supply type.
template< typename MCF >
std::vector< typename MCF::Digraph::Node >
residualCut( const MCF & mcf , const McfData< typename MCF::Value > & d )
{
 typedef typename MCF::Digraph::Node Node;
 const std::vector< int > ids = infeasibleCut( d , false );
 std::vector< Node > node_of( d.n , INVALID ) , cut;
 for( typename MCF::Digraph::NodeIt n( mcf._graph ) ; n != INVALID ; ++n )
  node_of[ mcf._node_id[ n ] ] = n;
 for( std::size_t i = 0 ; i != ids.size() ; ++i )
  cut.push_back( node_of[ ids[ i ] ] );
 return( cut );
 }

}

}

#endif
