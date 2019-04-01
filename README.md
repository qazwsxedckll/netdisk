# ftpserver
----------
服务器编译
/bin
gcc *.c -I /usr/include/mysql/ -lmysqlclient -lpthread
---------
客户端编译
/client
gcc *.c
-------
恢复数据库
mysql -u<username> -p<password> <dbname> < /netdisk.sql
