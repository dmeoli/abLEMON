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

#include <functional>
#include <queue>
#include <utility>
#include <vector>

#include <lemon/core.h>
#include <lemon/path.h>

///\ingroup shortest_path
///\file
///\brief A* algorithm.

namespace lemon
{

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
template< typename GR , typename LEN , typename HM >
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

 private:
 TEMPLATE_DIGRAPH_TYPEDEFS( Digraph );

 typedef std::pair< Value , int > Item;
 typedef std::priority_queue< Item , std::vector< Item > ,
   std::greater< Item > >
   Heap;

 const Digraph & _graph;
 const LengthMap & _length;
 const HeuristicMap & _heuristic;
 typename Digraph::template NodeMap< Value > _dist;
 typename Digraph::template NodeMap< Arc > _pred;
 BoolNodeMap _seen;
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
       _target( 0 ) , _scanned( 0 ) { }

 /// \brief Find a shortest path from \c s to \c t.
 ///
 /// \return \c true if \c t is reachable from \c s; its distance and a
 /// shortest path are then given by \ref dist() and \ref path().
 bool run( const Node & s , const Node & t ) {
  for( std::size_t i = 0 ; i < _touched.size() ; ++i ) {
   _seen[ _touched[ i ] ] = false;
   _pred[ _touched[ i ] ] = INVALID;
   }
  _touched.clear();
  _path.clear();
  _scanned = 0;
  Heap heap;
  _seen[ s ] = true;
  _touched.push_back( s );
  _dist[ s ] = 0;
  heap.push( Item( _heuristic[ s ] , _graph.id( s ) ) );
  while( ! heap.empty() ) {
   Item top = heap.top();
   heap.pop();
   Node x = _graph.nodeFromId( top.second );
   if( top.first != _dist[ x ] + _heuristic[ x ] ) {
    continue;  // a stale entry, of a distance improved since
    }
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
     }
    else if( d >= _dist[ y ] ) {
     continue;
     }
    _dist[ y ] = d;
    _pred[ y ] = a;
    heap.push( Item( d + _heuristic[ y ] , _graph.id( y ) ) );
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
