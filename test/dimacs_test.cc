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

// The DIMACS readers on small files, and on node numbers outside 1 to the
// number of nodes of the problem line, which are a FormatError

#include <sstream>
#include <string>

#include <lemon/dimacs.h>
#include <lemon/list_graph.h>
#include "test_tools.h"

using namespace lemon;

typedef ListDigraph::ArcMap< int > IntArcMap;

// reads the file as the problem of its problem line, and returns whether
// the reader threw FormatError
bool readFails( const std::string & file )
{
 std::istringstream is( file );
 ListDigraph g;
 IntArcMap low( g ) , cap( g ) , cost( g );
 ListDigraph::NodeMap< int > sup( g );
 ListDigraph::Node s , t;
 try {
  DimacsDescriptor desc = dimacsType( is );
  if( desc.type == DimacsDescriptor::MIN )
   readDimacsMin( is , g , low , cap , cost , sup , 0 , desc );
  else if( desc.type == DimacsDescriptor::MAX )
   readDimacsMax( is , g , cap , s , t , 0 , desc );
  else if( desc.type == DimacsDescriptor::SP )
   readDimacsSp( is , g , cap , s , desc );
  else {
   ListGraph ug;
   readDimacsMat( is , ug , desc );
   }
  }
 catch( FormatError & ) {
  return( true );
  }
 return( false );
 }

int main()
{
 // a correct file of each type
 check( ! readFails( "p min 2 1\nn 1 3\nn 2 -3\na 1 2 0 5 1\n" ) ,
	"A correct min file failed" );
 check( ! readFails( "p max 2 1\nn 1 s\nn 2 t\na 1 2 5\n" ) ,
	"A correct max file failed" );
 check( ! readFails( "p sp 2 1\nn 1\na 1 2 5\n" ) ,
	"A correct sp file failed" );
 check( ! readFails( "p mat 2 1\na 1 2\n" ) , "A correct mat file failed" );

 {
  std::istringstream is( "c a comment\np min 2 1\nn 1 3\nn 2 -3\n"
			 "a 1 2 0 5 7\n" );
  ListDigraph g;
  IntArcMap low( g ) , cap( g ) , cost( g );
  ListDigraph::NodeMap< int > sup( g );
  readDimacsMin( is , g , low , cap , cost , sup );
  check( countNodes( g ) == 2 && countArcs( g ) == 1 , "Wrong digraph" );
  ListDigraph::Arc a = ListDigraph::ArcIt( g );
  check( low[ a ] == 0 && cap[ a ] == 5 && cost[ a ] == 7 , "Wrong arc data" );
  check( sup[ g.source( a ) ] == 3 && sup[ g.target( a ) ] == -3 ,
	 "Wrong supplies" );
  }

 // node numbers 0 and n + 1 in every kind of line that has them
 const char * const bad[] = {
  "p min 2 1\nn 0 3\n" ,       "p min 2 1\nn 3 3\n" ,
  "p min 2 1\na 1 3 0 5 1\n" ,
  "p min 2 1\na 0 2 0 5 1\n" , "p max 2 1\nn 3 s\n" , "p max 2 1\nn 0 t\n" ,
  "p max 2 1\na 1 3 5\n" ,     "p sp 2 1\nn 3\n" ,    "p sp 2 1\na 0 2 5\n" ,
  "p mat 2 1\na 1 3\n" ,       "p mat 2 1\na 0 1\n"
  };
 for( unsigned i = 0 ; i < sizeof( bad ) / sizeof( bad[ 0 ] ) ; ++i ) {
  check( readFails( bad[ i ] ) , "A node number out of range was read" );
  }

 return( 0 );
 }
