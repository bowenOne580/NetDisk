#include"router.h"
#include"func.h"
#include"responder.h"
#include<fstream>
using namespace std;

int debug = 0;
extern map<int,void(*)(string &)> nodeid;
extern string response;
extern Response resp;

void Router::addRouting(string method, string path, Controller *obj, CTRL_FUN fun)
{
    RoutingElement ele;
    ele.method = method;
    ele.path = path;
    ele.object = obj;
    ele.fun = fun;
    table.push_back(ele);
}

void Router::handle(string method, string path)
{
    int flag = 0,sz = table.size(),len = path.length();
    string noParPath = {};
    for (int i=0;i<len;i++){
        if (path[i] == '?') break;
        noParPath+=path[i];
    }
    for (int i = 0; i < sz; i++)
    {
        if (table[i].method == method && table[i].path == noParPath)
        {
            ((*table[i].object).*(table[i].fun))(path);
            flag = 1;
            break;
        }
    }
    if (!flag) ((*table[1].object).*(table[1].fun))(path);
}

void Controller::OkResponse(){
    response = "OK";
    int len = response.length();
    resp.add("Content-Type:","text/plain;");
    resp.add("Content-Length:",to_string(len-1));
    resp.sendHeader();
    resp.sendBody(response,len);
}

bool Controller::getPars(string &path){
    string name = {},value = {};
    int len = path.length();
    bool valid = 1;
    for (auto par:pars) par.isCreated = 0;
    //预处理编码，将非ASCII转为ASCII存储
    string tmp = {};
    int now = -1;
    auto getHex = [=](int i)->int{
        if (path[i]>='A' && path[i]<='F') return path[i]-'A'+10;
        return path[i]-'0';
    };
    for (int i=0;i<len;i++){
        if (path[i] == '%'){
            tmp+=getHex(i+1)*16+getHex(i+2);
            i+=2;
        }
        else tmp+=path[i];
    }
    path = tmp;
    len = path.length();
    //解析参数
    for (int i=0;i<len;i++){
        if (path[i] == '?' || path[i] == '&'){
            value = {},name = {};
            int curr = i+1,flag = 0;
            while (path[curr]!='&' && curr<len){
                if (path[curr] == '=') flag = 1;
                else{
                    if (!flag) name+=path[curr];
                    else value+=path[curr];
                }
                curr++;
            }
            cout<<"Get parameter named "<<name;
            if (flag) cout<<" with value of "<<value<<endl;
            else cout<<" without value"<<endl;
            flag = 0;
            for (auto &par:pars){
                if (par.name == name){
                    flag = 1;
                    par.isCreated = 1,par.value = value;
                    break;
                }
            }
            if (!flag) valid = 0;
            i = curr-1;
        }
    }
    return valid;
}

void Controller::show(char *fullPath){
    string ret = {},s = {};
    ifstream input(fullPath);
    while (getline(input,s)){
        int len = s.length();
        for (int i=0;i<len;i++){
            if (i<len-4 && s[i] == '<' && s[i+1] == '!' && s[i+2] == '-'){
                //<!--xxx-->
                int head = i+4,end = i+4;
                while (s[end]!='>') end++;
                string k = {};
                for (int j=head;j<=end-3;j++) k+=s[j];
                int hashValue = Hash(k);
                if (!nodeid.count(hashValue)) analy(k);
                else nodeid[hashValue](k);
                ret+=k;
                i = end;
            }
            else ret+=s[i];
        }
        ret+="\n";
    }
    if (debug) cout<<ret<<endl;
    response = ret;
}

void Controller::htmlNormalShow(char *fullPath){
    show(fullPath);
    resp.add("Content-Type:","text/html;");
    resp.add("Content-Length:",to_string(response.length()));
    resp.sendHeader();
    resp.sendBody(response,response.length());
}
