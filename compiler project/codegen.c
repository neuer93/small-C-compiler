#include "codegen.h"

extern int g_structRedefine;
extern int g_varRedefine;
extern stmtBlock *currentBlock;
extern stmtBlock *globalBlock;
extern FILE* fout;

extern strcList *strcDefineList;
extern funVarList *funVarDefineList;
extern int *curTempVar;
int g_tempVar;
int g_labelNum;
int g_d;

void checkg_d(){
	if(g_d == FALSE) ++g_tempVar;
}

void tdg(){
	g_d = TRUE;
}

void fdg(){
	g_d = FALSE;
}

char *inputOutputInitial = "@.str = private unnamed_addr constant [3 x i8] c\"%d\\00\", align 1\ndeclare i32 @printf(i8*, ...)\ndeclare i32 @__isoc99_scanf(i8*, ...)\n";

char *outputM = "  %%%d = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([3 x i8]* @.str, i32 0, i32 0), i32 %%%d)\n";
char *outputMC = "  %%%d = call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([3 x i8]* @.str, i32 0, i32 0), i32 %d)\n";
char *inputM = "  %%%d = call i32 (i8*, ...)* @__isoc99_scanf(i8* getelementptr inbounds ([3 x i8]* @.str, i32 0, i32 0), i32* %s)\n";
char *llvmOpBianryNum[10]={
	"add nsw", //+
	"sub nsw", //-
	"mul nsw", //*
	"sdiv",// /
	"and",// &
	"xor",// ^
	"or",// |
	"srem",// %
	"shl",// << SL
	"ashr"// >> SR
};

char *llvmRelationNum[6]={
	"slt", //<
	"sgt", //> 
	"ne", //!=  NE
	"eq", //== EQ
	"sle",// <= NG
	"sge"// >= NL
};

char *llvmOpAssignNum[9]={
	"add nsw", //+= PA
	"sub nsw", //-= SA
	"mul nsw", //*= MA
	"sdiv",// /= DA
	"and",// &= AA
	"xor",// ^= NA
	"or",// |= OA
	"shl",// <<= SLA
	"ashr"// >>= SRA
};

char *retLabel = "label.ret";


char* getTypeNameCG(stmtBlock *scopeBlock,char *type){
	if (strcmp(type,"int") == 0) return "int";
	int i;
	strcSymTable *curStrcTable= scopeBlock->strcTable;
	for (i = 0; i < curStrcTable->strcNum; ++i)
		if (strcmp(curStrcTable->strcList[i]->name,type) == 0){
			if(curStrcTable->strcList[i]->redefine == -1) return type;
			char buf[20];
			sprintf(buf,".%d",curStrcTable->strcList[i]->redefine);
			return addString(type,buf);
		}
	return getTypeNameCG(scopeBlock->father,type);
}

char* getVarNameCG(varSymTableItem *item){
	char *res;
	if (item->redefine == -1) res = item->name;
	else {
		char buf[20];
		sprintf(buf,".%d",item->redefine);
		res = addString(item->name,buf);
	}
	if(item->defineBlock == NULL) return addString("@",res);
		else return addString("%",res);
}

char* getVarPtrCG(treeNode *expr){
	varSymTableItem *item;
	treeNode *op = getNthNodeChild(expr,2);
	char *name;
	if(strcmp(op->name,".") == 0){
		item = getVarItemViaName(currentBlock,expr->leftChild->leftChild->val);
		name = getVarNameCG(item);
		strcSymTableitem *strcItem = getStrcItemViaName(currentBlock,item->type);
		treeNode *memNode = getNthNodeChild(expr,3);
		char *memName = memNode->val;
		int memIndex = getIndexViaMemName(strcItem,memName);
		char *typeName = getTypeNameCG(currentBlock,item->type);
		name = getVarNameCG(item);
		char buf[100];
		sprintf(buf,"getelementptr inbounds (%%%s* %s, i32 0, i32 %d)",
				typeName,name,memIndex);
		return strdup(buf);
	}
	item = getVarItemViaName(currentBlock,expr->leftChild->val);
	name = getVarNameCG(item);
	if(item->isPara >= 0){
		char buf[20];
		sprintf(buf,"%%%d",item->isPara + 2);
		return strdup(buf);
	}
	if(item->arrNum < 0){		
		return name;
	}else{
		treeNode *temp = getNthNodeChild(expr,2);
		treeNode *tempExp = getNthNodeChild(temp,2);
		int reg;
		if(isConstant(tempExp)){
			++g_tempVar;
			reg = evalConstant(tempExp);
			fprintf(fout, "  %%%d = getelementptr inbounds [%d x i32]* %s, i32 0, i64 %d\n", g_tempVar,
				item->arrNum,name,reg);
		}else{
			reg = parseExprCG(tempExp,"","");
			++g_tempVar;
			fprintf(fout, "  %%%d = sext i32 %%%d to i64\n", g_tempVar,reg);
			reg = g_tempVar;
			++g_tempVar;
			fprintf(fout, "  %%%d = getelementptr inbounds [%d x i32]* %s, i32 0, i64 %%%d\n", g_tempVar,
				item->arrNum,name,reg);
		}
		char buf[20];
		sprintf(buf,"%%%d",g_tempVar);
		return strdup(buf);
	}
}

void outputStrcMemCG(strcMem* member){
	if (member->arrNum == 0) fprintf(fout, " i32");
		else{
			fprintf(fout, " [%d x i32]", member->arrList[1]);
		}
}

void outputDefInitial(treeNode *defs){
	if(strcmp(defs->leftChild->name,"e") == 0) return;
	treeNode *decs = getNthNodeChild(defs->leftChild,2);
	outputDecInitial(decs);
	outputDefInitial(getNthNodeChild(defs,2));
}

void outputDecInitial(treeNode *decs){
	treeNode *init = getNthNodeChild(decs->leftChild,3);
	treeNode *expr,*temp;
	if(init != NULL){
		treeNode* dec = decs->leftChild;
		treeNode *var = dec->leftChild;
		if(strcmp(var->leftChild->name,"VAR") == 0){
			treeNode *args = getNthNodeChild(init,2);
			int length = countArgs(args),i,reg;
			temp = getNthNodeChild(var,3);
			int ll = atoi(temp->val);
			for(i = 0;i<length;++i){
				expr = getExpInArgs(args,i);
				reg = evalConstant(expr);
				fprintf(fout, "  %%%d = getelementptr inbounds [%d x i32]* %%%s, i32 0, i64 %d\n", ++g_tempVar,
				ll,var->leftChild->leftChild->val,i);
				fprintf(fout, "  store i32 %d, i32* %%%d\n", reg,g_tempVar);
			}
		}else{
			int reg = evalConstant(init->leftChild);
			fprintf(fout, "  store i32 %d, i32* %%%s\n", reg,var->leftChild->val);
		}
	}
	temp = getNthNodeChild(decs,3);
	if(temp == NULL) return;
	outputDecInitial(temp);
}

int checkBinaryNum(char* op){
	if(strcmp(op,"+") == 0) return 0;
	if(strcmp(op,"-") == 0) return 1;
	if(strcmp(op,"*") == 0) return 2;
	if(strcmp(op,"/") == 0) return 3;
	if(strcmp(op,"&") == 0) return 4;
	if(strcmp(op,"^") == 0) return 5;
	if(strcmp(op,"|") == 0) return 6;
	if(strcmp(op,"%") == 0) return 7;
	if(strcmp(op,"SL") == 0) return 8;
	if(strcmp(op,"SR") == 0) return 9;
	return -1;
}

int checkAssignNum(char* op){
	if(strcmp(op,"PA") == 0) return 0;
	if(strcmp(op,"SA") == 0) return 1;
	if(strcmp(op,"MA") == 0) return 2;
	if(strcmp(op,"DA") == 0) return 3;
	if(strcmp(op,"AA") == 0) return 4;
	if(strcmp(op,"NA") == 0) return 5;
	if(strcmp(op,"OA") == 0) return 6;
	if(strcmp(op,"SLA") == 0) return 7;
	if(strcmp(op,"SRA") == 0) return 8;
	return -1;
}

int checkRealtionNum(char* op){
	if(strcmp(op,"<") == 0) return 0;
	if(strcmp(op,">") == 0) return 1;
	if(strcmp(op,"NE") == 0) return 2;
	if(strcmp(op,"EQ") == 0) return 3;
	if(strcmp(op,"NG") == 0) return 4;
	if(strcmp(op,"NL") == 0) return 5;
	return -1;
}

int checkStrcNameCG(char *name){
	int i;
	for (i = 0; i < strcDefineList->cur; ++i)
		if (strcmp(strcDefineList->strcList[i]->name,name) == 0){
			return FALSE;
		}
	return TRUE;
}

int checkVarNameCG(char *name,varList *curList){
	int i;
	for (i = 0; i < curList->cur; ++i)
		if (strcmp(curList->varList[i]->name,name) == 0){
			return FALSE;
		}
	return TRUE;
}

strcListItem* createNewStrcListItem(char* name,strcSymTableitem *item){
	strcListItem *temp = (strcListItem*)malloc(sizeof(strcListItem));

	temp->name = name;
	temp->item = item;

	return temp;
}

varList* createNewVarList(int varNum){
	varList *temp = (varList*)malloc(sizeof(varList));

	temp->varList = (pVarListItem*)malloc(sizeof(pVarListItem) * varNum);
	temp->varNum = varNum;
	temp->cur = 0;

	return temp;
}

varListItem* createNewVarListItem(char* name,varSymTableItem *item){
	varListItem *temp = (varListItem*)malloc(sizeof(varListItem));

	temp->name = name;
	temp->item = item;

	return temp;
}

void loadStrcInfo(stmtBlock *scopeBlock){
	if (scopeBlock == NULL) return;
	strcSymTable *curStrcTable = scopeBlock->strcTable;
	int i;
	for (i = 0; i < curStrcTable->strcNum; ++i)
	{
		char *name = curStrcTable->strcList[i]->name;
		strcDefineList->strcList[strcDefineList->cur] = createNewStrcListItem(name,curStrcTable->strcList[i]);
		if (checkStrcNameCG(name) == FALSE){
			++g_structRedefine;
			curStrcTable->strcList[i]->redefine = g_structRedefine;
		}
		++(strcDefineList->cur);
	}
	loadStrcInfo(scopeBlock->leftChild);
	loadStrcInfo(scopeBlock->rightBrother);
}

void loadVarInfoIni(stmtBlock *scopeBlock,varList *curList){
	if (scopeBlock == NULL) return;
	varSymTable *curVarTable = scopeBlock->varTable;
	int i;
	for (i = 0; i < curVarTable->varNum; ++i)
	{
		char *name = curVarTable->varList[i]->name;
		curList->varList[curList->cur] = createNewVarListItem(name,curVarTable->varList[i]);
		if (checkVarNameCG(name,curList) == FALSE){
			++g_varRedefine;
			curVarTable->varList[i]->redefine = g_varRedefine;
		}
		++(curList->cur);
	}
}

void loadVarInfo(stmtBlock *scopeBlock,varList *curList){
	if (scopeBlock == NULL) return;
	varSymTable *curVarTable = scopeBlock->varTable;
	int i;
	for (i = 0; i < curVarTable->varNum; ++i)
	{
		char *name = curVarTable->varList[i]->name;
		curList->varList[curList->cur] = createNewVarListItem(name,curVarTable->varList[i]);
		if (checkVarNameCG(name,curList) == FALSE){
			++g_varRedefine;
			curVarTable->varList[i]->redefine = g_varRedefine;
		}
		++(curList->cur);
	}
	loadVarInfo(scopeBlock->leftChild,curList);
	loadVarInfo(scopeBlock->rightBrother,curList);
}

codeLine* createNewCodeLine(char *code){
	codeLine *temp = (codeLine*)malloc(sizeof(codeLine));
	temp->code = strdup(code);
	temp->next = NULL;
	temp->label[0] = -1;
	temp->label[1] = -1;

	return temp;
}

codeBlock* createNewCodeBlock(){
	codeBlock *temp = (codeBlock*)malloc(sizeof(codeBlock));

	temp->head = NULL;
	temp->tail = NULL;

	return temp;
}

labelTableItem* createNewLabelTableItem(codeLine *targetLine,int labelNum){
	labelTableItem *temp = (labelTableItem *)malloc(sizeof(labelTableItem));

	temp->targetLine = targetLine;
	temp->labelNum = labelNum;
	temp->next = NULL;

	return temp;
}


labelTable* createNewLabelTable(){
	labelTable *temp = (labelTable*)malloc(sizeof(labelTable));

	temp->head = NULL;
	temp->tail = NULL;

	return temp;
}

void addCodeLineToBlock(codeBlock *block,codeLine *line){
	if (block->head == NULL){
		block->head = line;
		block->tail = line;
		return;
	}
	block->tail->next = line;
	block->tail = line;
}

void addCodeBlockToBlock(codeBlock *interBlock,codeBlock *extBlock){
	if (extBlock->head == NULL){
		extBlock->head = interBlock->head;
		extBlock->tail = interBlock->tail;
		free(interBlock);
		return;
	}
	extBlock->tail->next = interBlock->head;
	extBlock->tail = interBlock->tail;
	free(interBlock);
}

void addLableItemToTable(labelTable *table,labelTableItem *item){
	if (table->head == NULL){
		table->head = item;
		table->tail = item;
		return;
	}
	table->tail->next = item;
	table->tail = item;
}

int backPatching(labelTable *table,int labelNum){
	labelTableItem *tempItem = table->head;
	int res = 0,temp;
	while(tempItem != NULL){
		++res;
		temp = tempItem->labelNum;
		tempItem->targetLine->label[temp] = labelNum;
		tempItem = tempItem->next;
	}
	return res;
}

void parseStmtBlockCG(treeNode *stmtblock,char *continueLabel,char *breakLabel){
	outputDefInitial(getNthNodeChild(stmtblock,2));
	stmtBlock *tempBlock = currentBlock;
	currentBlock = stmtblock->block;
	treeNode *stmts = getNthNodeChild(stmtblock,3);
	treeNode *stmt;
	while(strcmp(stmts->leftChild->name,"e") != 0){
		stmt = stmts->leftChild;
		parseStmtCG(stmt,continueLabel,breakLabel);
		stmts = getNthNodeChild(stmts,2);
	}
	currentBlock = tempBlock;
}

void parseStmtCG(treeNode *stmt,char *continueLabel,char *breakLabel){
	treeNode *tempNode;
	if (strcmp(stmt->leftChild->name,"EXP") == 0){
		parseExprCG(stmt->leftChild,"","");
	}
	if (strcmp(stmt->leftChild->name,"STMTBLOCK") == 0){
		parseStmtBlockCG(stmt->leftChild,continueLabel,breakLabel);
	}
	if (strcmp(stmt->leftChild->name,"RET") == 0){
		tempNode = getNthNodeChild(stmt,2);
		if(isConstant(tempNode)){
			int reg = evalConstant(tempNode);
			fprintf(fout, "  store i32 %d, i32* %%1\n", reg);
			checkg_d();fprintf(fout, "  br label %%%s\n", retLabel);fdg();
		}else{
			int reg = parseExprCG(tempNode,"","");
			fprintf(fout, "  store i32 %%%d, i32* %%1\n", reg);
			checkg_d();fprintf(fout, "  br label %%%s\n", retLabel);fdg();
		}
	}

	if (strcmp(stmt->leftChild->name,"IF") == 0){
		treeNode *expr = getNthNodeChild(stmt,3);
		treeNode *stmtIf = getNthNodeChild(stmt,5);
		treeNode *estmt = getNthNodeChild(stmt,6);
		if (strcmp(estmt->leftChild->name,"e") == 0){
			char buf[50];
			sprintf(buf,"label.if.true.%d",++g_labelNum);
			char *trueL = strdup(buf);
			sprintf(buf,"label.if.next.%d",++g_labelNum);
			char *nextL = strdup(buf);
			if (isConstant(expr) == FALSE){
				int reg = parseExprCG(expr,trueL,nextL);
				if(isBoolExp(expr) == FALSE){
					++g_tempVar;
					fprintf(fout, "  %%%d = icmp ne i32 %%%d, 0\n", g_tempVar,reg);
					reg = g_tempVar;
				}
				checkg_d();fprintf(fout, "  br i1 %%%d, label %%%s, label %%%s\n",reg, trueL,nextL);fdg();
			}else{
				int reg = evalConstant(expr);
				++g_tempVar;
				fprintf(fout, "  %%%d = icmp ne i32 %d, 0\n", g_tempVar,reg);
				reg = g_tempVar;
				checkg_d();fprintf(fout, "  br i1 %%%d, label %%%s, label %%%s\n",reg, trueL,nextL);fdg();
			}
			fprintf(fout, "\n%s:\n", trueL);tdg();
			parseStmtCG(stmtIf,continueLabel,breakLabel);
			checkg_d();fprintf(fout, "  br label %%%s\n",nextL);fdg();
			fprintf(fout, "\n%s:\n", nextL);tdg();
		}else{
			char buf[50];
			sprintf(buf,"label.if.true.%d",++g_labelNum);
			char *trueL = strdup(buf);
			sprintf(buf,"label.if.false.%d",++g_labelNum);
			char *falseL = strdup(buf);
			sprintf(buf,"label.if.next.%d",++g_labelNum);
			char *nextL = strdup(buf);
			if (isConstant(expr) == FALSE){
				int reg = parseExprCG(expr,trueL,falseL);
				if(isBoolExp(expr) == FALSE){
					++g_tempVar;
					fprintf(fout, "  %%%d = icmp ne i32 %%%d, 0\n", g_tempVar,reg);
					reg = g_tempVar;
				}
				checkg_d();fprintf(fout, "  br i1 %%%d, label %%%s, label %%%s\n",reg, trueL,falseL);fdg();
			}else{
				int reg = evalConstant(expr);
				++g_tempVar;
				fprintf(fout, "  %%%d = icmp ne i32 %d, 0\n", g_tempVar,reg);
				reg = g_tempVar;
				checkg_d();fprintf(fout, "  br i1 %%%d, label %%%s, label %%%s\n",reg, trueL,falseL);fdg();
			}
			fprintf(fout, "\n%s:\n", trueL);tdg();
			parseStmtCG(stmtIf,continueLabel,breakLabel);
			checkg_d();fprintf(fout, "  br label %%%s\n",nextL);fdg();

			fprintf(fout, "\n%s:\n", falseL);tdg();
			parseStmtCG(getNthNodeChild(estmt,2),continueLabel,breakLabel);
			checkg_d();fprintf(fout, "  br label %%%s\n",nextL);fdg();
			fprintf(fout, "\n%s:\n", nextL);tdg();
		}
	}

	if (strcmp(stmt->leftChild->name,"FOR") == 0){
		treeNode *expr1,*expr2,*expr3,*forStmt;
		expr1 = getNthNodeChild(stmt,3);
		expr2 = getNthNodeChild(stmt,5);
		expr3 = getNthNodeChild(stmt,7);
		forStmt = getNthNodeChild(stmt,9);
		char buf[50];
		sprintf(buf,"label.for.condition.%d",++g_labelNum);
		char *l1 = strdup(buf);
		sprintf(buf,"label.for.stmt.%d",++g_labelNum);
		char *l2 = strdup(buf);
		sprintf(buf,"label.for.expr3.%d",++g_labelNum);
		char *l3 = strdup(buf);
		sprintf(buf,"label.for.next.%d",++g_labelNum);
		char *l4 = strdup(buf);
		parseExprCG(expr1,"","");

		checkg_d();fprintf(fout, "  br label %%%s\n",l1);fdg();
		fprintf(fout, "\n%s:\n", l1);tdg();

		if (isConstant(expr2) == FALSE){
			int reg = parseExprCG(expr2,l2,l4);
			if(isBoolExp(expr2) == FALSE){
				++g_tempVar;
				fprintf(fout, "  %%%d = icmp ne i32 %%%d, 0\n", g_tempVar,reg);
				reg = g_tempVar;
			}
			checkg_d();fprintf(fout, "  br i1 %%%d, label %%%s, label %%%s\n",reg, l2,l4);fdg();
		}else{
			int reg = evalConstant(expr2);
			++g_tempVar;
			fprintf(fout, "  %%%d = icmp ne i32 %d, 0\n", g_tempVar,reg);
			reg = g_tempVar;
			checkg_d();fprintf(fout, "  br i1 %%%d, label %%%s, label %%%s\n",reg, l2,l4);fdg();
		}

		fprintf(fout, "\n%s:\n", l2);tdg();

		parseStmtCG(forStmt,l1,l4);

		checkg_d();fprintf(fout, "  br label %%%s\n",l3);fdg();
		fprintf(fout, "\n%s:\n", l3);tdg();

		parseExprCG(expr3,"","");

		checkg_d();fprintf(fout, "  br label %%%s\n",l1);fdg();
		fprintf(fout, "\n%s:\n", l4);tdg();
	}
	if (strcmp(stmt->leftChild->name,"CONT") == 0){
		checkg_d();fprintf(fout, "  br label %%%s\n", continueLabel);fdg();
	}
	if (strcmp(stmt->leftChild->name,"BREAK") == 0){
		checkg_d();fprintf(fout, "  br label %%%s\n", breakLabel);fdg();
	}
	if (strcmp(stmt->leftChild->name,"READ") == 0){
		tempNode = getNthNodeChild(stmt,3);
		char* ptr = getVarPtrCG(tempNode);
		++g_tempVar;
		fprintf(fout,inputM,g_tempVar,ptr);
	}
	if (strcmp(stmt->leftChild->name,"WRITE") == 0){
		tempNode = getNthNodeChild(stmt,3);
		int reg;
		if(isConstant(tempNode)){
			++g_tempVar;
			reg = evalConstant(tempNode);
			fprintf(fout,outputMC,g_tempVar,reg);
		}else{
			reg = parseExprCG(tempNode,"","");
			++g_tempVar;
			fprintf(fout,outputM,g_tempVar,reg);
		}
	}
}

int parseExprCG(treeNode *expr,char *trueLable,char *falseLable){
	treeNode *tempNode = expr->leftChild;
	treeNode *expr1,*expr2;
	int reg1,reg2;
	char* ptr;
	if (strcmp(tempNode->name,"-") == 0){
		expr1 = getNthNodeChild(expr,2);
		reg1 = parseExprCG(expr1,trueLable,falseLable);
		++g_tempVar;
		fprintf(fout, "  %%%d = sub nsw i32 0, %%%d\n",g_tempVar,reg1);
		return g_tempVar;
	}

	if (strcmp(tempNode->name,"PREINC") == 0){
		expr1 = getNthNodeChild(expr,2);
		ptr = getVarPtrCG(expr1);
		++g_tempVar;
		fprintf(fout, "  %%%d = load i32* %s\n", g_tempVar, ptr);
		++g_tempVar;
		fprintf(fout, "  %%%d = add nsw i32 %%%d, 1\n", g_tempVar, g_tempVar - 1);
		fprintf(fout, "  store i32 %%%d, i32* %s \n", g_tempVar, ptr);
		return g_tempVar;
	}
	if (strcmp(tempNode->name,"PREDEC") == 0){
		expr1 = getNthNodeChild(expr,2);
		ptr = getVarPtrCG(expr1);
		++g_tempVar;
		fprintf(fout, "  %%%d = load i32* %s\n", g_tempVar, ptr);
		++g_tempVar;
		fprintf(fout, "  %%%d = sub nsw i32 %%%d, 1\n", g_tempVar, g_tempVar - 1);
		fprintf(fout, "  store i32 %%%d, i32* %s \n", g_tempVar, ptr);
		return g_tempVar;
	}

	if (strcmp(tempNode->name,"~") == 0){
		expr1 = getNthNodeChild(expr,2);
		reg1 = parseExprCG(expr1,"","");
		++g_tempVar;
		fprintf(fout, "  %%%d = xor i32 %%%d, -1\n", g_tempVar,reg1);
		return g_tempVar;
	}

	if (strcmp(tempNode->name,"!") == 0){
		expr1 = getNthNodeChild(expr,2);
		if(isBoolExp(expr1)){
			reg1 = parseExprCG(expr1,falseLable,trueLable);
			++g_tempVar;
			fprintf(fout, "  %%%d = xor i1 %%%d, true\n", g_tempVar,reg1);
			return g_tempVar;
		}else{
			reg1 = parseExprCG(expr1,"","");
			++g_tempVar;
			fprintf(fout, "  %%%d = icmp eq i32 %%%d, 0\n", g_tempVar,reg1);
			return g_tempVar;
		}
	}

	if (strcmp(tempNode->name,"(") == 0){
		return parseExprCG(getNthNodeChild(expr,2),trueLable,falseLable);
	}
	if (strcmp(tempNode->name,"INT") == 0){
		return (atoi(tempNode->val)) * (-1);
	}
	if (strcmp(tempNode->name,"e") == 0){
		return 0;
	}
	tempNode = getNthNodeChild(expr,2);
	int isNumB = checkBinaryNum(tempNode->name);

	if (isNumB >= 0){
		expr1 = expr->leftChild;
		expr2 = getNthNodeChild(expr,3);
		if (isConstant(expr1)){
			reg1 = evalConstant(expr1);
			reg2 = parseExprCG(expr2,trueLable,falseLable);
			++g_tempVar;
			fprintf(fout, "  %%%d = %s i32 %d, %%%d\n",g_tempVar,llvmOpBianryNum[isNumB],reg1,reg2);
			return g_tempVar;
		}
		if (isConstant(expr2)){
			reg2 = evalConstant(expr2);
			reg1 = parseExprCG(expr1,trueLable,falseLable);
			++g_tempVar;
			fprintf(fout, "  %%%d = %s i32 %%%d, %d\n",g_tempVar,llvmOpBianryNum[isNumB],reg1,reg2);
			return g_tempVar;
		}
		reg1 = parseExprCG(expr1,trueLable,falseLable);
		reg2 = parseExprCG(expr2,trueLable,falseLable);
		++g_tempVar;
		fprintf(fout, "  %%%d = %s i32 %%%d, %%%d\n",g_tempVar,llvmOpBianryNum[isNumB],reg1,reg2);
		return g_tempVar;
	}

	
	if (strcmp(tempNode->name,"=") == 0){
		expr1 = expr->leftChild;
		expr2 = getNthNodeChild(expr,3);
		char* ptr = getVarPtrCG(expr1);
		if (isConstant(expr2)){
			reg2 = evalConstant(expr2);
			fprintf(fout, "  store i32 %d, i32* %s\n",reg2,ptr);
		}else{
			reg2 = parseExprCG(expr2,"","");
			fprintf(fout, "  store i32 %%%d, i32* %s\n",reg2,ptr);
		}
		return -1;
	}

	int isBoolB = checkRealtionNum(tempNode->name);
	if (isBoolB != -1){
		expr1 = expr->leftChild;
		expr2 = getNthNodeChild(expr,3);
		if (isConstant(expr1)){
			reg1 = evalConstant(expr1);
			reg2 = parseExprCG(expr2,trueLable,falseLable);
			++g_tempVar;
			fprintf(fout, "  %%%d = icmp %s i32 %d, %%%d\n",g_tempVar,llvmRelationNum[isBoolB],reg1,reg2);
			return g_tempVar;
		}
		if (isConstant(expr2)){
			reg2 = evalConstant(expr2);
			reg1 = parseExprCG(expr1,trueLable,falseLable);
			++g_tempVar;
			fprintf(fout, "  %%%d = icmp %s i32 %%%d, %d\n",g_tempVar,llvmRelationNum[isBoolB],reg1,reg2);
			return g_tempVar;
		}
		reg1 = parseExprCG(expr1,trueLable,falseLable);
		reg2 = parseExprCG(expr2,trueLable,falseLable);
		++g_tempVar;
		fprintf(fout, "  %%%d = icmp %s i32 %%%d, %%%d\n",g_tempVar,llvmRelationNum[isBoolB],reg1,reg2);
		return g_tempVar;
	}
	
	if (strcmp(tempNode->name,"LAND") == 0){
		expr1 = expr->leftChild;
		expr2 = getNthNodeChild(expr,3);
		char buf[50];
		sprintf(buf,"label.expr.%d",++g_labelNum);
		char *exprL = strdup(buf);
		if (isConstant(expr1)){
			reg1 = evalConstant(expr1);
			++g_tempVar;
			fprintf(fout, "  %%%d = icmp ne i32 %d, 0\n", g_tempVar,reg1);
			checkg_d();fprintf(fout, "  br i1 %%%d, label %%%s, label %%%s\n",g_tempVar, exprL,falseLable);fdg();

			fprintf(fout, "\n%s:\n", exprL);tdg();

			reg2 = parseExprCG(expr2,trueLable,falseLable);
			if(isBoolExp(expr2) == FALSE){
				++g_tempVar;
				fprintf(fout, "  %%%d = icmp ne i32 %%%d, 0\n", g_tempVar,reg2);
				reg2 = g_tempVar;
			}
			return g_tempVar;
		}
		if (isConstant(expr2)){
			reg1 = parseExprCG(expr1,exprL,falseLable);
			if(isBoolExp(expr1) == FALSE){
				++g_tempVar;
				fprintf(fout, "  %%%d = icmp ne i32 %%%d, 0\n", g_tempVar,reg1);
				reg1 = g_tempVar;
			}
			checkg_d();fprintf(fout, "  br i1 %%%d, label %%%s, label %%%s\n",reg1, exprL,falseLable);fdg();

			fprintf(fout, "\n%s:\n", exprL);tdg();
			reg2 = evalConstant(expr2);
			++g_tempVar;
			fprintf(fout, "  %%%d = icmp ne i32 %d, 0\n", g_tempVar,reg2);
			return g_tempVar;
		}

		reg1 = parseExprCG(expr1,exprL,falseLable);
		if(isBoolExp(expr1) == FALSE){
			++g_tempVar;
			fprintf(fout, "  %%%d = icmp ne i32 %%%d, 0\n", g_tempVar,reg1);
			reg1 = g_tempVar;
		}
		checkg_d();fprintf(fout, "  br i1 %%%d, label %%%s, label %%%s\n",reg1, exprL,falseLable);fdg();

		fprintf(fout, "\n%s:\n", exprL);tdg();

		reg2 = parseExprCG(expr2,trueLable,falseLable);
		if(isBoolExp(expr2) == FALSE){
			++g_tempVar;
			fprintf(fout, "  %%%d = icmp ne i32 %%%d, 0\n", g_tempVar,reg2);
			reg2 = g_tempVar;
		}
		return g_tempVar;
	}
	
	if (strcmp(tempNode->name,"LOR") == 0){
		expr1 = expr->leftChild;
		expr2 = getNthNodeChild(expr,3);
		char buf[50];
		sprintf(buf,"label.expr.%d",++g_labelNum);
		char *exprL = strdup(buf);
		if (isConstant(expr1)){
			reg1 = evalConstant(expr1);
			++g_tempVar;
			fprintf(fout, "  %%%d = icmp ne i32 %d, 0\n", g_tempVar,reg1);
			checkg_d();fprintf(fout, "  br i1 %%%d, label %%%s, label %%%s\n",g_tempVar, trueLable,exprL);fdg();

			fprintf(fout, "\n%s:\n", exprL);tdg();

			reg2 = parseExprCG(expr2,trueLable,falseLable);
			if(isBoolExp(expr2) == FALSE){
				++g_tempVar;
				fprintf(fout, "  %%%d = icmp ne i32 %%%d, 0\n", g_tempVar,reg2);
				reg2 = g_tempVar;
			}
			return g_tempVar;
		}
		if (isConstant(expr2)){
			reg1 = parseExprCG(expr1,exprL,falseLable);
			if(isBoolExp(expr1) == FALSE){
				++g_tempVar;
				fprintf(fout, "  %%%d = icmp ne i32 %%%d, 0\n", g_tempVar,reg1);
				reg1 = g_tempVar;
			}
			checkg_d();fprintf(fout, "  br i1 %%%d, label %%%s, label %%%s\n",reg1, trueLable,exprL);fdg();

			fprintf(fout, "\n%s:\n", exprL);tdg();
			reg2 = evalConstant(expr2);
			++g_tempVar;
			fprintf(fout, "  %%%d = icmp ne i32 %d, 0\n", g_tempVar,reg2);
			return g_tempVar;
		}

		reg1 = parseExprCG(expr1,exprL,falseLable);
		if(isBoolExp(expr1) == FALSE){
			++g_tempVar;
			fprintf(fout, "  %%%d = icmp ne i32 %%%d, 0\n", g_tempVar,reg1);
			reg1 = g_tempVar;
		}
		checkg_d();fprintf(fout, "  br i1 %%%d, label %%%s, label %%%s\n",reg1, trueLable,exprL);fdg();

		fprintf(fout, "\n%s:\n", exprL);tdg();

		reg2 = parseExprCG(expr2,trueLable,falseLable);
		if(isBoolExp(expr2) == FALSE){
			++g_tempVar;
			fprintf(fout, "  %%%d = icmp ne i32 %%%d, 0\n", g_tempVar,reg2);
			reg2 = g_tempVar;
		}
		return g_tempVar;
	}
	
	int aNum = checkAssignNum(tempNode->name);
	if (aNum != -1){
		expr1 = expr->leftChild;
		expr2 = getNthNodeChild(expr,3);
		char* ptr = getVarPtrCG(expr1); 
		reg1 = parseExprCG(expr1,"","");
		if (isConstant(expr2)){
			reg2 = evalConstant(expr2);
			fprintf(fout, "  %%%d = %s i32 %%%d, %d\n", ++g_tempVar,llvmOpAssignNum[aNum],reg1,reg2);
		}else{
			reg2 = parseExprCG(expr2,"","");
			fprintf(fout, "  %%%d = %s i32 %%%d, %d\n", ++g_tempVar,llvmOpAssignNum[aNum],reg1,reg2);
		}
		fprintf(fout, "  store i32 %%%d, i32* %s\n",g_tempVar,ptr);
		return g_tempVar;
	}
	
	if (strcmp(tempNode->name,"(") == 0){
		char *name = expr->leftChild->val;
		treeNode *args = getNthNodeChild(expr,3);
		int length = countArgs(args);
		int *reg = (int*)malloc(sizeof(int) * length);
		treeNode **exprs = (treeNode**)malloc(sizeof(treeNode*) * length);
		int i;
		for(i = 0;i < length; ++i){
			exprs[i] = getExpInArgs(args,i);
			if(isConstant(exprs[i])) reg[i] = evalConstant(exprs[i]);
				else reg[i] = parseExprCG(exprs[i],"","");
		}
		++g_tempVar;
		fprintf(fout, "  %%%d = call i32 @%s(",g_tempVar,name);
		for (i = 0; i < length - 1; ++i)
		{
			if(isConstant(exprs[i]) == FALSE){
				fprintf(fout, "i32 %%%d,", reg[i]);	
			}else{
				fprintf(fout, "i32 %d,", reg[i]);
			}
		}
		if(length > 0){
			if(isConstant(exprs[length - 1]) == FALSE){
				fprintf(fout, "i32 %%%d", reg[length - 1]);	
			}else{
				fprintf(fout, "i32 %d", reg[length - 1]);
			}
		}
		fprintf(fout, ")\n");
		return g_tempVar;
	}
	
	if (strcmp(tempNode->name,"ARRS") == 0){
		char *name = expr->leftChild->val;
		varSymTableItem *item = getVarItemViaName(currentBlock,name);
		name = getVarNameCG(item);
		if(item->isPara >= 0){
			++g_tempVar;
			fprintf(fout, "  %%%d = load i32* %%%d\n", g_tempVar,item->isPara + 2);	
			return g_tempVar;
		}
		if(item->arrNum >= 0){
			ptr = getVarPtrCG(expr);
			++g_tempVar;
			fprintf(fout, "  %%%d = load i32* %s\n", g_tempVar,ptr);
			return g_tempVar;
		}
		++g_tempVar;
		fprintf(fout, "  %%%d = load i32* %s\n", g_tempVar,name);
		return g_tempVar;
	}

	if (strcmp(tempNode->name,".") == 0){
		ptr = getVarPtrCG(expr);
		++g_tempVar;
		fprintf(fout, "  %%%d = load i32* %s\n", g_tempVar,ptr);
		return g_tempVar;
	}
}

void iniStrc(){
	strcDefineList = (strcList *)malloc(sizeof(strcList));
	strcDefineList->strNum = countAllStrcNum(globalBlock);
	strcDefineList->strcList = (pstrcListItem *)malloc(sizeof(pstrcListItem) * strcDefineList->strNum);
	loadStrcInfo(globalBlock);
}

void iniVar(){
	funVarDefineList = (funVarList *)malloc(sizeof(funVarList));
	int i,length = globalBlock->funTable->funNum;
	funVarDefineList->funVarList = (pVarList *)malloc(sizeof(pVarList) * length);
	for (i = 0; i < length; ++i){
		g_varRedefine= 0;
		stmtBlock *tempBlock = getNthBlockChild(globalBlock,i+1);
		int temp = countAllVarNum(tempBlock->leftChild) + tempBlock->varTable->varNum;
		funVarDefineList->funVarList[i] = createNewVarList(temp);
		funVarDefineList->funVarList[i]->paraVarNum = globalBlock->funTable->funList[i]->paraNum;
		loadVarInfoIni(tempBlock,funVarDefineList->funVarList[i]);
		loadVarInfo(tempBlock->leftChild,funVarDefineList->funVarList[i]);
	}
}

void outputStrcCG(){
	int i;
	for (i = 0; i < strcDefineList->strNum; ++i)
	{
		strcSymTableitem *strcItem = strcDefineList->strcList[i]->item;
		fprintf(fout, "%%");
		if (strcItem->redefine == -1){
			fprintf(fout, "%s", strcItem->name);
		}
		else fprintf(fout, "%s.%d", strcItem->name,strcItem->redefine);
		foutput(" = type {");
		if (strcItem->memNum > 0){
			int j = 0;
			for (j = 0; j < strcItem->memNum - 1; ++j){
				outputStrcMemCG(strcItem->memList[j]);
				foutput(",");
			}
			outputStrcMemCG(strcItem->memList[strcItem->memNum - 1]);
			foutput(" ");
		}
		foutput("}");
		fendl();
	}
}

void outputGlobalVarCG(){
	varSymTable *varTable = globalBlock->varTable;
	int i;
	for (i = 0; i < varTable->varNum; ++i)
	{
		fprintf(fout, "@%s = global ", varTable->varList[i]->name);
		if(strcmp(varTable->varList[i]->type,"int") == 0){
			if(varTable->varList[i]->init == NULL){
				if(varTable->varList[i]->arrNum == -1) foutput("i32 0");
					else{
						fprintf(fout, "[%d x i32] zeroinitializer", varTable->varList[i]->arrNum);
					}
			}else{
				treeNode *init = varTable->varList[i]->init;
				if(varTable->varList[i]->arrNum == -1) {
					int reg = evalConstant(init->leftChild);
					fprintf(fout,"i32 %d",reg);
				}else{
					int arrNum = varTable->varList[i]->arrNum;
					fprintf(fout, "[%d x i32] [", arrNum);
					int i,reg;
					treeNode *args = getNthNodeChild(init,2);
					for(i = 0;i<arrNum-1;++i){
						treeNode *exprx = getExpInArgs(args,i);
						if (exprx == NULL) reg = 0;
						 	else reg = evalConstant(exprx);
						fprintf(fout, "i32 %d, ", reg);
					}
					if(arrNum>0){
						treeNode *exprx = getExpInArgs(args,arrNum - 1);
						if (exprx == NULL) reg = 0;
						 	else reg = evalConstant(exprx);
						fprintf(fout, "i32 %d]\n", reg);
					}
				}
			}
		}else{
			char *tempType = getTypeNameCG(globalBlock,varTable->varList[i]->type);
			if(varTable->varList[i]->arrNum == -1) {
				foutput("%");
				foutput(varTable->varList[i]->type);
				foutput(" zeroinitializer");
			}else{
				fprintf(fout, "[%d x %%%s] zeroinitializer", varTable->varList[i]->arrNum,varTable->varList[i]->type);
			}
		}
		fendl();
	}
}

void outputFunParaCG(funSymTableItem *item){
	if (item->paraNum == 0) return;
	int i;
	for (i = 0; i < item->paraNum - 1; ++i)
	{
		paraItem *para = item->paraList[i];
		fprintf(fout, "i32 %%%s,", para->name);
	}
	fprintf(fout, "i32 %%%s", item->paraList[item->paraNum - 1]->name);
}

void outputVarCG(int index){
	varList *curList = funVarDefineList->funVarList[index];
	int funVarNum = curList->paraVarNum;
	foutput("  %1 = alloca i32\n");
	int i;
	for (i = 0; i < funVarNum; ++i)
	{
		fprintf(fout, "  %%%d = alloca i32\n", i + 2);
	}
	g_tempVar = funVarNum + 1;
	for (i = funVarNum; i < curList->varNum; ++i)
	{
		varSymTableItem *item = curList->varList[i]->item;
		char *varName = getVarNameCG(item);
		char *typeName = getTypeNameCG(item->defineBlock,item->type);
		fprintf(fout, "  %s = alloca ", varName);
		if (item->arrNum == -1) {
			if (strcmp(typeName,"int") == 0) foutput("i32");
				else fprintf(fout, "%%%s",typeName);
		}else{
				if (strcmp(typeName,"int") == 0) fprintf(fout, "[%d x i32]", item->arrNum);
				else fprintf(fout, "[%d x %%%s]", item->arrNum , typeName);
			}
		fendl();
	}
}

void outputParaVarIniCG(int index){
	varList *curList = funVarDefineList->funVarList[index];
	foutput("  store i32 0, i32* %1\n");
	int paraVarNum = curList->paraVarNum;
	int i;
	for (i = 0; i < paraVarNum; ++i){
		  fprintf(fout, "  store i32 %%%s, i32* %%%d", curList->varList[i]->name, i + 2);
		  fendl();
	}
}

void outputFunCG(){
	funSymTable *funTable = globalBlock->funTable;
	int i;
	curTempVar = (int *)malloc(sizeof(int) * funTable->funNum);
	for (i = 0; i < funTable->funNum; ++i){
		g_tempVar = 0;
		g_labelNum = 0;
		g_d = TRUE;
		funSymTableItem *item = funTable->funList[i];
		fprintf(fout, "define i32 @%s(", item->name);
		outputFunParaCG(item);
		foutput("){");
		fendl();
		outputVarCG(i);
		outputParaVarIniCG(i);
		stmtBlock *temp = getNthBlockChild(globalBlock,i+1);
		parseStmtBlockCG(temp->stmt,"","");
		fendl();
		fprintf(fout, "%s:\n",retLabel);
		fprintf(fout,"  %%%d = load i32* %%1\n",++g_tempVar);
		fprintf(fout,"  ret i32 %%%d\n",g_tempVar);
  		//ret i32 %13
		foutput("}\n");
		fendl();
	}
}

void codeGenerator(){
	g_structRedefine = 0;
	iniStrc();
	iniVar();
	foutput(inputOutputInitial);
	fendl();
	outputStrcCG();
	fendl();
	outputGlobalVarCG();
	fendl();
	outputFunCG();
}