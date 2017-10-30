#ifndef _SYMBOLTABLE_H
#define _SYMBOLTABLE_H
#include "predefine.h"

extern anonStrcNum;

typedef struct _tree treeNode;
typedef struct _block stmtBlock;

// function symbol table
typedef struct _paraitem
{
	char *type;
	char *name;	
} paraItem,*pParaItem;

typedef struct _funitem
{
	char *name;
	char *retType;
	int paraNum,cur;
	pParaItem *paraList;
} funSymTableItem, *pFunSymTableItem;

typedef struct _funtable
{
	int  funNum,cur;
	pFunSymTableItem *funList;
} funSymTable;

funSymTable* createNewFunSymTable(int num);
funSymTableItem* createNewFunSymTableItem(char *name,char *retType,int paraNum);
paraItem* createNewParaItem(char *type,char *name);

void addFunInfo(funSymTable* curFunTable,treeNode* extdef);
void addParaInfo(funSymTableItem* curFunItem,treeNode* paras);

//struct symbol table
typedef struct _strcMem
{
	char *type;
	char *name;
	int arrNum,cur;
	int *arrList;
} strcMem, *pStrcMem;

typedef struct _strcitem
{
	char *name;
	int memNum,cur,redefine;
	pStrcMem *memList;
} strcSymTableitem, *pStrcSymTableitem;

typedef struct _strctable
{
	int strcNum,cur;
	pStrcSymTableitem *strcList;
} strcSymTable;

strcSymTable* createNewStrcSymTable(int num);
strcSymTableitem* createNewStrcSymTableItem(char *name,int memNum);
strcMem* createNewStrcMem(char *type,char *name,int arrNum);

void addStrcInfo(strcSymTable* curStrcTable,treeNode* extdef);
void addStrcMemInfo(strcSymTableitem* curStrcItem,treeNode* strcdefs);
void addStrcVarInfo(strcMem *curStrcVar,treeNode *var);
int getIndexViaMemName(strcSymTableitem* strc,char *name);

void addStrcInfoI(strcSymTable* curStrcTable,treeNode* def);

//variable symbol table
typedef struct _varitem
{
	char *type;
	char *name;
	int arrNum;
	int redefine;
	int isPara;
	treeNode *init;
	stmtBlock *defineBlock;
} varSymTableItem,*pVarSymTableItem;

typedef struct _variabletable
{
	int  varNum,cur;
	pVarSymTableItem *varList;
} varSymTable;

varSymTable* createNewVarSymTable(int num);
varSymTableItem* createNewVarSymTableItem(char *type,char *name,int arrNum);
void addVarInfo(varSymTable* curVarTable,treeNode* extdef);
void addFunParaToVarTable(funSymTableItem *funItem,varSymTable* curVarTable);
void addVarInfoI(varSymTable* curVarTable,treeNode* def);

//for debug
void outputFunTable(funSymTable* funTable);
void outputStrcTable(strcSymTable* strcTable);
void outputVarTable(varSymTable* varTable);
#endif
