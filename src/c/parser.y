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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

extern void set_input_string (const char *in);
extern void end_lexical_scan ();
extern int yylex();
extern int yyparse ();
extern void yyerror (const char* s);
%}

%union {
    char label[4096];
}

%token T_NEWLINE
%token T_CPA
%token T_POINTCUT
%token T_PC_AROUND
%token T_PC_BEGIN
%token T_PC_AFTER
%token T_PC_AROUND_ASYNC
%token T_PC_BEGIN_ASYNC
%token T_PC_AFTER_ASYNC
%token T_SCOPE
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

pointcut_assignment: T_POINTCUT T_ASSIGN T_PC_AROUND
    | T_POINTCUT T_ASSIGN T_PC_BEGIN
    | T_POINTCUT T_ASSIGN T_PC_AFTER
    | T_POINTCUT T_ASSIGN T_PC_AROUND_ASYNC
    | T_POINTCUT T_ASSIGN T_PC_BEGIN_ASYNC
    | T_POINTCUT T_ASSIGN T_PC_AFTER_ASYNC
;

scope_assignment: T_SCOPE T_ASSIGN T_LABEL
;

%%

void yyerror (const char *s)
{
}

int validate (const char *in)
{
    if (!in) {
        errno = EINVAL;
        return -1;
    }
    set_input_string (in);
    int rc = yyparse ();
    end_lexical_scan ();
    return rc;
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
