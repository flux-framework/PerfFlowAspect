/************************************************************\
 * Copyright 2021 Lawrence Livermore National Security, LLC
 * (c.f. AUTHORS, NOTICE.LLNS, COPYING)
 *
 * This file is part of the Flux resource manager framework.
 * For details, see https://github.com/flux-framework.
 *
 * SPDX-License-Identifier: LGPL-3.0
\************************************************************/

#ifndef ADVICE_BASE_HPP
#define ADVICE_BASE_HPP

struct advice_base_t
{
    virtual int before (const char *module,
                        const char *function,
                        const char *flow) = 0;
    virtual int after (const char *module,
                       const char *function,
                       const char *flow) = 0;
    virtual int before_async (const char *module,
                              const char *function,
                              const char *scope,
                              const char *flow) = 0;
    virtual int after_async (const char *moduln,
                             const char *function,
                             const char *scope,
                             const char *flow) = 0;
};

#endif // ADVICE_BASE_HPP

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
