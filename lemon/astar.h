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

#ifndef LEMON_ASTAR_H
 #define LEMON_ASTAR_H

#include <vector>

#include <lemon/bin_heap.h>
#include <lemon/core.h>
#include <lemon/path.h>

///\ingroup shortest_path
///\file
///\brief A* algorithm.

namespace lemon
{

/// \brief The default traits class of \ref AStar.
///
/// The heap is a \ref BinHeap on the tentative distances plus the
/// heuristic values, with a node map as cross reference map.
template< typename GR , typename LEN >
struct AStarDefaultTraits
{
 /// The type of the cross reference map of the heap
 typedef typename GR::template NodeMap< int > HeapCrossRef;
 /// The type of the heap
 typedef BinHeap< typename LEN::Value , HeapCrossRef > Heap;
 };

/// \ingroup shortest_path
///
/// \brief A* algorithm for a shortest path between two nodes
///
/// This class finds a shortest path from a source to a target node of a
/// digraph with nonnegative arc lengths, by Dijkstra's algorithm guided by
/// a heuristic: the nodes are scanned in the order of their tentative
/// distance from the source plus the heuristic value, an estimate of their
/// distance to the target, and the search stops when the target is
/// scanned. The heuristic must be admissible, i.e., never larger than the
/// distance to the target (e.g., the straight-line distance on a road
/// network), and the closer it is to that distance, the fewer nodes are
/// scanned; with a heuristic that is everywhere zero, it is Dijkstra's
/// algorithm stopped at the target. A heuristic that is also consistent,
/// i.e., for which \f$h(u) \leq \ell(uv) + h(v)\f$ on every arc, scans each
/// node at most once; with an admissible one that is not, a node whose
/// distance improves after its scan is scanned again.
///
/// \tparam GR The digraph type the algorithm runs on.
/// \tparam LEN The type of the map of the arc lengths, whose values must
/// be nonnegative.
/// \tparam HM The type of the map of the heuristic values of the nodes,
/// whose value type is that of the lengths.
/// \tparam TR The traits class, which gives the type of the heap [see
/// \ref AStarDefaultTraits and \ref SetHeap].
template< typename GR , typename LEN , typename HM ,
	  typename TR = AStarDefaultTraits< GR , LEN > >
class AStar
{
 public:
 /// The digraph type of the algorithm
 typedef GR Digraph;
 /// The type of the map of the arc lengths
 typedef LEN LengthMap;
 /// The type of the map of the heuristic values
 typedef HM HeuristicMap;
 /// The type of the lengths
 typedef typename LengthMap::Value Value;
 /// The type of the shortest path
 typedef lemon::Path< Digraph > Path;
 /// The type of the cross reference map of the heap
 typedef typename TR::HeapCrossRef HeapCrossRef;
 /// The type of the heap
 typedef typename TR::Heap Heap;

 /// \brief \ref named-templ-param "Named parameter" for setting the type
 /// of the heap.
 ///
 /// The heap is of type \c H, with a cross reference map of type \c CR.
 template< typename H ,
	   typename CR = typename Digraph::template NodeMap< int > >
 struct SetHeap
 {
  struct Traits : public TR
  {
   typedef CR HeapCrossRef;
   typedef H Heap;
   };
  typedef AStar< GR , LEN , HM , Traits > Create;
  };

 private:
 TEMPLATE_DIGRAPH_TYPEDEFS( Digraph );


 const Digraph & _graph;
 const LengthMap & _length;
 const HeuristicMap & _heuristic;
 typename Digraph::template NodeMap< Value > _dist;
 typename Digraph::template NodeMap< Arc > _pred;
 BoolNodeMap _seen;
 HeapCrossRef _cross;
 Heap _heap;
 std::vector< Node > _touched;
 Path _path;
 Value _target;
 int _scanned;

 public:
 /// \brief Constructor
 ///
 /// Constructor.
 ///
 /// \param graph The digraph the algorithm runs on.
 /// \param length The lengths of the arcs, which must be nonnegative.
 /// \param heuristic The heuristic values of the nodes, which must not
 /// exceed their distances to the target of each \ref run().
 AStar( const Digraph & graph , const LengthMap & length ,
	const HeuristicMap & heuristic )
     : _graph( graph ) , _length( length ) , _heuristic( heuristic ) ,
       _dist( graph ) , _pred( graph , INVALID ) , _seen( graph , false ) ,
       _cross( graph , Heap::PRE_HEAP ) , _heap( _cross ) , _target( 0 ) ,
       _scanned( 0 ) { }

 /// \brief Find a shortest path from \c s to \c t.
 ///
 /// \return \c true if \c t is reachable from \c s; its distance and a
 /// shortest path are then given by \ref dist() and \ref path().
 bool run( const Node & s , const Node & t ) {
  _heap.clear();
  for( std::size_t i = 0 ; i < _touched.size() ; ++i ) {
   _seen[ _touched[ i ] ] = false;
   _pred[ _touched[ i ] ] = INVALID;
   _cross[ _touched[ i ] ] = Heap::PRE_HEAP;
   }
  _touched.clear();
  _path.clear();
  _scanned = 0;
  _seen[ s ] = true;
  _touched.push_back( s );
  _dist[ s ] = 0;
  _heap.push( s , _heuristic[ s ] );
  while( ! _heap.empty() ) {
   Node x = _heap.top();
   _heap.pop();
   ++_scanned;
   if( x == t ) {
    _target = _dist[ t ];
    for( Node v = t ; _pred[ v ] != INVALID ;
	 v = _graph.source( _pred[ v ] ) ) {
     _path.addFront( _pred[ v ] );
     }
    return( true );
    }
   for( OutArcIt a( _graph , x ) ; a != INVALID ; ++a ) {
    Node y = _graph.target( a );
    Value d = _dist[ x ] + _length[ a ];
    if( ! _seen[ y ] ) {
     _seen[ y ] = true;
     _touched.push_back( y );
     _heap.push( y , d + _heuristic[ y ] );
     }
    else if( d >= _dist[ y ] ) {
     continue;
     }
    else if( _heap.state( y ) == Heap::IN_HEAP ) {
     _heap.decrease( y , d + _heuristic[ y ] );
     }
    else {  // scanned already, with a heuristic that is not consistent
     _heap.push( y , d + _heuristic[ y ] );
     }
    _dist[ y ] = d;
    _pred[ y ] = a;
    }
   }
  return( false );
  }

 /// \name Query Functions
 /// The results of the last \ref run() that returned \c true.

 ///@{

 /// \brief Return the length of a shortest path.
 Value dist() const { return( _target ); }

 /// \brief Return a shortest path, empty if the source is the target.
 const Path & path() const { return( _path ); }

 /// \brief Return the number of scans of nodes, a node scanned again
 /// counted again.
 int scanned() const { return( _scanned ); }

 ///@}
 };

}  //END OF NAMESPACE LEMON

#endif  //LEMON_ASTAR_H
