###################################################################
# Copyright 2021-2022 Lawrence Livermore National Security, LLC
# (c.f. AUTHORS, NOTICE.LLNS, COPYING)
#
# This file is part of the Flux resource manager framework.
# For details, see https://github.com/flux-framework.
#
# SPDX-License-Identifier: LGPL-3.0
###################################################################


def perfflowaspect(target):
    """Base class to decorate each specific advice class:
    Use @perfflowaspect on the advice class
    """

    def decorator_init(self):
        print("Decorator running")

    target.__init__ = decorator_init
    return target


#
# vi: ts=4 sw=4 expandtab
#
