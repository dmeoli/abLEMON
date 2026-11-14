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

// The skeletons of the LP and MIP interfaces, built without any solver: the
// part of the interface that LpBase implements on top of them

#include <lemon/lp_skeleton.h>
#include "test_tools.h"

using namespace lemon;

int main()
{
 // MipSkeleton implements every pure virtual member of MipSolver
 {
  MipSkeleton mip;
  MipSkeleton::Col x = mip.addCol();
  mip.colType( x , MipSkeleton::INTEGER );
  check( mip.colType( x ) == MipSkeleton::REAL ,
	 "MipSkeleton keeps no column type" );
  check( mip.solve() == MipSkeleton::SOLVED , "MipSkeleton did not solve" );
  }

 return( 0 );
 }
