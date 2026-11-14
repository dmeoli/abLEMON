// The LGF reader and writer of chapter 21: a network with integer
// capacities, floating-point costs, a name for each node and the source
// and the target as attributes is written to a string and read back into
// another digraph. The costs are written once with the default converter,
// which prints 6 significant digits, and once with a converter that
// prints 17, which gives back the same doubles.

#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>
#include <lemon/smart_graph.h>
#include <lemon/lgf_reader.h>
#include <lemon/lgf_writer.h>

using namespace lemon;

typedef SmartDigraph Digraph;

// a converter from a double to a token with all its digits
struct Exact
{
 std::string operator()( double v ) const {
  char buf[ 32 ];
  std::snprintf( buf , sizeof( buf ) , "%.17g" , v );
  return( buf );
  }
 };

int main()
{
 Digraph g;
 Digraph::NodeMap< std::string > name( g );
 Digraph::ArcMap< int > cap( g );
 Digraph::ArcMap< double > cost( g );
 Digraph::Node v[ 4 ];
 const char * names[ 4 ] = { "s" , "a" , "b" , "the sink" };
 for( int i = 0 ; i < 4 ; ++i ) {
  v[ i ] = g.addNode();
  name[ v[ i ] ] = names[ i ];
  }
 int arcs[ 5 ][ 2 ] = { { 0 , 1 } , { 0 , 2 } , { 1 , 2 } , { 1 , 3 } ,
			{ 2 , 3 } };
 for( int k = 0 ; k < 5 ; ++k ) {
  Digraph::Arc e = g.addArc( v[ arcs[ k ][ 0 ] ] , v[ arcs[ k ][ 1 ] ] );
  cap[ e ] = 10 + k;
  cost[ e ] = 1.0 / ( k + 3 );  // 1/3, 1/4, ... not exact in 6 digits
  }
 std::string caption = "a small network";

 std::ostringstream plain , exact;
 digraphWriter( g , plain )
   .nodeMap( "name" , name )
   .arcMap( "capacity" , cap )
   .arcMap( "cost" , cost )
   .node( "source" , v[ 0 ] )
   .node( "target" , v[ 3 ] )
   .attribute( "caption" , caption )
   .run();
 digraphWriter( g , exact )
   .nodeMap( "name" , name )
   .arcMap( "capacity" , cap )
   .arcMap( "cost" , cost , Exact() )
   .node( "source" , v[ 0 ] )
   .node( "target" , v[ 3 ] )
   .attribute( "caption" , caption )
   .run();
 std::cout << exact.str();

 bool ok = true;
 for( int pass = 0 ; pass < 2 ; ++pass ) {
  Digraph h;
  Digraph::NodeMap< std::string > hname( h );
  Digraph::ArcMap< int > hcap( h );
  Digraph::ArcMap< double > hcost( h );
  Digraph::Node src , trg;
  std::string hcaption;
  std::istringstream is( pass == 0 ? plain.str() : exact.str() );
  digraphReader( h , is )
    .nodeMap( "name" , hname )
    .arcMap( "capacity" , hcap )
    .arcMap( "cost" , hcost )
    .node( "source" , src )
    .node( "target" , trg )
    .attribute( "caption" , hcaption )
    .run();
  ok = ok && countNodes( h ) == 4 && countArcs( h ) == 5 &&
       hname[ src ] == "s" && hname[ trg ] == "the sink" &&
       hcaption == caption;
  // the labels are the ids, so the arcs come back in the same order
  bool same = true;
  for( Digraph::ArcIt e( g ) ; e != INVALID ; ++e ) {
   Digraph::Arc f = h.arcFromId( g.id( e ) );
   ok = ok && hcap[ f ] == cap[ e ] &&
	hname[ h.source( f ) ] == name[ g.source( e ) ];
   same = same && hcost[ f ] == cost[ e ];
   }
  ok = ok && same == ( pass == 1 );  // 6 digits lose, 17 do not
  }

 // a missing map is reported as a FormatError with its line
 try {
  Digraph h;
  Digraph::ArcMap< int > w( h );
  std::istringstream is( plain.str() );
  digraphReader( h , is ).arcMap( "weight" , w ).run();
  ok = false;
  }
 catch( const FormatError & err ) {
  std::cout << "expected error: " << err.what() << std::endl;
  }
 return( ok ? 0 : 1 );
 }
