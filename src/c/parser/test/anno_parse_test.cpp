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
#include <string>
#include "tap.h"
#include "perfflow_parser.hpp"

void parse_valid_annotations()
{
    int rc = -1;
    std::string pointcut, scope, flow;
    std::string anno1("@critical_path()");
    rc = perfflow_parser_parse(anno1.c_str(), pointcut, scope, flow);
    ok(rc == 0 && pointcut == "around" && scope == "NA" && flow == "NA",
       "%s is correctly parsed", anno1.c_str());

    pointcut = ""; scope = ""; flow = "";
    std::string anno2("@critical_path  (  )");
    rc = perfflow_parser_parse(anno2.c_str(), pointcut, scope, flow);
    ok(rc == 0 && pointcut == "around" && scope == "NA" && flow == "NA",
       "%s is correctly parsed", anno2.c_str());

    pointcut = ""; scope = ""; flow = "";
    std::string anno3("@perfflowaspect.aspect.critical_path ()");
    rc = perfflow_parser_parse(anno3.c_str(), pointcut, scope, flow);
    ok(rc == 0 && pointcut == "around" && scope == "NA" && flow == "NA",
       "%s is correctly parsed", anno3.c_str());

    pointcut = ""; scope = ""; flow = "";
    std::string anno4("@critical_path (pointcut='around')");
    rc = perfflow_parser_parse(anno4.c_str(), pointcut, scope, flow);
    ok(rc == 0 && pointcut == "around" && scope == "NA" && flow == "NA",
       "%s is correctly parsed", anno4.c_str());

    pointcut = ""; scope = ""; flow = "";
    std::string anno5("@critical_path (pointcut	='around')");
    rc = perfflow_parser_parse(anno5.c_str(), pointcut, scope, flow);
    ok(rc == 0 && pointcut == "around" && scope == "NA" && flow == "NA",
       "%s is correctly parsed", anno5.c_str());

    pointcut = ""; scope = ""; flow = "";
    std::string anno6("@critical_path (pointcut='before', scope='foo')");
    rc = perfflow_parser_parse(anno6.c_str(), pointcut, scope, flow);
    ok(rc == 0 && pointcut == "before" && scope == "foo" && flow == "NA",
       "%s is correctly parsed", anno6.c_str());

    pointcut = ""; scope = ""; flow = "";
    std::string anno7("@critical_path (scope='foo', pointcut='before', flow='in')");
    rc = perfflow_parser_parse(anno7.c_str(), pointcut, scope, flow);
    ok(rc == 0 && pointcut == "before" && scope == "foo" && flow == "in",
       "%s is correctly parsed", anno7.c_str());

    pointcut = ""; scope = "";
    std::string anno8("@critical_path (scope='', pointcut='before')");
    rc = perfflow_parser_parse(anno8.c_str(), pointcut, scope, flow);
    ok(rc == 0 && pointcut == "before" && scope == "" && flow == "NA",
       "%s is correctly parsed", anno8.c_str());
}

int main(int argc, char *argv[])
{
    int rc = -1;
    plan(8);
    parse_valid_annotations();
    done_testing();
    return EXIT_SUCCESS;
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
