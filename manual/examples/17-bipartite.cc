// The matchings of a bipartite graph (sections 17.2 and 17.3): a maximum
// matching by HopcroftKarp with the vertex cover that proves it, and a
// minimum cost assignment of the red nodes by MinCostAssignment, checked
// against its dual potentials and against all the assignments.

#include <algorithm>
#include <iostream>
#include <vector>
#include <lemon/list_graph.h>
#include <lemon/bipartite_matching.h>

using namespace lemon;

typedef ListBpGraph BpGraph;

int main()
{
 BpGraph g;
 std::vector< BpGraph::RedNode > r;
 std::vector< BpGraph::BlueNode > b;
 for( int i = 0 ; i < 4 ; ++i )
  r.push_back( g.addRedNode() );
 for( int j = 0 ; j < 5 ; ++j )
  b.push_back( g.addBlueNode() );

 // cost[i][j] < 0 means that r_i and b_j are not adjacent
 const int cost[ 4 ][ 5 ] = { { 4 , 1 , -1 , -1 , 7 } ,
			      { 2 , -1 , 3 , -1 , -1 } ,
			      { -1 , 2 , 5 , 4 , -1 } ,
			      { 3 , -1 , 6 , -1 , 2 } };
 BpGraph::EdgeMap< int > c( g );
 for( int i = 0 ; i < 4 ; ++i )
  for( int j = 0 ; j < 5 ; ++j )
   if( cost[ i ][ j ] >= 0 )
    c[ g.addEdge( r[ i ] , b[ j ] ) ] = cost[ i ][ j ];

 // maximum cardinality matching and minimum vertex cover (Konig)
 HopcroftKarp< BpGraph > hk( g );
 hk.run();
 int covered = 0;
 for( BpGraph::NodeIt n( g ) ; n != INVALID ; ++n )
  if( hk.cover( n ) )
   ++covered;
 for( BpGraph::EdgeIt e( g ) ; e != INVALID ; ++e )
  if( ! hk.cover( g.u( e ) ) && ! hk.cover( g.v( e ) ) )
   return( 1 );
 std::cout << "maximum matching: " << hk.matchingSize()
	   << ", vertex cover: " << covered << std::endl;
 if( hk.matchingSize() != 4 || covered != 4 )
  return( 1 );

 // minimum cost assignment of the red nodes
 MinCostAssignment< BpGraph > mca( g , c );
 if( ! mca.run() )
  return( 1 );
 for( BpGraph::EdgeIt e( g ) ; e != INVALID ; ++e ) {
  int slack = c[ e ] - mca.potential( g.blueNode( e ) ) +
	      mca.potential( g.redNode( e ) );
  if( slack < 0 || ( mca.matching( e ) && slack != 0 ) )
   return( 1 );
  }

 // the best assignment by enumeration of the injections of r into b
 int best = -1;
 std::vector< int > p;
 for( int j = 0 ; j < 5 ; ++j )
  p.push_back( j );
 do {
  int sum = 0;
  bool ok = true;
  for( int i = 0 ; i < 4 && ok ; ++i ) {
   ok = cost[ i ][ p[ i ] ] >= 0;
   sum += cost[ i ][ p[ i ] ];
   }
  if( ok && ( best < 0 || sum < best ) )
   best = sum;
  } while( std::next_permutation( p.begin() , p.end() ) );

 // the cost of the assignment from the potentials alone
 int dual = 0;
 for( int i = 0 ; i < 4 ; ++i )
  dual += mca.potential( g.blueNode( mca.matching( r[ i ] ) ) ) -
	  mca.potential( r[ i ] );
 if( dual != mca.totalCost() )
  return( 1 );

 std::cout << "minimum cost assignment: " << mca.totalCost()
	   << " (enumeration: " << best << ")" << std::endl;
 for( int i = 0 ; i < 4 ; ++i )
  for( int j = 0 ; j < 5 ; ++j )
   if( mca.matching( r[ i ] ) != INVALID &&
       g.blueNode( mca.matching( r[ i ] ) ) == b[ j ] )
    std::cout << "  r" << i << " -> b" << j << std::endl;
 return( mca.totalCost() == best ? 0 : 1 );
 }
