#include"responder.h"
#include<iostream>
using namespace std;

extern int sockClient;
extern string response;

static bool deb = 0;

char buf[2300005];

bool Response::count(const string &s){
    for (auto x:name){
        if (x == s) return true;
    }
    return false;
}

void Response::set(string s,string t){
    int sz = name.size();
    for (int i=0;i<sz;i++){
        if (name[i] == s) value[i] = t;
    }
}

void Response::add(string s,string t){
    if (count(s)) set(s,t);
    else{
        name.push_back(s);
        value.push_back(t);
    }
}

Response::Response(){
    add("HTTP/1.1 200 OK","");
    add("Content-Type:","");
    add("charset=","utf-8");
    flag = 1;
}

bool Response::ok(){
    return flag;
}

void Response::sendHeader(){
    int sz = name.size();
    string response = {};
    for (int i=sz-1;i>=0;i--){
        response = name[i]+value[i]+'\n'+response;
    }
    response+='\n';
    char buf[1005] = {};
    sz = response.length();
    for (int i=0;i<sz;i++) buf[i] = response[i];
    // buf[sz] = '\0';
    sz = send(sockClient, buf, sz, 0);
    if (sz != 0) flag = 0;
    if (deb) cout<<"Header:\n"<<response<<"\nend"<<endl;
}

void Response::sendBody(char *buf,int n){
    n = send(sockClient,buf,n,MSG_NOSIGNAL);
    cout<<"Return status: "<<n<<endl;
    if (n != 0) flag = 0;
    if (deb) cout<<"Body:\n"<<buf<<endl;
}

void Response::sendBody(string &s,int n){
    for (int i=0;i<n;i++) buf[i] = s[i];
    n = send(sockClient,buf,n,MSG_NOSIGNAL);
    cout<<"Return status: "<<n<<endl;
    if (n != 0) flag = 0;
    if (deb) cout<<"Body:\n"<<s<<endl;
}