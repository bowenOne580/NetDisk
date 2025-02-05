#include<vector>
#include<string>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
using namespace std;

class Response{
    public:
        Response();
        void sendHeader();
        void sendBody(string &,int);
        void sendBody(char *,int);
        bool ok();
        void add(string s,string t);
        void set(string s,string t);
        bool count(const string &s);
    private:
        vector<string> name,value;
        bool flag;
};