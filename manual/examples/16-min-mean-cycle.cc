// The minimum mean cycle of a digraph by the three algorithms of chapter 16:
// they must agree on the mean, the cycle they give must be a closed walk of
// the cost and the length they report, and the mean is certified by
// Bellman-Ford, which finds no negative cycle once it is subtracted from
// every arc (Theorem 16.1). HowardMmc also gives the cycle of each strongly
// connected component, and an upper bound when its iterations are limited.

#include <iostream>
#include <vector>
#include <lemon/smart_graph.h>
#include <lemon/path.h>
#include <lemon/karp_mmc.h>
#include <lemon/hartmann_orlin_mmc.h>
#include <lemon/howard_mmc.h>
#include <lemon/bellman_ford.h>

using namespace lemon;

typedef SmartDigraph::ArcMap< int > CostMap;

// true if path is a closed walk of the given cost and number of arcs
template< typename P >
bool isCycle( const SmartDigraph & g , const CostMap & cost , const P & path ,
	      int c , int size )
{
 if( path.length() != size || path.empty() )
  return( false );
 int sum = 0;
 for( int i = 0 ; i < path.length() ; ++i ) {
  SmartDigraph::Arc a = path.nth( i ) ,
		    b = path.nth( ( i + 1 ) % path.length() );
  if( g.target( a ) != g.source( b ) )
   return( false );
  sum += cost[ a ];
  }
 return( sum == c );
 }

template< typename MMC >
bool solve( const char * name , const SmartDigraph & g , const CostMap & cost ,
	    int & c , int & size )
{
 MMC mmc( g , cost );
 if( ! mmc.run() )
  return( false );
 c = mmc.cycleCost();
 size = mmc.cycleSize();
 std::cout << name << ": mean " << mmc.cycleMean() << " = " << c << "/" << size
	   << std::endl;
 return( isCycle( g , cost , mmc.cycle() , c , size ) );
 }

int main()
{
 SmartDigraph g;
 std::vector< SmartDigraph::Node > v;
 for( int i = 0 ; i < 7 ; ++i )
  v.push_back( g.addNode() );
 CostMap cost( g );
 // two strongly connected components, {0,1,2,3} and {4,5,6}, and an arc
 // from the first to the second
 const int arcs[][ 3 ] = { { 0 , 1 , 2 } , { 1 , 2 , 3 } , { 2 , 0 , 1 } ,
			   { 1 , 3 , 1 } ,
			   { 3 , 0 , 4 } , { 2 , 3 , 5 } , { 3 , 4 , -6 } ,
			   { 4 , 5 , 2 } ,
			   { 5 , 4 , 3 } , { 5 , 6 , 1 } , { 6 , 4 , 1 } };
 for( const auto & a : arcs )
  cost[ g.addArc( v[ a[ 0 ] ] , v[ a[ 1 ] ] ) ] = a[ 2 ];

 int c[ 3 ] , size[ 3 ];
 if( ! solve< KarpMmc< SmartDigraph > >( "Karp" , g , cost , c[ 0 ] ,
					 size[ 0 ] ) ||
     ! solve< HartmannOrlinMmc< SmartDigraph > >( "Hartmann-Orlin" , g , cost ,
						  c[ 1 ] , size[ 1 ] ) ||
     ! solve< HowardMmc< SmartDigraph > >( "Howard" , g , cost , c[ 2 ] ,
					   size[ 2 ] ) )
  return( 1 );
 // the same mean (compared without division), here 4/3
 for( int i = 1 ; i < 3 ; ++i )
  if( c[ i ] * size[ 0 ] != c[ 0 ] * size[ i ] )
   return( 1 );
 if( c[ 0 ] * 3 != 4 * size[ 0 ] )
  return( 1 );

 // certificate: the costs size * cost - c have no negative cycle
 CostMap reduced( g );
 for( SmartDigraph::ArcIt a( g ) ; a != INVALID ; ++a )
  reduced[ a ] = size[ 0 ] * cost[ a ] - c[ 0 ];
 BellmanFord< SmartDigraph , CostMap > bf( g , reduced );
 bf.init( 0 );  // a virtual source joined to every node by a zero arc
 if( ! bf.checkedStart() )
  return( 1 );

 // one cycle of minimum mean for each component that has a cycle
 HowardMmc< SmartDigraph > howard( g , cost );
 if( howard.findCycleMean() != HowardMmc< SmartDigraph >::OPTIMAL )
  return( 1 );
 if( howard.componentCycleNum() != 2 )
  return( 1 );
 for( int i = 0 ; i < howard.componentCycleNum() ; ++i ) {
  Path< SmartDigraph > p;
  howard.componentCycle( i , p );
  std::cout << "component cycle " << i << ": "
	    << howard.componentCycleCost( i ) << "/"
	    << howard.componentCycleSize( i ) << std::endl;
  if( ! isCycle( g , cost , p , howard.componentCycleCost( i ) ,
		 howard.componentCycleSize( i ) ) )
   return( 1 );
  }

 // with one iteration only, the mean found is an upper bound
 if( howard.findCycleMean( 1 ) == HowardMmc< SmartDigraph >::NO_CYCLE )
  return( 1 );
 return( howard.cycleCost() * size[ 0 ] >= c[ 0 ] * howard.cycleSize() ? 0
									: 1 );
 }
