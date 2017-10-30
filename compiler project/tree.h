#ifndef _TREE_H
#define _TREE_H
#include "predefine.h"
typedef struct _block stmtBlock;

typedef struct _tree
{
    struct _tree *leftChild,*rightBrother;
    char* name;
    char* val;
    stmtBlock* block;
} treeNode;

treeNode* getNewNode(char *name,char *val);
treeNode* getNthNodeChild(treeNode *father,int n);

int countVarArrNum(treeNode *var);
char* getVarName(treeNode *var);
char* getTypeName(treeNode *spec);

void setChild(treeNode * father,treeNode * child);
void setBrother(treeNode * older,treeNode * younger);

int checkTreeNodeFun(treeNode *node);
int countFunNum(treeNode *node);
int countParaNum(treeNode *node);

int checkTreeNodeStrc(treeNode *node);
int countStrcNum(treeNode *node);
int countMemNumStrcdefs(treeNode *node);
int countMemNumStrcdef(treeNode *node);

int checkTreeNodeVar(treeNode *extdef);
int countVarNum(treeNode *extdefs);
int countVarNumExtvars(treeNode *extvars);

//inner block
int countStrcNumI(treeNode *defs);
int countVarNumI(treeNode *defs);
int evalConstant(treeNode *expr);
int isConstant(treeNode *expr);
int isBoolExp(treeNode *expr);
int countArgs(treeNode *args);
treeNode* getExpInArgs(treeNode *args,int i);

int checkTreeNodeStrcI(treeNode *def);
#endif