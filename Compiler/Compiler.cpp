// Compiler.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <stdlib.h>



int main(int argc, char **argv)
{
	char* inFilePath = NULL;
	if (argc > 1)
		inFilePath = argv[1];
	char lex[50];
	sprintf(lex, "LexAnaly.exe %s", inFilePath? inFilePath : "in.txt");
	system(lex);
	system("GrammerAnaly.exe");
	system("VirtualMachine.exe");
    return 0;
}