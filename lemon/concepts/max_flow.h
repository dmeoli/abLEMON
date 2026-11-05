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

#ifndef LEMON_CONCEPTS_MAX_FLOW_H
 #define LEMON_CONCEPTS_MAX_FLOW_H

#include <vector>

#include <lemon/concept_check.h>
#include <lemon/concepts/digraph.h>
#include <lemon/concepts/maps.h>

///\ingroup concept
///\file
///\brief The concept of the maximum flow algorithms.

namespace lemon
{

namespace concepts
{

/// \ingroup concept
///
/// \brief Checks the general interface of a max flow algorithm.
template< typename GR , typename CAP >
struct MaxFlow
{

 template< typename MF >
 struct Constraints
 {

  typedef typename GR::Node Node;
  typedef typename GR::Arc Arc;
  typedef typename CAP::Value Value;
  typedef concepts::ReadWriteMap< Arc , Value > FlowMap;
  typedef concepts::WriteMap< Node , bool > CutMap;

  GR g;
  Node n;
  Arc e;
  CAP cap;
  FlowMap flow;
  CutMap cut;
  Value v;
  bool b;

  void constraints() {
   checkConcept< concepts::Digraph , GR >();

   const Constraints & me = *this;

   typedef typename MF ::template SetFlowMap< FlowMap >::Create MaxFlowType;
   // MF::Create has to be a type
   typedef typename MF::Create MaxFlowType2 LEMON_UNUSED_TYPEDEF;
   MaxFlowType max_flow( me.g , me.cap , me.n , me.n );
   const MaxFlowType & const_max_flow = max_flow;

   max_flow.capacityMap( cap ).flowMap( flow ).source( n ).target( n );

   typename MaxFlowType::Tolerance tol = const_max_flow.tolerance();
   max_flow.tolerance( tol );

   max_flow.init();
   max_flow.init( cap );
   max_flow.run();

   v = const_max_flow.flowValue();
   v = const_max_flow.flow( e );
   const FlowMap & fm = const_max_flow.flowMap();

   b = const_max_flow.minCut( n );
   const_max_flow.minCutMap( cut );

   ::lemon::ignore_unused_variable_warning( fm );
   }
  };
 };

}  // namespace concepts

}  // namespace lemon

#endif  // LEMON_CONCEPTS_MAX_FLOW_H
