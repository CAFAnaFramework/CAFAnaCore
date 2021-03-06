/// StanVar.h -- just the stan::math::var header
///  (They trigger some warnings that we want to suppress,
///   so centralize all the stuff turning them off into this one spot.)

#pragma once

#ifdef EIGEN_MAJOR_VERSION
#ifndef STAN_MATH_PRIM_FUN_EIGEN_HPP
#error "It is strongly advised not to include Eigen headers before stan_math headers, please rearrange your includes"
#endif
#endif

// Disable various pedantic warnings that stan triggers
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#pragma GCC diagnostic ignored "-Wignored-qualifiers"

// Supress a deprecation warning from tbb/task_scheduler_init.h that otherwise
// gets triggered a lot. Hopefully stan updates before tbb does.
#define __TBB_show_deprecation_message_task_scheduler_init_H

// stan::math::var itself
#include "stan/math/rev/core/var.hpp"

// Required for Eigen
#include "stan/math/rev/core/typedefs.hpp"

#pragma GCC diagnostic pop

// TODO retrieve any of the preprocessor stuff below we might need in practice,
// otherwise delete.
/*

// Stan's dependencies trigger some warnings. Save the warning state and then
// adjust it to ignore them.
#pragma GCC diagnostic push

// Only need to turn off this warning for higher versions of gcc
#if __GNUC__ >= 6
#pragma GCC diagnostic ignored "-Wmisleading-indentation"
#endif

#pragma GCC diagnostic ignored "-Wunused-function"

// Stan's var.hpp drags in a bunch of boost, but all it wants to do is test for
// __float128 which we don't have in any case.

// If the header hasn't been included
#ifndef BOOST_MATH_TOOLS_CONFIG_HPP
// Then set its include guard so its inclusion is ignored
#define BOOST_MATH_TOOLS_CONFIG_HPP
#include "stan/math/rev/core/var.hpp"
// Then put things back how they were
#undef BOOST_MATH_TOOLS_CONFIG_HPP
#else
// If it was included, let's not mess with it
#include "stan/math/rev/core/var.hpp"
#endif

// Return the warning state to how it started
#pragma GCC diagnostic pop
*/
