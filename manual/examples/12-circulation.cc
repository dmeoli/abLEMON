// The feasible circulation of section 12.4: a network with lower and
// upper bounds and supplies, solved by Circulation, and the same network
// with two capacities lowered, for which Circulation gives a barrier whose
// inequality (12.6) is checked here on the data.

#include <iostream>
#include <lemon/list_graph.h>
#include <lemon/circulation.h>

using namespace lemon;

typedef ListDigraph::ArcMap< int > ArcMap;
typedef ListDigraph::NodeMap< int > NodeMap;

int main()
{
 ListDigraph g;
 ListDigraph::Node a = g.addNode() , b = g.addNode() , c = g.addNode() ,
		   d = g.addNode();
 ArcMap lower( g ) , upper( g );
 NodeMap supply( g );
 ListDigraph::Arc ab = g.addArc( a , b ) , ac = g.addArc( a , c ) ,
		  bc = g.addArc( b , c ) , bd = g.addArc( b , d ) ,
		  cd = g.addArc( c , d );
 lower[ ab ] = 1;
 upper[ ab ] = 3;
 lower[ ac ] = 0;
 upper[ ac ] = 2;
 lower[ bc ] = 0;
 upper[ bc ] = 2;
 lower[ bd ] = 0;
 upper[ bd ] = 4;
 lower[ cd ] = 0;
 upper[ cd ] = 1;
 supply[ a ] = 4;
 supply[ b ] = 0;
 supply[ c ] = -1;
 supply[ d ] = -3;
 int errors = 0;

 Circulation< ListDigraph > circ( g , lower , upper , supply );
 bool feasible = circ.run();
 std::cout << "first network: " << ( feasible ? "feasible" : "infeasible" )
	   << std::endl;
 if( ! feasible || ! circ.checkFlow() )
  ++errors;
 for( ListDigraph::ArcIt e( g ) ; e != INVALID ; ++e )
  std::cout << "  flow on arc " << g.id( e ) << ": " << circ.flow( e )
	    << std::endl;

 // d can receive at most 2 units, but it needs 3
 upper[ bd ] = 1;
 Circulation< ListDigraph > circ2( g , lower , upper , supply );
 feasible = circ2.run();
 ListDigraph::NodeMap< bool > bar( g );
 circ2.barrierMap( bar );
 // (12.6): what can leave the barrier is less than its supply
 int out = 0 , sup = 0;
 for( ListDigraph::NodeIt n( g ) ; n != INVALID ; ++n )
  if( bar[ n ] )
   sup += supply[ n ];
 for( ListDigraph::ArcIt e( g ) ; e != INVALID ; ++e ) {
  if( bar[ g.source( e ) ] && ! bar[ g.target( e ) ] )
   out += upper[ e ];
  else if( ! bar[ g.source( e ) ] && bar[ g.target( e ) ] )
   out -= lower[ e ];
  }
 std::cout << "second network: " << ( feasible ? "feasible" : "infeasible" )
	   << ", barrier of supply " << sup << " and outflow at most " << out
	   << std::endl;
 if( feasible || ! circ2.checkBarrier() || ! ( out < sup ) )
  ++errors;

 return( errors );
 }
