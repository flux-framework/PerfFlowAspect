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

void test_valid_annotations()
{
    int rc = -1;
    std::string anno1("@critical_path()");
    rc = perfflow_parser_validate(anno1.c_str());
    ok(rc == 0, "%s is valid", anno1.c_str());

    std::string anno2("@critical_path  (  )");
    rc = perfflow_parser_validate(anno2.c_str());
    ok(rc == 0, "%s is valid", anno2.c_str());

    std::string anno3("@perfflowaspect.aspect.critical_path ()");
    rc = perfflow_parser_validate(anno3.c_str());
    ok(rc == 0, "%s is valid", anno3.c_str());

    std::string anno4("@critical_path (pointcut='around')");
    rc = perfflow_parser_validate(anno4.c_str());
    ok(rc == 0, "%s is valid", anno4.c_str());

    std::string anno5("@critical_path (pointcut	='around')");
    rc = perfflow_parser_validate(anno5.c_str());
    ok(rc == 0, "%s is valid", anno5.c_str());

    std::string anno6("@critical_path (pointcut='before', scope='foo')");
    rc = perfflow_parser_validate(anno6.c_str());
    ok(rc == 0, "%s is valid", anno6.c_str());

    std::string anno7("@critical_path (scope='foo', pointcut='before')");
    rc = perfflow_parser_validate(anno7.c_str());
    ok(rc == 0, "%s is valid", anno7.c_str());

    std::string anno8("@critical_path (scope='', flow='out', pointcut='before')");
    rc = perfflow_parser_validate(anno8.c_str());
    ok(rc == 0, "%s is valid", anno8.c_str());
}

void test_invalid_annotations()
{
    int rc = -1;
    std::string anno1("@critical _path()");
    rc = perfflow_parser_validate(anno1.c_str());
    ok(rc != 0, "%s is not valid", anno1.c_str());

    std::string anno2("@critcal_path  (  )");
    rc = perfflow_parser_validate(anno2.c_str());
    ok(rc != 0, "%s is not valid", anno2.c_str());

    std::string anno3("@aspect.critical_path ()");
    rc = perfflow_parser_validate(anno3.c_str());
    ok(rc != 0, "%s is not valid", anno3.c_str());

    std::string anno4("@critical_path (pointcut='arond')");
    rc = perfflow_parser_validate(anno4.c_str());
    ok(rc != 0, "%s is not valid", anno4.c_str());

    std::string anno5("@critical_path (point_cut='around')");
    rc = perfflow_parser_validate(anno5.c_str());
    ok(rc != 0, "%s is not valid", anno5.c_str());

    std::string anno6("@critical_path (pointcut='before' scope='foo')");
    rc = perfflow_parser_validate(anno6.c_str());
    ok(rc != 0, "%s is not valid", anno6.c_str());

    std::string anno7("@critical_path (scope='bar', pointcut='')");
    rc = perfflow_parser_validate(anno7.c_str());
    ok(rc != 0, "%s is not valid", anno7.c_str());
}

int main(int argc, char *argv[])
{
    int rc = -1;
    plan(15);
    test_valid_annotations();
    test_invalid_annotations();
    done_testing();
    return EXIT_SUCCESS;
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
