// The other formats of chapter 21: a maximum flow instance and a plain
// graph in DIMACS format, a graph in the graph6 format of nauty, and the
// EPS drawing of a digraph, all through strings.

#include <iostream>
#include <sstream>
#include <string>
#include <lemon/smart_graph.h>
#include <lemon/dimacs.h>
#include <lemon/nauty_reader.h>
#include <lemon/graph_to_eps.h>
#include <lemon/preflow.h>

using namespace lemon;

int main()
{
 bool ok = true;

 // a "p max" file: arc 2 -> 4 has a negative capacity, i.e., infinite
 std::istringstream max_file( "c a small maximum flow problem\n"
			      "p max 4 5\n"
			      "n 1 s\n"
			      "n 4 t\n"
			      "a 1 2 4\n"
			      "a 1 3 2\n"
			      "a 2 3 3\n"
			      "a 2 4 -1\n"
			      "a 3 4 5\n" );
 DimacsDescriptor desc = dimacsType( max_file );
 ok = ok && desc.type == DimacsDescriptor::MAX && desc.nodeNum == 4 &&
      desc.edgeNum == 5;
 SmartDigraph g;
 SmartDigraph::ArcMap< int > cap( g );
 SmartDigraph::Node s , t;
 readDimacsMax( max_file , g , cap , s , t , 1000 , desc );
 Preflow< SmartDigraph , SmartDigraph::ArcMap< int > > pre( g , cap , s , t );
 pre.run();
 ok = ok && countArcs( g ) == 5 && pre.flowValue() == 6 &&
      cap[ g.arcFromId( 3 ) ] == 1000;

 // a plain digraph written as "p mat" and read back as a graph
 std::ostringstream mat;
 writeDimacsMat( mat , g , "the arcs of the flow problem" );
 std::istringstream mat_in( mat.str() );
 SmartGraph u;
 readDimacsMat( mat_in , u );
 ok = ok && countNodes( u ) == 4 && countEdges( u ) == 5;

 // graph6: "D" is 5 nodes, "~{" the 10 bits of the upper triangle
 std::istringstream g6( "D~{\nC]\n" );
 SmartGraph k5 , c4;
 readNautyGraph( k5 , g6 );
 readNautyGraph( c4 , g6 );  // 4 nodes, bits 011110: a 4-cycle
 ok = ok && countNodes( k5 ) == 5 && countEdges( k5 ) == 10 &&
      countNodes( c4 ) == 4 && countEdges( c4 ) == 4;

 // the EPS drawing of the flow problem, the arc widths by capacity
 SmartDigraph::NodeMap< dim2::Point< double > > coords( g );
 double xy[ 4 ][ 2 ] = { { 0 , 0 } , { 2 , 1 } , { 2 , -1 } , { 4 , 0 } };
 for( int i = 0 ; i < 4 ; ++i )
  coords[ g.nodeFromId( i ) ] =
    dim2::Point< double >( xy[ i ][ 0 ] , xy[ i ][ 1 ] );
 std::ostringstream eps;
 graphToEps( g , eps )
   .coords( coords )
   .arcWidths( cap )
   .autoArcWidthScale()
   .arcWidthScale( 0.01 )
   .drawArrows()
   .title( "a small maximum flow problem" )
   .run();
 ok = ok && eps.str().compare( 0 , 23 , "%!PS-Adobe-2.0 EPSF-2.0" ) == 0 &&
      eps.str().find( "%%BoundingBox:" ) != std::string::npos &&
      eps.str().find( "showpage" ) != std::string::npos;

 std::cout << mat.str() << "max flow " << pre.flowValue() << ", EPS of "
	   << eps.str().size() << " bytes" << std::endl;
 return( ok ? 0 : 1 );
 }
