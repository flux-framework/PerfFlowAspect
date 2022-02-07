.. # Copyright 2021 Lawrence Livermore National Security, LLC and other
   # PerfFlowAspect Project Developers. See the top-level LICENSE file for
   # details.
   #
   # SPDX-License-Identifier: LGPL-3.0

##################
Build Instructions
##################

Python Build
------------

The minimum Python version needed is 3.6.

.. code:: bash

   python src/python/setup.py install


C Build
-------

Build Dependencies and Versions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

 ================ ================ =========== ======
   redhat          ubuntu           version     note
 ================ ================ =========== ======
   clang           clang            >= 6.0      *1*
   llvm-devel      llvm-dev         >= 6.0      *1*
   jansson-devel   libjansson-dev   >= 2.6
   openssl-devel   libssl-dev       >= 1.0.2
   cmake           cmake            >= 3.10
   flex            flex             >= 2.5.37
   bison           bison            >= 3.0.4
   make            make             >= 3.82
 ================ ================ =========== ======

.. note::

    If you use Clang >=9.0, there are two source code changes you need to make
    in `src/c/weaver/weave/perfflow_weave.cpp`.


Building PerfFlowAspect Annotation Parser and Runtime
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code:: bash

   cd src/c/parser
   make
   cd ../runtime
   make


Building PerfFlowAspect WeavePass LLVM Plugin
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

PerfFlowAspect WeavePass requires Clang and LLVM development packages as well
as a jansson-devel package for JSON manipulation. It also depends on
`src/c/parser/libperfflow_parser.so` so it additionally requires the
dependencies of our annotation parser code: i.e., `flex` and `bison`.

Note that LLVM_DIR must be set to the corresponding LLVM cmake directory which
may differ across different Linux distributions.

.. code:: bash

   cd ../weaver
   mkdir build
   cd build
   LLVM_DIR=/usr/lib/llvm-6.0/lib/cmake/llvm cmake ..
   make

