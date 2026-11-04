/* -*- mode: C++; indent-tabs-mode: nil; -*-
 *
 * This file is a part of LEMON, a generic C++ optimization library.
 *
 * Copyright (C) 2003-2010
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

#ifndef LEMON_STATIC_GRAPH_H
 #define LEMON_STATIC_GRAPH_H

///\ingroup graphs
///\file
///\brief StaticDigraph and StaticGraph classes.

#include <vector>

#include <lemon/core.h>
#include <lemon/bits/graph_extender.h>

namespace lemon
{

class StaticDigraphBase
{

 public:
 StaticDigraphBase()
     : built( false ) , node_num( 0 ) , arc_num( 0 ) , node_first_out( NULL ) ,
       node_first_in( NULL ) , arc_source( NULL ) , arc_target( NULL ) ,
       arc_next_in( NULL ) , arc_next_out( NULL ) { }

 ~StaticDigraphBase() {
  if( built ) {
   delete[] node_first_out;
   delete[] node_first_in;
   delete[] arc_source;
   delete[] arc_target;
   delete[] arc_next_out;
   delete[] arc_next_in;
   }
  }

 class Node
 {
  friend class StaticDigraphBase;

  protected:
  int id;
  Node( int _id ) : id( _id ) { }

  public:
  Node() : id( -1 ) { }
  Node( Invalid ) : id( -1 ) { }
  bool operator==( const Node & node ) const { return( id == node.id ); }
  bool operator!=( const Node & node ) const { return( id != node.id ); }
  bool operator<( const Node & node ) const { return( id < node.id ); }
  };

 class Arc
 {
  friend class StaticDigraphBase;

  protected:
  int id;
  Arc( int _id ) : id( _id ) { }

  public:
  Arc() : id( -1 ) { }
  Arc( Invalid ) : id( -1 ) { }
  bool operator==( const Arc & arc ) const { return( id == arc.id ); }
  bool operator!=( const Arc & arc ) const { return( id != arc.id ); }
  bool operator<( const Arc & arc ) const { return( id < arc.id ); }
  };

 Node source( const Arc & e ) const { return( Node( arc_source[ e.id ] ) ); }
 Node target( const Arc & e ) const { return( Node( arc_target[ e.id ] ) ); }

 void first( Node & n ) const { n.id = node_num - 1; }
 static void next( Node & n ) { --n.id; }

 void first( Arc & e ) const { e.id = arc_num - 1; }
 static void next( Arc & e ) { --e.id; }

 void firstOut( Arc & e , const Node & n ) const {
  e.id = node_first_out[ n.id ] != node_first_out[ n.id + 1 ]
	   ? node_first_out[ n.id ]
	   : -1;
  }
 void nextOut( Arc & e ) const { e.id = arc_next_out[ e.id ]; }

 void firstIn( Arc & e , const Node & n ) const {
  e.id = node_first_in[ n.id ];
  }
 void nextIn( Arc & e ) const { e.id = arc_next_in[ e.id ]; }

 static int id( const Node & n ) { return( n.id ); }
 static Node nodeFromId( int id ) { return( Node( id ) ); }
 int maxNodeId() const { return( node_num - 1 ); }

 static int id( const Arc & e ) { return( e.id ); }
 static Arc arcFromId( int id ) { return( Arc( id ) ); }
 int maxArcId() const { return( arc_num - 1 ); }

 typedef True NodeNumTag;
 typedef True ArcNumTag;

 int nodeNum() const { return( node_num ); }
 int arcNum() const { return( arc_num ); }

 private:
 template< typename Digraph , typename NodeRefMap >
 class ArcLess
 {
  public:
  typedef typename Digraph::Arc Arc;

  ArcLess( const Digraph & _graph , const NodeRefMap & _nodeRef )
      : digraph( _graph ) , nodeRef( _nodeRef ) { }

  bool operator()( const Arc & left , const Arc & right ) const {
   return( nodeRef[ digraph.target( left ) ] <
	    nodeRef[ digraph.target( right ) ] );
   }

  private:
  const Digraph & digraph;
  const NodeRefMap & nodeRef;
  };

 public:
 typedef True BuildTag;

 void clear() {
  if( built ) {
   delete[] node_first_out;
   delete[] node_first_in;
   delete[] arc_source;
   delete[] arc_target;
   delete[] arc_next_out;
   delete[] arc_next_in;
   }
  built = false;
  node_num = 0;
  arc_num = 0;
  }

 template< typename Digraph , typename NodeRefMap , typename ArcRefMap >
 void build( const Digraph & digraph , NodeRefMap & nodeRef ,
	     ArcRefMap & arcRef ) {
  typedef typename Digraph::Arc GArc;

  built = true;

  node_num = countNodes( digraph );
  arc_num = countArcs( digraph );

  node_first_out = new int[ node_num + 1 ];
  node_first_in = new int[ node_num ];

  arc_source = new int[ arc_num ];
  arc_target = new int[ arc_num ];
  arc_next_out = new int[ arc_num ];
  arc_next_in = new int[ arc_num ];

  int node_index = 0;
  for( typename Digraph::NodeIt n( digraph ) ; n != INVALID ; ++n ) {
   nodeRef[ n ] = Node( node_index );
   node_first_in[ node_index ] = -1;
   ++node_index;
   }

  ArcLess< Digraph , NodeRefMap > arcLess( digraph , nodeRef );

  int arc_index = 0;
  for( typename Digraph::NodeIt n( digraph ) ; n != INVALID ; ++n ) {
   int source = nodeRef[ n ].id;
   std::vector< GArc > arcs;
   for( typename Digraph::OutArcIt e( digraph , n ) ; e != INVALID ; ++e ) {
    arcs.push_back( e );
    }
   if( ! arcs.empty() ) {
    node_first_out[ source ] = arc_index;
    std::sort( arcs.begin() , arcs.end() , arcLess );
    for( typename std::vector< GArc >::iterator it = arcs.begin() ;
	 it != arcs.end() ; ++it ) {
     int target = nodeRef[ digraph.target( *it ) ].id;
     arcRef[ *it ] = Arc( arc_index );
     arc_source[ arc_index ] = source;
     arc_target[ arc_index ] = target;
     arc_next_in[ arc_index ] = node_first_in[ target ];
     node_first_in[ target ] = arc_index;
     arc_next_out[ arc_index ] = arc_index + 1;
     ++arc_index;
     }
    arc_next_out[ arc_index - 1 ] = -1;
    }
   else {
    node_first_out[ source ] = arc_index;
    }
   }
  node_first_out[ node_num ] = arc_num;
  }

 template< typename ArcListIterator >
 void build( int n , ArcListIterator first , ArcListIterator last ) {
  built = true;

  node_num = n;
  arc_num = static_cast< int >( std::distance( first , last ) );

  node_first_out = new int[ node_num + 1 ];
  node_first_in = new int[ node_num ];

  arc_source = new int[ arc_num ];
  arc_target = new int[ arc_num ];
  arc_next_out = new int[ arc_num ];
  arc_next_in = new int[ arc_num ];

  for( int i = 0 ; i != node_num ; ++i ) {
   node_first_in[ i ] = -1;
   }

  int arc_index = 0;
  for( int i = 0 ; i != node_num ; ++i ) {
   node_first_out[ i ] = arc_index;
   for( ; first != last && ( *first ).first == i ; ++first ) {
    int j = ( *first ).second;
    LEMON_ASSERT( j >= 0 && j < node_num ,
		  "Wrong arc list for StaticDigraph::build()" );
    arc_source[ arc_index ] = i;
    arc_target[ arc_index ] = j;
    arc_next_in[ arc_index ] = node_first_in[ j ];
    node_first_in[ j ] = arc_index;
    arc_next_out[ arc_index ] = arc_index + 1;
    ++arc_index;
    }
   if( arc_index > node_first_out[ i ] )
    arc_next_out[ arc_index - 1 ] = -1;
   }
  LEMON_ASSERT( first == last , "Wrong arc list for StaticDigraph::build()" );
  node_first_out[ node_num ] = arc_num;
  }

 protected:
 void fastFirstOut( Arc & e , const Node & n ) const {
  e.id = node_first_out[ n.id ];
  }

 static void fastNextOut( Arc & e ) { ++e.id; }
 void fastLastOut( Arc & e , const Node & n ) const {
  e.id = node_first_out[ n.id + 1 ];
  }

 protected:
 bool built;
 int node_num;
 int arc_num;
 int * node_first_out;
 int * node_first_in;
 int * arc_source;
 int * arc_target;
 int * arc_next_in;
 int * arc_next_out;
 };

typedef DigraphExtender< StaticDigraphBase > ExtendedStaticDigraphBase;


/// \ingroup graphs
///
/// \brief A static directed graph class.
///
/// \ref StaticDigraph is a highly efficient digraph implementation,
/// but it is fully static.
/// It stores only two \c int values for each node and only four \c int
/// values for each arc. Moreover it provides faster item iteration than
/// \ref ListDigraph and \ref SmartDigraph, especially using \c OutArcIt
/// iterators, since its arcs are stored in an appropriate order.
/// However it only provides build() and clear() functions and does not
/// support any other modification of the digraph.
///
/// Since this digraph structure is completely static, its nodes and arcs
/// can be indexed with integers from the ranges <tt>[0..nodeNum()-1]</tt>
/// and <tt>[0..arcNum()-1]</tt>, respectively.
/// The index of an item is the same as its ID, it can be obtained
/// using the corresponding \ref index() or \ref concepts::Digraph::id()
/// "id()" function. A node or arc with a certain index can be obtained
/// using node() or arc().
///
/// This type fully conforms to the \ref concepts::Digraph "Digraph concept".
/// Most of its member functions and nested classes are documented
/// only in the concept class.
///
/// This class provides constant time counting for nodes and arcs.
///
/// \sa concepts::Digraph
class StaticDigraph : public ExtendedStaticDigraphBase
{

 private:
 /// Graphs are \e not copy constructible. Use DigraphCopy instead.
 StaticDigraph( const StaticDigraph & ) : ExtendedStaticDigraphBase() { };
 /// \brief Assignment of a graph to another one is \e not allowed.
 /// Use DigraphCopy instead.
 void operator=( const StaticDigraph & ) { }

 public:
 typedef ExtendedStaticDigraphBase Parent;

 public:
 /// \brief Constructor
 ///
 /// Default constructor.
 StaticDigraph() : Parent() { }

 /// \brief The node with the given index.
 ///
 /// This function returns the node with the given index.
 /// \sa index()
 static Node node( int ix ) { return( Parent::nodeFromId( ix ) ); }

 /// \brief The arc with the given index.
 ///
 /// This function returns the arc with the given index.
 /// \sa index()
 static Arc arc( int ix ) { return( Parent::arcFromId( ix ) ); }

 /// \brief The index of the given node.
 ///
 /// This function returns the index of the the given node.
 /// \sa node()
 static int index( Node node ) { return( Parent::id( node ) ); }

 /// \brief The index of the given arc.
 ///
 /// This function returns the index of the the given arc.
 /// \sa arc()
 static int index( Arc arc ) { return( Parent::id( arc ) ); }

 /// \brief Number of nodes.
 ///
 /// This function returns the number of nodes.
 int nodeNum() const { return( node_num ); }

 /// \brief Number of arcs.
 ///
 /// This function returns the number of arcs.
 int arcNum() const { return( arc_num ); }

 /// \brief Build the digraph copying another digraph.
 ///
 /// This function builds the digraph copying another digraph of any
 /// kind. It can be called more than once, but in such case, the whole
 /// structure and all maps will be cleared and rebuilt.
 ///
 /// This method also makes possible to copy a digraph to a StaticDigraph
 /// structure using \ref DigraphCopy.
 ///
 /// \param digraph An existing digraph to be copied.
 /// \param nodeRef The node references will be copied into this map.
 /// Its key type must be \c Digraph::Node and its value type must be
 /// \c StaticDigraph::Node.
 /// It must conform to the \ref concepts::ReadWriteMap "ReadWriteMap"
 /// concept.
 /// \param arcRef The arc references will be copied into this map.
 /// Its key type must be \c Digraph::Arc and its value type must be
 /// \c StaticDigraph::Arc.
 /// It must conform to the \ref concepts::WriteMap "WriteMap" concept.
 ///
 /// \note If you do not need the arc references, then you could use
 /// \ref NullMap for the last parameter. However the node references
 /// are required by the function itself, thus they must be readable
 /// from the map.
 template< typename Digraph , typename NodeRefMap , typename ArcRefMap >
 void build( const Digraph & digraph , NodeRefMap & nodeRef ,
	     ArcRefMap & arcRef ) {
  if( built )
   Parent::clear();
  Parent::build( digraph , nodeRef , arcRef );
  }

 /// \brief Build the digraph from an arc list.
 ///
 /// This function builds the digraph from the given arc list.
 /// It can be called more than once, but in such case, the whole
 /// structure and all maps will be cleared and rebuilt.
 ///
 /// The list of the arcs must be given in the range <tt>[begin, end)</tt>
 /// specified by STL compatible itartors whose \c value_type must be
 /// <tt>std::pair< int , int ></tt>.
 /// Each arc must be specified by a pair of integer indices
 /// from the range <tt>[0..n-1]</tt>. <i>The pairs must be in a
 /// non-decreasing order with respect to their first values.</i>
 /// If the k-th pair in the list is <tt>(i,j)</tt>, then
 /// <tt>arc(k-1)</tt> will connect <tt>node(i)</tt> to <tt>node(j)</tt>.
 ///
 /// \param n The number of nodes.
 /// \param begin An iterator pointing to the beginning of the arc list.
 /// \param end An iterator pointing to the end of the arc list.
 ///
 /// For example, a simple digraph can be constructed like this.
 /// \code
 /// std::vector< std::pair< int , int > > arcs;
 /// arcs.push_back( std::make_pair( 0 , 1 ) );
 /// arcs.push_back( std::make_pair( 0 , 2 ) );
 /// arcs.push_back( std::make_pair( 1 , 3 ) );
 /// arcs.push_back( std::make_pair( 1 , 2 ) );
 /// arcs.push_back( std::make_pair( 3 , 0 ) );
 /// StaticDigraph gr;
 /// gr.build( 4 , arcs.begin() , arcs.end() );
 /// \endcode
 template< typename ArcListIterator >
 void build( int n , ArcListIterator begin , ArcListIterator end ) {
  if( built )
   Parent::clear();
  StaticDigraphBase::build( n , begin , end );
  notifier( Node() ).build();
  notifier( Arc() ).build();
  }

 /// \brief Clear the digraph.
 ///
 /// This function erases all nodes and arcs from the digraph.
 void clear() { Parent::clear(); }

 protected:
 using Parent::fastFirstOut;
 using Parent::fastNextOut;
 using Parent::fastLastOut;

 public:
 class OutArcIt : public Arc
 {
  public:
  OutArcIt() { }

  OutArcIt( Invalid i ) : Arc( i ) { }

  OutArcIt( const StaticDigraph & digraph , const Node & node ) {
   digraph.fastFirstOut( *this , node );
   digraph.fastLastOut( last , node );
   if( last == *this )
    *this = INVALID;
   }

  OutArcIt( const StaticDigraph & digraph , const Arc & arc ) : Arc( arc ) {
   if( arc != INVALID ) {
    digraph.fastLastOut( last , digraph.source( arc ) );
    }
   }

  OutArcIt & operator++() {
   StaticDigraph::fastNextOut( *this );
   if( last == *this )
    *this = INVALID;
   return( *this );
   }

  private:
  Arc last;
  };

 Node baseNode( const OutArcIt & arc ) const {
  return( Parent::source( static_cast< const Arc & >( arc ) ) );
  }

 Node runningNode( const OutArcIt & arc ) const {
  return( Parent::target( static_cast< const Arc & >( arc ) ) );
  }

 Node baseNode( const InArcIt & arc ) const {
  return( Parent::target( static_cast< const Arc & >( arc ) ) );
  }

 Node runningNode( const InArcIt & arc ) const {
  return( Parent::source( static_cast< const Arc & >( arc ) ) );
  }
 };


// The undirected static graph: the edge e is made of the arcs 2e, from v(e)
// to u(e), and 2e + 1, from u(e) to v(e), as in SmartGraph, and the arcs
// out of each node are stored one after the other, so that the arcs into a
// node are the opposites of those out of it
class StaticGraphBase
{
 public:
 StaticGraphBase() : _node_num( 0 ) { }

 typedef StaticGraphBase Graph;

 class Node;
 class Arc;
 class Edge;

 class Node
 {
  friend class StaticGraphBase;

  protected:
  int _id;
  explicit Node( int id ) : _id( id ) { }

  public:
  Node() : _id( -1 ) { }
  Node( Invalid ) : _id( -1 ) { }
  bool operator==( const Node & node ) const { return( _id == node._id ); }
  bool operator!=( const Node & node ) const { return( _id != node._id ); }
  bool operator<( const Node & node ) const { return( _id < node._id ); }
  };

 class Edge
 {
  friend class StaticGraphBase;

  protected:
  int _id;
  explicit Edge( int id ) : _id( id ) { }

  public:
  Edge() : _id( -1 ) { }
  Edge( Invalid ) : _id( -1 ) { }
  bool operator==( const Edge & edge ) const { return( _id == edge._id ); }
  bool operator!=( const Edge & edge ) const { return( _id != edge._id ); }
  bool operator<( const Edge & edge ) const { return( _id < edge._id ); }
  };

 class Arc
 {
  friend class StaticGraphBase;

  protected:
  int _id;
  explicit Arc( int id ) : _id( id ) { }

  public:
  operator Edge() const {
   return( _id != -1 ? Edge( _id / 2 ) : Edge( INVALID ) );
   }

  Arc() : _id( -1 ) { }
  Arc( Invalid ) : _id( -1 ) { }
  bool operator==( const Arc & arc ) const { return( _id == arc._id ); }
  bool operator!=( const Arc & arc ) const { return( _id != arc._id ); }
  bool operator<( const Arc & arc ) const { return( _id < arc._id ); }
  };

 typedef True NodeNumTag;
 typedef True EdgeNumTag;
 typedef True ArcNumTag;
 typedef True BuildTag;

 int nodeNum() const { return( _node_num ); }
 int edgeNum() const { return( int( _target.size() / 2 ) ); }
 int arcNum() const { return( int( _target.size() ) ); }

 int maxNodeId() const { return( _node_num - 1 ); }
 int maxEdgeId() const { return( int( _target.size() / 2 ) - 1 ); }
 int maxArcId() const { return( int( _target.size() ) - 1 ); }

 Node source( Arc a ) const { return( Node( _target[ a._id ^ 1 ] ) ); }
 Node target( Arc a ) const { return( Node( _target[ a._id ] ) ); }

 Node u( Edge e ) const { return( Node( _target[ 2 * e._id ] ) ); }
 Node v( Edge e ) const { return( Node( _target[ 2 * e._id + 1 ] ) ); }

 static bool direction( Arc a ) { return( ( a._id & 1 ) == 1 ); }
 static Arc direct( Edge e , bool d ) {
  return( Arc( e._id * 2 + ( d ? 1 : 0 ) ) );
  }

 void first( Node & node ) const { node._id = _node_num - 1; }
 static void next( Node & node ) { --node._id; }
 void first( Arc & arc ) const { arc._id = int( _target.size() ) - 1; }
 static void next( Arc & arc ) { --arc._id; }
 void first( Edge & edge ) const { edge._id = int( _target.size() / 2 ) - 1; }
 static void next( Edge & edge ) { --edge._id; }

 void firstOut( Arc & arc , const Node & v ) const {
  int p = _first[ v._id ];
  arc._id = p < _first[ v._id + 1 ] ? _out[ p ] : -1;
  }
 void nextOut( Arc & arc ) const {
  int p = _pos[ arc._id ] + 1;
  arc._id = p < _first[ _target[ arc._id ^ 1 ] + 1 ] ? _out[ p ] : -1;
  }

 void firstIn( Arc & arc , const Node & v ) const {
  firstOut( arc , v );
  if( arc._id != -1 ) {
   arc._id ^= 1;
   }
  }
 void nextIn( Arc & arc ) const {
  arc._id ^= 1;
  nextOut( arc );
  if( arc._id != -1 ) {
   arc._id ^= 1;
   }
  }

 void firstInc( Edge & edge , bool & d , const Node & v ) const {
  Arc arc;
  firstOut( arc , v );
  edge._id = arc._id == -1 ? -1 : arc._id / 2;
  d = arc._id == -1 || direction( arc );
  }
 void nextInc( Edge & edge , bool & d ) const {
  Arc arc = direct( edge , d );
  nextOut( arc );
  edge._id = arc._id == -1 ? -1 : arc._id / 2;
  d = arc._id == -1 || direction( arc );
  }

 static int id( Node v ) { return( v._id ); }
 static int id( Arc a ) { return( a._id ); }
 static int id( Edge e ) { return( e._id ); }

 static Node nodeFromId( int id ) { return( Node( id ) ); }
 static Arc arcFromId( int id ) { return( Arc( id ) ); }
 static Edge edgeFromId( int id ) { return( Edge( id ) ); }

 bool valid( Node n ) const { return( n._id >= 0 && n._id < _node_num ); }
 bool valid( Arc a ) const {
  return( a._id >= 0 && a._id < int( _target.size() ) );
  }
 bool valid( Edge e ) const {
  return( e._id >= 0 && 2 * e._id < int( _target.size() ) );
  }

 void clear() {
  _node_num = 0;
  _target.clear();
  _pos.clear();
  _out.clear();
  _first.assign( 1 , 0 );
  }

 // the graph of n nodes whose edge i joins the nodes ends[ i ]
 void build( int n , const std::vector< std::pair< int , int > > & ends ) {
  _node_num = n;
  int m = int( ends.size() );
  _target.resize( 2 * m );
  _pos.resize( 2 * m );
  _out.resize( 2 * m );
  _first.assign( n + 1 , 0 );
  for( int i = 0 ; i < m ; ++i ) {
   LEMON_ASSERT( ends[ i ].first >= 0 && ends[ i ].first < n &&
		   ends[ i ].second >= 0 && ends[ i ].second < n ,
		 "Wrong edge list for StaticGraph::build()" );
   _target[ 2 * i ] = ends[ i ].first;
   _target[ 2 * i + 1 ] = ends[ i ].second;
   ++_first[ ends[ i ].first + 1 ];   // arc 2i + 1 leaves u
   ++_first[ ends[ i ].second + 1 ];  // arc 2i leaves v
   }
  for( int v = 0 ; v < n ; ++v ) {
   _first[ v + 1 ] += _first[ v ];
   }
  std::vector< int > fill( _first.begin() , _first.end() - 1 );
  for( int a = 0 ; a < 2 * m ; ++a ) {
   int p = fill[ _target[ a ^ 1 ] ]++;
   _out[ p ] = a;
   _pos[ a ] = p;
   }
  }

 template< typename Graph , typename NodeRefMap , typename EdgeRefMap >
 void build( const Graph & graph , NodeRefMap & nodeRef ,
	     EdgeRefMap & edgeRef ) {
  int n = 0;
  for( typename Graph::NodeIt v( graph ) ; v != INVALID ; ++v ) {
   nodeRef[ v ] = Node( n++ );
   }
  std::vector< std::pair< int , int > > ends;
  for( typename Graph::EdgeIt e( graph ) ; e != INVALID ; ++e ) {
   edgeRef[ e ] = Edge( int( ends.size() ) );
   ends.push_back( std::make_pair( nodeRef[ graph.u( e ) ]._id ,
				   nodeRef[ graph.v( e ) ]._id ) );
   }
  build( n , ends );
  }

 private:
 int _node_num;
 std::vector< int > _target;  // the target of each arc
 std::vector< int > _pos;     // the position of each arc in _out
 std::vector< int > _out;     // the arcs out of node v, from _first[ v ]
 std::vector< int > _first;
 };

typedef GraphExtender< StaticGraphBase > ExtendedStaticGraphBase;

/// \ingroup graphs
///
/// \brief A static undirected graph class.
///
/// \ref StaticGraph is the undirected counterpart of \ref StaticDigraph:
/// a graph whose structure is built once, with \ref build(), and cannot be
/// modified after that (but it can be built again), in exchange for
/// arrays that hold the arcs out of each node one after the other, with no
/// pointer to follow. It takes less memory than \ref SmartGraph and \ref
/// ListGraph, and the incidences of a node are scanned contiguously.
///
/// The nodes and the edges are numbered from 0, in the order in which
/// \ref build() is given them, and \ref node() and \ref edge() find them
/// from their index; the edges and the arcs are related as in \ref
/// SmartGraph.
///
/// This type fully conforms to the \ref concepts::Graph "Graph concept".
/// Most of its member functions and nested classes are documented only in
/// the concept class.
///
/// This class provides constant time counting for nodes, edges and arcs.
///
/// \sa concepts::Graph
/// \sa StaticDigraph
class StaticGraph : public ExtendedStaticGraphBase
{
 private:
 /// Graphs are \e not copy constructible. Use GraphCopy instead.
 StaticGraph( const StaticGraph & ) : ExtendedStaticGraphBase() { }
 /// \brief Assignment of a graph to another one is \e not allowed.
 /// Use GraphCopy instead.
 void operator=( const StaticGraph & ) { }

 public:
 typedef ExtendedStaticGraphBase Parent;

 /// \brief Constructor
 ///
 /// Default constructor, of the empty graph.
 StaticGraph() : Parent() { Parent::clear(); }

 /// \brief The node with the given index.
 static Node node( int ix ) { return( Parent::nodeFromId( ix ) ); }

 /// \brief The edge with the given index.
 static Edge edge( int ix ) { return( Parent::edgeFromId( ix ) ); }

 /// \brief The index of the given node.
 static int index( Node node ) { return( Parent::id( node ) ); }

 /// \brief The index of the given edge.
 static int index( Edge edge ) { return( Parent::id( edge ) ); }

 /// \brief Build the graph copying another graph.
 ///
 /// This function builds the graph copying another undirected graph of
 /// any kind, with its nodes and edges in the order of their iterators.
 /// It can be called more than once, in which case the whole structure
 /// and all the maps are cleared and rebuilt. It also makes it possible to
 /// copy a graph to a StaticGraph with \ref GraphCopy.
 ///
 /// \param graph An existing graph to be copied.
 /// \param nodeRef The node references will be copied into this map,
 /// which must be readable and writable, with key type \c Graph::Node and
 /// value type \c StaticGraph::Node.
 /// \param edgeRef The edge references will be copied into this map,
 /// which must be writable, with key type \c Graph::Edge and value type
 /// \c StaticGraph::Edge; \ref NullMap if they are not needed.
 template< typename Graph , typename NodeRefMap , typename EdgeRefMap >
 void build( const Graph & graph , NodeRefMap & nodeRef ,
	     EdgeRefMap & edgeRef ) {
  Parent::clear();
  Parent::build( graph , nodeRef , edgeRef );
  }

 /// \brief Build the graph from a list of edges.
 ///
 /// This function builds the graph with \c n nodes and the edges in the
 /// range <tt>[begin, end)</tt>, given by STL compatible iterators whose
 /// \c value_type is <tt>std::pair< int , int ></tt>: if the k-th pair is
 /// <tt>(i,j)</tt>, then <tt>edge(k-1)</tt> joins <tt>node(i)</tt>, its
 /// \c u(), and <tt>node(j)</tt>, its \c v(). Unlike in
 /// StaticDigraph::build(), the pairs may come in any order. It can be
 /// called more than once, in which case the whole structure and all the
 /// maps are cleared and rebuilt.
 template< typename EdgeListIterator >
 void build( int n , EdgeListIterator begin , EdgeListIterator end ) {
  Parent::clear();
  std::vector< std::pair< int , int > > ends;
  for( ; begin != end ; ++begin ) {
   ends.push_back(
     std::make_pair( int( ( *begin ).first ) , int( ( *begin ).second ) ) );
   }
  StaticGraphBase::build( n , ends );
  notifier( Node() ).build();
  notifier( Edge() ).build();
  notifier( Arc() ).build();
  }

 /// \brief Clear the graph.
 ///
 /// This function erases all the nodes and the edges of the graph.
 void clear() { Parent::clear(); }
 };

}

#endif
