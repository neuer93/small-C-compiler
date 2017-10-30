#ifndef _TYPECHECK_H
#define _TYPECHECK_H

typedef struct _tree treeNode;
typedef struct _funtable funSymTable;
typedef struct _strctable strcSymTable;
typedef struct _variabletable varSymTable;
typedef struct _funitem funSymTableItem;
typedef struct _strcitem strcSymTableitem;
typedef struct _varitem varSymTableItem;
typedef struct _block stmtBlock;

void checkRedefinePara(funSymTableItem* curFunItem,char *name);
void checkRedefineFun(funSymTable* curFunTable,char *name);
void checkRedefineStrcMem(strcSymTableitem* curStrcItem,char *name);
void checkRedefineStrc(strcSymTable* curStrcTable,char *name);
void checkRedefineVar(varSymTable* curVarTable,char *name);

void checkTypeAvailableGlobal(char *name);
int checkTypeAvailableBlock(strcSymTable* curStrcTable,char *name);

void checkVarAvailableGlobal(char *name);
int checkVarAvailableBlock(varSymTable* curVarTable,char *name);

int isArrType(char *type);
int isStrcType(char *type);
char* arrTypeToType(char *arrType);
char* typeToArrType(char *arrType);
char* typeCheckExpr(treeNode *expr,char *type);

char* getVarType(char *name);
char* getIDTypeGlobal(char *name);
char* getIDTypeBlock(varSymTable *curVarTable,char *name);

#endif