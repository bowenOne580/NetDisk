#pragma once
#include<iostream>
#include<string>
#include <windows.h>
#include <vector>
#include<map>
#include<algorithm>
using namespace std;

#define DECLARE_CTRL_FUN(n) public: virtual void f##n(string a);
#define IMPLEMENT_CTRL_FUN(className,n,funName) void className::f##n(string a) { funName(a); }
#define REGISTER_CTRL_FUN(method,path,className,insName,n) addRouting(method,path,insName,(CTRL_FUN)&className::f##n);
#define CREATE_INSTANCE(className,InsName) className *InsName = new className;

struct ParameterElement{
    bool isCreated;
    string name,value;
};

class Controller
{
public:
    virtual void f0(string path){}
    virtual void f1(string path){}
    virtual void f2(string path){}
    virtual void f3(string path){}
    virtual void f4(string path){}
    virtual void f5(string path){}
    bool getPars(string &path);
    void OkResponse();
    void htmlNormalShow(char *s);
    virtual void show(char *fullPath);
    virtual void analy(string &s){}
protected:
    vector<ParameterElement> pars;
    map<int,ParameterElement *> hashes;
};

typedef void (Controller::*CTRL_FUN)(string);

struct RoutingElement
{
    string method;
    string path;
    Controller *object;
    CTRL_FUN fun;
};

class Router
{
public:
    Router();
    void setupRouting();
    void handle(string method, string path);

private:
    vector<RoutingElement> table;

protected:
    void addRouting(string method, string path, Controller *obj, CTRL_FUN fun);
};