##############################################################
# Copyright 2021 Lawrence Livermore National Security, LLC
# (c.f. AUTHORS, NOTICE.LLNS, COPYING)
#
# This file is part of the Flux resource manager framework.
# For details, see https://github.com/flux-framework.
#
# SPDX-License-Identifier: LGPL-3.0
##############################################################

set(CMAKE_C_COMPILER "/opt/rocm-6.3.1/lib/llvm/bin/amdclang" CACHE PATH "")
set(CMAKE_CXX_COMPILER "/opt/rocm-6.3.1/lib/llvm/bin/amdclang++" CACHE PATH "")

# To enable fine-grained profiling with Caliper, utilize the below options.
# Caliper should be built with the same compilers as the PerfFlowAspect.
# set(PERFFLOWASPECT_WITH_CALIPER ON CACHE BOOL "") 
# set(caliper_DIR "path-to-caliper-install-directory" CACHE PATH "") 
