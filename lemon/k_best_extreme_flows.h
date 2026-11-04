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

#ifndef LEMON_K_BEST_EXTREME_FLOWS_H
 #define LEMON_K_BEST_EXTREME_FLOWS_H

#include <algorithm>
#include <functional>
#include <iterator>
#include <limits>
#include <map>
#include <queue>
#include <set>
#include <utility>
#include <vector>

#include <lemon/core.h>
#include <lemon/network_simplex.h>

///\ingroup min_cost_flow_algs
///\file
///\brief The K best extreme flows of a minimum cost flow problem.

namespace lemon
{

/// \ingroup min_cost_flow_algs
///
/// \brief The K best extreme flows of a minimum cost flow problem.
///
/// This class finds the K vertices of least cost of the polyhedron of the
/// flows of a \ref min_cost_flow "minimum cost flow problem" with the
/// \c GEQ supply constraints, in the order of their costs: first an
/// optimal vertex, then the other optimal ones, if any, then those whose
/// cost exceeds the optimal one by at most a given gap. These are the
/// flows that a Lagrangian or a Benders' decomposition wants several of,
/// since each gives a different (sub)gradient, while a flow inside a face
/// gives a convex combination of those of its vertices.
///
/// The algorithm is the ranking of the vertices of Murty
/// \cite murty68ranking: the next vertex is adjacent to one of those
/// already given, i.e., one pivot of the network simplex away from one of
/// their bases, along the cycle that a nonbasic arc closes with the tree of
/// the basis, saturated. The candidates are kept in a queue by their
/// costs, and the bases of a degenerate vertex, which the pivots along a
/// cycle of zero amount reach, are all expanded. The first vertex is the
/// optimal flow that \ref NetworkSimplex finds, or one given by
/// \ref initFlow(), brought to a vertex first, if it is not one, by
/// saturating the cycles of the arcs strictly within their bounds, whose
/// costs are zero at an optimal flow.
///
/// \tparam GR The digraph type the algorithm runs on.
/// \tparam V The number type of the flow amounts, of the bounds and of the
/// supplies; by default, \c int.
/// \tparam C The number type of the costs; by default, the same as \c V.
///
/// \note The number of the bases of a degenerate vertex can be large, and
/// all of them are visited when the vertex is expanded; each pivot takes
/// \f$O(n)\f$ time, and each basis \f$O(nm)\f$ for all its pivots. A
/// candidate pivot waits in the queue as its cost alone, the basis it leads
/// to being made when it is taken, so that the memory is \f$O(n+m)\f$ for
/// each basis expanded, and their number is bounded by \ref basisLimit().
template< typename GR , typename V = int , typename C = V >
class KBestExtremeFlows
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

 // The arcs are 0, ..., m - 1, those of the digraph, and m, ..., m + n - 1,
 // one from the root (the node n) to every node, which takes what a node
 // receives beyond its demand under GEQ, with no upper bound and no cost
 struct State
 {
  std::vector< Value > flow;
  std::vector< char > tree;
  Cost cost;
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
 int _basis_limit;
 int _vertex_limit;
 int _method;
 bool _exact;
 std::vector< Value > _init;
 Value _feps;

 std::vector< std::vector< Value > > _flows;
 std::vector< Cost > _costs;

 public:
 /// \brief Constructor.
 ///
 /// The bounds are 0 and infinite, the costs 1 and the supplies 0 until
 /// they are given.
 explicit KBestExtremeFlows( const GR & graph )
     : _graph( graph ) , _n( countNodes( graph ) ) ,
       _m( countArcs( graph ) ) , _node_id( graph ) , _arc_id( graph ) ,
       _source( _m + _n ) , _target( _m + _n ) , _lower( _m + _n , 0 ) ,
       _upper( _m + _n , 0 ) , _supply( _n , 0 ) , _inf( _m + _n , 1 ) ,
       _cost( _m + _n , 0 ) , _has_init( false ) , _basis_limit( 1000 ) ,
       _vertex_limit( 0 ) , _method( RANKING ) ,
       _exact( true ) , _feps( 0 ) {
  int i = 0;
  for( NodeIt u( graph ) ; u != INVALID ; ++u )
   _node_id[ u ] = i++;
  i = 0;
  for( ArcIt a( graph ) ; a != INVALID ; ++a , ++i ) {
   _arc_id[ a ] = i;
   _source[ i ] = _node_id[ graph.source( a ) ];
   _target[ i ] = _node_id[ graph.target( a ) ];
   _cost[ i ] = 1;
   }
  for( int u = 0 ; u != _n ; ++u ) {
   _source[ _m + u ] = _n;
   _target[ _m + u ] = u;
   }
  }

 /// \brief Sets the lower bounds of the arcs.
 template< typename LowerMap >
 KBestExtremeFlows & lowerMap( const LowerMap & map ) {
  for( ArcIt a( _graph ) ; a != INVALID ; ++a )
   _lower[ _arc_id[ a ] ] = map[ a ];
  return( *this );
  }

 /// \brief Sets the upper bounds of the arcs.
 ///
 /// An upper bound that is infinite or the largest value of \c Value
 /// means no bound.
 template< typename UpperMap >
 KBestExtremeFlows & upperMap( const UpperMap & map ) {
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
 KBestExtremeFlows & costMap( const CostMap & map ) {
  for( ArcIt a( _graph ) ; a != INVALID ; ++a )
   _cost[ _arc_id[ a ] ] = map[ a ];
  return( *this );
  }

 /// \brief Sets the supplies of the nodes (negative for the demands).
 template< typename SupplyMap >
 KBestExtremeFlows & supplyMap( const SupplyMap & map ) {
  for( NodeIt u( _graph ) ; u != INVALID ; ++u )
   _supply[ _node_id[ u ] ] = map[ u ];
  return( *this );
  }

 /// \brief Gives the optimal flow to start from.
 ///
 /// The flow must be optimal for the data given, not necessarily a
 /// vertex; then no algorithm of minimum cost flow is run.
 template< typename FlowMap >
 KBestExtremeFlows & initFlow( const FlowMap & map ) {
  _init.assign( _m , 0 );
  for( ArcIt a( _graph ) ; a != INVALID ; ++a )
   _init[ _arc_id[ a ] ] = map[ a ];
  _has_init = true;
  return( *this );
  }

 /// \brief The ways the flows are generated [see \ref method()].
 enum Method {
  /// The ranking of Murty: the k best vertices, in order.
  RANKING ,
  /// The cycles of least reduced cost: the optimal vertices that the
  /// cycles of zero reduced cost lead to, then the vertices one cycle away
  /// from them, in the order of their costs.
  CYCLES
  };

 /// \brief Sets the way the flows are generated.
 ///
 /// With \ref RANKING, the default, a \ref run() gives the k best
 /// vertices in order, as long as the limits on the bases allow it [see
 /// \ref basisLimit() and \ref vertexBasisLimit()]. With \ref CYCLES it
 /// enumerates no basis: from the optimal vertex and potentials optimal
 /// for it, the cycles of the residual graph whose arcs have a reduced
 /// cost of zero, saturated and made vertices, are the other optimal
 /// vertices, from which the same is done again; then, from each optimal
 /// vertex and for each residual arc, the cycle of least reduced cost
 /// through the arc (one Dijkstra's algorithm for each node), saturated and
 /// made a vertex, the cost not growing, gives a vertex of larger cost, and
 /// these are given in the order of their costs. The flows are distinct
 /// vertices in the order of their costs, the optimal ones first, but
 /// those of larger cost are only the best of the vertices one cycle away
 /// from an optimal one [see \ref exact()]; each optimal vertex costs
 /// \f$O(nm\log n)\f$ time and \f$O(km)\f$ memory.
 KBestExtremeFlows & method( Method m ) {
  _method = m;
  return( *this );
  }

 /// \brief Finds the k best extreme flows.
 ///
 /// \param k The largest number of flows to find.
 /// \param gap The largest excess of the cost of a flow over the optimal
 /// one; a negative one, the default, sets no limit.
 ///
 /// \return The number of the flows found, which is less than \c k if
 /// the problem has fewer vertices within the gap, and zero if it has no
 /// optimal flow (it is infeasible or unbounded).
 int run( int k , Cost gap = -1 ) {
  if( _method == CYCLES )
   return( runCycles( k , gap ) );
  _flows.clear();
  _costs.clear();
  _exact = true;
  State s;
  if( k <= 0 || ! firstState( s ) )
   return( 0 );
  // a negative gap is no gap at all
  const bool bounded = gap >= 0;
  const Cost limit = bounded ? s.cost + gap : s.cost;

  // a candidate is the pivot of the arc r from the expanded basis j, -1
  // for the first basis, kept by its cost alone: the basis it leads to is
  // made when it leaves the queue
  typedef std::pair< Cost , std::pair< int , int > > Item;
  std::priority_queue< Item , std::vector< Item > , std::greater< Item > >
    queue;
  struct Expanded
  {
   State state;
   std::vector< int > parent , depth;
   };
  std::vector< Expanded > done;
  // a basis is its tree and the bounds its nonbasic arcs are at, i.e.,
  // the tree and the flow
  std::set< std::pair< std::vector< char > , std::vector< Value > > >
    seen_bases;
  std::map< std::vector< Value > , int > seen_flows;
  queue.push( Item( s.cost , std::make_pair( -1 , -1 ) ) );
  while( ! queue.empty() && int( _flows.size() ) < k ) {
   const Item it = queue.top();
   queue.pop();
   if( bounded && it.first > limit )
    break;
   State st;
   if( it.second.first < 0 )
    st = s;
   else {
    const Expanded & e = done[ it.second.first ];
    pivot( e.state , e.parent , e.depth , it.second.second , st );
    }
   if( seen_bases.count( std::make_pair( st.tree , st.flow ) ) )
    continue;
   if( int( done.size() ) >= _basis_limit ) {
    _exact = false;
    break;
    }
   std::vector< Value > f( st.flow.begin() , st.flow.begin() + _m );
   auto sf = seen_flows.insert( std::make_pair( f , 0 ) );
   if( sf.second ) {
    _flows.push_back( f );
    _costs.push_back( st.cost );
    }
   // the bases of a vertex beyond the limit are not expanded; a basis
   // counts only if it leads to another vertex, so that those whose
   // pivots are all degenerate do not take the place of one that does
   int & count = sf.first->second;
   if( _vertex_limit > 0 && count >= _vertex_limit ) {
    _exact = false;
    continue;
    }
   seen_bases.insert( std::make_pair( st.tree , st.flow ) );
   // every pivot of the basis, saturating the cycle of a nonbasic arc
   done.push_back( Expanded() );
   Expanded & e = done.back();
   rootTree( st , e.parent , e.depth );
   e.state = std::move( st );
   const int j = int( done.size() ) - 1;
   std::vector< std::pair< Cost , int > > degenerate;
   bool moves = false;
   for( int r = 0 ; r != 2 * ( _m + _n ) ; ++r ) {
    if( e.state.tree[ r / 2 ] || ! open( e.state , r ) )
     continue;
    Cost c;
    Value amount;
    if( ! pivotCost( e.state , e.parent , e.depth , r , c , amount ) ||
	( bounded && c > limit ) )
     continue;
    if( amount <= _feps )
     degenerate.push_back( std::make_pair( c , r ) );
    else {
     moves = true;
     queue.push( Item( c , std::make_pair( j , r ) ) );
     }
    }
   // a degenerate pivot leads to another basis of the same vertex, which
   // is not looked for once the vertex has used up its bases
   if( moves )
    ++count;
   if( _vertex_limit > 0 && count >= _vertex_limit && ! degenerate.empty() )
    _exact = false;
   else
    for( std::size_t h = 0 ; h != degenerate.size() ; ++h )
     queue.push( Item( degenerate[ h ].first ,
		       std::make_pair( j , degenerate[ h ].second ) ) );
   }
  // a vertex reached only through a basis left out may come after one of
  // larger cost: the flows are put back in the order of their costs
  if( ! _exact ) {
   std::vector< int > order( _flows.size() );
   for( std::size_t i = 0 ; i != order.size() ; ++i )
    order[ i ] = int( i );
   std::stable_sort( order.begin() , order.end() ,
		     [ this ]( int x , int y ) {
		      return( _costs[ x ] < _costs[ y ] ); } );
   std::vector< std::vector< Value > > flows;
   std::vector< Cost > costs;
   for( std::size_t i = 0 ; i != order.size() ; ++i ) {
    flows.push_back( std::move( _flows[ order[ i ] ] ) );
    costs.push_back( _costs[ order[ i ] ] );
    }
   _flows.swap( flows );
   _costs.swap( costs );
   }
  return( int( _flows.size() ) );
  }

 /// \brief Sets the largest number of bases a \ref run() expands.
 ///
 /// A degenerate vertex has many bases, and the ranking expands all of
 /// those it meets, each taking \f$O(n+m)\f$ memory and \f$O(n(n+m))\f$
 /// time; when the limit is reached the run stops, the flows it gives are
 /// still distinct vertices in the order of their costs, but they may not
 /// be the k best ones [see \ref exact()]. By default, 1000.
 KBestExtremeFlows & basisLimit( int limit ) {
  _basis_limit = limit;
  return( *this );
  }

 /// \brief Sets the largest number of bases of one vertex a \ref run()
 /// expands.
 ///
 /// At a degenerate vertex the pivots of zero amount lead to its other
 /// bases, and a vertex of a problem as a shortest path, a path whose
 /// other basic arcs carry no flow, has as many bases as there are trees
 /// containing it; the next vertex is adjacent to a basis of one of those
 /// given, but expanding them all may take the whole \ref basisLimit().
 /// With a positive limit, a basis counting only if one of its pivots
 /// leads to another vertex, the bases of a vertex beyond it are not
 /// expanded and the degenerate pivots of its last one are not taken: the
 /// flows given are distinct vertices in the order of their costs, the
 /// first one optimal, adjacent to the bases expanded, but they may not be
 /// the k best ones [see \ref exact()]. By default, no limit.
 KBestExtremeFlows & vertexBasisLimit( int limit ) {
  _vertex_limit = limit;
  return( *this );
  }

 /// \brief Whether the last \ref run() is exact.
 ///
 /// Always false with \ref CYCLES [see \ref method()]; with \ref RANKING,
 /// false if the last \ref run() stopped at the limit of the bases
 /// [see \ref basisLimit()] or left out a basis or a pivot for the limit
 /// of the bases of a vertex [see \ref vertexBasisLimit()], so that the
 /// flows it gives may not be the k best ones.
 bool exact() const { return( _exact ); }

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
 // The run of CYCLES [see method()]
 int runCycles( int k , Cost gap ) {
  _flows.clear();
  _costs.clear();
  _exact = false;
  State s;
  if( k <= 0 || ! firstState( s ) )
   return( 0 );
  const bool bounded = gap >= 0;
  const Cost limit = bounded ? s.cost + gap : s.cost;
  const int nn = _n + 1 , rr = 2 * ( _m + _n );

  // the tolerance on the reduced costs: zero for an exact type, otherwise
  // relative to the largest cost
  Cost ceps = 0;
  if( ! std::numeric_limits< Cost >::is_exact ) {
   Cost big = 1;
   for( int i = 0 ; i != _m ; ++i )
    big = ( std::max )( big , _cost[ i ] < 0 ? Cost( -_cost[ i ] )
					      : _cost[ i ] );
   ceps = static_cast< Cost >( big * std::numeric_limits< Cost >::epsilon() *
			       1e6 );
   }

  // potentials optimal for the first vertex: the distances in its residual
  // graph from a source joined to every node, which has no negative cycle
  std::vector< Cost > pi( nn , 0 );
  for( int pass = 0 ; pass != nn ; ++pass ) {
   bool changed = false;
   for( int r = 0 ; r != rr ; ++r )
    if( open( s , r ) && pi[ tail( r ) ] + resCost( r ) <
	                 pi[ head( r ) ] - ceps ) {
     pi[ head( r ) ] = pi[ tail( r ) ] + resCost( r );
     changed = true;
     }
   if( ! changed )
    break;
   }
  // the reduced costs, nonnegative on the residual arcs of every optimal
  // vertex, the potentials being optimal for all of them
  std::vector< Cost > rc( rr );
  for( int r = 0 ; r != rr ; ++r ) {
   rc[ r ] = resCost( r ) + pi[ tail( r ) ] - pi[ head( r ) ];
   if( rc[ r ] < 0 )
    rc[ r ] = 0;
   }

  std::set< std::vector< Value > > seen;
  auto give = [ & ]( const State & x ) -> bool {
   std::vector< Value > f( x.flow.begin() , x.flow.begin() + _m );
   if( ! seen.insert( f ).second )
    return( false );
   _flows.push_back( f );
   _costs.push_back( x.cost );
   return( true );
   };

  // the cycle of the residual arc r closed by the path from head( r ) to
  // tail( r ) of the tree pred, saturated and made a vertex
  auto step = [ & ]( const State & x , int r ,
		     const std::vector< int > & pred , State & y ) -> bool {
   std::vector< int > cycle( 1 , r );
   for( int w = tail( r ) ; w != head( r ) ; w = tail( pred[ w ] ) )
    cycle.push_back( pred[ w ] );
   Value amount = 0;
   bool first = true;
   for( std::size_t h = 0 ; h != cycle.size() ; ++h ) {
    Value c;
    if( ! residual( x , cycle[ h ] , c ) )
     continue;
    if( first || c < amount )
     amount = c;
    first = false;
    }
   if( first || amount <= _feps )
    return( false );
   y = x;
   for( std::size_t h = 0 ; h != cycle.size() ; ++h )
    y.flow[ cycle[ h ] / 2 ] += cycle[ h ] % 2 == 0 ? amount : -amount;
   purify( y );
   y.cost = 0;
   for( int i = 0 ; i != _m ; ++i )
    y.cost += _cost[ i ] * y.flow[ i ];
   return( true );
   };

  // the paths from v along the open residual arcs of x, by Dijkstra's
  // algorithm on the reduced costs, those of zero reduced cost alone if
  // zero is true
  auto paths = [ & ]( const std::vector< std::vector< int > > & out , int v ,
		      bool zero , std::vector< Cost > & dist ,
		      std::vector< int > & pred ) {
   dist.assign( nn , -1 );
   pred.assign( nn , -1 );
   typedef std::pair< Cost , int > QI;
   std::priority_queue< QI , std::vector< QI > , std::greater< QI > > q;
   std::vector< char > done( nn , 0 );
   dist[ v ] = 0;
   q.push( QI( 0 , v ) );
   while( ! q.empty() ) {
    const int w = q.top().second;
    q.pop();
    if( done[ w ] )
     continue;
    done[ w ] = 1;
    for( std::size_t t = 0 ; t != out[ w ].size() ; ++t ) {
     const int r = out[ w ][ t ] , z = head( r );
     if( zero && rc[ r ] > ceps )
      continue;
     const Cost d = dist[ w ] + rc[ r ];
     if( ! done[ z ] && ( dist[ z ] < 0 || d < dist[ z ] ) ) {
      dist[ z ] = d;
      pred[ z ] = r;
      q.push( QI( d , z ) );
      }
     }
    }
   };

  auto outOf = [ & ]( const State & x )
		      -> std::vector< std::vector< int > > {
   std::vector< std::vector< int > > out( nn );
   for( int r = 0 ; r != rr ; ++r )
    if( open( x , r ) )
     out[ tail( r ) ].push_back( r );
   return( out );
   };

  // the optimal vertices, through the cycles of zero reduced cost
  std::vector< State > optimal( 1 , s );
  give( s );
  std::vector< Cost > dist;
  std::vector< int > pred;
  for( std::size_t i = 0 ; i != optimal.size() &&
	                   int( _flows.size() ) < k ; ++i ) {
   const State x = optimal[ i ];
   const auto out = outOf( x );
   for( int v = 0 ; v != nn && int( _flows.size() ) < k ; ++v ) {
    paths( out , v , true , dist , pred );
    for( int r = 0 ; r != rr && int( _flows.size() ) < k ; ++r ) {
     if( head( r ) != v || rc[ r ] > ceps || ! open( x , r ) ||
	 ( tail( r ) != v && pred[ tail( r ) ] < 0 ) )
      continue;
     State y;
     if( step( x , r , pred , y ) && y.cost <= s.cost + ceps && give( y ) )
      optimal.push_back( y );
     }
    }
   }

  // the vertices one cycle of least reduced cost away from an optimal one,
  // the best ones kept
  std::multimap< Cost , std::vector< Value > > cand;
  const std::size_t want = std::size_t( k ) - _flows.size();
  for( std::size_t i = 0 ; i != optimal.size() && want > 0 ; ++i ) {
   const State & x = optimal[ i ];
   const auto out = outOf( x );
   for( int v = 0 ; v != nn ; ++v ) {
    paths( out , v , false , dist , pred );
    for( int r = 0 ; r != rr ; ++r ) {
     if( head( r ) != v || ! open( x , r ) ||
	 ( tail( r ) != v && pred[ tail( r ) ] < 0 ) ||
	 rc[ r ] + ( tail( r ) != v ? dist[ tail( r ) ] : Cost( 0 ) ) <= ceps )
      continue;
     State y;
     if( ! step( x , r , pred , y ) || ( bounded && y.cost > limit ) ||
	 y.cost <= s.cost + ceps )
      continue;
     std::vector< Value > f( y.flow.begin() , y.flow.begin() + _m );
     if( seen.count( f ) )
      continue;
     cand.insert( std::make_pair( y.cost , f ) );
     while( cand.size() > 2 * want )
      cand.erase( std::prev( cand.end() ) );
     }
    }
   }
  for( auto it = cand.begin() ; it != cand.end() &&
	                         int( _flows.size() ) < k ; ++it )
   if( seen.insert( it->second ).second ) {
    _flows.push_back( it->second );
    _costs.push_back( it->first );
    }
  return( int( _flows.size() ) );
  }

 // Whether the arc i is strictly within its bounds
 bool isFree( const State & s , int i ) const {
  return( s.flow[ i ] > _lower[ i ] + _feps &&
	  ( _inf[ i ] || s.flow[ i ] < _upper[ i ] - _feps ) );
  }

 // Whether flow may go along the residual arc r (2 i forward, 2 i + 1
 // backward), and how much
 bool open( const State & s , int r ) const {
  const int i = r / 2;
  if( r % 2 == 0 )
   return( _inf[ i ] || s.flow[ i ] < _upper[ i ] - _feps );
  return( s.flow[ i ] > _lower[ i ] + _feps );
  }

 bool residual( const State & s , int r , Value & amount ) const {
  const int i = r / 2;
  if( r % 2 == 0 ) {
   if( _inf[ i ] )
    return( false );
   amount = _upper[ i ] - s.flow[ i ];
   }
  else
   amount = s.flow[ i ] - _lower[ i ];
  return( true );
  }

 int tail( int r ) const {
  return( r % 2 == 0 ? _source[ r / 2 ] : _target[ r / 2 ] );
  }

 int head( int r ) const {
  return( r % 2 == 0 ? _target[ r / 2 ] : _source[ r / 2 ] );
  }

 Cost resCost( int r ) const {
  return( r % 2 == 0 ? _cost[ r / 2 ] : -_cost[ r / 2 ] );
  }

 // The first state: an optimal flow, made a vertex, with a basis
 bool firstState( State & s ) {
  std::vector< Value > f( _m , 0 );
  _feps = 0;
  {
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
   if( ! _has_init && ns.run() != NS::OPTIMAL )
    return( false );
   // the tolerance on the flows of NetworkSimplex: zero for an exact
   // type, otherwise relative to the largest supply or finite bound
   if( ! std::numeric_limits< Value >::is_exact ) {
    Value big = 1;
    for( int i = 0 ; i != _m ; ++i ) {
     big = ( std::max )( big , Value( _lower[ i ] < 0 ? -_lower[ i ]
						       : _lower[ i ] ) );
     if( ! _inf[ i ] )
      big = ( std::max )( big , Value( _upper[ i ] < 0 ? -_upper[ i ]
							: _upper[ i ] ) );
     }
    for( int u = 0 ; u != _n ; ++u )
     big = ( std::max )( big , Value( _supply[ u ] < 0 ? -_supply[ u ]
						       : _supply[ u ] ) );
    _feps = static_cast< Value >( big *
				  std::numeric_limits< Value >::epsilon() *
				  1e6 );
    }
   if( _has_init )
    f = _init;
   else
    for( ArcIt a( _graph ) ; a != INVALID ; ++a )
     f[ _arc_id[ a ] ] = ns.flow( a );
   }
  s.flow.assign( _m + _n , 0 );
  std::copy( f.begin() , f.end() , s.flow.begin() );
  std::vector< Value > net( _n , 0 );
  for( int i = 0 ; i != _m ; ++i ) {
   net[ _source[ i ] ] += f[ i ];
   net[ _target[ i ] ] -= f[ i ];
   }
  for( int u = 0 ; u != _n ; ++u )
   s.flow[ _m + u ] = net[ u ] - _supply[ u ];
  purify( s );
  s.cost = 0;
  for( int i = 0 ; i != _m ; ++i )
   s.cost += _cost[ i ] * s.flow[ i ];
  basis( s );
  return( true );
  }

 // Union-find on the n + 1 nodes
 static int find( std::vector< int > & p , int u ) {
  while( p[ u ] != u )
   u = p[ u ] = p[ p[ u ] ];
  return( u );
  }

 // The path between u and v in the forest of the arcs in on, as residual
 // arcs from u to v; false if there is none
 bool forestPath( const State & s , const std::vector< char > & on , int u ,
		  int v , std::vector< int > & path ) const {
  const int nn = _n + 1;
  std::vector< int > pred( nn , -2 );
  std::vector< int > queue( 1 , u );
  pred[ u ] = -1;
  for( std::size_t h = 0 ; h != queue.size() && pred[ v ] == -2 ; ++h ) {
   const int w = queue[ h ];
   for( int i = 0 ; i != _m + _n ; ++i ) {
    if( ! on[ i ] )
     continue;
    int r = -1;
    if( _source[ i ] == w )
     r = 2 * i;
    else if( _target[ i ] == w )
     r = 2 * i + 1;
    if( r < 0 || pred[ head( r ) ] != -2 )
     continue;
    pred[ head( r ) ] = r;
    queue.push_back( head( r ) );
    }
   }
  if( pred[ v ] == -2 )
   return( false );
  path.clear();
  for( int w = v ; w != u ; w = tail( pred[ w ] ) )
   path.push_back( pred[ w ] );
  std::reverse( path.begin() , path.end() );
  ( void )s;
  return( true );
  }

 // The flow made a vertex: while the arcs strictly within their bounds
 // close a cycle, the cycle is saturated in a direction in which it is
 // bounded, that in which the cost does not grow first, so that the cost
 // stays as it is at an optimal flow, where such a cycle has a reduced
 // cost of zero, and does not grow elsewhere
 void purify( State & s ) const {
  const int nn = _n + 1;
  for( ; ; ) {
   std::vector< int > p( nn );
   for( int u = 0 ; u != nn ; ++u )
    p[ u ] = u;
   std::vector< char > on( _m + _n , 0 );
   int closing = -1;
   for( int i = 0 ; i != _m + _n && closing < 0 ; ++i ) {
    if( ! isFree( s , i ) )
     continue;
    const int a = find( p , _source[ i ] ) , b = find( p , _target[ i ] );
    if( a == b )
     closing = i;
    else {
     p[ a ] = b;
     on[ i ] = 1;
     }
    }
   if( closing < 0 )
    return;
   std::vector< int > cycle;
   forestPath( s , on , _target[ closing ] , _source[ closing ] , cycle );
   cycle.insert( cycle.begin() , 2 * closing );
   // the direction in which the cost does not grow is tried first
   Cost cc = 0;
   for( std::size_t h = 0 ; h != cycle.size() ; ++h )
    cc += resCost( cycle[ h ] );
   if( cc > 0 )
    for( std::size_t h = 0 ; h != cycle.size() ; ++h )
     cycle[ h ] ^= 1;
   Value amount = 0;
   bool bounded = false;
   for( int dir = 0 ; dir != 2 && ! bounded ; ++dir ) {
    if( dir == 1 )
     for( std::size_t h = 0 ; h != cycle.size() ; ++h )
      cycle[ h ] ^= 1;
    bounded = true;
    bool first = true;
    for( std::size_t h = 0 ; h != cycle.size() && bounded ; ++h ) {
     Value r;
     if( ! residual( s , cycle[ h ] , r ) )
      continue;
     if( first || r < amount )
      amount = r;
     first = false;
     }
    bounded = ! first;
    }
   for( std::size_t h = 0 ; h != cycle.size() ; ++h )
    s.flow[ cycle[ h ] / 2 ] += cycle[ h ] % 2 == 0 ? amount : -amount;
   }
  }

 // A basis of a vertex: the arcs strictly within their bounds, a forest,
 // completed to a spanning tree of the n + 1 nodes by other arcs
 void basis( State & s ) const {
  const int nn = _n + 1;
  std::vector< int > p( nn );
  for( int u = 0 ; u != nn ; ++u )
   p[ u ] = u;
  s.tree.assign( _m + _n , 0 );
  for( int pass = 0 ; pass != 2 ; ++pass )
   for( int i = 0 ; i != _m + _n ; ++i ) {
    if( s.tree[ i ] || ( pass == 0 && ! isFree( s , i ) ) )
     continue;
    const int a = find( p , _source[ i ] ) , b = find( p , _target[ i ] );
    if( a != b ) {
     p[ a ] = b;
     s.tree[ i ] = 1;
     }
    }
  }

 // The tree of a basis hung from the root: the residual arc from the
 // parent of each node to it, and the depth of each node
 void rootTree( const State & s , std::vector< int > & parent ,
		std::vector< int > & depth ) const {
  const int nn = _n + 1;
  parent.assign( nn , -1 );
  depth.assign( nn , -1 );
  std::vector< int > queue( 1 , _n );
  depth[ _n ] = 0;
  std::vector< std::vector< int > > adj( nn );
  for( int i = 0 ; i != _m + _n ; ++i )
   if( s.tree[ i ] ) {
    adj[ _source[ i ] ].push_back( 2 * i );
    adj[ _target[ i ] ].push_back( 2 * i + 1 );
    }
  for( std::size_t h = 0 ; h != queue.size() ; ++h ) {
   const int w = queue[ h ];
   for( std::size_t t = 0 ; t != adj[ w ].size() ; ++t ) {
    const int r = adj[ w ][ t ] , x = head( r );
    if( depth[ x ] >= 0 )
     continue;
    depth[ x ] = depth[ w ] + 1;
    parent[ x ] = r;
    queue.push_back( x );
    }
   }
  }

 // The cycle the nonbasic residual arc r closes with the tree
 void cycleOf( const std::vector< int > & parent ,
	       const std::vector< int > & depth , int r ,
	       std::vector< int > & cycle ) const {
  // the cycle: r from u to v, then the tree path from v back to u
  const int u = tail( r ) , v = head( r );
  std::vector< int > up_v , up_u;
  int a = v , b = u;
  while( depth[ a ] > depth[ b ] ) {
   up_v.push_back( parent[ a ] ^ 1 );
   a = tail( parent[ a ] );
   }
  while( depth[ b ] > depth[ a ] ) {
   up_u.push_back( parent[ b ] );
   b = tail( parent[ b ] );
   }
  while( a != b ) {
   up_v.push_back( parent[ a ] ^ 1 );
   a = tail( parent[ a ] );
   up_u.push_back( parent[ b ] );
   b = tail( parent[ b ] );
   }
  cycle.assign( 1 , r );
  cycle.insert( cycle.end() , up_v.begin() , up_v.end() );
  cycle.insert( cycle.end() , up_u.rbegin() , up_u.rend() );
  }

 // The amount the cycle of r carries when saturated, and the position of
 // the arc that leaves; false if the cycle has no bound
 bool saturate( const State & s , const std::vector< int > & cycle ,
		Value & amount , int & leave ) const {
  amount = 0;
  leave = -1;
  for( std::size_t h = 0 ; h != cycle.size() ; ++h ) {
   Value c;
   if( ! residual( s , cycle[ h ] , c ) )
    continue;
   if( leave < 0 || c < amount ) {
    amount = c;
    leave = int( h );
    }
   }
  return( leave >= 0 );
  }

 // The cost of the basis the pivot of r leads to, without making it
 bool pivotCost( const State & s , const std::vector< int > & parent ,
		 const std::vector< int > & depth , int r , Cost & cost ,
		 Value & amount ) const {
  std::vector< int > cycle;
  cycleOf( parent , depth , r , cycle );
  int leave;
  if( ! saturate( s , cycle , amount , leave ) )
   return( false );
  Cost dc = 0;
  for( std::size_t h = 0 ; h != cycle.size() ; ++h )
   dc += resCost( cycle[ h ] );
  cost = s.cost + dc * amount;
  return( true );
  }

 // The pivot of the nonbasic residual arc r: the cycle it closes with the
 // tree, saturated; false if the cycle has no bound (a ray, not an edge)
 bool pivot( const State & s , const std::vector< int > & parent ,
	     const std::vector< int > & depth , int r , State & nx ) const {
  std::vector< int > cycle;
  cycleOf( parent , depth , r , cycle );
  Value amount;
  int leave;
  if( ! saturate( s , cycle , amount , leave ) )
   return( false );
  nx = s;
  Cost dc = 0;
  for( std::size_t h = 0 ; h != cycle.size() ; ++h ) {
   const int x = cycle[ h ];
   nx.flow[ x / 2 ] += x % 2 == 0 ? amount : -amount;
   dc += resCost( x );
   }
  nx.cost = s.cost + dc * amount;
  if( leave != 0 ) {
   nx.tree[ r / 2 ] = 1;
   nx.tree[ cycle[ leave ] / 2 ] = 0;
   }
  return( true );
  }
 };

}

#endif
