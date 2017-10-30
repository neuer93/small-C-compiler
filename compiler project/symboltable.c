#include "symboltable.h"

extern stmtBlock *currentBlock;
//for debug
void outputFunTable(funSymTable* funTable){
	int i = 0;
	for (; i < funTable->funNum; ++i)
	{
		pFunSymTableItem item = funTable->funList[i];
		fprintf(stderr, "%s %s(", item->retType,item->name);
		int j = 0;
		for (; j < item->paraNum; ++j)
		{
			pParaItem para = item->paraList[j];
			fprintf(stderr, "%s %s,", para->type,para->name);
		}
		fprintf(stderr, ")\n");
	}
	fprintf(stderr, "\n");
}

void outputStrcTable(strcSymTable* strcTable){
	int i = 0;
	for (; i < strcTable->strcNum; ++i)
	{
		pStrcSymTableitem item = strcTable->strcList[i];
		fprintf(stderr, "struct %s %d{",item->name,item->memNum);
		int j = 0;
		for (; j < item->memNum; ++j)
		{
			pStrcMem mem = item->memList[j];
			fprintf(stderr, "%s %s", mem->type,mem->name);
			if(mem->arrNum > 0) fprintf(stderr, "[%d],", mem->arrList[0]);
			else fprintf(stderr, ",");
		}
		fprintf(stderr, "}\n");
	}
	fprintf(stderr, "----\n");
}

void outputVarTable(varSymTable* varTable){
	int i = 0;
	for (; i < varTable->varNum; ++i)
	{
		pVarSymTableItem item = varTable->varList[i];
		fprintf(stderr, "%s %s", item->type,item->name);
		if(item->arrNum != -1) fprintf(stderr, "[%d]",item->arrNum);
		fprintf(stderr, "\n");
	}
	fprintf(stderr, "~~~~\n");
}

//fun

funSymTable* createNewFunSymTable(int num){
	funSymTable* temp = (funSymTable*)malloc(sizeof(funSymTable));

	temp->funNum = num;
	temp->cur = 0;
	temp->funList = (pFunSymTableItem*)malloc(sizeof(pFunSymTableItem) * num);

	return temp;
}

funSymTableItem* createNewFunSymTableItem(char *name,char *retType,int paraNum){
	pFunSymTableItem temp = (pFunSymTableItem)malloc(sizeof(funSymTableItem));

	temp->name = strdup(name);
	temp->retType = strdup(retType);
	temp->paraNum = paraNum;
	temp->cur = 0;
	temp->paraList = (pParaItem*)malloc(sizeof(paraItem) * paraNum);

	return  temp;
}

paraItem* createNewParaItem(char *type,char *name){
	paraItem* temp = (paraItem*)malloc(sizeof(paraItem));

	temp->type = type;
	temp->name = name;

	return temp;
}

void addParaInfo(funSymTableItem* curFunItem,treeNode* paras){
	treeNode *curParas = paras;
	treeNode *para,*temp,*spec;
	char *type,*name;
	while(curFunItem->cur < curFunItem->paraNum){
		para = paras->leftChild;
		spec = para->leftChild;
		type = strdup(getTypeName(spec));
		
		checkTypeAvailableGlobal(type);

		temp = getNthNodeChild(para,2);
		name = strdup(temp->leftChild->val);
		
		checkRedefinePara(curFunItem,name);

		curFunItem->paraList[curFunItem->cur] = createNewParaItem(type,name);
		++(curFunItem->cur);
		paras = getNthNodeChild(paras,3);
	}
}

void addFunInfo(funSymTable* curFunTable,treeNode* extdef){
	char *retType,*name;
	treeNode *temp,*fun;
	int paraNum;
	fun = getNthNodeChild(extdef,2);
	retType = "int";
	treeNode *paras = getNthNodeChild(fun,3);
	paraNum = countParaNum(paras);
	name = strdup(fun->leftChild->val);
	
	checkRedefineFun(curFunTable,name);

	curFunTable->funList[curFunTable->cur] = 
		createNewFunSymTableItem(name,retType,paraNum);
	addParaInfo(curFunTable->funList[curFunTable->cur],paras);
	++(curFunTable->cur);
}


//struct

strcSymTable* createNewStrcSymTable(int num){
	strcSymTable* temp = (strcSymTable*)malloc(sizeof(strcSymTable));

	temp->strcNum = num;
	temp->cur = 0;
	temp->strcList = (pStrcSymTableitem*)malloc(sizeof(pStrcSymTableitem) * num);

	return temp;
}

strcSymTableitem* createNewStrcSymTableItem(char *name,int memNum){
	pStrcSymTableitem temp = (pStrcSymTableitem)malloc(sizeof(strcSymTableitem));

	temp->name = strdup(name);
	temp->memNum = memNum;
	temp->cur = 0;
	temp->redefine = -1;
	temp->memList = (pStrcMem*)malloc(sizeof(strcMem) * memNum);

	return  temp;
}


strcMem* createNewStrcMem(char *type,char *name,int arrNum){
	strcMem* temp = (strcMem*)malloc(sizeof(strcMem));

	temp->type = type;
	temp->name = name;
	temp->arrNum = arrNum;
	temp->cur = arrNum;
	temp->arrList = (int*)malloc(sizeof(int) * arrNum);

	return temp;
}

void addStrcVarInfo(strcMem *curStrcVar,treeNode *var){
	if (curStrcVar->cur == 0)
	{
		return;	
	}
	treeNode *intNode = getNthNodeChild(var,3);
	curStrcVar->arrList[curStrcVar->cur] = atoi(intNode->val);
	--(curStrcVar->cur);
	addStrcVarInfo(curStrcVar,var->leftChild);
}

void addStrcMemInfo(strcSymTableitem* curStrcItem,treeNode* strcdefs){
	if (strcmp(strcdefs->leftChild->name,"e") == 0)
	{
		return;
	}
	treeNode *strcdef = strcdefs->leftChild,*temp;
	char *type = "int";
	treeNode *strcdecs = getNthNodeChild(strcdef,2);
	while(strcdecs != NULL){
		int arrNum = countVarArrNum(strcdecs->leftChild);
		char *name = getVarName(strcdecs->leftChild);

		checkRedefineStrcMem(curStrcItem,name);

		curStrcItem->memList[curStrcItem->cur] = createNewStrcMem(type,name,arrNum);
		addStrcVarInfo(curStrcItem->memList[curStrcItem->cur],strcdecs->leftChild);
		strcdecs = getNthNodeChild(strcdecs,3);
		++(curStrcItem->cur);
	}
	addStrcMemInfo(curStrcItem,getNthNodeChild(strcdefs,2));
}

void addStrcInfo(strcSymTable* curStrcTable,treeNode* extdef){
	char *name = "struct.";
	treeNode *stspec;
	int memNum;
	stspec = extdef->leftChild->leftChild;
	name = getTypeName(extdef->leftChild);

	checkRedefineStrc(curStrcTable,name);

	treeNode *strcdefs = getNthNodeChild(stspec,4);
	memNum = countMemNumStrcdefs(strcdefs);
	curStrcTable->strcList[curStrcTable->cur] = 
		createNewStrcSymTableItem(name,memNum);
	addStrcMemInfo(curStrcTable->strcList[curStrcTable->cur],strcdefs);
	++(curStrcTable->cur);
}

void addStrcInfoI(strcSymTable* curStrcTable,treeNode* def){
	char *name = "struct.";
	treeNode *stspec;
	int memNum;
	stspec = def->leftChild->leftChild;
	name = getTypeName(def->leftChild);

	checkRedefineStrc(curStrcTable,name);

	treeNode *strcdefs = getNthNodeChild(stspec,4);
	memNum = countMemNumStrcdefs(strcdefs);
	curStrcTable->strcList[curStrcTable->cur] = 
		createNewStrcSymTableItem(name,memNum);
	addStrcMemInfo(curStrcTable->strcList[curStrcTable->cur],strcdefs);
	++(curStrcTable->cur);
}

//Variable
varSymTable* createNewVarSymTable(int num){
	varSymTable *temp = (varSymTable *)malloc(sizeof(varSymTable));
	
	temp->varNum = num;
	temp->cur = 0;
	temp->varList = (pVarSymTableItem *)malloc(sizeof(pVarSymTableItem)  * num);

	return temp;
}

varSymTableItem* createNewVarSymTableItem(char *type,char *name,int arrNum){
	varSymTableItem *temp = (varSymTableItem *)malloc(sizeof(varSymTableItem));

	temp->type = type;
	temp->name = name;
	temp->arrNum = arrNum;
	temp->redefine = -1;
	temp->isPara = -1;
	temp->init = NULL;

	return temp;
}

void addVarInfo(varSymTable* curVarTable,treeNode* extdef){
	char *name,*type;
	int arrNum;
	treeNode *temp;
	if (strcmp(extdef->leftChild->leftChild->name,"TYPE") == 0){
	 	type = strdup("int");
	} else {
	 	temp = getNthNodeChild(extdef->leftChild->leftChild,2);
	 	type = "struct.";
	 	if (strcmp(temp->name,"ID") == 0)
	 	{
		 	type = addString(type, temp->val);
	 	}else{
	 		type = addString(type, temp->leftChild->val);
	 	}
	 	
	 	checkTypeAvailableGlobal(type);

	}
	treeNode *extvars = getNthNodeChild(extdef,2);
	if (strcmp(extvars->leftChild->name,"e") == 0)
	{
		return;
	}
	while(extvars != NULL){
		char *id = extvars->leftChild->leftChild->leftChild->name;
		if(strcmp(id,"ID") == 0){
			name = extvars->leftChild->leftChild->leftChild->val;
			arrNum = -1;
		}else{
			treeNode *varNode = extvars->leftChild->leftChild;
			name = varNode->leftChild->leftChild->val;
			temp = getNthNodeChild(varNode,3);
			arrNum = atoi(temp->val);
		}
		
		checkRedefineVar(curVarTable,name);

		curVarTable->varList[curVarTable->cur] = 
			createNewVarSymTableItem(type,name,arrNum);

		treeNode *dec = extvars->leftChild;
		treeNode *temp1 = getNthNodeChild(dec,3);
		if(temp1 != NULL){
			curVarTable->varList[curVarTable->cur]->init = temp1;
		}
		++(curVarTable->cur);
		extvars = getNthNodeChild(extvars,3);
	}
}

void addFunParaToVarTable(funSymTableItem *funItem,varSymTable* curVarTable){
	int  i;
	for (i = 0; i < funItem->paraNum; ++i)
	{
		curVarTable->varList[i] = createNewVarSymTableItem(funItem->paraList[i]->type,funItem->paraList[i]->name,-1);
		curVarTable->varList[i]->isPara = i;
	}
	curVarTable->cur = funItem->paraNum;
}

void addVarInfoI(varSymTable* curVarTable,treeNode* def){
	char *name,*type;
	int arrNum;
	treeNode *temp;
	type = getTypeName(def->leftChild);

	checkTypeAvailableGlobal(type);

	treeNode *decs = getNthNodeChild(def,2);
	while(decs != NULL){
		char *id = decs->leftChild->leftChild->leftChild->name;
		if(strcmp(id,"ID") == 0){
			name = decs->leftChild->leftChild->leftChild->val;
			arrNum = -1;
		}else{
			treeNode *varNode = decs->leftChild->leftChild;
			name = varNode->leftChild->leftChild->val;
			temp = getNthNodeChild(varNode,3);
			arrNum = atoi(temp->val);
		}
		
		checkRedefineVar(curVarTable,name);

		curVarTable->varList[curVarTable->cur] = 
			createNewVarSymTableItem(type,name,arrNum);
		curVarTable->varList[curVarTable->cur]->defineBlock = currentBlock;
		++(curVarTable->cur);
		decs = getNthNodeChild(decs,3);
	}
}

int getIndexViaMemName(strcSymTableitem* strc,char *name){
	int length = strc->memNum;
	int i;
	for (i = 0; i < length; ++i)
		if(strcmp(strc->memList[i]->name,name) == 0) return i;
	return -1;
}