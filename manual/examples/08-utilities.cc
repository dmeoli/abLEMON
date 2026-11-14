// The auxiliary tools of sections 8.3 to 8.9: an elevator of labels, the
// two radix sorts, the random number generator, points and boxes, a
// counter, a timer and a parser of the command line.

#include <iostream>
#include <sstream>
#include <vector>
#include <lemon/list_graph.h>
#include <lemon/elevator.h>
#include <lemon/radix_sort.h>
#include <lemon/random.h>
#include <lemon/dim2.h>
#include <lemon/counter.h>
#include <lemon/time_measure.h>
#include <lemon/arg_parser.h>

using namespace lemon;

int key( int x )
{
 return( x % 10 );
 }  // the last decimal digit

int main()
{
 int bad = 0;

 // Elevator: four nodes on levels 0, 0, 1 and 2 (out of 0 .. 4)
 ListDigraph g;
 ListDigraph::Node n[ 4 ];
 for( int i = 0 ; i < 4 ; ++i )
  n[ i ] = g.addNode();
 Elevator< ListDigraph , ListDigraph::Node > lev( g , 4 );
 lev.initStart();
 lev.initAddItem( n[ 0 ] );
 lev.initAddItem( n[ 1 ] );
 lev.initNewLevel();
 lev.initAddItem( n[ 2 ] );
 lev.initNewLevel();
 lev.initAddItem( n[ 3 ] );
 lev.initFinish();
 lev.activate( n[ 1 ] );
 lev.activate( n[ 2 ] );
 if( lev.highestActive() != n[ 2 ] || lev.highestActiveLevel() != 1 ||
     lev.onLevel( 0 ) != 2 || lev.activesOnLevel( 0 ) != 1 )
  ++bad;
 lev.liftHighestActive( 3 );  // n2 from level 1 to level 3
 if( lev[ n[ 2 ] ] != 3 || ! lev.emptyLevel( 1 ) || lev.aboveLevel( 0 ) != 2 )
  ++bad;

 // radix sorts: by value, and stably by the last digit
 std::vector< int > x = { 42 , -7 , 15 , 0 , 32 , -12 , 15 , 5 };
 std::vector< int > y = x;
 radixSort( x.begin() , x.end() );
 for( std::size_t i = 1 ; i < x.size() ; ++i )
  if( x[ i - 1 ] > x[ i ] )
   ++bad;
 std::vector< int > z = { 42 , 15 , 32 , 5 , 12 };
 stableRadixSort( z.begin() , z.end() , key );
 if( z != std::vector< int >( { 42 , 32 , 12 , 15 , 5 } ) )
  ++bad;

 // random numbers: one seed, one sequence
 Random32 r1( 2026 ) , r2( 2026 );
 for( int i = 0 ; i < 100 ; ++i ) {
  int a = r1[ 6 ] + 1 , b = r2.integer( 1 , 7 );
  double u = r1();
  r2();
  if( a != b || a < 1 || a > 6 || u < 0 || u >= 1 )
   ++bad;
  }
 Random32 saved( r1 );  // the state is copied
 if( saved.integer< int >() != r1.integer< int >() )
  ++bad;

 // points and boxes
 dim2::Box< double > box;
 box.add( dim2::Point< double >( 1 , 2 ) );
 box.add( dim2::Point< double >( -1 , 5 ) );
 if( box.width() != 2 || box.height() != 3 ||
     ! box.inside( dim2::Point< double >( 0 , 3 ) ) ||
     box.inside( dim2::Point< double >( 2 , 3 ) ) )
  ++bad;
 dim2::Point< int > p( 3 , 4 );
 if( p.normSquare() != 25 || rot90( p ) != dim2::Point< int >( -4 , 3 ) )
  ++bad;

 // a counter that prints its value into a string at its destruction
 std::ostringstream report;
 {
  Counter ops( "operations: " , report );
  Counter::NoSubCounter cmp( ops );
  for( int i = 0 ; i < 10 ; ++i )
   ++cmp;
  ops += 5;
  }
 if( report.str() != "operations: 15\n" )
  ++bad;

 // a timer stopped and restarted, whose times do not decrease
 Timer t;
 double first = t.realTime();
 t.stop();
 t.start();
 if( t.realTime() < first || t.running() != 1 )
  ++bad;

 // a command line parsed, with an option of each kind
 const char * argv[] = {
  "prog" , "-n" , "7" , "-eps" , "1e-3" , "-v" , "input.lgf"
  };
 ArgParser ap( 7 , argv );
 int nodes = 0;
 ap.refOption( "n" , "number of nodes" , nodes )
   .doubleOption( "eps" , "tolerance" , 1e-6 )
   .boolOption( "v" , "verbose" )
   .other( "file" , "the input" );
 ap.throwOnProblems();
 ap.parse();
 double eps = ap[ "eps" ];
 bool verbose = ap[ "v" ];
 if( nodes != 7 || eps != 1e-3 || ! verbose || ap.files().size() != 1 ||
     ap.files()[ 0 ] != "input.lgf" || ! ap.given( "n" ) )
  ++bad;

 std::cout << bad << " errors" << std::endl;
 return( bad == 0 ? 0 : 1 );
 }
