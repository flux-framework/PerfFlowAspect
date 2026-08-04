..
   # Copyright 2021 Lawrence Livermore National Security, LLC and other
   # PerfFlowAspect Project Developers. See the top-level LICENSE file for
   # details.
   #
   # SPDX-License-Identifier: LGPL-3.0

###################
 Adiak Integration
###################

PerfFlowAspect can be built with Adiak, a tool that collects metadata on HPC
runs. Learn more about Adiak `here <https://github.com/LLNL/Adiak>`_.

PerfFlowAspect integration with Adiak is currently only supported in C/C++. When
integrated, the generated PerfFlowAspect `.pfw` trace file contains Adiak
metadata. `Note: the trace file must be generated in object format.`

Adiak must be installed prior to PerfFlowAspect integration.

*************
 C/C++ Build
*************

Adiak is enabled by specifying ``PERFFLOWASPECT_WITH_ADIAK=On`` along with the
path to Adiak's package configuration file, i.e.
``<installpath>/lib/cmake/adiak>``.

.. code:: bash

   cmake -DCMAKE_C_COMPILER=<path-to-clang20-compiler> \
         -DCMAKE_CXX_COMPILER=<path-to-clang20++-compiler> \
         -DPERFFLOWASPECT_WITH_ADIAK=On \
         -Dadiak_DIR=<installpath>/lib/cmake/adiak> ../

Adiak can gather additional metadata related to MPI. The flag
``PERFFLOWASPECT_WITH_MPI=On`` must be specified.

.. code:: bash

   cmake -DCMAKE_C_COMPILER=<path-to-clang20-compiler> \
         -DCMAKE_CXX_COMPILER=<path-to-clang20++-compiler> \
         -DPERFFLOWASPECT_WITH_ADIAK=On \
         -Dadiak_DIR=<installpath>/lib/cmake/adiak> \
         -DPERFFLOWASPECT_WITH_MPI=On ../

***************
 C/C++ Example
***************

Adiak's metadata can only be displayed in the object format of a PerfFlowAspect
`.pfw` trace. Thus, ``PERFFLOW_OPTIONS="log-format=Object"`` must be specified

.. code:: bash

   PERFFLOW_OPTIONS="log-format=Object" ./smoketest

**********************
 Output Demonstration
**********************

The following is a snippet of running the CUDA smoketest on LLNL's Matrix
Cluster. The Adiak metadata appears in the Chrome Trace Format otherData
section.

.. code:: json

   {
   "displayTimeUnit": "us",
   "otherData": {
      "walltime": {"tv_sec": 0, "tv_usec": 903734},
      "systime": {"tv_sec": 0, "tv_usec": 370000},
      "cputime": {"tv_sec": 0, "tv_usec": 40000},
      "numhosts": 1,
      "jobsize": 1,
      "cluster": "matrix",
      "hostname": "matrix10",
      "working_directory": "/g/g14/greene36/PerfFlowAspect/src/c/build/test",
      "executablepath": "/g/g14/greene36/PerfFlowAspect/src/c/build/test/smoketest_cuda",
      "executable": "smoketest_cuda",
      "launchday": 1761091200,
      "launchdate": 1761148496,
      "uid": "Spencer Greene",
      "user": "greene36",
      "adiakversion": "0.4.1"

   },
   "traceEvents": [
   {"name": "main", "cat": "/g/g14/greene36/PerfFlowAspect/src/c/test/smoketest_cuda_wrapper.cpp", "pid": 3220242, "tid": 3220242, "ts": 1761148496078466.0, "ph": "B"},
   {"name": "main", "cat": "/g/g14/greene36/PerfFlowAspect/src/c/test/smoketest_cuda_wrapper.cpp", "pid": 3220242, "tid": 3220242, "ts": 1761148496968874.0, "ph": "E"}, ...
