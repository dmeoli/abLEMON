// The minimum cut of a digraph by HaoOrlin (section 15.2): the two phases
// run one at a time from a chosen node, then together by run(), and each
// value is checked against the cuts of all the subsets of the nodes.

#include <iostream>
#include <vector>
#include <lemon/smart_graph.h>
#include <lemon/hao_orlin.h>

using namespace lemon;

typedef SmartDigraph::ArcMap< int > CapMap;

// the capacity of the arcs that leave the set of nodes coded by mask
int outCapacity( const SmartDigraph & g , const CapMap & cap , unsigned mask )
{
 int sum = 0;
 for( SmartDigraph::ArcIt a( g ) ; a != INVALID ; ++a )
  if( ( mask >> g.id( g.source( a ) ) & 1 ) &&
      ! ( mask >> g.id( g.target( a ) ) & 1 ) )
   sum += cap[ a ];
 return( sum );
 }

int main()
{
 SmartDigraph g;
 std::vector< SmartDigraph::Node > v;
 for( int i = 0 ; i < 6 ; ++i )
  v.push_back( g.addNode() );
 CapMap cap( g );
 const int arcs[][ 3 ] = { { 0 , 1 , 6 } , { 1 , 2 , 4 } , { 2 , 0 , 6 } ,
			   { 1 , 3 , 5 } , { 4 , 2 , 2 } , { 3 , 4 , 6 } ,
			   { 4 , 5 , 6 } , { 5 , 3 , 5 } , { 3 , 1 , 2 } };
 for( const auto & a : arcs )
  cap[ g.addArc( v[ a[ 0 ] ] , v[ a[ 1 ] ] ) ] = a[ 2 ];

 // the minimum over the sets that contain node 0, and over all the sets
 const unsigned all = ( 1u << 6 ) - 1;
 int best_out = 1 << 30 , best = 1 << 30;
 for( unsigned mask = 1 ; mask < all ; ++mask ) {
  int c = outCapacity( g , cap , mask );
  if( c < best )
   best = c;
  if( ( mask & 1 ) && c < best_out )
   best_out = c;
  }

 HaoOrlin< SmartDigraph > ho( g , cap );
 SmartDigraph::NodeMap< bool > cut( g );

 ho.init( v[ 0 ] );
 ho.calculateOut();  // the sets that contain node 0
 ho.minCutMap( cut );
 int out = ho.minCutValue();
 std::cout << "with node 0 inside: " << out << std::endl;
 if( out != best_out || ! cut[ v[ 0 ] ] )
  return( 1 );

 ho.calculateIn();  // the sets that do not contain it
 int value = ho.minCutMap( cut );
 std::cout << "minimum cut: " << value << ", X =";
 unsigned mask = 0;
 for( SmartDigraph::NodeIt n( g ) ; n != INVALID ; ++n )
  if( cut[ n ] )
   mask |= 1u << g.id( n );
 for( int i = 0 ; i < 6 ; ++i )
  if( mask >> i & 1 )
   std::cout << ' ' << i;
 std::cout << std::endl;
 if( value != best || outCapacity( g , cap , mask ) != value )
  return( 1 );
 if( mask == 0 || mask == all )
  return( 1 );

 ho.run();  // init(), calculateOut() and calculateIn() from the first node
 return( ho.minCutValue() == best ? 0 : 1 );
 }
