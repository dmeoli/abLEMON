// Items, iterators and maps (sections 3.2 to 3.4): the edges of an
// undirected graph and their two arcs, the iterators and the ranges, the
// value that a map gives to an item added after it, and a generic
// function whose graph type is checked against the Graph concept.

#include <iostream>
#include <lemon/concepts/graph.h>
#include <lemon/list_graph.h>

using namespace lemon;

// The sum of the weights of the edges incident to v, for any graph type
template< typename GR , typename WM >
typename WM::Value incidentWeight( const GR & g , const WM & w ,
				   typename GR::Node v )
{
 checkConcept< concepts::Graph , GR >();
 typename WM::Value sum = 0;
 for( typename GR::IncEdgeIt e( g , v ) ; e != INVALID ; ++e )
  sum += w[ e ];
 return( sum );
 }

int main()
{
 int errors = 0;
 ListGraph g;
 ListGraph::Node a = g.addNode() , b = g.addNode() , c = g.addNode();
 ListGraph::Edge ab = g.addEdge( a , b ) , bc = g.addEdge( b , c ) ,
		 ca = g.addEdge( c , a );

 // the two arcs of an edge: the forward one goes from u() to v()
 ListGraph::Arc fw = g.direct( ab , true ) , bw = g.direct( ab , false );
 if( g.source( fw ) != g.u( ab ) || g.target( fw ) != g.v( ab ) )
  ++errors;
 if( g.oppositeArc( fw ) != bw || ! g.direction( fw ) || g.direction( bw ) )
  ++errors;
 if( ListGraph::Edge( bw ) != ab )
  ++errors;  // an arc converts to its edge
 if( g.direct( ab , b ) != bw )
  ++errors;  // the arc of ab leaving b
 if( g.oppositeNode( a , ab ) != b )
  ++errors;

 // a default-constructed item is INVALID in every graph of abLEMON
 ListGraph::Node none;
 if( none != INVALID )
  ++errors;

 // iterators and ranges visit the same items
 int nodes = 0 , arcs = 0 , incident = 0;
 for( ListGraph::NodeIt n( g ) ; n != INVALID ; ++n )
  ++nodes;
 for( ListGraph::Arc e : g.arcs() ) {
  ( void )e;
  ++arcs;
  }
 for( ListGraph::IncEdgeIt e( g , a ) ; e != INVALID ; ++e ) {
  if( g.oppositeNode( a , e ) == g.runningNode( e ) )
   ++incident;
  }
 if( nodes != 3 || arcs != 6 || incident != 2 )
  ++errors;

 // a map with an initial value, and an item added after the map
 ListGraph::EdgeMap< int > weight( g , 1 );
 weight[ bc ] = 5;
 weight.set( ca , 2 );
 ListGraph::Node d = g.addNode();
 ListGraph::Edge cd = g.addEdge( c , d );
 if( weight[ cd ] != 0 )
  ++errors;  // a new item gets Value(), not 1
 weight[ cd ] = 4;
 if( incidentWeight( g , weight , c ) != 11 )
  ++errors;

 // ids: unique among the items of a kind, and reused after an erase
 int id_d = g.id( d );
 if( g.nodeFromId( id_d ) != d || id_d > g.maxNodeId() )
  ++errors;
 g.erase( d );  // erases cd too
 if( g.valid( d ) || g.valid( cd ) )
  ++errors;
 ListGraph::Node e = g.addNode();
 if( g.id( e ) != id_d )
  ++errors;  // the freed id is taken again

 std::cout << "items, iterators and maps: " << errors << " errors"
	   << std::endl;
 return( errors == 0 ? 0 : 1 );
 }
