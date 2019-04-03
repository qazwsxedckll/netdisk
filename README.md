# ftpserver

----------

服务器编译

/bin

make

执行

1. 修改server.conf里ip地址为本机地址
2. 修改sql.conf里user和password为自己数据库的
3. 执行./a.out ../conf/server.conf

---------

客户端编译

/client

gcc *.c -lpthread

执行

./a.out [服务器ip] [服务器port] 

----------

测试数据库

mysql -u<username> -p<password> <dbname> < /netdisk.sql
                                      
测试用户

1. username: Evilolipop, password: 123
2. username: Luke, password: luke
