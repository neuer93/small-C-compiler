#include "predefine.h"
#include "typecheck.h"

extern stmtBlock *currentBlock;

void checkRedefinePara(funSymTableItem* curFunItem,char *name){
	int length = curFunItem->cur;
	int i = 0;
	for (; i < length; ++i)
	{
		if (strcmp(name,curFunItem->paraList[i]->name) == 0)
			tcErrorOutput(1,"redefine");
	}
}

void checkRedefineFun(funSymTable* curFunTable,char *name){
	int length = curFunTable->cur;
	int i = 0;
	for (; i < length; ++i)
	{
		if (strcmp(name,curFunTable->funList[i]->name) == 0)
			tcErrorOutput(1,name);
	}
}

void checkRedefineStrcMem(strcSymTableitem* curStrcItem,char *name){
	int length = curStrcItem->cur;
	int i = 0;
	for (; i < length; ++i)
	{
		if (strcmp(name,curStrcItem->memList[i]->name) == 0)
			tcErrorOutput(1,name);
	}
}

void checkRedefineStrc(strcSymTable* curStrcTable,char *name){
	int length = curStrcTable->cur;
	int i = 0;
	for (; i < length; ++i)
	{
		if (strcmp(name,curStrcTable->strcList[i]->name) == 0)
			tcErrorOutput(1,name);
	}
}

void checkRedefineVar(varSymTable* curVarTable,char *name){
	int length = curVarTable->cur;
	int i = 0;
	for (; i < length; ++i)
	{
		if (strcmp(name,curVarTable->varList[i]->name) == 0)
			tcErrorOutput(1,name);
	}
}

void checkTypeAvailableGlobal(char *name){
	if (strcmp(name,"int") == 0)
	{
		return;
	}
	stmtBlock *tempBlock = currentBlock;
	while(tempBlock != NULL){
		if(checkTypeAvailableBlock(tempBlock->strcTable,name)) return;
		tempBlock = tempBlock->father;
	}
	tcErrorOutput(1,name);
}

int checkTypeAvailableBlock(strcSymTable* curStrcTable,char *name){
	int length = curStrcTable->cur;
	int i = 0;
	for (; i < length; ++i)
	{
		if (strcmp(name,curStrcTable->strcList[i]->name) == 0)
			return TRUE;
	}
	return FALSE;
}

void checkVarAvailableGlobal(char *name){
	stmtBlock *tempBlock = currentBlock;
	while(tempBlock != NULL){
		if(checkVarAvailableBlock(tempBlock->varTable,name)) return;
		tempBlock = tempBlock->father;
	}
	tcErrorOutput(1,name);
}
int checkVarAvailableBlock(varSymTable* curVarTable,char *name){
	int length = curVarTable->cur;
	int i = 0;
	for (; i < length; ++i)
	{
		if (strcmp(name,curVarTable->varList[i]->name) == 0)
			return TRUE;
	}
	return FALSE;
}

int isArrType(char *type){
	char *prefix = "arr-";
	if (strlen(type) < 4) return FALSE;
	int i;
	for (i = 0; i < 4; ++i)
		if (prefix[i] != type[i])
			return FALSE;
	return TRUE;
}

int isStrcType(char *type){
	char *prefix = "struct.";
	if (strlen(type) < 7) return FALSE;
	int i;
	for (i = 0; i < 7; ++i)
		if (prefix[i] != type[i])
			return FALSE;
	return TRUE;
}

char* arrTypeToType(char *arrType){
	return subString(arrType,4,strlen(arrType));
}

char* typeToArrType(char *arrType){
	return addString("arr-",arrType);
}

char* getIDTypeGlobal(char *name){
	stmtBlock *tempBlock = currentBlock;
	while(tempBlock != NULL){
		char *temp = getIDTypeBlock(tempBlock->varTable,name);
		if(strlen(temp) > 0) return temp;
		tempBlock = tempBlock->father;
	}
}

char* getIDTypeBlock(varSymTable *curVarTable,char *name){
	int length = curVarTable->cur;
	int i = 0;
	for (; i < length; ++i)
	{
		if (strcmp(name,curVarTable->varList[i]->name) == 0){
			if (curVarTable->varList[i]->arrNum == -1)
				return curVarTable->varList[i]->type;
			char *temp = typeToArrType(curVarTable->varList[i]->type);
			return temp;
		}
	}
	return "";
}

char* typeCheckExpr(treeNode *expr,char *type){	
	if (strcmp(expr->leftChild->name,"INT") == 0){
		if (strcmp(type,"top") == 0) return "int";
		if (strcmp(type,"int") == 0) return "int";
		tcErrorOutput(1,type);
	}

	if (strcmp(expr->leftChild->name,"e") == 0){
		if (strcmp(type,"top") == 0) return "e";
		if (strcmp(type,"e") == 0) return "e";
		tcErrorOutput(1,type);
	}
	treeNode *firstChild,*secondChild;
	firstChild = expr->leftChild;
	secondChild = expr->leftChild->rightBrother;

	if ((strcmp(secondChild->name,"EXP") == 0) 
			&& (strcmp(firstChild->name,"(") != 0)){
		typeCheckExpr(secondChild,"int");
		if (strcmp(type,"top") == 0) return "int";
		if (strcmp(type,"int") == 0) return "int";
		tcErrorOutput(1,type);
	}

	if (strcmp(firstChild->name,"(") == 0){
		char *tempType = typeCheckExpr(secondChild,type);
		if (strcmp(type,"top") == 0) return tempType;
		if (strcmp(type,tempType) == 0) return tempType;
		tcErrorOutput(1,type);
	}

	if ((strcmp(firstChild->name,"EXP") == 0) 
		&& (strcmp(secondChild->name,"=") != 0)
			&& (strcmp(secondChild->name,".") != 0)){
		typeCheckExpr(firstChild,"int");
		typeCheckExpr(getNthNodeChild(expr,3),"int");
		if (strcmp(type,"top") == 0) return "int";
		if (strcmp(type,"int") == 0) return "int";
		tcErrorOutput(1,type);
	}

	if (strcmp(secondChild->name,"=") == 0){
		treeNode *thirdChild = getNthNodeChild(expr,3);
		char *tempType1 = typeCheckExpr(firstChild,type);
		char *tempType2 = typeCheckExpr(thirdChild,type);
		if (strcmp(tempType1,tempType2) != 0) tcErrorOutput(1,type);
		if (strcmp(type,"top") == 0) return tempType1;
		if (strcmp(type,tempType1) == 0) return tempType1;
		tcErrorOutput(1,type);
	}

	if (strcmp(secondChild->name,"ARRS") == 0){
		char *name = firstChild->val;
		checkVarAvailableGlobal(name);
		char *IDtype = getIDTypeGlobal(name);
		int arr = isArrType(IDtype);
		if (strcmp(secondChild->leftChild->name,"e") == 0){
			if (arr == TRUE) tcErrorOutput(1,type);
		}
		if (strcmp(secondChild->leftChild->name,"e") != 0){
			if (arr == FALSE) tcErrorOutput(1,type);
		}
		if (arr == FALSE){
			if (strcmp(type,"top") == 0) return IDtype;
			if (strcmp(type,IDtype) == 0) return IDtype;
			tcErrorOutput(1,type);
		}

		typeCheckExpr(getNthNodeChild(secondChild,2),"int");
		char *tempType = arrTypeToType(IDtype);
		if (strcmp(type,"top") == 0) return tempType;
		if (strcmp(type,tempType) == 0) return tempType;
		tcErrorOutput(1,type);
	}
	if (strcmp(secondChild->name,".") == 0){
		if (strcmp(firstChild->leftChild->name,"ID") != 0) tcErrorOutput(1,type);
		char *name = firstChild->leftChild->val;
		checkVarAvailableGlobal(name);
		char *IDtype = getIDTypeGlobal(name);
		if(isStrcType(IDtype) == FALSE) tcErrorOutput(1,type);
		//check mem in strc
		//todo
		if (strcmp(type,"top") == 0) return "int";
		if (strcmp(type,"int") == 0) return "int";
		tcErrorOutput(1,type);
	}

	if (strcmp(secondChild->name,"(") == 0){
		//check fun type
		//todo
		if (strcmp(type,"top") == 0) return "int";
		if (strcmp(type,"int") == 0) return "int";
		tcErrorOutput(1,type);
	}

}