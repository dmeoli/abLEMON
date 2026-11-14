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

// Elevator with fewer levels than items, and the lift of an item of
// LinkedElevator from the end or the start of its level

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

int main()
{
 Digraph g;
 std::vector< Node > v;
 for( int i = 0 ; i < 5 ; ++i )
  v.push_back( g.addNode() );
 checkFewLevels< Elevator< Digraph , Node > >( g , v );
 checkLinkedLift( g , v );
 return( 0 );
 }
