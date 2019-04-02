# ftpserver

----------

服务器编译

/bin

gcc *.c -I /usr/include/mysql/ -lmysqlclient -lpthread

执行

1. 修改server.conf里ip地址为本机地址
2. 修改sql.conf里user和password为自己数据库的
3. 执行./a.out ../conf/server.conf

---------

客户端编译

/client

gcc *.c

执行

./a.out [本地ip] [server.conf里的port] 

----------

测试数据库

mysql -u<username> -p<password> <dbname> < /netdisk.sql
