##############################################################
# Copyright 2021 Lawrence Livermore National Security, LLC
# (c.f. AUTHORS, NOTICE.LLNS, COPYING)
#
# This file is part of the Flux resource manager framework.
# For details, see https://github.com/flux-framework.
#
# SPDX-License-Identifier: LGPL-3.0
##############################################################

import sys
import functools
from .advice_dispatch import AdviceDispatcher
from .advice_chrome import ChromeTracingAdvice

# TODO: Use TOML configuration to support multiple advice
advice_kind = "ChromeTracing"


def _disable(func):
    "Do nothing decorator (e.g., disables func)"

    def do_nothing_func(*args, **kargs):
        pass

    return do_nothing_func


def critical_path(pointcut="around", scope=""):
    def decorator_critical_path(func):
        @functools.wraps(func)
        def wrapper_critial_path(*args, **kwargs):
            f = func
            try:
                global advice_kind
                f = AdviceDispatcher.get(advice_kind, pointcut, scope)(func)
            except KeyError as e:
                print("perfflow error:", str(e), file=sys.stderr)
            return f(*args, **kwargs)

        return wrapper_critial_path

    return decorator_critical_path if ChromeTracingAdvice.enable_logging else _disable


def sync_event(pointcut, name, category):
    if ChromeTracingAdvice.enable_logging:
        clobj = AdviceDispatcher.get_advice(advice_kind)
        clobj.sync_event(pointcut, name, category)
    else:
        return _disable


def async_event(pointcut, name, category, scope=None, id=None):
    if ChromeTracingAdvice.enable_logging:
        clobj = AdviceDispatcher.get_advice(advice_kind)
        clobj.async_event(pointcut, name, category, scope=scope, id=id)
    else:
        return _disable


#
# vi: ts=4 sw=4 expandtab
#
