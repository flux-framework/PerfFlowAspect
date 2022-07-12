.. # Copyright 2021 Lawrence Livermore National Security, LLC and other
   # PerfFlowAspect Project Developers. See the top-level LICENSE file for
   # details.
   #
   # SPDX-License-Identifier: LGPL-3.0

##################
Build Instructions
##################

Python Install
--------------

The minimum Python version needed is 3.6. You can get PerfFlowAspect from its
GitHub repository using this command:

.. code:: bash

   $ git clone https://github.com/flux-framework/PerfFlowAspect

This will create a directory called ``PerfFlowAspect``.

To use PerfFlowAspect, you will need to update your ``PYTHONPATH`` with the
path to the PerfFlowAspect python directory:

.. code:: bash

   $ cd src/python
   $ export PYTHONPATH=$PWD:$PYTHONPATH


C Build
-------

Build Dependencies and Versions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

 ================ ================ ===========
   redhat          ubuntu           version   
 ================ ================ ===========
   clang           clang            >= 6.0
   llvm-devel      llvm-dev         >= 6.0
   jansson-devel   libjansson-dev   >= 2.6
   openssl-devel   libssl-dev       >= 1.0.2
   cmake           cmake            >= 3.10
   flex            flex             >= 2.5.37
   bison           bison            >= 3.0.4
   make            make             >= 3.82
 ================ ================ ===========

Building PerfFlowAspect
^^^^^^^^^^^^^^^^^^^^^^^

PerfFlowAspect uses CMake and requires Clang and LLVM development packages as well
as a ``jansson-devel`` package for JSON manipulation. It additionally requires the
dependencies of our annotation parser code: i.e., ``flex`` and ``bison``. Note that 
``LLVM_DIR`` must be set to the corresponding LLVM cmake directory which may 
differ across different Linux distributions.

.. code:: bash

    $ module load clang/10.0.1-gcc-8.3.1 (on LLNL systems only)
    $ cd PerfFlowAspect/src/c
    $ mkdir build && cd build
    $ cmake -DCMAKE_CXX_COMPILER=clang++ ../
    $ make (note: parallel make (make -j) not supported yet)
    
    $ find . -print | grep lib # successful build produces 3 libraries
    ./build/parser/libperfflow_parser.so
    ./build/runtime/libperfflow_runtime.so
    ./build/weaver/weave/libWeavePass.so   
