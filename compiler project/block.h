#ifndef _BLOCK_H
#define _BLOCK_H
#include "predefine.h"
#include "symboltable.h"

typedef struct _tree treeNode;
typedef struct _funtable funSymTable;
typedef struct _strctable strcSymTable;
typedef struct _variabletable varSymTable;
typedef struct _funitem funSymTableItem;
typedef struct _varitem varSymTableItem;
typedef struct _strcitem strcSymTableitem;

typedef struct _block
{
	struct _block *leftChild,*rightBrother,*father;
	int  isExternal;
	funSymTable *funTable;
	strcSymTable *strcTable;
	varSymTable *varTable;
	treeNode *stmt;
} stmtBlock;

stmtBlock* createExtenalBlock();
stmtBlock* createBlock(stmtBlock *father,treeNode *stmtblock,funSymTableItem *funItem);
void setBlockChild(stmtBlock *father,stmtBlock *child);
void setBlockBrother(stmtBlock *older,stmtBlock *younger);
stmtBlock* getNthBlockChild(stmtBlock *father,int n);

void parseStmt(stmtBlock *scopeBlock,treeNode *stmt);

int countAllStrcNum(stmtBlock *scopeBlock);
int countAllVarNum(stmtBlock *scopeBlock);
varSymTableItem* getVarItemViaName(stmtBlock *curBlock,char *name);
strcSymTableitem* getStrcItemViaName(stmtBlock *curBlock,char *name);
#endif