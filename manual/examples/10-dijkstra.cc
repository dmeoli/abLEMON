// Dijkstra's algorithm and the two point-to-point searches of sections
// 10.2 and 10.3 on a grid: the distances checked against the optimality
// conditions (10.2), two sources, the function-type interface, a path of
// least bottleneck by other operation traits, and BidirDijkstra and AStar
// with the Manhattan distance as the heuristic.

#include <cstdlib>
#include <iostream>
#include <vector>
#include <lemon/list_graph.h>
#include <lemon/dijkstra.h>
#include <lemon/bidir_dijkstra.h>
#include <lemon/astar.h>
#include <lemon/maps.h>
#include <lemon/path.h>

using namespace lemon;

typedef ListDigraph::Node Node;
typedef ListDigraph::Arc Arc;
typedef ListDigraph::ArcMap< int > LengthMap;

// the operations of a path of least bottleneck: the "length" of a path is
// its longest arc, which is nondecreasing along the path as a sum of
// nonnegative lengths is
struct BottleneckOps
{
 typedef int Value;
 static Value zero() { return( 0 ); }
 Value plus( const Value & l , const Value & r ) const {
  return( l < r ? r : l );
  }
 bool less( const Value & l , const Value & r ) const { return( l < r ); }
 };

int main()
{
 const int K = 12;  // a K x K grid, with an arc each way between neighbours
 ListDigraph g;
 std::vector< Node > v( K * K );
 for( int i = 0 ; i < K * K ; ++i )
  v[ i ] = g.addNode();
 LengthMap len( g );
 ListDigraph::NodeMap< int > row( g ) , col( g );
 for( int r = 0 ; r < K ; ++r )
  for( int c = 0 ; c < K ; ++c ) {
   row[ v[ r * K + c ] ] = r;
   col[ v[ r * K + c ] ] = c;
   // the lengths are at least 1, so that the Manhattan distance to the
   // target is a consistent heuristic
   if( c + 1 < K ) {
    len[ g.addArc( v[ r * K + c ] , v[ r * K + c + 1 ] ) ] =
      1 + ( r * 7 + c * 3 ) % 5;
    len[ g.addArc( v[ r * K + c + 1 ] , v[ r * K + c ] ) ] =
      1 + ( r * 3 + c * 5 ) % 4;
    }
   if( r + 1 < K ) {
    len[ g.addArc( v[ r * K + c ] , v[ ( r + 1 ) * K + c ] ) ] =
      1 + ( r * 5 + c * 7 ) % 6;
    len[ g.addArc( v[ ( r + 1 ) * K + c ] , v[ r * K + c ] ) ] =
      1 + ( r + c ) % 3;
    }
   }
 Node s = v[ 0 ] , t = v[ K * K - 1 ];
 int bad = 0;

 // one source: the distances satisfy (10.2), tight on the tree arcs
 Dijkstra< ListDigraph , LengthMap > dijkstra( g , len );
 dijkstra.run( s );
 for( ListDigraph::ArcIt a( g ) ; a != INVALID ; ++a ) {
  int red =
    len[ a ] + dijkstra.dist( g.source( a ) ) - dijkstra.dist( g.target( a ) );
  if( red < 0 || ( dijkstra.predArc( g.target( a ) ) == a && red != 0 ) )
   ++bad;
  }
 int d_st = dijkstra.dist( t );

 // two sources at distance 0: the distance from the nearer one
 Dijkstra< ListDigraph , LengthMap > multi( g , len );
 multi.init();
 multi.addSource( s );
 multi.addSource( t );
 multi.start();
 Dijkstra< ListDigraph , LengthMap > from_t( g , len );
 from_t.run( t );
 for( int i = 0 ; i < K * K ; ++i ) {
  int a = dijkstra.dist( v[ i ] ) , b = from_t.dist( v[ i ] );
  if( multi.dist( v[ i ] ) != ( a < b ? a : b ) )
   ++bad;
  }

 // the function-type interface, stopped when t is processed
 Path< ListDigraph > p;
 int d = 0;
 if( ! lemon::dijkstra( g , len ).path( p ).dist( d ).run( s , t ) ||
     d != d_st || ! checkPath( g , p ) || pathTarget( g , p ) != t )
  ++bad;

 // a path whose longest arc is as short as possible
 Dijkstra< ListDigraph , LengthMap >::SetOperationTraits<
   BottleneckOps >::Create bottleneck( g , len );
 bottleneck.run( s );
 int longest = 0;
 Path< ListDigraph > bp = bottleneck.path( t );
 for( Path< ListDigraph >::ArcIt a( bp ) ; a != INVALID ; ++a )
  if( len[ a ] > longest )
   longest = len[ a ];
 if( longest != bottleneck.dist( t ) || longest > 3 )
  ++bad;

 // the two point-to-point searches agree with Dijkstra
 BidirDijkstra< ListDigraph , LengthMap > bidir( g , len );
 if( ! bidir.run( s , t ) || bidir.dist() != d_st ||
     ! checkPath( g , bidir.path() ) || pathSource( g , bidir.path() ) != s )
  ++bad;
 ListDigraph::NodeMap< int > manhattan( g );
 for( int i = 0 ; i < K * K ; ++i )
  manhattan[ v[ i ] ] = std::abs( row[ v[ i ] ] - row[ t ] ) +
			std::abs( col[ v[ i ] ] - col[ t ] );
 typedef ListDigraph::NodeMap< int > HeuristicMap;
 AStar< ListDigraph , LengthMap , HeuristicMap > astar( g , len , manhattan );
 typedef ConstMap< Node , int > ZeroMap;
 ZeroMap zero( 0 );
 AStar< ListDigraph , LengthMap , ZeroMap > blind( g , len , zero );
 if( ! astar.run( s , t ) || astar.dist() != d_st || ! blind.run( s , t ) ||
     blind.dist() != d_st )
  ++bad;
 // a consistent heuristic scans each node at most once
 if( astar.scanned() > blind.scanned() || blind.scanned() > K * K )
  ++bad;

 std::cout << "distance " << d_st << ", A* scanned " << astar.scanned()
	   << " nodes, with h = 0 " << blind.scanned() << "; " << bad
	   << " errors" << std::endl;
 return( bad == 0 ? 0 : 1 );
 }
