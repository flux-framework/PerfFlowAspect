#! /usr/bin/python3

##############################################################
# Copyright 2021 Lawrence Livermore National Security, LLC
# (c.f. AUTHORS, NOTICE.LLNS, COPYING)
#
# This file is part of the Flux resource manager framework.
# For details, see https://github.com/flux-framework.
#
# SPDX-License-Identifier: LGPL-3.0
##############################################################

import os
import sys
import threading
import time
import json
import logging
import functools
import hashlib
import psutil
from urllib.parse import urlparse
from .aspect_base import perfflowaspect

# TODO: move those into ChromeTracingAdvice
counter_mutex = threading.Lock()
counter = 0
perfflow_options = {}


def cannonicalize_perfflow_options():
    if perfflow_options.get("name") is None:
        perfflow_options["name"] = "generic"
    if perfflow_options.get("log-filename-include") is None:
        perfflow_options["log-filename-include"] = "hostname,pid"
    if perfflow_options.get("log-dir") is None:
        perfflow_options["log-dir"] = "./"
    if perfflow_options.get("log-enable") is None:
        perfflow_options["log-enable"] = "True"
    if perfflow_options.get("cpu-mem-usage") is None:
        perfflow_options["cpu-mem-usage"] = "False"
    if perfflow_options.get("log-event") is None:
        perfflow_options["log-event"] = "Verbose"


def parse_perfflow_options():
    options_list = []
    options = os.getenv("PERFFLOW_OPTIONS")
    if options is not None:
        options_list = options.split(":")
    for opt in options_list:
        kv = opt.split("=")
        if len(kv) == 2:
            perfflow_options[kv[0]] = kv[1]
        else:
            print("Ill-formed option: {}".format(opt), file=sys.stderr)
    cannonicalize_perfflow_options()


def get_foreign_wm():
    foreign_job_id = os.getenv("SLURM_JOB_ID")
    if foreign_job_id is not None:
        return "slurm"
    foreign_job_id = os.getenv("LSB_JOBID")
    if foreign_job_id is not None:
        return "lsf"
    return ""


def get_uniq_id_from_foreign_wm():
    uniq_id = None
    foreign_wm = get_foreign_wm()
    if foreign_wm == "slurm":
        job_id = os.getenv("SLURM_JOB_ID")
        step_id = os.getenv("SLURM_STEP_ID")
        if job_id is not None and step_id is not None:
            uniq_id = job_id + "." + step_id
    elif foreign_wm == "lsf":
        job_id = os.getenv("LSB_JOBID")
        step_id = os.getenv("LS_JOBPID")
        if job_id is not None and step_id is not None:
            uniq_id = job_id + "." + step_id
    return uniq_id


def get_perfflow_instance_path():
    instance_id = ""
    flux_job_id = os.getenv("FLUX_JOB_ID")
    if flux_job_id is None:
        foreign_id = get_uniq_id_from_foreign_wm()
        if foreign_id is not None:
            hash = hashlib.sha1(foreign_id.encode("utf-8"))
            instance_id = hash.hexdigest()[0:8]
        else:
            uniq_id = os.getenv("FLUX_URI")
            if uniq_id is None:
                uniq_id = "1"
            else:
                uniq_id = urlparse(uniq_id).path
                uniq_id = os.path.dirname(uniq_id)
            uniq_id = hashlib.sha1(uniq_id.encode("utf-8"))
            instance_id = uniq_id.hexdigest()[0:8]
    else:
        instance_id = flux_job_id

    instance_path = os.getenv("PERFFLOW_INSTANCE_PATH")
    if instance_path is None:
        instance_path = instance_id
    else:
        instance_path = instance_path + "." + instance_id

    return instance_path


def set_perfflow_instance_path(path):
    os.environ["PERFFLOW_INSTANCE_PATH"] = path


@perfflowaspect
class ChromeTracingAdvice:
    """Chrome Tracing Advice Class: define pointcuts for this advice"""

    # PERFFLOW_OPTIONS envVar
    # To specify the name of this workflow component (default: name=generic)
    #     PERFFLOW_OPTIONS="name=foo"
    # To customize output filename (default: log-filename-include=hostname,pid)
    #     PERFFLOW_OPTIONS="log-filename-include=<metadata1, metadata2, ...>
    #         Supported metadata:
    #             name: workflow component name
    #             instance-path: hierarchical component path
    #             hostname: name of the host where this process is running
    #             pid: process id
    # To change the directory in which the log file is created
    #     PERFFLOW_OPTIONS="log-dir=DIR"
    # To disable logging (default: log-enable=True)
    #     PERFFLOW_OPTIONS="log-enable=False"
    # To collect CPU and memory usage metrics (default: cpu-mem-usage=False)
    #     PERFFLOW_OPTIONS="cpu-mem-usage=True"
    # To collect B (begin) and E (end) events as single X (complete) duration event (default: log-event=Verbose)
    #     PERFFLOW_OPTIONS="log-event=Compact"
    # You can combine the options in colon (:) delimited format

    parse_perfflow_options()
    inst_path = get_perfflow_instance_path()
    set_perfflow_instance_path(inst_path)

    fn = "perfflow"
    for inc in perfflow_options["log-filename-include"].split(","):
        if inc == "name":
            fn += "." + perfflow_options["name"]
        elif inc == "instance-path":
            fn += ".{" + inst_path + "}"
        elif inc == "hostname":
            fn += "." + os.uname()[1]
        elif inc == "pid":
            fn += "." + str(os.getpid())
        else:
            print(
                "perfflow warning: unknown option param={}".format(inc), file=sys.stderr
            )

    fn += ".pfw"

    log_dir = perfflow_options["log-dir"]
    if log_dir is not None:
        os.makedirs(log_dir, exist_ok=True)
        fn = os.path.join(log_dir, fn)

    log_enable = perfflow_options["log-enable"]
    if log_enable in ["True", "true", "TRUE"]:
        enable_logging = True
    elif log_enable in ["False", "false", "FALSE"]:
        enable_logging = False
    else:
        raise ValueError("perfflow invalid option: log-enable=[True|False]")

    cpu_mem_usage = perfflow_options["cpu-mem-usage"]
    if cpu_mem_usage in ["True", "true", "TRUE"]:
        enable_cpu_mem_usage = True
    elif cpu_mem_usage in ["False", "false", "FALSE"]:
        enable_cpu_mem_usage = False
    else:
        raise ValueError("perfflow invalid option: cpu-mem-usage=[True|False]")

    log_event = perfflow_options["log-event"]
    if log_event in ["Compact", "compact", "COMPACT"]:
        enable_compact_log_event = True
    elif log_event in ["Verbose", "verbose", "VERBOSE"]:
        enable_compact_log_event = False
    else:
        raise ValueError("perfflow invalid option: log-event=[Compact|Verbose]")

    logger = None

    def __init__(self):
        pass

    @staticmethod
    def __create_event(name, cat):
        return {
            "name": name,
            "cat": cat,
            "pid": os.getpid(),
            "tid": threading.get_native_id(),  # Needs Python 3.8
            "ts": time.time() * 1000000,
        }

    @classmethod
    def create_sync_event(cls, name, cat, ph):
        if ph not in ["B", "E"]:
            raise ValueError("Invalid Phase: {}".format(ph))
        event = cls.__create_event(name, cat)
        event["ph"] = ph
        return event

    @classmethod
    def create_async_event(cls, name, cat, ph, scope=None, id=None):
        if ph not in ["b", "n", "e", "s", "t", "f"]:
            raise ValueError("Invalid Phase: {}".format(ph))
        event = cls.__create_event(name, cat)
        event["ph"] = ph
        if id:
            event["id"] = id
        else:
            event["id"] = 123  # Async events need an ID, this sets the default.
        if scope:
            event["scope"] = scope
        return event

    sync_pointcut_phase_map = {"before": "B", "after": "E"}

    @classmethod
    def sync_event(cls, pointcut, *args, **kwargs):
        try:
            phase = cls.sync_pointcut_phase_map[pointcut]
        except KeyError:
            raise ValueError("Invalid pointcut: {}".format(pointcut))
        event = cls.create_sync_event(*args, phase, **kwargs)
        cls.__flush_log(json.dumps(event) + ",")

    async_pointcut_phase_map = {"before": "b", "instant": "n", "after": "e"}

    @classmethod
    def async_event(cls, pointcut, *args, **kwargs):
        try:
            phase = cls.async_pointcut_phase_map[pointcut]
        except KeyError:
            raise ValueError("Invalid pointcut: {}".format(pointcut))
        event = cls.create_async_event(*args, phase, **kwargs)
        cls.__flush_log(json.dumps(event) + ",")

    @classmethod
    def __create_event_from_func(cls, func):
        return cls.__create_event(func.__qualname__, func.__module__)

    @staticmethod
    def __flush_log(s):
        if ChromeTracingAdvice.logger is None:
            ChromeTracingAdvice.logger = logging.getLogger("perfflow")
            ChromeTracingAdvice.logger.setLevel(logging.DEBUG)
            ChromeTracingAdvice.logger.propagate = False
            fh = logging.FileHandler(ChromeTracingAdvice.fn)
            fh.setLevel(logging.DEBUG)
            formatter = logging.Formatter("%(message)s")
            fh.setFormatter(formatter)
            ChromeTracingAdvice.logger.addHandler(fh)
            ChromeTracingAdvice.logger.debug("[")
        ChromeTracingAdvice.logger.debug(s)

    @staticmethod
    def __update_log(func, event_type, event_args=None, event_ts=None, event_dur=None):
        global counter, counter_mutex
        counter_mutex.acquire()
        event = ChromeTracingAdvice.__create_event_from_func(func)
        event["ph"] = event_type
        if event_args is not None:
            event["args"] = event_args
        if event_ts is not None:
            event["ts"] = event_ts
        if event_dur is not None:
            event["dur"] = event_dur
        ChromeTracingAdvice.__flush_log(json.dumps(event) + ",")
        counter = counter + 1
        counter_mutex.release()

    @staticmethod
    def __update_async_log(
        scope, func, event_type, event_args=None, event_ts=None, event_dur=None
    ):
        global counter, counter_mutex
        counter_mutex.acquire()
        event = ChromeTracingAdvice.__create_event_from_func(func)
        event["scope"] = scope
        event["id"] = event["tid"]
        event["ph"] = event_type
        if event_args is not None:
            event["args"] = event_args
        if event_ts is not None:
            event["ts"] = event_ts
        if event_dur is not None:
            event["dur"] = event_dur
        ChromeTracingAdvice.__flush_log(json.dumps(event) + ",")
        counter = counter + 1
        counter_mutex.release()

    @staticmethod
    def before(func):
        @functools.wraps(func)
        def trace(*args, **kwargs):
            ChromeTracingAdvice.__update_log(func, "B")
            return func(*args, **kwargs)

        return trace

    @staticmethod
    def after(func):
        @functools.wraps(func)
        def trace(*args, **kwargs):
            rc = func(*args, **kwargs)
            ChromeTracingAdvice.__update_log(func, "E")
            return rc

        return trace

    @staticmethod
    def around(func):
        @functools.wraps(func)
        def trace(*args, **kwargs):
            # Obtain start timestamp for tracing consistency.
            ts_start = time.time() * 1000000

            if not ChromeTracingAdvice.enable_compact_log_event:
                ChromeTracingAdvice.__update_log(func, "B", event_ts=ts_start)

            if ChromeTracingAdvice.enable_cpu_mem_usage:
                p = psutil.Process(os.getpid())
                cpu_start = p.cpu_times()
                cpu_start = cpu_start[0]
                time_start = time.time()

            rc = func(*args, **kwargs)

            # Obtain end timestamp to calculate durations.
            ts_end = time.time() * 1000000

            if ChromeTracingAdvice.enable_cpu_mem_usage:
                time_end = time.time() - time_start
                cpu_end = p.cpu_times()
                cpu_end = cpu_end[0]
                cpu_end = cpu_end - cpu_start
                cpu_usage = (cpu_end / time_end) * 100
                mem_usage = p.memory_info().rss
                if mem_usage > 0:
                    mem_usage = mem_usage / 1000
                # Update trace with CPU and memory usage information.
                ev_args = {"cpu_usage": cpu_usage, "memory_usage": mem_usage}
                ChromeTracingAdvice.__update_log(
                    func,
                    "C",
                    event_ts=ts_start,
                    event_args=ev_args,
                )

            if ChromeTracingAdvice.enable_cpu_mem_usage:
                # We need to write the zero values to the trace for correct Perfetto Visualization.
                ev_args = {"cpu_usage": 0, "memory_usage": 0}
                ChromeTracingAdvice.__update_log(
                    func,
                    "C",
                    event_ts=ts_end,
                    event_args=ev_args,
                )

            if ChromeTracingAdvice.enable_compact_log_event:
                dur = ts_end - ts_start
                ChromeTracingAdvice.__update_log(
                    func,
                    "X",
                    event_ts=ts_start,
                    event_dur=dur,
                )
            else:
                ChromeTracingAdvice.__update_log(func, "E", event_ts=ts_end)
            return rc

        return trace

    @staticmethod
    def before_async(scope):
        def before_async_(func):
            @functools.wraps(func)
            def trace(*args, **kwargs):
                ChromeTracingAdvice.__update_async_log(scope, func, "b")
                return func(*args, **kwargs)

            return trace

        return before_async_

    @staticmethod
    def after_async(scope):
        def after_async_(func):
            @functools.wraps(func)
            def trace(*args, **kwargs):
                rc = func(*args, **kwargs)
                ChromeTracingAdvice.__update_async_log(scope, func, "e")
                return rc

            return trace

        return after_async_

    @staticmethod
    def around_async(func):
        @functools.wraps(func)
        def trace(*args, **kwargs):
            scope = None
            ChromeTracingAdvice.__update_async_log(scope, func, "b")
            rc = func(*args, **kwargs)
            ChromeTracingAdvice.__update_async_log(scope, func, "e")
            return rc

        return trace


#
# vi: ts=4 sw=4 expandtab
#
