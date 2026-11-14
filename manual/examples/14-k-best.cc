// The k best flows of sections 14.3 to 14.5 on a network small enough to
// enumerate all its integer flows: KBestFlows must give the k cheapest of
// them in order, KBestExtremeFlows the k cheapest vertices (the flows
// whose arcs strictly within their bounds make no cycle), by the ranking
// and by the cycles of least reduced cost, and runPotentials() optimal
// potentials of the optimal flow, by both of its methods.

#include <algorithm>
#include <iostream>
#include <vector>
#include <lemon/list_graph.h>
#include <lemon/k_best_flows.h>
#include <lemon/k_best_extreme_flows.h>

using namespace lemon;

typedef ListDigraph::ArcMap< int > ArcMap;
typedef ListDigraph::NodeMap< int > NodeMap;

const int M = 5;
const int SRC[ M ] = { 0 , 0 , 1 , 1 , 2 } , TGT[ M ] = { 1 , 2 , 2 , 3 , 3 };
const int CAP[ M ] = { 2 , 2 , 1 , 2 , 2 } , COST[ M ] = { 1 , 2 , 0 , 3 , 1 };
const int SUP[ 4 ] = { 2 , 0 , 0 , -2 };

// Whether the arcs strictly within their bounds make no cycle
bool vertex( const std::vector< int > & f )
{
 int p[ 4 ] = { 0 , 1 , 2 , 3 };
 for( int i = 0 ; i != M ; ++i ) {
  if( f[ i ] == 0 || f[ i ] == CAP[ i ] )
   continue;
  int a = SRC[ i ] , b = TGT[ i ];
  while( p[ a ] != a )
   a = p[ a ];
  while( p[ b ] != b )
   b = p[ b ];
  if( a == b )
   return( false );
  p[ a ] = b;
  }
 return( true );
 }

int main()
{
 ListDigraph g;
 std::vector< ListDigraph::Node > node;
 for( int u = 0 ; u != 4 ; ++u )
  node.push_back( g.addNode() );
 ArcMap upper( g ) , cost( g );
 NodeMap supply( g );
 std::vector< ListDigraph::Arc > arc;
 for( int i = 0 ; i != M ; ++i ) {
  arc.push_back( g.addArc( node[ SRC[ i ] ] , node[ TGT[ i ] ] ) );
  upper[ arc[ i ] ] = CAP[ i ];
  cost[ arc[ i ] ] = COST[ i ];
  }
 for( int u = 0 ; u != 4 ; ++u )
  supply[ node[ u ] ] = SUP[ u ];
 int errors = 0;

 // all the integer flows, and the costs of those and of the vertices
 std::vector< int > all , vertices;
 std::vector< int > f( M , 0 );
 for( int code = 0 ; code != 3 * 3 * 2 * 3 * 3 ; ++code ) {
  int c = code;
  for( int i = 0 ; i != M ; ++i ) {
   f[ i ] = c % ( CAP[ i ] + 1 );
   c /= CAP[ i ] + 1;
   }
  int net[ 4 ] = { 0 , 0 , 0 , 0 } , total = 0;
  for( int i = 0 ; i != M ; ++i ) {
   net[ SRC[ i ] ] += f[ i ];
   net[ TGT[ i ] ] -= f[ i ];
   total += COST[ i ] * f[ i ];
   }
  if( net[ 0 ] != SUP[ 0 ] || net[ 1 ] != SUP[ 1 ] || net[ 2 ] != SUP[ 2 ] ||
      net[ 3 ] != SUP[ 3 ] )
   continue;
  all.push_back( total );
  if( vertex( f ) )
   vertices.push_back( total );
  }
 std::sort( all.begin() , all.end() );
 std::sort( vertices.begin() , vertices.end() );

 // the k best integer flows
 KBestFlows< ListDigraph > kbf( g );
 kbf.upperMap( upper ).costMap( cost ).supplyMap( supply );
 const int k = kbf.run( 100 );
 std::cout << k << " integer flows (" << all.size() << " enumerated):";
 for( int i = 0 ; i != k ; ++i )
  std::cout << ' ' << kbf.totalCost( i );
 std::cout << std::endl;
 if( k != int( all.size() ) )
  ++errors;
 for( int i = 0 ; i != k && i != int( all.size() ) ; ++i )
  if( kbf.totalCost( i ) != all[ i ] )
   ++errors;

 // the k best vertices, by the ranking of Murty and by the cycles
 KBestExtremeFlows< ListDigraph > kef( g );
 kef.upperMap( upper ).costMap( cost ).supplyMap( supply );
 const int kv = kef.run( 100 );
 std::cout << kv << " vertices (" << vertices.size() << " enumerated):";
 for( int i = 0 ; i != kv ; ++i )
  std::cout << ' ' << kef.totalCost( i );
 std::cout << std::endl;
 if( ! kef.exact() || kv != int( vertices.size() ) )
  ++errors;
 for( int i = 0 ; i != kv && i != int( vertices.size() ) ; ++i )
  if( kef.totalCost( i ) != vertices[ i ] )
   ++errors;
 kef.method( KBestExtremeFlows< ListDigraph >::CYCLES );
 const int kc = kef.run( 3 );
 std::cout << "by the cycles:";
 for( int i = 0 ; i != kc ; ++i ) {
  std::cout << ' ' << kef.totalCost( i );
  if( kef.totalCost( i ) < vertices[ 0 ] ||
      ( i > 0 && kef.totalCost( i ) < kef.totalCost( i - 1 ) ) )
   ++errors;
  }
 std::cout << std::endl;
 if( kc == 0 || kef.totalCost( 0 ) != vertices[ 0 ] || kef.exact() )
  ++errors;

 // optimal potentials: every residual arc of the optimal flow has a
 // nonnegative reduced cost, and no node a positive potential (GEQ)
 for( int method = 0 ; method != 2 ; ++method ) {
  kef.potentialMethod( method == 0
			 ? KBestExtremeFlows< ListDigraph >::PATHS
			 : KBestExtremeFlows< ListDigraph >::PIVOTS );
  const int kp = kef.runPotentials( 10 );
  std::cout << kp << " optimal potentials by "
	    << ( method == 0 ? "PATHS" : "PIVOTS" ) << std::endl;
  if( kp == 0 )
   ++errors;
  for( int j = 0 ; j != kp ; ++j ) {
   for( int i = 0 ; i != M ; ++i ) {
    const int x = kbf.flow( 0 , arc[ i ] );
    const int rc = COST[ i ] + kef.potential( j , node[ SRC[ i ] ] ) -
		   kef.potential( j , node[ TGT[ i ] ] );
    if( ( x < CAP[ i ] && rc < 0 ) || ( x > 0 && rc > 0 ) )
     ++errors;
    }
   for( int u = 0 ; u != 4 ; ++u )
    if( kef.potential( j , node[ u ] ) > 0 )
     ++errors;
   }
  }
 return( errors );
 }
