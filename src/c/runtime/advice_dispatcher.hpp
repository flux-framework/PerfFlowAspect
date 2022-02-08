/*****************************************************************\
 * Copyright 2021-2022 Lawrence Livermore National Security, LLC
 * (c.f. AUTHORS, NOTICE.LLNS, COPYING)
 *
 * This file is part of the Flux resource manager framework.
 * For details, see https://github.com/flux-framework.
 *
 * SPDX-License-Identifier: LGPL-3.0
\*****************************************************************/

#ifndef ADVICE_DISPATCHER_HPP
#define ADVICE_DISPATCHER_HPP

#include <string>
#include <memory>
#include "advice_base.hpp"

std::shared_ptr<advice_base_t> create_advice (const std::string &kind);

#endif // ADVICE_DISPATCHER_HPP

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
