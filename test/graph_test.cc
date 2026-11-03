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

#include <algorithm>
#include <vector>

#include <lemon/concepts/graph.h>
#include <lemon/list_graph.h>
#include <lemon/smart_graph.h>
#include <lemon/static_graph.h>
#include <lemon/random.h>
#include <lemon/full_graph.h>
#include <lemon/grid_graph.h>
#include <lemon/hypercube_graph.h>

#include "test_tools.h"
#include "graph_test.h"

using namespace lemon;
using namespace lemon::concepts;

template< class Graph >
void checkGraphBuild()
{
 TEMPLATE_GRAPH_TYPEDEFS( Graph );

 Graph G;
 checkGraphNodeList( G , 0 );
 checkGraphEdgeList( G , 0 );
 checkGraphArcList( G , 0 );

 G.reserveNode( 3 );
 G.reserveEdge( 3 );

 Node n1 = G.addNode() , n2 = G.addNode() , n3 = G.addNode();
 checkGraphNodeList( G , 3 );
 checkGraphEdgeList( G , 0 );
 checkGraphArcList( G , 0 );

 Edge e1 = G.addEdge( n1 , n2 );
 check( ( G.u( e1 ) == n1 && G.v( e1 ) == n2 ) ||
	  ( G.u( e1 ) == n2 && G.v( e1 ) == n1 ) ,
	"Wrong edge" );

 checkGraphNodeList( G , 3 );
 checkGraphEdgeList( G , 1 );
 checkGraphArcList( G , 2 );

 checkGraphIncEdgeArcLists( G , n1 , 1 );
 checkGraphIncEdgeArcLists( G , n2 , 1 );
 checkGraphIncEdgeArcLists( G , n3 , 0 );

 checkGraphConEdgeList( G , 1 );
 checkGraphConArcList( G , 2 );

 Edge e2 = G.addEdge( n2 , n1 ) , e3 = G.addEdge( n2 , n3 );
 ::lemon::ignore_unused_variable_warning( e2 , e3 );

 checkGraphNodeList( G , 3 );
 checkGraphEdgeList( G , 3 );
 checkGraphArcList( G , 6 );

 checkGraphIncEdgeArcLists( G , n1 , 2 );
 checkGraphIncEdgeArcLists( G , n2 , 3 );
 checkGraphIncEdgeArcLists( G , n3 , 1 );

 checkGraphConEdgeList( G , 3 );
 checkGraphConArcList( G , 6 );

 checkArcDirections( G );

 checkNodeIds( G );
 checkArcIds( G );
 checkEdgeIds( G );
 checkGraphNodeMap( G );
 checkGraphArcMap( G );
 checkGraphEdgeMap( G );
 }

template< class Graph >
void checkGraphAlter()
{
 TEMPLATE_GRAPH_TYPEDEFS( Graph );

 Graph G;
 Node n1 = G.addNode() , n2 = G.addNode() , n3 = G.addNode() ,
   n4 = G.addNode();
 Edge e1 = G.addEdge( n1 , n2 ) , e2 = G.addEdge( n2 , n1 ) ,
      e3 = G.addEdge( n2 , n3 ) , e4 = G.addEdge( n1 , n4 ) ,
      e5 = G.addEdge( n4 , n3 );
 ::lemon::ignore_unused_variable_warning( e1 , e3 , e4 , e5 );

 checkGraphNodeList( G , 4 );
 checkGraphEdgeList( G , 5 );
 checkGraphArcList( G , 10 );

 // Check changeU() and changeV()
 if( G.u( e2 ) == n2 ) {
  G.changeU( e2 , n3 );
  }
 else {
  G.changeV( e2 , n3 );
  }

 checkGraphNodeList( G , 4 );
 checkGraphEdgeList( G , 5 );
 checkGraphArcList( G , 10 );

 checkGraphIncEdgeArcLists( G , n1 , 3 );
 checkGraphIncEdgeArcLists( G , n2 , 2 );
 checkGraphIncEdgeArcLists( G , n3 , 3 );
 checkGraphIncEdgeArcLists( G , n4 , 2 );

 checkGraphConEdgeList( G , 5 );
 checkGraphConArcList( G , 10 );

 if( G.u( e2 ) == n1 ) {
  G.changeU( e2 , n2 );
  }
 else {
  G.changeV( e2 , n2 );
  }

 checkGraphNodeList( G , 4 );
 checkGraphEdgeList( G , 5 );
 checkGraphArcList( G , 10 );

 checkGraphIncEdgeArcLists( G , n1 , 2 );
 checkGraphIncEdgeArcLists( G , n2 , 3 );
 checkGraphIncEdgeArcLists( G , n3 , 3 );
 checkGraphIncEdgeArcLists( G , n4 , 2 );

 checkGraphConEdgeList( G , 5 );
 checkGraphConArcList( G , 10 );

 // Check contract()
 G.contract( n1 , n4 , false );

 checkGraphNodeList( G , 3 );
 checkGraphEdgeList( G , 5 );
 checkGraphArcList( G , 10 );

 checkGraphIncEdgeArcLists( G , n1 , 4 );
 checkGraphIncEdgeArcLists( G , n2 , 3 );
 checkGraphIncEdgeArcLists( G , n3 , 3 );

 checkGraphConEdgeList( G , 5 );
 checkGraphConArcList( G , 10 );

 G.contract( n2 , n3 );

 checkGraphNodeList( G , 2 );
 checkGraphEdgeList( G , 3 );
 checkGraphArcList( G , 6 );

 checkGraphIncEdgeArcLists( G , n1 , 4 );
 checkGraphIncEdgeArcLists( G , n2 , 2 );

 checkGraphConEdgeList( G , 3 );
 checkGraphConArcList( G , 6 );
 }

template< class Graph >
void checkGraphErase()
{
 TEMPLATE_GRAPH_TYPEDEFS( Graph );

 Graph G;
 Node n1 = G.addNode() , n2 = G.addNode() , n3 = G.addNode() ,
   n4 = G.addNode();
 Edge e1 = G.addEdge( n1 , n2 ) , e2 = G.addEdge( n2 , n1 ) ,
      e3 = G.addEdge( n2 , n3 ) , e4 = G.addEdge( n1 , n4 ) ,
      e5 = G.addEdge( n4 , n3 );
 ::lemon::ignore_unused_variable_warning( e1 , e3 , e4 , e5 );

 // Check edge deletion
 G.erase( e2 );

 checkGraphNodeList( G , 4 );
 checkGraphEdgeList( G , 4 );
 checkGraphArcList( G , 8 );

 checkGraphIncEdgeArcLists( G , n1 , 2 );
 checkGraphIncEdgeArcLists( G , n2 , 2 );
 checkGraphIncEdgeArcLists( G , n3 , 2 );
 checkGraphIncEdgeArcLists( G , n4 , 2 );

 checkGraphConEdgeList( G , 4 );
 checkGraphConArcList( G , 8 );

 // Check node deletion
 G.erase( n3 );

 checkGraphNodeList( G , 3 );
 checkGraphEdgeList( G , 2 );
 checkGraphArcList( G , 4 );

 checkGraphIncEdgeArcLists( G , n1 , 2 );
 checkGraphIncEdgeArcLists( G , n2 , 1 );
 checkGraphIncEdgeArcLists( G , n4 , 1 );

 checkGraphConEdgeList( G , 2 );
 checkGraphConArcList( G , 4 );
 }


template< class Graph >
void checkGraphSnapshot()
{
 TEMPLATE_GRAPH_TYPEDEFS( Graph );

 Graph G;
 Node n1 = G.addNode() , n2 = G.addNode() , n3 = G.addNode();
 Edge e1 = G.addEdge( n1 , n2 ) , e2 = G.addEdge( n2 , n1 ) ,
      e3 = G.addEdge( n2 , n3 );
 ::lemon::ignore_unused_variable_warning( e1 , e2 , e3 );

 checkGraphNodeList( G , 3 );
 checkGraphEdgeList( G , 3 );
 checkGraphArcList( G , 6 );

 typename Graph::Snapshot snapshot( G );

 Node n = G.addNode();
 G.addEdge( n3 , n );
 G.addEdge( n , n3 );
 G.addEdge( n3 , n2 );

 checkGraphNodeList( G , 4 );
 checkGraphEdgeList( G , 6 );
 checkGraphArcList( G , 12 );

 snapshot.restore();

 checkGraphNodeList( G , 3 );
 checkGraphEdgeList( G , 3 );
 checkGraphArcList( G , 6 );

 checkGraphIncEdgeArcLists( G , n1 , 2 );
 checkGraphIncEdgeArcLists( G , n2 , 3 );
 checkGraphIncEdgeArcLists( G , n3 , 1 );

 checkGraphConEdgeList( G , 3 );
 checkGraphConArcList( G , 6 );

 checkNodeIds( G );
 checkEdgeIds( G );
 checkArcIds( G );
 checkGraphNodeMap( G );
 checkGraphEdgeMap( G );
 checkGraphArcMap( G );

 G.addNode();
 snapshot.save( G );

 G.addEdge( G.addNode() , G.addNode() );

 snapshot.restore();
 snapshot.save( G );

 checkGraphNodeList( G , 4 );
 checkGraphEdgeList( G , 3 );
 checkGraphArcList( G , 6 );

 G.addEdge( G.addNode() , G.addNode() );

 snapshot.restore();

 checkGraphNodeList( G , 4 );
 checkGraphEdgeList( G , 3 );
 checkGraphArcList( G , 6 );
 }

void checkFullGraph( int num )
{
 typedef FullGraph Graph;
 GRAPH_TYPEDEFS( Graph );

 Graph G( num );
 check( G.nodeNum() == num && G.edgeNum() == num * ( num - 1 ) / 2 ,
	"Wrong size" );

 G.resize( num );
 check( G.nodeNum() == num && G.edgeNum() == num * ( num - 1 ) / 2 ,
	"Wrong size" );

 checkGraphNodeList( G , num );
 checkGraphEdgeList( G , num * ( num - 1 ) / 2 );

 for( NodeIt n( G ) ; n != INVALID ; ++n ) {
  checkGraphOutArcList( G , n , num - 1 );
  checkGraphInArcList( G , n , num - 1 );
  checkGraphIncEdgeList( G , n , num - 1 );
  }

 checkGraphConArcList( G , num * ( num - 1 ) );
 checkGraphConEdgeList( G , num * ( num - 1 ) / 2 );

 checkArcDirections( G );

 checkNodeIds( G );
 checkArcIds( G );
 checkEdgeIds( G );
 checkGraphNodeMap( G );
 checkGraphArcMap( G );
 checkGraphEdgeMap( G );


 for( int i = 0 ; i < G.nodeNum() ; ++i ) {
  check( G.index( G( i ) ) == i , "Wrong index" );
  }

 for( NodeIt u( G ) ; u != INVALID ; ++u ) {
  for( NodeIt v( G ) ; v != INVALID ; ++v ) {
   Edge e = G.edge( u , v );
   Arc a = G.arc( u , v );
   if( u == v ) {
    check( e == INVALID , "Wrong edge lookup" );
    check( a == INVALID , "Wrong arc lookup" );
    }
   else {
    check( ( G.u( e ) == u && G.v( e ) == v ) ||
	     ( G.u( e ) == v && G.v( e ) == u ) ,
	   "Wrong edge lookup" );
    check( G.source( a ) == u && G.target( a ) == v , "Wrong arc lookup" );
    }
   }
  }
 }

void checkConcepts()
{
 {  // Checking graph components
  checkConcept< BaseGraphComponent , BaseGraphComponent >();

  checkConcept< IDableGraphComponent<> , IDableGraphComponent<> >();

  checkConcept< IterableGraphComponent<> , IterableGraphComponent<> >();

  checkConcept< MappableGraphComponent<> , MappableGraphComponent<> >();
  }
 {  // Checking skeleton graph
  checkConcept< Graph , Graph >();
  }
 {  // Checking ListGraph
  checkConcept< Graph , ListGraph >();
  checkConcept< AlterableGraphComponent<> , ListGraph >();
  checkConcept< ExtendableGraphComponent<> , ListGraph >();
  checkConcept< ClearableGraphComponent<> , ListGraph >();
  checkConcept< ErasableGraphComponent<> , ListGraph >();
  }
 {  // Checking SmartGraph
  checkConcept< Graph , SmartGraph >();
  checkConcept< Graph , StaticGraph >();
  checkConcept< ClearableGraphComponent<> , StaticGraph >();
  checkConcept< AlterableGraphComponent<> , SmartGraph >();
  checkConcept< ExtendableGraphComponent<> , SmartGraph >();
  checkConcept< ClearableGraphComponent<> , SmartGraph >();
  }
 {  // Checking FullGraph
  checkConcept< Graph , FullGraph >();
  }
 {  // Checking GridGraph
  checkConcept< Graph , GridGraph >();
  }
 {  // Checking HypercubeGraph
  checkConcept< Graph , HypercubeGraph >();
  }
 }

template< typename Graph >
void checkGraphValidity()
{
 TEMPLATE_GRAPH_TYPEDEFS( Graph );
 Graph g;

 Node n1 = g.addNode() , n2 = g.addNode() , n3 = g.addNode();

 Edge e1 = g.addEdge( n1 , n2 ) , e2 = g.addEdge( n2 , n3 );
 ::lemon::ignore_unused_variable_warning( e2 );

 check( g.valid( n1 ) , "Wrong validity check" );
 check( g.valid( e1 ) , "Wrong validity check" );
 check( g.valid( g.direct( e1 , true ) ) , "Wrong validity check" );

 check( ! g.valid( g.nodeFromId( -1 ) ) , "Wrong validity check" );
 check( ! g.valid( g.edgeFromId( -1 ) ) , "Wrong validity check" );
 check( ! g.valid( g.arcFromId( -1 ) ) , "Wrong validity check" );
 }

template< typename Graph >
void checkGraphValidityErase()
{
 TEMPLATE_GRAPH_TYPEDEFS( Graph );
 Graph g;

 Node n1 = g.addNode() , n2 = g.addNode() , n3 = g.addNode();

 Edge e1 = g.addEdge( n1 , n2 ) , e2 = g.addEdge( n2 , n3 );

 check( g.valid( n1 ) , "Wrong validity check" );
 check( g.valid( e1 ) , "Wrong validity check" );
 check( g.valid( g.direct( e1 , true ) ) , "Wrong validity check" );

 g.erase( n1 );

 check( ! g.valid( n1 ) , "Wrong validity check" );
 check( g.valid( n2 ) , "Wrong validity check" );
 check( g.valid( n3 ) , "Wrong validity check" );
 check( ! g.valid( e1 ) , "Wrong validity check" );
 check( g.valid( e2 ) , "Wrong validity check" );

 check( ! g.valid( g.nodeFromId( -1 ) ) , "Wrong validity check" );
 check( ! g.valid( g.edgeFromId( -1 ) ) , "Wrong validity check" );
 check( ! g.valid( g.arcFromId( -1 ) ) , "Wrong validity check" );
 }

void checkGridGraph( int width , int height )
{
 typedef GridGraph Graph;
 GRAPH_TYPEDEFS( Graph );
 Graph G( width , height );

 check( G.width() == width , "Wrong column number" );
 check( G.height() == height , "Wrong row number" );

 G.resize( width , height );
 check( G.width() == width , "Wrong column number" );
 check( G.height() == height , "Wrong row number" );

 for( int i = 0 ; i < width ; ++i ) {
  for( int j = 0 ; j < height ; ++j ) {
   check( G.col( G( i , j ) ) == i , "Wrong column" );
   check( G.row( G( i , j ) ) == j , "Wrong row" );
   check( G.pos( G( i , j ) ).x == i , "Wrong column" );
   check( G.pos( G( i , j ) ).y == j , "Wrong row" );
   }
  }

 for( int j = 0 ; j < height ; ++j ) {
  for( int i = 0 ; i < width - 1 ; ++i ) {
   check( G.source( G.right( G( i , j ) ) ) == G( i , j ) , "Wrong right" );
   check( G.target( G.right( G( i , j ) ) ) == G( i + 1 , j ) ,
	  "Wrong right" );
   }
  check( G.right( G( width - 1 , j ) ) == INVALID , "Wrong right" );
  }

 for( int j = 0 ; j < height ; ++j ) {
  for( int i = 1 ; i < width ; ++i ) {
   check( G.source( G.left( G( i , j ) ) ) == G( i , j ) , "Wrong left" );
   check( G.target( G.left( G( i , j ) ) ) == G( i - 1 , j ) , "Wrong left" );
   }
  check( G.left( G( 0 , j ) ) == INVALID , "Wrong left" );
  }

 for( int i = 0 ; i < width ; ++i ) {
  for( int j = 0 ; j < height - 1 ; ++j ) {
   check( G.source( G.up( G( i , j ) ) ) == G( i , j ) , "Wrong up" );
   check( G.target( G.up( G( i , j ) ) ) == G( i , j + 1 ) , "Wrong up" );
   }
  check( G.up( G( i , height - 1 ) ) == INVALID , "Wrong up" );
  }

 for( int i = 0 ; i < width ; ++i ) {
  for( int j = 1 ; j < height ; ++j ) {
   check( G.source( G.down( G( i , j ) ) ) == G( i , j ) , "Wrong down" );
   check( G.target( G.down( G( i , j ) ) ) == G( i , j - 1 ) , "Wrong down" );
   }
  check( G.down( G( i , 0 ) ) == INVALID , "Wrong down" );
  }

 checkGraphNodeList( G , width * height );
 checkGraphEdgeList( G , width * ( height - 1 ) + ( width - 1 ) * height );
 checkGraphArcList( G ,
		    2 * ( width * ( height - 1 ) + ( width - 1 ) * height ) );

 for( NodeIt n( G ) ; n != INVALID ; ++n ) {
  int nb = 4;
  if( G.col( n ) == 0 )
   --nb;
  if( G.col( n ) == width - 1 )
   --nb;
  if( G.row( n ) == 0 )
   --nb;
  if( G.row( n ) == height - 1 )
   --nb;

  checkGraphOutArcList( G , n , nb );
  checkGraphInArcList( G , n , nb );
  checkGraphIncEdgeList( G , n , nb );
  }

 checkArcDirections( G );

 checkGraphConArcList(
   G , 2 * ( width * ( height - 1 ) + ( width - 1 ) * height ) );
 checkGraphConEdgeList( G , width * ( height - 1 ) + ( width - 1 ) * height );

 checkNodeIds( G );
 checkArcIds( G );
 checkEdgeIds( G );
 checkGraphNodeMap( G );
 checkGraphArcMap( G );
 checkGraphEdgeMap( G );
 }

void checkHypercubeGraph( int dim )
{
 GRAPH_TYPEDEFS( HypercubeGraph );

 HypercubeGraph G( dim );
 check( G.dimension() == dim , "Wrong dimension" );

 G.resize( dim );
 check( G.dimension() == dim , "Wrong dimension" );

 checkGraphNodeList( G , 1 << dim );
 checkGraphEdgeList( G , dim * ( 1 << ( dim - 1 ) ) );
 checkGraphArcList( G , dim * ( 1 << dim ) );

 Node n = G.nodeFromId( dim );
 ::lemon::ignore_unused_variable_warning( n );

 for( NodeIt n( G ) ; n != INVALID ; ++n ) {
  checkGraphIncEdgeList( G , n , dim );
  for( IncEdgeIt e( G , n ) ; e != INVALID ; ++e ) {
   check( ( G.u( e ) == n &&
	    G.id( G.v( e ) ) == ( G.id( n ) ^ ( 1 << G.dimension( e ) ) ) ) ||
	    ( G.v( e ) == n &&
	      G.id( G.u( e ) ) == ( G.id( n ) ^ ( 1 << G.dimension( e ) ) ) ) ,
	  "Wrong edge or wrong dimension" );
   }

  checkGraphOutArcList( G , n , dim );
  for( OutArcIt a( G , n ) ; a != INVALID ; ++a ) {
   check( G.source( a ) == n &&
	    G.id( G.target( a ) ) == ( G.id( n ) ^ ( 1 << G.dimension( a ) ) ) ,
	  "Wrong arc or wrong dimension" );
   }

  checkGraphInArcList( G , n , dim );
  for( InArcIt a( G , n ) ; a != INVALID ; ++a ) {
   check( G.target( a ) == n &&
	    G.id( G.source( a ) ) == ( G.id( n ) ^ ( 1 << G.dimension( a ) ) ) ,
	  "Wrong arc or wrong dimension" );
   }
  }

 checkGraphConArcList( G , ( 1 << dim ) * dim );
 checkGraphConEdgeList( G , dim * ( 1 << ( dim - 1 ) ) );

 checkArcDirections( G );

 checkNodeIds( G );
 checkArcIds( G );
 checkEdgeIds( G );
 checkGraphNodeMap( G );
 checkGraphArcMap( G );
 checkGraphEdgeMap( G );
 }

// StaticGraph, built from a random ListGraph with loops and parallel edges
// by GraphCopy and by its list of edges, against that graph
void checkStaticGraph()
{
 GRAPH_TYPEDEFS( StaticGraph );

 StaticGraph G;
 checkGraphNodeList( G , 0 );
 checkGraphEdgeList( G , 0 );
 checkGraphArcList( G , 0 );

 for( int round = 0 ; round < 3 ; ++round ) {
  ListGraph L;
  std::vector< ListGraph::Node > ln;
  int n = 1 + rnd[ 12 ];
  for( int i = 0 ; i < n ; ++i ) {
   ln.push_back( L.addNode() );
   }
  std::vector< std::pair< int , int > > ends;
  for( int k = rnd[ 3 * n ] ; k > 0 ; --k ) {
   int i = rnd[ n ] , j = rnd[ n ];
   L.addEdge( ln[ i ] , ln[ j ] );
   ends.push_back( std::make_pair( i , j ) );
   }

  // a map of G, which the builds below clear and rebuild
  IntNodeMap degree( G , 0 );
  ListGraph::NodeMap< Node > nref( L );
  ListGraph::EdgeMap< Edge > eref( L );
  for( int how = 0 ; how < 2 ; ++how ) {
   if( how == 0 ) {
    graphCopy( L , G ).nodeRef( nref ).edgeRef( eref ).run();
    }
   else {
    G.build( n , ends.begin() , ends.end() );
    int k = 0;
    for( int i = 0 ; i < n ; ++i ) {
     nref[ ln[ i ] ] = G.node( i );
     }
    // the edges of L, in the order in which they were added
    std::vector< ListGraph::Edge > le;
    for( ListGraph::EdgeIt e( L ) ; e != INVALID ; ++e ) {
     le.push_back( e );
     }
    std::sort( le.begin() , le.end() ,
	       [ &L ]( ListGraph::Edge a , ListGraph::Edge b ) {
		return( L.id( a ) < L.id( b ) );
	        } );
    for( ListGraph::Edge e : le ) {
     eref[ e ] = G.edge( k++ );
     }
    }
   checkGraphNodeList( G , n );
   checkGraphEdgeList( G , int( ends.size() ) );
   checkGraphArcList( G , 2 * int( ends.size() ) );
   for( ListGraph::EdgeIt e( L ) ; e != INVALID ; ++e ) {
    check( G.u( eref[ e ] ) == nref[ L.u( e ) ] &&
	     G.v( eref[ e ] ) == nref[ L.v( e ) ] ,
	   "Wrong ends of an edge" );
    }
   for( int i = 0 ; i < n ; ++i ) {
    Node v = nref[ ln[ i ] ];
    std::vector< int > inc_l , inc_g;
    for( ListGraph::IncEdgeIt e( L , ln[ i ] ) ; e != INVALID ; ++e ) {
     inc_l.push_back( G.id( eref[ e ] ) );
     }
    for( IncEdgeIt e( G , v ) ; e != INVALID ; ++e ) {
     inc_g.push_back( G.id( e ) );
     }
    std::sort( inc_l.begin() , inc_l.end() );
    std::sort( inc_g.begin() , inc_g.end() );
    check( inc_l == inc_g , "Wrong incident edges" );
    checkGraphIncEdgeArcLists( G , v , int( inc_g.size() ) );
    for( OutArcIt a( G , v ) ; a != INVALID ; ++a ) {
     check( G.source( a ) == v , "Wrong source of an out-arc" );
     }
    for( InArcIt a( G , v ) ; a != INVALID ; ++a ) {
     check( G.target( a ) == v , "Wrong target of an in-arc" );
     }
    check( G.index( v ) == G.id( v ) && G.node( G.index( v ) ) == v ,
	   "Wrong index" );
    }
   checkGraphConArcList( G , 2 * int( ends.size() ) );
   checkGraphConEdgeList( G , int( ends.size() ) );
   checkArcDirections( G );
   checkNodeIds( G );
   checkArcIds( G );
   checkEdgeIds( G );
   checkGraphNodeMap( G );
   checkGraphArcMap( G );
   checkGraphEdgeMap( G );
   for( NodeIt v( G ) ; v != INVALID ; ++v ) {
    degree[ v ] = 1;
    }
   }
  }
 G.clear();
 checkGraphNodeList( G , 0 );
 checkGraphEdgeList( G , 0 );
 }

void checkGraphs()
{
 {  // Checking ListGraph
  checkGraphBuild< ListGraph >();
  checkGraphAlter< ListGraph >();
  checkGraphErase< ListGraph >();
  checkGraphSnapshot< ListGraph >();
  checkGraphValidityErase< ListGraph >();
  }
 {  // Checking SmartGraph
  checkGraphBuild< SmartGraph >();
  checkGraphSnapshot< SmartGraph >();
  checkGraphValidity< SmartGraph >();
  }
 {  // Checking StaticGraph
  checkStaticGraph();
  }
 {  // Checking FullGraph
  checkFullGraph( 7 );
  checkFullGraph( 8 );
  }
 {  // Checking GridGraph
  checkGridGraph( 5 , 8 );
  checkGridGraph( 8 , 5 );
  checkGridGraph( 5 , 5 );
  checkGridGraph( 0 , 0 );
  checkGridGraph( 1 , 1 );
  }
 {  // Checking HypercubeGraph
  checkHypercubeGraph( 1 );
  checkHypercubeGraph( 2 );
  checkHypercubeGraph( 3 );
  checkHypercubeGraph( 4 );
  }
 }

int main()
{
 checkConcepts();
 checkGraphs();
 return( 0 );
 }
