#!/bin/bash

set -x 

rm perfflow.smktst*

PERFFLOW_OPTIONS="name=smktst:log-filename-include=name" ./smoketest.py
PERFFLOW_OPTIONS="name=smktst_dt:log-filename-include=name" ./smoketest_direct.py
PERFFLOW_OPTIONS="name=smktst_multi_thd:log-filename-include=name" ./smoketest_multithreading.py 
PERFFLOW_OPTIONS="name=smktst_multi_proc:log-filename-include=name" ./smoketest_multiprocessing.py 
PERFFLOW_OPTIONS="name=smktst_future:log-filename-include=name" ./smoketest_future.py 
PERFFLOW_OPTIONS="name=smktst_future2:log-filename-include=name" ./smoketest_future2.py 
PERFFLOW_OPTIONS="name=smktst_future_dt:log-filename-include=name" ./smoketest_future_direct.py 

