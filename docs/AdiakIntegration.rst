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
