// The minimum cost flow of section 13.2: a small transshipment problem
// solved by the four algorithms, whose total costs must agree and whose
// potentials must meet the optimality conditions (13.4) with the flow of
// each; then the same problem with the LEQ supply type and more supply
// than demand, which only NetworkSimplex takes.

#include <iostream>
#include <lemon/list_graph.h>
#include <lemon/network_simplex.h>
#include <lemon/cost_scaling.h>
#include <lemon/capacity_scaling.h>
#include <lemon/cycle_canceling.h>

using namespace lemon;

typedef ListDigraph::ArcMap< int > ArcMap;
typedef ListDigraph::NodeMap< int > NodeMap;

// Whether flow and potentials of an algorithm meet the complementary
// slackness conditions (13.4) of the GEQ form, and the flow the bounds
// and the supplies (as equalities, since the supplies sum to zero)
template< typename MCF >
bool optimal( const ListDigraph & g , const MCF & mcf , const ArcMap & upper ,
              const ArcMap & cost , const NodeMap & supply ) {
  for( ListDigraph::ArcIt a( g ) ; a != INVALID ; ++a ) {
    const int f = mcf.flow( a );
    const int rc = cost[ a ] + mcf.potential( g.source( a ) ) -
                   mcf.potential( g.target( a ) );
    if( f < 0 || f > upper[ a ] )
      return( false );
    if( ( rc > 0 && f != 0 ) || ( rc < 0 && f != upper[ a ] ) )
      return( false );
  }
  for( ListDigraph::NodeIt n( g ) ; n != INVALID ; ++n ) {
    int net = 0;
    for( ListDigraph::OutArcIt a( g , n ) ; a != INVALID ; ++a )
      net += mcf.flow( a );
    for( ListDigraph::InArcIt a( g , n ) ; a != INVALID ; ++a )
      net -= mcf.flow( a );
    if( net != supply[ n ] || mcf.potential( n ) > 0 )
      return( false );
  }
  return( true );
}

int main() {
  ListDigraph g;
  ListDigraph::Node s1 = g.addNode() , s2 = g.addNode() , m1 = g.addNode() ,
                    m2 = g.addNode() , t1 = g.addNode() , t2 = g.addNode() ,
                    t3 = g.addNode();
  ArcMap upper( g ) , cost( g );
  NodeMap supply( g );
  struct { ListDigraph::Node u , v; int cap , cost; } arcs[] = {
    { s1 , m1 , 4 , 2 } , { s1 , m2 , 3 , 4 } , { s2 , m1 , 3 , 3 } ,
    { s2 , m2 , 4 , 1 } , { m1 , m2 , 2 , 1 } , { m1 , t1 , 3 , 2 } ,
    { m1 , t2 , 3 , 5 } , { m2 , t2 , 4 , 2 } , { m2 , t3 , 3 , 3 } ,
    { s1 , t1 , 1 , 3 } };
  ListDigraph::Arc id[ 10 ];
  for( int i = 0 ; i != 10 ; ++i ) {
    id[ i ] = g.addArc( arcs[ i ].u , arcs[ i ].v );
    upper[ id[ i ] ] = arcs[ i ].cap;
    cost[ id[ i ] ] = arcs[ i ].cost;
  }
  supply[ s1 ] = 5; supply[ s2 ] = 4; supply[ m1 ] = 0; supply[ m2 ] = 0;
  supply[ t1 ] = -3; supply[ t2 ] = -3; supply[ t3 ] = -3;
  int errors = 0;

  NetworkSimplex< ListDigraph > ns( g );
  ns.upperMap( upper ).costMap( cost ).supplyMap( supply );
  CostScaling< ListDigraph > cs( g );
  cs.upperMap( upper ).costMap( cost ).supplyMap( supply );
  CapacityScaling< ListDigraph > cap( g );
  cap.upperMap( upper ).costMap( cost ).supplyMap( supply );
  CycleCanceling< ListDigraph > cc( g );
  cc.upperMap( upper ).costMap( cost ).supplyMap( supply );

  if( ns.run() != NetworkSimplex< ListDigraph >::OPTIMAL ||
      cs.run() != CostScaling< ListDigraph >::OPTIMAL ||
      cap.run() != CapacityScaling< ListDigraph >::OPTIMAL ||
      cc.run() != CycleCanceling< ListDigraph >::OPTIMAL )
    return( 1 );
  std::cout << "total costs: " << ns.totalCost() << ' ' << cs.totalCost()
            << ' ' << cap.totalCost() << ' ' << cc.totalCost() << std::endl;
  if( ns.totalCost() != 36 || cs.totalCost() != 36 ||
      cap.totalCost() != 36 || cc.totalCost() != 36 )
    ++errors;
  if( ! optimal( g , ns , upper , cost , supply ) ||
      ! optimal( g , cs , upper , cost , supply ) ||
      ! optimal( g , cap , upper , cost , supply ) ||
      ! optimal( g , cc , upper , cost , supply ) )
    ++errors;

  // A second run after a change of one cost, the other parameters being
  // kept: (m2,t3) is the only arc into t3, whose 3 units cost 10 more each
  const ListDigraph::Arc first = id[ 8 ];
  cost[ first ] += 10;
  ns.costMap( cost );
  if( ns.run() != NetworkSimplex< ListDigraph >::OPTIMAL ||
      ! optimal( g , ns , upper , cost , supply ) || ns.totalCost() != 66 )
    ++errors;
  std::cout << "after the change of a cost: " << ns.totalCost() << std::endl;
  cost[ first ] -= 10;

  // More supply than demand: under LEQ the demands are met exactly and
  // some supply may stay where it is
  supply[ s2 ] = 6;
  ns.costMap( cost ).supplyMap( supply ).supplyType(
    NetworkSimplex< ListDigraph >::LEQ );
  if( ns.run() != NetworkSimplex< ListDigraph >::OPTIMAL )
    ++errors;
  std::cout << "LEQ with two more units at s2: " << ns.totalCost()
            << std::endl;
  for( ListDigraph::NodeIt n( g ) ; n != INVALID ; ++n )
    if( ns.potential( n ) < 0 )
      ++errors;
  // under GEQ the same data are infeasible
  ns.supplyType( NetworkSimplex< ListDigraph >::GEQ );
  if( ns.run() != NetworkSimplex< ListDigraph >::INFEASIBLE )
    ++errors;

  return( errors );
}
