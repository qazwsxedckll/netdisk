# netdisk

----------

服务器编译

/server

1. make

如果找不到<openssl/rsa.h>

`sudo apt install libssl-dev`

如果找不到<mysql/mysql.h>

`sudo apt install  libmysqlclient-dev`

2. 生成服务器rsa

`openssl genrsa -out server_rsa.key 3072`

`openssl rsa -in server_rsa.key -pubout -out server_rsa_pub.key`

将服务器公钥放入客户端

`cp server_rsa_pub.key ../client/`

3. 执行

   1. 修改server.conf里ip地址为本机地址(默认为127.0.0.1 8888)
   2. 修改sql.conf里user和password为自己数据库的
   3. `./server ../conf/server.conf`

---------

客户端编译

/client

1. make

2. `./client <服务器ip> <服务器port>` (默认 `./client 127.0.0.1 8888`)

----------------

测试数据库

登录mysql

`create database netdisk;`

`mysql -u <username> -p netdisk < server/netdisk.sql`
