#include"func.h"
#include"router.h"
#include"responder.h"
#include"Config.h"
#include<fstream>
using namespace std;
#pragma comment(lib,"ws2_32.lib")

WSADATA wsaData;
SOCKET sockServer,sockClient;
string response;
Router *thisRouter;
Response resp;
ofstream output;
char szData[2100000];
string Data,method,path;
int flag = 0,gloLen = 0,nowLen,fileSize;
string POSTPath,ContentName;

Router::Router(){
    thisRouter = this;
}
Router router;

int init(){
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		cout<<"Winsock init failed!"<<endl;
		return -1;
	}
	sockServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockServer == INVALID_SOCKET)
	{
		cout<<"Fail to create server thread!"<<endl;
		WSACleanup();
		return -1;
	}
	cout<<"Init completed."<<endl;
    return 0;
}
int bindPort(int port){
    SOCKADDR_IN addr = {};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
    // addr.sin_addr.S_un.S_addr = INADDR_ANY;
	addr.sin_addr.S_un.S_addr = inet_addr(addrs);
	if (bind(sockServer, (sockaddr*)&addr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		cout<<"Port bind failed!"<<endl;
		closesocket(sockServer);
		WSACleanup();
		return -1;
	}
	if (listen(sockServer, 100) == SOCKET_ERROR)
	{
		cout<<"Port listen failed!"<<endl;
		closesocket(sockServer);
		WSACleanup();
		return -1;
	}
    cout<<"Server running on "<<addrs<<":"<<port<<endl;
    return 0;
}
void resolve(char *szData,int len,string &method,string &path){
	if (flag){
		ofstream tmpOut("tmp.txt");
		tmpOut.write(szData,len);
		tmpOut.close();
		if (method!="POST") method = "null";
		if (flag == 1){
			int fl = 0;
			for (int i=0;i<len && gloLen;i++){
				if (szData[i] == 'f' && szData[i+7] == 'e'){
					i+=10;
					ContentName = {};
					while (szData[i]!='\"'){
						ContentName+=szData[i];
						i++;
					}
					cout<<"Get content named "<<ContentName<<endl;
				}
				if (szData[i] == 'p' && szData[i+1] == 'e' && szData[i+2] == ':'){
					while (!(szData[i] == 13)) i++;
					i+=3;
					gloLen-=i+1;
					fileSize-=(i+1);
					flag = 2;
					i++;
					for (int j=len-10;j>=i && gloLen;j--){
						if (checkEndFile(szData,j+2,"------We")){
							flag = 1,gloLen = 0;
							for (int p=i;p<j;p++) output<<szData[p];
							fileSize-=(len-j);
							cout<<"Real File Size: "<<fileSize<<" bytes"<<endl;
							break;
						}
					}
					if (flag == 2){
						for (int j=i;j<len;j++){
							output<<szData[j];
							gloLen--;
						}
					}
					break;
				}
			}
		}
		else{
			gloLen-=len;
			if (!gloLen){
				for (int i=len-1;i>=0;i--){
					if (checkEndFile(szData,i+2,"------We")){
						flag = 1;
						output.write(szData,i);
						fileSize-=(len-i);
						cout<<"Real File Size: "<<fileSize<<" bytes"<<endl;
						break;
					}
				}
			}
			else output.write(szData,len);
		}
		if (!gloLen){
			flag = 0;
			output.close();
			cout<<"Receive POST Data Successfully"<<endl;
		}
		// else{
		// 	system("cls");
		// 	cout<<"Remaining "<<gloLen<<" bytes"<<endl;
		// }
		return;
	}
	for (int i=0;i<len;i++){
		if (szData[i] == ' ') break;
		method+=szData[i];
	}
	for (int i=0;i<len;i++){
		if (szData[i] == '/'){
			for (int j=i;j<len;j++){
				if (szData[j] == ' ') break;
				path+=szData[j];
			}
			break;
		}
	}
	if (path.find("%20")){ //处理空格
		string s = {};
		int pLen = path.length();
		for (int i=0;i<pLen;i++){
			if (path[i] == '%' && path[i+1] == '2' && path[i+2] == '0'){
				s+=' ';
				i+=2;
			}
			else s+=path[i];
		}
		path = s;
	}
	cout<<"Result: \n"<<"Method is "<<method<<"\nPath is "<<path<<endl<<endl;
	if (method == "GET") flag = 0;
	if (method == "POST"){
		flag = 1;
		POSTPath = path;
		output.open("tmpExchangeFile.txt",ios::binary);
		int pos = Data.find("text/plain");
		for (int i=0;i<len;i++){
			if (szData[i] == '-' && szData[i+1] == 'L' && szData[i+2] == 'e'){
				i+=9;
				gloLen = 0;
				while (szData[i]<='9' && szData[i]>='0'){
					gloLen = gloLen*10+szData[i]-'0';
					i++;
				}
				fileSize = gloLen;
				break;
			}
		}
		cout<<"POST Content Length: "<<gloLen<<endl; //获取Content-Length
		if (pos>=0 && pos<len){
			for (int i=len-gloLen;i<len;i++) output<<szData[i];
			gloLen = 0;
			flag = 0;
			output.close();
			cout<<"Receive POST Data Successfully"<<endl;
		}
	}
}
int main()
{
	ios::sync_with_stdio(false);
    if (init() == -1) return 0;
    if (bindPort(port) == -1) return 0;
	buildIndexingTree();
	thisRouter->setupRouting();
	cout<<"Route setup completed."<<endl;
	while (true)
	{
		SOCKADDR_IN clientAddr = {};
		int nAddrLen = sizeof(SOCKADDR_IN);
		sockClient = accept(sockServer, (sockaddr*)&clientAddr, &nAddrLen);

		if (INVALID_SOCKET == sockClient)
		{
			cout<<"Fail to accept client connection!"<<endl;
			continue;
		}
		while (true)
		{
			int ret = recv(sockClient, szData, blockLen, 0);
			if (ret > 0)
			{
                // cout<<"Client returned data:\n"<<szData<<endl;
				if (!flag){
					Data = {};
					for (int i=0;i<ret;i++) Data+=szData[i];
				}
				method = "",path = "";
				resolve(szData,ret,method,path);
				response = {};
				if (method == "GET") thisRouter->handle(method,path);
				else if (!(flag+gloLen)) thisRouter->handle("POST",POSTPath);
				ret = resp.ok();
				if (ret == SOCKET_ERROR){
					cout<<"Fail to send response"<<endl;
				}
			}
			else if (ret<0) // 发生错误
			{
				cout<<"Fail to receive client data!"<<endl;
				break;
			}
			else break;
		}
		closesocket(sockClient);
	}
	closesocket(sockServer);
	WSACleanup();
	return 0;
}