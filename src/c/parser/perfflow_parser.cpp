/************************************************************\
 * Copyright 2021 Lawrence Livermore National Security, LLC
 * (c.f. AUTHORS, NOTICE.LLNS, COPYING)
 *
 * This file is part of the Flux resource manager framework.
 * For details, see https://github.com/flux-framework.
 *
 * SPDX-License-Identifier: LGPL-3.0
\************************************************************/

#include <cerrno>
#include "parser.tab.h"
#include "perfflow_parser.hpp"

extern int validate(const char *);
extern int parse(const char *, std::string &, std::string &, std::string &);

int perfflow_parser_validate(const char *anno)
{
    if (!anno)
    {
        errno = EINVAL;
        return -1;
    }
    return validate(anno);
}

int perfflow_parser_parse(const char *anno,
                          std::string &pointcut,
                          std::string &scope,
                          std::string &flow)
{
    if (!anno)
    {
        errno = EINVAL;
        return -1;
    }
    return parse(anno, pointcut, scope, flow);
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
