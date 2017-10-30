%{
#include <stdio.h>  
#include "y.tab.h"
#include "predefine.h"
%}

dec     [0-9]
oct     [0-7]
hex     [0-9a-fA-F]

%%
return                          {yylval.node = getNewNode("RET",strdup(yytext));       return RET;}
struct                          {yylval.node = getNewNode("STRUCT",strdup(yytext));   return STRUCT;}
if                              {yylval.node = getNewNode("IF",strdup(yytext));       return IF;}
else                            {yylval.node = getNewNode("ELSE",strdup(yytext));     return ELSE;}
break                           {yylval.node = getNewNode("BREAK",strdup(yytext));    return BREAK;}
continue                        {yylval.node = getNewNode("CONT",strdup(yytext));     return CONT;}
for                             {yylval.node = getNewNode("FOR",strdup(yytext));      return FOR;}
int                             {yylval.node = getNewNode("TYPE",strdup(yytext));     return TYPE;}
write                           {yylval.node = getNewNode("WRITE",strdup(yytext));    return WRITE;}
read                            {yylval.node = getNewNode("READ",strdup(yytext));     return READ;}

[1-9]+{dec}*                    {yylval.node = getNewNode("INT",strdup(yytext));      return INT;}
0{oct}*                         {yytext = todec(yytext);yylval.node = getNewNode("INT",strdup(yytext));      return INT;}
0(x|X){hex}+                    {yytext = todec(yytext);yylval.node = getNewNode("INT",strdup(yytext));      return INT;}

[a-zA-Z\_][\_a-zA-Z0-9]*        {yylval.node = getNewNode("ID",strdup(yytext));       return ID;}

[*,;.(){}!~/%+-><&^|=\[\]]      {yylval.node = getNewNode(strdup(yytext),strdup(yytext));return *yytext;}

"++"                            {yylval.node = getNewNode("PREINC","++");             return PREINC;}
"--"                            {yylval.node = getNewNode("PREDEC","--");             return PREDEC;}
"<<"                            {yylval.node = getNewNode("SL","<<");                 return SL;}
">>"                            {yylval.node = getNewNode("SR",">>");                 return SR;}
">="                            {yylval.node = getNewNode("NL",">=");                 return NL;}
"<="                            {yylval.node = getNewNode("NG","<=");                 return NG;}
"=="                            {yylval.node = getNewNode("EQ","==");                 return EQ;}
"!="                            {yylval.node = getNewNode("NE","!=");                 return NE;}
"&&"                            {yylval.node = getNewNode("LAND","&&");               return LAND;}
"||"                            {yylval.node = getNewNode("LOR","||");                return LOR;}

"+="                            {yylval.node = getNewNode("PA","+=");                 return PA;}
"-="                            {yylval.node = getNewNode("SA","-=");                 return SA;}
"*="                            {yylval.node = getNewNode("MA","*=");                 return MA;}
"/="                            {yylval.node = getNewNode("DA","/=");                 return DA;}
"&="                            {yylval.node = getNewNode("AA","&=");                 return AA;}
"^="                            {yylval.node = getNewNode("NA","^=");                 return NA;}
"|="                            {yylval.node = getNewNode("OA","|=");                 return OA;}
"<<="                           {yylval.node = getNewNode("SLA","<<=");               return SLA;}
">>="                           {yylval.node = getNewNode("SRA",">>=");               return SRA;}

.|\n        ;
%%

int yywrap()
{
  fclose(yyin);
  return 1;
}
