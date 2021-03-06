// VirtualMachine.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "stdafx.h"
#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#pragma warning(disable:4996)
using namespace std;

int ram[1000]; //内存空间
int cache[1000]; //缓存空间
map<int, int> lableMap; //标号映射

//指令结构体
struct Instruction
{
	string op;			//操作
	string first;		//第一个操作值
	string second;		//第二个操作值
	string position;	//存放位置
};

//指令集
vector<Instruction> instructions;

//得到字符串转换后的值
int getValue(string value) {
	string temp;
	int result;
	if (value.find("[") != string::npos)
	{
		temp = value.substr(1, value.length() - 2);
		result = ram[stoi(temp)];
	}
	else if (value.find("{") != string::npos)
	{
		temp = value.substr(1, value.length() - 2);
		result = cache[stoi(temp)];
	}
	else
	{
		result = stoi(value);
	}
	return result;
}

//得到具体位置
int getPosition(string value)
{
	return stoi(value.substr(1, value.length() - 2));
}

//处理四元式
void handle(string op, string first, string second, string position) {
	if (op.compare("move") == 0)
	{
		if (position.find("[") != string::npos)
		{
			ram[getPosition(position)] = getValue(first);
		}
		else if (position.find("{") != string::npos)
		{
			cache[getPosition(position)] = getValue(first);
		}
	}
	else if (op.compare("add") == 0)
	{
		if (position.find("[") != string::npos)
		{
			ram[getPosition(position)] = getValue(first) + getValue(second);
		}
		else if (position.find("{") != string::npos)
		{
			cache[getPosition(position)] = getValue(first) + getValue(second);
		}
	}
	else if (op.compare("sub") == 0)
	{
		if (position.find("[") != string::npos)
		{
			ram[getPosition(position)] = getValue(first) - getValue(second);
		}
		else if (position.find("{") != string::npos)
		{
			cache[getPosition(position)] = getValue(first) - getValue(second);
		}
	}
	else if (op.compare("mult") == 0)
	{
		if (position.find("[") != string::npos)
		{
			ram[getPosition(position)] = getValue(first) * getValue(second);
		}
		else if (position.find("{") != string::npos)
		{
			cache[getPosition(position)] = getValue(first) * getValue(second);
		}
	}
	else if (op.compare("div") == 0)
	{
		if (position.find("[") != string::npos)
		{
			ram[getPosition(position)] = getValue(first) / getValue(second);
		}
		else if (position.find("{") != string::npos)
		{
			cache[getPosition(position)] = getValue(first) / getValue(second);
		}
	}
	else if (op.compare("gt") == 0)
	{
		if (position.find("[") != string::npos)
		{
			ram[getPosition(position)] = getValue(first) > getValue(second);
		}
		else if (position.find("{") != string::npos)
		{
			cache[getPosition(position)] = getValue(first) > getValue(second);
		}
	}
	else if (op.compare("les") == 0)
	{
		if (position.find("[") != string::npos)
		{
			ram[getPosition(position)] = getValue(first) < getValue(second);
		}
		else if (position.find("{") != string::npos)
		{
			cache[getPosition(position)] = getValue(first) < getValue(second);
		}
	}
	else if (op.compare("ge") == 0)
	{
		if (position.find("[") != string::npos)
		{
			ram[getPosition(position)] = getValue(first) >= getValue(second);
		}
		else if (position.find("{") != string::npos)
		{
			cache[getPosition(position)] = getValue(first) >= getValue(second);
		}
	}
	else if (op.compare("le") == 0)
	{
		if (position.find("[") != string::npos)
		{
			ram[getPosition(position)] = getValue(first) <= getValue(second);
		}
		else if (position.find("{") != string::npos)
		{
			cache[getPosition(position)] = getValue(first) <= getValue(second);
		}
	}
	else if (op.compare("eq") == 0)
	{
		if (position.find("[") != string::npos)
		{
			ram[getPosition(position)] = getValue(first) == getValue(second);
		}
		else if (position.find("{") != string::npos)
		{
			cache[getPosition(position)] = getValue(first) == getValue(second);
		}
	}
	else if (op.compare("noteq") == 0)
	{
		if (position.find("[") != string::npos)
		{
			ram[getPosition(position)] = getValue(first) != getValue(second);
		}
		else if (position.find("{") != string::npos)
		{
			cache[getPosition(position)] = getValue(first) != getValue(second);
		}
	}
	else if (op.compare("read") == 0)
	{
		int v;
		cin >> v;
		ram[getPosition(position)] = v;
	}
	else if (op.compare("write") == 0)
	{
		cout << getValue(first) << endl;
	}

}

//读取并处理文件
void readAndHandle(ifstream& in) {
	string op;
	string first;
	string second;
	string position;
	int count = 0;
	bool flag = false;
	memset(ram,0,sizeof(ram));
	while (in >> op >> first >> second >> position)
	{
		if (op == "end") { flag = true; break; }
		Instruction i;
		i.op = op;
		i.first = first;
		i.second = second;
		i.position = position;
		if (i.op.compare("setLabel") == 0)
		{
			lableMap.insert(pair<int, int>(stoi(first), count));
		}
		instructions.push_back(i);
		count++;
	}
	in.close();
	if (!flag) return;
	//system("cls");
	for (unsigned int i = 0; i < instructions.size(); i++)
	{
		if (instructions[i].op.compare("jmp") == 0) //jmp 无条件跳
		{
			i = lableMap[stoi(instructions[i].first)];
		}
		else if (instructions[i].op.compare("jz") == 0) //jz 不等于则跳
		{
			if (getValue(instructions[i].first) == 0)
			{
				i = lableMap[stoi(instructions[i].second)];
			}
		}
		else
		{
			handle(instructions[i].op, instructions[i].first, instructions[i].second, instructions[i].position);
		}
	}
}

int main(int argc, char **argv)
{
	char* inFilePath = NULL;
	ifstream in;
	if (argc > 1)
		in.open(argv[1]);
	else in.open("instruction.txt");
	readAndHandle(in);
	in.close();
	instructions.clear();
	return 0;
}