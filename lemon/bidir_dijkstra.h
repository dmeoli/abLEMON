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

#include <vector>

#include <lemon/bin_heap.h>
#include <lemon/core.h>
#include <lemon/path.h>

///\ingroup shortest_path
///\file
///\brief Bidirectional Dijkstra algorithm.

namespace lemon
{

/// \brief The default traits class of \ref BidirDijkstra.
///
/// The heaps of its two searches are \ref BinHeap "BinHeaps" on the
/// lengths, with node maps as cross reference maps.
template< typename GR , typename LEN >
struct BidirDijkstraDefaultTraits
{
 /// The type of the cross reference maps of the heaps
 typedef typename GR::template NodeMap< int > HeapCrossRef;
 /// The type of the heaps of the two searches
 typedef BinHeap< typename LEN::Value , HeapCrossRef > Heap;
 };

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
/// \tparam TR The traits class, which gives the type of the heaps of the
/// two searches [see \ref BidirDijkstraDefaultTraits and \ref SetHeap].
template< typename GR , typename LEN = typename GR::template ArcMap< int > ,
	  typename TR = BidirDijkstraDefaultTraits< GR , LEN > >
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
 /// The type of the cross reference maps of the heaps
 typedef typename TR::HeapCrossRef HeapCrossRef;
 /// The type of the heaps of the two searches
 typedef typename TR::Heap Heap;

 /// \brief \ref named-templ-param "Named parameter" for setting the type
 /// of the heaps.
 ///
 /// The heaps of the two searches are of type \c H, with cross
 /// reference maps of type \c CR, e.g., a \ref FibHeap or a \ref
 /// RadixHeap in place of the \ref BinHeap of the default.
 template< typename H ,
	   typename CR = typename Digraph::template NodeMap< int > >
 struct SetHeap
 {
  struct Traits : public TR
  {
   typedef CR HeapCrossRef;
   typedef H Heap;
   };
  typedef BidirDijkstra< GR , LEN , Traits > Create;
  };

 private:
 TEMPLATE_DIGRAPH_TYPEDEFS( Digraph );

 // one of the two searches: the tentative distances and the arcs that
 // reach the nodes, the heap telling those seen (in or past it) and those
 // finished (past it)
 struct Search
 {
  typename Digraph::template NodeMap< Value > dist;
  typename Digraph::template NodeMap< Arc > pred;
  HeapCrossRef cross;
  Heap heap;
  explicit Search( const Digraph & g )
      : dist( g ) , pred( g , INVALID ) , cross( g , Heap::PRE_HEAP ) ,
	heap( cross ) { }
  bool seen( const Node & y ) const {
   return( heap.state( y ) != Heap::PRE_HEAP );
   }
  bool done( const Node & y ) const {
   return( heap.state( y ) == Heap::POST_HEAP );
   }
  };

 const Digraph & _graph;
 const LengthMap & _length;
 Search _fwd , _bwd;
 std::vector< Node > _touched;
 Path _path;
 Value _dist;

 // the node y reached by the search at distance d along the arc a
 void relax( Search & s , const Node & y , Value d , const Arc & a ) {
  if( s.done( y ) ) {
   return;
   }
  if( ! s.seen( y ) ) {
   _touched.push_back( y );
   s.heap.push( y , d );
   }
  else if( d < s.dist[ y ] ) {
   s.heap.decrease( y , d );
   }
  else {
   return;
   }
  s.dist[ y ] = d;
  s.pred[ y ] = a;
  }

 void reset() {
  _fwd.heap.clear();
  _bwd.heap.clear();
  for( std::size_t i = 0 ; i < _touched.size() ; ++i ) {
   Node n = _touched[ i ];
   _fwd.cross[ n ] = _bwd.cross[ n ] = Heap::PRE_HEAP;
   _fwd.pred[ n ] = _bwd.pred[ n ] = INVALID;
   }
  _touched.clear();
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
  while( ! _fwd.heap.empty() && ! _bwd.heap.empty() ) {
   Value top_fwd = _fwd.heap.prio() , top_bwd = _bwd.heap.prio();
   if( found && top_fwd + top_bwd >= _dist ) {
    break;
    }
   bool forward = top_fwd <= top_bwd;
   Search & s = forward ? _fwd : _bwd;
   Search & o = forward ? _bwd : _fwd;
   Node x = s.heap.top();
   s.heap.pop();
   if( forward ) {
    for( OutArcIt a( _graph , x ) ; a != INVALID ; ++a ) {
     Node y = _graph.target( a );
     Value d = s.dist[ x ] + _length[ a ];
     relax( s , y , d , a );
     if( o.seen( y ) && ( ! found || d + o.dist[ y ] < _dist ) ) {
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
     if( o.seen( y ) && ( ! found || d + o.dist[ y ] < _dist ) ) {
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
