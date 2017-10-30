#include "predefine.h"
#include "tree.h"

treeNode *getNewNode(char *name,char *val){
    treeNode *temp;
    temp = (treeNode *)malloc(sizeof(treeNode));
    temp->name = strdup(name);
    temp->val = strdup(val);
    temp->leftChild = NULL;
    temp->rightBrother = NULL;
    temp->block = NULL;
    return temp;
}

void setChild(treeNode *father,treeNode *child){
    if(father->leftChild != NULL){
        setBrother(father->leftChild,child);
    }else{
        father->leftChild = child;
    }
}

void setBrother(treeNode *older,treeNode *younger){
    if (older->rightBrother != NULL)
    {
        setBrother(older->rightBrother,younger);
    }else{
        older->rightBrother = younger;
    }
}

treeNode* getNthNodeChild(treeNode *father,int n){
    if (n <= 0)
    {
        return NULL;
    }
    int temp = n;
    treeNode *p = father->leftChild;
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

int countVarArrNum(treeNode *var){
    if (strcmp(var->leftChild->name,"ID") == 0)
    {
        return 0;
    }
    return 1 + countVarArrNum(var->leftChild);
}

char* getVarName(treeNode *var){
    if (strcmp(var->leftChild->name,"ID") == 0)
    {
        return strdup(var->leftChild->val);
    }
    return getVarName(var->leftChild);
}

char* getTypeName(treeNode *spec){
    if (strcmp(spec->leftChild->name,"TYPE") == 0)
    {
        return strdup("int");
    }
    char *temp = "struct.";
    treeNode *stspec = getNthNodeChild(spec,1);
    treeNode *opttag = getNthNodeChild(stspec,2);
    if (strcmp(opttag->name,"ID") == 0 )
    {
        return addString(temp,opttag->val);
    }
    if (strcmp(opttag->leftChild->name,"e") == 0)
    {
        //free(opttag->leftChild);
        char tmp[20];
        sprintf(tmp,"%d",anonStrcNum);
        char *name = addString("anon.strc.",tmp);
        treeNode* IDNode = getNewNode("ID",name);
        opttag->leftChild = IDNode;
        ++anonStrcNum;
    }
    return addString(temp,opttag->leftChild->val);
}

int isConstant(treeNode *expr){
    if (strcmp(expr->leftChild->name,"INT") == 0) return TRUE;
    treeNode *temp = getNthNodeChild(expr,2);
    if (strcmp(expr->leftChild->name,"PREINC") == 0) return FALSE;
    if (strcmp(expr->leftChild->name,"PREDEC") == 0) return FALSE;
    if (strcmp(temp->name,"EXP") == 0) return isConstant(temp);
    if (strcmp(expr->leftChild->name,"ID") == 0) return FALSE;
    temp = getNthNodeChild(expr,3);
    if (strcmp(temp->name,"EXP") == 0) 
            return isConstant(getNthNodeChild(expr,3)) && isConstant(expr->leftChild);
    return FALSE;
}

int isBoolExp(treeNode *expr){
    treeNode *tempNode = getNthNodeChild(expr,2);
    if(tempNode == NULL) return FALSE;
    int isNumB = checkRealtionNum(tempNode->name);
    if (isNumB >= 0) return TRUE;
    if (strcmp(tempNode->name,"LAND") == 0) return TRUE;
    if (strcmp(tempNode->name,"LOR") == 0) return TRUE;
    tempNode = expr->leftChild;
    if (strcmp(tempNode->name,"!") == 0) return TRUE;
    if (strcmp(tempNode->name,"(") == 0) 
            return isBoolExp(getNthNodeChild(expr,2));
    return FALSE;
}

int countArgs(treeNode *args){
    treeNode *temp = getNthNodeChild(args,3);
    if(temp == NULL) return 1;
    return 1 + countArgs(temp);
}

treeNode* getExpInArgs(treeNode *args,int i){
    if(args == NULL) return NULL;
    if(i == 0) return args->leftChild;
    treeNode *temp = getNthNodeChild(args,3);
    return getExpInArgs(temp,i-1);
}

int evalConstant(treeNode *expr){
    if(strcmp(expr->leftChild->name,"INT") == 0) 
        return atoi(expr->leftChild->val);
    treeNode *temp = getNthNodeChild(expr,2);
    if(strcmp(expr->leftChild->name,"-") == 0)
        return (-1) * evalConstant(temp);
    if(strcmp(expr->leftChild->name,"~") == 0){
        int t = evalConstant(temp);
        return ~t;
    }
    if(strcmp(expr->leftChild->name,"!") == 0){
        int t = evalConstant(temp);
        return !t;
    }
    if(strcmp(temp->name,"+") == 0){
        int t1 = evalConstant(expr->leftChild);
        int t2 = evalConstant(getNthNodeChild(expr,3));
        return t1 + t2;
    }
    if(strcmp(temp->name,"-") == 0){
        int t1 = evalConstant(expr->leftChild);
        int t2 = evalConstant(getNthNodeChild(expr,3));
        return t1 - t2;
    }
    if(strcmp(temp->name,"*") == 0){
        int t1 = evalConstant(expr->leftChild);
        int t2 = evalConstant(getNthNodeChild(expr,3));
        return t1 * t2;
    }
    if(strcmp(temp->name,"/") == 0){
        int t1 = evalConstant(expr->leftChild);
        int t2 = evalConstant(getNthNodeChild(expr,3));
        return t1 / t2;
    }
    if(strcmp(temp->name,"&") == 0){
        int t1 = evalConstant(expr->leftChild);
        int t2 = evalConstant(getNthNodeChild(expr,3));
        return t1 & t2;
    }
    if(strcmp(temp->name,"^") == 0){
        int t1 = evalConstant(expr->leftChild);
        int t2 = evalConstant(getNthNodeChild(expr,3));
        return t1 ^ t2;
    }
    if(strcmp(temp->name,"|") == 0){
        int t1 = evalConstant(expr->leftChild);
        int t2 = evalConstant(getNthNodeChild(expr,3));
        return t1 | t2;
    }
    if(strcmp(temp->name,"%") == 0){
        int t1 = evalConstant(expr->leftChild);
        int t2 = evalConstant(getNthNodeChild(expr,3));
        return t1 % t2;
    }
    if(strcmp(temp->name,">") == 0){
        int t1 = evalConstant(expr->leftChild);
        int t2 = evalConstant(getNthNodeChild(expr,3));
        return t1 > t2;
    }
    if(strcmp(temp->name,"<") == 0){
        int t1 = evalConstant(expr->leftChild);
        int t2 = evalConstant(getNthNodeChild(expr,3));
        return t1 < t2;
    }
    if(strcmp(temp->name,"SL") == 0){
        int t1 = evalConstant(expr->leftChild);
        int t2 = evalConstant(getNthNodeChild(expr,3));
        return t1 << t2;
    }
    if(strcmp(temp->name,"SR") == 0){
        int t1 = evalConstant(expr->leftChild);
        int t2 = evalConstant(getNthNodeChild(expr,3));
        return t1 >> t2;
    }
    if(strcmp(temp->name,"NL") == 0){
        int t1 = evalConstant(expr->leftChild);
        int t2 = evalConstant(getNthNodeChild(expr,3));
        return t1 >= t2;
    }
    if(strcmp(temp->name,"NG") == 0){
        int t1 = evalConstant(expr->leftChild);
        int t2 = evalConstant(getNthNodeChild(expr,3));
        return t1 <= t2;
    }
    if(strcmp(temp->name,"EQ") == 0){
        int t1 = evalConstant(expr->leftChild);
        int t2 = evalConstant(getNthNodeChild(expr,3));
        return t1 == t2;
    }
    if(strcmp(temp->name,"NE") == 0){
        int t1 = evalConstant(expr->leftChild);
        int t2 = evalConstant(getNthNodeChild(expr,3));
        return t1 != t2;
    }
    if(strcmp(temp->name,"LAND") == 0){
        int t1 = evalConstant(expr->leftChild);
        int t2 = evalConstant(getNthNodeChild(expr,3));
        return t1 && t2;
    }
    if(strcmp(temp->name,"LOR") == 0){
        int t1 = evalConstant(expr->leftChild);
        int t2 = evalConstant(getNthNodeChild(expr,3));
        return t1 || t2;
    }
    return atoi(temp->val);
}

int checkTreeNodeFun(treeNode *node){
    if (strcmp(node->name,"EXTDEF") != 0) return FALSE;
    treeNode *funNode = getNthNodeChild(node,2);
    if (strcmp(funNode->name,"FUNC") == 0)
    {
        return TRUE;
    }else
    {
        return FALSE;
    }
}

int checkTreeNodeStrc(treeNode *node){
    if (strcmp(node->name,"EXTDEF") != 0) return FALSE;
    if (checkTreeNodeFun(node)) return FALSE;
    treeNode *stspec = node->leftChild->leftChild;
    if (strcmp(stspec->name,"STSPEC") != 0) return FALSE;
    treeNode *temp = getNthNodeChild(stspec,2);
    if (strcmp(temp->name,"OPTTAG") == 0)
    {
        return TRUE;
    }else
    {
        return FALSE;
    }
}

int checkTreeNodeVar(treeNode *extdef){
    if (strcmp(extdef->name,"EXTDEF") != 0) return FALSE;
    if (checkTreeNodeFun(extdef))
    {
        return FALSE;
    }
    treeNode *temp = getNthNodeChild(extdef,2);
    if (strcmp(temp->name,"e") == 0)
    {
        return FALSE;
    }
    return TRUE;
}

int countFunNum(treeNode *node){
    if (strcmp(node->name,"EXTDEFS") != 0) return 0;
    treeNode *p = node->leftChild;
    if (strcmp(p->name,"e") == 0) return 0;
    return checkTreeNodeFun(p) + countFunNum(p->rightBrother);
}

int countStrcNum(treeNode *node){
    if (strcmp(node->name,"EXTDEFS") != 0) return 0;
    treeNode *p = node->leftChild;
    if (strcmp(p->name,"e") == 0) return 0;
    return checkTreeNodeStrc(p) + countStrcNum(p->rightBrother);    
}

int countVarNum(treeNode *extdefs){
    if (strcmp(extdefs->name,"EXTDEFS") != 0) return 0;
    treeNode *p = extdefs->leftChild;
    if (strcmp(p->name,"e") == 0) return 0;
    treeNode *extdef = extdefs->leftChild;
    treeNode *extvars = getNthNodeChild(extdef,2);
    return countVarNumExtvars(extvars) + countVarNum(extdef->rightBrother);
}

int countParaNum(treeNode *node){
    if (strcmp(node->name,"PARAS") != 0) return 0;
    if (strcmp(node->leftChild->name,"e") == 0)
    {
        return 0;
    }
    treeNode *p = getNthNodeChild(node,3);
    if (p == NULL)
    {
        return 1;
    }
    return  (1 + countParaNum(p));
}

int countMemNumStrcdefs(treeNode *node){
    if (strcmp(node->name,"STRCDEFS") != 0) return 0;
    if (strcmp(node->leftChild->name,"e") == 0)
    {
        return 0;
    }
    return countMemNumStrcdef(node->leftChild) 
        + countMemNumStrcdefs(node->leftChild->rightBrother);
}

int countMemNumStrcdef(treeNode *node){
    if (strcmp(node->name,"STRCDEF") != 0) return 0;
    treeNode *p = getNthNodeChild(node,2);
    int res = 0;
    while(p != NULL){
        ++res;
        p = getNthNodeChild(p,3);
    }
    return res;
}

int countVarNumExtvars(treeNode *extvars){
    if (strcmp(extvars->name,"EXTVARS") != 0) return 0;
    if (strcmp(extvars->leftChild->name,"e") == 0) return 0;
    treeNode *temp = getNthNodeChild(extvars,3);
    if (temp == NULL)
    {
        return 1;
    }
    return 1 + countVarNumExtvars(temp);
}

// For inner block
int countStrcNumI(treeNode *defs){
    if (strcmp(defs->leftChild->name,"e") == 0)
    {
        return 0;
    }
    treeNode *spec = getNthNodeChild(defs->leftChild,1);
    int res = 0;
    if (strcmp(spec->leftChild->name,"STSPEC") == 0)
    {
        treeNode *temp = getNthNodeChild(spec->leftChild,2);
        if(strcmp(temp->name,"OPTTAG") == 0) res = 1;
    }
    return res + countStrcNumI(getNthNodeChild(defs,2));
}

int countVarNumI(treeNode *defs){
    if (strcmp(defs->leftChild->name,"e") == 0)
    {
        return 0;
    }
    treeNode *decs = getNthNodeChild(defs->leftChild,2);
    int res = 0;
    while(decs != NULL){
        ++res;
        decs = getNthNodeChild(decs,3);
    }
    return res + countVarNumI(getNthNodeChild(defs,2));
}

int checkTreeNodeStrcI(treeNode *def){
    treeNode *spec = getNthNodeChild(def,1);
    int res = FALSE;
    if (strcmp(spec->leftChild->name,"STSPEC") == 0)
    {
        treeNode *temp = getNthNodeChild(spec->leftChild,2);
        if(strcmp(temp->name,"OPTTAG") == 0) res = TRUE;
    }
    return res;
}