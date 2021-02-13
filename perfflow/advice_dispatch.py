##############################################################
# Copyright 2021 Lawrence Livermore National Security, LLC
# (c.f. AUTHORS, NOTICE.LLNS, COPYING)
#
# This file is part of the Flux resource manager framework.
# For details, see https://github.com/flux-framework.
#
# SPDX-License-Identifier: LGPL-3.0
##############################################################

from .advice_chrome import ChromeTracingAdvice


class AdviceDispatcher:

    # Note that you need to extend advice_dict When you add
    # a new type of advice in order to integrate it into
    # this dispacher class.
    advice_dict = dict(ChromeTracing=ChromeTracingAdvice)

    def _dispatch(clobj, pointcut, scope):
        if pointcut == "around":
            return clobj.around
        elif pointcut == "before":
            return clobj.before
        elif pointcut == "after":
            return clobj.after
        elif pointcut == "around_async":
            return clobj.around_async
        elif pointcut == "before_async":
            return clobj.before_async(scope)
        elif pointcut == "after_async":
            return clobj.after_async(scope)
        raise KeyError("unknown pointcut", pointcut)

    def get(key, pointcut, scope):
        clobj = AdviceDispatcher.advice_dict[key]
        return AdviceDispatcher._dispatch(clobj, pointcut, scope)


#
# vi: ts=4 sw=4 expandtab
#
