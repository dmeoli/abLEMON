// The maximum flow of section 12.2 and 12.3: the network of Figure 12.1,
// solved by Preflow and by EdmondsKarp, the minimum cut of both checked
// against the flow value, and a second Preflow started from the maximum
// flow of the first after the capacity of one arc has grown.

#include <iostream>
#include <lemon/list_graph.h>
#include <lemon/preflow.h>
#include <lemon/edmonds_karp.h>

using namespace lemon;

typedef ListDigraph::ArcMap< int > CapMap;

// The capacity of the arcs that leave the source side of a cut
template< typename CutMap >
int cutCapacity( const ListDigraph & g , const CapMap & cap ,
		 const CutMap & side )
{
 int c = 0;
 for( ListDigraph::ArcIt a( g ) ; a != INVALID ; ++a )
  if( side[ g.source( a ) ] && ! side[ g.target( a ) ] )
   c += cap[ a ];
 return( c );
 }

int main()
{
 ListDigraph g;
 ListDigraph::Node s = g.addNode() , a = g.addNode() , b = g.addNode() ,
		   c = g.addNode() , d = g.addNode() , t = g.addNode();
 CapMap cap( g );
 cap[ g.addArc( s , a ) ] = 10;
 cap[ g.addArc( s , b ) ] = 5;
 cap[ g.addArc( a , b ) ] = 4;
 cap[ g.addArc( a , c ) ] = 6;
 cap[ g.addArc( b , d ) ] = 9;
 cap[ g.addArc( c , d ) ] = 2;
 cap[ g.addArc( c , t ) ] = 8;
 ListDigraph::Arc dt = g.addArc( d , t );
 cap[ dt ] = 6;
 int errors = 0;

 // Preflow: both phases, then the cut of the first phase
 Preflow< ListDigraph > pf( g , cap , s , t );
 pf.run();
 ListDigraph::NodeMap< bool > cut( g );
 pf.minCutMap( cut );
 std::cout << "Preflow: value " << pf.flowValue() << ", cut capacity "
	   << cutCapacity( g , cap , cut ) << std::endl;
 if( pf.flowValue() != 12 || cutCapacity( g , cap , cut ) != 12 )
  ++errors;
 if( ! ( cut[ s ] && cut[ a ] && cut[ b ] && cut[ d ] && ! cut[ c ] &&
	 ! cut[ t ] ) )
  ++errors;

 // EdmondsKarp, one augmenting path at a time
 EdmondsKarp< ListDigraph > ek( g , cap , s , t );
 ek.init();
 int paths = 0;
 while( ek.augment() )
  ++paths;
 ListDigraph::NodeMap< bool > cut2( g );
 ek.minCutMap( cut2 );
 std::cout << "EdmondsKarp: value " << ek.flowValue() << " by " << paths
	   << " augmenting paths" << std::endl;
 if( ek.flowValue() != 12 || cutCapacity( g , cap , cut2 ) != 12 )
  ++errors;

 // A larger capacity of the arc (d,t): the old maximum flow is a feasible
 // flow of the new network, from which a second Preflow starts
 CapMap cap2( g );
 for( ListDigraph::ArcIt e( g ) ; e != INVALID ; ++e )
  cap2[ e ] = cap[ e ];
 cap2[ dt ] = 10;
 Preflow< ListDigraph > pf2( g , cap2 , s , t );
 if( ! pf2.init( pf.flowMap() ) )
  ++errors;
 pf2.startFirstPhase();
 std::cout << "Preflow from the old flow: value " << pf2.flowValue()
	   << std::endl;
 if( pf2.flowValue() != 15 )
  ++errors;

 // A map that is no flow of the network is refused by EdmondsKarp, which
 // checks the conservation and the bounds (the flow is set anyway)
 CapMap bad( g , 0 );
 bad[ dt ] = 1;
 if( ek.init( bad ) )
  ++errors;
 if( ! ek.init( pf.flowMap() ) || ek.flowValue() != 12 || ek.augment() )
  ++errors;

 return( errors );
 }
