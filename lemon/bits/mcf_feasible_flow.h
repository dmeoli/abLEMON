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

#ifndef LEMON_BITS_MCF_FEASIBLE_FLOW_H
 #define LEMON_BITS_MCF_FEASIBLE_FLOW_H

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

#include <lemon/bellman_ford.h>

//\ingroup min_cost_flow_algs
//\file
//\brief The feasible flow that CostScaling and CycleCanceling start from,
//and the bound that replaces an infinite one in them and in CapacityScaling.

namespace lemon
{

namespace _mcf_bits
{

// The tolerance on the flows: zero for an exact value type, otherwise
// relative to the largest absolute value of the supplies and of the
// finite bounds, as the one of NetworkSimplex. Below it, an excess, a
// deficit or a residual capacity is taken as zero.
template< typename IV , typename BV , typename VV >
typename VV::value_type flowTolerance( int root , const IV & first_out ,
				       const IV & target , const BV & forward ,
				       const VV & lower , const VV & upper ,
				       const VV & supply , bool has_lower )
{
 typedef typename VV::value_type Value;
 if( std::numeric_limits< Value >::is_exact )
  return( 0 );
 const Value MAX = ( std::numeric_limits< Value >::max )();
 Value m = 1;
 for( int u = 0 ; u != root ; ++u ) {
  m = ( std::max )( m , Value( std::abs( supply[ u ] ) ) );
  for( int j = first_out[ u ] ; j != first_out[ u + 1 ] ; ++j ) {
   if( ! forward[ j ] || target[ j ] == root )
    continue;
   if( upper[ j ] < MAX )
    m = ( std::max )( m , Value( std::abs( upper[ j ] ) ) );
   if( has_lower )
    m = ( std::max )( m , Value( std::abs( lower[ j ] ) ) );
   }
  }
 return( m * std::numeric_limits< Value >::epsilon() * 1e6 );
 }

// The tolerance on the reduced costs: zero for an exact cost type,
// otherwise relative to the largest absolute value of the costs of the
// arcs of the digraph. A reduced cost above its opposite is taken as
// nonnegative.
template< typename IV , typename BV , typename CV >
typename CV::value_type costTolerance( int root , const IV & first_out ,
				       const IV & target , const BV & forward ,
				       const CV & cost )
{
 typedef typename CV::value_type Cost;
 if( std::numeric_limits< Cost >::is_exact )
  return( 0 );
 Cost m = 1;
 for( int u = 0 ; u != root ; ++u )
  for( int j = first_out[ u ] ; j != first_out[ u + 1 ] ; ++j )
   if( forward[ j ] && target[ j ] != root )
    m = ( std::max )( m , Cost( std::abs( cost[ j ] ) ) );
 return( m * std::numeric_limits< Cost >::epsilon() * 1e6 );
 }

// Finds a flow within the bounds that meets the supplies on the
// residual network that CostScaling and CycleCanceling work on, which
// they used to find with Circulation on the digraph and its maps: with
// the same push-relabel algorithm (Goldberg and Tarjan, 1988), i.e., the
// active node of highest level first, every node at level zero at the
// start, a push towards any lower level and the first empty level as the
// proof that no such flow exists (Cherkassky and Goldberg, 1997), from the
// flow that the arcs into the nodes with a deficit take greedily, as
// Circulation does.
//
// It sets the residual capacities of the arcs of the digraph and of
// those of the root, the ones out of the root having root_cap if the
// supplies sum to zero, gives in sup_num the number of the nodes whose
// supply net of the lower bounds is positive, and returns false if
// there is no such flow. The last arc out of every node is the one to
// the root, which is not an arc of the digraph. In floating point, an
// excess or a residual capacity within feps, the tolerance on the flows,
// is zero, since what round-off leaves of an excess can find no way out.
template< typename MCF >
bool feasibleFlow( MCF & mcf , typename MCF::Value root_cap , int & sup_num ,
		   typename MCF::Value feps )
{
 typedef typename MCF::Value Value;
 const int n = mcf._root;
 std::vector< Value > ex( mcf._supply.begin() , mcf._supply.begin() + n );
 if( mcf._has_lower ) {
  for( int u = 0 ; u != n ; ++u ) {
   const int last_out = mcf._first_out[ u + 1 ] - 1;
   for( int j = mcf._first_out[ u ] ; j != last_out ; ++j ) {
    if( ! mcf._forward[ j ] )
     continue;
    ex[ u ] -= mcf._lower[ j ];
    ex[ mcf._target[ j ] ] += mcf._lower[ j ];
    }
   }
  }
 sup_num = 0;
 for( int u = 0 ; u != n ; ++u ) {
  if( ex[ u ] <= feps && ex[ u ] >= -feps )
   ex[ u ] = 0;
  if( ex[ u ] > 0 )
   ++sup_num;
  }

 // The flow to start from
 for( int u = 0 ; u != n ; ++u ) {
  const int last_out = mcf._first_out[ u + 1 ] - 1;
  for( int j = mcf._first_out[ u ] ; j != last_out ; ++j ) {
   if( ! mcf._forward[ j ] )
    continue;
   const int v = mcf._target[ j ] , rj = mcf._reverse[ j ];
   const Value cap =
     mcf._has_lower ? mcf._upper[ j ] - mcf._lower[ j ] : mcf._upper[ j ];
   Value f = -ex[ v ];
   if( f > cap )
    f = cap;
   if( f < 0 )
    f = 0;
   mcf._res_cap[ j ] = cap - f;
   mcf._res_cap[ rj ] = f;
   ex[ u ] -= f;
   ex[ v ] += f;
   }
  }

 // The active nodes, i.e., those with a positive excess, in a list for
 // each level, and how many nodes each level has
 std::vector< int > level( n , 0 ) , count( n + 1 , 0 ) , next( n , -1 ) ,
   head( n + 1 , -1 );
 count[ 0 ] = n;
 for( int u = 0 ; u != n ; ++u ) {
  if( ex[ u ] > feps ) {
   next[ u ] = head[ 0 ];
   head[ 0 ] = u;
   }
  }
 for( int top = 0 ; ; ) {
  while( top >= 0 && head[ top ] < 0 )
   --top;
  if( top < 0 )
   break;
  const int u = head[ top ];
  Value e = ex[ u ];
  int low = n;
  const int last_out = mcf._first_out[ u + 1 ] - 1;
  for( int j = mcf._first_out[ u ] ; j != last_out ; ++j ) {
   const Value r = mcf._res_cap[ j ];
   const int v = mcf._target[ j ];
   if( r <= feps || v == u )
    continue;
   const int lv = level[ v ];
   if( lv >= top ) {
    if( lv < low )
     low = lv;
    continue;
    }
   const Value d = r < e ? r : e;
   mcf._res_cap[ j ] = r - d;
   mcf._res_cap[ mcf._reverse[ j ] ] += d;
   if( ex[ v ] <= feps && ex[ v ] + d > feps ) {
    next[ v ] = head[ lv ];
    head[ lv ] = v;
    }
   ex[ v ] += d;
   if( ( e -= d ) <= feps )
    break;
   }
  ex[ u ] = e;
  head[ top ] = next[ u ];
  if( e <= feps )
   continue;
  // u is lifted above the lowest level it has a residual arc to, and
  // no such arc, or no node left on its level, proves that no flow
  // exists
  if( low == n || --count[ top ] == 0 )
   return( false );
  level[ u ] = ++low;
  ++count[ low ];
  next[ u ] = head[ low ];
  head[ low ] = u;
  top = low;
  }

 // A residual capacity within the tolerance goes to the reverse arc, so
 // that the algorithm does not send round-off around
 if( feps > 0 )
  for( int j = 0 ; j != mcf._first_out[ n ] ; ++j ) {
   const Value r = mcf._res_cap[ j ];
   if( r != 0 && r <= feps && r >= -feps ) {
    mcf._res_cap[ mcf._reverse[ j ] ] += r;
    mcf._res_cap[ j ] = 0;
    }
   }

 // The arcs of the root: with supplies that sum to less than zero, the
 // ones out of it carry what is left of the deficits
 for( int a = mcf._first_out[ n ] ; a != mcf._res_arc_num ; ++a ) {
  const int ra = mcf._reverse[ a ];
  if( mcf._sum_supply < 0 ) {
   mcf._res_cap[ a ] = -mcf._sum_supply + 1;
   mcf._res_cap[ ra ] = -ex[ mcf._target[ a ] ];
   }
  else {
   mcf._res_cap[ a ] = root_cap;
   mcf._res_cap[ ra ] = 0;
   }
  mcf._cost[ a ] = 0;
  mcf._cost[ ra ] = 0;
  }
 return( true );
 }

// Whether a cycle of negative cost is made of arcs of infinite upper bound
// (those of the digraph, not those of the root): if the problem is
// feasible, it is then unbounded. Bellman-Ford from a virtual node joined
// to all the others, on those arcs only, which still improves a distance
// after as many rounds as there are nodes if and only if there is one.
template< typename IV , typename BV , typename VV , typename CV >
bool negativeInfiniteCycle( int root , const IV & first_out ,
			    const IV & target ,
			    const BV & forward , const VV & upper ,
			    const CV & cost )
{
 typedef typename VV::value_type Value;
 typedef typename CV::value_type Cost;
 const Value MAX = ( std::numeric_limits< Value >::max )();
 bool negative = false;
 for( int u = 0 ; u != root && ! negative ; ++u )
  for( int j = first_out[ u ] ; j != first_out[ u + 1 ] ; ++j )
   if( forward[ j ] && target[ j ] != root && upper[ j ] >= MAX &&
       cost[ j ] < 0 ) {
    negative = true;
    break;
    }
 if( ! negative )
  return( false );
 std::vector< Cost > dist( root , Cost( 0 ) );
 for( int round = 0 ; round != root ; ++round ) {
  bool changed = false;
  for( int u = 0 ; u != root ; ++u )
   for( int j = first_out[ u ] ; j != first_out[ u + 1 ] ; ++j ) {
    const int v = target[ j ];
    if( forward[ j ] && v != root && upper[ j ] >= MAX &&
	dist[ u ] + cost[ j ] < dist[ v ] ) {
     dist[ v ] = dist[ u ] + cost[ j ];
     changed = true;
     }
    }
  if( ! changed )
   return( false );
  }
 return( true );
 }

// The operations of BellmanFord in CycleCanceling and CostScaling: a
// distance improves only by more than the tolerance on the reduced costs,
// which is zero for an exact cost type, since in floating point the
// round-off of the sums of the distances can make a cycle of cost zero
// (e.g., an arc and its reverse) look negative for ever. The tolerance is
// the one of the run of this thread.
template< typename C >
struct ToleranceBfOperationTraits
    : public BellmanFordDefaultOperationTraits< C >
{
 static C & epsilon() {
  static thread_local C eps = 0;
  return( eps );
  }
 static bool less( const C & left , const C & right ) {
  return( left < right - epsilon() );
  }
 };

// The bound, net of the lower bound, that replaces an infinite upper
// bound. When no cycle of negative cost is made of arcs of infinite upper
// bound, some optimal flow sends on every arc at most B beyond its lower
// bound, B being the sum of the absolute values of the supplies and of
// the finite bounds: a flow is made of paths, which carry at most the
// supplies, and of cycles, each of which has an arc of finite bound, the
// other ones having a nonnegative cost and then being of no use. One more
// than B makes an arc that reaches it keep a nonnegative reduced cost in
// an optimal solution, so that the potentials are optimal for the
// infinite bounds too. The bound is cut where the sums of the flows over
// the arcs that it replaces would no longer fit in Value.
template< typename IV , typename BV , typename VV >
typename VV::value_type infiniteBound( int root , const IV & first_out ,
				       const IV & target , const BV & forward ,
				       const VV & lower , const VV & upper ,
				       const VV & supply , bool has_lower )
{
 typedef typename VV::value_type Value;
 const Value MAX = ( std::numeric_limits< Value >::max )();
 long double b = 1;
 int inf = 0;
 for( int u = 0 ; u != root ; ++u ) {
  b += std::abs( static_cast< long double >( supply[ u ] ) );
  for( int j = first_out[ u ] ; j != first_out[ u + 1 ] ; ++j ) {
   if( ! forward[ j ] || target[ j ] == root )
    continue;
   if( upper[ j ] < MAX )
    b += std::abs( static_cast< long double >( upper[ j ] ) );
   else
    ++inf;
   if( has_lower )
    b += std::abs( static_cast< long double >( lower[ j ] ) );
   }
  }
 const long double most = static_cast< long double >( MAX ) / ( inf + 2 );
 return( b < most ? static_cast< Value >( b )
		   : static_cast< Value >( most ) );
 }

}

}

#endif
