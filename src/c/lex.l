/************************************************************\
 * Copyright 2021 Lawrence Livermore National Security, LLC
 * (c.f. AUTHORS, NOTICE.LLNS, COPYING)
 *
 * This file is part of the Flux resource manager framework.
 * For details, see https://github.com/flux-framework.
 *
 * SPDX-License-Identifier: LGPL-3.0
\************************************************************/

%option noyywrap

%{
#include <stdio.h>
#define YY_DECL int yylex ()
#include "parser.tab.h"
%}

label              ["]([^"\\\n]|\\(.|\n))*["]

%%

[ \t]   ; // ignore all whitespace
"@perfflow.aspect.critical_path" { return T_CPA; }
"@critical_path"   { return T_CPA; }
"pointcut"         { return T_POINTCUT; }
"\"around\""       { return T_PC_AROUND; }
"\"before\""       { return T_PC_BEGIN; }
"\"after\""        { return T_PC_AFTER; }
"\"around_async\"" { return T_PC_AROUND_ASYNC; }
"\"before_async\"" { return T_PC_BEGIN_ASYNC; }
"\"after_async\""  { return T_PC_AFTER_ASYNC; }
"scope"            { return T_SCOPE; }
{label}            { snprintf (yylval.label, 4096, "%s", yytext); return T_LABEL; }
\n                 { return T_NEWLINE; }
","                { return T_COMMA; }
"="                { return T_ASSIGN; }
"("                { return T_LEFT; }
")"                { return T_RIGHT; }
.                  {}

%%

void set_input_string (const char *in) {
    yy_scan_string (in);
}

void end_lexical_scan () {
    yy_delete_buffer(YY_CURRENT_BUFFER);
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
