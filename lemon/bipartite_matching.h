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

#ifndef LEMON_BIPARTITE_MATCHING_H
 #define LEMON_BIPARTITE_MATCHING_H

#include <functional>
#include <limits>
#include <queue>
#include <utility>
#include <vector>

#include <lemon/core.h>

///\ingroup matching
///\file
///\brief Maximum matching and minimum cost assignment in bipartite graphs.

namespace lemon
{

/// \ingroup matching
///
/// \brief Maximum cardinality matching in bipartite graphs
///
/// This class implements the algorithm of Hopcroft and Karp, which finds a
/// maximum cardinality matching of a bipartite graph in
/// \f$O(m\sqrt{n})\f$ time: in each phase, a breadth-first search from the
/// unmatched red nodes finds the length of the shortest augmenting paths,
/// and a depth-first search augments along a maximal set of disjoint ones
/// of that length. It can be started from the empty matching or from a
/// greedy one.
///
/// The dual solution is a minimum vertex cover, whose size equals that of
/// the matching (König's theorem): with the nodes that an alternating path
/// from an unmatched red node reaches, it is made of the red nodes not
/// reached and the blue nodes reached [see cover()].
///
/// \tparam BPGR The bipartite graph type the algorithm runs on.
template< typename BPGR >
class HopcroftKarp
{
 public:
 /// The bipartite graph type of the algorithm
 typedef BPGR BpGraph;
 /// The type of the matching map
 typedef typename BpGraph::template NodeMap< typename BpGraph::Edge >
   MatchingMap;

 private:
 TEMPLATE_BPGRAPH_TYPEDEFS( BpGraph );

 const BpGraph & _graph;
 MatchingMap _matching;
 IntRedNodeMap _dist;
 typename BpGraph::template RedNodeMap< IncEdgeIt > _current;
 BoolNodeMap _cover;
 int _size;

 static const int INF = std::numeric_limits< int >::max();

 // the red node matched to the blue node v, or INVALID
 Node mateOf( const BlueNode & v ) const {
  return( _matching[ v ] == INVALID
	     ? Node( INVALID )
	     : Node( _graph.redNode( _matching[ v ] ) ) );
  }

 // the lengths of the alternating paths from the unmatched red nodes to
 // the red nodes; true if an augmenting path exists
 bool bfs() {
  std::queue< RedNode > queue;
  for( RedNodeIt u( _graph ) ; u != INVALID ; ++u ) {
   if( _matching[ u ] == INVALID ) {
    _dist[ u ] = 0;
    queue.push( u );
    }
   else {
    _dist[ u ] = INF;
    }
   }
  bool found = false;
  while( ! queue.empty() ) {
   RedNode u = queue.front();
   queue.pop();
   for( IncEdgeIt e( _graph , u ) ; e != INVALID ; ++e ) {
    Node w = mateOf( _graph.blueNode( e ) );
    if( w == INVALID ) {
     found = true;
     }
    else if( _dist[ _graph.asRedNodeUnsafe( w ) ] == INF ) {
     _dist[ _graph.asRedNodeUnsafe( w ) ] = _dist[ u ] + 1;
     queue.push( _graph.asRedNodeUnsafe( w ) );
     }
    }
   }
  return( found );
  }

 // a shortest augmenting path from the unmatched red node s along the
 // layers of bfs(), followed without recursion; each red node keeps the
 // edge it has reached, and one that leads nowhere leaves its layer
 bool dfs( const RedNode & s ) {
  std::vector< RedNode > stack( 1 , s );
  while( ! stack.empty() ) {
   RedNode x = stack.back();
   bool advanced = false;
   for( ; _current[ x ] != INVALID ; ++_current[ x ] ) {
    Node w = mateOf( _graph.blueNode( _current[ x ] ) );
    if( w == INVALID ) {
     for( std::size_t i = stack.size() ; i-- > 0 ; ) {
      Edge e = _current[ stack[ i ] ];
      _matching[ stack[ i ] ] = e;
      _matching[ _graph.blueNode( e ) ] = e;
      }
     return( true );
     }
    RedNode y = _graph.asRedNodeUnsafe( w );
    if( _dist[ y ] == _dist[ x ] + 1 ) {
     stack.push_back( y );
     advanced = true;
     break;
     }
    }
   if( ! advanced ) {
    _dist[ x ] = INF;
    stack.pop_back();
    if( ! stack.empty() ) {
     ++_current[ stack.back() ];
     }
    }
   }
  return( false );
  }

 // the minimum vertex cover of König's theorem
 void findCover() {
  BoolNodeMap reached( _graph , false );
  std::queue< RedNode > queue;
  for( RedNodeIt u( _graph ) ; u != INVALID ; ++u ) {
   if( _matching[ u ] == INVALID ) {
    reached[ u ] = true;
    queue.push( u );
    }
   }
  while( ! queue.empty() ) {
   RedNode u = queue.front();
   queue.pop();
   for( IncEdgeIt e( _graph , u ) ; e != INVALID ; ++e ) {
    BlueNode v = _graph.blueNode( e );
    if( reached[ v ] || _matching[ v ] == INVALID ) {
     continue;
     }
    reached[ v ] = true;
    RedNode w = _graph.redNode( _matching[ v ] );
    if( ! reached[ w ] ) {
     reached[ w ] = true;
     queue.push( w );
     }
    }
   }
  for( NodeIt n( _graph ) ; n != INVALID ; ++n ) {
   _cover[ n ] = _graph.red( n ) ? ! reached[ n ] : bool( reached[ n ] );
   }
  }

 public:
 /// \brief Constructor
 ///
 /// Constructor.
 explicit HopcroftKarp( const BpGraph & graph )
     : _graph( graph ) , _matching( graph , INVALID ) , _dist( graph ) ,
       _current( graph ) , _cover( graph , false ) , _size( 0 ) { }

 /// \name Execution Control
 /// The simplest way to execute the algorithm is to use the \c run()
 /// member function.\n
 /// If you need better control on the execution, you have to call
 /// one of the functions \ref init() or \ref greedyInit() first, then
 /// \ref start().

 ///@{

 /// \brief Set the initial matching to the empty matching.
 void init() {
  for( NodeIt n( _graph ) ; n != INVALID ; ++n ) {
   _matching[ n ] = INVALID;
   }
  _size = 0;
  }

 /// \brief Find an initial matching in a greedy way.
 ///
 /// This function matches each red node to the first unmatched blue node
 /// among its neighbours, if any.
 void greedyInit() {
  init();
  for( RedNodeIt u( _graph ) ; u != INVALID ; ++u ) {
   for( IncEdgeIt e( _graph , u ) ; e != INVALID ; ++e ) {
    if( _matching[ _graph.blueNode( e ) ] == INVALID ) {
     _matching[ u ] = e;
     _matching[ _graph.blueNode( e ) ] = e;
     ++_size;
     break;
     }
    }
   }
  }

 /// \brief Augment the initial matching to a maximum one.
 ///
 /// This function augments the initial matching, set by \ref init() or
 /// \ref greedyInit(), until it is maximum, and finds the vertex cover
 /// that proves it.
 void start() {
  while( bfs() ) {
   for( RedNodeIt u( _graph ) ; u != INVALID ; ++u ) {
    _current[ u ] = IncEdgeIt( _graph , u );
    }
   for( RedNodeIt u( _graph ) ; u != INVALID ; ++u ) {
    if( _matching[ u ] == INVALID && dfs( u ) ) {
     ++_size;
     }
    }
   }
  findCover();
  }

 /// \brief Run the algorithm.
 ///
 /// This function runs the algorithm from a greedy initial matching.
 ///
 /// \note mm.run() is just a shortcut of the following code.
 /// \code
 /// mm.greedyInit();
 /// mm.start();
 /// \endcode
 void run() {
  greedyInit();
  start();
  }

 ///@}

 /// \name Primal Solution
 /// Functions to get the primal solution, i.e., the maximum matching.

 ///@{

 /// \brief Return the size (cardinality) of the matching.
 int matchingSize() const { return( _size ); }

 /// \brief Return \c true if the given edge is in the matching.
 bool matching( const Edge & edge ) const {
  return( _matching[ _graph.redNode( edge ) ] == edge );
  }

 /// \brief Return the matching edge incident to the given node, or
 /// \c INVALID if the node is not covered by the matching.
 Edge matching( const Node & n ) const { return( _matching[ n ] ); }

 /// \brief Return a const reference to the matching map.
 ///
 /// This function returns a const reference to a node map that stores
 /// the matching edge incident to each node.
 const MatchingMap & matchingMap() const { return( _matching ); }

 /// \brief Return the mate of the given node, or \c INVALID if it is not
 /// covered by the matching.
 Node mate( const Node & n ) const {
  return( _matching[ n ] == INVALID
	     ? Node( INVALID )
	     : _graph.oppositeNode( n , _matching[ n ] ) );
  }

 ///@}

 /// \name Dual Solution
 /// Functions to get the dual solution, i.e., a minimum vertex cover.

 ///@{

 /// \brief Return \c true if the given node is in the minimum vertex
 /// cover, whose size equals that of the maximum matching.
 bool cover( const Node & n ) const { return( _cover[ n ] ); }

 ///@}
 };

/// \ingroup matching
///
/// \brief Minimum cost assignment in bipartite graphs
///
/// This class finds a matching of minimum cost among those that cover every
/// red node of a bipartite graph, possibly not complete and with any number
/// of blue nodes, if one exists. It implements the method of successive
/// shortest augmenting paths: each red node is matched in turn along a
/// shortest path of the residual graph, found by Dijkstra's algorithm on
/// the costs reduced by node potentials, which the method keeps
/// nonnegative; it takes \f$O(n_r\, m \log n)\f$ time, where \f$n_r\f$ is
/// the number of red nodes.
///
/// The potentials are the dual solution: for every edge \f$e\f$ between
/// the red node \f$r\f$ and the blue node \f$b\f$,
/// \f$c(e) \geq \pi(b) - \pi(r)\f$, with equality on the matching, and the
/// blue nodes left unmatched have the same potential, the largest of the
/// blue nodes [see potential()].
///
/// \tparam BPGR The bipartite graph type the algorithm runs on.
/// \tparam CM The type of the cost map, whose values must be of a signed
/// arithmetic type.
template< typename BPGR ,
	  typename CM = typename BPGR::template EdgeMap< int > >
class MinCostAssignment
{
 public:
 /// The bipartite graph type of the algorithm
 typedef BPGR BpGraph;
 /// The type of the cost map
 typedef CM CostMap;
 /// The type of the costs
 typedef typename CostMap::Value Value;
 /// The type of the matching map
 typedef typename BpGraph::template NodeMap< typename BpGraph::Edge >
   MatchingMap;

 private:
 TEMPLATE_BPGRAPH_TYPEDEFS( BpGraph );

 const BpGraph & _graph;
 const CostMap & _cost;
 MatchingMap _matching;
 typename BpGraph::template NodeMap< Value > _pot , _dist;
 BoolNodeMap _seen , _done;
 typename BpGraph::template NodeMap< Edge > _pred;
 Value _total;

 typedef std::pair< Value , int > Item;
 typedef std::priority_queue< Item , std::vector< Item > ,
			      std::greater< Item > >
   Heap;

 // the node y reached at distance d along the edge e, unless it is
 // finished or already reached at no larger distance
 void relax( const Node & y , Value d , const Edge & e ,
	     std::vector< Node > & reached , Heap & heap ) {
  if( _done[ y ] ) {
   return;
   }
  if( ! _seen[ y ] ) {
   _seen[ y ] = true;
   reached.push_back( y );
   }
  else if( d >= _dist[ y ] ) {
   return;
   }
  _dist[ y ] = d;
  _pred[ y ] = e;
  heap.push( Item( d , _graph.id( y ) ) );
  }

 // a shortest path of the residual graph from the unmatched red node s to
 // an unmatched blue node, which the matching is augmented along, and the
 // potentials updated so that the reduced costs stay nonnegative; false if
 // no unmatched blue node is reached
 bool augment( const RedNode & s ) {
  std::vector< Node > reached;
  Heap heap;
  relax( s , 0 , INVALID , reached , heap );
  Node t = INVALID;
  while( ! heap.empty() ) {
   Item top = heap.top();
   heap.pop();
   Node x = _graph.nodeFromId( top.second );
   if( _done[ x ] || top.first > _dist[ x ] ) {
    continue;
    }
   _done[ x ] = true;
   if( _graph.blue( x ) ) {
    if( _matching[ x ] == INVALID ) {
     t = x;
     break;
     }
    // from a matched blue node, only back along its matching edge, whose
    // reduced cost in this direction is that of its opposite
    Edge e = _matching[ x ];
    Node y = _graph.redNode( e );
    relax( y , _dist[ x ] - _cost[ e ] + _pot[ x ] - _pot[ y ] , e , reached ,
	   heap );
    continue;
    }
   for( IncEdgeIt e( _graph , x ) ; e != INVALID ; ++e ) {
    if( e != _matching[ x ] ) {
     Node y = _graph.blueNode( e );
     relax( y , _dist[ x ] + _cost[ e ] + _pot[ x ] - _pot[ y ] , e , reached ,
	    heap );
     }
    }
   }

  bool found = t != INVALID;
  if( found ) {
   // the potentials, raised by the distance found, the largest one being
   // that of t, which all the nodes not finished get
   Value dt = _dist[ t ];
   for( NodeIt n( _graph ) ; n != INVALID ; ++n ) {
    _pot[ n ] += ( _done[ n ] && _dist[ n ] < dt ) ? _dist[ n ] : dt;
    }
   // the matching, along the predecessors from t back to s
   Node b = t;
   for( ; ; ) {
    Edge e = _pred[ b ];
    Node r = _graph.redNode( e );
    Edge old = _matching[ r ];
    _matching[ r ] = e;
    _matching[ b ] = e;
    _total += _cost[ e ];
    if( old == INVALID ) {
     break;
     }
    _total -= _cost[ old ];
    b = _graph.blueNode( old );
    }
   }
  for( std::size_t i = 0 ; i < reached.size() ; ++i ) {
   _seen[ reached[ i ] ] = false;
   _done[ reached[ i ] ] = false;
   }
  return( found );
  }

 public:
 /// \brief Constructor
 ///
 /// Constructor.
 ///
 /// \param graph The bipartite graph the algorithm runs on.
 /// \param cost The costs of the edges.
 MinCostAssignment( const BpGraph & graph , const CostMap & cost )
     : _graph( graph ) , _cost( cost ) , _matching( graph , INVALID ) ,
       _pot( graph , 0 ) , _dist( graph , 0 ) , _seen( graph , false ) ,
       _done( graph , false ) , _pred( graph , INVALID ) , _total( 0 ) { }

 /// \brief Run the algorithm.
 ///
 /// This function finds a minimum cost matching that covers every red
 /// node.
 ///
 /// \return \c true if such a matching exists; if it does not, the
 /// matching is left as the methods had grown it, and the potentials have
 /// no meaning.
 bool run() {
  _total = 0;
  for( NodeIt n( _graph ) ; n != INVALID ; ++n ) {
   _matching[ n ] = INVALID;
   _pot[ n ] = 0;
   }
  // the potential of a red node, the opposite of its cheapest edge, so
  // that every reduced cost is nonnegative from the start
  for( RedNodeIt u( _graph ) ; u != INVALID ; ++u ) {
   IncEdgeIt e( _graph , u );
   if( e == INVALID ) {
    return( false );
    }
   Value least = _cost[ e ];
   for( ++e ; e != INVALID ; ++e ) {
    if( _cost[ e ] < least ) {
     least = _cost[ e ];
     }
    }
   _pot[ u ] = -least;
   }
  for( RedNodeIt u( _graph ) ; u != INVALID ; ++u ) {
   if( ! augment( u ) ) {
    return( false );
    }
   }
  return( true );
  }

 /// \name Query Functions
 /// The results of the algorithm, after \ref run() has returned \c true.

 ///@{

 /// \brief Return the cost of the matching.
 Value totalCost() const { return( _total ); }

 /// \brief Return \c true if the given edge is in the matching.
 bool matching( const Edge & edge ) const {
  return( _matching[ _graph.redNode( edge ) ] == edge );
  }

 /// \brief Return the matching edge incident to the given node, or
 /// \c INVALID if the node is not covered by the matching.
 Edge matching( const Node & n ) const { return( _matching[ n ] ); }

 /// \brief Return a const reference to the matching map.
 const MatchingMap & matchingMap() const { return( _matching ); }

 /// \brief Return the potential of the given node, i.e., the dual
 /// solution [see the description of the class].
 Value potential( const Node & n ) const { return( _pot[ n ] ); }

 ///@}
 };

}  //END OF NAMESPACE LEMON

#endif  //LEMON_BIPARTITE_MATCHING_H
