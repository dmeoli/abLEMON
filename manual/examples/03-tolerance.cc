// Tolerance (section 3.7): the exact comparisons of the general template,
// the absolute epsilon of the floating-point specializations, and the
// counting functions of core.h that read the tags of a graph.

#include <iostream>
#include <lemon/core.h>
#include <lemon/list_graph.h>
#include <lemon/smart_graph.h>
#include <lemon/tolerance.h>

using namespace lemon;

int main()
{
 int errors = 0;

 // the general template compares exactly, with static functions
 if( Tolerance< int >::less( 2 , 2 ) || ! Tolerance< int >::positive( 1 ) )
  ++errors;

 // double: epsilon 1e-10 by default, an absolute threshold
 Tolerance< double > tol;
 if( tol.epsilon() != 1e-10 )
  ++errors;
 double x = 0.1 + 0.2;  // 0.30000000000000004 in binary
 if( x == 0.3 )
  ++errors;
 if( tol.different( x , 0.3 ) || tol.nonZero( x - 0.3 ) )
  ++errors;
 if( ! tol.less( 0.3 , 0.3 + 1e-9 ) )
  ++errors;
 // the threshold does not scale with the operands
 if( ! tol.different( 1e20 , 1e20 + 1e5 ) )
  ++errors;  // relative gap 1e-15
 if( tol.different( 1e-12 , 2e-11 ) )
  ++errors;  // one is 20 times the other

 // a looser epsilon for one object, the default for the others
 Tolerance< double > loose( 1e-3 );
 if( ! loose.negative( -0.01 ) || loose.negative( -0.0001 ) )
  ++errors;
 if( Tolerance< float >::defaultEpsilon() != 1e-4f )
  ++errors;
 // the default of long double is the double 1e-14, widened
 if( Tolerance< long double >::defaultEpsilon() !=
     static_cast< long double >( 1e-14 ) )
  ++errors;

 // countNodes() is O(1) on SmartDigraph (it has NodeNumTag) and a scan
 // on ListDigraph; both give the same number
 SmartDigraph sg;
 ListDigraph lg;
 for( int i = 0 ; i < 5 ; ++i ) {
  sg.addNode();
  lg.addNode();
  }
 if( countNodes( sg ) != 5 || countNodes( lg ) != 5 )
  ++errors;
 if( sg.nodeNum() != 5 )
  ++errors;

 std::cout << "tolerances: " << errors << " errors" << std::endl;
 return( errors == 0 ? 0 : 1 );
 }
