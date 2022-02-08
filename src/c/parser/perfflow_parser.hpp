/*****************************************************************\
 * Copyright 2021-2022 Lawrence Livermore National Security, LLC
 * (c.f. AUTHORS, NOTICE.LLNS, COPYING)
 *
 * This file is part of the Flux resource manager framework.
 * For details, see https://github.com/flux-framework.
 *
 * SPDX-License-Identifier: LGPL-3.0
\*****************************************************************/

#ifndef PERFFLOW_PARSER_HPP
#define PERFFLOW_PARSER_HPP

#include <string>

int perfflow_parser_validate (const char *anno);

int perfflow_parser_parse (const char *anno,
                           std::string &pointcut,
                           std::string &scope,
                           std::string &flow);
	
#endif // PERFFLOW_PARSER_HPP

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
