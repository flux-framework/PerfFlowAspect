..
   # Copyright 2021 Lawrence Livermore National Security, LLC and other
   # PerfFlowAspect Project Developers. See the top-level LICENSE file for
   # details.
   #
   # SPDX-License-Identifier: LGPL-3.0

##############################################
 Visualization of PerfFlowAspect Output Files
##############################################

There are two types of logging allowed in PerfFlowAspect trace files which are
``verbose`` and ``compact``. ``Verbose`` logging uses B (begin) and E (end)
events in the trace file as shown below:

.. code:: JSON

   {"name": "foo", "cat": "/PerfFlowAspect/src/c/test/smoketest.cpp", "pid": 3134, "tid": 3134, "ts": 1679127184455376.0, "ph": "B"},
   {"name": "bar", "cat": "/PerfFlowAspect/src/c/test/smoketest.cpp", "pid": 3134, "tid": 3134, "ts": 1679127184456525.0, "ph": "B"},
   {"name": "bas", "cat": "/PerfFlowAspect/src/c/test/smoketest.cpp", "pid": 3134, "tid": 3134, "ts": 1679127184457610.0, "ph": "B"},
   {"name": "bas", "cat": "/PerfFlowAspect/src/c/test/smoketest.cpp", "pid": 3134, "tid": 3134, "ts": 1679127184457636.0, "ph": "E"},
   {"name": "bar", "cat": "/PerfFlowAspect/src/c/test/smoketest.cpp", "pid": 3134, "tid": 3134, "ts": 1679127184457657.0, "ph": "E"},
   {"name": "foo", "cat": "/PerfFlowAspect/src/c/test/smoketest.cpp", "pid": 3134, "tid": 3134, "ts": 1679127184457676.0, "ph": "E"},

The above trace file is generated for three functions with ``around`` pointcut
annotations. The same trace file will be reduced to half the lines with
``compact`` logging, as can be seen below:

.. code:: JSON

   {"name": "bas", "cat": "/PerfFlowAspect/src/c/test/smoketest.cpp", "pid": 2688, "tid": 2688, "ts": 1679127137181517.0, "ph": "X", "dur": 600.0},
   {"name": "bar", "cat": "/PerfFlowAspect/src/c/test/smoketest.cpp", "pid": 2688, "tid": 2688, "ts": 1679127137179879.0, "ph": "X", "dur": 2885.0},
   {"name": "foo", "cat": "/PerfFlowAspect/src/c/test/smoketest.cpp", "pid": 2688, "tid": 2688, "ts": 1679127137177783.0, "ph": "X", "dur": 5532.0},

The visualization of both these trace files will be the same in Perfetto UI, as
shown below:

.. figure:: images/fig1.png
   :scale: 80%
   :align: center

Here is a caption.
