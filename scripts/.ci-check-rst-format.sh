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

./scripts/check-rst-format.sh

RES=$(git ls-files -m)

echo -e "$RES"

if [ -z "$RES" ]; then
    exit 0
else
    echo -e "Formatting issue found in RST files. Please run ./scripts/check-rst-format.sh."
    exit 1
fi

#
