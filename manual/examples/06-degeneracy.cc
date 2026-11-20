// Iterable maps (section 6.6): the smallest-last ordering and the core
// number of each node of a graph (Algorithm 6.1), with the nodes kept in
// buckets by their current degree in an IterableIntMap, in O(n + m) time;
// an IterableBoolMap then gives the nodes of the largest core.

#include <iostream>
#include <vector>
#include <lemon/list_graph.h>
#include <lemon/maps.h>

using namespace lemon;

int main()
{
 ListGraph g;
 std::vector< ListGraph::Node > v( 7 );
 for( int i = 0 ; i < 7 ; ++i )
  v[ i ] = g.addNode();
 // a clique on 0, 1, 2, 3, the path 3 - 4 - 5 and the edge 5 - 1, and the
 // pendant node 6 on 4
 for( int i = 0 ; i < 4 ; ++i )
  for( int j = i + 1 ; j < 4 ; ++j )
   g.addEdge( v[ i ] , v[ j ] );
 g.addEdge( v[ 3 ] , v[ 4 ] );
 g.addEdge( v[ 4 ] , v[ 5 ] );
 g.addEdge( v[ 5 ] , v[ 1 ] );
 g.addEdge( v[ 4 ] , v[ 6 ] );

 // the current degree of the nodes not yet removed; -1 for the removed
 // ones, which IterableIntMap leaves out of its lists
 IterableIntMap< ListGraph , ListGraph::Node > deg( g );
 for( ListGraph::NodeIt n( g ) ; n != INVALID ; ++n ) {
  int d = 0;
  for( ListGraph::IncEdgeIt e( g , n ) ; e != INVALID ; ++e )
   ++d;
  deg.set( n , d );
  }

 ListGraph::NodeMap< int > core( g );
 std::vector< ListGraph::Node > order;  // the smallest-last order, reversed
 int k = 0 , d = 0;
 for( int removed = 0 ; removed < countNodes( g ) ; ++removed ) {
  while( IterableIntMap< ListGraph , ListGraph::Node >::ItemIt( deg , d ) ==
	 INVALID )
   ++d;
  ListGraph::Node u =
    IterableIntMap< ListGraph , ListGraph::Node >::ItemIt( deg , d );
  if( d > k )
   k = d;
  core[ u ] = k;
  order.push_back( u );
  deg.set( u , -1 );
  for( ListGraph::IncEdgeIt e( g , u ) ; e != INVALID ; ++e ) {
   ListGraph::Node w = g.runningNode( e );
   if( deg[ w ] > 0 )
    deg.set( w , deg[ w ] - 1 );
   }
  if( d > 0 )
   --d;  // a degree drops by at most one per removal
  }

 // the nodes of the largest core, i.e., of core number k
 IterableBoolMap< ListGraph , ListGraph::Node > top( g , false );
 for( ListGraph::NodeIt n( g ) ; n != INVALID ; ++n )
  top[ n ] = core[ n ] == k;

 std::cout << "degeneracy " << k << ", nodes in the " << k
	   << "-core: " << top.trueNum()
	   << ", first removed: " << g.id( order[ 0 ] ) << std::endl;
 bool ok = k == 3 && top.trueNum() == 4 && core[ v[ 4 ] ] == 2 &&
	   core[ v[ 5 ] ] == 2 && core[ v[ 6 ] ] == 1 && order[ 0 ] == v[ 6 ];
 for( IterableBoolMap< ListGraph , ListGraph::Node >::TrueIt n( top ) ;
      n != INVALID ; ++n )
  ok = ok && g.id( n ) < 4;
 return( ok ? 0 : 1 );
 }
