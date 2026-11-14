// The minimum cut of an undirected graph by NagamochiIbaraki (section 15.3)
// and the minimum cuts of all the pairs of nodes by the Gomory-Hu tree
// (section 15.4), each checked against the cuts of all the subsets of the
// nodes and against a maximum flow.

#include <iostream>
#include <vector>
#include <lemon/smart_graph.h>
#include <lemon/nagamochi_ibaraki.h>
#include <lemon/gomory_hu.h>
#include <lemon/preflow.h>

using namespace lemon;

typedef SmartGraph::EdgeMap< int > CapMap;

// the capacity of the edges between the set coded by mask and the rest
int cutCapacity( const SmartGraph & g , const CapMap & cap , unsigned mask ) {
  int sum = 0;
  for( SmartGraph::EdgeIt e( g ) ; e != INVALID ; ++e )
    if( ( mask >> g.id( g.u( e ) ) & 1 ) != ( mask >> g.id( g.v( e ) ) & 1 ) )
      sum += cap[ e ];
  return sum;
}

int main() {
  const int n = 6;
  SmartGraph g;
  std::vector< SmartGraph::Node > v;
  for( int i = 0 ; i < n ; ++i )
    v.push_back( g.addNode() );
  CapMap cap( g );
  const int edges[][ 3 ] = { { 0 , 1 , 3 } , { 0 , 2 , 2 } , { 1 , 2 , 4 } ,
                             { 1 , 3 , 1 } , { 2 , 4 , 2 } , { 3 , 4 , 3 } ,
                             { 3 , 5 , 4 } , { 4 , 5 , 2 } };
  for( const auto & e : edges )
    cap[ g.addEdge( v[ e[ 0 ] ] , v[ e[ 1 ] ] ) ] = e[ 2 ];

  // the global minimum cut by enumeration
  const unsigned all = ( 1u << n ) - 1;
  int best = 1 << 30;
  for( unsigned mask = 1 ; mask < all ; ++mask ) {
    int c = cutCapacity( g , cap , mask );
    if( c < best ) best = c;
  }

  NagamochiIbaraki< SmartGraph > ni( g , cap );
  ni.init();
  int phases = 0;
  while( ! ni.processNextPhase() )  // or ni.start(), or ni.run()
    ++phases;
  SmartGraph::NodeMap< bool > cut( g );
  int value = ni.minCutMap( cut );
  unsigned mask = 0;
  for( SmartGraph::NodeIt u( g ) ; u != INVALID ; ++u )
    if( cut[ u ] ) mask |= 1u << g.id( u );
  std::cout << "minimum cut: " << value << " after " << phases + 1
            << " phases" << std::endl;
  if( value != best || cutCapacity( g , cap , mask ) != value ) return 1;

  // the Gomory-Hu tree: its edges, then the cut of every pair of nodes
  GomoryHu< SmartGraph > gh( g , cap );
  gh.run();
  for( SmartGraph::NodeIt u( g ) ; u != INVALID ; ++u )
    if( gh.predNode( u ) != INVALID )
      std::cout << "tree edge " << g.id( u ) << " - "
                << g.id( gh.predNode( u ) ) << ", weight "
                << gh.predValue( u ) << std::endl;
  for( int s = 0 ; s < n ; ++s )
    for( int t = s + 1 ; t < n ; ++t ) {
      Preflow< SmartGraph , CapMap > pf( g , cap , v[ s ] , v[ t ] );
      pf.runMinCut();
      int lambda = gh.minCutValue( v[ s ] , v[ t ] );
      gh.minCutMap( v[ s ] , v[ t ] , cut );
      int sum = 0;
      for( GomoryHu< SmartGraph >::MinCutEdgeIt e( gh , v[ s ] , v[ t ] ) ;
           e != INVALID ; ++e )
        sum += cap[ e ];
      if( lambda != pf.flowValue() || sum != lambda || ! cut[ v[ s ] ] ||
          cut[ v[ t ] ] )
        return 1;
    }
  return 0;
}
