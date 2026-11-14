// The LP interface of chapter 20: the maximum flow problem written as an
// LP over the arcs of a digraph, one column per arc and one row per inner
// node, by a function that takes any LpSolver. It is built on LpSkeleton,
// which compiles with no solver and checks the model and the algebra of
// the expressions; when the library has an LP solver (LEMON_HAVE_LP), the
// same model is solved by the default one and its value compared with
// that of Preflow.

#include <cmath>
#include <iostream>
#include <lemon/config.h>
#include <lemon/list_graph.h>
#include <lemon/preflow.h>
#include <lemon/lp_skeleton.h>
#ifdef LEMON_HAVE_LP
#include <lemon/lp.h>
#endif

using namespace lemon;

typedef ListDigraph Digraph;
typedef Digraph::ArcMap< int > CapMap;

// the maximum flow LP: max sum of the flow out of s minus into s,
// conservation at every node but s and t, 0 <= x_a <= cap_a
void buildMaxFlow( LpSolver & lp , const Digraph & g , const CapMap & cap ,
		   Digraph::Node s , Digraph::Node t ,
		   Digraph::ArcMap< LpSolver::Col > & x )
{
 lp.addColSet( x );  // one column per arc
 for( Digraph::ArcIt a( g ) ; a != INVALID ; ++a )
  lp.colBounds( x[ a ] , 0 , cap[ a ] );
 for( Digraph::NodeIt v( g ) ; v != INVALID ; ++v ) {
  if( v == s || v == t )
   continue;
  LpSolver::Expr in , out;
  for( Digraph::InArcIt a( g , v ) ; a != INVALID ; ++a )
   in += x[ a ];
  for( Digraph::OutArcIt a( g , v ) ; a != INVALID ; ++a )
   out += x[ a ];
  lp.addRow( in == out );
  }
 LpSolver::Expr value;
 for( Digraph::OutArcIt a( g , s ) ; a != INVALID ; ++a )
  value += x[ a ];
 for( Digraph::InArcIt a( g , s ) ; a != INVALID ; ++a )
  value -= x[ a ];
 lp.obj( value );
 lp.max();
 }

int countCols( const LpBase & lp )
{
 int k = 0;
 for( LpBase::ColIt c( lp ) ; c != INVALID ; ++c )
  ++k;
 return( k );
 }

int countRows( const LpBase & lp )
{
 int k = 0;
 for( LpBase::RowIt r( lp ) ; r != INVALID ; ++r )
  ++k;
 return( k );
 }

int main()
{
 Digraph g;
 Digraph::Node s = g.addNode() , a = g.addNode() , b = g.addNode() ,
	       t = g.addNode();
 CapMap cap( g );
 cap[ g.addArc( s , a ) ] = 4;
 cap[ g.addArc( s , b ) ] = 2;
 cap[ g.addArc( a , b ) ] = 3;
 cap[ g.addArc( a , t ) ] = 1;
 cap[ g.addArc( b , t ) ] = 5;
 bool ok = true;

 // the model on the skeleton: two rows (a and b), five columns
 LpSkeleton skel;
 Digraph::ArcMap< LpSolver::Col > xs( g );
 buildMaxFlow( skel , g , cap , s , t , xs );
 ok = ok && countCols( skel ) == 5 && countRows( skel ) == 2;

 // the algebra of expressions and constraints, kept by LpBase itself
 LpBase::Col p = skel.addCol() , q = skel.addCol();
 LpBase::Expr e = 2 * p + 3 * ( q - p ) + 4;  // -p + 3 q + 4
 ok = ok && e[ p ] == -1 && e[ q ] == 3 && *e == 4;
 LpBase::Constr c = ( 1 <= e ) <= 10;  // a two-sided row
 ok = ok && c.lowerBound() == 1 && c.upperBound() == 10;
 LpBase::Constr d = e <= q;  // 0 <= q - e
 ok = ok && d.lowerBounded() && ! d.upperBounded() && d.expr()[ p ] == 1;
 e += 0.001 * q - 3 * q;
 e.simplify( 0.01 );  // drops the term of q
 ok = ok && e[ q ] == 0 && e[ p ] == -1;

 Preflow< Digraph , CapMap > pre( g , cap , s , t );
 pre.run();
 std::cout << "maximum flow by Preflow: " << pre.flowValue() << std::endl;

#ifdef LEMON_HAVE_LP
 Lp lp;  // the default LP solver
 Digraph::ArcMap< LpSolver::Col > x( g );
 buildMaxFlow( lp , g , cap , s , t , x );
 lp.solve();
 ok = ok && lp.primalType() == LpSolver::OPTIMAL &&
      std::fabs( lp.primal() - pre.flowValue() ) < 1e-6;
 std::cout << "maximum flow by " << lp.solverName() << ": " << lp.primal()
	   << std::endl;
#else
 std::cout << "no LP solver: the model was only built" << std::endl;
#endif
 return( ok ? 0 : 1 );
 }
