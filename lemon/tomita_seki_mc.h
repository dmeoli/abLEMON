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

#ifndef LEMON_TOMITA_SEKI_MC_H
 #define LEMON_TOMITA_SEKI_MC_H

#include <algorithm>
#include <vector>

#include <lemon/core.h>

///\ingroup approx_algs
///\file
///\brief The exact maximum clique algorithm of Tomita and Seki.

namespace lemon
{

/// \addtogroup approx_algs
/// @{

/// \brief The exact maximum clique algorithm of Tomita and Seki
///
/// This class finds a maximum clique of an undirected graph by the branch
/// and bound of Tomita and Seki (MCQ): the candidates that may extend the
/// current clique are colored greedily, so that those of the same color
/// are not adjacent, and a branch is cut as soon as the current clique
/// and the number of colors of its candidates cannot exceed the best
/// clique found. Unlike \ref GrossoLocatelliPullanMc, a heuristic, it
/// proves that the clique it finds is maximum, in time that may grow
/// exponentially with the size of the graph; a limit on the number of
/// nodes of the search tree stops it early, with the best clique found.
///
/// \tparam GR The undirected graph type the algorithm runs on.
template< typename GR >
class TomitaSekiMc
{
 public:
 /// The undirected graph type of the algorithm
 typedef GR Graph;

 private:
 TEMPLATE_GRAPH_TYPEDEFS( GR );

 const GR & _graph;
 IntNodeMap _id;
 std::vector< Node > _nodes;
 std::vector< std::vector< char > > _adj;
 std::vector< int > _clique , _best;
 long long _tree , _tree_limit;
 bool _stopped;

 // the candidates P, colored greedily in their order: order lists them
 // by color, and color[ k ] is the number of colors of order[ 0..k ]
 void colorSort( const std::vector< int > & p , std::vector< int > & order ,
		 std::vector< int > & color ) const {
  std::vector< std::vector< int > > classes;
  for( std::size_t i = 0 ; i < p.size() ; ++i ) {
   int u = p[ i ];
   std::size_t c = 0;
   for( ; c < classes.size() ; ++c ) {
    bool free = true;
    for( std::size_t j = 0 ; j < classes[ c ].size() && free ; ++j ) {
     free = ! _adj[ u ][ classes[ c ][ j ] ];
     }
    if( free ) {
     break;
     }
    }
   if( c == classes.size() ) {
    classes.push_back( std::vector< int >() );
    }
   classes[ c ].push_back( u );
   }
  order.clear();
  color.clear();
  for( std::size_t c = 0 ; c < classes.size() ; ++c ) {
   for( std::size_t j = 0 ; j < classes[ c ].size() ; ++j ) {
    order.push_back( classes[ c ][ j ] );
    color.push_back( int( c ) + 1 );
    }
   }
  }

 void expand( const std::vector< int > & p ) {
  std::vector< int > order , color;
  colorSort( p , order , color );
  for( std::size_t k = order.size() ; k-- > 0 ; ) {
   if( int( _clique.size() ) + color[ k ] <= int( _best.size() ) ) {
    return;
    }
   if( _tree_limit >= 0 && _tree >= _tree_limit ) {
    _stopped = true;
    return;
    }
   ++_tree;
   int v = order[ k ];
   _clique.push_back( v );
   // the candidates still left (those before v) adjacent to v
   std::vector< int > np;
   for( std::size_t j = 0 ; j < k ; ++j ) {
    if( _adj[ v ][ order[ j ] ] ) {
     np.push_back( order[ j ] );
     }
    }
   if( np.empty() ) {
    if( _clique.size() > _best.size() ) {
     _best = _clique;
     }
    }
   else {
    expand( np );
    }
   _clique.pop_back();
   if( _stopped ) {
    return;
    }
   }
  }

 public:
 /// \brief Constructor
 ///
 /// Constructor.
 ///
 /// \param graph The undirected graph the algorithm runs on.
 explicit TomitaSekiMc( const GR & graph )
     : _graph( graph ) , _id( graph ) , _tree( 0 ) , _tree_limit( -1 ) ,
       _stopped( false ) { }

 /// \brief Set the limit on the nodes of the search tree
 ///
 /// This function sets the largest number of nodes of the search tree,
 /// i.e., of the cliques that the search extends, after which \ref run()
 /// stops; a negative one, the default, is no limit.
 TomitaSekiMc & treeLimit( long long limit ) {
  _tree_limit = limit;
  return( *this );
  }

 /// \brief Run the algorithm.
 ///
 /// \return \c true if the clique found is proved maximum, \c false if
 /// the limit on the search tree stopped the search first.
 bool run() {
  _nodes.clear();
  for( NodeIt n( _graph ) ; n != INVALID ; ++n ) {
   _id[ n ] = int( _nodes.size() );
   _nodes.push_back( n );
   }
  int n = int( _nodes.size() );
  _adj.assign( n , std::vector< char >( n , 0 ) );
  std::vector< int > degree( n , 0 );
  for( EdgeIt e( _graph ) ; e != INVALID ; ++e ) {
   int u = _id[ _graph.u( e ) ] , v = _id[ _graph.v( e ) ];
   if( u != v && ! _adj[ u ][ v ] ) {
    _adj[ u ][ v ] = _adj[ v ][ u ] = 1;
    ++degree[ u ];
    ++degree[ v ];
    }
   }
  // the candidates by nonincreasing degree, as Tomita and Seki order them
  std::vector< int > p( n );
  for( int i = 0 ; i < n ; ++i ) {
   p[ i ] = i;
   }
  std::stable_sort( p.begin() , p.end() , [ &degree ]( int a , int b ) {
   return( degree[ a ] > degree[ b ] );
   } );
  _clique.clear();
  _best.clear();
  _tree = 0;
  _stopped = false;
  if( n > 0 ) {
   expand( p );
   }
  return( ! _stopped );
  }

 /// \name Query Functions
 /// The results of the last \ref run().

 /// @{

 /// \brief The size of the clique found.
 int cliqueSize() const { return( int( _best.size() ) ); }

 /// \brief The number of nodes of the search tree of the last run.
 long long treeSize() const { return( _tree ); }

 /// \brief Gives back the clique found in a \c bool node map
 ///
 /// This function gives back the characteristic vector of the clique
 /// found in the given node map, which must be a
 /// \ref concepts::WriteMap "writable" node map with \c bool (or
 /// convertible) value type.
 template< typename CliqueMap >
 void cliqueMap( CliqueMap & map ) const {
  for( NodeIt n( _graph ) ; n != INVALID ; ++n ) {
   map[ n ] = false;
   }
  for( std::size_t i = 0 ; i < _best.size() ; ++i ) {
   map[ _nodes[ _best[ i ] ] ] = true;
   }
  }

 /// @}
 };

/// @}

}  //END OF NAMESPACE LEMON

#endif  //LEMON_TOMITA_SEKI_MC_H
