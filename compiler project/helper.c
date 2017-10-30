#include "predefine.h"

extern FILE* fout;

char* addString(char *s1,char *s2){
    char *temp = (char*)malloc(sizeof(char) * (strlen(s1) + strlen(s2)));
    int i = 0;
    for (i = 0; i < strlen(s1); ++i)
    {
        temp[i] = s1[i];
    }
    for (i = 0; i <=strlen(s2); ++i)
    {
        temp[i + strlen(s1)] = s2[i];
    }
    return temp;
}

char* subString(char *s,int begin,int end){
    if(end > strlen(s)) return subString(s,begin,strlen(s));
    char *temp = (char*)malloc(sizeof(char) * (end - begin));
    int i;
    for (i = 0; i < end - begin; ++i)
    {
        temp[i] = s[i + begin];
    }
    return temp;
}

char* todec(char* num){
    char* end;
    int temp = (int)strtoul(num,&end,0);
    char buf[100];
    sprintf(buf,"%d",temp);
    return strdup(buf);
}

void tcErrorOutput(int errorID,char* errorInfo){
    fprintf(stderr, "typecheck-error: %s\n", errorInfo);
    fprintf(fout, "Error.\n");
    fclose(fout);
    exit(1);
}

void dbs(char *info){
    fprintf(stderr, "%s\n", info);
}

void dbi(int info){
    fprintf(stderr, "%d\n", info);
}

void fendl(){
    fprintf(fout, "\n");
}

void foutput(char *s){
    fprintf(fout, "%s", s);
}