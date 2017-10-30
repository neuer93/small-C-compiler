%{
/**
 * @author: Haizhong Zheng (coconut)
 * @studentID: 5110309588
 * @date: 2013-10-31 (MFSBD~)
 * @This is the lex file includes all syntax rules in smallC.
 * 
 */
#include <stdlib.h> 
#include <stdio.h>
#include <string.h>
#include "predefine.h"
typedef struct _tree treeNode;
extern treeNode emptyNode;
extern FILE* fout;
extern treeNode* rootNode;
int yyerror(char *);

%}

%union
{
    struct _tree *node;
}

%start PROGRAM

%token <node> RET STRUCT IF ELSE BREAK CONT FOR READ WRITE
%token <node> TYPE ID INT

%type <node> PROGRAM EXTDEFS EXTDEF EXTVARS SPEC STSPEC OPTTAG VAR FUNC PARAS PARA STMTBLOCK 
%type <node> STMTS STMT ESTMT DEFS DEF DECS DEC INIT ARRS ARGS EXP 
%type <node> STRCDEFS STRCDEF STRCDECS
%type <node> '(' ')' '{' '}' '[' ']' ',' ';'

%right <node> PA SA MA DA AA NA OA SLA SRA '='
%left <node> LOR
%left <node> LAND
%left <node> '|'
%left <node> '^'
%left <node> '&'
%left <node> EQ NE
%left <node> '>' '<' NG NL
%left <node> SL SR
%left <node> '+' '-'
%left <node> '*' '/' '%'
%right <node> '!' '~' PREINC PREDEC
%left <node> '.'

%%
PROGRAM:
    EXTDEFS                                 {$$ = getNewNode("PROGRAM","");setChild($$,$1);rootNode = $$;}
    ;                                       

EXTDEFS:
    EXTDEF EXTDEFS                          {$$ = getNewNode("EXTDEFS","");setChild($$,$1);setChild($$,$2);}
    |                                       {$$ = getNewNode("EXTDEFS","");setChild($$,&emptyNode);}
    ;

EXTDEF:
    SPEC EXTVARS ';'                        {$$ = getNewNode("EXTDEF","");setChild($$,$1);setChild($$,$2);setChild($$,$3);}
    | SPEC FUNC STMTBLOCK                   {$$ = getNewNode("EXTDEF","");setChild($$,$1);setChild($$,$2);setChild($$,$3);}
    ;

EXTVARS:
    DEC                                     {$$ = getNewNode("EXTVARS","");setChild($$,$1);}
    | DEC ',' EXTVARS                       {$$ = getNewNode("EXTVARS","");setChild($$,$1);setChild($$,$2);setChild($$,$3);}
    |                                       {$$ = getNewNode("EXTVARS","");setChild($$,&emptyNode);}
    ;

SPEC:
    TYPE                                    {$$ = getNewNode("SPEC","");setChild($$,$1);}
    | STSPEC                                {$$ = getNewNode("SPEC","");setChild($$,$1);}
    ;

STSPEC:
    STRUCT OPTTAG '{' STRCDEFS '}'          {$$ = getNewNode("STSPEC","");setChild($$,$1);setChild($$,$2);setChild($$,$3);setChild($$,$4);setChild($$,$5);}
    | STRUCT ID                             {$$ = getNewNode("STSPEC","");setChild($$,$1);setChild($$,$2);}
    ;

OPTTAG:
    ID                                      {$$ = getNewNode("OPTTAG","");setChild($$,$1);}
    |                                       {$$ = getNewNode("OPTTAG","");setChild($$,&emptyNode);}
    ;

VAR:
    ID                                      {$$ = getNewNode("VAR","");setChild($$,$1);}
    | VAR '[' INT ']'                       {$$ = getNewNode("VAR","");setChild($$,$1);setChild($$,$2);setChild($$,$3);setChild($$,$4);}
    ;

FUNC:
    ID '(' PARAS ')'                        {$$ = getNewNode("FUNC","");setChild($$,$1);setChild($$,$2);setChild($$,$3);setChild($$,$4);}
    ;

PARAS:
    PARA ',' PARAS                          {$$ = getNewNode("PARAS","");setChild($$,$1);setChild($$,$2);setChild($$,$3);}
    | PARA                                  {$$ = getNewNode("PARAS","");setChild($$,$1);}
    |                                       {$$ = getNewNode("PARAS","");setChild($$,&emptyNode);}
    ;

PARA:
    SPEC VAR                                {$$ = getNewNode("PARA","");setChild($$,$1);setChild($$,$2);}
    ;

STMTBLOCK:
    '{' DEFS STMTS '}'                      {$$ = getNewNode("STMTBLOCK","");setChild($$,$1);setChild($$,$2);setChild($$,$3);setChild($$,$4);}
    ;

STMTS:
    STMT STMTS                              {$$ = getNewNode("STMTS","");setChild($$,$1);setChild($$,$2);}
    |                                       {$$ = getNewNode("STMTS","");setChild($$,&emptyNode);}
    ;

STMT:
    EXP ';'                                 {$$ = getNewNode("STMT","");setChild($$,$1);setChild($$,$2);}
    | STMTBLOCK                             {$$ = getNewNode("STMT","");setChild($$,$1);}
    | RET EXP ';'                           {$$ = getNewNode("STMT","");setChild($$,$1);setChild($$,$2);setChild($$,$3);}
    | IF '(' EXP ')' STMT ESTMT             {$$ = getNewNode("STMT","");setChild($$,$1);setChild($$,$2);setChild($$,$3);setChild($$,$4);setChild($$,$5);setChild($$,$6);}
    | FOR '(' EXP ';' EXP ';' EXP ')' STMT  {$$ = getNewNode("STMT","");setChild($$,$1);setChild($$,$2);setChild($$,$3);setChild($$,$4);setChild($$,$5);setChild($$,$6);setChild($$,$7);setChild($$,$8);setChild($$,$9);}
    | CONT ';'                              {$$ = getNewNode("STMT","");setChild($$,$1);setChild($$,$2);}
    | BREAK ';'                             {$$ = getNewNode("STMT","");setChild($$,$1);setChild($$,$2);}
    | READ '(' EXP ')' ';'                  {$$ = getNewNode("STMT","");setChild($$,$1);setChild($$,$2);setChild($$,$3);setChild($$,$4);setChild($$,$5);}
    | WRITE '(' EXP ')' ';'                 {$$ = getNewNode("STMT","");setChild($$,$1);setChild($$,$2);setChild($$,$3);setChild($$,$4);setChild($$,$5);}
    ;

ESTMT:
    ELSE STMT                               {$$ = getNewNode("ESTMT","");setChild($$,$1);setChild($$,$2);}
    |                                       {$$ = getNewNode("ESTMT","");setChild($$,&emptyNode);}
    ;

DEFS:
    DEF DEFS                                {$$ = getNewNode("DEFS","");setChild($$,$1);setChild($$,$2);}
    |                                       {$$ = getNewNode("DEFS","");setChild($$,&emptyNode);}
    ;

DEF:
    SPEC DECS ';'                           {$$ = getNewNode("DEF","");setChild($$,$1);setChild($$,$2);setChild($$,$3);}
    ;

DECS:
    DEC ',' DECS                            {$$ = getNewNode("DECS","");setChild($$,$1);setChild($$,$2);setChild($$,$3);}
    | DEC                                   {$$ = getNewNode("DECS","");setChild($$,$1);}
    ;

DEC:
    VAR                                     {$$ = getNewNode("DEC","");setChild($$,$1);}
    | VAR '=' INIT                          {$$ = getNewNode("DEC","");setChild($$,$1);setChild($$,$2);setChild($$,$3);}
    ;

STRCDEFS:
    STRCDEF STRCDEFS                        {$$ = getNewNode("STRCDEFS","");setChild($$,$1);setChild($$,$2);}
    |                                       {$$ = getNewNode("STRCDEFS","");setChild($$,&emptyNode);}
    ;

STRCDEF:
    SPEC STRCDECS ';'                       {$$ = getNewNode("STRCDEF","");setChild($$,$1);setChild($$,$2);setChild($$,$3);}
    ;

STRCDECS:
    VAR ',' STRCDECS                        {$$ = getNewNode("STRCDECS","");setChild($$,$1);setChild($$,$2);setChild($$,$3);}
    |VAR                                    {$$ = getNewNode("STRCDECS","");setChild($$,$1);}
    ;

INIT:
    EXP                                     {$$ = getNewNode("INIT","");setChild($$,$1);}
    | '{' ARGS '}'                          {$$ = getNewNode("INIT","");setChild($$,$1);setChild($$,$2);setChild($$,$3);}
    ;

ARRS:
    '[' EXP ']' ARRS                        {$$ = getNewNode("ARRS","");setChild($$,$1);setChild($$,$2);setChild($$,$3);setChild($$,$4);}
    |                                       {$$ = getNewNode("ARRS","");setChild($$,&emptyNode);}
    ;

ARGS:
    EXP ',' ARGS                            {$$ = getNewNode("ARGS","");setChild($$,$1);setChild($$,$2);setChild($$,$3);}
    | EXP                                   {$$ = getNewNode("ARGS","");setChild($$,$1);}
    ;

EXP:
     '-' EXP %prec PREINC                   {$$ = getNewNode("EXP","");setChild($$,$1);setChild($$,$2);}
    | PREINC EXP                            {$$ = getNewNode("EXP","");setChild($$,$1);setChild($$,$2);}
    | PREDEC EXP                            {$$ = getNewNode("EXP","");setChild($$,$1);setChild($$,$2);}
    | '~' EXP                               {$$ = getNewNode("EXP","");setChild($$,$1);setChild($$,$2);}
    | '!' EXP                               {$$ = getNewNode("EXP","");setChild($$,$1);setChild($$,$2);}
    | EXP '+' EXP                           {$$ = getNewNode("EXP","");setChild($$,$1);setChild($$,$2);setChild($$,$3);}
    | EXP '-' EXP                           {$$ = getNewNode("EXP","");setChild($$,$1);setChild($$,$2);setChild($$,$3);}
    | EXP '*' EXP                           {$$ = getNewNode("EXP","");setChild($$,$1);setChild($$,$2);setChild($$,$3);}
    | EXP '/' EXP                           {$$ = getNewNode("EXP","");setChild($$,$1);setChild($$,$2);setChild($$,$3);}
    | EXP '&' EXP                           {$$ = getNewNode("EXP","");setChild($$,$1);setChild($$,$2);setChild($$,$3);}
    | EXP '^' EXP                           {$$ = getNewNode("EXP","");setChild($$,$1);setChild($$,$2);setChild($$,$3);}
    | EXP '|' EXP                           {$$ = getNewNode("EXP","");setChild($$,$1);setChild($$,$2);setChild($$,$3);}
    | EXP '%' EXP                           {$$ = getNewNode("EXP","");setChild($$,$1);setChild($$,$2);setChild($$,$3);}
    | EXP '=' EXP                           {$$ = getNewNode("EXP","");setChild($$,$1);setChild($$,$2);setChild($$,$3);}
    | EXP '>' EXP                           {$$ = getNewNode("EXP","");setChild($$,$1);setChild($$,$2);setChild($$,$3);}
    | EXP '<' EXP                           {$$ = getNewNode("EXP","");setChild($$,$1);setChild($$,$2);setChild($$,$3);}
    | EXP SL EXP                            {$$ = getNewNode("EXP","");setChild($$,$1);setChild($$,$2);setChild($$,$3);}
    | EXP SR EXP                            {$$ = getNewNode("EXP","");setChild($$,$1);setChild($$,$2);setChild($$,$3);}
    | EXP NL EXP                            {$$ = getNewNode("EXP","");setChild($$,$1);setChild($$,$2);setChild($$,$3);}
    | EXP NG EXP                            {$$ = getNewNode("EXP","");setChild($$,$1);setChild($$,$2);setChild($$,$3);}
    | EXP EQ EXP                            {$$ = getNewNode("EXP","");setChild($$,$1);setChild($$,$2);setChild($$,$3);}
    | EXP NE EXP                            {$$ = getNewNode("EXP","");setChild($$,$1);setChild($$,$2);setChild($$,$3);}
    | EXP LAND EXP                          {$$ = getNewNode("EXP","");setChild($$,$1);setChild($$,$2);setChild($$,$3);}
    | EXP LOR EXP                           {$$ = getNewNode("EXP","");setChild($$,$1);setChild($$,$2);setChild($$,$3);}
    | EXP PA EXP                            {$$ = getNewNode("EXP","");setChild($$,$1);setChild($$,$2);setChild($$,$3);}
    | EXP SA EXP                            {$$ = getNewNode("EXP","");setChild($$,$1);setChild($$,$2);setChild($$,$3);}
    | EXP MA EXP                            {$$ = getNewNode("EXP","");setChild($$,$1);setChild($$,$2);setChild($$,$3);}
    | EXP DA EXP                            {$$ = getNewNode("EXP","");setChild($$,$1);setChild($$,$2);setChild($$,$3);}
    | EXP AA EXP                            {$$ = getNewNode("EXP","");setChild($$,$1);setChild($$,$2);setChild($$,$3);}
    | EXP NA EXP                            {$$ = getNewNode("EXP","");setChild($$,$1);setChild($$,$2);setChild($$,$3);}
    | EXP OA EXP                            {$$ = getNewNode("EXP","");setChild($$,$1);setChild($$,$2);setChild($$,$3);}
    | EXP SLA EXP                           {$$ = getNewNode("EXP","");setChild($$,$1);setChild($$,$2);setChild($$,$3);}
    | EXP SRA EXP                           {$$ = getNewNode("EXP","");setChild($$,$1);setChild($$,$2);setChild($$,$3);}
    | '(' EXP ')'                           {$$ = getNewNode("EXP","");setChild($$,$1);setChild($$,$2);setChild($$,$3);}
    | ID '(' ARGS ')'                       {$$ = getNewNode("EXP","");setChild($$,$1);setChild($$,$2);setChild($$,$3);}
    | ID ARRS                               {$$ = getNewNode("EXP","");setChild($$,$1);setChild($$,$2);}
    | EXP '.' ID                            {$$ = getNewNode("EXP","");setChild($$,$1);setChild($$,$2);setChild($$,$3);}
    | INT                                   {$$ = getNewNode("EXP","");setChild($$,$1);}
    |                                       {$$ = getNewNode("EXP","");setChild($$,&emptyNode);}
    ;
%%
int yyerror(char *s) { 
    fprintf(fout, "Error\n"); 
    fclose(fout);
    exit(1);
}