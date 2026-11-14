// The certificates of chapter 14: an infeasible problem, for which each of
// the four algorithms gives sets of nodes whose violation (14.1) is
// positive, and an unbounded one, for which each gives cycles of negative
// cost made of arcs of infinite capacity; every certificate is checked
// here on the data, not taken on trust.

#include <iostream>
#include <set>
#include <vector>
#include <lemon/list_graph.h>
#include <lemon/network_simplex.h>
#include <lemon/cost_scaling.h>
#include <lemon/capacity_scaling.h>
#include <lemon/cycle_canceling.h>

using namespace lemon;

typedef ListDigraph::Node Node;
typedef ListDigraph::Arc Arc;
typedef ListDigraph::ArcMap< int > ArcMap;
typedef ListDigraph::NodeMap< int > NodeMap;

// The violation (14.1) of a set of nodes under GEQ, with no lower bounds:
// its supply less the capacity of the arcs that leave it
int violation( const ListDigraph & g , const ArcMap & upper ,
               const NodeMap & supply , const std::vector< Node > & set ) {
  ListDigraph::NodeMap< bool > in( g , false );
  int v = 0;
  for( std::size_t i = 0 ; i != set.size() ; ++i ) {
    in[ set[ i ] ] = true;
    v += supply[ set[ i ] ];
  }
  for( ListDigraph::ArcIt a( g ) ; a != INVALID ; ++a )
    if( in[ g.source( a ) ] && ! in[ g.target( a ) ] )
      v -= upper[ a ];
  return( v );
}

template< typename MCF >
int checkInfeasible( const char * name , const ListDigraph & g ,
                     const ArcMap & upper , const ArcMap & cost ,
                     const NodeMap & supply ) {
  MCF mcf( g );
  mcf.upperMap( upper ).costMap( cost ).supplyMap( supply );
  if( mcf.run() != MCF::INFEASIBLE )
    return( 1 );
  int errors = 0;
  const std::vector< Node > cut = mcf.infeasibleCut();
  std::cout << name << ": a cut of violation "
            << violation( g , upper , supply , cut );
  if( cut.empty() || violation( g , upper , supply , cut ) <= 0 )
    ++errors;
  // the k most violated sets, in order and distinct
  const std::vector< std::vector< Node > > best = mcf.infeasibleCuts( 4 );
  std::set< std::set< int > > seen;
  std::cout << ", the best " << best.size() << ":";
  for( std::size_t j = 0 ; j != best.size() ; ++j ) {
    const int v = violation( g , upper , supply , best[ j ] );
    std::cout << ' ' << v;
    if( v <= 0 || ( j > 0 && v > violation( g , upper , supply ,
                                            best[ j - 1 ] ) ) )
      ++errors;
    std::set< int > ids;
    for( std::size_t h = 0 ; h != best[ j ].size() ; ++h )
      ids.insert( g.id( best[ j ][ h ] ) );
    if( ! seen.insert( ids ).second )
      ++errors;
  }
  // disjoint violated sets, with no arc between two of them
  const std::vector< std::vector< Node > > dis =
    mcf.disjointInfeasibleCuts();
  ListDigraph::NodeMap< int > owner( g , -1 );
  for( std::size_t j = 0 ; j != dis.size() ; ++j ) {
    if( violation( g , upper , supply , dis[ j ] ) <= 0 )
      ++errors;
    for( std::size_t h = 0 ; h != dis[ j ].size() ; ++h ) {
      if( owner[ dis[ j ][ h ] ] >= 0 )
        ++errors;
      owner[ dis[ j ][ h ] ] = int( j );
    }
  }
  for( ListDigraph::ArcIt a( g ) ; a != INVALID ; ++a )
    if( owner[ g.source( a ) ] >= 0 && owner[ g.target( a ) ] >= 0 &&
        owner[ g.source( a ) ] != owner[ g.target( a ) ] )
      ++errors;
  std::cout << ", " << dis.size() << " disjoint" << std::endl;
  return( errors );
}

// Whether the arcs make a directed cycle of negative cost, each of
// infinite upper bound
bool negativeCycle( const ListDigraph & g , const ArcMap & cost ,
                    const std::vector< bool > & inf ,
                    const std::vector< Arc > & cycle ) {
  if( cycle.empty() )
    return( false );
  int c = 0;
  for( std::size_t h = 0 ; h != cycle.size() ; ++h ) {
    const Arc next = cycle[ ( h + 1 ) % cycle.size() ];
    if( g.target( cycle[ h ] ) != g.source( next ) ||
        ! inf[ g.id( cycle[ h ] ) ] )
      return( false );
    c += cost[ cycle[ h ] ];
  }
  return( c < 0 );
}

template< typename MCF >
int checkUnbounded( const char * name , const ListDigraph & g ,
                    const ArcMap & upper , const ArcMap & cost ,
                    const std::vector< bool > & inf ) {
  MCF mcf( g );
  ArcMap up( g );
  for( ListDigraph::ArcIt a( g ) ; a != INVALID ; ++a )
    up[ a ] = inf[ g.id( a ) ] ? mcf.INF : upper[ a ];
  mcf.upperMap( up ).costMap( cost );
  if( mcf.run() != MCF::UNBOUNDED )
    return( 1 );
  int errors = 0;
  if( ! negativeCycle( g , cost , inf , mcf.unboundedCycle() ) )
    ++errors;
  const std::vector< std::vector< Arc > > cycles = mcf.unboundedCycles( 3 );
  std::set< int > used;
  for( std::size_t j = 0 ; j != cycles.size() ; ++j ) {
    if( ! negativeCycle( g , cost , inf , cycles[ j ] ) )
      ++errors;
    for( std::size_t h = 0 ; h != cycles[ j ].size() ; ++h )
      if( ! used.insert( g.id( cycles[ j ][ h ] ) ).second )
        ++errors;
  }
  std::cout << name << ": " << cycles.size()
            << " arc-disjoint negative cycles" << std::endl;
  if( cycles.size() != 2 )
    ++errors;
  return( errors );
}

int main() {
  int errors = 0;
  {
    // Infeasible: a sends 4 units on an arc of capacity 2, d sends 3 on
    // one of capacity 1, and b, with no supply, can only feed a
    ListDigraph g;
    Node a = g.addNode() , b = g.addNode() , c = g.addNode() ,
         d = g.addNode() , e = g.addNode();
    ArcMap upper( g ) , cost( g , 1 );
    NodeMap supply( g );
    upper[ g.addArc( a , c ) ] = 2;
    upper[ g.addArc( d , e ) ] = 1;
    upper[ g.addArc( b , a ) ] = 5;
    upper[ g.addArc( c , e ) ] = 9;
    supply[ a ] = 4; supply[ b ] = 0; supply[ c ] = -4;
    supply[ d ] = 3; supply[ e ] = -3;
    errors += checkInfeasible< NetworkSimplex< ListDigraph > >(
      "NetworkSimplex" , g , upper , cost , supply );
    errors += checkInfeasible< CostScaling< ListDigraph > >(
      "CostScaling" , g , upper , cost , supply );
    errors += checkInfeasible< CapacityScaling< ListDigraph > >(
      "CapacityScaling" , g , upper , cost , supply );
    errors += checkInfeasible< CycleCanceling< ListDigraph > >(
      "CycleCanceling" , g , upper , cost , supply );
  }
  {
    // Unbounded: the cycles p, q, r (cost -1) and r, s (cost -2) of
    // infinite capacity, and an arc (q,p) of cost -10 but finite capacity
    ListDigraph g;
    Node p = g.addNode() , q = g.addNode() , r = g.addNode() ,
         s = g.addNode();
    ArcMap upper( g , 0 ) , cost( g );
    std::vector< bool > inf;
    struct { Node u , v; int cost; bool inf; } arcs[] = {
      { p , q , -2 , true } , { q , r , 1 , true } , { r , p , 0 , true } ,
      { r , s , -3 , true } , { s , r , 1 , true } , { q , p , -10 , false } };
    for( int i = 0 ; i != 6 ; ++i ) {
      Arc x = g.addArc( arcs[ i ].u , arcs[ i ].v );
      cost[ x ] = arcs[ i ].cost;
      upper[ x ] = 5;
      inf.push_back( arcs[ i ].inf );
    }
    errors += checkUnbounded< NetworkSimplex< ListDigraph > >(
      "NetworkSimplex" , g , upper , cost , inf );
    errors += checkUnbounded< CostScaling< ListDigraph > >(
      "CostScaling" , g , upper , cost , inf );
    errors += checkUnbounded< CapacityScaling< ListDigraph > >(
      "CapacityScaling" , g , upper , cost , inf );
    errors += checkUnbounded< CycleCanceling< ListDigraph > >(
      "CycleCanceling" , g , upper , cost , inf );
  }
  return( errors );
}
