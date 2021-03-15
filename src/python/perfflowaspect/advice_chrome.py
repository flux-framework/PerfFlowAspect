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
import threading
import time
import json
import logging
import functools
from .aspect_base import perfflowaspect

# TODO: move those into ChromeTracingAdvice
before_counter_mutex = threading.Lock()
after_counter_mutex = threading.Lock()
counter_mutex = threading.Lock()
before_counter = 0
after_counter = 0
counter = 0


@perfflowaspect
class ChromeTracingAdvice:
    """ Chrome Tracing Advice Class: define pointcuts for this advice """

    # TODO: add support for PERFLOW_OPTIONS
    # TODO: especially PERFLOW_OPTIONS="log_file=my_name.log"
    # TODO: support for TOML config
    fn = "perfflow.out" + os.uname()[1] + "." + str(os.getpid()) + ".pfw"
    logger = None

    def __init__(self):
        pass

    @staticmethod
    def __create_event(func):
        return {
            "name": func.__name__,
            "cat": func.__module__,
            "pid": os.getpid(),
            "tid": threading.get_ident(),
            "ts": time.time() * 1000000,
        }

    @staticmethod
    def __flush_log(s):
        if ChromeTracingAdvice.logger == None:
            ChromeTracingAdvice.logger = logging.getLogger("perfflow")
            ChromeTracingAdvice.logger.setLevel(logging.DEBUG)
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
            event = ChromeTracingAdvice.__create_event(func)
            event["ph"] = "B"
            ChromeTracingAdvice.__flush_log(json.dumps(event) + ",")
            return func(*args, **kwargs)

        return trace

    @staticmethod
    def after(func):
        @functools.wraps(func)
        def trace(*args, **kwargs):
            rc = func(*args, **kwargs)
            event = ChromeTracingAdvice.__create_event(func)
            event["ph"] = "E"
            ChromeTracingAdvice.__flush_log(json.dumps(event) + ",")
            return rc

        return trace

    @staticmethod
    def around(func):
        @functools.wraps(func)
        def trace(*args, **kwargs):
            event = ChromeTracingAdvice.__create_event(func)
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
                event = ChromeTracingAdvice.__create_event(func)
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
                rc = func(*args, **kwargs)
                global after_counter, after_counter_mutex
                after_counter_mutex.acquire()
                event = ChromeTracingAdvice.__create_event(func)
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
            event = ChromeTracingAdvice.__create_event(func)
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
