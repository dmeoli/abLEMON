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

#ifndef LEMON_TEST_HEAP_TEST_H
 #define LEMON_TEST_HEAP_TEST_H

#include <algorithm>
#include <functional>
#include <vector>

#include <lemon/core.h>
#include <lemon/dijkstra.h>
#include <lemon/maps.h>

#include "test_tools.h"

// Function templates that run a class of the concept concepts::Heap: they
// check the order of the priorities given back after push(), increase()
// and erase(), and the shortest path tree of a Dijkstra that uses the heap

namespace lemon
{

// the priorities pushed, and the increases of heapIncreaseTest()
const int heap_test_seq[] = { 2 , 28 , 19 , 27 , 33 , 25 , 13 , 41 , 10 , 26 ,
			      1 , 9 , 4 , 34 };
const int heap_test_inc[] = { 20 , 28 , 34 , 16 , 0 , 46 , 44 , 0 , 42 , 32 ,
			      14 , 8 , 6 , 37 };

const int heap_test_len = sizeof( heap_test_seq ) /
			  sizeof( heap_test_seq[ 0 ] );

template< typename Heap >
void heapSortTest()
{
 RangeMap< int > map( heap_test_len , -1 );
 Heap heap( map );

 std::vector< int > v( heap_test_len );
 for( int i = 0 ; i < heap_test_len ; ++i ) {
  v[ i ] = heap_test_seq[ i ];
  heap.push( i , v[ i ] );
  }
 std::sort( v.begin() , v.end() );
 for( int i = 0 ; i < heap_test_len ; ++i ) {
  check( v[ i ] == heap.prio() , "Wrong order in heap sort." );
  heap.pop();
  }
 }

// Priorities of a floating-point type, many of them equal and many below
// one: the heap has to give them back in order, ties and fractions included
template< typename Heap >
void heapTiesTest()
{
 const int n = 200;
 RangeMap< int > map( n , -1 );
 Heap heap( map );
 std::vector< double > v( n );
 for( int i = 0 ; i < n ; ++i ) {
  v[ i ] = ( ( i * 37 ) % 11 ) / 4.0;
  heap.push( i , v[ i ] );
  }
 std::sort( v.begin() , v.end() );
 for( int i = 0 ; i < n ; ++i ) {
  check( ! heap.empty() , "Heap emptied too early." );
  check( v[ i ] == heap.prio() , "Wrong order with ties and fractions." );
  heap.pop();
  }
 check( heap.empty() , "Heap not emptied." );
 }

// Two items of equal priority, popped one after the other: a heap that
// keeps a minimum which is no longer a root loops for ever on the second
template< typename Heap >
void heapEqualPairTest()
{
 RangeMap< int > map( 2 , -1 );
 Heap heap( map );
 heap.push( 0 , 0 );
 heap.push( 1 , 0 );
 heap.pop();
 heap.pop();
 check( heap.empty() , "Heap not emptied after two equal priorities." );
 }

template< typename Heap >
void heapIncreaseTest()
{
 RangeMap< int > map( heap_test_len , -1 );

 Heap heap( map );

 std::vector< int > v( heap_test_len );
 for( int i = 0 ; i < heap_test_len ; ++i ) {
  v[ i ] = heap_test_seq[ i ];
  heap.push( i , v[ i ] );
  }
 for( int i = 0 ; i < heap_test_len ; ++i ) {
  v[ i ] += heap_test_inc[ i ];
  heap.increase( i , v[ i ] );
  }
 std::sort( v.begin() , v.end() );
 for( int i = 0 ; i < heap_test_len ; ++i ) {
  check( v[ i ] == heap.prio() , "Wrong order in heap increase test." );
  heap.pop();
  }
 }

// erase() removes the given item, also where no priority is below the
// minimum: under std::greater (the heap of maxima), and on doubles so
// large that x - 1 == x, where an erase by decreasing below the minimum
// removed the minimum in its place
template< template< typename , typename , typename > class H >
void heapEraseTest()
{
 {
  typedef H< double , RangeMap< int > , std::greater< double > > Heap;
  RangeMap< int > map( 10 , -1 );
  Heap heap( map );
  for( int i = 0 ; i < 10 ; ++i )
   heap.push( i , i );
  heap.erase( 1 );
  heap.erase( 9 );
  check( heap.state( 1 ) == Heap::POST_HEAP &&
	   heap.state( 9 ) == Heap::POST_HEAP ,
	 "Wrong item erased under std::greater." );
  for( int i = 8 ; i >= 0 ; --i ) {
   if( i == 1 )
    continue;
   check( heap.top() == i , "Wrong order after erase under std::greater." );
   heap.pop();
   }
  check( heap.empty() , "Wrong size after erase under std::greater." );
  }
 {
  typedef H< double , RangeMap< int > , std::less< double > > Heap;
  RangeMap< int > map( 5 , -1 );
  Heap heap( map );
  for( int i = 0 ; i < 5 ; ++i )
   heap.push( i , 1e20 * ( i + 1 ) );
  heap.erase( 3 );
  heap.erase( 0 );
  check( heap.state( 3 ) == Heap::POST_HEAP &&
	   heap.state( 0 ) == Heap::POST_HEAP ,
	 "Wrong item erased on large priorities." );
  const int rest[] = { 1 , 2 , 4 };
  for( int k = 0 ; k < 3 ; ++k ) {
   check( heap.top() == rest[ k ] , "Wrong order after erase on large "
				   "priorities." );
   heap.pop();
   }
  }
 }

template< typename Heap , typename GR , typename LEN >
void dijkstraHeapTest( const GR & digraph , const LEN & length ,
		       typename GR::Node source )
{
 TEMPLATE_DIGRAPH_TYPEDEFS( GR );

 typename Dijkstra< GR , LEN >::template SetStandardHeap<
   Heap >::Create dijkstra( digraph , length );

 dijkstra.run( source );

 for( ArcIt a( digraph ) ; a != INVALID ; ++a ) {
  Node s = digraph.source( a );
  Node t = digraph.target( a );
  if( dijkstra.reached( s ) ) {
   check( dijkstra.dist( t ) - dijkstra.dist( s ) <= length[ a ] ,
	  "Error in shortest path tree." );
   }
  }

 for( NodeIt n( digraph ) ; n != INVALID ; ++n ) {
  if( dijkstra.reached( n ) && dijkstra.predArc( n ) != INVALID ) {
   Arc a = dijkstra.predArc( n );
   Node s = digraph.source( a );
   check( dijkstra.dist( n ) - dijkstra.dist( s ) == length[ a ] ,
	  "Error in shortest path tree." );
   }
  }
 }

}  //namespace lemon

#endif
