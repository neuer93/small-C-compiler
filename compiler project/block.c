#include "block.h"
#include "predefine.h"

extern stmtBlock *currentBlock;
extern treeNode* rootNode;

void setBlockChild(stmtBlock *father,stmtBlock *child){
	if (father->leftChild == NULL)
	{
		father->leftChild = child;
		return;
	}
	setBlockBrother(father->leftChild,child);
}

void setBlockBrother(stmtBlock *older,stmtBlock *younger){
	if (older->rightBrother == NULL)
	{
		older->rightBrother = younger;
		return;
	}
	setBlockBrother(older->rightBrother,younger);
}

stmtBlock* getNthBlockChild(stmtBlock *father,int n){
	if (n <= 0)	return NULL;
    int temp = n;
    stmtBlock *p = father->leftChild;
    while(temp > 1){
        --temp;
        p = p->rightBrother;
        if (p == NULL)
        {
            return NULL;
        }
    }
    return p;
}

varSymTableItem* getVarItemViaName(stmtBlock *curBlock,char *name){
	int length = curBlock->varTable->varNum;
	int i;
	for (i = 0; i < length; ++i)
		if(strcmp(curBlock->varTable->varList[i]->name,name) == 0)
			return curBlock->varTable->varList[i];
	getVarItemViaName(curBlock->father,name);
}

strcSymTableitem* getStrcItemViaName(stmtBlock *curBlock,char *name){
	int length = curBlock->strcTable->strcNum;
	int i;
	for (i = 0; i < length; ++i)
		if(strcmp(curBlock->strcTable->strcList[i]->name,name) == 0)
			return curBlock->strcTable->strcList[i];
	getStrcItemViaName(curBlock->father,name);
}

stmtBlock* createExtenalBlock(){
	stmtBlock *extBlock = (stmtBlock*)malloc(sizeof(stmtBlock));
	currentBlock = extBlock;
	extBlock->father = NULL;
	extBlock->leftChild = NULL;
	extBlock->rightBrother = NULL;
	extBlock->isExternal = TRUE;
	extBlock->stmt = NULL;

	anonStrcNum = 0;

	int funNum = countFunNum(rootNode->leftChild);
	extBlock->funTable = createNewFunSymTable(funNum);
	int strcNum = countStrcNum(rootNode->leftChild);
	extBlock->strcTable = createNewStrcSymTable(strcNum);
	int varNum = countVarNum(rootNode->leftChild);
	extBlock->varTable = createNewVarSymTable(varNum);
	treeNode *p = rootNode->leftChild;
	while(strcmp(p->leftChild->name,"e") != 0){
		treeNode *extdef = p->leftChild;
		if (checkTreeNodeFun(extdef)) {
			addFunInfo(extBlock->funTable,extdef);
			treeNode *stmtblock = getNthNodeChild(extdef,3);
			stmtBlock *childBlock = createBlock(extBlock,stmtblock,
				extBlock->funTable->funList[extBlock->funTable->cur - 1]);
			currentBlock = extBlock;
			setBlockChild(extBlock,childBlock);
		}
		if (checkTreeNodeStrc(extdef))
		{
			addStrcInfo(extBlock->strcTable,extdef);
		}
		if (checkTreeNodeVar(extdef))
		{
			addVarInfo(extBlock->varTable,extdef);
		}
		p = getNthNodeChild(p,2);
	}
	//outputFunTable(extBlock->funTable);
	//outputStrcTable(extBlock->strcTable);
	//outputVarTable(extBlock->varTable);
	return extBlock;
}

stmtBlock* createBlock(stmtBlock *father,treeNode *stmtblock,funSymTableItem *funItem){
	stmtBlock *newBlock = (stmtBlock*)malloc(sizeof(stmtBlock));
	newBlock->father = father;
	newBlock->leftChild = NULL;
	newBlock->rightBrother = NULL;
	newBlock->isExternal = FALSE;
	newBlock->funTable = NULL;
	newBlock->stmt = stmtblock;
	currentBlock = newBlock;
	stmtblock->block = currentBlock;

	treeNode *defs = getNthNodeChild(stmtblock,2);
	treeNode *stmts = getNthNodeChild(stmtblock,3);
	int strcNum = countStrcNumI(defs); 
	int varNum = countVarNumI(defs);
	if (funItem != NULL){
		int paraNum = funItem->paraNum;
		newBlock->varTable = createNewVarSymTable(varNum+paraNum);
		newBlock->varTable->varList[0];
		addFunParaToVarTable(funItem,newBlock->varTable);
	}else{
		newBlock->varTable = createNewVarSymTable(varNum);
	}

	newBlock->strcTable = createNewStrcSymTable(strcNum);
	treeNode *p = getNthNodeChild(stmtblock,2);
	while(strcmp(p->leftChild->name,"e") != 0){
		treeNode *def = p->leftChild;
		if (checkTreeNodeStrcI(def))
		{
			addStrcInfoI(newBlock->strcTable,def);
		}
		addVarInfoI(newBlock->varTable,def);
		p = getNthNodeChild(p,2);
	}
	while(strcmp(stmts->leftChild->name,"e") != 0){
		treeNode *stmt = stmts->leftChild;
		parseStmt(newBlock,stmt);
		stmts = getNthNodeChild(stmts,2);
	}
	//outputStrcTable(newBlock->strcTable);
	//outputVarTable(newBlock->varTable);
	return newBlock;
}

void parseStmt(stmtBlock *scopeBlock,treeNode *stmt){
	treeNode *expr;
	if (strcmp(stmt->leftChild->name,"EXP") == 0){
		typeCheckExpr(stmt->leftChild,"top");
	}
	if (strcmp(stmt->leftChild->name,"STMTBLOCK") == 0){
		treeNode *childstmtblock = stmt->leftChild;
		stmtBlock *childBlock = createBlock(scopeBlock,childstmtblock,NULL);
		setBlockChild(scopeBlock,childBlock);
		currentBlock = scopeBlock;
	}
	if (strcmp(stmt->leftChild->name,"RET") == 0){
		expr = getNthNodeChild(stmt,2);
		typeCheckExpr(expr,"int");
	}
	if (strcmp(stmt->leftChild->name,"IF") == 0){
		expr = getNthNodeChild(stmt,3);
		typeCheckExpr(expr,"int");
		parseStmt(scopeBlock,getNthNodeChild(stmt,5));
		treeNode *estmt = getNthNodeChild(stmt,6);
		if (strcmp(estmt->leftChild->name,"e") != 0)
		{
			parseStmt(scopeBlock,getNthNodeChild(estmt,2));
		}
	}
	if (strcmp(stmt->leftChild->name,"FOR") == 0){
		expr = getNthNodeChild(stmt,3);
		typeCheckExpr(expr,"top");
		expr = getNthNodeChild(stmt,5);
		typeCheckExpr(expr,"int");
		expr = getNthNodeChild(stmt,7);
		typeCheckExpr(expr,"top");
		parseStmt(scopeBlock,getNthNodeChild(stmt,9));
	}
	if (strcmp(stmt->leftChild->name,"CONT") == 0){
		//nothing to do
	}
	if (strcmp(stmt->leftChild->name,"BREAK") == 0){
		//nothing to do
	}
	if (strcmp(stmt->leftChild->name,"READ") == 0){
		/* code */
	}
	if (strcmp(stmt->leftChild->name,"WRITE") == 0){
		expr = getNthNodeChild(stmt,3);
		typeCheckExpr(expr,"int");
	}
}

int countAllStrcNum(stmtBlock *scopeBlock){
	if (scopeBlock == NULL) return 0;
	int res = scopeBlock->strcTable->strcNum;
	res += countAllStrcNum(scopeBlock->leftChild);
	res += countAllStrcNum(scopeBlock->rightBrother);
	return res;
}

int countAllVarNum(stmtBlock *scopeBlock){
	if (scopeBlock == NULL) return 0;
	int res = scopeBlock->varTable->varNum;
	res += countAllVarNum(scopeBlock->leftChild);
	res += countAllVarNum(scopeBlock->rightBrother);
	return res;
}