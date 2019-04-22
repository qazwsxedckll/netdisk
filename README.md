# netdisk

----------

服务器编译

/server

make

如果找不到<openssl/rsa.h>

sudo apt install libssl-dev

如果找不到<mysql/mysql.h>

sudo apt install  libmysqlclient-dev

生成服务器rsa
openssl genrsa -out server_rsa.key 3072
openssl rsa -in server_rsa.key -pubout -out server_rsa_pub.key
将服务器公钥放入客户端
cp server_rsa_pub.key ../client/

执行

1. 修改server.conf里ip地址为本机地址
2. 修改sql.conf里user和password为自己数据库的
3. 执行./server ../conf/server.conf

---------

客户端编译

/client

make

执行

./client <服务器ip> <服务器port> 
