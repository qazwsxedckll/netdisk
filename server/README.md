# ftpserver

----------

服务器编译

/server

make

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

----------

测试数据库

mysql -u<username> -p<password> <dbname> < /netdisk.sql
                                      
