// Suurballe's algorithm of section 10.5 on the digraph of Figure 10.1: the
// shortest path s -> a -> b -> t blocks any second arc-disjoint path, and
// the two paths of least total length do not use a -> b; the flow and the
// potentials satisfy the conditions (10.9).

#include <iostream>
#include <lemon/list_graph.h>
#include <lemon/suurballe.h>

using namespace lemon;

typedef ListDigraph::Node Node;
typedef ListDigraph::Arc Arc;
typedef ListDigraph::ArcMap< int > LengthMap;

int main() {
  ListDigraph g;
  Node s = g.addNode() , a = g.addNode() , b = g.addNode() , t = g.addNode();
  LengthMap len( g );
  len[ g.addArc( s , a ) ] = 1;
  Arc ab = g.addArc( a , b );
  len[ ab ] = 1;
  len[ g.addArc( b , t ) ] = 1;
  len[ g.addArc( s , b ) ] = 2;
  len[ g.addArc( a , t ) ] = 2;
  int bad = 0;

  Suurballe< ListDigraph , LengthMap > suurballe( g , len );
  if( suurballe.run( s , t ) != 2 )  // k = 2 by default
    ++bad;
  if( suurballe.totalLength() != 6 || suurballe.flow( ab ) != 0 )
    ++bad;

  // the two paths share no arc and each goes from s to t
  ListDigraph::ArcMap< int > used( g , 0 );
  for( int i = 0 ; i < suurballe.pathNum() ; ++i ) {
    const Suurballe< ListDigraph , LengthMap >::Path & p = suurballe.path( i );
    if( ! checkPath( g , p ) || pathSource( g , p ) != s ||
        pathTarget( g , p ) != t )
      ++bad;
    for( Suurballe< ListDigraph , LengthMap >::Path::ArcIt e( p ) ;
         e != INVALID ; ++e )
      ++used[ e ];
  }
  for( ListDigraph::ArcIt e( g ) ; e != INVALID ; ++e )
    if( used[ e ] > 1 || used[ e ] != suurballe.flow( e ) )
      ++bad;

  // complementary slackness: reduced length >= 0 on the arcs without flow,
  // <= 0 on those with flow
  for( ListDigraph::ArcIt e( g ) ; e != INVALID ; ++e ) {
    int red = len[ e ] + suurballe.potential( g.source( e ) ) -
              suurballe.potential( g.target( e ) );
    if( ( suurballe.flow( e ) == 0 && red < 0 ) ||
        ( suurballe.flow( e ) == 1 && red > 0 ) )
      ++bad;
  }

  // a third path does not exist: two are returned
  if( suurballe.run( s , t , 3 ) != 2 )
    ++bad;

  std::cout << suurballe.pathNum() << " paths of total length "
            << suurballe.totalLength() << "; " << bad << " errors"
            << std::endl;
  return( bad == 0 ? 0 : 1 );
}
