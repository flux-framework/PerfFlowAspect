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

Host Config Files
^^^^^^^^^^^^^^^^^

To handle build options, third-party library paths, and other
environment-specific configurations, PerfFlowAspect relies on CMake's
initial-cache file mechanism.

These initial-cache files are called host-config files in PerfFlowAspect, since
we typically create a file for each platform or specific system if necessary.

Example configuration files can be found in the ``host-configs/`` directory.
Assuming you are in a ``build/`` directory, you can call the host-config file
as follows:

.. code:: bash

   $ cmake -C host-configs/{config_file}.cmake ../

Build Dependencies and Versions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

 ================ ================ =========== ================
   redhat          ubuntu           version     note
 ================ ================ =========== ================
   clang           clang            >= 6.0      see note below
   llvm-devel      llvm-dev         >= 6.0      see note below
   jansson-devel   libjansson-dev   >= 2.6
   openssl-devel   libssl-dev       >= 1.0.2
   cmake           cmake            >= 3.10
   flex            flex             >= 2.5.37
   bison           bison            >= 3.0.4
   make            make             >= 3.82
 ================ ================ =========== ================

.. note::

    If you use Clang >=9.0, there are two source code changes you need to make
    in ``src/c/weaver/weave/perfflow_weave.cpp``.


Building PerfFlowAspect Annotation Parser and Runtime
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. code:: bash

   $ cd src/c/parser
   $ make
   $ cd ../runtime
   $ make


Building PerfFlowAspect WeavePass LLVM Plugin
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

PerfFlowAspect WeavePass requires Clang and LLVM development packages as well
as a ``jansson-devel`` package for JSON manipulation. It also depends on
``src/c/parser/libperfflow_parser.so`` so it additionally requires the
dependencies of our annotation parser code: i.e., ``flex`` and ``bison``.

Note that ``LLVM_DIR`` must be set to the corresponding LLVM cmake directory
which may differ across different Linux distributions.

.. code:: bash

   $ cd ../weaver
   $ mkdir build
   $ cd build
   $ LLVM_DIR=/usr/lib/llvm-6.0/lib/cmake/llvm cmake ..
   $ make

