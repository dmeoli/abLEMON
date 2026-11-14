// The residual digraph (section 5.4): a maximum flow found by augmenting
// along the shortest paths of ResidualDigraph (Algorithm 5.1), checked
// against Preflow, and the minimum cut read from the last search.

#include <iostream>
#include <limits>
#include <lemon/adaptors.h>
#include <lemon/bfs.h>
#include <lemon/list_graph.h>
#include <lemon/path.h>
#include <lemon/preflow.h>

using namespace lemon;

int main()
{
 typedef ListDigraph::ArcMap< int > IntArcMap;
 ListDigraph g;
 ListDigraph::Node s = g.addNode() , a = g.addNode() , b = g.addNode() ,
		   c = g.addNode() , t = g.addNode();
 IntArcMap cap( g ) , flow( g , 0 );
 cap[ g.addArc( s , a ) ] = 4;
 cap[ g.addArc( s , b ) ] = 3;
 cap[ g.addArc( a , b ) ] = 2;
 cap[ g.addArc( a , c ) ] = 2;
 cap[ g.addArc( b , c ) ] = 4;
 cap[ g.addArc( b , t ) ] = 2;
 cap[ g.addArc( c , t ) ] = 5;

 typedef ResidualDigraph< const ListDigraph , IntArcMap > Residual;
 Residual res( g , cap , flow );
 Residual::ResidualCapacity rcap = res.residualCapacity();

 int value = 0 , augmentations = 0;
 Bfs< Residual > bfs( res );
 for( ; ; ) {
  bfs.run( s , t );  // a shortest augmenting path, if any
  if( ! bfs.reached( t ) )
   break;
  Path< Residual > p = bfs.path( t );
  int delta = std::numeric_limits< int >::max();
  for( Path< Residual >::ArcIt e( p ) ; e != INVALID ; ++e )
   if( rcap[ e ] < delta )
    delta = rcap[ e ];
  for( Path< Residual >::ArcIt e( p ) ; e != INVALID ; ++e )
   res.augment( e , delta );
  value += delta;
  ++augmentations;
  }

 // the nodes reached by the last search are the source side of a minimum
 // cut, whose capacity equals the value of the flow
 int cut = 0;
 for( ListDigraph::ArcIt e( g ) ; e != INVALID ; ++e )
  if( bfs.reached( g.source( e ) ) && ! bfs.reached( g.target( e ) ) )
   cut += cap[ e ];

 Preflow< ListDigraph , IntArcMap > preflow( g , cap , s , t );
 preflow.run();

 std::cout << "flow value " << value << " after " << augmentations
	   << " augmentations, cut " << cut << ", Preflow "
	   << preflow.flowValue() << std::endl;
 return( ( value == 7 && cut == 7 && preflow.flowValue() == 7 ) ? 0 : 1 );
 }
