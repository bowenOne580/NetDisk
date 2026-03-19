#include"func.h"
#include"router.h"
#include"responder.h"
#include"Config.h"
#include<fstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <cstring>
#include <cstdlib>
#include <cerrno>
using namespace std;
#pragma comment(lib,"ws2_32.lib")

// 守护进程相关
static bool daemon_mode = false;
static int pid_file_fd = -1;
static string pid_file_path;

void remove_pid_file();

void handle_exit_signal(int) {
	remove_pid_file();
	_exit(0);
}

void daemonize() {
    pid_t pid = fork();
    if (pid < 0) {
        cerr << "Fork failed!" << endl;
        exit(1);
    }
    if (pid > 0) {
        cout << "Server started in background, PID: " << pid << endl;
        exit(0);
    }
    
    // 创建新会话
    if (setsid() < 0) {
        cerr << "setsid failed!" << endl;
        exit(1);
    }
    
    // 忽略 SIGCHLD 信号
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    
    // 关闭标准输入输出
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    
    // 重定向到 /dev/null
    open("/dev/null", O_RDONLY);
    open("/dev/null", O_WRONLY);
    open("/dev/null", O_WRONLY);
}

bool write_pid_file(const string& path) {
	pid_file_path = path;
	pid_file_fd = open(path.c_str(), O_CREAT | O_WRONLY, 0644);
    if (pid_file_fd < 0) {
        cerr << "Cannot create PID file!" << endl;
        return false;
    }
	if (lockf(pid_file_fd, F_TLOCK, 0) < 0) {
		cerr << "Another server instance may already be running." << endl;
		close(pid_file_fd);
		pid_file_fd = -1;
		return false;
	}
	ftruncate(pid_file_fd, 0);
    string pid_str = to_string(getpid());
    write(pid_file_fd, pid_str.c_str(), pid_str.length());
    return true;
}

void remove_pid_file() {
    if (pid_file_fd >= 0) {
        close(pid_file_fd);
		pid_file_fd = -1;
	}
	if (!pid_file_path.empty()) {
		remove(pid_file_path.c_str());
    }
}

int sockServer,sockClient;
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
	sockServer = socket(AF_INET, SOCK_STREAM, 0);
	if (sockServer == -1)
	{
		cout<<"Fail to create server thread!"<<endl;
		return -1;
	}
	int opt = 1;
	setsockopt(sockServer, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	#ifdef SO_REUSEPORT
	setsockopt(sockServer, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
	#endif
	cout<<"Init completed."<<endl;
    return 0;
}
int bindPort(int port){
    struct sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
	// addr.sin_addr.s_addr = inet_addr(addrs);
	if (bind(sockServer, (sockaddr*)&addr, sizeof(addr)) != 0)
	{
		cout<<"Port bind failed!"<<endl;
		return -1;
	}
	if (listen(sockServer, 1) != 0)
	{
		cout<<"Port listen failed!"<<endl;
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
				if (i + 7 < len && szData[i] == 'f' && szData[i+7] == 'e'){
					i+=10;
					ContentName = {};
					while (i < len && szData[i]!='\"'){
						ContentName+=szData[i];
						i++;
					}
					cout<<"Get content named "<<ContentName<<endl;
				}
				if (i + 2 < len && szData[i] == 'p' && szData[i+1] == 'e' && szData[i+2] == ':'){
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
				for (int i=len-10;i>=0;i--){
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
	if (path.find("%20") != string::npos){ //处理空格
		string s = {};
		int pLen = path.length();
		for (int i=0;i<pLen;i++){
			if (i + 2 < pLen && path[i] == '%' && path[i+1] == '2' && path[i+2] == '0'){
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
		for (int i=0;i<len;i++){
			if (i + 2 < len && szData[i] == '-' && szData[i+1] == 'L' && szData[i+2] == 'e'){
				i+=9;
				gloLen = 0;
				while (i < len && szData[i]<='9' && szData[i]>='0'){
					gloLen = gloLen*10+szData[i]-'0';
					i++;
				}
				fileSize = gloLen;
				break;
			}
		}
		cout<<"POST Content Length: "<<gloLen<<endl; //获取Content-Length
		int pos = Data.find("delete");
		if (pos>=0 && pos<len){
			for (int i=len-gloLen;i<len;i++) output<<szData[i];
			gloLen = 0;
			flag = 0;
			output.close();
			cout<<"Receive POST Data Successfully"<<endl;
			return;
		}
		pos = Data.find("------We");
		if (pos>=0 && pos<len){
			for (int i=pos;i<len;i++) szData[i-pos] = Data[i];
			resolve(szData,len-pos,method,path);
		}
	}
}
int main(int argn,char **argv)
{
	ios::sync_with_stdio(false);
	cin.tie(0);
	
	string pid_file = "server.pid";
	bool stop_server = false;
	bool restart_server = false;
	
	for (int i = 1; i < argn; i++) {
		if (argv[i][0] == 'p' && argv[i][1] == 'o') {
			// port=XXXX
			int P = 0, len = strlen(argv[i]);
			for (int j = 5; j < len; j++) P = P * 10 + argv[i][j] - '0';
			cout << "Rebind port to " << P << endl;
			port = P;
		}
		else if (strcmp(argv[i], "--daemon") == 0 || strcmp(argv[i], "-d") == 0) {
			daemon_mode = true;
		}
		else if (strcmp(argv[i], "--pid") == 0 && i + 1 < argn) {
			pid_file = argv[++i];
		}
		else if (strcmp(argv[i], "--stop") == 0) {
			stop_server = true;
		}
		else if (strcmp(argv[i], "--restart") == 0) {
			restart_server = true;
		}
		else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
			cout << "Usage: ./server [options]" << endl;
			cout << "Options:" << endl;
			cout << "  port=XXXX    Set server port (default: 8000)" << endl;
			cout << "  -d, --daemon Run as daemon (background process)" << endl;
			cout << "  --pid FILE   PID file path (default: server.pid)" << endl;
			cout << "  --stop       Stop running server" << endl;
			cout << "  --restart    Restart server" << endl;
			cout << "  -h, --help   Show this help message" << endl;
			return 0;
		}
	}
	
	// 停止服务器
	if (stop_server) {
		ifstream ifs(pid_file);
		if (ifs.good()) {
			pid_t pid;
			ifs >> pid;
			ifs.close();
			cout << "Stopping server (PID: " << pid << ")..." << endl;
			if (kill(pid, SIGTERM) == 0) {
				remove(pid_file.c_str());
				cout << "Server stopped." << endl;
			} else {
				cout << "Failed to stop server, errno=" << errno << endl;
			}
		} else {
			cout << "PID file not found. Server may not be running." << endl;
		}
		return 0;
	}
	
	// 重启服务器
	if (restart_server) {
		ifstream ifs(pid_file);
		if (ifs.good()) {
			pid_t pid;
			ifs >> pid;
			ifs.close();
			cout << "Restarting server (PID: " << pid << ")..." << endl;
			kill(pid, SIGTERM);
			sleep(1);
			remove(pid_file.c_str());
		}
		daemon_mode = true;
	}
	
	// 守护进程模式
	if (daemon_mode) {
		daemonize();
		if (!write_pid_file(pid_file)) {
			return 1;
		}
		atexit(remove_pid_file);
		signal(SIGTERM, handle_exit_signal);
		signal(SIGINT, handle_exit_signal);
	}
	
    if (init() == -1) return 0;
    if (bindPort(port) == -1) return 0;
	buildIndexingTree();
	thisRouter->setupRouting();
	cout<<"Route setup completed."<<endl;
	while (true)
	{
		struct sockaddr_in clientAddr = {};
		socklen_t nAddrLen = sizeof(struct sockaddr_in);
		sockClient = accept(sockServer, (sockaddr*)&clientAddr, &nAddrLen);
		if (0 > sockClient)
		{
			cout<<"Fail to accept client connection!"<<endl;
			continue;
		}
		while (true)
		{
			int ret = recv(sockClient, szData, blockLen, MSG_NOSIGNAL);
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
				resp = Response();
				if (method == "GET") thisRouter->handle(method,path);
				else if (!(flag+gloLen)) thisRouter->handle("POST",POSTPath);
				ret = resp.ok();
				if (ret == 0){
					cout<<"Fail to send response"<<endl;
					break;
				}
			}
			else if (ret<0) // 发生错误
			{
				cout<<"Fail to receive client data!"<<endl;
				break;
			}
			else break;
		}
		close(sockClient);
	}
	close(sockServer);
	return 0;
}