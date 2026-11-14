// The minimum cost arborescence of section 11.2 on the digraph of Figure
// 11.2: the cycle a -> b -> a of cheap arcs is contracted, and the dual
// solution read from the algorithm proves the arborescence optimal by the
// conditions (11.5).

#include <iostream>
#include <set>
#include <vector>
#include <lemon/list_graph.h>
#include <lemon/min_cost_arborescence.h>

using namespace lemon;

typedef ListDigraph::Node Node;
typedef ListDigraph::Arc Arc;
typedef ListDigraph::ArcMap< int > CostMap;

int main() {
  ListDigraph g;
  Node r = g.addNode() , a = g.addNode() , b = g.addNode() ,
       c = g.addNode() , d = g.addNode();
  CostMap cost( g );
  const struct { Node u , v; int w; } arcs[] = {
    { r , a , 5 } , { r , b , 6 } , { a , b , 1 } , { b , a , 2 } ,
    { b , c , 3 } , { c , d , 2 } , { a , d , 7 } , { d , c , 1 } ,
    { r , d , 9 } };
  for( const auto & e : arcs )
    cost[ g.addArc( e.u , e.v ) ] = e.w;
  int bad = 0;

  typedef MinCostArborescence< ListDigraph , CostMap > MCA;
  MCA mca( g , cost );
  mca.run( r );
  // r -> a, a -> b, b -> c, c -> d: 5 + 1 + 3 + 2
  if( mca.arborescenceCost() != 11 || mca.pred( b ) == INVALID ||
      g.source( mca.pred( b ) ) != a || mca.pred( r ) != INVALID )
    ++bad;

  // the dual: a value y_X >= 0 on each set X of nodes, the root outside;
  // an arc pays the y_X of the sets it enters, at most its cost, exactly
  // its cost if it is in the arborescence, and the y_X sum to its cost
  std::vector< std::set< Node > > sets( mca.dualNum() );
  for( int k = 0 ; k < mca.dualNum() ; ++k ) {
    for( MCA::DualIt it( mca , k ) ; it != INVALID ; ++it )
      sets[ k ].insert( it );
    if( mca.dualValue( k ) < 0 || sets[ k ].count( r ) ||
        int( sets[ k ].size() ) != mca.dualSize( k ) )
      ++bad;
  }
  for( ListDigraph::ArcIt e( g ) ; e != INVALID ; ++e ) {
    int paid = 0;
    for( int k = 0 ; k < mca.dualNum() ; ++k )
      if( sets[ k ].count( g.target( e ) ) &&
          ! sets[ k ].count( g.source( e ) ) )
        paid += mca.dualValue( k );
    if( paid > cost[ e ] || ( mca.arborescence( e ) && paid != cost[ e ] ) )
      ++bad;
  }
  if( mca.dualValue() != mca.arborescenceCost() )
    ++bad;

  // the function-type interface
  ListDigraph::ArcMap< bool > arb( g );
  if( minCostArborescence( g , cost , r , arb ) != 11 )
    ++bad;

  std::cout << "arborescence of cost " << mca.arborescenceCost() << ", "
            << mca.dualNum() << " dual variables; " << bad << " errors"
            << std::endl;
  return( bad == 0 ? 0 : 1 );
}
