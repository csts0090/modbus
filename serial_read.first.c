#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <mysql/mysql.h>
 
#include "modbus.h"
 
#define MODBUS_SERIAL_DEV           "/dev/ttyM0"
#define MODBUS_SERIAL_BAUDRATE      9600    /* 9600, 38400, 115200, ... */
#define MODBUS_SERIAL_PARITY        'N'     /* 'N', 'E', or 'O' */
#define MODBUS_SERIAL_DATABITS      8       /* 5, 6, 7, or 8 */
#define MODBUS_SERIAL_STOPBITS      1       /* 1 or 2 */

#define MODBUS_DEVICE_ID            10
#define MODBUS_TIMEOUT_SEC          3
#define MODBUS_TIMEOUT_USEC         0
#define MODBUS_DEBUG                ON

/*---------------database_start----------------------*/
#define MyHOST "127.0.0.1"
#define MyNAME "root"
#define MyDB "SystemControl"
#define Passwd "gh37143715"
#define QSIZE 256

MYSQL *dp;
MYSQL_RES *qp;
MYSQL_ROW row;
char query[QSIZE] = "UPDATE DeviceDataField SET data = '12345678' where address = 5409 AND station_no = 10";

int opendb();
/*----------------database_end---------------------------*/

int sql_write(char *station_no, char *address, char *data);

 
int main(int argc, char *argv[])
{
    modbus_t            *ctx;
    struct timeval      timeout;
    int                 ret, i, rc, ii;
    int nb_pointers;
   // unit16_t *tab_rp_registers;
    uint16_t        regs[MODBUS_MAX_READ_REGISTERS] = {0};
    char            regs2[MODBUS_MAX_READ_REGISTERS]={0};
    FILE *fp = NULL;

    int err, rows, cols;
 

   if(argc != 4){
        printf("INsufficient argument");
        return -1;
    }

    ctx = modbus_new_rtu(MODBUS_SERIAL_DEV,
            MODBUS_SERIAL_BAUDRATE,
            MODBUS_SERIAL_PARITY,
            MODBUS_SERIAL_DATABITS,
            MODBUS_SERIAL_STOPBITS);
 
    if (ctx == NULL) {
        fprintf(stderr, "Unable to create the libmodbus context\n");
        exit(-1);
    }
 
    
    i = modbus_rtu_get_serial_mode(ctx);
    if( i == MODBUS_RTU_RS232)
    {
	printf("Serial mode = RS232\n");
	ret = modbus_rtu_set_serial_mode(ctx, MODBUS_RTU_RS232);
	if(ret < 0)
	    fprintf(stderr, "modbus_rtu_set_serial_mode() error: %s\n", strerror(errno));
    }
    else if(i == MODBUS_RTU_RS485)
    {
	printf("Serial mode = RS485\n");
	ret = modbus_rtu_set_serial_mode(ctx, MODBUS_RTU_RS485);
	if(ret < 0)
	    fprintf(stderr, "modbus_rtu_set_serial_mode() error: %s\n", strerror(errno));
    }
   /* else
    {
	printf("Serial mode = RS485\n");
	ret = modbus_rtu_set_serial_mode(ctx, MODBUS_RTU_RS485);
	if(ret < 0)
	    fprintf(stderr, "modbus_rtu_set_serial_mode() error: %s\n", strerror(errno));
    }
*/
    /* set slave device ID */
    modbus_set_slave(ctx, strtol(argv[1], NULL, 10));
 
    /* Debug mode */
    modbus_set_debug(ctx, MODBUS_DEBUG);

    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "Connection failed: %s\n",
                modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }


    /* Allocate and initialize the memory to store the registers */
    /*  nb_points = (UT_REGISTERS_NB > UT_INPUT_REGISTERS_NB) ?
        UT_REGISTERS_NB : UT_INPUT_REGISTERS_NB;
    tab_rp_registers = (uint16_t *) malloc(nb_points * sizeof(uint16_t));
    memset(tab_rp_registers, 0, nb_points * sizeof(uint16_t));
    */

     /* write data in test.txt */
    fp = fopen("test.txt", "w");
    if(fp == NULL)
    {
        printf("fail to open file!\n");
        return -1;
    }

    err = opendb();
    if(err)
    {
	/*Database is not open*/
	return err;
    }
 	




    rc = modbus_read_registers(ctx, strtol(argv[2], NULL, 16), strtol(argv[3], NULL, 10), regs);
    if (rc < 0) {
        fprintf(stderr, "%s\n", modbus_strerror(errno));
    }
    else {
            printf("HOLDING REGISTERS:\n");
            for (ii=0; ii < rc; ii++) {
                sprintf(regs2, "%d", regs[ii]);
                fputs(regs2, fp);
                fputs("\n", fp);
                printf("[%d]=%d\n", ii, regs[ii]);
		sql_write(argv[1], argv[2], regs2);
		
            }
		if( mysql_query(dp, query))
		{
			fprintf(stderr, "%s\n", mysql_error(dp));
		}

		qp = mysql_store_result(dp);
		mysql_free_result(qp);
	
		mysql_close(dp);


    }

    fclose(fp);

    /* Close the connection */
    modbus_close(ctx);

    modbus_free(ctx);
 
    return 0;
}

int opendb()
{
        dp = mysql_init(NULL);
        if(dp == NULL)
        {
                fprintf(stderr,"mysql_init() faild\n");
                return 2;
        }

        mysql_real_connect(dp, MyHOST, MyNAME, Passwd, MyDB, 0, NULL, 0);
        if(dp == NULL)
        {
                fprintf(stderr,"mysql_real_connect() faild\n");
                return 3;
        }

        if(mysql_select_db(dp, MyDB) != 0)
        {
                fprintf(stderr, "mysql_select_db() failed\n");
                mysql_close(dp);
                return 4;
        }
        return 0;
}

int sql_write(char *station_no, char *address, char *data)
{
	int q_length = strlen(query);

	/* copy station_no to query[] */

	int sta_no_size = strlen(station_no);
	char sta_no[3];
	strcpy(sta_no, station_no);
	memmove( query+(q_length-2), sta_no, sta_no_size);
	
	/* copy address to query[] */

	int addre_size = strlen(address);
	char addre[addre_size];
	strcpy(addre, address);
	if(addre_size > 4)
	{
		int distance = addre_size - 4;	
		memmove(query + (q_length - 19 + distance),query+(q_length - 19), 19);
		memmove(query + (q_length - 20 - addre_size) , addre, addre_size);
	}
	else
	{
		memmove(query + (q_length - 20 - addre_size) , addre, addre_size);
	}

	
	/**/
	char empty[9];
	strcpy(empty, "        ");
	int emp_size = strlen(empty);	
	int data_size = strlen(data);
	char data_tmp[data_size];
	strcpy(data_tmp, data);
	if(data_size < 8)
	{
		int dis_emp = emp_size - data_size;

		//memmove(query+33+data_size+4 ,query+(q_length- 20 - addre_size - 16 ), 36+addre_size);

		memmove(query+35, empty, dis_emp);
		memmove(query+35+dis_emp, data_tmp, data_size);
	}
	else if(data_size == 8)
	{
		memmove(query+35, data_tmp, data_size);
	
	}
	printf("%s\n",query);	
	
	
}

/*int SIZE(char *p)
{
	int num = 0;
	while(*p != '\0')
	{
		num++;
		p++;
	}
	return num;
}
*/
