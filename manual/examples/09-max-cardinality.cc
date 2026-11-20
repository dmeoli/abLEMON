// Maximum cardinality search (section 9.6) as a test of chordality: the
// search order of a chordal graph, reversed, is a perfect elimination
// order, i.e., the neighbours of each node searched before it form a
// clique; on a chordless cycle of four nodes the test fails.

#include <iostream>
#include <vector>
#include <lemon/list_graph.h>
#include <lemon/max_cardinality_search.h>

using namespace lemon;

typedef ListGraph::Node Node;

// true if the order of the search is a perfect elimination order reversed
bool chordal( const ListGraph & g , int & bad )
{
 MaxCardinalitySearch< ListGraph > mcs( g );
 std::vector< Node > order;
 mcs.init();
 for( ListGraph::NodeIt n( g ) ; n != INVALID ; ++n )
  if( ! mcs.reached( n ) ) {
   mcs.addSource( n );
   while( ! mcs.emptyQueue() )
    order.push_back( mcs.processNextNode() );
   }
 ListGraph::NodeMap< int > rank( g );
 for( int i = 0 ; i < int( order.size() ) ; ++i )
  rank[ order[ i ] ] = i;
 bool ok = true;
 for( int i = 0 ; i < int( order.size() ) ; ++i ) {
  std::vector< Node > before;  // the neighbours searched before
  for( ListGraph::IncEdgeIt e( g , order[ i ] ) ; e != INVALID ; ++e ) {
   Node w = g.oppositeNode( order[ i ] , e );
   if( rank[ w ] < i )
    before.push_back( w );
   }
  // with unit capacities the cardinality is the number of them
  if( mcs.cardinality( order[ i ] ) != int( before.size() ) )
   ++bad;
  for( std::size_t a = 0 ; a < before.size() ; ++a )
   for( std::size_t b = a + 1 ; b < before.size() ; ++b )
    if( findEdge( g , before[ a ] , before[ b ] ) == INVALID )
     ok = false;
  }
 return( ok );
 }

int main()
{
 int bad = 0;

 // a triangulated hexagon: the cycle 0 .. 5 and the chords 0-2, 0-3, 0-4
 ListGraph g;
 std::vector< Node > v;
 for( int i = 0 ; i < 6 ; ++i )
  v.push_back( g.addNode() );
 for( int i = 0 ; i < 6 ; ++i )
  g.addEdge( v[ i ] , v[ ( i + 1 ) % 6 ] );
 for( int i = 2 ; i <= 4 ; ++i )
  g.addEdge( v[ 0 ] , v[ i ] );
 if( ! chordal( g , bad ) )
  ++bad;

 // the cycle of four nodes, with no chord
 ListGraph c;
 std::vector< Node > w;
 for( int i = 0 ; i < 4 ; ++i )
  w.push_back( c.addNode() );
 for( int i = 0 ; i < 4 ; ++i )
  c.addEdge( w[ i ] , w[ ( i + 1 ) % 4 ] );
 if( chordal( c , bad ) )
  ++bad;

 std::cout << bad << " errors" << std::endl;
 return( bad == 0 ? 0 : 1 );
 }
