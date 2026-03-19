## Web 简易应用

提供 Windows 和 Linux 两端 cpp 代码，可以自行编译，一定要将编译得到的文件放在与 site 和 data 同级的文件夹，否则会出现路径错误。

提供了在 Ubuntu 下和 Windows11 msvc 下编译的可执行文件，不保证其他环境的可用性。

**重要！！！data 文件夹下请自行创建 root 文件夹，这是网盘存储的根路径，网盘中的所有文件都会存储在此路径下。**

## 使用方式
```
# 1. 前台运行（默认）
./server

# 2. 后台运行（守护进程）
./server -d --pid server.pid

# 3. 后台运行 + 自定义端口
./server -d port=8080 --pid server.pid

# 4. 自定义 PID 文件路径
./server -d --pid /var/run/netdisk.pid

# 5. 停止服务器
./server --stop --pid server.pid

# 6. 重启服务器
./server --restart port=2055 --pid server.pid

# 6. 重启服务器
./server --restart

# 7. 查看帮助
./server --help
```

## 如何构建

在 Windows 下，直接编译所有文件，生成可执行文件。

在 Linux 下，你可以通过本项目提供的 Makefile 来自动化构建过程：

1. make build
   编译 /source 目录下的源代码（你需要将代码保存在该目录下），在当前目录生成可执行文件 server
2. make run
   编译并运行 server
3. make run PORT=xxxx
   指定端口运行，其中 xxxx 为你输入的端口号，程序会自动绑定该端口
