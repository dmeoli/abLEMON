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

#ifndef LEMON_K_BEST_FLOWS_H
 #define LEMON_K_BEST_FLOWS_H

#include <functional>
#include <limits>
#include <queue>
#include <utility>
#include <vector>

#include <lemon/core.h>
#include <lemon/network_simplex.h>

///\ingroup min_cost_flow_algs
///\file
///\brief The K best integer flows of a minimum cost flow problem.

namespace lemon
{

/// \ingroup min_cost_flow_algs
///
/// \brief The K best integer flows of a minimum cost flow problem.
///
/// This class finds the K integer flows of least cost of a \ref
/// min_cost_flow "minimum cost flow problem" with the \c GEQ supply
/// constraints, in the order of their costs: first an optimal flow,
/// then the other optimal flows, if any, then the suboptimal ones.
///
/// The algorithm is the one of Hamacher \cite hamacher95kbestflows. The
/// second best flow of a set of flows whose best one is known differs
/// from it by one unit along a cycle of the residual network, which is the
/// one of least cost: the difference between the two is a circulation in
/// that network, whose cycles all have a nonnegative cost, and one unit
/// along any of them gives a flow of the set no costlier than the second
/// one. The set is then split on an arc where the two flows differ, as
/// Lawler \cite lawler72kbest does
/// for the K best solutions of any problem: on one side the flow of the
/// arc keeps the value it has in the best flow, which remains the best
/// of that side, and on the other it takes the value of the second one,
/// which becomes the best of that side; the next flow is the cheapest of
/// the second best flows of the sides. The cycle of least cost through
/// each residual arc is found by the algorithm of Dijkstra with the
/// reduced costs of potentials that the algorithm of Bellman and Ford
/// computes: one search from each node, and one more for each residual
/// arc whose shortest path back is its own reverse, which is not a cycle.
/// The k-th flow then takes \f$O(nm\log n)\f$ time after the (k-1)-th,
/// and \f$O(m^2\log n)\f$ at worst.
///
/// The first flow is an optimal one that \ref NetworkSimplex finds, or
/// any optimal flow given by \ref initFlow(): the sequence does not
/// depend on the algorithm that found it, but for the order of the flows
/// of equal cost.
///
/// \tparam GR The digraph type the algorithm runs on.
/// \tparam V The number type of the flow amounts, of the bounds and of the
/// supplies; by default, \c int.
/// \tparam C The number type of the costs; by default, the same as \c V.
///
/// \warning The bounds and the supplies must be integer (with a
/// floating-point \c V too, whose values are then integer ones), since
/// the flows differ from each other by whole units; the upper bounds may
/// be infinite, but then no cycle of negative cost may be made of arcs
/// of infinite upper bound, or the problem has no optimal flow.
template< typename GR , typename V = int , typename C = V >
class KBestFlows
{
 public:
 /// The type of the digraph
 typedef GR Digraph;
 /// The type of the flow amounts, of the bounds and of the supplies
 typedef V Value;
 /// The type of the costs
 typedef C Cost;

 private:
 TEMPLATE_DIGRAPH_TYPEDEFS( GR );

 // The arcs of the problem are 0, ..., m - 1, those of the digraph, and
 // m, ..., m + n - 1, one from the root (the node n) to every node, which
 // takes what a node receives beyond its demand under GEQ; the residual
 // arc 2 i is the arc i forward, 2 i + 1 backward
 struct Set
 {
  std::vector< Value > lower , upper , flow;
  std::vector< char > inf;
  Cost cost;
  // the second best flow of the set, as a cycle of residual arcs
  bool has_next;
  Cost next_cost;
  std::vector< int > cycle;
  };

 const GR & _graph;
 int _n , _m;
 IntNodeMap _node_id;
 IntArcMap _arc_id;
 std::vector< int > _source , _target;
 std::vector< Value > _lower , _upper , _supply;
 std::vector< char > _inf;
 std::vector< Cost > _cost;
 bool _has_init;
 std::vector< Value > _init;

 std::vector< std::vector< Value > > _flows;
 std::vector< Cost > _costs;

 public:
 /// \brief Constructor.
 ///
 /// The bounds are 0 and infinite, the costs 1 and the supplies 0 until
 /// they are given.
 explicit KBestFlows( const GR & graph )
     : _graph( graph ) , _n( countNodes( graph ) ) ,
       _m( countArcs( graph ) ) , _node_id( graph ) , _arc_id( graph ) ,
       _source( _m ) , _target( _m ) , _lower( _m , 0 ) , _upper( _m , 0 ) ,
       _supply( _n , 0 ) , _inf( _m , 1 ) , _cost( _m , 1 ) ,
       _has_init( false ) {
  int i = 0;
  for( NodeIt u( graph ) ; u != INVALID ; ++u )
   _node_id[ u ] = i++;
  i = 0;
  for( ArcIt a( graph ) ; a != INVALID ; ++a , ++i ) {
   _arc_id[ a ] = i;
   _source[ i ] = _node_id[ graph.source( a ) ];
   _target[ i ] = _node_id[ graph.target( a ) ];
   }
  }

 /// \brief Sets the lower bounds of the arcs.
 template< typename LowerMap >
 KBestFlows & lowerMap( const LowerMap & map ) {
  for( ArcIt a( _graph ) ; a != INVALID ; ++a )
   _lower[ _arc_id[ a ] ] = map[ a ];
  return( *this );
  }

 /// \brief Sets the upper bounds of the arcs.
 ///
 /// An upper bound that is infinite or the largest value of \c Value
 /// means no bound.
 template< typename UpperMap >
 KBestFlows & upperMap( const UpperMap & map ) {
  const Value MAX = ( std::numeric_limits< Value >::max )();
  for( ArcIt a( _graph ) ; a != INVALID ; ++a ) {
   const int i = _arc_id[ a ];
   _upper[ i ] = map[ a ];
   _inf[ i ] = map[ a ] >= MAX;
   }
  return( *this );
  }

 /// \brief Sets the costs of the arcs.
 template< typename CostMap >
 KBestFlows & costMap( const CostMap & map ) {
  for( ArcIt a( _graph ) ; a != INVALID ; ++a )
   _cost[ _arc_id[ a ] ] = map[ a ];
  return( *this );
  }

 /// \brief Sets the supplies of the nodes (negative for the demands).
 template< typename SupplyMap >
 KBestFlows & supplyMap( const SupplyMap & map ) {
  for( NodeIt u( _graph ) ; u != INVALID ; ++u )
   _supply[ _node_id[ u ] ] = map[ u ];
  return( *this );
  }

 /// \brief Gives the optimal flow to start from.
 ///
 /// The flow must be optimal for the data given; then no algorithm of
 /// minimum cost flow is run.
 template< typename FlowMap >
 KBestFlows & initFlow( const FlowMap & map ) {
  _init.assign( _m , 0 );
  for( ArcIt a( _graph ) ; a != INVALID ; ++a )
   _init[ _arc_id[ a ] ] = map[ a ];
  _has_init = true;
  return( *this );
  }

 /// \brief Finds the k best flows.
 ///
 /// \return The number of the flows found, which is less than \c k if
 /// the problem has fewer integer flows, and zero if it has no optimal
 /// flow (it is infeasible or unbounded).
 int run( int k ) {
  _flows.clear();
  _costs.clear();
  if( k <= 0 || ! firstFlow() )
   return( 0 );

  // the root takes from the supplies what the nodes receive beyond their
  // demands, i.e., out - in - supply on each node
  Set s;
  s.lower.assign( _m + _n , 0 );
  s.upper.assign( _m + _n , 0 );
  s.inf.assign( _m + _n , 1 );
  s.flow.assign( _m + _n , 0 );
  for( int i = 0 ; i != _m ; ++i ) {
   s.lower[ i ] = _lower[ i ];
   s.upper[ i ] = _upper[ i ];
   s.inf[ i ] = _inf[ i ];
   s.flow[ i ] = _flows[ 0 ][ i ];
   }
  std::vector< Value > net( _n , 0 );
  for( int i = 0 ; i != _m ; ++i ) {
   net[ _source[ i ] ] += s.flow[ i ];
   net[ _target[ i ] ] -= s.flow[ i ];
   }
  for( int u = 0 ; u != _n ; ++u )
   s.flow[ _m + u ] = net[ u ] - _supply[ u ];
  s.cost = _costs[ 0 ];
  secondBest( s );

  std::vector< Set > sets( 1 , s );
  while( int( _flows.size() ) < k ) {
   int best = -1;
   for( int j = 0 ; j != int( sets.size() ) ; ++j )
    if( sets[ j ].has_next &&
	( best < 0 || sets[ j ].next_cost < sets[ best ].next_cost ) )
     best = j;
   if( best < 0 )
    break;

   // the second best flow of the set, which is the next flow, and the
   // split of the set on the first arc of its cycle
   Set & a = sets[ best ];
   Set b = a;
   for( std::size_t h = 0 ; h != a.cycle.size() ; ++h ) {
    const int r = a.cycle[ h ];
    b.flow[ r / 2 ] += r % 2 == 0 ? Value( 1 ) : Value( -1 );
    }
   b.cost = a.next_cost;
   const int r = a.cycle.front() , i = r / 2;
   if( r % 2 == 0 ) {
    a.upper[ i ] = a.flow[ i ];
    a.inf[ i ] = 0;
    b.lower[ i ] = a.flow[ i ] + 1;
    }
   else {
    a.lower[ i ] = a.flow[ i ];
    b.upper[ i ] = a.flow[ i ] - 1;
    b.inf[ i ] = 0;
    }
   _flows.push_back(
     std::vector< Value >( b.flow.begin() , b.flow.begin() + _m ) );
   _costs.push_back( b.cost );
   secondBest( a );
   secondBest( b );
   sets.push_back( b );
   }
  return( int( _flows.size() ) );
  }

 /// \brief The number of the flows found by the last \ref run().
 int flowNum() const { return( int( _flows.size() ) ); }

 /// \brief The total cost of the i-th flow, the first being 0.
 Cost totalCost( int i ) const { return( _costs[ i ] ); }

 /// \brief The flow on an arc in the i-th flow, the first being 0.
 Value flow( int i , const Arc & a ) const {
  return( _flows[ i ][ _arc_id[ a ] ] );
  }

 /// \brief Copies the i-th flow, the first being 0, into a map.
 template< typename FlowMap >
 void flowMap( int i , FlowMap & map ) const {
  for( ArcIt a( _graph ) ; a != INVALID ; ++a )
   map.set( a , _flows[ i ][ _arc_id[ a ] ] );
  }

 private:
 // An optimal flow, the given one or that of NetworkSimplex, as the
 // first flow; false if there is none
 bool firstFlow() {
  std::vector< Value > f( _m , 0 );
  if( _has_init )
   f = _init;
  else {
   typedef NetworkSimplex< GR , Value , Cost > NS;
   NS ns( _graph );
   typename Digraph::template ArcMap< Value > low( _graph ) , up( _graph );
   typename Digraph::template ArcMap< Cost > cost( _graph );
   typename Digraph::template NodeMap< Value > sup( _graph );
   for( ArcIt a( _graph ) ; a != INVALID ; ++a ) {
    const int i = _arc_id[ a ];
    low[ a ] = _lower[ i ];
    up[ a ] = _inf[ i ] ? ns.INF : _upper[ i ];
    cost[ a ] = _cost[ i ];
    }
   for( NodeIt u( _graph ) ; u != INVALID ; ++u )
    sup[ u ] = _supply[ _node_id[ u ] ];
   ns.lowerMap( low ).upperMap( up ).costMap( cost ).supplyMap( sup );
   if( ns.run() != NS::OPTIMAL )
    return( false );
   for( ArcIt a( _graph ) ; a != INVALID ; ++a )
    f[ _arc_id[ a ] ] = ns.flow( a );
   }
  Cost c = 0;
  for( int i = 0 ; i != _m ; ++i )
   c += _cost[ i ] * f[ i ];
  _flows.push_back( f );
  _costs.push_back( c );
  return( true );
  }

 int tail( int r ) const {
  const int i = r / 2;
  const int s = i < _m ? _source[ i ] : _n;
  const int t = i < _m ? _target[ i ] : i - _m;
  return( r % 2 == 0 ? s : t );
  }

 int head( int r ) const {
  const int i = r / 2;
  const int s = i < _m ? _source[ i ] : _n;
  const int t = i < _m ? _target[ i ] : i - _m;
  return( r % 2 == 0 ? t : s );
  }

 Cost resCost( int r ) const {
  const int i = r / 2;
  const Cost c = i < _m ? _cost[ i ] : Cost( 0 );
  return( r % 2 == 0 ? c : -c );
  }

 // Whether one more unit may go along the residual arc r of the set
 static bool open( const Set & s , int r ) {
  const int i = r / 2;
  if( r % 2 == 0 )
   return( s.inf[ i ] || s.flow[ i ] + 1 <= s.upper[ i ] );
  return( s.flow[ i ] - 1 >= s.lower[ i ] );
  }

 // The second best flow of a set whose best flow is s.flow: one unit
 // along the residual cycle of least cost that is not an arc and its own
 // reverse
 void secondBest( Set & s ) {
  s.has_next = false;
  s.cycle.clear();
  const int nn = _n + 1 , rn = 2 * ( _m + _n );
  std::vector< std::vector< int > > out( nn );
  for( int r = 0 ; r != rn ; ++r )
   if( open( s , r ) )
    out[ tail( r ) ].push_back( r );

  // potentials, by Bellman-Ford from a virtual node joined to all: the
  // best flow of the set leaves no cycle of negative cost
  std::vector< Cost > pi( nn , Cost( 0 ) );
  for( int round = 0 ; round != nn ; ++round ) {
   bool changed = false;
   for( int u = 0 ; u != nn ; ++u )
    for( std::size_t h = 0 ; h != out[ u ].size() ; ++h ) {
     const int r = out[ u ][ h ] , v = head( r );
     if( pi[ u ] + resCost( r ) < pi[ v ] ) {
      pi[ v ] = pi[ u ] + resCost( r );
      changed = true;
      }
     }
   if( ! changed )
    break;
   }

  const Cost INFC = ( std::numeric_limits< Cost >::max )();
  std::vector< Cost > dist( nn );
  std::vector< int > pred( nn );
  typedef std::pair< Cost , int > Item;
  // Dijkstra from s0 with the reduced costs, without the residual arc
  // skip; dist and pred are those of the nodes reached
  auto dijkstra = [ & ]( int s0 , int skip ) {
   std::fill( dist.begin() , dist.end() , INFC );
   std::fill( pred.begin() , pred.end() , -1 );
   std::priority_queue< Item , std::vector< Item > , std::greater< Item > >
     heap;
   dist[ s0 ] = 0;
   heap.push( Item( Cost( 0 ) , s0 ) );
   while( ! heap.empty() ) {
    const Item it = heap.top();
    heap.pop();
    const int u = it.second;
    if( it.first > dist[ u ] )
     continue;
    for( std::size_t h = 0 ; h != out[ u ].size() ; ++h ) {
     const int r = out[ u ][ h ];
     if( r == skip )
      continue;
     const int v = head( r );
     Cost rc = resCost( r ) + pi[ u ] - pi[ v ];
     if( rc < 0 )
      rc = 0;
     if( dist[ u ] + rc < dist[ v ] ) {
      dist[ v ] = dist[ u ] + rc;
      pred[ v ] = r;
      heap.push( Item( dist[ v ] , v ) );
      }
     }
    }
   };

  // the cycle of least cost through each residual arc r = (u, v) is r and
  // a shortest path from v to u that does not take the reverse of r
  Cost best = INFC;
  for( int v = 0 ; v != nn ; ++v ) {
   bool done = false;
   for( int r = 0 ; r != rn ; ++r ) {
    if( head( r ) != v || ! open( s , r ) )
     continue;
    if( ! done ) {
     dijkstra( v , -1 );
     done = true;
     }
    const int u = tail( r ) , rr = r ^ 1;
    std::vector< Cost > d2;
    std::vector< int > p2;
    const std::vector< Cost > * dp = & dist;
    const std::vector< int > * pp = & pred;
    if( pred[ u ] == rr ) {
     // the shortest path is the reverse of r alone: one without it
     const std::vector< Cost > d0 = dist;
     const std::vector< int > p0 = pred;
     dijkstra( v , rr );
     d2 = dist;
     p2 = pred;
     dist = d0;
     pred = p0;
     dp = & d2;
     pp = & p2;
     }
    if( ( *dp )[ u ] == INFC )
     continue;
    const Cost rc = resCost( r ) + pi[ u ] - pi[ v ];
    const Cost c = ( rc < 0 ? Cost( 0 ) : rc ) + ( *dp )[ u ];
    if( c < best ) {
     best = c;
     s.cycle.assign( 1 , r );
     std::vector< int > path;
     for( int w = u ; w != v ; w = tail( ( *pp )[ w ] ) )
      path.push_back( ( *pp )[ w ] );
     s.cycle.insert( s.cycle.end() , path.rbegin() , path.rend() );
     }
    }
  }
  if( s.cycle.empty() )
   return;
  // the exact cost, as the sum of the costs of the cycle
  Cost c = 0;
  for( std::size_t h = 0 ; h != s.cycle.size() ; ++h )
   c += resCost( s.cycle[ h ] );
  s.has_next = true;
  s.next_cost = s.cost + c;
  }
 };

}

#endif
