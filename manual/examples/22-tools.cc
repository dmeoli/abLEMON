// What the tools of chapter 22 do on a minimum cost flow instance, in one
// program: the options of dimacs-solver parsed by ArgParser from a given
// command line, the DIMACS file read as dimacs-solver reads it, the type of
// the supply constraints chosen by the sign of the sum of the supplies,
// and the conversion to LGF that dimacs-to-lgf writes, read back.

#include <iostream>
#include <sstream>
#include <string>
#include <lemon/arg_parser.h>
#include <lemon/smart_graph.h>
#include <lemon/dimacs.h>
#include <lemon/lgf_reader.h>
#include <lemon/lgf_writer.h>
#include <lemon/network_simplex.h>

using namespace lemon;

typedef SmartDigraph Digraph;

int main()
{
 // the command line "dimacs-solver -q -double -infcap 100 net.min"
 const char * argv[] = { "dimacs-solver" , "-q" ,  "-double" ,
			 "-infcap" ,       "100" , "net.min" };
 ArgParser ap( 6 , argv );
 ap.throwOnProblems();  // an exception rather than exit(1)
 ap.other( "[INFILE [OUTFILE]]" )
   .boolOption( "q" , "Do not print any report" )
   .boolOption( "int" , "Use 'int' for capacities, costs etc. (default)" )
   .optionGroup( "datatype" , "int" )
   .boolOption( "double" , "Use 'double' for capacities, costs etc." )
   .optionGroup( "datatype" , "double" )
   .onlyOneGroup( "datatype" )
   .stringOption( "infcap" , "Value used for 'very high' capacities" , "0" )
   .run();
 bool ok = ap.given( "q" ) && ap.given( "double" ) && ! ap.given( "int" ) &&
	   ap.files().size() == 1 && ap.files()[ 0 ] == "net.min";
 double infty;
 std::istringstream( static_cast< std::string >( ap[ "infcap" ] ) ) >> infty;
 ok = ok && infty == 100;

 // a "p min" file: node 1 supplies 4, node 4 demands 3, so that the sum
 // is positive and the supplies are constraints of the type LEQ; the arc
 // 2 -> 4 has an upper bound below its lower bound, i.e., infinite
 std::istringstream min_file( "p min 4 5\n"
			      "n 1 4\n"
			      "n 4 -3\n"
			      "a 1 2 0 4 1\n"
			      "a 1 3 0 4 2\n"
			      "a 2 3 0 2 1\n"
			      "a 2 4 0 -1 3\n"
			      "a 3 4 1 5 1\n" );
 Digraph g;
 Digraph::ArcMap< double > lower( g ) , cap( g ) , cost( g );
 Digraph::NodeMap< double > sup( g );
 readDimacsMin( min_file , g , lower , cap , cost , sup , infty );
 double sum = 0;
 for( Digraph::NodeIt n( g ) ; n != INVALID ; ++n )
  sum += sup[ n ];
 NetworkSimplex< Digraph , double > ns( g );
 ns.lowerMap( lower ).upperMap( cap ).costMap( cost ).supplyMap( sup );
 if( sum > 0 )
  ns.supplyType( ns.LEQ );
 ok =
   ok && sum == 1 && cap[ g.arcFromId( 3 ) ] == 100 && ns.run() == ns.OPTIMAL;
 // the 3 units demanded go along 1 -> 3 -> 4 at cost 3 each (the path
 // 1 -> 2 -> 4 costs 4), and node 1 keeps the fourth unit
 ok = ok && ns.totalCost() == 9;

 // dimacs-to-lgf: the same maps under the same names, read back
 std::ostringstream lgf;
 digraphWriter( g , lgf )
   .nodeMap( "supply" , sup )
   .arcMap( "lower" , lower )
   .arcMap( "capacity" , cap )
   .arcMap( "cost" , cost )
   .attribute( "problem" , "min" )
   .run();
 Digraph h;
 Digraph::ArcMap< double > hcost( h );
 std::string problem;
 std::istringstream lgf_in( lgf.str() );
 digraphReader( h , lgf_in )
   .arcMap( "cost" , hcost )
   .attribute( "problem" , problem )
   .run();
 ok = ok && countArcs( h ) == 5 && problem == "min" &&
      hcost[ h.arcFromId( 3 ) ] == 3;

 std::cout << lgf.str() << "minimum cost " << ns.totalCost() << std::endl;
 return( ok ? 0 : 1 );
 }
