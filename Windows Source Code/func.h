#pragma once
#include<iostream>
#include<map>
#include<string>
#include<cstring>
#include<windows.h>
using namespace std;
const int p = 131,mod = 998244353;

void gettime(string &s);

int Hash(char *s);

int Hash(string s);

string getNext(string &s,int &pos,char end);

bool checkEndFile(char *s,int beg,const char *t);

void buildIndexingTree();