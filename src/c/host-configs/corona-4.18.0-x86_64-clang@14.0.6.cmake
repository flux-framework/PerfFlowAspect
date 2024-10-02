##############################################################
# Copyright 2021 Lawrence Livermore National Security, LLC
# (c.f. AUTHORS, NOTICE.LLNS, COPYING)
#
# This file is part of the Flux resource manager framework.
# For details, see https://github.com/flux-framework.
#
# SPDX-License-Identifier: LGPL-3.0
##############################################################

set(CMAKE_C_COMPILER "/usr/tce/packages/clang/clang-14.0.6/bin/clang" CACHE PATH "")
set(CMAKE_CXX_COMPILER "/usr/tce/packages/clang/clang-14.0.6/bin/clang++" CACHE PATH "")
set(LLVM_DIR "/usr/tce/packages/clang/clang-14.0.6/lib/cmake/llvm" CACHE PATH "")

# Clang 14 supports both legacy and new pass manager.
# Use the -flegacy-pass-manager if you want to use the legacy pass.
# Use -fpass-plugin=<weavePass.so> to use the new pass
# Use both C and CXX flags to support both targets.
 set(CMAKE_C_FLAGS "-flegacy-pass-manager" CACHE STRING "")
 set(CMAKE_CXX_FLAGS "-flegacy-pass-manager" CACHE STRING "")
 set(PERFFLOWASPECT_WITH_CUDA "OFF" CACHE STRING "")