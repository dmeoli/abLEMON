// Kruskal's algorithm of section 11.1 on the graph of Figure 11.1, with a
// cost map in and a bool map out, then with a sorted sequence in and an
// output iterator out; the tree is checked by the cycle condition (11.2).

#include <algorithm>
#include <iostream>
#include <iterator>
#include <utility>
#include <vector>
#include <lemon/list_graph.h>
#include <lemon/kruskal.h>
#include <lemon/bfs.h>
#include <lemon/adaptors.h>

using namespace lemon;

typedef ListGraph::Node Node;
typedef ListGraph::Edge Edge;

int main() {
  ListGraph g;
  Node v[ 6 ];
  for( int i = 0 ; i < 6 ; ++i )
    v[ i ] = g.addNode();
  ListGraph::EdgeMap< int > cost( g );
  const int edges[][ 3 ] = { { 0 , 1 , 4 } , { 0 , 2 , 1 } , { 1 , 2 , 2 } ,
                             { 1 , 3 , 5 } , { 2 , 3 , 8 } , { 2 , 4 , 9 } ,
                             { 3 , 4 , 3 } , { 3 , 5 , 6 } , { 4 , 5 , 7 } };
  for( const auto & e : edges )
    cost[ g.addEdge( v[ e[ 0 ] ] , v[ e[ 1 ] ] ) ] = e[ 2 ];
  int bad = 0;

  // a cost map in, a bool map out
  ListGraph::EdgeMap< bool > tree( g );
  int total = kruskal( g , cost , tree );
  int in_tree = 0;
  for( ListGraph::EdgeIt e( g ) ; e != INVALID ; ++e )
    in_tree += tree[ e ];
  if( total != 17 || in_tree != 5 )
    ++bad;

  // (11.2): no edge out of the tree costs less than an edge of the tree
  // path between its ends
  FilterEdges< ListGraph > t( g , tree );
  for( ListGraph::EdgeIt e( g ) ; e != INVALID ; ++e ) {
    if( tree[ e ] )
      continue;
    Bfs< FilterEdges< ListGraph > > bfs( t );
    bfs.run( g.u( e ) , g.v( e ) );
    for( Node n = g.v( e ) ; n != g.u( e ) ; n = bfs.predNode( n ) )
      if( cost[ bfs.predArc( n ) ] > cost[ e ] )
        ++bad;
  }

  // a sequence of (edge, cost) pairs in increasing order of cost in, an
  // output iterator out
  std::vector< std::pair< Edge , int > > seq;
  for( ListGraph::EdgeIt e( g ) ; e != INVALID ; ++e )
    seq.push_back( std::make_pair( Edge( e ) , cost[ e ] ) );
  std::sort( seq.begin() , seq.end() ,
             []( const std::pair< Edge , int > & l ,
                 const std::pair< Edge , int > & r ) {
               return( l.second < r.second );
             } );
  std::vector< Edge > chosen;
  if( kruskal( g , seq , std::back_inserter( chosen ) ) != 17 ||
      chosen.size() != 5 )
    ++bad;
  for( std::size_t i = 0 ; i < chosen.size() ; ++i )
    if( ! tree[ chosen[ i ] ] )
      ++bad;

  std::cout << "tree of cost " << total << " with " << chosen.size()
            << " edges; " << bad << " errors" << std::endl;
  return( bad == 0 ? 0 : 1 );
}
