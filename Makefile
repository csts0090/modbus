CC = arm-linux-gnueabihf-gcc
serial_read3:serial_read3.c 
	$(CC) -o serial_read3 serial_read3.c   modbus-tcp.c modbus.c modbus-rtu.c modbus-data.c  -std=gnu99 `mysql_config --cflags --libs` -lpthread
serial_read1:serial_read1.c 
	$(CC) -o serial_read1 serial_read1.c  modbus-tcp.c modbus.c modbus-rtu.c modbus-data.c  `mysql_config --cflags --libs` 

client_serial:client_serial.c 
	$(CC) -o client_serial client_serial.c  modbus-tcp.c modbus.c modbus-rtu.c modbus-data.c
 
client_serial2:client_serial2.c 
	$(CC) -o client_serial2 client_serial2.c  modbus-tcp.c modbus.c modbus-rtu.c modbus-data.c
 
client_tcp:client_tcp.c 
	$(CC) -o client_tcp client_tcp.c  modbus-tcp.c modbus.c modbus-rtu.c modbus-data.c 

client_tcp2:client_tcp2.c 
	$(CC) -o client_tcp2 client_tcp2.c  modbus-tcp.c modbus.c modbus-rtu.c modbus-data.c 
	
client_tcp_remote:client_tcp_remote.c 
	$(CC) -o client_tcp_remote client_tcp_remote.c  modbus-tcp.c modbus.c modbus-rtu.c modbus-data.c  

serial_read2:serial_read2.c 
	$(CC) -o serial_read2 serial_read2.c  -std=c99 -lpthread 
serial_read:serial_read.c 
	$(CC) -o serial_read serial_read.c  modbus-tcp.c modbus.c modbus-rtu.c modbus-data.c  `mysql_config --cflags --libs` 
