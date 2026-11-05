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

#ifndef LEMON_CONCEPTS_MIN_COST_FLOW_H
 #define LEMON_CONCEPTS_MIN_COST_FLOW_H

#include <vector>

#include <lemon/concept_check.h>
#include <lemon/concepts/digraph.h>
#include <lemon/concepts/maps.h>

///\ingroup concept
///\file
///\brief The concept of the minimum cost flow algorithms.

namespace lemon
{

namespace concepts
{

/// \ingroup concept
///
/// \brief Check the interface of an MCF algorithm.
template< typename GR , typename Value , typename Cost >
class MinCostFlow
{
 public:
 template< typename MCF >
 struct Constraints
 {
  void constraints() {
   checkConcept< concepts::Digraph , GR >();

   const Constraints & me = *this;

   MCF mcf( me.g );
   const MCF & const_mcf = mcf;

   b = mcf.reset()
	 .resetParams()
	 .lowerMap( me.lower )
	 .upperMap( me.upper )
	 .costMap( me.cost )
	 .supplyMap( me.sup )
	 .stSupply( me.n , me.n , me.k )
	 .run();

   c = const_mcf.totalCost();
   x = const_mcf.template totalCost< double >();
   v = const_mcf.flow( me.a );
   c = const_mcf.potential( me.n );
   const_mcf.flowMap( fm );
   const_mcf.potentialMap( pm );
   mcf.tolerances( v , c );
   typename MCF::FlowTolerance ft = const_mcf.flowTolerance();
   typename MCF::CostTolerance ct = const_mcf.costTolerance();
   ignore_unused_variable_warning( ft , ct );
   std::vector< Node > cut = const_mcf.infeasibleCut();
   std::vector< Arc > cycle = const_mcf.unboundedCycle();
   ignore_unused_variable_warning( cut , cycle );
   }

  typedef typename GR::Node Node;
  typedef typename GR::Arc Arc;
  typedef concepts::ReadMap< Node , Value > NM;
  typedef concepts::ReadMap< Arc , Value > VAM;
  typedef concepts::ReadMap< Arc , Cost > CAM;
  typedef concepts::WriteMap< Arc , Value > FlowMap;
  typedef concepts::WriteMap< Node , Cost > PotMap;

  GR g;
  VAM lower;
  VAM upper;
  CAM cost;
  NM sup;
  Node n;
  Arc a;
  Value k;

  FlowMap fm;
  PotMap pm;
  bool b;
  double x;
  typename MCF::Value v;
  typename MCF::Cost c;
  };
 };

}  // namespace concepts

}  // namespace lemon

#endif  // LEMON_CONCEPTS_MIN_COST_FLOW_H
