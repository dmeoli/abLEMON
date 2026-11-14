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

// Elevator with fewer levels than items, the lift of an item of
// LinkedElevator from the end or the start of its level, the active item of
// a level, and the lift of the levels to the top

#ifndef _GLIBCXX_ASSERTIONS
 #define _GLIBCXX_ASSERTIONS
#endif

#include <lemon/elevator.h>
#include <lemon/smart_graph.h>

#include "test_tools.h"

using namespace lemon;

typedef SmartDigraph Digraph;
typedef Digraph::Node Node;

template< typename E >
void checkFewLevels( const Digraph & g , const std::vector< Node > & v )
{
 // five items, levels 0 .. 2: v[ 0 ] on level 0, v[ 1 ] on level 1, the
 // other three on the top level
 E e( g , 2 );
 e.initStart();
 e.initAddItem( v[ 0 ] );
 e.initNewLevel();
 e.initAddItem( v[ 1 ] );
 e.initFinish();
 check( e.onLevel( 0 ) == 1 && e.onLevel( 1 ) == 1 && e.onLevel( 2 ) == 3 ,
	"Wrong number of items on a level." );
 check( e.aboveLevel( 0 ) == 4 , "Wrong number of items above a level." );
 check( e[ v[ 4 ] ] == 2 , "Wrong level of an item." );
 }

void checkLinkedLift( const Digraph & g , const std::vector< Node > & v )
{
 // three items on level 0, then the last and the first one lifted
 LinkedElevator< Digraph , Node > e( g , 4 );
 e.initStart();
 e.initAddItem( v[ 0 ] );
 e.initAddItem( v[ 1 ] );
 e.initAddItem( v[ 2 ] );
 e.initFinish();
 int on0 = e.onLevel( 0 );
 for( int i = 0 ; i < 3 ; ++i )
  e.activate( v[ i ] );
 Node top = e.highestActive();
 e.lift( top , 2 );
 check( e.onLevel( 0 ) == on0 - 1 && e.onLevel( 2 ) == 1 ,
	"Wrong levels after the lift of an item." );
 check( e[ top ] == 2 , "Wrong level of a lifted item." );
 Node next = INVALID;
 for( int i = 0 ; i < 3 ; ++i )
  if( v[ i ] != top && e[ v[ i ] ] == 0 && next == INVALID )
   next = v[ i ];
 e.lift( next , 3 );
 check( e.onLevel( 0 ) == on0 - 2 && e.onLevel( 3 ) == 1 &&
	  e.onLevel( 2 ) == 1 ,
	"Wrong levels after the lift of a second item." );
 }

// the lift of the active item of a level, returned by both classes, and
// activeOn() on a level without items, which reads no item map; the test
// is built with the checks of the standard library, if any, so that a read
// of a map at INVALID aborts it
template< typename E >
void checkActiveOn( const Digraph & g , const std::vector< Node > & v )
{
 E e( g , 4 );
 e.initStart();
 e.initAddItem( v[ 0 ] );
 e.initAddItem( v[ 1 ] );
 e.initFinish();
 check( e.activeOn( 1 ) == INVALID && e.activeOn( 2 ) == INVALID ,
	"activeOn() found an item on an empty level." );
 e.activate( v[ 0 ] );
 Node a = e.activeOn( 0 );
 check( a == v[ 0 ] , "Wrong active item on a level." );
 check( e.liftActiveOn( 0 ) == a , "liftActiveOn() returned a wrong item." );
 check( e[ a ] == 1 && e.activeOn( 1 ) == a && e.activeOn( 0 ) == INVALID ,
	"Wrong levels after liftActiveOn()." );
 }

// liftToTop( l ) lifts every item on and above level l to the top level and
// deactivates it, also above an empty level
template< typename E >
void checkLiftToTop( const Digraph & g , const std::vector< Node > & v )
{
 // v[ 0 ] on level 0, v[ 1 ] on level 1, v[ 2 ] on level 3, level 2 empty
 E e( g , 5 );
 e.initStart();
 e.initAddItem( v[ 0 ] );
 e.initNewLevel();
 e.initAddItem( v[ 1 ] );
 e.initNewLevel();
 e.initNewLevel();
 e.initAddItem( v[ 2 ] );
 e.initFinish();
 for( int i = 0 ; i < 3 ; ++i )
  e.activate( v[ i ] );
 e.liftToTop( 1 );
 check( e[ v[ 0 ] ] == 0 && e.active( v[ 0 ] ) ,
	"liftToTop() moved an item below its level." );
 for( int i = 1 ; i < 3 ; ++i )
  check( e[ v[ i ] ] == 5 && ! e.active( v[ i ] ) ,
	 "liftToTop() did not lift an item to the top level." );
 check( e.highestActive() == v[ 0 ] && e.highestActiveLevel() == 0 ,
	"Wrong highest active item after liftToTop()." );
 check( e.emptyLevel( 1 ) && e.emptyLevel( 3 ) ,
	"liftToTop() left an item on its level." );
 }

int main()
{
 Digraph g;
 std::vector< Node > v;
 for( int i = 0 ; i < 5 ; ++i )
  v.push_back( g.addNode() );
 checkFewLevels< Elevator< Digraph , Node > >( g , v );
 checkLinkedLift( g , v );
 checkActiveOn< Elevator< Digraph , Node > >( g , v );
 checkActiveOn< LinkedElevator< Digraph , Node > >( g , v );
 checkLiftToTop< Elevator< Digraph , Node > >( g , v );
 checkLiftToTop< LinkedElevator< Digraph , Node > >( g , v );
 return( 0 );
 }
