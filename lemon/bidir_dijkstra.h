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

#ifndef LEMON_BIDIR_DIJKSTRA_H
 #define LEMON_BIDIR_DIJKSTRA_H

#include <functional>
#include <queue>
#include <utility>
#include <vector>

#include <lemon/core.h>
#include <lemon/path.h>

///\ingroup shortest_path
///\file
///\brief Bidirectional Dijkstra algorithm.

namespace lemon
{

/// \ingroup shortest_path
///
/// \brief Bidirectional Dijkstra algorithm for a shortest path between
/// two nodes
///
/// This class finds a shortest path from a source to a target node of a
/// digraph with nonnegative arc lengths, by two searches of Dijkstra's
/// algorithm, a forward one from the source along the arcs and a backward
/// one from the target against them, which grow alternately, the one whose
/// smallest tentative distance is smaller. Each arc that joins the two
/// searches gives a path, and the shortest one found is optimal as soon as
/// the smallest tentative distances of the two searches sum to at least
/// its length; the searches then stop, usually having scanned far fewer
/// nodes than a search from the source alone.
///
/// One object answers any number of queries, each with \ref run(), in
/// time proportional to the part of the digraph that it scans.
///
/// \tparam GR The digraph type the algorithm runs on.
/// \tparam LEN The type of the map of the arc lengths, whose values must
/// be nonnegative.
template< typename GR , typename LEN = typename GR::template ArcMap< int > >
class BidirDijkstra
{
 public:
 /// The digraph type of the algorithm
 typedef GR Digraph;
 /// The type of the map of the arc lengths
 typedef LEN LengthMap;
 /// The type of the lengths
 typedef typename LengthMap::Value Value;
 /// The type of the shortest path
 typedef lemon::Path< Digraph > Path;

 private:
 TEMPLATE_DIGRAPH_TYPEDEFS( Digraph );

 typedef std::pair< Value , int > Item;
 typedef std::priority_queue< Item , std::vector< Item > ,
   std::greater< Item > >
   Heap;

 // one of the two searches: the tentative distances, the arcs that reach
 // the nodes, which are seen or finished
 struct Search
 {
  typename Digraph::template NodeMap< Value > dist;
  typename Digraph::template NodeMap< Arc > pred;
  BoolNodeMap seen , done;
  Heap heap;
  explicit Search( const Digraph & g )
      : dist( g ) , pred( g , INVALID ) , seen( g , false ) , done( g ,
								    false ) { }
  };

 const Digraph & _graph;
 const LengthMap & _length;
 Search _fwd , _bwd;
 std::vector< Node > _touched;
 Path _path;
 Value _dist;

 // the node y reached by the search at distance d along the arc a
 void relax( Search & s , const Node & y , Value d , const Arc & a ) {
  if( s.done[ y ] ) {
   return;
   }
  if( ! s.seen[ y ] ) {
   s.seen[ y ] = true;
   _touched.push_back( y );
   }
  else if( d >= s.dist[ y ] ) {
   return;
   }
  s.dist[ y ] = d;
  s.pred[ y ] = a;
  s.heap.push( Item( d , _graph.id( y ) ) );
  }

 // the stale entries off the top of the heap; false if it is empty
 bool clean( Search & s ) {
  while( ! s.heap.empty() ) {
   Node x = _graph.nodeFromId( s.heap.top().second );
   if( ! s.done[ x ] && s.heap.top().first == s.dist[ x ] ) {
    return( true );
    }
   s.heap.pop();
   }
  return( false );
  }

 void reset() {
  for( std::size_t i = 0 ; i < _touched.size() ; ++i ) {
   Node n = _touched[ i ];
   _fwd.seen[ n ] = _fwd.done[ n ] = false;
   _bwd.seen[ n ] = _bwd.done[ n ] = false;
   _fwd.pred[ n ] = _bwd.pred[ n ] = INVALID;
   }
  _touched.clear();
  _fwd.heap = Heap();
  _bwd.heap = Heap();
  _path.clear();
  }

 public:
 /// \brief Constructor
 ///
 /// Constructor.
 ///
 /// \param graph The digraph the algorithm runs on.
 /// \param length The lengths of the arcs, which must be nonnegative.
 BidirDijkstra( const Digraph & graph , const LengthMap & length )
     : _graph( graph ) , _length( length ) , _fwd( graph ) , _bwd( graph ) ,
       _dist( 0 ) { }

 /// \brief Find a shortest path from \c s to \c t.
 ///
 /// \return \c true if \c t is reachable from \c s; its distance and a
 /// shortest path are then given by \ref dist() and \ref path().
 bool run( const Node & s , const Node & t ) {
  reset();
  _dist = 0;
  if( s == t ) {
   return( true );
   }
  relax( _fwd , s , 0 , INVALID );
  relax( _bwd , t , 0 , INVALID );
  bool found = false;
  Arc meet = INVALID;
  while( clean( _fwd ) && clean( _bwd ) ) {
   Value top_fwd = _fwd.heap.top().first , top_bwd = _bwd.heap.top().first;
   if( found && top_fwd + top_bwd >= _dist ) {
    break;
    }
   bool forward = top_fwd <= top_bwd;
   Search & s = forward ? _fwd : _bwd;
   Search & o = forward ? _bwd : _fwd;
   Node x = _graph.nodeFromId( s.heap.top().second );
   s.heap.pop();
   s.done[ x ] = true;
   if( forward ) {
    for( OutArcIt a( _graph , x ) ; a != INVALID ; ++a ) {
     Node y = _graph.target( a );
     Value d = s.dist[ x ] + _length[ a ];
     relax( s , y , d , a );
     if( o.seen[ y ] && ( ! found || d + o.dist[ y ] < _dist ) ) {
      found = true;
      _dist = d + o.dist[ y ];
      meet = a;
      }
     }
    }
   else {
    for( InArcIt a( _graph , x ) ; a != INVALID ; ++a ) {
     Node y = _graph.source( a );
     Value d = s.dist[ x ] + _length[ a ];
     relax( s , y , d , a );
     if( o.seen[ y ] && ( ! found || d + o.dist[ y ] < _dist ) ) {
      found = true;
      _dist = d + o.dist[ y ];
      meet = a;
      }
     }
    }
   }
  if( ! found ) {
   return( false );
   }
  // the forward tree up to the arc that met the other search, the arc,
  // and the backward tree from it on
  for( Node v = _graph.source( meet ) ; _fwd.pred[ v ] != INVALID ;
       v = _graph.source( _fwd.pred[ v ] ) ) {
   _path.addFront( _fwd.pred[ v ] );
   }
  _path.addBack( meet );
  for( Node v = _graph.target( meet ) ; _bwd.pred[ v ] != INVALID ;
       v = _graph.target( _bwd.pred[ v ] ) ) {
   _path.addBack( _bwd.pred[ v ] );
   }
  return( true );
  }

 /// \name Query Functions
 /// The results of the last \ref run() that returned \c true.

 ///@{

 /// \brief Return the length of a shortest path.
 Value dist() const { return( _dist ); }

 /// \brief Return a shortest path, empty if the source is the target.
 const Path & path() const { return( _path ); }

 ///@}
 };

}  //END OF NAMESPACE LEMON

#endif  //LEMON_BIDIR_DIJKSTRA_H
