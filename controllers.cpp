#include<fstream>
#include<string>
#include<cmath>
#include<codecvt>
#include<locale>
#include<cstdlib>
#include"router.h"
#include"func.h"
#include"responder.h"
#include <unistd.h>
#include<sys/types.h>
#include<sys/stat.h>

static bool deb = 0;
char absolPath[1005],Data[2300005];
char tmpData[2300005];

extern int blockLen,fileSize;
extern Response resp;
extern map<int,void(*)(string &)> nodeid;
extern string response,ContentName;

class _404:public Controller{
    DECLARE_CTRL_FUN(0);
public:
    void exec(string path){
        string ret = {},tmp = {};
        char fullPath[1005] = {};
        strcpy(fullPath,absolPath);
        strcat(fullPath,"/site/404.html");
        htmlNormalShow(fullPath);
    }
} tmp_404;
IMPLEMENT_CTRL_FUN(_404,0,exec);

class MainPage:public Controller{
    DECLARE_CTRL_FUN(0);
public:
    void exec(string path){
        if (!getPars(path)){
            cout<<"Parameter Invalid"<<endl;
            tmp_404.f0(path);
            return;
        }
        char fullPath[1005] = {};
        strcpy(fullPath,absolPath);
        strcat(fullPath,"/site/index.html");
        htmlNormalShow(fullPath);
    }
    char s[1005];
};
IMPLEMENT_CTRL_FUN(MainPage,0,exec);

const int PriN = 1e6+10;
class Prime:public Controller{
    DECLARE_CTRL_FUN(0);
    DECLARE_CTRL_FUN(1);
    public:
        void euler(){
            for (int i=2;i<=PriN-10;i++){
                if (!vis[i]) vis[i] = pri[++tot] = i;
                for (int j=1;j<=tot;j++){
                    if (vis[i]<pri[j] || 1ll*i*pri[j]>PriN-10) break;
                    vis[i*pri[j]] = pri[j];
                }
            }
        }
        void findprime(string & s){
            int par = stoi(pars[0].value);
            if (par>tot) s = "n is too large!<br>";
            else s = "The "+to_string(par);
            if (par == 1) s+="st";
            else if (par == 2) s+="nd";
            else s+="th";
            s+=" prime number is "+to_string(pri[par])+"<br>";
        }
        void queryprime(string &s){
            getPars(s);
            response = "{\n\"prime\": ";
            int par = stoi(pars[0].value);
            if (par>tot) response+="-1";
            else response+=to_string(pri[par]);
            response+="\n}";
            // cout<<response<<endl;
            resp.add("Content-Type:","application/json;");
            resp.add("Content-Length:",to_string(response.length()));
            resp.sendHeader();
            resp.sendBody(response,response.length());
        }
        void listPrime(string &s){
            if (!pars[2].isCreated) pars[2].value = pars[1].value;
            int cnt = 0,lb = stoi(pars[1].value),ub = stoi(pars[2].value);
            int pos = lower_bound(pri+1,pri+1+tot,lb)-pri;
            s = {};
            while (pos<=tot && pri[pos]>=lb && pri[pos]<=ub){
                s+="<li>"+to_string(pri[pos])+"</li>\n";
                pos++,cnt++;
            }
            s = "A total of "+to_string(cnt)+" primes found in given range ["+pars[1].value+","+pars[2].value+"]\n"+s;
        }
        void checkPrime(string &s){
            int len = pars[3].value.length();
            if (len>16){
                s = "Number too large, can't check<br>";
                return;
            }
            long long x = stoll(pars[3].value);
            int lim = sqrtl(x),flag = 1;
            for (int i=2;i<=lim;i++){
                if (x%i == 0){
                    flag = 0;
                    break;
                }
            }
            s = "Number "+pars[3].value+" is";
            if (!flag) s+=" not";
            s+=" a prime<br>";
        }
        Prime();
        void analy(string &s);
        void exec(string path){
            for (auto x:pars) x.isCreated = 0;
            if (!getPars(path)){
                cout<<"Parameter Invalid"<<endl;
                tmp_404.f0(path);
                return;
            }
            char fullPath[1005] = {};
            strcpy(fullPath,absolPath);
            strcat(fullPath,"/site/prime/prime.html");
            show(fullPath);
            resp.add("Content-Type:","text/html;");
            resp.add("Content-Length:",to_string(response.length()));
            resp.sendHeader();
            int len = response.length();
            int num = ceil(1.0*len/blockLen),res = len-(num-1)*blockLen,now = 0;
            for (int i=1;i<num;i++){
                for (int j=0;j<blockLen;j++){
                    Data[j] = response[now++];
                }
                resp.sendBody(Data,blockLen);
            }
            for (int i=0;i<res;i++){
                Data[i] = response[now++];
            }
            resp.sendBody(Data,res);
        }
    private:
        map<int,void(Prime::*)(string &)> funcid;
        int vis[PriN],pri[PriN],tot;
};
Prime *thisPrime;
Prime::Prime(){
    thisPrime = this;
    tot = 0;
    for (int i=2;i<=PriN-10;i++) vis[i] = pri[i] = 0;
    euler();
    if (deb) cout<<"First "<<tot<<" primes caculated"<<endl;
    pars.push_back({0,"n",""});
    pars.push_back({0,"lb",""});
    pars.push_back({0,"ub",""});
    pars.push_back({0,"isp",""});
    hashes[Hash("Prime")] = &pars[0];
    hashes[Hash("List")] = &pars[1];
    hashes[Hash("IsPrime")] = &pars[3];
    funcid[Hash("Prime")] = &Prime::findprime;
    funcid[Hash("List")] = &Prime::listPrime;
    funcid[Hash("IsPrime")] = &Prime::checkPrime;
}
void Prime::analy(string &s){
    int hashValue = Hash(s);
    if (hashes.count(hashValue) && hashes[hashValue]->isCreated){
        (thisPrime->*funcid[hashValue])(s);
    }
    else s = {};
}
IMPLEMENT_CTRL_FUN(Prime,0,exec);
IMPLEMENT_CTRL_FUN(Prime,1,queryprime);

struct Resources{
    /*
    类型1：路�?
    类型2：文�?
    类型3：大�?
    类型4：创建时�?
    */
    string s;
    int type;
};
struct Node{
    int fatherId,type;
    vector<int> sonId;
    string name,path;
    vector<Resources> res;
};
int gloCnt;

class TopicTree:public Controller{
    /*
    树形组织方式�?
    nodeid(fatherid):name,type;(禁止出现空格)
    Type说明�?
    0:不可修改，根节点
    1:可修改，普通节�?
    2:可修改，问题节点
    3:可修改，文件节点
    其他说明�?
    节点可重名，但需要由路径Hash作为唯一标识符，考虑维护map
    */
    DECLARE_CTRL_FUN(0);
    DECLARE_CTRL_FUN(1);
    DECLARE_CTRL_FUN(2);
    DECLARE_CTRL_FUN(3);
    DECLARE_CTRL_FUN(4);
    DECLARE_CTRL_FUN(5);
    public:
        TopicTree();
        //以下为共用代�?
        void initPars(string &path){
            for (auto x:pars) x.isCreated = 0;
            pars[0].value = "/root";
            if (!getPars(path)){
                cout<<"Parameter Invalid"<<endl;
                tmp_404.f0(path);
                return;
            }
        }
        //以下为信息存储与维护模块
        void build(){
            ifstream input(dbpath);
            string s;
            getline(input,s);
            tot = stoi(s);
            tre.clear();
            ban.clear();
            id.clear();
            tre.resize(tot+1);
            ban.resize(tot+1);
            id.resize(tot+1);
            pathid.clear();
            while (getline(input,s)){
                int len = s.length(),id = 0,faid = 0,tp = 0;
                string name = {};
                int now = 0;
                id = stoi(getNext(s,now,'('));
                faid = stoi(getNext(s,++now,')'));
                now+=2;
                name = getNext(s,now,',');
                tp = stoi(getNext(s,++now,','));
                int sz = s[++now]-'0';
                if (sz) sz = stoi(getNext(s,now,','));
                tre[id].fatherId = faid;
                tre[id].type = tp;
                tre[id].name = name;
                tre[faid].sonId.push_back(id);
                tre[id].res.resize(sz);
                for (int i=0;i<sz;i++){
                    name = getNext(s,++now,',');
                    char end = ((i == sz-1)?';':',');
                    tp = stoi(getNext(s,++now,end));
                    tre[id].res[i].s = name;
                    tre[id].res[i].type = tp;
                }
                if (deb){
                    cout<<"Node info: id="<<id<<",faid="<<faid<<",name="<<name;
                    cout<<",tp="<<tp<<endl;
                }
            }
            if (deb) cout<<"Tree Building Completed"<<endl;
        }
        void dfs(int u,string path){
            path = path+"/"+tre[u].name;
            if (deb) cout<<"Path for node "<<u<<" is "<<path<<endl;
            tre[u].path = path;
            pathid[Hash(path)] = u;
            for (auto x:tre[u].sonId){
                dfs(x,path);
            }
        }
        void dfs(int u,int fa,ofstream &out){
            int sz = tre[u].res.size();
            id[u] = ++gloCnt;
            out<<id[u]<<"("<<id[fa]<<"):"<<tre[u].name<<","<<tre[u].type<<","<<sz;
            for (int i=0;i<sz;i++){
                if (!i) out<<',';
                out<<tre[u].res[i].s<<","<<tre[u].res[i].type;
                if (i!=sz-1) out<<',';
            }
            out<<";\n";
            for (auto x:tre[u].sonId){
                if (!ban[x]) dfs(x,u,out);
            }
        }
        void save(){
            gloCnt = 0;
            ofstream output(dbpath);
            output<<tot<<endl;
            dfs(1,0,output);
        }
        int deldfs(int u){
            int sz = 1;
            pathid.erase(Hash(tre[u].path));
            for (auto x:tre[u].sonId){
                sz+=deldfs(x);
            }
            if (deb) cout<<"Removing path "<<tre[u].path<<endl;
            string s = "./data"+tre[u].path;
            if (tre[u].type<=1) rmdir(s.c_str());
            else if (tre[u].type == 3){
                char s2[1005] = "./data";
                int len = tre[u].path.length();
                for (int i=0;i<len;i++){
                    s2[i+6] = tre[u].path[i];
                }
                s2[len+6] = '\0';
                remove(s2);
            }
            return sz;
        }
        //以下�? HTML 填充模块
        void showTypeName(string &s){
            int node = pathid[Hash(pars[0].value)];
            s = tre[node].name;
            if (tre[node].type <= 1) s = "Folder: "+s;
            else if (tre[node].type == 2) s = "Problem: "+s;
            else s = "File: "+s;
        }
        void showName(string &s){
            s = tre[pathid[Hash(pars[0].value)]].res[0].s;
        }
        void showDes(string &s){
            s = tre[pathid[Hash(pars[0].value)]].res[1].s;
        }
        void showSol(string &s){
            s = tre[pathid[Hash(pars[0].value)]].res[2].s;
        }
        void showCreTime(string &s){
            int node = pathid[Hash(pars[0].value)];
            for (auto x:tre[node].res){
                if (x.type == 4){
                    s = "Create Time: "+x.s;
                    break;
                }
            }
        }
        void showSize(string &s){
            int node = pathid[Hash(pars[0].value)];
            for (auto x:tre[node].res){
                if (x.type == 3){
                    s = "File Size: "+x.s;
                    break;
                }
            }
        }
        void listItems(string &s){
            if (!pathid.count(Hash(pars[0].value))){
                s = "Empty";
                return;
            }
            int node = pathid[Hash(pars[0].value)];
            if (!tre[node].sonId.size()){
                s = "Empty";
                return;
            }
            if (!node){
                s = "<a href=\"/rbook?expath=/root\">root</a><br>";
                return;
            }
            s = {};
            if (tre[node].fatherId){
                s+="<a href=\"/rbook?expath="+tre[tre[node].fatherId].path+"\">..</a><br>";
            }
            for (auto x:tre[node].sonId){
                string li = "<input type=\"checkbox\" id=\""+tre[x].name+"\" name=\""+tre[x].name+"\">"+"\n<label for=\""+tre[x].name+"\">"+"<a href=\"/rbook?expath="+tre[x].path+"\"/>"+tre[x].name+"</a></label><br>\n";
                // string li = "<li><a href=\"/rbook?expath="+tre[x].path+"\"/>"+tre[x].name+"</a></li>\n";
                s+=li;
            }
        }
        void curpath(string &s){
            s = pars[0].value;
            if (!pathid.count(Hash(s))){
                s = "!Invalid";
                return;
            }
        }
        //以下为信息接受、处理与发送模�?
        void addNode(string &path,int noCre = 0){
            string node = {},fa = {};
            if (!pathid.count(Hash(path))){
                int len = path.length();
                for (int i=len-1;i>=0;i--){
                    if (path[i] == '/'){
                        for (int j=i+1;j<len;j++) node+=path[j];
                        for (int j=0;j<i;j++) fa+=path[j];
                        break;
                    }
                }
                tot++;
                Node tmp;
                tmp.fatherId = pathid[Hash(fa)];
                tmp.name = node;
                tmp.path = path;
                tmp.type = 1;
                tre.push_back(tmp);
                tre[tre[tot].fatherId].sonId.push_back(tot);
                ban.push_back(0);
                id.push_back(tot);
                pathid[Hash(path)] = tot;
                if (!noCre){
                    string s = "./data"+path;
                    mkdir(s.c_str(),NULL);
                }
            }
        }
        void removeNode(int node){
            id.clear();
            ban.clear();
            int newtot = tot-deldfs(node);
            ban[node] = 1;
            tot = newtot;
            // save();
            // build();
            // dfs(1,"");
        }
        void Remove(string &path){
            initPars(path);
            string Path = pars[0].value;
            ifstream input("tmpExchangeFile.txt");
            string s;
            while (getline(input,s)){
                int pos = 0;
                s = getNext(s,pos,'=');
                s = Path+'/'+s;
                if (deb) cout<<"Node: "<<s<<endl;
                if (pathid[Hash(s)]) removeNode(pathid[Hash(s)]);
            }
            save();
            build();
            dfs(1,"");
            char fullPath[1005] = {};
            strcpy(fullPath,absolPath);
            strcat(fullPath,"/site/rbook/redir.html");
            htmlNormalShow(fullPath);
        }
        void addCol(string &Path){
            getPars(Path);
            addNode(pars[0].value);
            save();
            OkResponse();
        }
        void addFile(string &path){
            initPars(path);
            pars[0].value = pars[0].value+'/'+ContentName;
            string Path = pars[0].value;
            addNode(Path,1);
            int node = tot;
            string t = {};
            gettime(t);
            tre[node].type = 3;
            tre[node].res.push_back({ContentName,1});
            tre[node].res.push_back({t,4});
            t = "("+to_string(fileSize)+" bytes)";
            double tmp = 1.0*fileSize/1024/1024;
            int szInt = (int)tmp,szFloat = (tmp-szInt)*100;
            t = to_string(szInt)+"."+to_string(szFloat)+" MB"+t;
            tre[node].res.push_back({t,3});
            Path = "./data"+Path;
            ifstream input("tmpExchangeFile.txt",ios::binary);
            ofstream output(Path,ios::binary);
            while (input.read(tmpData,blockLen)) output.write(tmpData,blockLen);
            output.write(tmpData,fileSize%blockLen);
            input.close();
            output.close();
            save();
            char fullPath[1005] = {};
            strcpy(fullPath,absolPath);
            strcat(fullPath,"/site/rbook/showfile.html");
            htmlNormalShow(fullPath);
        }
        void probAddPage(string path){
            initPars(path);
            char fullPath[1005] = {};
            strcpy(fullPath,absolPath);
            strcat(fullPath,"/site/rbook/prob.html");
            htmlNormalShow(fullPath);
        }
        void addProb(string &s){
            getPars(s);
            addNode(pars[0].value);
            int node = tot;
            string t = {};
            gettime(t);
            tre[node].type = 2;
            tre[node].res.push_back({tre[node].name,2});
            tre[node].res.push_back({pars[1].value,2});
            tre[node].res.push_back({pars[2].value,2});
            tre[node].res.push_back({t,4});
            save();
            OkResponse();
        }
        void analy(string &s);
        void exec(string &path){
            initPars(path);
            char fullPath[1005] = {};
            strcpy(fullPath,absolPath);
            int hashValue = Hash(pars[0].value),node = 0;
            if (pathid.count(hashValue)) node = pathid[hashValue];
            if (tre[node].type<=1) strcat(fullPath,"/site/rbook/rbook.html");
            else if (tre[node].type == 2) strcat(fullPath,"/site/rbook/showprob.html");
            else if (tre[node].type == 3) strcat(fullPath,"/site/rbook/showfile.html");
            htmlNormalShow(fullPath);
        }
    private:
        map<int,void(TopicTree::*)(string &)> funcid;
        map<long long,int> pathid;
        int tot;
        string dbpath;
        vector<Node> tre;
        vector<int> id,ban;
};
TopicTree *thisTP;
TopicTree::TopicTree(){
    thisTP = this;
    dbpath = "data/book.txt";
    pars.push_back({1,"expath","/root"});
    pars.push_back({0,"desc",""});
    pars.push_back({0,"sol",""});
    hashes[Hash("Path")] = &pars[0];
    hashes[Hash("List")] = &pars[0];
    hashes[Hash("Name")] = &pars[0];
    hashes[Hash("TypeName")] = &pars[0];
    hashes[Hash("Description")] = &pars[0];
    hashes[Hash("Solution")] = &pars[0];
    hashes[Hash("CreTime")] = &pars[0];
    hashes[Hash("Size")] = &pars[0];
    funcid[Hash("Size")] = &TopicTree::showSize;
    funcid[Hash("Path")] = &TopicTree::curpath;
    funcid[Hash("List")] = &TopicTree::listItems;
    funcid[Hash("Name")] = &TopicTree::showName;
    funcid[Hash("TypeName")] = &TopicTree::showTypeName;
    funcid[Hash("Description")] = &TopicTree::showDes;
    funcid[Hash("Solution")] = &TopicTree::showSol;
    funcid[Hash("CreTime")] = &TopicTree::showCreTime;
    funcid[Hash("Size")] = &TopicTree::showSize;
    pathid[Hash("/")] = 0;
    build();
    dfs(1,"");
    save();
}
void TopicTree::analy(string &s){
    int hashValue = Hash(s);
    if (hashes.count(hashValue) && hashes[hashValue]->isCreated){
        (thisTP->*funcid[hashValue])(s);
    }
    else s = {};
}
IMPLEMENT_CTRL_FUN(TopicTree,0,exec);
IMPLEMENT_CTRL_FUN(TopicTree,1,addCol);
IMPLEMENT_CTRL_FUN(TopicTree,2,probAddPage);
IMPLEMENT_CTRL_FUN(TopicTree,3,addProb);
IMPLEMENT_CTRL_FUN(TopicTree,4,addFile);
IMPLEMENT_CTRL_FUN(TopicTree,5,Remove);

class ImageTrans:public Controller{
    DECLARE_CTRL_FUN(0);
    DECLARE_CTRL_FUN(1);
    public:
        ImageTrans();
        void upload(string path,char* fullPath){
            ifstream input(fullPath,ios::binary);
            input.seekg(0,ios::end);
            int sz = input.tellg();
            input.seekg(0);
            sz++;
            if (path.find(".ico")<path.length()) resp.add("Content-Type:","image/x-icon;");
            else if (path.find(".jpg")<path.length()) resp.add("Content-Type:","image/jpg;");
            else if (path.find(".js")<path.length()) resp.add("Content-Type:","text/javascript;");
            else if (path.find(".css")<path.length()) resp.add("Content-Type:","text/css;");
            else if (path.find(".png")<path.length()) resp.add("Content-Type:","image/png;");
            else resp.add("Content-Type:","image/jpg;");
            resp.add("Content-Length:",to_string(sz-1));
            resp.sendHeader();
            int num = ceil(1.0*sz/blockLen),res = sz-(num-1)*blockLen;
            //缓存分隔处理
            for (int i=1;i<num;i++){
                input.read(Data,blockLen);
                resp.sendBody(Data,blockLen);
            }
            input.read(Data,res);
            resp.sendBody(Data,res);
            input.close();
        }
        void exec(string path){
            char fullPath[1005] = {},alt[1005] = "/site";
            int len = path.length();
            for (int i=0;i<len;i++) alt[i+5] = path[i];
            alt[len+5] = '\0';
            if (deb) cout<<"Image path is "<<alt<<endl;
            strcpy(fullPath,absolPath);
            strcat(fullPath,alt);
            upload(path,fullPath);
        }
        void download(string path){
            for (auto x:pars) x.isCreated = 0;
            if (!getPars(path)){
                cout<<"Parameter Invalid"<<endl;
                tmp_404.f0(path);
                return;
            }
            char fullPath[1005] = {},alt[105] = "/data";
            int len = pars[0].value.length();
            for (int i=5;i<len+5;i++){
                alt[i] = pars[0].value[i-5];
            }
            alt[len+5] = '\0';
            if (deb) cout<<"Download file path is "<<alt<<endl;
            strcpy(fullPath,absolPath);
            strcat(fullPath,alt);
            upload(path,fullPath);
        }
};
ImageTrans::ImageTrans(){
    pars.push_back({0,"path",""});
}
IMPLEMENT_CTRL_FUN(ImageTrans,0,exec);
IMPLEMENT_CTRL_FUN(ImageTrans,1,download);

void Router::setupRouting() {
    getcwd(absolPath,1000);
    CREATE_INSTANCE(MainPage,MPIns);
    CREATE_INSTANCE(_404,_404Ins);
    CREATE_INSTANCE(Prime,PrimeIns);
    CREATE_INSTANCE(ImageTrans,ITIns);
    CREATE_INSTANCE(TopicTree,TPIns);
	REGISTER_CTRL_FUN("GET","/",MainPage,MPIns,0)
    REGISTER_CTRL_FUN("GET","/404",_404,_404Ins,0)
    REGISTER_CTRL_FUN("GET","/prime",Prime,PrimeIns,0)
    REGISTER_CTRL_FUN("GET","/get/prime",Prime,PrimeIns,1)
    REGISTER_CTRL_FUN("GET","/favicon.ico",ImageTrans,ITIns,0)
    REGISTER_CTRL_FUN("GET","/images/avatar.jpg",ImageTrans,ITIns,0)
    REGISTER_CTRL_FUN("GET","/images/DragonDance.gif",ImageTrans,ITIns,0)
    REGISTER_CTRL_FUN("GET","/prime/main.js",ImageTrans,ITIns,0)
    REGISTER_CTRL_FUN("GET","/rbook/prob.js",ImageTrans,ITIns,0)
    REGISTER_CTRL_FUN("GET","/rbook/showprob.js",ImageTrans,ITIns,0)
    REGISTER_CTRL_FUN("GET","/download",ImageTrans,ITIns,1)
    REGISTER_CTRL_FUN("GET","/styles/common.css",ImageTrans,ITIns,0)
    REGISTER_CTRL_FUN("GET","/styles/MainPage.css",ImageTrans,ITIns,0)
    REGISTER_CTRL_FUN("GET","/styles/rbook.css",ImageTrans,ITIns,0)
    REGISTER_CTRL_FUN("GET","/images/background.png",ImageTrans,ITIns,0)
    REGISTER_CTRL_FUN("GET","/rbook/main.js",ImageTrans,ITIns,0)
    REGISTER_CTRL_FUN("GET","/rbook/redir.js",ImageTrans,ITIns,0)
    REGISTER_CTRL_FUN("GET","/rbook",TopicTree,TPIns,0)
    REGISTER_CTRL_FUN("GET","/get/rbook",TopicTree,TPIns,1)
    REGISTER_CTRL_FUN("GET","/rbook/addproblem",TopicTree,TPIns,2)
    REGISTER_CTRL_FUN("GET","/get/rbook/prob",TopicTree,TPIns,3)
    REGISTER_CTRL_FUN("POST","/rbook/upload",TopicTree,TPIns,4)
    REGISTER_CTRL_FUN("POST","/rbook/delete",TopicTree,TPIns,5)
}