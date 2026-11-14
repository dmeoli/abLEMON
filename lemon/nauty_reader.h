/* -*- mode: C++; indent-tabs-mode: nil; -*-
 *
 * This file is a part of LEMON, a generic C++ optimization library.
 *
 * Copyright (C) 2003-2009
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

#ifndef LEMON_NAUTY_READER_H
 #define LEMON_NAUTY_READER_H

#include <vector>
#include <iostream>
#include <string>

/// \ingroup nauty_group
/// \file
/// \brief Nauty file reader.

namespace lemon
{

/// \ingroup nauty_group
///
/// \brief Nauty file reader
///
/// The \e geng program is in the \e gtools suite of the nauty
/// package. This tool can generate all non-isomorphic undirected
/// graphs of several classes with given node number (e.g.
/// general, connected, biconnected, triangle-free, 4-cycle-free,
/// bipartite and graphs with given edge number and degree
/// constraints). This function reads a \e nauty \e graph6 \e format
/// line from the given stream and builds it in the given graph.
///
/// The site of nauty package: http://cs.anu.edu.au/~bdm/nauty/
///
/// For example, the number of all non-isomorphic planar graphs
/// can be computed with the following code.
///\code
/// int num = 0;
/// SmartGraph graph;
/// while( readNautyGraph( graph , std::cin ) ) {
///  PlanarityChecking< SmartGraph > pc( graph );
///  if( pc.run() )
///   ++num;
///  }
/// std::cout << "Number of planar graphs: " << num << std::endl;
///\endcode
///
/// The nauty files are quite huge, therefore instead of the direct
/// file generation pipelining is recommended. For example,
///\code
/// ./geng -c 10 | ./num_of_planar_graphs
///\endcode
template< typename Graph >
std::istream & readNautyGraph( Graph & graph , std::istream & is = std::cin )
{
 graph.clear();

 std::string line;
 if( getline( is , line ) ) {
  std::size_t index = 0;

  int n;

  // a line too short for its header, its number of nodes or its edges is
  // not a graph: the graph is left empty and the stream failed, where the
  // line was read past its end
  if( ! line.empty() && line[ index ] == '>' ) {
   index += 10;
   }
  if( index >= line.size() ) {
   is.setstate( std::ios::failbit );
   return( is );
   }

  char c = line[ index++ ];
  c -= 63;
  if( c != 63 ) {
   n = int( c );
   }
  else {
   if( index + 3 > line.size() ) {
    is.setstate( std::ios::failbit );
    return( is );
    }
   c = line[ index++ ];
   c -= 63;
   n = ( int( c ) << 12 );
   c = line[ index++ ];
   c -= 63;
   n |= ( int( c ) << 6 );
   c = line[ index++ ];
   c -= 63;
   n |= int( c );
   }

  const std::size_t bytes = ( std::size_t( n ) * ( n - 1 ) / 2 + 5 ) / 6;
  if( n < 0 || index + bytes > line.size() ) {
   is.setstate( std::ios::failbit );
   return( is );
   }

  std::vector< typename Graph::Node > nodes;
  for( int i = 0 ; i < n ; ++i ) {
   nodes.push_back( graph.addNode() );
   }

  int bit = -1;
  for( int j = 0 ; j < n ; ++j ) {
   for( int i = 0 ; i < j ; ++i ) {
    if( bit == -1 ) {
     c = line[ index++ ];
     c -= 63;
     bit = 5;
     }
    bool b = ( c & ( 1 << ( bit-- ) ) ) != 0;

    if( b ) {
     graph.addEdge( nodes[ i ] , nodes[ j ] );
     }
    }
   }
  }
 return( is );
 }
}

#endif
