// complier2.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string>
#include <vector>
#include <stack>
#include <set>
#include <map>
using namespace std;

struct Attr {
	map<string, string*> str;
	Attr* clear() {
		str.clear();
		return this;
	}
	Attr* add(string s, string* p) {
		this->str[s] = p;
		return this;
	}
}emptyAttr;

struct Production {
	int id;
	vector<string> word;
};

struct SymbolItem {
	string name;
	int address;
	int val;
	bool init;
};
map<string, SymbolItem> symbolTable;
vector<string*> ps;
stack<string> analyStack;
stack<Attr> attrStack;
set<string> symNs;
map<pair<string, string>, Production> analyTable;
vector< pair<string, string> > words;
vector< pair<int, int> > wordPos;
string curTopWord;
Attr curTopAttr;
bool success;
int readPos;
int flag;
int curLine;
int nextSymbolAddress;
int nextTmpAddress;
int nextLabel;

ofstream out("instruction.txt");

void addTable(string left, string str, string* right, int len, int id)
{
	Production p;
	p.id = id;
	for (int i = 0; i < len; i++)
		p.word.push_back(right[i]);
	analyTable[make_pair(left, str)] = p;
	delete[] right;
}

void initTable()
{
	addTable("<program>", "{", new string[4]{ "{","<declaration_list>","<statement_list>","}" }, 4, 0);

	addTable("<declaration_list>", "if", new string[1]{ "ε" }, 1, 1);
	addTable("<declaration_list>", "while", new string[1]{ "ε" }, 1, 2);
	addTable("<declaration_list>", "for", new string[1]{ "ε" }, 1, 3);
	addTable("<declaration_list>", "read", new string[1]{ "ε" }, 1, 4);
	addTable("<declaration_list>", "int", new string[2]{ "<declaration_stat>","<declaration_list>" }, 2, 5);
	addTable("<declaration_list>", "write", new string[1]{ "ε" }, 1, 6);
	addTable("<declaration_list>", "ID", new string[1]{ "ε" }, 1, 7);
	addTable("<declaration_list>", "{", new string[1]{ "ε" }, 1, 8);
	addTable("<declaration_list>", "}", new string[1]{ "ε" }, 1, 9);
	addTable("<declaration_list>", ";", new string[1]{ "ε" }, 1, 10);

	addTable("<declaration_stat>", "int", new string[4]{ "int","ID","@name-def",";" }, 4, 11);

	addTable("<statement_list>", "if", new string[2]{ "<statement>","<statement_list>" }, 2, 12);
	addTable("<statement_list>", "while", new string[2]{ "<statement>","<statement_list>" }, 2, 13);
	addTable("<statement_list>", "for", new string[2]{ "<statement>","<statement_list>" }, 2, 14);
	addTable("<statement_list>", "read", new string[2]{ "<statement>","<statement_list>" }, 2, 15);
	addTable("<statement_list>", "write", new string[2]{ "<statement>","<statement_list>" }, 2, 16);
	addTable("<statement_list>", "ID", new string[2]{ "<statement>","<statement_list>" }, 2, 17);
	addTable("<statement_list>", "{", new string[2]{ "<statement>","<statement_list>" }, 2, 18);
	addTable("<statement_list>", "}", new string[1]{ "ε" }, 1, 19);
	addTable("<statement_list>", ";", new string[2]{ "<statement>","<statement_list>" }, 2, 20);

	addTable("<statement>", "if", new string[1]{ "<if_stat>" }, 1, 21);
	addTable("<statement>", "while", new string[1]{ "<while_stat>" }, 1, 22);
	addTable("<statement>", "for", new string[1]{ "<for_stat>" }, 1, 23);
	addTable("<statement>", "read", new string[1]{ "<read_stat>" }, 1, 24);
	addTable("<statement>", "write", new string[1]{ "<write_stat>" }, 1, 25);
	addTable("<statement>", "ID", new string[1]{ "<assignment_stat>" }, 1, 26);
	addTable("<statement>", "{", new string[1]{ "<compound_stat>" }, 1, 27);
	addTable("<statement>", ";", new string[1]{ ";" }, 1, 28);

	addTable("<if_stat>", "if", new string[9]{ "if","(","<bool_expression>","@jz",")","<statement>","@jmp","@setLabel","<if_stat>'" }, 9, 29);

	addTable("<if_stat>'", "if", new string[2]{ "ε","@setLabel" }, 2, 30);
	addTable("<if_stat>'", "while", new string[2]{ "ε","@setLabel" }, 2, 31);
	addTable("<if_stat>'", "else", new string[3]{ "else","<statement>","@setLabel" }, 3, 32);
	addTable("<if_stat>'", "for", new string[2]{ "ε","@setLabel" }, 2, 33);
	addTable("<if_stat>'", "read", new string[2]{ "ε","@setLabel" }, 2, 34);
	addTable("<if_stat>'", "write", new string[2]{ "ε","@setLabel" }, 2, 35);
	addTable("<if_stat>'", "ID", new string[2]{ "ε","@setLabel" }, 2, 36);
	addTable("<if_stat>'", "{", new string[2]{ "ε","@setLabel" }, 2, 37);
	addTable("<if_stat>'", "}", new string[2]{ "ε","@setLabel" }, 2, 38);
	addTable("<if_stat>'", ";", new string[2]{ "ε","@setLabel" }, 2, 39);

	addTable("<while_stat>", "while", new string[9]{ "while","(","@setLabel","<bool_expression>","@jz",")","<statement>","@jmp","@setLabel" }, 9, 40);

	addTable("<for_stat>", "for", new string[17]{ "for","(","<assignment_expression>",";","@setLabel","<bool_expression>",";","@jz","@jmp","@setLabel","<assignment_expression>","@jmp",")","@setLabel","<statement>","@jmp","@setLabel" }, 17, 41);

	addTable("<read_stat>", "read", new string[5]{ "read","ID","@name-look","@read",";" }, 5, 42);

	addTable("<write_stat>", "write", new string[4]{ "write","<arithmetic_expression>","@write",";" }, 4, 43);

	addTable("<compound_stat>", "{", new string[3]{ "{","<statement_list>","}" }, 3, 44);

	addTable("<assignment_stat>", "ID", new string[2]{ "<assignment_expression>",";" }, 2, 45);

	addTable("<bool_expression>", "ID", new string[2]{ "<arithmetic_expression>","<bool_expression>'" }, 2, 46);
	addTable("<bool_expression>", "NUM", new string[2]{ "<arithmetic_expression>","<bool_expression>'" }, 2, 47);
	addTable("<bool_expression>", "(", new string[2]{ "<arithmetic_expression>","<bool_expression>'" }, 2, 48);

	addTable("<bool_expression>'", ">", new string[3]{ ">","<arithmetic_expression>","@gt" }, 3, 49);
	addTable("<bool_expression>'", "<", new string[3]{ "<","<arithmetic_expression>","@les" }, 3, 50);
	addTable("<bool_expression>'", ">=", new string[3]{ ">=","<arithmetic_expression>","@ge" }, 3, 51);
	addTable("<bool_expression>'", "<=", new string[3]{ "<=","<arithmetic_expression>","@le" }, 3, 52);
	addTable("<bool_expression>'", "==", new string[3]{ "==","<arithmetic_expression>","@eq" }, 3, 53);
	addTable("<bool_expression>'", "!=", new string[3]{ "!=","<arithmetic_expression>","@noteq" }, 3, 54);

	addTable("<assignment_expression>", "ID", new string[5]{ "ID","@name-look","=","<arithmetic_expression>","@move" }, 5, 55);

	addTable("<arithmetic_expression>", "ID", new string[3]{ "<term>","@move","<arithmetic_expression>'" }, 3, 56);
	addTable("<arithmetic_expression>", "NUM", new string[3]{ "<term>","@move","<arithmetic_expression>'" }, 3, 57);
	addTable("<arithmetic_expression>", "(", new string[3]{ "<term>","@move","<arithmetic_expression>'" }, 3, 58);

	addTable("<arithmetic_expression>'", "+", new string[4]{ "+","<term>","@add","<arithmetic_expression>'" }, 4, 59);
	addTable("<arithmetic_expression>'", "-", new string[4]{ "-","<term>","@sub","<arithmetic_expression>'" }, 4, 60);
	addTable("<arithmetic_expression>'", ">", new string[1]{ "ε" }, 1, 61);
	addTable("<arithmetic_expression>'", "<", new string[1]{ "ε" }, 1, 62);
	addTable("<arithmetic_expression>'", ">=", new string[1]{ "ε" }, 1, 63);
	addTable("<arithmetic_expression>'", "<=", new string[1]{ "ε" }, 1, 64);
	addTable("<arithmetic_expression>'", "==", new string[1]{ "ε" }, 1, 65);
	addTable("<arithmetic_expression>'", "!=", new string[1]{ "ε" }, 1, 66);
	addTable("<arithmetic_expression>'", ";", new string[1]{ "ε" }, 1, 67);
	addTable("<arithmetic_expression>'", ")", new string[1]{ "ε" }, 1, 68);

	addTable("<term>", "ID", new string[3]{ "<factor>","@move","<term>'" }, 3, 69);
	addTable("<term>", "NUM", new string[3]{ "<factor>","@move","<term>'" }, 3, 70);
	addTable("<term>", "(", new string[3]{ "<factor>","@move","<term>'" }, 3, 71);

	addTable("<term>'", "+", new string[1]{ "ε" }, 1, 72);
	addTable("<term>'", "-", new string[1]{ "ε" }, 1, 73);
	addTable("<term>'", "*", new string[4]{ "*","<factor>","@mult","<term>'" }, 4, 74);
	addTable("<term>'", "/", new string[4]{ "/","<factor>","@div","<term>'" }, 4, 75);
	addTable("<term>'", ">", new string[1]{ "ε" }, 1, 76);
	addTable("<term>'", "<", new string[1]{ "ε" }, 1, 77);
	addTable("<term>'", ">=", new string[1]{ "ε" }, 1, 78);
	addTable("<term>'", "<=", new string[1]{ "ε" }, 1, 79);
	addTable("<term>'", "==", new string[1]{ "ε" }, 1, 80);
	addTable("<term>'", "!=", new string[1]{ "ε" }, 1, 81);
	addTable("<term>'", ";", new string[1]{ "ε" }, 1, 82);
	addTable("<term>'", ")", new string[1]{ "ε" }, 1, 83);

	addTable("<factor>", "ID", new string[2]{ "ID","@name-look" }, 2, 84);
	addTable("<factor>", "NUM", new string[1]{ "NUM" }, 1, 85);
	addTable("<factor>", "(", new string[4]{ "(","<arithmetic_expression>","@move",")" }, 4, 86);

	//非终结符
	symNs.insert("<program>");
	symNs.insert("<declaration_list>");
	symNs.insert("<declaration_stat>");
	symNs.insert("<statement_list>");
	symNs.insert("<statement>");
	symNs.insert("<if_stat>");
	symNs.insert("<if_stat>'");
	symNs.insert("<while_stat>");
	symNs.insert("<for_stat>");
	symNs.insert("<read_stat>");
	symNs.insert("<write_stat>");
	symNs.insert("<compound_stat>");
	symNs.insert("<assignment_stat>");
	symNs.insert("<bool_expression>");
	symNs.insert("<bool_expression>'");
	symNs.insert("<assignment_expression>");
	symNs.insert("<arithmetic_expression>");
	symNs.insert("<arithmetic_expression>'");
	symNs.insert("<term>");
	symNs.insert("<term>'");
	symNs.insert("<factor>");
}

//预读单词
int readWords()
{
	ifstream in;
	in.open("lex.txt");
	string s1, s2;
	int line, pos;
	while (in >> s1 >> s2 >> line >> pos)
	{
		words.push_back(make_pair(s1, s2));
		wordPos.push_back(make_pair(line, pos));
		if (s1 == "#" && s2 == "#")break;
	}
	words.push_back(make_pair("#", "#"));
	wordPos.push_back(make_pair(line, pos));
	readPos = 0;
	int errorCnt = 0;
	in >> errorCnt;
	in.close();
	return errorCnt;
}

pair<string, string> getSym(bool next = false)
{
	return next ? words[readPos + 1] : words[readPos++];
}

pair<int, int> getSymPos()
{
	return wordPos[readPos - 1];
}

bool judgeSymN(string s)
{
	return symNs.count(s) != 0;
}

void debug()
{
	stack<string> tmp;
	while (!analyStack.empty()) {
		string s = analyStack.top();
		tmp.push(analyStack.top());
		analyStack.pop();
		cout << s << " ";
	}
	cout << "\n--------------------" << endl;

	while (!tmp.empty()) {
		analyStack.push(tmp.top());
		tmp.pop();
	}
}

void error(char* msg = NULL)
{
	flag = 0;
	success = false;
	pair<int, int>pos = getSymPos();
	if (msg)
	{
		printf("行 %d 列 %d  语法错误: %s\n", pos.first, pos.second, msg);
		return;
	}
	vector<string> remainingStack; //栈内剩余的符号
	while (!analyStack.empty())
	{
		remainingStack.push_back(analyStack.top());
		analyStack.pop();
	}
	//<term> @ <arithmetic_expression>' @ ;
	if (curTopWord == ";" || remainingStack.size() > 3 && curTopWord == "<term>'"  && remainingStack[1] == "<arithmetic_expression>'" && remainingStack[3] == ";")
	{
		printf("行 %d 列 %d  语法错误: 缺少“;”(在“%s”的前面)\n", pos.first, pos.second, words[readPos - 1].second.c_str());
	}
	else if (curTopWord == ")")
	{
		printf("行 %d 列 %d  语法错误: 缺少“)”(在“%s”的前面)\n", pos.first, pos.second, words[readPos - 1].second.c_str());
	}
	else printf("行 %d 列 %d  语法错误！！\n", pos.first, pos.second);

}

//获得下一个临时变量的地址
string getNextTmpAddress() {
	return  "{" + to_string(nextTmpAddress++) + "}";
}

//处理产生式，根据产生式生成属性
void dealProduction(Attr& leftAttr, Production& p) {
	string* pLeft = leftAttr.str["value"];
	Attr attr;
 	if (p.id == 11) {
		//int ID @name-def ;
		string * pIdName = new string;
		attr.str["idName"] = pIdName;
		attrStack.push(emptyAttr); //;
		attrStack.push(attr); //@name-def
		attrStack.push(attr); //ID
		attrStack.push(emptyAttr); //int
	}
	else if (p.id == 55) {
		//addTable("<assignment_expression>", "ID", new string[5]{ "ID","@name-look","=","<arithmetic_expression>","@move" }, 5, 55);
		string* pIdName = new string;		ps.push_back(pIdName);
		string * pIdAddress = new string;   ps.push_back(pIdAddress);
		string* pExpValue = new string;     ps.push_back(pExpValue);

		*pExpValue = getNextTmpAddress();

		attrStack.push(*attr.add("value", pExpValue)->add("leftValue", pIdAddress)->add("idName",pIdName)); //@move
		attrStack.push(*attr.clear()->add("value", pExpValue)); //<arithmetic_expression>
		attrStack.push(emptyAttr); //=
		attrStack.push(*attr.clear()->add("idName", pIdName)->add("idAddress", pIdAddress)); //@name-look
		attrStack.push(*attr.clear()->add("idName", pIdName)); //ID
	}
	else if (p.id == 84) {
		//addTable("<factor>", "ID", new string[2]{ "ID","@name-look" }, 2, 84);
		string* pIdName = new string;	ps.push_back(pIdName);
		attrStack.push(*attr.add("idName", pIdName)->add("value", leftAttr.str["value"])); //@name-look
		attrStack.push(*attr.clear()->add("idName", pIdName)); //ID

	}
	else if (p.id == 85) {
		//addTable("<factor>", "NUM", new string[1]{ "NUM" }, 1, 85);
		attrStack.push(*attr.add("value", leftAttr.str["value"])); //@name-look
	}
	else if (p.id == 86) {
		//addTable("<factor>", "(", new string[4]{ "(","<arithmetic_expression>","@move",")" }, 4, 86);
		string* pValue = new string;	ps.push_back(pValue);
		*pValue = getNextTmpAddress();

		attrStack.push(emptyAttr); //)
		attrStack.push(*attr.add("value", pValue)->add("leftValue", leftAttr.str["value"])); //@move
		attrStack.push(*attr.add("value", pValue)); //<arithmetic_expression>  
		attrStack.push(emptyAttr); //(
	}
	else if (p.id == 69 || p.id == 70 || p.id == 71) {
		/*
		addTable("<term>", "ID", new string[3]{ "<factor>","@move","<term>'" }, 3, 69);
		addTable("<term>", "NUM", new string[3]{ "<factor>","@move","<term>'" }, 3, 70);
		addTable("<term>", "(", new string[3]{ "<factor>","@move","<term>'" }, 3, 71);*/
		string* pFactorValue = new string;		ps.push_back(pFactorValue);
		*pFactorValue = getNextTmpAddress();

		attrStack.push(*attr.add("value", leftAttr.str["value"])); //<term'>
		attrStack.push(*attr.clear()->add("value", pFactorValue)->add("leftValue", leftAttr.str["value"])); //@move
		attrStack.push(*attr.clear()->add("value", pFactorValue)); //<factor>
	}
	else if (p.id == 74 || p.id == 75) {
		/*	addTable("<term>'", "*", new string[4]{ "*","<factor>","@mult","<term>'" }, 4, 74);
		addTable("<term>'", "/", new string[4]{ "/","<factor>","@div","<term>'" }, 4, 75);*/
		string* pFactorValue = new string;		ps.push_back(pFactorValue);
		*pFactorValue = getNextTmpAddress();

		attrStack.push(*attr.clear()->add("value", leftAttr.str["value"])); //<term>'
		attrStack.push(*attr.clear()->add("leftValue", leftAttr.str["value"])->add("value", pFactorValue)); // @mult / div
		attrStack.push(*attr.clear()->add("value", pFactorValue)); // <factor>
		attrStack.push(emptyAttr); // * / /
	}
	else if (p.id == 56 || p.id == 57 || p.id == 58) {
		/*
		addTable("<arithmetic_expression>", "ID", new string[3]{ "<term>","@move","<arithmetic_expression>'" }, 3, 56);
		addTable("<arithmetic_expression>", "NUM", new string[3]{ "<term>","@move","<arithmetic_expression>'" }, 3, 57);
		addTable("<arithmetic_expression>", "(", new string[3]{ "<term>","@move","<arithmetic_expression>'" }, 3, 58);*/
		string* pTermValue = new string;		ps.push_back(pTermValue);
		*pTermValue = getNextTmpAddress();
		attrStack.push(*attr.add("value", leftAttr.str["value"])); //<arithmetic_expression>
		attrStack.push(*attr.clear()->add("value", pTermValue)->add("leftValue", leftAttr.str["value"])); //@move
		attrStack.push(*attr.clear()->add("value", pTermValue)); //<term>
	}
	else if (p.id == 59 || p.id == 60) {
		/*
		addTable("<arithmetic_expression>'", "+", new string[4]{ "+","<term>","@add","<arithmetic_expression>'" }, 4, 59);
		addTable("<arithmetic_expression>'", "-", new string[4]{ "-","<term>","@sub","<arithmetic_expression>'" }, 4, 60);*/
		string* pTermValue = new string;		ps.push_back(pTermValue);
		*pTermValue = getNextTmpAddress();

		attrStack.push(*attr.add("value", leftAttr.str["value"])); //<arithmetic_expression>'
		attrStack.push(*attr.clear()->add("value", pTermValue)->add("leftValue", leftAttr.str["value"])); //@add / sub
		attrStack.push(*attr.clear()->add("value", pTermValue)); //<term>
		attrStack.push(emptyAttr); // + / -
	}
	else if (p.id == 42) {
		//addTable("<read_stat>", "read", new string[4]{ "read","ID", "@name-look","@read",";" }, 4, 42);
		string* pIdName = new string;		ps.push_back(pIdName);
		string* pIdAddress = new string;	ps.push_back(pIdAddress);

		attrStack.push(emptyAttr); // ;
		attrStack.push(*attr.add("idAddress", pIdAddress)->add("idName", pIdName)); //@read
		attrStack.push(*attr.clear()->add("idAddress", pIdAddress)->add("idName", pIdName)); //@name-look
		attrStack.push(*attr.clear()->add("idName", pIdName)); //ID
		attrStack.push(emptyAttr); //read
	}
	else if (p.id == 43) {
		//addTable("<write_stat>", "write", new string[4]{ "write","<arithmetic_expression>","@write",";" }, 4, 43);

		string* pExpValue = new string;		ps.push_back(pExpValue);
		*pExpValue = getNextTmpAddress();

		attrStack.push(emptyAttr); //;
		attrStack.push(*attr.add("value", pExpValue)); //@write
		attrStack.push(attr); //<arithmetic_expression>
		attrStack.push(emptyAttr); //write
	}
	else if (p.id == 46 || p.id == 47 || p.id == 48) {
		//addTable("<bool_expression>", "ID", new string[2]{ "<arithmetic_expression>","<bool_expression>'" }, 2, 46);
		string* pExpValue = new string;		ps.push_back(pExpValue);
		*pExpValue = getNextTmpAddress();

		attrStack.push(*attr.add("boolValue", leftAttr.str["boolValue"])->add("expValue", pExpValue)); //<bool_expression>'
		attrStack.push(*attr.clear()->add("value", pExpValue)); //<arithmetic_expression>
	}
	else if (p.id >= 49 && p.id <= 54) {
		//addTable("<bool_expression>'", ">", new string[3]{ ">","<arithmetic_expression>","@gt" }, 3, 49);
		string* pExpValue = new string;		ps.push_back(pExpValue);
		*pExpValue = getNextTmpAddress();

		attrStack.push(*attr.add("boolValue", leftAttr.str["boolValue"])->add("leftExpValue", leftAttr.str["expValue"])->add("rightExpValue", pExpValue)); //@action
		attrStack.push(*attr.clear()->add("value", pExpValue)); //<arithmetic_expression>
		attrStack.push(emptyAttr); //opt
	}
	else if (p.id == 29) {
		//addTable("<if_stat>", "if", new string[9]{ "if","(","<bool_expression>","@jz",")","<statement>","@jmp","@setLabel","<if_stat>'" }, 9, 29);
		string* label1 = new string; string* label2 = new string;
		ps.push_back(label1); ps.push_back(label2);
		*label1 = to_string(nextLabel++);
		*label2 = to_string(nextLabel++);

		string* boolValue = new string;		ps.push_back(boolValue);
		*boolValue = getNextTmpAddress();

		attrStack.push(*attr.add("label", label2)); //<if_stat>'
		attrStack.push(*attr.clear()->add("label", label1)); //@setLabel1
		attrStack.push(*attr.clear()->add("label", label2)); //@jmp
		attrStack.push(emptyAttr); //<statement>
		attrStack.push(emptyAttr); //)
		attrStack.push(*attr.clear()->add("value", boolValue)->add("label", label1)); //@jz
		attrStack.push(*attr.clear()->add("boolValue", boolValue)); //<bool_expression>
		attrStack.push(emptyAttr); //(
		attrStack.push(emptyAttr); //if
	}
	else if (p.id >= 30 && p.id <= 39 && p.id != 32) {
		//addTable("<if_stat>'", "if", new string[2]{ "ε","@setLabel" }, 2, 30);
		attrStack.push(*attr.add("label", leftAttr.str["label"]));
	}
	else if (p.id == 32) {
		//addTable("<if_stat>'", "else", new string[3]{ "else","<statement>","@setLabel" }, 3, 32);
		attrStack.push(*attr.add("label", leftAttr.str["label"])); //setlabel
		attrStack.push(emptyAttr); //statement
		attrStack.push(emptyAttr); //else
	}
	else if (p.id == 40) {
		//addTable("<while_stat>", "while", new string[9]{ "while","(","@setLabel","<bool_expression>","@jz",")","<statement>","@jmp","@setLabel" }, 9, 40);
		string* label1 = new string; string* label2 = new string;
		ps.push_back(label1); ps.push_back(label2);
		*label1 = to_string(nextLabel++);
		*label2 = to_string(nextLabel++);

		string* boolValue = new string;		ps.push_back(boolValue);
		*boolValue = getNextTmpAddress();

		attrStack.push(*attr.add("label", label2)); //@setLabel
		attrStack.push(*attr.clear()->add("label", label1)); //@jmp
		attrStack.push(emptyAttr); //<statement>
		attrStack.push(emptyAttr); //)
		attrStack.push(*attr.clear()->add("label", label2)->add("value", boolValue)); //@jz
		attrStack.push(*attr.clear()->add("boolValue", boolValue)); //<bool_expression>
		attrStack.push(*attr.clear()->add("label", label1)); //@setLabel
		attrStack.push(emptyAttr); //(
		attrStack.push(emptyAttr); //while
	}
	else if (p.id == 41) {
		//addTable("<for_stat>", "for", new string[17]{ "for","(","<assignment_expression>",";","@setLabel","<bool_expression>",";","@jz","@jmp","@setLabel","<assignment_expression>","@jmp",")","@setLabel","<statement>","@jmp","@setLabel" }, 17, 41);
		string* label1 = new string; string* label2 = new string; string* label3 = new string; string* label4 = new string;
		ps.push_back(label1); ps.push_back(label2); ps.push_back(label3); ps.push_back(label4);
		*label1 = to_string(nextLabel++);
		*label2 = to_string(nextLabel++);
		*label3 = to_string(nextLabel++);
		*label4 = to_string(nextLabel++);

		string* boolValue = new string;  ps.push_back(boolValue);
		*boolValue = getNextTmpAddress();

		attrStack.push(*attr.clear()->add("label", label2)); //@setLabel
		attrStack.push(*attr.clear()->add("label", label3)); //@jmp
		attrStack.push(emptyAttr); //<statement>
		attrStack.push(*attr.clear()->add("label", label4)); //@setLabel
		attrStack.push(emptyAttr); //)
		attrStack.push(*attr.clear()->add("label", label1)); //@jmp
		attrStack.push(emptyAttr); //<assignment_expression>
		attrStack.push(*attr.clear()->add("label", label3)); //@setLabel
		attrStack.push(*attr.clear()->add("label", label4)); //@jmp
		attrStack.push(*attr.clear()->add("label", label2)->add("value", boolValue)); //@jz
		attrStack.push(emptyAttr); //;
		attrStack.push(*attr.clear()->add("boolValue", boolValue)); //<bool_expression>
		attrStack.push(*attr.clear()->add("label", label1)); //@setLabel
		attrStack.push(emptyAttr); //;
		attrStack.push(emptyAttr); //<assignment_expression>
		attrStack.push(emptyAttr); //(
		attrStack.push(emptyAttr); //for
	}
	else {
		for (auto it = p.word.rbegin(); it != p.word.rend(); it++)
		{
			if (*it == "ε")continue;
			attrStack.push(emptyAttr);
		}
	}

}

void dealAction(string& actionName, Attr& actionAttr) {
	if (actionName == "@name-def" && actionAttr.str.count("idName")) {
		if (symbolTable.count(*actionAttr.str["idName"])) {
			char errorMsg[100];
			sprintf(errorMsg, "“int %s”: 重定义", actionAttr.str["idName"]->c_str());
			error(errorMsg);
		}
		SymbolItem item;
		item.address = nextSymbolAddress++;
		item.name = *actionAttr.str["idName"];
		item.init = false;
		symbolTable[item.name] = item;
		out << "new\t_\t_\t[" << item.address << "]\n";
	}

	else if (actionName == "@name-look" && actionAttr.str.count("idName")) {
		if (symbolTable.count(*actionAttr.str["idName"])) {
			string ad = "[" + to_string(symbolTable[*actionAttr.str["idName"]].address) + "]";
			if (actionAttr.str.count("idAddress")) {
				//symbolTable[*actionAttr.str["idName"]].init = true;
				*actionAttr.str["idAddress"] = ad;
			}

			if (actionAttr.str.count("value"))	{
				if (!symbolTable[*actionAttr.str["idName"]].init) {
					char errorMsg[100];
					sprintf(errorMsg, "使用了未初始化的局部变量“%s”", actionAttr.str["idName"]->c_str());
					error(errorMsg);
				}else
					*actionAttr.str["value"] = ad;
			}
		}
		else {
			char errorMsg[100];
			sprintf(errorMsg, "“%s”: 未声明的标识符", actionAttr.str["idName"]->c_str());
			error(errorMsg);
		}
	}
	else if (actionName == "@move") {
		if (actionAttr.str.count("idName")) {
			symbolTable[*actionAttr.str["idName"]].init = true;
		}
		out << "move\t" << *actionAttr.str["value"] << "\t_\t" << *actionAttr.str["leftValue"] << "\n";
	}
	else if (actionName == "@mult") {
		out << "mult\t" << *actionAttr.str["leftValue"] << "\t" << *actionAttr.str["value"] << "\t" << *actionAttr.str["leftValue"] << "\n";
	}
	else if (actionName == "@div") {
		out << "div\t" << *actionAttr.str["leftValue"] << "\t" << *actionAttr.str["value"] << "\t" << *actionAttr.str["leftValue"] << "\n";
	}
	else if (actionName == "@add") {
		out << "add\t" << *actionAttr.str["leftValue"] << "\t" << *actionAttr.str["value"] << "\t" << *actionAttr.str["leftValue"] << "\n";
	}
	else if (actionName == "@sub") {
		out << "sub\t" << *actionAttr.str["leftValue"] << "\t" << *actionAttr.str["value"] << "\t" << *actionAttr.str["leftValue"] << "\n";
	}
	else if (actionName == "@read") {
		if (actionAttr.str.count("idName")) {
			symbolTable[*actionAttr.str["idName"]].init = true;
		}
		out << "read\t_\t_\t" << *actionAttr.str["idAddress"] << "\n";
	}
	else if (actionName == "@write") {
		out << "write\t" << *actionAttr.str["value"] << "\t_\t_\n";
	}
	else if (actionName == "@gt" || actionName == "@les" || actionName == "@ge" || actionName == "@le" || actionName == "@eq" || actionName == "@noteq") {
		out << actionName.substr(1) << "\t" << *actionAttr.str["leftExpValue"] << "\t" 
		<< *actionAttr.str["rightExpValue"] << "\t" << *actionAttr.str["boolValue"] << "\n";
	}
	else if (actionName == "@jmp") {  //无条件跳转
		out << "jmp\t" << *actionAttr.str["label"] << "\t_\t_\n";
	}
	else if (actionName == "@jz") {  //等于零跳转
		out << "jz\t" << *actionAttr.str["value"] << "\t" << *actionAttr.str["label"] << "\t_\n";
	}
	else if (actionName == "@setLabel") {
		out << "setLabel\t" << *actionAttr.str["label"] << "\t_\t_\n";
	}
}


//处理属性
void dealAttr(string& word, Attr& attr, pair<string, string>& sym) {
	if (word == "ID"){
		if (attr.str.count("idName") != 0 && sym.first == "ID")
			*attr.str["idName"] = sym.second;
	}
	else if (word == "NUM") {
		if (attr.str.count("value") != 0 && sym.first == "NUM")
			*attr.str["value"] = sym.second;
	}
}


void analy()
{
	analyStack.push({ "#" });			  attrStack.push(emptyAttr);
	analyStack.push({ "<program>" });     attrStack.push(emptyAttr);
	success = true;
	flag = 1;
	nextSymbolAddress = 0;
	nextTmpAddress = 0;
	nextLabel = 1;
	pair<string, string> sym = getSym();
	string str = sym.first;
	while (flag)
	{
		curTopWord = analyStack.top();
		curTopAttr = attrStack.top();
		attrStack.pop();
		analyStack.pop();
		if (curTopWord[0] != '@')
		{
			dealAttr(curTopWord, curTopAttr, sym);
			if (!judgeSymN(curTopWord)) //判断栈顶是否为终结符
			{
				if (curTopWord == str)
				{
					if (curTopWord == "#")
					{
						flag = 0;
						continue;
					}
					sym = getSym();
					str = sym.first;
				}
				else error();
			}
			else if (analyTable.count(make_pair(curTopWord, str)) != 0)
			{
				Production& p = analyTable[make_pair(curTopWord, str)];
				//将右部反向入栈
				for (auto it = p.word.rbegin(); it != p.word.rend(); it++)
				{
					if (*it == "ε")continue;
					analyStack.push(*it);
				}
				dealProduction(curTopAttr, p);
				//debug();
				continue;
			}
			else error();
		}
		else
		{
			//处理动作
			dealAction(curTopWord, curTopAttr);

		}
	}
	if (success) {
		out << "end\t_\t_\t_\n";
		printf("语法分析通过.\n");
	}
}

int main()
{
	//system("LexAnaly.exe in.txt");
	int lexErrorCnt = readWords();
	if (lexErrorCnt > 0) {
		printf("发现%d处词法错误！\n", lexErrorCnt);
		return 0;
	}
	initTable();
	analy();
	for (auto it = ps.begin(); it != ps.end(); it++)
		delete *it;
	return 0;
}