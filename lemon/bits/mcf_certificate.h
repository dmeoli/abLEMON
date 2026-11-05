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

#ifndef LEMON_BITS_MCF_CERTIFICATE_H
 #define LEMON_BITS_MCF_CERTIFICATE_H

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include <lemon/bits/mcf_tolerance.h>
#include <lemon/howard_mmc.h>
#include <lemon/path.h>
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
// infinite if inf[ i ], and the nodes are 0, ..., n - 1; feps and ceps
// are the tolerances of the algorithm on the flows and on the reduced costs
// [see bits/mcf_tolerance.h], negative if there are none.
template< typename V >
struct McfData
{
 int n;
 long double feps = -1 , ceps = -1;
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
// whose terms have absolute values that sum to scale and by more than the
// tolerance tol of the algorithm, if it is not negative: exactly for an
// integer type, beyond the larger of tol and the epsilon of Tolerance
// times the scale otherwise.
template< typename V >
bool positiveBeyond( long double x , long double , long double ,
		     std::true_type )
{
 return( x > 0 );
 }

template< typename V >
bool positiveBeyond( long double x , long double scale , long double tol ,
		     std::false_type )
{
 return( x > ( std::max )( tol , static_cast< long double >(
				   Tolerance< V >::defaultEpsilon() ) *
				   ( 1 + scale ) ) );
 }

template< typename V >
bool positiveBeyond( long double x , long double scale , long double tol = -1 )
{
 return( positiveBeyond< V >(
   x , scale , tol ,
   std::integral_constant< bool ,
			   std::numeric_limits< V >::is_integer >() ) );
 }

// The violation of the inequality of a set of nodes (in[ u ]) under GEQ,
//
//   sum_{ v in N' } sup( v ) - sum_{ uv out of N' } upper( uv )
//                            + sum_{ uv into N' } lower( uv ) ,
//
// and under LEQ (leq) the same with the arcs taken backwards and the
// supplies negated, on the data as they were given; in scale the sum of
// the absolute values of its terms. False if an arc out of the set (into
// it under LEQ) has an infinite upper bound, i.e., the set proves nothing.
template< typename V >
bool cutViolation( const McfData< V > & d , bool leq ,
		   const std::vector< char > & in , long double & gap ,
		   long double & scale )
{
 const std::vector< int > & from = leq ? d.target : d.source;
 const std::vector< int > & to = leq ? d.source : d.target;
 gap = scale = 0;
 for( int u = 0 ; u != d.n ; ++u )
  if( in[ u ] ) {
   const long double x =
     leq ? -static_cast< long double >( d.supply[ u ] ) : d.supply[ u ];
   gap += x;
   scale += std::abs( x );
   }
 for( int i = 0 ; i != int( d.source.size() ) ; ++i ) {
  if( in[ from[ i ] ] && ! in[ to[ i ] ] ) {
   if( d.inf[ i ] )
    return( false );
   gap -= d.upper[ i ];
   scale += std::abs( static_cast< long double >( d.upper[ i ] ) );
   }
  else if( ! in[ from[ i ] ] && in[ to[ i ] ] ) {
   gap += d.lower[ i ];
   scale += std::abs( static_cast< long double >( d.lower[ i ] ) );
   }
  }
 return( true );
 }

// The set of nodes of largest violation [see cutViolation()] among those
// that contain the nodes u with fix[ u ] == 1 and none of those with
// fix[ u ] == 0 (fix[ u ] < 0 leaves u free): the side of the source of a
// minimum cut of the flow that takes the supplies, net of the lower
// bounds, from a source to the nodes and the demands from the nodes to a
// sink (Gale, 1957; Hoffman, 1960), whose capacity is the sum of the
// positive supplies less the violation of the side, the nodes fixed in or
// out by arcs from the source or to the sink that no minimum cut may
// contain. In in, the characteristic vector of the set.
template< typename V >
void maxViolatedSet( const McfData< V > & d , bool leq ,
		     const std::vector< int > & fix , std::vector< char > & in ,
		     const std::vector< V > * warm = nullptr ,
		     std::vector< V > * flow = nullptr )
{
 typedef SmartDigraph::Node Node;
 const int n = d.n , m = int( d.source.size() );

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
 // A capacity that no minimum cut below pos may contain, in place of an
 // infinite one, and a larger one for the arcs that fix the nodes
 const long double MAXV =
   static_cast< long double >( ( std::numeric_limits< V >::max )() );
 const V big = static_cast< V >( pos + 1 < MAXV ? pos + 1 : MAXV );
 const V fixed = static_cast< V >( 2 * pos + 3 < MAXV ? 2 * pos + 3 : MAXV );

 // The arcs are the m of the problem, then one from the source and one
 // to the sink for each node, whatever the data and the fixed nodes, so
 // that the flow of a part is a preflow of each of its children, whose
 // capacities are only larger
 SmartDigraph g;
 g.reserveNode( n + 2 );
 g.reserveArc( m + 2 * n );
 for( int u = 0 ; u != n + 2 ; ++u )
  g.addNode();
 const Node s = g.nodeFromId( n ) , t = g.nodeFromId( n + 1 );
 SmartDigraph::ArcMap< V > cap( g );
 for( int i = 0 ; i != m ; ++i ) {
  const V c = from[ i ] == to[ i ]
		? V( 0 )
		: d.inf[ i ] ? big
			     : static_cast< V >( d.upper[ i ] - d.lower[ i ] );
  cap.set( g.addArc( g.nodeFromId( from[ i ] ) , g.nodeFromId( to[ i ] ) ) ,
	   c > 0 ? c : V( 0 ) );
  }
 for( int u = 0 ; u != n ; ++u ) {
  V cs = sup[ u ] > 0 ? static_cast< V >( sup[ u ] ) : V( 0 );
  V ct = sup[ u ] < 0 ? static_cast< V >( -sup[ u ] ) : V( 0 );
  if( ! fix.empty() && fix[ u ] == 1 )
   cs += fixed;
  else if( ! fix.empty() && fix[ u ] == 0 )
   ct += fixed;
  cap.set( g.addArc( s , g.nodeFromId( u ) ) , cs );
  cap.set( g.addArc( g.nodeFromId( u ) , t ) , ct );
  }

 Preflow< SmartDigraph , SmartDigraph::ArcMap< V > > pf( g , cap , s , t );
 if( d.feps >= 0 )
  pf.tolerance( makeTolerance( static_cast< V >( d.feps ) ) );
 bool started = false;
 if( warm != nullptr && int( warm->size() ) == m + 2 * n ) {
  SmartDigraph::ArcMap< V > f0( g );
  for( int i = 0 ; i != m + 2 * n ; ++i )
   f0.set( g.arcFromId( i ) , ( *warm )[ i ] );
  started = pf.init( f0 );
  }
 if( ! started )
  pf.init();
 pf.startFirstPhase();
 in.assign( n , 0 );
 for( int u = 0 ; u != n ; ++u )
  in[ u ] = pf.minCut( g.nodeFromId( u ) ) ? 1 : 0;
 if( flow != nullptr ) {
  flow->resize( m + 2 * n );
  for( int i = 0 ; i != m + 2 * n ; ++i )
   ( *flow )[ i ] = pf.flowMap()[ g.arcFromId( i ) ];
  }
 }

// A set N' of nodes that proves the problem infeasible. Under GEQ the
// supply of N' exceeds what can leave it, i.e., its violation [see
// cutViolation()] is positive, and under LEQ (leq) the demand of N'
// exceeds what can enter it. No arc out of N' (into it under LEQ) has an
// infinite upper bound. It is hint, the set that the algorithm had at
// hand when it found no feasible flow, if that proves it, and the set of
// largest violation [see maxViolatedSet()] otherwise; it is empty if no
// set is found (e.g., the problem is feasible), and the inequality is
// checked before it is given, so that what round-off in floating point
// makes a mistake of is not given at all.
template< typename V >
std::vector< int > infeasibleCut( const McfData< V > & d , bool leq ,
				  const std::vector< int > & hint =
				    std::vector< int >() )
{
 std::vector< int > cut;
 if( d.n == 0 )
  return( cut );
 long double gap , scale;
 std::vector< char > in( d.n , 0 );
 if( ! hint.empty() ) {
  for( std::size_t h = 0 ; h != hint.size() ; ++h )
   in[ hint[ h ] ] = 1;
  if( cutViolation( d , leq , in , gap , scale ) &&
      positiveBeyond< V >( gap , scale , d.feps ) )
   return( hint );
  }
 maxViolatedSet( d , leq , std::vector< int >() , in );
 if( ! cutViolation( d , leq , in , gap , scale ) ||
     ! positiveBeyond< V >( gap , scale , d.feps ) )
  return( cut );
 for( int u = 0 ; u != d.n ; ++u )
  if( in[ u ] )
   cut.push_back( u );
 return( cut );
 }

// The k sets of nodes of largest violation [see cutViolation()], in the
// order of their violations, those that prove the problem infeasible:
// Lawler's partition of the sets by the nodes that they contain or not
// (Hamacher, Picard and Queyranne, 1984), i.e., once the best set of a
// part, in which some nodes are fixed in or out, is given, the part is
// split by the free nodes in turn, the j-th child keeping the first j - 1
// of them as in the set and taking the j-th one out if the set has it,
// in otherwise. The best set of a child is no more violated than that of
// its part, so a child waits with the violation of its part as a bound and
// its best set is found by maxViolatedSet() only when it is the most
// promising one, from the maximum flow of its part, which is a preflow of
// the child since the child only fixes one node more: a maximum flow is
// made for each part that might hold one of the k sets, rather than for
// every child of every set given.
template< typename V >
std::vector< std::vector< int > > infeasibleCuts( const McfData< V > & d ,
						  bool leq , int k )
{
 struct Part
 {
  std::vector< int > fix;
  std::vector< char > in;
  std::shared_ptr< const std::vector< V > > warm , flow;
  long double gap , scale;
  bool solved , ok;
  };
 std::vector< std::vector< int > > cuts;
 if( d.n == 0 || k <= 0 )
  return( cuts );
 auto solve = [ & ]( Part & p ) {
  std::vector< V > f;
  maxViolatedSet( d , leq , p.fix , p.in , p.warm.get() , &f );
  p.flow = std::make_shared< const std::vector< V > >( std::move( f ) );
  p.warm.reset();
  bool kept = true;
  for( int w = 0 ; w != d.n && kept ; ++w )
   if( p.fix[ w ] >= 0 && p.in[ w ] != p.fix[ w ] )
    kept = false;
  long double gap = 0 , scale = 1;
  p.ok = kept && cutViolation( d , leq , p.in , gap , scale ) &&
	 positiveBeyond< V >( gap , scale , d.feps );
  p.gap = gap;
  p.scale = scale;
  p.solved = true;
  };
 std::vector< Part > parts( 1 );
 parts[ 0 ].fix.assign( d.n , -1 );
 solve( parts[ 0 ] );
 while( int( cuts.size() ) < k ) {
  int best = -1;
  for( int j = 0 ; j != int( parts.size() ) ; ++j )
   if( ( ! parts[ j ].solved || parts[ j ].ok ) &&
       ( best < 0 || parts[ j ].gap > parts[ best ].gap ||
	 ( parts[ j ].gap == parts[ best ].gap && parts[ j ].solved &&
	   ! parts[ best ].solved ) ) )
    best = j;
  if( best < 0 )
   break;
  if( ! parts[ best ].solved ) {
   solve( parts[ best ] );
   continue;
   }
  const Part p = parts[ best ];
  parts.erase( parts.begin() + best );
  std::vector< int > cut;
  for( int u = 0 ; u != d.n ; ++u )
   if( p.in[ u ] )
    cut.push_back( u );
  cuts.push_back( cut );

  std::vector< int > fix = p.fix;
  for( int u = 0 ; u != d.n ; ++u ) {
   if( fix[ u ] >= 0 )
    continue;
   Part c;
   c.fix = fix;
   c.fix[ u ] = p.in[ u ] ? 0 : 1;
   c.warm = p.flow;
   c.gap = p.gap;
   c.scale = p.scale;
   c.solved = false;
   c.ok = false;
   parts.push_back( c );
   fix[ u ] = p.in[ u ];
   }
  }
 return( cuts );
 }

// Sets of nodes that prove the problem infeasible and are pairwise
// disjoint, with no arc between any two of them: the weakly connected
// components of the set of infeasibleCut() whose violation is positive,
// in the order of their violations, which sum to at least that of the
// set, since no arc joins two of them.
template< typename V >
std::vector< std::vector< int > > disjointInfeasibleCuts(
  const McfData< V > & d , bool leq ,
  const std::vector< int > & hint = std::vector< int >() )
{
 std::vector< std::vector< int > > cuts;
 const std::vector< int > cut = infeasibleCut( d , leq , hint );
 if( cut.empty() )
  return( cuts );
 std::vector< int > comp( d.n , -1 ) , par( d.n );
 for( int u = 0 ; u != d.n ; ++u )
  par[ u ] = u;
 std::vector< char > in( d.n , 0 );
 for( std::size_t h = 0 ; h != cut.size() ; ++h )
  in[ cut[ h ] ] = 1;
 struct Find
 {
  std::vector< int > & p;
  int operator()( int u ) {
   while( p[ u ] != u )
    u = p[ u ] = p[ p[ u ] ];
   return( u );
   }
  } find = { par };
 for( int i = 0 ; i != int( d.source.size() ) ; ++i )
  if( in[ d.source[ i ] ] && in[ d.target[ i ] ] )
   par[ find( d.source[ i ] ) ] = find( d.target[ i ] );
 std::vector< std::vector< int > > groups;
 for( std::size_t h = 0 ; h != cut.size() ; ++h ) {
  const int r = find( cut[ h ] );
  if( comp[ r ] < 0 ) {
   comp[ r ] = int( groups.size() );
   groups.push_back( std::vector< int >() );
   }
  groups[ comp[ r ] ].push_back( cut[ h ] );
  }
 std::vector< std::pair< long double , int > > order;
 for( int j = 0 ; j != int( groups.size() ) ; ++j ) {
  std::vector< char > one( d.n , 0 );
  for( std::size_t h = 0 ; h != groups[ j ].size() ; ++h )
   one[ groups[ j ][ h ] ] = 1;
  long double gap , scale;
  if( cutViolation( d , leq , one , gap , scale ) &&
      positiveBeyond< V >( gap , scale , d.feps ) )
   order.push_back( std::make_pair( -gap , j ) );
  }
 std::sort( order.begin() , order.end() );
 for( std::size_t h = 0 ; h != order.size() ; ++h )
  cuts.push_back( groups[ order[ h ].second ] );
 return( cuts );
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
 const Tolerance< Cost > tol =
   d.ceps >= 0 ? makeTolerance( static_cast< Cost >( d.ceps ) )
	       : Tolerance< Cost >();
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
 if( ! positiveBeyond< Cost >( -c , scale , d.ceps ) )
  cycle.clear();
 return( cycle );
 }

// Directed cycles of negative cost made of arcs of infinite upper bound,
// pairwise arc-disjoint, at most k of them, as the indices of their arcs
// in the direction of the flow: the cycle of minimum mean cost (Howard's
// algorithm) among the arcs of infinite upper bound, then that among
// those not yet taken, and so on while the mean is negative beyond the
// round-off. The most negative cycle would be NP-hard to find (it would
// decide whether a digraph is Hamiltonian); the minimum mean one is the
// most negative per arc, and the order is that of the means.
template< typename V , typename CV >
std::vector< std::vector< int > > negativeCycles( const McfData< V > & d ,
						  const CV & cost , int k )
{
 typedef typename CV::value_type Cost;
 typedef SmartDigraph::Arc Arc;
 std::vector< std::vector< int > > cycles;
 const int m = int( d.source.size() );
 std::vector< char > taken( m , 0 );
 while( int( cycles.size() ) < k ) {
  SmartDigraph g;
  g.reserveNode( d.n );
  for( int u = 0 ; u != d.n ; ++u )
   g.addNode();
  std::vector< int > orig;
  SmartDigraph::ArcMap< Cost > len( g );
  long double scale = 0;
  for( int i = 0 ; i != m ; ++i )
   if( d.inf[ i ] && ! taken[ i ] ) {
    const Arc a = g.addArc( g.nodeFromId( d.source[ i ] ) ,
			    g.nodeFromId( d.target[ i ] ) );
    len[ a ] = cost[ i ];
    orig.push_back( i );
    scale = ( std::max )( scale ,
			  std::abs( static_cast< long double >( cost[ i ] ) ) );
    }
  if( orig.empty() )
   break;
  Path< SmartDigraph > path;
  typedef HowardMmc< SmartDigraph , SmartDigraph::ArcMap< Cost > > Mmc;
  Mmc mmc( g , len );
  if( d.ceps >= 0 )
   mmc.tolerance(
     makeTolerance( static_cast< typename Mmc::LargeCost >( d.ceps ) ) );
  mmc.cycle( path );
  if( ! mmc.findCycleMean() ||
      ! positiveBeyond< Cost >(
	-static_cast< long double >( mmc.cycleCost() ) / mmc.cycleSize() ,
	scale , d.ceps ) ||
      ! mmc.findCycle() )
   break;
  std::vector< int > cycle;
  for( typename Path< SmartDigraph >::ArcIt a( path ) ; a != INVALID ; ++a ) {
   const int i = orig[ g.id( Arc( a ) ) ];
   cycle.push_back( i );
   taken[ i ] = 1;
   }
  cycles.push_back( cycle );
  }
 return( cycles );
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
 d.feps = epsilonOf( mcf.flowTolerance() );
 d.ceps = epsilonOf( mcf.costTolerance() );
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
 const std::vector< int > ids =
   infeasibleCut( d , false , mcf._cut_hint );
 std::vector< Node > node_of( d.n , INVALID ) , cut;
 for( typename MCF::Digraph::NodeIt n( mcf._graph ) ; n != INVALID ; ++n )
  node_of[ mcf._node_id[ n ] ] = n;
 for( std::size_t i = 0 ; i != ids.size() ; ++i )
  cut.push_back( node_of[ ids[ i ] ] );
 return( cut );
 }

// The costs of the arcs of the residual data d, out of the costs of the
// residual arcs of the algorithm.
template< typename V , typename CV >
std::vector< typename CV::value_type > residualCosts( const McfData< V > & d ,
						      const CV & res_cost )
{
 std::vector< typename CV::value_type > cost( d.ref.size() );
 for( std::size_t i = 0 ; i != cost.size() ; ++i )
  cost[ i ] = res_cost[ d.ref[ i ] ];
 return( cost );
 }

// Sets of nodes of the residual data as nodes of the digraph of MCF.
template< typename MCF >
std::vector< std::vector< typename MCF::Digraph::Node > >
residualNodeSets( const MCF & mcf , int n ,
		  const std::vector< std::vector< int > > & ids )
{
 typedef typename MCF::Digraph::Node Node;
 std::vector< Node > node_of( n , INVALID );
 for( typename MCF::Digraph::NodeIt u( mcf._graph ) ; u != INVALID ; ++u )
  node_of[ mcf._node_id[ u ] ] = u;
 std::vector< std::vector< Node > > sets( ids.size() );
 for( std::size_t j = 0 ; j != ids.size() ; ++j )
  for( std::size_t h = 0 ; h != ids[ j ].size() ; ++h )
   sets[ j ].push_back( node_of[ ids[ j ][ h ] ] );
 return( sets );
 }

// Sets of arcs of the residual data d as arcs of the digraph of MCF.
template< typename MCF >
std::vector< std::vector< typename MCF::Digraph::Arc > >
residualArcSets( const MCF & mcf , const McfData< typename MCF::Value > & d ,
		 const std::vector< std::vector< int > > & ids )
{
 typedef typename MCF::Digraph::Arc Arc;
 std::vector< Arc > arc_of( mcf._res_arc_num , INVALID );
 for( typename MCF::Digraph::ArcIt a( mcf._graph ) ; a != INVALID ; ++a )
  arc_of[ mcf._arc_idf[ a ] ] = a;
 std::vector< std::vector< Arc > > sets( ids.size() );
 for( std::size_t j = 0 ; j != ids.size() ; ++j )
  for( std::size_t h = 0 ; h != ids[ j ].size() ; ++h )
   sets[ j ].push_back( arc_of[ d.ref[ ids[ j ][ h ] ] ] );
 return( sets );
 }

}

}

#endif
