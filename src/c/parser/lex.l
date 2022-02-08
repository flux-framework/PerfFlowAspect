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
#include <cstdio>
#include <iostream>
#define YY_DECL int yylex ()
#include "parser.tab.h"
%}

flowtype           ['](in|out|inout|outin)[']
label              [']([^'\\\n]|\\(.|\n))*[']
pcutop             ['](around|before|after)(_async)?[']

%%

[ \t]   ; // ignore all whitespace
"@perfflowaspect.aspect.critical_path" { return T_CPA; }
"@critical_path" { return T_CPA; }
"pointcut"       { snprintf (yylval.str, 4096, "%s", yytext); return T_PCUT; }
{pcutop}         { snprintf (yylval.str, 4096, "%s", yytext); return T_PC_OP; }
"flow"           { snprintf (yylval.str, 4096, "%s", yytext); return T_FLOW; }
{flowtype}       { snprintf (yylval.str, 4096, "%s", yytext); return T_FW_TY; }
"scope"          { snprintf (yylval.str, 4096, "%s", yytext); return T_SCOPE; }
{label}          { snprintf (yylval.str, 4096, "%s", yytext); return T_LABEL; }
\n               { return T_NEWLINE; }
","              { return T_COMMA; }
"="              { return T_ASSIGN; }
"("              { return T_LEFT; }
")"              { return T_RIGHT; }
.                {}

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
