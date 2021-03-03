/************************************************************\
 * Copyright 2021 Lawrence Livermore National Security, LLC
 * (c.f. AUTHORS, NOTICE.LLNS, COPYING)
 *
 * This file is part of the Flux resource manager framework.
 * For details, see https://github.com/flux-framework.
 *
 * SPDX-License-Identifier: LGPL-3.0
\************************************************************/

#include <cstdio>
#include <fstream>
#include <system_error>
#include "advice_dispatcher.hpp"

// TODO: Use TOML configuration to support multiple advice
__attribute__((init_priority(101)))
std::shared_ptr<advice_base_t> advice = nullptr;

__attribute__((init_priority(101)))
const std::string advice_kind = "ChromeTracing";

__attribute__((constructor))
void perfflow_weave_init ()
{
    try {
        if (!(advice = create_advice (advice_kind))) {
            // USE C IO since it more reliable in libray init function
            fprintf (stderr, "error: unknown (%s)\n", advice_kind.c_str ());
            fprintf (stderr, "error: perfflow advice not activated\n");
        }
    } catch (std::ofstream::failure &e) {
        fprintf (stderr, "error: perfflow advice (%s)\n", e.what ());
    } catch (std::system_error &e) {
        fprintf (stderr,
                 "error: perfflow advice (%s)\n",
                 e.code ().message ().c_str ());
    }
    return;
}

extern "C" void perfflow_weave_before (int async,
                                       const char *module,
                                       const char *function,
                                       const char *scope,
                                       const char *flow)
{
    if (advice == nullptr)
        return;
    if (async)
        advice->before_async (module, function, scope, flow);
    else
        advice->before (module, function, flow);
    return;
}

extern "C" void perfflow_weave_after (int async,
                                      const char *module,
                                      const char *function,
                                      const char *scope,
                                      const char *flow)
{
    if (advice == nullptr)
        return;
    if (async)
        advice->after_async (module, function, scope, flow);
    else
        advice->after (module, function, flow);
    return;
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
