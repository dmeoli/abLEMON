// The union-find structures of section 8.2: the components of a graph by
// UnionFind, the classes enumerated by UnionFindEnum, and the minimum of
// each class kept by HeapUnionFind through a join and a split.

#include <iostream>
#include <iterator>
#include <vector>
#include <lemon/list_graph.h>
#include <lemon/unionfind.h>

using namespace lemon;

int main()
{
 typedef ListGraph::Node Node;
 ListGraph g;
 std::vector< Node > v;
 for( int i = 0 ; i < 6 ; ++i )
  v.push_back( g.addNode() );
 // two components, {0, 1, 2} and {3, 4}, and the isolated node 5
 g.addEdge( v[ 0 ] , v[ 1 ] );
 g.addEdge( v[ 1 ] , v[ 2 ] );
 g.addEdge( v[ 2 ] , v[ 0 ] );
 g.addEdge( v[ 3 ] , v[ 4 ] );

 int bad = 0;

 // UnionFind: one join per edge, false on the edge that closes a cycle
 ListGraph::NodeMap< int > index( g );
 UnionFind< ListGraph::NodeMap< int > > uf( index );
 for( ListGraph::NodeIt n( g ) ; n != INVALID ; ++n )
  uf.insert( n );
 int cycles = 0;
 for( ListGraph::EdgeIt e( g ) ; e != INVALID ; ++e )
  if( ! uf.join( g.u( e ) , g.v( e ) ) )
   ++cycles;
 if( cycles != 1 || uf.find( v[ 0 ] ) != uf.find( v[ 2 ] ) ||
     uf.find( v[ 0 ] ) == uf.find( v[ 3 ] ) || uf.size( v[ 1 ] ) != 3 ||
     uf.size( v[ 5 ] ) != 1 )
  ++bad;

 // UnionFindEnum: the same classes, enumerated with their items
 typedef UnionFindEnum< ListGraph::NodeMap< int > > UFE;
 ListGraph::NodeMap< int > index2( g );
 UFE ufe( index2 );
 for( ListGraph::NodeIt n( g ) ; n != INVALID ; ++n )
  ufe.insert( n );
 for( ListGraph::EdgeIt e( g ) ; e != INVALID ; ++e )
  ufe.join( g.u( e ) , g.v( e ) );
 int classes = 0 , items = 0;
 for( UFE::ClassIt c( ufe ) ; c != INVALID ; ++c ) {
  ++classes;
  for( UFE::ItemIt i( ufe , c ) ; i != INVALID ; ++i )
   ++items;
  }
 if( classes != 3 || items != 6 )
  ++bad;

 // HeapUnionFind: priorities 5, 3, 8, 1 on v0 .. v3; join three classes,
 // read the minimum, change a priority, split back
 typedef HeapUnionFind< int , ListGraph::NodeMap< int > > HUF;
 ListGraph::NodeMap< int > index3( g );
 HUF huf( index3 );
 std::vector< int > cls;
 const int prio[] = { 5 , 3 , 8 , 1 };
 for( int i = 0 ; i < 4 ; ++i )
  cls.push_back( huf.insert( v[ i ] , prio[ i ] ) );
 int joined = huf.join( cls.begin() , cls.begin() + 3 );  // v0, v1, v2
 if( huf.classPrio( joined ) != 3 || huf.classTop( joined ) != v[ 1 ] ||
     huf.find( v[ 2 ] ) != joined )
  ++bad;
 huf.decrease( v[ 2 ] , 2 );
 if( huf.classPrio( joined ) != 2 || huf.classTop( joined ) != v[ 2 ] )
  ++bad;
 std::vector< int > parts;
 huf.split( joined , std::back_inserter( parts ) );
 if( parts.size() != 3 || huf.classPrio( huf.find( v[ 0 ] ) ) != 5 ||
     huf.classPrio( huf.find( v[ 3 ] ) ) != 1 )
  ++bad;

 std::cout << classes << " classes, " << cycles << " cycle edge, " << bad
	   << " errors" << std::endl;
 return( bad == 0 ? 0 : 1 );
 }
