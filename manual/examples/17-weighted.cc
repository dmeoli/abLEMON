// Weighted matchings of a general graph (sections 17.5 and 17.6): a
// triangle of weight 5 with a pendant edge at two of its nodes. The
// maximum weight matching is checked against its dual solution (node
// potentials and blossom values, scaled by dualScale); the graph has no
// perfect matching, while the triangle alone has a perfect fractional one,
// whose value exceeds that of every matching.

#include <iostream>
#include <lemon/list_graph.h>
#include <lemon/matching.h>
#include <lemon/fractional_matching.h>

using namespace lemon;

typedef ListGraph Graph;
typedef MaxWeightedMatching< Graph > MWM;

// y_u + y_v + sum of z_B over the blossoms B containing u and v - w_uv,
// all scaled by dualScale: nonnegative, and zero on the matching
int reducedWeight( const Graph & g , const Graph::EdgeMap< int > & w ,
		   const MWM & mwm , Graph::Edge e )
{
 int rw = mwm.nodeValue( g.u( e ) ) + mwm.nodeValue( g.v( e ) ) -
	  MWM::dualScale * w[ e ];
 for( int k = 0 ; k < mwm.blossomNum() ; ++k ) {
  bool su = false , sv = false;
  for( MWM::BlossomIt n( mwm , k ) ; n != INVALID ; ++n ) {
   if( Graph::Node( n ) == g.u( e ) )
    su = true;
   if( Graph::Node( n ) == g.v( e ) )
    sv = true;
   }
  if( su && sv )
   rw += mwm.blossomValue( k );
  }
 return( rw );
 }

int main()
{
 Graph g;
 Graph::Node a = g.addNode() , b = g.addNode() , c = g.addNode() ,
	     y = g.addNode() , z = g.addNode();
 Graph::EdgeMap< int > w( g );
 w[ g.addEdge( a , b ) ] = 5;
 w[ g.addEdge( b , c ) ] = 5;
 w[ g.addEdge( c , a ) ] = 5;
 w[ g.addEdge( a , y ) ] = 1;
 w[ g.addEdge( b , z ) ] = 2;

 MWM mwm( g , w );
 mwm.run();
 std::cout << "maximum weight matching: " << mwm.matchingWeight() << " ("
	   << mwm.matchingSize() << " edges), dual value " << mwm.dualValue()
	   << " / " << MWM::dualScale << ", " << mwm.blossomNum()
	   << " blossom(s)" << std::endl;
 if( mwm.matchingWeight() != 7 )
  return( 1 );
 if( mwm.dualValue() != MWM::dualScale * mwm.matchingWeight() )
  return( 1 );
 for( Graph::EdgeIt e( g ) ; e != INVALID ; ++e ) {
  int rw = reducedWeight( g , w , mwm , e );
  if( rw < 0 || ( mwm.matching( e ) && rw != 0 ) )
   return( 1 );
  }

 // five nodes: no perfect matching
 MaxWeightedPerfectMatching< Graph > mwpm( g , w );
 if( mwpm.run() )
  return( 1 );

 // the maximum fractional matching covers four nodes (value 2)
 MaxFractionalMatching< Graph > mfm( g );
 mfm.run();
 std::cout << "covered nodes in a maximum fractional matching: "
	   << mfm.matchingSize() << std::endl;
 if( mfm.matchingSize() != 4 )
  return( 1 );
 // the barrier S proves it: n - |covered| = |S| - |N(S)|
 Graph::NodeMap< bool > adj( g , false );
 int s_size = 0 , n_size = 0;
 for( Graph::NodeIt n( g ) ; n != INVALID ; ++n )
  if( mfm.barrier( n ) ) {
   ++s_size;
   for( Graph::IncEdgeIt e( g , n ) ; e != INVALID ; ++e )
    adj[ g.oppositeNode( n , e ) ] = true;
   }
 for( Graph::NodeIt n( g ) ; n != INVALID ; ++n )
  if( adj[ n ] )
   ++n_size;
 std::cout << "barrier of " << s_size << " nodes with " << n_size
	   << " neighbours" << std::endl;
 if( countNodes( g ) - mfm.matchingSize() != s_size - n_size )
  return( 1 );

 // the triangle alone: value 15/2 by halves, 5 by any matching
 Graph t;
 Graph::Node u0 = t.addNode() , u1 = t.addNode() , u2 = t.addNode();
 Graph::EdgeMap< int > tw( t );
 tw[ t.addEdge( u0 , u1 ) ] = 5;
 tw[ t.addEdge( u1 , u2 ) ] = 5;
 tw[ t.addEdge( u2 , u0 ) ] = 5;
 typedef MaxWeightedPerfectFractionalMatching< Graph > MWPFM;
 MWPFM frac( t , tw );
 if( ! frac.run() )
  return( 1 );
 std::cout << "perfect fractional matching of the triangle: "
	   << frac.matchingWeight() << " / " << MWPFM::primalScale
	   << ", dual value " << frac.dualValue() << " / " << MWPFM::dualScale
	   << std::endl;
 for( Graph::EdgeIt e( t ) ; e != INVALID ; ++e )
  if( frac.matching( e ) != 1 )
   return( 1 );  // x_e = 1/2, scaled by 2
 MaxWeightedPerfectMatching< Graph > integral( t , tw );
 if( integral.run() )
  return( 1 );
 return( frac.matchingWeight() == 15 &&
	      frac.dualValue() * MWPFM::primalScale ==
		MWPFM::dualScale * frac.matchingWeight()
	    ? 0
	    : 1 );
 }
