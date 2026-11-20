// A maximum matching of a general graph by MaxMatching (section 17.4), on
// the graph of Figure 17.1: a triangle and two single nodes hang from the
// node x, which is also joined to a square. The Gallai-Edmonds
// decomposition proves the matching maximum by the Tutte-Berge formula.

#include <iostream>
#include <lemon/list_graph.h>
#include <lemon/connectivity.h>
#include <lemon/adaptors.h>
#include <lemon/matching.h>

using namespace lemon;

int main()
{
 ListGraph g;
 ListGraph::Node a = g.addNode() , b = g.addNode() , c = g.addNode() ,
		 d = g.addNode() , e = g.addNode() , x = g.addNode() ,
		 p = g.addNode() , q = g.addNode() , r = g.addNode() ,
		 s = g.addNode();
 g.addEdge( a , b );
 g.addEdge( b , c );
 g.addEdge( c , a );
 g.addEdge( x , a );
 g.addEdge( x , d );
 g.addEdge( x , e );
 g.addEdge( x , p );
 g.addEdge( p , q );
 g.addEdge( q , r );
 g.addEdge( r , s );
 g.addEdge( s , p );

 MaxMatching< ListGraph > mm( g );
 mm.run();

 // the components of the subgraph induced by the EVEN nodes, and the
 // barrier made of the ODD nodes
 ListGraph::NodeMap< bool > even( g );
 int barrier = 0;
 for( ListGraph::NodeIt n( g ) ; n != INVALID ; ++n ) {
  even[ n ] = mm.status( n ) == MaxMatching< ListGraph >::EVEN;
  if( mm.barrier( n ) )
   ++barrier;
  }
 FilterNodes< ListGraph > d_graph( g , even );
 int odd_comps = countConnectedComponents( d_graph );

 // Tutte-Berge: n - 2 |M| = (odd components of G - A) - |A|
 int deficiency = countNodes( g ) - 2 * mm.matchingSize();
 std::cout << "matching size " << mm.matchingSize() << ", " << odd_comps
	   << " components in D, " << barrier << " barrier node(s)"
	   << std::endl;
 for( ListGraph::NodeIt n( g ) ; n != INVALID ; ++n )
  std::cout << "  node " << g.id( n ) << ": status " << int( mm.status( n ) )
	    << ", mate "
	    << ( mm.mate( n ) == INVALID ? -1 : g.id( mm.mate( n ) ) )
	    << std::endl;
 if( mm.matchingSize() != 4 || deficiency != odd_comps - barrier )
  return( 1 );
 if( ! mm.barrier( x ) )
  return( 1 );
 if( mm.status( p ) != MaxMatching< ListGraph >::MATCHED )
  return( 1 );

 // without the decomposition the size is the same
 MaxMatching< ListGraph > fast( g );
 fast.run( false );
 return( fast.matchingSize() == mm.matchingSize() ? 0 : 1 );
 }
