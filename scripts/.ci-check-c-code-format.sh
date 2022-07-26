#!/bin/bash

##############################################################
# Copyright 2021 Lawrence Livermore National Security, LLC
# (c.f. AUTHORS, NOTICE.LLNS, COPYING)
#
# This file is part of the Flux resource manager framework.
# For details, see https://github.com/flux-framework.
#
# SPDX-License-Identifier: LGPL-3.0
##############################################################

TMP=`./scripts/check-c-code-format.sh`

RES=$(echo "$TMP" | grep Formatted)

echo -e "$RES"

if [ -z "$RES" ]; then
    exit 0
else
    exit 1
fi

#
