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

#ifndef LEMON_BITS_MCF_TOLERANCE_H
 #define LEMON_BITS_MCF_TOLERANCE_H

#include <limits>

//\ingroup min_cost_flow_algs
//\file
//\brief The default tolerances of the minimum cost flow algorithms.
//
//The one definition of the tolerances that NetworkSimplex, CostScaling,
//CapacityScaling and CycleCanceling, their certificates and
//KBestExtremeFlows use when the caller sets none [see their
//tolerances()]: zero for an exact type, so that on integer data every
//comparison is exact, and otherwise \f$10^6\f$ times the machine epsilon of
//the type times the largest absolute value of the data, at least 1. The
//data of the flows are the supplies and the finite lower and upper bounds,
//those of the costs the costs of the arcs.

namespace lemon
{

namespace _mcf_bits
{

// The largest absolute value of the finite data it is given, at least 1
template< typename T >
class Largest
{
 public:
 Largest() : _m( 1 ) { }

 void add( T v ) {
  if( v < 0 )
   v = -v;
  if( v > _m && v < ( std::numeric_limits< T >::max )() &&
      ( ! std::numeric_limits< T >::has_infinity ||
	v != std::numeric_limits< T >::infinity() ) )
   _m = v;
  }

 T value() const { return( _m ); }

 private:
 T _m;
 };

// The default tolerance of a datum of type T whose data have largest
// absolute value big [see Largest]
template< typename T >
T dataTolerance( T big )
{
 if( std::numeric_limits< T >::is_exact )
  return( 0 );
 return(
   static_cast< T >( big * std::numeric_limits< T >::epsilon() * 1e6 ) );
 }

}  // namespace _mcf_bits

}  // namespace lemon

#endif  // LEMON_BITS_MCF_TOLERANCE_H
