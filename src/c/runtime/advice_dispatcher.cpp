/************************************************************\
 * Copyright 2021 Lawrence Livermore National Security, LLC
 * (c.f. AUTHORS, NOTICE.LLNS, COPYING)
 *
 * This file is part of the Flux resource manager framework.
 * For details, see https://github.com/flux-framework.
 *
 * SPDX-License-Identifier: LGPL-3.0
\************************************************************/

#include <iostream>

#include "advice_dispatcher.hpp"
#include "advice_chrome_tracing.hpp"

std::shared_ptr<advice_base_t> create_advice (const std::string &kind)
{
    std::shared_ptr<advice_base_t> advice = nullptr;
    try {
        if (kind == "ChromeTracing") {
            advice = std::make_shared<advice_chrome_tracing_t> ();
            std::cout << "PFA create_advice " << std::endl;
        }
    } catch (std::bad_alloc &e) {
        errno = ENOMEM;
        advice = nullptr;
    }
    return advice;
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
