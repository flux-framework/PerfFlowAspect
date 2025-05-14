..
   # Copyright 2021 Lawrence Livermore National Security, LLC and other
   # PerfFlowAspect Project Developers. See the top-level LICENSE file for
   # details.
   #
   # SPDX-License-Identifier: LGPL-3.0

#####################
 Caliper Integration
#####################

PerfFlowAspect can be built with Caliper to leverage collection of additional
performance data, such as hardware performance counters on CPUs and GPU
measurements on NVIDIA GPUs. `Caliper <https://github.com/llnl/caliper>`_ is an
instrumentation and performance annotation library. A Caliper install is
required before building PerfFlowAspect:

.. code:: bash

   cmake -Dcaliper_DIR=<path-to-install>/share/lib/caliper ../

Caliper can be configured at runtime, for example:

.. code:: bash

   CALI_CONFIG=runtime-report ./smoketest
   CALI_CONFIG=runtime-report,output=test.cali ./smoketest

.. code:: bash

   Path        Min time/rank Max time/rank Avg time/rank Time %
   _Z3fooRKSs       0.004527      0.004527      0.004527 45.778137
     _Z3barv        0.004511      0.004511      0.004511 45.616341
       _Z3basv      0.000079      0.000079      0.000079  0.798867

.. code:: bash

   CALI_PAPI_COUNTERS=PAPI_TOT_CYC,PAPI_L2_DCM CALI_SERVICES_ENABLE=event,trace,papi,report ./smoketest

.. code:: bash

   event.begin#annotation papi.PAPI_TOT_CYC papi.PAPI_L2_DCM annotation                 region.count event.end#annotation
   _Z3fooRKSs                        118289              679
   _Z3barv                           200050              765 _Z3fooRKSs
   _Z3basv                           115098              352 _Z3fooRKSs/_Z3barv
                                      66564              242 _Z3fooRKSs/_Z3barv/_Z3basv            1 _Z3basv
                                     117061              385 _Z3fooRKSs/_Z3barv                    1 _Z3barv
                                      93592              206 _Z3fooRKSs                            1 _Z3fooRKSs
   _Z3fooRKSs                        146308              332
   _Z3barv                            87811              255 _Z3fooRKSs
   _Z3basv                            84904              244 _Z3fooRKSs/_Z3barv
                                      34547               66 _Z3fooRKSs/_Z3barv/_Z3basv            1 _Z3basv
                                      82540              168 _Z3fooRKSs/_Z3barv                    1 _Z3barv
                                      80711              144 _Z3fooRKSs                            1 _Z3fooRKSs
   _Z3fooRKSs                        127765              183
   _Z3barv                            85440              241 _Z3fooRKSs
   _Z3basv                            82100              250 _Z3fooRKSs/_Z3barv
                                      33969               67 _Z3fooRKSs/_Z3barv/_Z3basv            1 _Z3basv
                                      81511              161 _Z3fooRKSs/_Z3barv                    1 _Z3barv
                                      77498              128 _Z3fooRKSs                            1 _Z3fooRKSs
   _Z3fooRKSs                        119853              164
   _Z3barv                            83285              227 _Z3fooRKSs
   _Z3basv                            82702              297 _Z3fooRKSs/_Z3barv
                                      34170               78 _Z3fooRKSs/_Z3barv/_Z3basv            1 _Z3basv
                                      81589              149 _Z3fooRKSs/_Z3barv                    1 _Z3barv
                                      78920              119 _Z3fooRKSs                            1 _Z3fooRKSs

.. code:: bash

   PERFFLOW_OPTIONS='cpu-mem-usage=True:log-event=compact' CALI_CONFIG="load(time_exclusive.json),spot" ./smoketest

.. code:: bash

   Path        Min time/rank Max time/rank Avg time/rank Total time spot.channel
   _Z3fooRKSs       0.018068      0.018068      0.018068   0.018068 regionprofile
     _Z3barv        0.009124      0.009124      0.009124   0.009124 regionprofile
       _Z3basv      0.000074      0.000074      0.000074   0.000074 regionprofile
