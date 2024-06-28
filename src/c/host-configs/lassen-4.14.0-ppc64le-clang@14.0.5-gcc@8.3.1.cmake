##############################################################
# Copyright 2021 Lawrence Livermore National Security, LLC
# (c.f. AUTHORS, NOTICE.LLNS, COPYING)
#
# This file is part of the Flux resource manager framework.
# For details, see https://github.com/flux-framework.
#
# SPDX-License-Identifier: LGPL-3.0
##############################################################

set(CMAKE_C_COMPILER "/usr/tce/packages/clang/clang-14.0.5-gcc-8.3.1/bin/clang" CACHE PATH "")
set(CMAKE_CXX_COMPILER "/usr/tce/packages/clang/clang-14.0.5-gcc-8.3.1/bin/clang++" CACHE PATH "")
set(LLVM_DIR "/usr/tce/packages/clang/clang-ibm-14.0.5-gcc-8.3.1/lib/cmake/llvm" CACHE PATH "")
# Use both C and CXX flags to support both targets.
set(CMAKE_C_FLAGS "-flegacy-pass-manager" CACHE STRING "") 
set(CMAKE_CXX_FLAGS "-flegacy-pass-manager" CACHE STRING "") 
