#include "stdafx.h"

#include <iostream>
#include <stdio.h>
#include <string>
#include <string.h>
#include <fstream>
#include <map>
using namespace std;
char buf[128]; //已处理的字符串缓冲
int buf_pos; //字符串缓冲位置
int state_cur; //当前状态
int state[18][128]; //状态转换表
bool finalStates[18]; //终态
bool complete; //是否处理完成(遇到EOF)
int curLine, curPos, prePos; //当前处理的 行, 列
int errorCnt;
string words[7] = { "if", "else", "for", "while", "int", "write","read" }; //保留字列表
string delimiters[6] = { "(",")",";","{","}","," };
string operators[11] = { "+","-","*","/","=","<",">",">=","<=","!=","==" };
/*char symName[][20] = {
"ifsym", "elsesym", "forsym","whilesym", "intsym", "writesym","readsym",
"lparen","rparen","semicolon","lbparen","rbparen","comma",
"plus","minus","times","slash","eql","lss","gtr","geq","leq", "neql", "deql" ,
"num","comment","id"
};*/

char symName[][20] = {
	"if", "else", "for","while", "int", "write","read",
	"(",")",";","{","}",",",
	"+","-","*","/","=","<",">",">=","<=", "!=", "==" ,
	"NUM","comment","ID"
};
char finalStateNames[6][20] = { "无符号整数","分界符","运算符","注释","标识符","保留字" }; //终态名字
map<string, char*> symMap;
ofstream lexFs("lex.txt");
//初始化状态表
void initState()
{
	for (int i = 0; i<7; i++)symMap[words[i]] = symName[i];
	for (int i = 7; i<13; i++)symMap[delimiters[i - 7]] = symName[i];
	for (int i = 13; i<24; i++)symMap[operators[i - 13]] = symName[i];

	//全置为-1， state[i][j] = -1表示 i状态没有j弧
	memset(state, -1, sizeof(state));

	state[0]['0'] = 1;

	for (int c = '1'; c <= '9'; c++) state[0][c] = 2;
	for (int c = '0'; c <= '9'; c++) state[2][c] = 2;

	state[0]['('] = 11; state[0][')'] = 11;
	state[0][';'] = 11; state[0]['{'] = 11;
	state[0]['}'] = 11; state[0][','] = 11;

	state[0]['+'] = 12; state[0]['-'] = 12; state[0]['*'] = 12;
	state[0]['>'] = 3; state[0]['<'] = 3; state[0]['='] = 3;
	state[3]['='] = 12;
	state[0]['!'] = 4; state[4]['='] = 12;

	state[0]['/'] = 5;

	state[5]['*'] = 6; state[6]['*'] = 7;
	state[7]['*'] = 7; state[7]['/'] = 8;

	//除*外的字符
	for (int c = 0; c<127; c++)
		if (c != '*')
			state[6][c] = 6;

	//除 * 和 / 外的字符
	for (int c = 0; c<127; c++)
		if (c != '*' && c != '/')
			state[7][c] = 6;


	//a~z, A~Z
	for (int c = 'a'; c <= 'z'; c++) state[0][c] = 9;
	for (int c = 'A'; c <= 'Z'; c++) state[0][c] = 9;

	for (int c = 'a'; c <= 'z'; c++) state[9][c] = 9;
	for (int c = 'A'; c <= 'Z'; c++) state[9][c] = 9;
	for (int c = '0'; c <= '9'; c++) state[9][c] = 9;

	finalStates[1] = finalStates[2] = finalStates[11]
		= finalStates[3] = finalStates[12]
		= finalStates[5] = finalStates[8] = finalStates[9] = true;
}


//判断状态s是否为终态
bool judgeFinalState(int s)
{
	return finalStates[s];
}

//处理终态finalState
void dealFinalState(int finalState)
{
	if (finalState == 8)
	{
		buf_pos = 0;
		state_cur = 0;
		return;
	}
	buf[buf_pos] = '\0';
	//确定当前状态的名字
	char* stateName = finalStateNames[finalState - 10];

	if (finalState == 1 || finalState == 2)
		stateName = symName[24]; //num
	else
	{
		if (symMap.count(string(buf)) != 0)
			stateName = symMap[string(buf)];
		else
			stateName = symName[26]; //id
	}
	char outStr[260];
	//sprintf(outStr, "%s %d (%d,%d) %s\n", buf, finalState, curLine, curPos-1, stateName);
	sprintf(outStr, "%s\t%s\t%d %d\n", stateName, buf, curPos == 0 ? curLine - 1 : curLine, prePos);
	lexFs << outStr;
	buf_pos = 0;
	state_cur = 0;
}

//处理错误，curState为当前状态
void dealError(int curState)
{
	buf[buf_pos] = '\0';
	printf("行 %d 列 %d: %s , 错误! \n", curPos == 0 ? curLine - 1 : curLine, prePos, buf);
	buf_pos = 0;
	state_cur = 0;
	errorCnt ++;
}

//分析，c为当前字符
void analy(int c)
{
	if (c == EOF)
	{
		//遇到了EOF在判断一次当前状态
		if (judgeFinalState(state_cur))
			dealFinalState(state_cur);
		else
			dealError(state_cur);
		return;
	}
	if (state[state_cur][c] != -1)
	{
		buf[buf_pos++] = c;
		state_cur = state[state_cur][c];
		//处理下一个字符
		char nextc = getchar();
		prePos = curPos++;
		if (nextc == '\n')curPos = 0, curLine++;
		analy(nextc);
	}
	else
	{
		if (state_cur == 0)
		{
			//初态遇到一个无法识别的字符处理下一个字符
			if (c != ' ' && c != '\n' && c != '\t') //忽略空格行换制表符
				dealError(state_cur);


			char nextc = getchar();
			prePos = curPos++;
			if (nextc == '\n')curPos = 0, curLine++;
			analy(nextc);
			return;
		}
		if (judgeFinalState(state_cur))
			dealFinalState(state_cur);
		else
			dealError(state_cur);
		analy(c);
	}
}

int main(int argc, char **argv)
{
	errorCnt = 0;
	char* inFilePath = NULL;
	if (argc > 1)
		inFilePath = argv[1];
	//初始化状态表
	initState();
	complete = false;
	curLine = 1, curPos = 1, prePos = 0;
	buf_pos = 0;
	state_cur = 0;

	if (inFilePath != NULL) freopen(inFilePath, "r", stdin);
	else freopen("./in.txt", "r", stdin);

	//开始分析
	analy(getchar());
	lexFs << "#\t#\t" << curLine << " 1" << endl;
	lexFs << errorCnt << endl;

	//还原stdout 
	printf("词法分析完成!\n");
	return 0;
}