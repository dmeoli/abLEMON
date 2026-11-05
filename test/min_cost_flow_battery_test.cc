/* -*- mode: C++; indent-tabs-mode: nil; -*-
 *
 * This file is a part of LEMON, a generic C++ optimization library.
 *
 * Copyright (C) 2026 the authors of abLEMON.
 *
 * Permission to use, modify and distribute this software is granted
 * provided that this copyright notice appears in all copies. For
 * precise terms see the accompanying LICENSE file.
 *
 * This software is provided "AS IS" with no warranty of any kind,
 * express or implied, and with no claim as to its suitability for any
 * purpose.
 *
 */

// The battery of the minimum cost flow algorithms: every algorithm, with
// each of its methods, on the limit cases and on pseudo-random instances,
// for an integer and a floating-point value type. Each answer is checked
// by its own certificate (an optimal flow by its feasibility, by the
// complementary slackness of its potentials and by a dual cost equal to
// the primal one, an infeasible problem by its cut, an unbounded one by
// its cycle), and all the answers on an instance have to agree on the
// result and on the optimal cost. The first argument, "long" or "double",
// selects the value type, and without one both run; a second one keeps
// only the solvers whose name contains it (and the first solver, against
// which they are compared).

#include <cmath>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <algorithm>
#include <limits>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <lemon/smart_graph.h>
#include <lemon/network_simplex.h>
#include <lemon/capacity_scaling.h>
#include <lemon/cost_scaling.h>
#include <lemon/cycle_canceling.h>


using namespace lemon;

// Every failure is printed, and the test fails at the end if there was
// any, so that one run shows all of them
int failures = 0;

// Whether each run is printed before it starts, as the environment
// variable LEMON_BATTERY_VERBOSE asks
#ifdef _MSC_VER
 #pragma warning( disable : 4996 )  // getenv(), which is only read here
#endif
const bool verbose = std::getenv( "LEMON_BATTERY_VERBOSE" ) != nullptr;

void expect( bool ok , const std::string & what )
{
 if( ! ok ) {
  ++failures;
  std::cerr << what << std::endl;
  }
 }

typedef SmartDigraph Digraph;
typedef Digraph::Node Node;
typedef Digraph::Arc Arc;

enum Result { INFEASIBLE , OPTIMAL , UNBOUNDED };

const char * resultName( int r )
{
 return( r == INFEASIBLE ? "INFEASIBLE"
			 : r == OPTIMAL ? "OPTIMAL" : "UNBOUNDED" );
 }

// An instance: a digraph with lower and upper bounds (the latter possibly
// infinite), costs and supplies of the GEQ type
template< typename V >
struct Instance
{
 std::string name;
 Digraph g;
 Digraph::ArcMap< V > lower , upper , cost;
 Digraph::NodeMap< V > supply;
 V inf;

 explicit Instance( const std::string & n )
     : name( n ) , lower( g , 0 ) , upper( g , 0 ) , cost( g , 0 ) ,
       supply( g , 0 ) ,
       inf( std::numeric_limits< V >::has_infinity
	      ? std::numeric_limits< V >::infinity()
	      : ( std::numeric_limits< V >::max )() ) {}

 Node node( V s = 0 ) {
  const Node n = g.addNode();
  supply[ n ] = s;
  return( n );
  }

 Arc arc( Node u , Node v , V c , V up , V low = 0 ) {
  const Arc a = g.addArc( u , v );
  cost[ a ] = c;
  upper[ a ] = up;
  lower[ a ] = low;
  return( a );
  }
 };

// What an algorithm answers
template< typename V >
struct Answer
{
 int result;
 double cost;
 std::vector< V > flow;
 std::vector< V > pi;
 std::vector< Node > cut;
 std::vector< Arc > cycle;
 std::vector< std::vector< Node > > cuts , disjoint;
 std::vector< std::vector< Arc > > cycles;
 };

template< typename V >
struct Solver
{
 std::string name;
 std::function< Answer< V >( const Instance< V > & ) > run;
 };

template< typename MCF , typename V , typename PT >
Answer< V > answerOf( const MCF & mcf , PT pt , const Instance< V > & p )
{
 Answer< V > a;
 a.result = pt == MCF::INFEASIBLE ? INFEASIBLE
	    : pt == MCF::OPTIMAL ? OPTIMAL : UNBOUNDED;
 a.cost = 0;
 if( a.result == OPTIMAL ) {
  a.cost = mcf.template totalCost< double >();
  // indexed by the ids, which are 0, 1, ... in a SmartDigraph
  a.flow.resize( countArcs( p.g ) );
  a.pi.resize( countNodes( p.g ) );
  for( Digraph::ArcIt e( p.g ) ; e != INVALID ; ++e )
   a.flow[ p.g.id( e ) ] = mcf.flow( e );
  for( Digraph::NodeIt n( p.g ) ; n != INVALID ; ++n )
   a.pi[ p.g.id( n ) ] = mcf.potential( n );
  }
 a.cut = mcf.infeasibleCut();
 a.cycle = mcf.unboundedCycle();
 a.cuts = mcf.infeasibleCuts( 5 );
 a.disjoint = mcf.disjointInfeasibleCuts();
 a.cycles = mcf.unboundedCycles( 5 );
 return( a );
 }

template< typename V >
std::vector< Solver< V > > solvers()
{
 typedef NetworkSimplex< Digraph , V , V > NS;
 typedef CostScaling< Digraph , V , V > COS;
 typedef CycleCanceling< Digraph , V , V > CC;
 typedef CapacityScaling< Digraph , V , V > CAS;
 std::vector< Solver< V > > s;
 const char * nsn[] = { "FIRST_ELIGIBLE" , "BEST_ELIGIBLE" , "BLOCK_SEARCH" ,
			"CANDIDATE_LIST" , "ALTERING_LIST" };
 const typename NS::PivotRule nsr[] = { NS::FIRST_ELIGIBLE ,
					NS::BEST_ELIGIBLE , NS::BLOCK_SEARCH ,
					NS::CANDIDATE_LIST ,
					NS::ALTERING_LIST };
 for( int k = 0 ; k != 5 ; ++k ) {
  const typename NS::PivotRule r = nsr[ k ];
  s.push_back( { std::string( "NetworkSimplex " ) + nsn[ k ] ,
		 [ r ]( const Instance< V > & p ) {
		  NS mcf( p.g );
		  mcf.lowerMap( p.lower ).upperMap( p.upper ).costMap(
		    p.cost ).supplyMap( p.supply );
		  return( answerOf( mcf , mcf.run( r ) , p ) );
		  } } );
  }
 const char * csn[] = { "PUSH" , "AUGMENT" , "PARTIAL_AUGMENT" };
 const typename COS::Method csm[] = { COS::PUSH , COS::AUGMENT ,
				      COS::PARTIAL_AUGMENT };
 for( int k = 0 ; k != 3 ; ++k ) {
  const typename COS::Method m = csm[ k ];
  s.push_back( { std::string( "CostScaling " ) + csn[ k ] ,
		 [ m ]( const Instance< V > & p ) {
		  COS mcf( p.g );
		  mcf.lowerMap( p.lower ).upperMap( p.upper ).costMap(
		    p.cost ).supplyMap( p.supply );
		  return( answerOf( mcf , mcf.run( m ) , p ) );
		  } } );
  }
 const char * ccn[] = { "SIMPLE_CYCLE_CANCELING" ,
			"MINIMUM_MEAN_CYCLE_CANCELING" , "CANCEL_AND_TIGHTEN" };
 const typename CC::Method ccm[] = { CC::SIMPLE_CYCLE_CANCELING ,
				     CC::MINIMUM_MEAN_CYCLE_CANCELING ,
				     CC::CANCEL_AND_TIGHTEN };
 for( int k = 0 ; k != 3 ; ++k ) {
  const typename CC::Method m = ccm[ k ];
  s.push_back( { std::string( "CycleCanceling " ) + ccn[ k ] ,
		 [ m ]( const Instance< V > & p ) {
		  CC mcf( p.g );
		  mcf.lowerMap( p.lower ).upperMap( p.upper ).costMap(
		    p.cost ).supplyMap( p.supply );
		  return( answerOf( mcf , mcf.run( m ) , p ) );
		  } } );
  }
 const int factors[] = { 1 , 4 };
 for( int k = 0 ; k != 2 ; ++k ) {
  const int f = factors[ k ];
  s.push_back( { "CapacityScaling factor " + std::to_string( f ) ,
		 [ f ]( const Instance< V > & p ) {
		  CAS mcf( p.g );
		  mcf.lowerMap( p.lower ).upperMap( p.upper ).costMap(
		    p.cost ).supplyMap( p.supply );
		  return( answerOf( mcf , mcf.run( f ) , p ) );
		  } } );
  }
 return( s );
 }

// The tolerance of a comparison between quantities whose absolute values
// are at most scale: none for an integer type
template< typename V >
double tolerance( double scale )
{
 return( std::numeric_limits< V >::is_integer ? 0 : 1e-9 * ( 1 + scale ) );
 }

// The largest absolute value of the finite data of an instance
template< typename V >
double scaleOf( const Instance< V > & p )
{
 double s = 1;
 for( Digraph::ArcIt a( p.g ) ; a != INVALID ; ++a ) {
  s = std::max( s , std::abs( double( p.cost[ a ] ) ) );
  s = std::max( s , std::abs( double( p.lower[ a ] ) ) );
  if( p.upper[ a ] != p.inf )
   s = std::max( s , std::abs( double( p.upper[ a ] ) ) );
  }
 for( Digraph::NodeIt n( p.g ) ; n != INVALID ; ++n )
  s = std::max( s , std::abs( double( p.supply[ n ] ) ) );
 return( s );
 }

// An optimal answer: a flow within the bounds that meets the supplies,
// potentials that are nonpositive, zero where the supply is exceeded,
// and in complementary slackness with it, and a dual cost equal to the
// primal one
template< typename V >
void checkOptimal( const Instance< V > & p , const Answer< V > & a ,
		   const std::string & id )
{
 const double s = scaleOf( p );
 const int n = countNodes( p.g );
 const double tf = tolerance< V >( s * n ) ,
	      tc = tolerance< V >( s * n * n );
 std::vector< double > net( n , 0 );
 double primal = 0 , dual = 0;
 for( Digraph::ArcIt e( p.g ) ; e != INVALID ; ++e ) {
  const double f = a.flow[ p.g.id( e ) ];
  expect( f >= p.lower[ e ] - tf && ( p.upper[ e ] == p.inf ||
				     f <= p.upper[ e ] + tf ) ,
	 "Flow out of its bounds: " + id );
  net[ p.g.id( p.g.source( e ) ) ] += f;
  net[ p.g.id( p.g.target( e ) ) ] -= f;
  primal += double( p.cost[ e ] ) * f;
  const double rc = double( p.cost[ e ] ) +
		    a.pi[ p.g.id( p.g.source( e ) ) ] -
		    a.pi[ p.g.id( p.g.target( e ) ) ];
  if( rc > tc )
   expect( f <= p.lower[ e ] + tf , "Slackness at a positive reduced cost: " +
				     id );
  if( rc < -tc ) {
   expect( p.upper[ e ] != p.inf && f >= p.upper[ e ] - tf ,
	  "Slackness at a negative reduced cost: " + id );
   dual += double( p.upper[ e ] ) * rc;
   }
  dual += double( p.lower[ e ] ) * std::max( rc , 0.0 );
  }
 for( Digraph::NodeIt v( p.g ) ; v != INVALID ; ++v ) {
  const int u = p.g.id( v );
  expect( net[ u ] >= p.supply[ v ] - tf , "Supply not met: " + id );
  expect( a.pi[ u ] <= tc , "Positive potential: " + id );
  if( net[ u ] > p.supply[ v ] + tf )
   expect( a.pi[ u ] >= -tc , "Slackness of an exceeded supply: " + id );
  dual -= double( p.supply[ v ] ) * a.pi[ u ];
  }
 // The dual cost of the GEQ problem: - sum sup pi + sum low max( rc , 0 )
 // + sum up min( rc , 0 ), with rc the reduced costs
 const double t = tolerance< V >( s * s * n * n );
 expect( std::abs( primal - a.cost ) <= t , "Wrong total cost: " + id );
 expect( std::abs( primal - dual ) <= t ,
	"Dual cost not equal to the primal one: " + id );
 }

// The violation of a set of nodes, its supply less what can leave it;
// false if an arc of infinite upper bound leaves it
template< typename V >
bool violation( const Instance< V > & p , const std::vector< char > & in ,
		double & gap )
{
 gap = 0;
 for( Digraph::NodeIt u( p.g ) ; u != INVALID ; ++u )
  if( in[ p.g.id( u ) ] )
   gap += p.supply[ u ];
 for( Digraph::ArcIt e( p.g ) ; e != INVALID ; ++e ) {
  const bool s = in[ p.g.id( p.g.source( e ) ) ] ,
	     t = in[ p.g.id( p.g.target( e ) ) ];
  if( s && ! t ) {
   if( p.upper[ e ] == p.inf )
    return( false );
   gap -= p.upper[ e ];
   }
  else if( ! s && t )
   gap += p.lower[ e ];
  }
 return( true );
 }

template< typename V >
std::vector< char > charOf( const Instance< V > & p ,
			    const std::vector< Node > & set )
{
 std::vector< char > in( countNodes( p.g ) , 0 );
 for( std::size_t i = 0 ; i != set.size() ; ++i )
  in[ p.g.id( set[ i ] ) ] = 1;
 return( in );
 }

// An infeasible answer: a set of nodes whose supply exceeds what can
// leave it; the k best sets, whose violations are the k largest ones of
// all the subsets of the nodes, enumerated, in order; disjoint sets, each
// violated, with no arc between two of them
template< typename V >
void checkCut( const Instance< V > & p , const Answer< V > & a ,
	       const std::string & id )
{
 const double tol = tolerance< V >( scaleOf( p ) * countArcs( p.g ) );
 expect( ! a.cut.empty() , "No infeasibility cut: " + id );
 double gap;
 expect( violation( p , charOf( p , a.cut ) , gap ) ,
	 "Infinite arc out of the cut: " + id );
 expect( gap > tol , "Wrong infeasibility cut: " + id );

 const int n = countNodes( p.g );
 std::vector< double > all;
 for( long mask = 1 ; n <= 14 && mask < ( 1L << n ) ; ++mask ) {
  std::vector< char > in( n );
  for( int u = 0 ; u != n ; ++u )
   in[ u ] = ( mask >> u ) & 1;
  double g;
  if( violation( p , in , g ) && g > tol )
   all.push_back( g );
  }
 std::sort( all.rbegin() , all.rend() );
 expect( ! a.cuts.empty() && a.cuts.size() == std::min< std::size_t >(
					      5 , all.size() ) ,
	 "Wrong number of best infeasibility cuts: " + id );
 std::set< std::vector< char > > seen;
 for( std::size_t j = 0 ; j != a.cuts.size() && j != all.size() ; ++j ) {
  const std::vector< char > in = charOf( p , a.cuts[ j ] );
  expect( violation( p , in , gap ) && std::abs( gap - all[ j ] ) <= tol ,
	  "Wrong violation of a best infeasibility cut: " + id );
  expect( seen.insert( in ).second , "A best cut given twice: " + id );
  }

 expect( ! a.disjoint.empty() , "No disjoint infeasibility cut: " + id );
 std::vector< int > owner( n , -1 );
 for( std::size_t j = 0 ; j != a.disjoint.size() ; ++j ) {
  const std::vector< char > in = charOf( p , a.disjoint[ j ] );
  expect( violation( p , in , gap ) && gap > tol ,
	  "Wrong disjoint infeasibility cut: " + id );
  for( std::size_t h = 0 ; h != a.disjoint[ j ].size() ; ++h ) {
   const int u = p.g.id( a.disjoint[ j ][ h ] );
   expect( owner[ u ] < 0 , "Disjoint cuts that meet: " + id );
   owner[ u ] = int( j );
   }
  }
 for( Digraph::ArcIt e( p.g ) ; e != INVALID ; ++e ) {
  const int s = owner[ p.g.id( p.g.source( e ) ) ] ,
	    t = owner[ p.g.id( p.g.target( e ) ) ];
  expect( s < 0 || t < 0 || s == t , "An arc between disjoint cuts: " + id );
  }
 }

// One cycle: directed, of negative cost, of infinite upper bounds; its
// mean cost in mean
template< typename V >
void checkOneCycle( const Instance< V > & p , const std::vector< Arc > & cyc ,
		    const std::string & id , double & mean )
{
 expect( ! cyc.empty() , "No unbounded cycle: " + id );
 double c = 0;
 for( std::size_t i = 0 ; i != cyc.size() ; ++i ) {
  const Arc e = cyc[ i ];
  c += p.cost[ e ];
  expect( p.upper[ e ] == p.inf , "Finite arc in the cycle: " + id );
  expect( p.g.target( e ) == p.g.source( cyc[ ( i + 1 ) % cyc.size() ] ) ,
	  "The unbounded cycle is not a cycle: " + id );
  }
 expect( c < 0 , "The unbounded cycle has a nonnegative cost: " + id );
 mean = cyc.empty() ? 0 : c / cyc.size();
 }

// An unbounded answer: a directed cycle of negative cost and infinite
// upper bounds, and arc-disjoint ones in the order of their means
template< typename V >
void checkCycle( const Instance< V > & p , const Answer< V > & a ,
		 const std::string & id )
{
 double mean , last = -std::numeric_limits< double >::infinity();
 checkOneCycle( p , a.cycle , id , mean );
 expect( ! a.cycles.empty() && a.cycles.size() <= 5 ,
	 "Wrong number of unbounded cycles: " + id );
 std::set< int > used;
 const double tol = tolerance< V >( scaleOf( p ) );
 for( std::size_t j = 0 ; j != a.cycles.size() ; ++j ) {
  checkOneCycle( p , a.cycles[ j ] , id , mean );
  expect( mean >= last - tol , "Unbounded cycles out of order: " + id );
  last = mean;
  for( std::size_t h = 0 ; h != a.cycles[ j ].size() ; ++h )
   expect( used.insert( p.g.id( a.cycles[ j ][ h ] ) ).second ,
	   "Unbounded cycles that share an arc: " + id );
  }
 }

// Every solver on an instance, each answer checked by itself and against
// the first one
template< typename V >
void runBattery( const Instance< V > & p ,
		 const std::vector< Solver< V > > & s )
{
 Answer< V > first;
 for( std::size_t k = 0 ; k != s.size() ; ++k ) {
  const std::string id = s[ k ].name + " on " + p.name;
  if( verbose )
   std::cout << id << std::endl;
  const Answer< V > a = s[ k ].run( p );
  if( a.result == OPTIMAL )
   checkOptimal( p , a , id );
  else if( a.result == INFEASIBLE )
   checkCut( p , a , id );
  else
   checkCycle( p , a , id );
  if( k == 0 ) {
   first = a;
   continue;
   }
  expect( a.result == first.result ,
	 id + " answers " + resultName( a.result ) + ", " + s[ 0 ].name +
	   " " + resultName( first.result ) );
  if( a.result == OPTIMAL && first.result == OPTIMAL )
   expect( std::abs( a.cost - first.cost ) <=
	    tolerance< V >( scaleOf( p ) * scaleOf( p ) *
			    countNodes( p.g ) * countNodes( p.g ) ) ,
	  id + " has the cost " + std::to_string( a.cost ) + ", " +
	    s[ 0 ].name + " " + std::to_string( first.cost ) );
  }
 }

// The limit cases, with the data multiplied by unit, which is not a
// power of two for the floating-point type
template< typename V >
std::vector< Instance< V > * > limitCases( V unit )
{
 std::vector< Instance< V > * > c;
 Instance< V > * p;

 c.push_back( p = new Instance< V >( "the empty digraph" ) );

 c.push_back( p = new Instance< V >( "one node and no supply" ) );
 p->node();

 c.push_back( p = new Instance< V >( "one node with a supply" ) );
 p->node( unit );

 c.push_back( p = new Instance< V >( "one node with a demand" ) );
 p->node( -unit );

 {
  c.push_back( p = new Instance< V >( "a loop of negative cost" ) );
  const Node u = p->node();
  p->arc( u , u , -unit , 3 * unit );
  }
 {
  c.push_back( p = new Instance< V >(
    "a loop of negative cost and infinite capacity" ) );
  const Node u = p->node();
  p->arc( u , u , -unit , p->inf );
  }
 {
  c.push_back( p = new Instance< V >( "two parallel arcs" ) );
  const Node u = p->node( 3 * unit ) , v = p->node( -3 * unit );
  p->arc( u , v , 2 * unit , 2 * unit );
  p->arc( u , v , unit , 2 * unit );
  }
 {
  c.push_back( p = new Instance< V >( "an arc of capacity zero" ) );
  const Node u = p->node( unit ) , v = p->node( -unit );
  p->arc( u , v , unit , 0 );
  }
 {
  c.push_back( p = new Instance< V >( "a flow fixed by its bounds" ) );
  const Node u = p->node( 2 * unit ) , v = p->node( -2 * unit );
  p->arc( u , v , -unit , 2 * unit , 2 * unit );
  }
 {
  c.push_back( p = new Instance< V >( "lower bounds that force a cycle" ) );
  const Node u = p->node() , v = p->node();
  p->arc( u , v , unit , 3 * unit , unit );
  p->arc( v , u , unit , 3 * unit );
  }
 {
  c.push_back( p = new Instance< V >( "a cut too small" ) );
  const Node u = p->node( 3 * unit ) , v = p->node() , w = p->node(
    -3 * unit );
  p->arc( u , v , unit , p->inf );
  p->arc( v , w , unit , 2 * unit );
  }
 {
  c.push_back( p = new Instance< V >( "lower bounds that cannot be met" ) );
  const Node u = p->node() , v = p->node() , w = p->node();
  p->arc( u , v , unit , 3 * unit , 2 * unit );
  p->arc( v , w , unit , unit );
  p->arc( w , u , unit , p->inf );
  }
 {
  c.push_back( p = new Instance< V >( "supplies that exceed the demands" ) );
  const Node u = p->node( 2 * unit ) , v = p->node( -unit );
  p->arc( u , v , unit , p->inf );
  }
 {
  c.push_back( p = new Instance< V >( "demands that exceed the supplies" ) );
  const Node u = p->node( unit ) , v = p->node( -2 * unit ) , w = p->node(
    -unit );
  p->arc( u , v , unit , p->inf );
  p->arc( u , w , 2 * unit , p->inf );
  }
 {
  c.push_back( p = new Instance< V >(
    "a negative cycle of infinite capacity" ) );
  const Node u = p->node( unit ) , v = p->node() , w = p->node( -unit );
  p->arc( u , v , unit , p->inf );
  p->arc( v , w , unit , p->inf );
  p->arc( w , v , -2 * unit , p->inf );
  }
 {
  c.push_back( p = new Instance< V >(
    "a negative cycle of infinite capacity on an infeasible problem" ) );
  const Node u = p->node( 2 * unit ) , v = p->node() , w = p->node( -2 *
								  unit );
  p->arc( u , v , unit , unit );
  p->arc( v , w , unit , p->inf );
  p->arc( w , v , -2 * unit , p->inf );
  }
 {
  c.push_back( p = new Instance< V >(
    "a negative cycle with one arc of finite capacity" ) );
  const Node u = p->node( unit ) , v = p->node() , w = p->node( -unit );
  p->arc( u , v , unit , p->inf );
  p->arc( v , w , unit , p->inf );
  p->arc( w , v , -2 * unit , 5 * unit );
  }
 {
  c.push_back( p = new Instance< V >( "two components" ) );
  const Node u = p->node( unit ) , v = p->node( -unit ) , w = p->node(
    2 * unit ) , x = p->node( -2 * unit );
  p->arc( u , v , 3 * unit , p->inf );
  p->arc( w , x , -unit , 2 * unit );
  p->arc( x , w , unit , p->inf );
  }
 {
  c.push_back( p = new Instance< V >( "costs of both signs on a path" ) );
  const Node u = p->node( 2 * unit ) , v = p->node() , w = p->node() ,
	     x = p->node( -2 * unit );
  p->arc( u , v , -3 * unit , unit );
  p->arc( u , w , 2 * unit , p->inf );
  p->arc( v , x , unit , p->inf );
  p->arc( w , x , -unit , p->inf );
  p->arc( v , w , 0 , unit );
  }
 return( c );
 }

// A pseudo-random instance: up to 12 nodes, three times as many arcs,
// among them loops and parallel arcs, lower bounds on some, infinite
// upper bounds on others, costs of both signs, supplies that sum to
// zero but on one instance in eight, all multiplied by unit
template< typename V >
Instance< V > * randomInstance( unsigned int & r , int seed , V unit )
{
 Instance< V > * p = new Instance< V >( "the random instance " +
					std::to_string( seed ) );
 r = r * 1103515245 + 12345;
 const int n = 1 + ( r >> 16 ) % 12;
 std::vector< Node > nodes;
 for( int i = 0 ; i != n ; ++i )
  nodes.push_back( p->node() );
 for( int k = 0 ; k != 3 * n ; ++k ) {
  r = r * 1103515245 + 12345;
  const Node u = nodes[ ( r >> 8 ) % n ] , v = nodes[ ( r >> 20 ) % n ];
  r = r * 1103515245 + 12345;
  const V low = ( r >> 4 ) % 10 < 3 ? V( ( r >> 8 ) % 4 ) * unit : V( 0 );
  const V up = ( r >> 12 ) % 20 < 3 ? p->inf
				    : low + V( ( r >> 16 ) % 10 ) * unit;
  p->arc( u , v , V( long( ( r >> 20 ) % 31 ) - 10 ) * unit , up , low );
  }
 V total = 0;
 for( int i = 1 ; i != n ; ++i ) {
  r = r * 1103515245 + 12345;
  p->supply[ nodes[ i ] ] = V( long( ( r >> 16 ) % 9 ) - 4 ) * unit;
  total += p->supply[ nodes[ i ] ];
  }
 r = r * 1103515245 + 12345;
 p->supply[ nodes[ 0 ] ] = -total + ( ( r >> 16 ) % 8 == 0 ? unit : V( 0 ) );
 return( p );
 }

// Two instances that the general checks do not tell apart from others:
// in the first, eight sets tie for the largest violation, the two nodes of
// supply unit, whose arcs to the two of demand unit are closed, with any
// subset of three nodes of no supply and no arc, and the best five sets
// are all of them, which come from the one maximum flow of the root
// [see infeasibleCuts()]; in the second, three node-disjoint cycles of
// negative cost and infinite capacity are all given, which come from one
// run of Howard's algorithm [see unboundedCycles()]
template< typename V >
void checkMany( V unit , const std::vector< Solver< V > > & s )
{
 Instance< V > ties( "eight sets that tie" );
 for( int h = 0 ; h != 2 ; ++h )
  ties.arc( ties.node( unit ) , ties.node( -unit ) , unit , 0 );
 for( int h = 0 ; h != 3 ; ++h )
  ties.node();
 Instance< V > rays( "three disjoint negative cycles" );
 for( int h = 0 ; h != 3 ; ++h ) {
  const Node u = rays.node() , v = rays.node();
  rays.arc( u , v , -V( h + 1 ) * unit , rays.inf );
  rays.arc( v , u , unit / 2 , rays.inf );
  }
 for( std::size_t k = 0 ; k != s.size() ; ++k ) {
  const Answer< V > a = s[ k ].run( ties );
  expect( a.result == INFEASIBLE && a.cuts.size() == 5 ,
	  "Not five best cuts of " + s[ k ].name + " on " + ties.name );
  if( a.result == INFEASIBLE )
   checkCut( ties , a , s[ k ].name + " on " + ties.name );
  for( std::size_t j = 0 ; j != a.cuts.size() ; ++j ) {
   double gap;
   expect( violation( ties , charOf( ties , a.cuts[ j ] ) , gap ) &&
	     std::abs( gap - 2 * double( unit ) ) <=
	       tolerance< V >( scaleOf( ties ) ) ,
	   "A best cut that does not tie of " + s[ k ].name );
   }
  const Answer< V > b = s[ k ].run( rays );
  expect( b.result == UNBOUNDED && b.cycles.size() == 3 ,
	  "Not three cycles of " + s[ k ].name + " on " + rays.name );
  if( b.result == UNBOUNDED )
   checkCycle( rays , b , s[ k ].name + " on " + rays.name );
  }
 }

// The solvers whose name contains this text, all if it is empty
std::string only;

template< typename V >
void runAll( V unit , int seeds )
{
 std::vector< Solver< V > > s;
 const std::vector< Solver< V > > all = solvers< V >();
 for( std::size_t k = 0 ; k != all.size() ; ++k )
  if( k == 0 || all[ k ].name.find( only ) != std::string::npos )
   s.push_back( all[ k ] );
 checkMany( unit , s );
 std::vector< Instance< V > * > c = limitCases( unit );
 unsigned int r = 2026;
 for( int seed = 0 ; seed != seeds ; ++seed )
  c.push_back( randomInstance( r , seed , unit ) );
 std::ostringstream u;
 u << " with the unit " << unit;
 for( std::size_t i = 0 ; i != c.size() ; ++i ) {
  c[ i ]->name += u.str();
  runBattery( *c[ i ] , s );
  delete c[ i ];
  }
 }

int main( int argc , char ** argv )
{
 const std::string type = argc > 1 ? argv[ 1 ] : "";
 if( argc > 2 )
  only = argv[ 2 ];
 if( type.empty() || type == "long" )
  runAll< long >( 1 , 500 );
 if( type.empty() || type == "double" ) {
  runAll< double >( 1 , 500 );
  runAll< double >( 0.1 , 500 );
  runAll< double >( 1e-3 , 200 );
  runAll< double >( 1e6 / 3 , 200 );
  }
 if( failures != 0 )
  std::cerr << failures << " failures" << std::endl;
 return( failures == 0 ? 0 : 1 );
 }
