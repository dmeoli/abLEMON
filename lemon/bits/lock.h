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

#ifndef LEMON_BITS_LOCK_H
 #define LEMON_BITS_LOCK_H

#include <mutex>

namespace lemon
{
namespace bits
{
// The lock of the environment shared by the copies of a CPLEX solver: a
// std::mutex, with the names that the solver calls
class Lock
{
 public:
 void lock() { _lock.lock(); }
 void unlock() { _lock.unlock(); }

 private:
 std::mutex _lock;
 };
}
}

#endif
