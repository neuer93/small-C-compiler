#ifndef _CODEGEN_H
#define _CODEGEN_H
#include "predefine.h"

typedef struct _tree treeNode;
typedef struct _funtable funSymTable;
typedef struct _strctable strcSymTable;
typedef struct _variabletable varSymTable;
typedef struct _funitem funSymTableItem;
typedef struct _strcitem strcSymTableitem;
typedef struct _varitem varSymTableItem;
typedef struct _block stmtBlock;
typedef struct _strcMem strcMem;


//struct list
typedef struct _definestructitem{
	char *name;
	strcSymTableitem *item;
} strcListItem, *pstrcListItem;

typedef struct _definestructlist{
	pstrcListItem *strcList;
	int strNum,cur;
} strcList;

strcList *strcDefineList;

//variable list
typedef struct _definevaritem{
	char *name;
	varSymTableItem *item;
} varListItem, *pVarListItem;

typedef struct _definevarlist{
	pVarListItem *varList;
	int varNum,cur,paraVarNum;
} varList,*pVarList;

typedef struct _definefunvarlist{
	pVarList *funVarList;
	int funNum,cur;
} funVarList;

funVarList *funVarDefineList;
int *curTempVar;

strcListItem* createNewStrcListItem(char* name,strcSymTableitem *item);
varList* createNewVarList(int varNum);
varListItem* createNewVarListItem(char* name,varSymTableItem *item);
int checkStrcNameCG(char *name);
int checkVarNameCG(char *name,varList *curList);
int checkBinaryNum(char* op);
int checkRealtionNum(char* op);
void fdg();
void tdb();
void checkg_d();

// store code

typedef struct _codeline
{
	char *code;
	struct _codeline *next;
	int label[2];
} codeLine;

typedef struct _codeblock
{
	codeLine *head;
	codeLine *tail;
} codeBlock;

typedef struct _labeltableitem
{
	codeLine *targetLine;
	struct _labeltableitem *next;
	int labelNum;
} labelTableItem;

typedef struct _labeltable
{
	labelTableItem *head;
	labelTableItem *tail;
} labelTable;

labelTable* retTable;

codeLine* createNewCodeLine(char *code);
codeBlock* createNewCodeBlock();
labelTableItem* createNewLabelTableItem(codeLine *targetLine,int labelNum);
labelTable* createNewLabelTable();
void addCodeLineToBlock(codeBlock *block,codeLine *line);
void addCodeBlockToBlock(codeBlock *extBlock,codeBlock *interBlock);
void addLableItemToTable(labelTable *table,labelTableItem *item);
int backPatching(labelTable *table,int labelNum);

void codeGenerator();
void iniStrc();
void iniVar();
void outputDefInitial(treeNode *defs);
void outputDecInitial(treeNode *decs);
void loadStrcInfo(stmtBlock *scopeBlock);
void loadVarInfo(stmtBlock *scopeBlock,varList *curList);
void loadVarInfoIni(stmtBlock *scopeBlock,varList *curList);
void outputStrcCG();
void outputStrcMemCG(strcMem* member);
void outputGlobalVarCG();
void outputFunCG();
void outputFunParaCG(funSymTableItem *item);
void outputVarCG(int index);
void outputParaVarIniCG(int index);

void parseStmtBlockCG(treeNode *stmtblock,char *continueLabel,char *breakLabel);
void parseStmtCG(treeNode *stmt,char *continueLabel,char *breakLabel);
int parseExprCG(treeNode *expr,char *trueLable,char *falseLable);

char* getTypeNameCG(stmtBlock *scopeBlock,char *type);
char* getVarNameCG(varSymTableItem *item);
char* getVarPtrCG(treeNode *expr);


#endif