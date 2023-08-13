%{
#include <string>
#include "Object.hpp"
#include "parser.tab.hpp"
%}

%option noyywrap
%option yylineno

number				(0|[1-9][0-9]*)
string				(\"([^\n\r\"\\]|\\[rnt\"\\])+\")
comment				(\/\/[^\r\n]*((\r)|(\n)|(\r\n)))
whitespace			[\t\r\n ]

%%

\:								return COLON;
\,								return COMMA;
\[								return LBRACKET;
\]								return RBRACKET;
\{								return LBRACE;
\}								return RBRACE;
{string}						{
									yylval.string_literal = new std::string(yytext);
									return STRING_LITERAL;
								}
{number}						{
									yylval.number_literal = atoi(yytext);
									return NUMBER_LITERAL;
								}
{comment}						;
{whitespace}					;
.								{
									printf("unexpected character at line %d: '%s'\n", yylineno, yytext);
									exit(420);
								}

%%

