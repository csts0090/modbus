#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
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
#define QSIZE 256

MYSQL *dp;
MYSQL_RES *qp;
MYSQL_ROW row;
char query[QSIZE] = "UPDATE process_res SET data = '12345678' where address = 5409 AND station_no = 10";
char query2[QSIZE]= "INSERT INTO process_res (id,station_no,address,p_id,data) VALUES( 00002,10,1521,65536,'00000000')";

int opendb();
/*----------------database_end---------------------------*/

int sql_write(char *station_no, char *address, char *data);

struct f_ile
{
	char sch_id[4];
	char sta_no[3];
	char addr[5];
        char t_start[20];
	char t_stop[20];
	char period[2];
	
};
 
int main(int argc, char *argv[])
{
   if(argc != 1){
        printf("INsufficient argument");
        return -1;
    }
    //檔案處理
    int fileLength = 0;
	FILE *fptr;
	int i = 0;
	char *test;
	if(!(fptr = fopen(argv[1], "r")))
		return -1;
	//移動指標到檔案的結尾
	if(fseek(fptr, 0, SEEK_END))
		return -1;
	//取得檔案的大小
	fileLength = ftell(fptr);
	//移動指標到檔案的起始
	rewind(fptr);
	char s[fileLength];
	int num=0;
	while(fgets(s, fileLength, fptr) != NULL)
	{
		num++;
	}
	rewind(fptr);
	struct f_ile sch[num-1];
	int k=0;
	while(fgets(s, fileLength, fptr) != NULL)
	{
		if(i == 0){
			for(int j=0; j< num; j++){
				strcpy(sch[j].sch_id, s);
				//printf("%s\n",sch[i].sch_id);
			}
		}
		else{  
			test = strtok(s, " ");
			for(int j=0; test != NULL; j++)
			{
				if(j == 0)strcpy(sch[k].sta_no,test); 
				if(j == 1)strcpy(sch[k].addr, test); 
				if(j == 2)strcpy(sch[k].t_start, test); 
				if(j == 3){
				strncat(sch[k].t_start," ", 1); 	
				strncat(sch[k].t_start,test,strlen(test)); 
				}
				if(j == 4)strcpy(sch[k].t_stop, test); 
				if(j == 5){
				strncat(sch[k].t_stop, " ", 1); 
				strncat(sch[k].t_stop, test,strlen(test));
				} 
				if(j == 6)strcpy(sch[k].period, test); 
				
				//printf("%s\n", test);
				test = strtok(NULL, " ");
				
			}
			k++;
		     }
		i++;
	}
			//for(int k=0;k<=num-2;k++)
			//printf("%s %s %s %s %s %s\n",sch[k].sch_id,sch[k].sta_no, sch[k].addr,sch[k].t_start,sch[k].t_stop,sch[k].period);
		pthread_t threads[num-1];
		for(int k=0;k<=num-2;k++)
			pthread_create(&threads[k], NULL, sch_rule, (void*)&sch[k]);
		void *result;
		for(int k=0;k<=num-2;k++)
			pthread_join(thread[k], &result);
		
		return 0;
		

//檔案處理結束
}
pthread_mutex_t beers_lock = PTHREAD_MUTEX_INITIALIZER;
void* sch_rule(void* parm)
{
   //pthread_mutex_lock(&beers_lock);
    struct f_ile* temp = (struct f_ile*)parm;
    printf("%s %s %s %s %s %s\n",(*temp).sch_id,temp->sta_no, temp->addr,temp->t_start,temp->t_stop,temp->period);
   //pthread_mutex_unlock(&beers_lock);
   	
 /*   modbus_t            *ctx;
    struct timeval      timeout;
    int                 ret, i, rc, ii;
    int nb_pointers;
   // unit16_t *tab_rp_registers;
    uint16_t        regs[MODBUS_MAX_READ_REGISTERS] = {0};
    char            regs2[MODBUS_MAX_READ_REGISTERS]={0};
    FILE *fp = NULL;

    int err, rows, cols;
 


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
    else
    {
	printf("Serial mode = RS485\n");
	ret = modbus_rtu_set_serial_mode(ctx, MODBUS_RTU_RS485);
	if(ret < 0)
	    fprintf(stderr, "modbus_rtu_set_serial_mode() error: %s\n", strerror(errno));
    }

    // set slave device ID 
    modbus_set_slave(ctx, strtol(temp.sta_no, NULL, 10));
 
    // Debug mode 
    modbus_set_debug(ctx, MODBUS_DEBUG);

    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "Connection failed: %s\n",
                modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }


    // Allocate and initialize the memory to store the registers 
    //  nb_points = (UT_REGISTERS_NB > UT_INPUT_REGISTERS_NB) ?
     //   UT_REGISTERS_NB : UT_INPUT_REGISTERS_NB;
    //tab_rp_registers = (uint16_t *) malloc(nb_points * sizeof(uint16_t));
    //memset(tab_rp_registers, 0, nb_points * sizeof(uint16_t));
    

     // write data in test.txt 
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
 */
    return NULL;
}

int opendb()
{
	
	char passwd[25];/*避免使用strings指令得知密碼*/
	passwd[0]='g';
	passwd[1]='h';
	passwd[2]='3';
	passwd[3]='7';
	passwd[4]='1';
	passwd[5]='4';
	passwd[6]='3';
	passwd[7]='7';
	passwd[8]='1';
	passwd[9]='5';
        dp = mysql_init(NULL);
        if(dp == NULL)
        {
                fprintf(stderr,"mysql_init() faild\n");
                return 2;
        }

        mysql_real_connect(dp, MyHOST, MyNAME, passwd, MyDB, 0, NULL, 0);
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

		memmove(query+31, empty, dis_emp);
		memmove(query+31+dis_emp, data_tmp, data_size);
	}
	else if(data_size == 8)
	{
		memmove(query+31, data_tmp, data_size);
	
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
