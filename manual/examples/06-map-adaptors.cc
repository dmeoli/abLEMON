// Map adaptors (section 6.4) and map utilities (section 6.7): the travel
// time of an arc as the quotient of two arc maps, passed to Dijkstra
// without being stored; a combination by a lambda, whose value type is
// deduced; and the utilities that scan a map.

#include <iostream>
#include <lemon/dijkstra.h>
#include <lemon/list_graph.h>
#include <lemon/maps.h>

using namespace lemon;

int main()
{
 typedef ListDigraph::ArcMap< double > DoubleArcMap;
 ListDigraph g;
 ListDigraph::Node s = g.addNode() , a = g.addNode() , b = g.addNode() ,
		   t = g.addNode();
 DoubleArcMap length( g ) , speed( g );
 ListDigraph::Arc sa = g.addArc( s , a ) , sb = g.addArc( s , b ) ,
		  at = g.addArc( a , t ) , bt = g.addArc( b , t );
 length[ sa ] = 10;
 speed[ sa ] = 5;  // 2 hours
 length[ sb ] = 6;
 speed[ sb ] = 1;  // 6 hours
 length[ at ] = 30;
 speed[ at ] = 10;  // 3 hours
 length[ bt ] = 2;
 speed[ bt ] = 2;  // 1 hour

 // the time of an arc, computed when Dijkstra reads it
 typedef DivMap< DoubleArcMap , DoubleArcMap > TimeMap;
 TimeMap time( length , speed );
 Dijkstra< ListDigraph , TimeMap > dijkstra( g , time );
 dijkstra.run( s , t );
 double fastest = dijkstra.dist( t );  // 5, through a

 // the same time plus a fixed delay of one hour per arc, by a lambda: the
 // value type of the CombineMap is the type that the lambda returns
 auto delayed = combineMap(
   length , speed , []( double l , double v ) { return( l / v + 1 ); } );
 Dijkstra< ListDigraph , decltype( delayed ) > dijkstra2( g , delayed );
 dijkstra2.run( s , t );
 double with_delay = dijkstra2.dist( t );  // 7, through a

 // the utilities: the slowest arc, the arcs of speed below 5, a copy
 ListDigraph::Arc slowest = mapMin( g , speed );
 int slow = mapCountIf( g , speed , []( double v ) { return( v < 5 ); } );
 DoubleArcMap copy( g );
 mapCopy( g , time , copy );
 bool same = mapCompare( g , copy , time );
 double longest = mapMaxValue( g , time );  // 6, the arc (s, b)

 std::cout << "fastest " << fastest << ", with delays " << with_delay
	   << ", slow arcs " << slow << ", longest time " << longest
	   << std::endl;
 return( ( fastest == 5 && with_delay == 7 && slowest == sb && slow == 2 &&
	    same && longest == 6 )
	    ? 0
	    : 1 );
 }
