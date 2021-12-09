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
from .advice_chrome import ChromeTracingAdvice


def _disable(func):
    "Do nothing decorator (e.g., disables func)"
    def do_nothing_func(*args, **kargs):
        print("perfflow warning: logging has been disabled for {}".format(func.__name__), file=sys.stderr)
        pass
    return do_nothing_func


class AdviceDispatcher:

    # Note that you need to extend advice_dict When you add
    # a new type of advice in order to integrate it into
    # this dispacher class.
    advice_dict = dict(ChromeTracing=ChromeTracingAdvice)

    def _dispatch(clobj, pointcut, scope):
        if pointcut == "around":
            return clobj.around if ChromeTracingAdvice.enable_logging else _disable
        elif pointcut == "before":
            return clobj.before if ChromeTracingAdvice.enable_logging else _disable
        elif pointcut == "after":
            return clobj.after if ChromeTracingAdvice.enable_logging else _disable
        elif pointcut == "around_async":
            return clobj.around_async if ChromeTracingAdvice.enable_logging else _disable
        elif pointcut == "before_async":
            return clobj.before_async(scope) if ChromeTracingAdvice.enable_logging else _disable
        elif pointcut == "after_async":
            return clobj.after_async(scope) if ChromeTracingAdvice.enable_logging else _disable
        raise KeyError("unknown pointcut", pointcut)

    def get(key, pointcut, scope):
        clobj = AdviceDispatcher.advice_dict[key]
        return AdviceDispatcher._dispatch(clobj, pointcut, scope)

    def get_advice(key):
        return AdviceDispatcher.advice_dict[key]


#
# vi: ts=4 sw=4 expandtab
#
