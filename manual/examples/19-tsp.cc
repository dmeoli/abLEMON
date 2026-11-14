// The five TSP heuristics of chapter 19 on twelve points of a circle,
// numbered out of their angular order: the optimal tour is the regular
// polygon, every tour must cost at least its perimeter, Christofides'
// tour at most 3/2 of it, and the 2-opt tour, which on points in convex
// position has no crossing edge, exactly the perimeter. Then on 40 random
// points of the unit square, where the heuristics differ, every tour must
// cost at least the minimum spanning tree, Christofides' tour at most twice
// it, and 2-opt must not worsen the tour it starts from.

#include <cmath>
#include <iostream>
#include <vector>
#include <lemon/full_graph.h>
#include <lemon/path.h>
#include <lemon/nearest_neighbor_tsp.h>
#include <lemon/greedy_tsp.h>
#include <lemon/insertion_tsp.h>
#include <lemon/christofides_tsp.h>
#include <lemon/opt2_tsp.h>
#include <lemon/kruskal.h>
#include <lemon/random.h>

using namespace lemon;

typedef FullGraph::EdgeMap< double > CostMap;

// whether the sequence visits every node once and its cost is c
bool checkTour( const FullGraph & g , const CostMap & cost ,
                const std::vector< FullGraph::Node > & tour , double c ) {
  if( int( tour.size() ) != g.nodeNum() ) return false;
  FullGraph::NodeMap< bool > seen( g , false );
  double sum = 0;
  for( std::size_t i = 0 ; i < tour.size() ; ++i ) {
    if( seen[ tour[ i ] ] ) return false;
    seen[ tour[ i ] ] = true;
    sum += cost[ g.edge( tour[ i ] , tour[ ( i + 1 ) % tour.size() ] ) ];
  }
  return std::fabs( sum - c ) < 1e-9;
}

int main() {
  const int n = 12;
  const double pi = 3.14159265358979323846;
  FullGraph g( n );
  std::vector< double > x( n ) , y( n );
  for( int i = 0 ; i < n ; ++i ) {      // node i sits at the position
    int pos = ( 5 * i ) % n;            // 5 i mod 12 of the polygon
    x[ i ] = std::cos( 2 * pi * pos / n );
    y[ i ] = std::sin( 2 * pi * pos / n );
  }
  CostMap cost( g );
  for( FullGraph::EdgeIt e( g ) ; e != INVALID ; ++e ) {
    int u = g.id( g.u( e ) ) , v = g.id( g.v( e ) );
    cost[ e ] = std::hypot( x[ u ] - x[ v ] , y[ u ] - y[ v ] );
  }
  const double opt = n * 2 * std::sin( pi / n );   // the perimeter
  bool ok = true;

  NearestNeighborTsp< CostMap > nn( g , cost );
  GreedyTsp< CostMap > gr( g , cost );
  InsertionTsp< CostMap > ins( g , cost );
  ChristofidesTsp< CostMap > chr( g , cost );
  double c_nn = nn.run() , c_gr = gr.run() ,
         c_ins = ins.run() , c_chr = chr.run();   // FARTHEST by default
  ok = ok && checkTour( g , cost , nn.tourNodes() , c_nn )
          && checkTour( g , cost , gr.tourNodes() , c_gr )
          && checkTour( g , cost , ins.tourNodes() , c_ins )
          && checkTour( g , cost , chr.tourNodes() , c_chr );
  ok = ok && c_nn >= opt - 1e-9 && c_gr >= opt - 1e-9
          && c_ins >= opt - 1e-9 && c_chr <= 1.5 * opt + 1e-9;

  // 2-opt from the order of the ids, and from the insertion tour
  Opt2Tsp< CostMap > opt2( g , cost );
  double c_id = opt2.run();
  ok = ok && checkTour( g , cost , opt2.tourNodes() , c_id )
          && std::fabs( c_id - opt ) < 1e-9;
  Path< FullGraph > p;
  ins.tour( p );                        // the tour as a path of n arcs
  double c_impr = opt2.run( p );
  ok = ok && c_impr <= c_ins + 1e-9 && std::fabs( c_impr - opt ) < 1e-9;

  std::cout << "optimum " << opt << "\nnearest neighbor " << c_nn
            << "\ngreedy " << c_gr << "\ninsertion " << c_ins
            << "\nChristofides " << c_chr << "\n2-opt " << c_id
            << "\n2-opt after insertion " << c_impr << std::endl;

  const int m = 40;
  FullGraph h( m );
  Random r( 19 );
  std::vector< double > px( m ) , py( m );
  for( int i = 0 ; i < m ; ++i ) { px[ i ] = r(); py[ i ] = r(); }
  CostMap d( h );
  for( FullGraph::EdgeIt e( h ) ; e != INVALID ; ++e ) {
    int u = h.id( h.u( e ) ) , v = h.id( h.v( e ) );
    d[ e ] = std::hypot( px[ u ] - px[ v ] , py[ u ] - py[ v ] );
  }
  FullGraph::EdgeMap< bool > tree( h );
  double mst = kruskal( h , d , tree );
  NearestNeighborTsp< CostMap > nn2( h , d );
  GreedyTsp< CostMap > gr2( h , d );
  ChristofidesTsp< CostMap > chr2( h , d );
  Opt2Tsp< CostMap > o2( h , d );
  std::cout << "\nrandom points: spanning tree " << mst;
  double c[ 7 ] = { nn2.run() , gr2.run() , 0 , 0 , 0 , 0 , chr2.run() };
  const char * rule[ 4 ] = { "nearest" , "farthest" , "cheapest" , "random" };
  for( int k = 0 ; k < 4 ; ++k ) {
    InsertionTsp< CostMap > ins2( h , d );
    c[ 2 + k ] = ins2.run( InsertionTsp< CostMap >::SelectionRule( k ) );
    ok = ok && checkTour( h , d , ins2.tourNodes() , c[ 2 + k ] );
    double impr = o2.run( ins2.tourNodes() );
    ok = ok && impr <= c[ 2 + k ] + 1e-9;
    std::cout << "\ninsertion (" << rule[ k ] << ") " << c[ 2 + k ]
              << ", after 2-opt " << impr;
  }
  for( int k = 0 ; k < 7 ; ++k ) ok = ok && c[ k ] >= mst - 1e-9;
  ok = ok && c[ 6 ] <= 2 * mst + 1e-9
          && checkTour( h , d , chr2.tourNodes() , c[ 6 ] );
  std::cout << "\nnearest neighbor " << c[ 0 ] << ", greedy " << c[ 1 ]
            << ", Christofides " << c[ 6 ] << std::endl;
  return ok ? 0 : 1;
}
