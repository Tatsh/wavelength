%{
#include <io.h>

#include "utl/DataFile.h"

// Input that matches no rule is skipped.
#define ECHO
#define YY_INPUT(buf, result, max) ((result) = DataLexInput((buf), (max)))
%}

%option noyywrap nounistd

%%

\n                                       { ++gDataLine; }
[-+]?[0-9]+                              { return kDataTokenInt; }
[-+]?[0-9]*\.[0-9]*                      { return kDataTokenFloat; }
[-+]?[0-9]*\.[0-9]*(e|E)(\+|-)?[0-9]+    { return kDataTokenFloat; }
\"[^"]*\"                                { return kDataTokenString; }
[^ \t\r\n(){}<>]+                        { return kDataTokenSymbol; }
;[^\n]*\n                                { yyless(yyleng - 1); }
"("                                      { return kDataTokenArrayOpen; }
")"                                      { return kDataTokenArrayClose; }
"{"                                      { return kDataTokenMacroOpen; }
"}"                                      { return kDataTokenMacroClose; }
"<<"[^>]*">>"                            { return kDataTokenMergeInclude; }
"<"[^>]*">"                              { return kDataTokenInclude; }

%%
