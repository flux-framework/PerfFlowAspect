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
from urllib.parse import urlparse
from .aspect_base import perfflowaspect

# TODO: move those into ChromeTracingAdvice
before_counter_mutex = threading.Lock()
after_counter_mutex = threading.Lock()
counter_mutex = threading.Lock()
before_counter = 0
after_counter = 0
counter = 0
perfflow_options = {}


def cannonicalize_perfflow_options():
    if perfflow_options.get("name") is None:
        perfflow_options["name"] = "generic"
    if perfflow_options.get("log-filename-include") is None:
        perfflow_options["log-filename-include"] = "hostname,pid"
    if perfflow_options.get("log-dir") is None:
        perfflow_options["log-dir"] = "./"


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

    logger = None

    def __init__(self):
        pass

    @staticmethod
    def __create_event(name, cat):
        return {
            "name": name,
            "cat": cat,
            "pid": os.getpid(),
            "tid": threading.get_ident(),
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
    def before(func):
        @functools.wraps(func)
        def trace(*args, **kwargs):
            event = ChromeTracingAdvice.__create_event_from_func(func)
            event["ph"] = "B"
            ChromeTracingAdvice.__flush_log(json.dumps(event) + ",")
            return func(*args, **kwargs)

        return trace

    @staticmethod
    def after(func):
        @functools.wraps(func)
        def trace(*args, **kwargs):
            rc = func(*args, **kwargs)
            event = ChromeTracingAdvice.__create_event_from_func(func)
            event["ph"] = "E"
            ChromeTracingAdvice.__flush_log(json.dumps(event) + ",")
            return rc

        return trace

    @staticmethod
    def around(func):
        @functools.wraps(func)
        def trace(*args, **kwargs):
            event = ChromeTracingAdvice.__create_event_from_func(func)
            event["ph"] = "B"
            ChromeTracingAdvice.__flush_log(json.dumps(event) + ",")
            rc = func(*args, **kwargs)
            event["ts"] = time.time() * 1000000
            event["ph"] = "E"
            ChromeTracingAdvice.__flush_log(json.dumps(event) + ",")
            return rc

        return trace

    @staticmethod
    def before_async(scope):
        def before_async_(func):
            @functools.wraps(func)
            def trace(*args, **kwargs):
                global before_counter, before_counter_mutex
                before_counter_mutex.acquire()
                event = ChromeTracingAdvice.__create_event_from_func(func)
                event["scope"] = scope
                event["id"] = before_counter
                event["ph"] = "b"
                before_counter = before_counter + 1
                before_counter_mutex.release()
                ChromeTracingAdvice.__flush_log(json.dumps(event) + ",")
                return func(*args, **kwargs)

            return trace

        return before_async_

    @staticmethod
    def after_async(scope):
        def after_async_(func):
            @functools.wraps(func)
            def trace(*args, **kwargs):
                global after_counter, after_counter_mutex
                after_counter_mutex.acquire()
                event = ChromeTracingAdvice.__create_event_from_func(func)
                event["scope"] = scope
                event["id"] = after_counter
                event["ph"] = "e"
                after_counter = after_counter + 1
                after_counter_mutex.release()
                ChromeTracingAdvice.__flush_log(json.dumps(event) + ",")
                return func(*args, **kwargs)

            return trace

        return after_async_

    @staticmethod
    def around_async(func):
        @functools.wraps(func)
        def trace(*args, **kwargs):
            global counter, counter_mutex
            counter_mutex.acquire()
            event = ChromeTracingAdvice.__create_event_from_func(func)
            event["id"] = 8192
            event["ph"] = "b"
            counter = counter + 1
            counter_mutex.release()
            ChromeTracingAdvice.__flush_log(json.dumps(event) + ",")
            rc = func(*args, **kwargs)
            event["ts"] = time.time() * 1000000
            event["ph"] = "e"
            ChromeTracingAdvice.__flush_log(json.dumps(event) + ",")
            return rc

        return trace


#
# vi: ts=4 sw=4 expandtab
#
