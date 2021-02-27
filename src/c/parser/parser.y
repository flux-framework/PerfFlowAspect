/************************************************************\
 * Copyright 2021 Lawrence Livermore National Security, LLC
 * (c.f. AUTHORS, NOTICE.LLNS, COPYING)
 *
 * This file is part of the Flux resource manager framework.
 * For details, see https://github.com/flux-framework.
 *
 * SPDX-License-Identifier: LGPL-3.0
\************************************************************/

%{

#include <iostream>
#include <cstdlib>
#include <cerrno>
#include <string>
#include <cstring>
#include <map>

std::map<std::string, std::string> params;
extern void set_input_string (const char *in);
extern void end_lexical_scan ();
extern int yylex();
extern int yyparse ();
extern void yyerror (const char* s);
%}

%union {
    char str[4096];
}

%token T_NEWLINE
%token T_CPA
%token<str> T_PCUT
%token<str> T_PC_OP
%token<str> T_SCOPE
%token<str> T_LABEL
%token T_COMMA
%token T_ASSIGN
%token T_LEFT
%token T_RIGHT

%start annotation

%%

annotation:
    | annotation line
;

line: T_NEWLINE
    | T_CPA T_LEFT args T_RIGHT
;

args:
    | pointcut_assignment
    | scope_assignment
    | pointcut_assignment T_COMMA scope_assignment
    | scope_assignment T_COMMA pointcut_assignment
;

pointcut_assignment: T_PCUT T_ASSIGN T_PC_OP { params[$1] = $3; }
;

scope_assignment: T_SCOPE T_ASSIGN T_LABEL { params[$1] = $3; }
;

%%

void yyerror (const char *s)
{
}

int validate (const char *in)
{
    int rc = -1;
    if (!in) {
        errno = EINVAL;
        return rc;
    }
    set_input_string (in);
    rc = yyparse ();
    end_lexical_scan ();
    params.clear ();
    return rc;
}

int parse (const char *in, std::string &pointcut, std::string &scope)
{
    int rc = -1;
    if (!in) {
        errno = EINVAL;
        return rc;
    }
    set_input_string (in);
    rc = yyparse ();
    end_lexical_scan ();

    auto p_iter = params.find ("pointcut");
    if (p_iter != params.end ()) {
        pointcut = p_iter->second;
        pointcut = pointcut.substr (1, pointcut.size () - 2);
    } else {
        pointcut = "around";
    }

    auto p_iter2 = params.find ("scope");
    if (p_iter2 != params.end ()) {
        scope = p_iter2->second;
        scope = scope.substr (1, scope.size () - 2);
    } else {
        scope = "NA";
    }

    params.clear ();
    return rc;
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
