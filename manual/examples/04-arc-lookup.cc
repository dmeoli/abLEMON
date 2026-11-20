// Finding the arcs between two nodes (section 4.8): findArc() and ConArcIt,
// which scan the arcs out of the source, and the three look-up structures,
// on a digraph with parallel arcs; ArcLookUp and AllArcLookUp are refreshed
// after a change, DynArcLookUp follows it by itself.

#include <iostream>
#include <lemon/core.h>
#include <lemon/smart_graph.h>

using namespace lemon;

int main()
{
 int errors = 0;
 SmartDigraph g;
 const int n = 6;
 SmartDigraph::Node v[ n ];
 for( int i = 0 ; i < n ; ++i )
  v[ i ] = g.addNode();
 for( int i = 0 ; i < n ; ++i )
  for( int j = 0 ; j < n ; ++j )
   if( i != j && ( i + j ) % 3 != 0 )
    g.addArc( v[ i ] , v[ j ] );
 g.addArc( v[ 0 ] , v[ 1 ] );  // two more arcs parallel to v0 -> v1
 g.addArc( v[ 0 ] , v[ 1 ] );

 ArcLookUp< SmartDigraph > one( g );
 AllArcLookUp< SmartDigraph > all( g );
 DynArcLookUp< SmartDigraph > dyn( g );

 // every pair: the four ways agree on whether an arc exists
 for( int i = 0 ; i < n ; ++i )
  for( int j = 0 ; j < n ; ++j ) {
   SmartDigraph::Arc a = findArc( g , v[ i ] , v[ j ] );
   bool exists = ( i != j && ( i + j ) % 3 != 0 );
   if( ( a != INVALID ) != exists )
    ++errors;
   if( ( one( v[ i ] , v[ j ] ) != INVALID ) != exists )
    ++errors;
   if( ( all( v[ i ] , v[ j ] ) != INVALID ) != exists )
    ++errors;
   if( ( dyn( v[ i ] , v[ j ] ) != INVALID ) != exists )
    ++errors;
   if( one( v[ i ] , v[ j ] ) != INVALID &&
       g.target( one( v[ i ] , v[ j ] ) ) != v[ j ] )
    ++errors;
   }

 // all the parallel arcs: three from v0 to v1
 int by_find = 0 , by_con = 0 , by_all = 0 , by_dyn = 0;
 for( SmartDigraph::Arc a = findArc( g , v[ 0 ] , v[ 1 ] ) ; a != INVALID ;
      a = findArc( g , v[ 0 ] , v[ 1 ] , a ) )
  ++by_find;
 for( ConArcIt< SmartDigraph > a( g , v[ 0 ] , v[ 1 ] ) ; a != INVALID ; ++a )
  ++by_con;
 for( SmartDigraph::Arc a = all( v[ 0 ] , v[ 1 ] ) ; a != INVALID ;
      a = all( v[ 0 ] , v[ 1 ] , a ) )
  ++by_all;
 for( SmartDigraph::Arc a = dyn( v[ 0 ] , v[ 1 ] ) ; a != INVALID ;
      a = dyn( v[ 0 ] , v[ 1 ] , a ) )
  ++by_dyn;
 if( by_find != 3 || by_con != 3 || by_all != 3 || by_dyn != 3 )
  ++errors;

 // a new arc: DynArcLookUp sees it at once, ArcLookUp after refresh() of
 // its source, AllArcLookUp after refresh() of all the nodes
 SmartDigraph::Arc a03 = g.addArc( v[ 0 ] , v[ 3 ] );
 if( dyn( v[ 0 ] , v[ 3 ] ) != a03 )
  ++errors;
 one.refresh( v[ 0 ] );
 if( one( v[ 0 ] , v[ 3 ] ) != a03 )
  ++errors;
 all.refresh();
 if( all( v[ 0 ] , v[ 3 ] ) != a03 )
  ++errors;

 std::cout << "arc look-up: " << errors << " errors" << std::endl;
 return( errors == 0 ? 0 : 1 );
 }
