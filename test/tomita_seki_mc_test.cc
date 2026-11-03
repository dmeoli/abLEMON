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


// TomitaSekiMc against the enumeration of all the subsets of nodes on
// small random graphs, and against GrossoLocatelliPullanMc, which cannot
// find a larger clique, on larger ones; the limit on the search tree

#include <vector>

#include <lemon/full_graph.h>
#include <lemon/grosso_locatelli_pullan_mc.h>
#include <lemon/list_graph.h>
#include <lemon/random.h>
#include <lemon/tomita_seki_mc.h>

#include "test_tools.h"

using namespace lemon;

typedef ListGraph Graph;
GRAPH_TYPEDEFS( Graph );

// the size of a maximum clique, by enumeration
int bruteForce( const Graph & g )
{
 std::vector< Node > v;
 for( NodeIt n( g ) ; n != INVALID ; ++n ) {
  v.push_back( n );
  }
 int n = int( v.size() ) , best = 0;
 std::vector< std::vector< char > > adj( n , std::vector< char >( n , 0 ) );
 for( int i = 0 ; i < n ; ++i ) {
  for( int j = 0 ; j < n ; ++j ) {
   adj[ i ][ j ] = findEdge( g , v[ i ] , v[ j ] ) != INVALID;
   }
  }
 for( long mask = 1 ; mask < ( 1L << n ) ; ++mask ) {
  int size = 0;
  bool clique = true;
  for( int i = 0 ; i < n && clique ; ++i ) {
   if( mask >> i & 1 ) {
    ++size;
    for( int j = i + 1 ; j < n && clique ; ++j ) {
     clique = ! ( mask >> j & 1 ) || adj[ i ][ j ];
     }
    }
   }
  if( clique && size > best ) {
   best = size;
   }
  }
 return( best );
 }

// the clique found is a clique of the size given
void checkClique( const Graph & g , const TomitaSekiMc< Graph > & mc )
{
 BoolNodeMap in( g );
 mc.cliqueMap( in );
 std::vector< Node > c;
 for( NodeIt n( g ) ; n != INVALID ; ++n ) {
  if( in[ n ] ) {
   c.push_back( n );
   }
  }
 check( int( c.size() ) == mc.cliqueSize() , "wrong size of the clique" );
 for( std::size_t i = 0 ; i < c.size() ; ++i ) {
  for( std::size_t j = i + 1 ; j < c.size() ; ++j ) {
   check( findEdge( g , c[ i ] , c[ j ] ) != INVALID , "not a clique" );
   }
  }
 }

void randomGraph( Graph & g , int n , double p )
{
 std::vector< Node > v;
 for( int i = 0 ; i < n ; ++i ) {
  v.push_back( g.addNode() );
  }
 for( int i = 0 ; i < n ; ++i ) {
  if( rnd() < .1 ) {
   g.addEdge( v[ i ] , v[ i ] );
   }
  for( int j = i + 1 ; j < n ; ++j ) {
   while( rnd() < p ) {
    g.addEdge( v[ i ] , v[ j ] );
    if( rnd() < .8 ) {
     break;
     }
    }
   }
  }
 }

int main()
{
 rnd.seed( 20261103 );
 for( int n : { 0 , 1 , 2 , 5 , 10 , 16 } ) {
  for( double p : { 0. , .2 , .5 , .8 , 1. } ) {
   for( int k = 0 ; k < 4 ; ++k ) {
    Graph g;
    randomGraph( g , n , p );
    TomitaSekiMc< Graph > mc( g );
    check( mc.run() , "no limit, yet not proved" );
    check( mc.cliqueSize() == bruteForce( g ) , "not maximum" );
    checkClique( g , mc );
    }
   }
  }
 const double larger[][ 2 ] = { { 80 , .3 } , { 80 , .6 } , { 40 , .9 } };
 for( const auto & size : larger ) {
  int n = int( size[ 0 ] );
  double p = size[ 1 ];
  {
   Graph g;
   randomGraph( g , n , p );
   TomitaSekiMc< Graph > mc( g );
   check( mc.run() , "no limit, yet not proved" );
   checkClique( g , mc );
   GrossoLocatelliPullanMc< Graph > glp( g );
   glp.run();
   check( glp.cliqueSize() <= mc.cliqueSize() ,
	  "the heuristic found a larger clique" );
   // a limit of one node of the tree stops the search, with a clique
   TomitaSekiMc< Graph > stopped( g );
   check( ! stopped.treeLimit( 1 ).run() && stopped.treeSize() == 1 ,
	  "the limit did not stop the search" );
   checkClique( g , stopped );
   }
  }
 // the complete graph, whose clique is all of it
 FullGraph full( 30 );
 TomitaSekiMc< FullGraph > fmc( full );
 check( fmc.run() && fmc.cliqueSize() == 30 , "wrong clique of K30" );
 return( 0 );
 }
