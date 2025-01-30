#include"func.h"
map<int,void(*)(string &)> nodeid;
void gettime(string &s){
    s = "";
    time_t Time = time(0);
    tm* Info = localtime(&Time);
    s+=to_string(Info->tm_year+1900)+"."+to_string(Info->tm_mon+1)+"."+to_string(Info->tm_mday)+" ";
    if (Info->tm_hour<10) s+="0";
    s+=to_string(Info->tm_hour)+":";
    if (Info->tm_min<10) s+="0";
    s+=to_string(Info->tm_min)+":";
    if (Info->tm_sec<10) s+='0';
    s+=to_string(Info->tm_sec);
}

void showDir(string &s){
    s = "<div class=\"Dir\"><a href=\"\\\">Main Page</a><a href=\"/prime\">Find Prime Numbers</a><a href=\"/rbook\">Offline Netdisk</a></div>";
}

int Hash(char *s){
    int len = strlen(s);
    long long ret = 0;
    for (int i=0;i<len;i++) ret = (ret*p+(int)s[i])%mod;
    ret = (ret*p+len)%mod;
    return ret;
}

int Hash(string s){
    int len = s.length();
    long long ret = 0;
    for (int i=0;i<len;i++) ret = (ret*p+(int)s[i])%mod;
    ret = (ret*p+len)%mod;
    return ret;
}

string getNext(string &s,int &pos,char end){
    string tmp = {};
    int len = s.length();
    while (s[pos]!=end && pos<s.length()){
        tmp+=s[pos];
        pos++;
    }
    return tmp;
}

bool checkEndFile(char *t,int beg,const char *s){
    int len = strlen(s);
    for (int i=0;i<len;i++){
        if (t[beg+i]!=s[i]) return false;
    }
    return true;
}

void buildIndexingTree(){ //tag to function
    nodeid[Hash("Time")] = gettime;
    nodeid[Hash("DirCol")] = showDir;
}