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

# check format for all *.cpp, *.h, and *.c files
FILES=$(find src/c -type f  \( -name "*.[ch]" -o -name *.cpp \))

astyle --errors-to-stdout \
       --preserve-date \
       --style=allman \
       --indent=spaces=4 \
       --convert-tabs \
       --attach-extern-c \
       --indent-col1-comments \
       --min-conditional-indent=0 \
       --max-continuation-indent=40 \
       --pad-oper \
       --pad-header \
       --unpad-paren \
       --align-pointer=name \
       --align-reference=name \
       --break-closing-braces \
       --keep-one-line-blocks \
       --keep-one-line-statements \
       --max-code-length=80 \
       --break-after-logical \
       --indent-switches \
       --add-braces \
       ${FILES}

#