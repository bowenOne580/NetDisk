Web 简易应用

提供 Windows 和 Linux 两端 cpp 代码，可以自行编译，一定要将编译得到的文件放在与 site 和 data 同级的文件夹，否则会出现路径错误。

提供了在 Ubuntu 下和 Windows11 msvc 下编译的可执行文件，不保证其他环境的可用性。

重要！！！data 文件夹下请自行创建 root 文件夹，这是网盘存储的根路径，网盘中的所有文件都会存储在此路径下。

使用方式
```
# 1. 前台运行（默认）
./server

# 2. 后台运行（守护进程）
./server -d

# 3. 后台运行 + 自定义端口
./server -d port=8080

# 4. 自定义 PID 文件路径
./server -d --pid /var/run/netdisk.pid

# 5. 停止服务器
./server --stop

# 6. 重启服务器
./server --restart

# 7. 查看帮助
./server --help
```
