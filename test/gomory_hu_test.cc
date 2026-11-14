/* -*- mode: C++; indent-tabs-mode: nil; -*-
 *
 * This file is a part of LEMON, a generic C++ optimization library.
 *
 * Copyright (C) 2003-2013
 * Egervary Jeno Kombinatorikus Optimalizalasi Kutatocsoport
 * (Egervary Research Group on Combinatorial Optimization, EGRES).
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

#include <iostream>

#include "test_tools.h"
#include <lemon/smart_graph.h>
#include <lemon/concepts/graph.h>
#include <lemon/concepts/maps.h>
#include <lemon/lgf_reader.h>
#include <lemon/gomory_hu.h>
#include <cstdlib>
#include <vector>

using namespace std;
using namespace lemon;

typedef SmartGraph Graph;

char test_lgf[] = "@nodes\n"
		  "label\n"
		  "0\n"
		  "1\n"
		  "2\n"
		  "3\n"
		  "4\n"
		  "@arcs\n"
		  "     label capacity\n"
		  "0 1  0     1\n"
		  "1 2  1     1\n"
		  "2 3  2     1\n"
		  "0 3  4     5\n"
		  "0 3  5     10\n"
		  "0 3  6     7\n"
		  "4 2  7     1\n"
		  "@attributes\n"
		  "source 0\n"
		  "target 3\n";

void checkGomoryHuCompile()
{
 typedef int Value;
 typedef concepts::Graph Graph;

 typedef Graph::Node Node;
 typedef Graph::Edge Edge;
 typedef concepts::ReadMap< Edge , Value > CapMap;
 typedef concepts::ReadWriteMap< Node , bool > CutMap;

 Graph g;
 Node n;
 CapMap cap;
 CutMap cut;
 Value v;
 int d;
 ::lemon::ignore_unused_variable_warning( v , d );

 GomoryHu< Graph , CapMap > gh_test( g , cap );
 const GomoryHu< Graph , CapMap > & const_gh_test = gh_test;

 gh_test.run();
 gh_test.tolerance( const_gh_test.tolerance() );

 n = const_gh_test.predNode( n );
 v = const_gh_test.predValue( n );
 d = const_gh_test.rootDist( n );
 v = const_gh_test.minCutValue( n , n );
 v = const_gh_test.minCutMap( n , n , cut );
 }

GRAPH_TYPEDEFS( Graph );
typedef Graph::EdgeMap< int > IntEdgeMap;
typedef Graph::NodeMap< bool > BoolNodeMap;

int cutValue( const Graph & graph , const BoolNodeMap & cut ,
	      const IntEdgeMap & capacity )
{

 int sum = 0;
 for( EdgeIt e( graph ) ; e != INVALID ; ++e ) {
  Node s = graph.u( e );
  Node t = graph.v( e );

  if( cut[ s ] != cut[ t ] ) {
   sum += capacity[ e ];
   }
  }
 return( sum );
 }

// A cut map that counts its writes and the writes on INVALID
struct CountingCutMap
{
 typedef Node Key;
 typedef bool Value;

 BoolNodeMap map;
 int sets , invalid_sets;

 CountingCutMap( const Graph & graph )
     : map( graph , false ) , sets( 0 ) , invalid_sets( 0 ) { }

 void set( const Node & n , bool v ) {
  ++sets;
  if( n == INVALID )
   ++invalid_sets;
  else
   map[ n ] = v;
  }

 bool operator[]( const Node & n ) const { return( map[ n ] ); }
 };

// minCutMap() writes each node once, also on a deep tree
void checkMinCutMapWrites()
{
 Graph graph;
 IntEdgeMap capacity( graph );
 const int n = 20;
 std::vector< Node > nodes;
 for( int i = 0 ; i < n ; ++i )
  nodes.push_back( graph.addNode() );
 for( int i = 0 ; i + 1 < n ; ++i )
  capacity[ graph.addEdge( nodes[ i ] , nodes[ i + 1 ] ) ] = i + 1;

 GomoryHu< Graph > ght( graph , capacity );
 ght.run();

 for( int i = 0 ; i < n ; ++i )
  for( int j = 0 ; j < n ; ++j ) {
   if( i == j )
    continue;
   CountingCutMap cm( graph );
   int value = ght.minCutMap( nodes[ i ] , nodes[ j ] , cm );
   check( cm.sets == n , "minCutMap() writes a node more than once" );
   check( cm.invalid_sets == 0 , "minCutMap() writes INVALID" );
   check( value == std::min( i , j ) + 1 , "Wrong cut value" );
   check( value == cutValue( graph , cm.map , capacity ) , "Wrong cut" );
   check( cm[ nodes[ i ] ] && ! cm[ nodes[ j ] ] , "Wrong sides" );
   }
 }

int main()
{
 Graph graph;
 IntEdgeMap capacity( graph );

 std::istringstream input( test_lgf );
 GraphReader< Graph >( graph , input ).edgeMap( "capacity" , capacity ).run();

 GomoryHu< Graph > ght( graph , capacity );
 ght.run();

 for( NodeIt u( graph ) ; u != INVALID ; ++u ) {
  for( NodeIt v( graph ) ; v != u ; ++v ) {
   Preflow< Graph , IntEdgeMap > pf( graph , capacity , u , v );
   pf.runMinCut();
   BoolNodeMap cm( graph );
   ght.minCutMap( u , v , cm );
   check( pf.flowValue() == ght.minCutValue( u , v ) , "Wrong cut 1" );
   check( cm[ u ] != cm[ v ] , "Wrong cut 2" );
   check( pf.flowValue() == cutValue( graph , cm , capacity ) ,
	  "Wrong cut 3" );

   int sum = 0;
   for( GomoryHu< Graph >::MinCutEdgeIt a( ght , u , v ) ; a != INVALID ; ++a )
    sum += capacity[ a ];
   check( sum == ght.minCutValue( u , v ) , "Problem with MinCutEdgeIt" );

   sum = 0;
   for( GomoryHu< Graph >::MinCutNodeIt n( ght , u , v , true ) ;
	n != INVALID ;
	++n )
    sum++;
   for( GomoryHu< Graph >::MinCutNodeIt n( ght , u , v , false ) ;
	n != INVALID ;
	++n )
    sum++;
   check( sum == countNodes( graph ) , "Problem with MinCutNodeIt" );
   }
  }

 checkMinCutMapWrites();

 return( 0 );
 }
