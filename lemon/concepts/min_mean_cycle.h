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

#ifndef LEMON_CONCEPTS_MIN_MEAN_CYCLE_H
 #define LEMON_CONCEPTS_MIN_MEAN_CYCLE_H

#include <vector>

#include <lemon/concept_check.h>
#include <lemon/concepts/digraph.h>
#include <lemon/concepts/maps.h>
#include <lemon/path.h>

///\ingroup concept
///\file
///\brief The concept of the minimum mean cycle algorithms.

namespace lemon
{

namespace concepts
{

/// \ingroup concept
///
/// \brief Check the interface of an MMC algorithm.
template< typename GR , typename Cost >
struct MinMeanCycle
{
 template< typename MMC >
 struct Constraints
 {
  void constraints() {
   const Constraints & me = *this;

   typedef typename MMC ::template SetPath<
     ListPath< GR > >::template SetLargeCost< Cost >::Create MmcAlg;
   MmcAlg mmc( me.g , me.cost );
   const MmcAlg & const_mmc = mmc;

   typename MmcAlg::Tolerance tol = const_mmc.tolerance();
   mmc.tolerance( tol );

   b = mmc.cycle( p ).run();
   b = mmc.findCycleMean();
   b = mmc.findCycle();

   v = const_mmc.cycleCost();
   i = const_mmc.cycleSize();
   d = const_mmc.cycleMean();
   p = const_mmc.cycle();
   }

  typedef concepts::ReadMap< typename GR::Arc , Cost > CM;

  GR g;
  CM cost;
  ListPath< GR > p;
  Cost v;
  int i;
  double d;
  bool b;
  };
 };

}  // namespace concepts

}  // namespace lemon

#endif  // LEMON_CONCEPTS_MIN_MEAN_CYCLE_H
