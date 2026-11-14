// Floating-point data (section 13.3): supplies of 0.1, 0.2 and -0.3,
// which do not sum to zero in binary, and fractional capacities and
// costs, solved by the four algorithms with the tolerances they derive
// from the data; the total costs must agree within the tolerance on the
// costs, and the tolerances in use are read back.

#include <cmath>
#include <iostream>
#include <limits>
#include <lemon/smart_graph.h>
#include <lemon/network_simplex.h>
#include <lemon/cost_scaling.h>
#include <lemon/capacity_scaling.h>
#include <lemon/cycle_canceling.h>

using namespace lemon;

typedef SmartDigraph::ArcMap< double > ArcMap;
typedef SmartDigraph::NodeMap< double > NodeMap;

int main()
{
 SmartDigraph g;
 SmartDigraph::Node u = g.addNode() , v = g.addNode() , w = g.addNode() ,
		    z = g.addNode();
 ArcMap upper( g ) , cost( g );
 NodeMap supply( g );
 SmartDigraph::Arc a;
 a = g.addArc( u , w );
 upper[ a ] = 0.25;
 cost[ a ] = 1.1;
 a = g.addArc( u , z );
 upper[ a ] = 0.15;
 cost[ a ] = 0.7;
 a = g.addArc( v , w );
 upper[ a ] = 0.3;
 cost[ a ] = 0.2;
 a = g.addArc( z , w );
 upper[ a ] = 0.1;
 cost[ a ] = 0.3;
 a = g.addArc( v , z );
 upper[ a ] = 0.2;
 cost[ a ] = 0.4;
 supply[ u ] = 0.1;
 supply[ v ] = 0.2;
 supply[ w ] = -0.3;
 supply[ z ] = 0.0;
 std::cout << "sum of the supplies: " << ( 0.1 + 0.2 - 0.3 ) << std::endl;

 typedef NetworkSimplex< SmartDigraph , double > NS;
 typedef CostScaling< SmartDigraph , double > CS;
 typedef CapacityScaling< SmartDigraph , double > CAP;
 typedef CycleCanceling< SmartDigraph , double > CC;
 NS ns( g );
 CS cs( g );
 CAP cap( g );
 CC cc( g );
 ns.upperMap( upper ).costMap( cost ).supplyMap( supply );
 cs.upperMap( upper ).costMap( cost ).supplyMap( supply );
 cap.upperMap( upper ).costMap( cost ).supplyMap( supply );
 cc.upperMap( upper ).costMap( cost ).supplyMap( supply );
 int errors = 0;
 if( ns.run() != NS::OPTIMAL || cs.run() != CS::OPTIMAL ||
     cap.run() != CAP::OPTIMAL || cc.run() != CC::OPTIMAL )
  return( 1 );

 // 0.1 along u, z, w at a cost of 1.0 and 0.2 along (v,w): 0.1 + 0.04
 const double best = 0.14;
 const double ceps = ns.costTolerance().epsilon();
 std::cout << "total costs: " << ns.totalCost() << ' ' << cs.totalCost() << ' '
	   << cap.totalCost() << ' ' << cc.totalCost() << std::endl;
 std::cout << "tolerances of NetworkSimplex: flows "
	   << ns.flowTolerance().epsilon() << ", costs " << ceps << std::endl;
 const double tc[] = { ns.totalCost() , cs.totalCost() , cap.totalCost() ,
		       cc.totalCost() };
 for( int i = 0 ; i != 4 ; ++i )
  if( std::abs( tc[ i ] - best ) > 1e-9 )
   ++errors;

 // The default tolerances are 10^6 units in the last place of the largest
 // datum, taken as 1 if it is smaller: 1 for the flows, whose data are
 // below 1, and the largest cost, 1.1, for the costs
 const double ulp = std::numeric_limits< double >::epsilon();
 if( ns.flowTolerance().epsilon() != 1.0 * ulp * 1e6 ||
     ceps != 1.1 * ulp * 1e6 ||
     cs.flowTolerance().epsilon() != 1.0 * ulp * 1e6 )
  ++errors;

 // Tolerances set by the caller are those in use
 ns.tolerances( 1e-12 , 1e-12 );
 if( ns.run() != NS::OPTIMAL || ns.flowTolerance().epsilon() != 1e-12 ||
     std::abs( ns.totalCost() - best ) > 1e-9 )
  ++errors;

 return( errors );
 }
