#ifndef _PREDEFINE_H
#define _PREDEFINE_H

#include "stdio.h"
#include "block.h"
#include "tree.h"
#include "typecheck.h"
#include "symboltable.h"
#include "codegen.h"
#include <stdlib.h>
#include <string.h>


#define TRUE 1
#define FALSE 0
typedef struct _block stmtBlock;
typedef struct _tree treeNode;


//helper
char* addString(char *s1,char *s2);
char* subString(char *s,int begin,int end);

void fendl();
void foutput(char *s);

char* todec(char* num);

void tcErrorOutput(int errorID,char* errorInfo);
void dbs(char *info);
void dbi(int info);



FILE* fout;
int anonStrcNum;
stmtBlock *currentBlock,*globalBlock;
treeNode emptyNode;
treeNode* rootNode;

int g_structRedefine;
int g_varRedefine;

#endif