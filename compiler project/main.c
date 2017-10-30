#include "predefine.h"
#include "block.h"
#include "y.tab.h"

extern treeNode emptyNode;
extern FILE* fout;
extern treeNode* rootNode;
extern stmtBlock *currentBlock;
extern stmtBlock *globalBlock;

void output(int indent,treeNode *cur){
    int i;
    treeNode *p = cur;
    while(p){
        for (i = 0; i < indent; ++i){fprintf(fout,"   ");}
        if (strlen(p->val) != 0)
        {
            fprintf(fout,"|%s -----> %s\n",p->name,p->val);
        }else{
            fprintf(fout,"|%s\n", p->name);
        }
        output(indent + 1,p->leftChild);
        p = p->rightBrother;
    }
}

void initialEmpty(){
    emptyNode.name = "e";
    emptyNode.val = "ε";
    emptyNode.leftChild = NULL;
    emptyNode.rightBrother = NULL;
}

int main(int argc,char *argv[])
{
    //initial
    initialEmpty();
    extern FILE* yyin;
    if (argc != 3)
    {
        fprintf(stderr,"Error with file number\n");
        exit(1);
    }
    if ((yyin  = fopen(argv[1],"r")) == 0)
    {
        fprintf(stderr,"Error with inputfile:%s\n",argv[1]);
        exit(1);
    }
    if ((fout  = fopen(argv[2],"w")) == 0)
    {
        fprintf(stderr, "Error with outputfile:%s\n",argv[2]);
        exit(1);
    }
    yyparse();
    currentBlock = createExtenalBlock();
    globalBlock = currentBlock;
    //fprintf(stderr, "%d\n", currentBlock->funTable->funNum);
    //fprintf(stderr, "%d\n", currentBlock->strcTable->strcNum);
    codeGenerator();
    //output(0,rootNode);
    fclose(fout);
    return 0;
}