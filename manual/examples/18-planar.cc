// The planarity algorithms of sections 18.5 and 18.6: the embedding of the
// cube, whose faces are counted against Euler's formula, its straight-line
// drawing on the grid, checked to have no crossing, its colorings with six
// and five colors, and the Kuratowski subdivision found in K3,3.

#include <iostream>
#include <lemon/list_graph.h>
#include <lemon/connectivity.h>
#include <lemon/planarity.h>

using namespace lemon;
typedef ListGraph Graph;
typedef dim2::Point< int > Point;

// The sign of the turn p -> q -> r
long long turn( Point p , Point q , Point r )
{
 return( static_cast< long long >( q.x - p.x ) * ( r.y - p.y ) -
	  static_cast< long long >( q.y - p.y ) * ( r.x - p.x ) );
 }

// Whether the open segments pq and rs cross (they share no end here)
bool cross( Point p , Point q , Point r , Point s )
{
 return( ( ( turn( p , q , r ) > 0 ) != ( turn( p , q , s ) > 0 ) ) &&
	  ( ( turn( r , s , p ) > 0 ) != ( turn( r , s , q ) > 0 ) ) &&
	  turn( p , q , r ) != 0 && turn( p , q , s ) != 0 );
 }

int main()
{
 int errors = 0;

 // The cube: nodes 0..7 are the binary strings of length 3
 Graph g;
 Graph::Node v[ 8 ];
 for( int i = 0 ; i < 8 ; ++i )
  v[ i ] = g.addNode();
 for( int i = 0 ; i < 8 ; ++i )
  for( int b = 1 ; b < 8 ; b <<= 1 )
   if( i < ( i ^ b ) )
    g.addEdge( v[ i ] , v[ i ^ b ] );

 PlanarEmbedding< Graph > pe( g );
 if( ! pe.run() || ! checkPlanarity( g ) )
  ++errors;

 // The faces are the orbits of a -> next(opposite(a))
 Graph::ArcMap< int > face( g , -1 );
 int faces = 0;
 for( Graph::ArcIt a( g ) ; a != INVALID ; ++a ) {
  if( face[ a ] != -1 )
   continue;
  for( Graph::Arc b = a ; face[ b ] == -1 ; b = pe.next( g.oppositeArc( b ) ) )
   face[ b ] = faces;
  ++faces;
  }
 std::cout << "faces of the cube: " << faces << std::endl;
 if( faces != countEdges( g ) - countNodes( g ) + 2 )
  ++errors;

 // The straight-line drawing on the grid
 PlanarDrawing< Graph > pd( g );
 pd.run( pe.embeddingMap() );
 for( int i = 0 ; i < 8 ; ++i )
  std::cout << "node " << i << " at (" << pd[ v[ i ] ].x << ", "
	    << pd[ v[ i ] ].y << ")" << std::endl;
 for( Graph::EdgeIt e( g ) ; e != INVALID ; ++e )
  for( Graph::EdgeIt f( g ) ; f != INVALID ; ++f ) {
   Graph::Node a = g.u( e ) , b = g.v( e ) , c = g.u( f ) , d = g.v( f );
   if( a == c || a == d || b == c || b == d )
    continue;
   if( cross( pd[ a ] , pd[ b ] , pd[ c ] , pd[ d ] ) )
    ++errors;
   }
 for( int i = 0 ; i < 8 ; ++i )
  for( int j = i + 1 ; j < 8 ; ++j )
   if( pd[ v[ i ] ] == pd[ v[ j ] ] )
    ++errors;

 // Six colors by the greedy rule, five with the Kempe recoloring
 PlanarColoring< Graph > six( g ) , five( g );
 if( ! six.runSixColoring() || ! five.runFiveColoring() )
  ++errors;
 for( Graph::EdgeIt e( g ) ; e != INVALID ; ++e ) {
  if( six.colorIndex( g.u( e ) ) == six.colorIndex( g.v( e ) ) )
   ++errors;
  if( five.colorIndex( g.u( e ) ) == five.colorIndex( g.v( e ) ) )
   ++errors;
  }
 for( int i = 0 ; i < 8 ; ++i )
  if( five.colorIndex( v[ i ] ) < 0 || five.colorIndex( v[ i ] ) > 4 )
   ++errors;

 // K3,3 is not planar, and the subdivision found is K3,3 itself
 Graph k;
 Graph::Node r[ 3 ] , s[ 3 ];
 for( int i = 0 ; i < 3 ; ++i )
  r[ i ] = k.addNode() , s[ i ] = k.addNode();
 for( int i = 0 ; i < 3 ; ++i )
  for( int j = 0 ; j < 3 ; ++j )
   k.addEdge( r[ i ] , s[ j ] );
 PlanarEmbedding< Graph > pk( k );
 if( pk.run() || checkPlanarity( k ) )
  ++errors;
 int marked = 0;
 for( Graph::EdgeIt e( k ) ; e != INVALID ; ++e )
  if( pk.kuratowski( e ) )
   ++marked;
 std::cout << "edges of the Kuratowski subdivision of K3,3: " << marked
	   << std::endl;
 if( marked != 9 )
  ++errors;

 return( errors == 0 ? 0 : 1 );
 }
