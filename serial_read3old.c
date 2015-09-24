#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>
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

char query[QSIZE]   = "INSERT INTO Schedule_data (schedule_id, station_no, address, data, time) VALUES (   1,  10, 1523, '00000000', '2015-04-24 00:00:00')";
char query2[QSIZE]  = "UPDATE Schedule_data SET data= '      13' where address=1522 and schedule_id=1 and station_no= 11";
char query3[QSIZE]  = "select * from Schedule_data where schedule_id=1   and station_no=10   and address= 1521";

/*----------------database_end---------------------------*/


struct f_ile
{
	char sch_id[4];
	char sta_no[3];
	char addr[5];
        char t_start[20];
	char t_stop[20];
	char period[2];
	
};

void* sch_rule(void *a);

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

void sql_write(char *sch_id, char *station_no, char *address, char *data, char *time);
void sql_write2(char *station_no, char *address, char *data);
 
int main(int argc, char *argv[])
{
   if(argc != 2){
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
		//	for(int k=0;k<=num-2;k++)
		//	printf("%s %s %s %s %s %s\n",sch[k].sch_id,sch[k].sta_no, sch[k].addr,sch[k].t_start,sch[k].t_stop,sch[k].period);
		pthread_t threads[num-1];
		for(int k=0;k<=num-2;k++)
		{
			pthread_create(&threads[k], NULL, sch_rule, (void *)&sch[k]);
		}
		void *result;
		for(int k=0;k<=num-2;k++)
			pthread_join(threads[k], &result);
		
		return 0;
		

}   

pthread_mutex_t beers_lock = PTHREAD_MUTEX_INITIALIZER;	
void* sch_rule(void* parm)
{
   MYSQL_ROW row;

   //pthread_mutex_lock(&beers_lock);
    struct f_ile* temp = (struct f_ile*)parm;
    //printf("%s %s %s %s %s %s\n",temp->sch_id,temp->sta_no, temp->addr,temp->t_start,temp->t_stop,temp->period);
    //pthread_mutex_unlock(&beers_lock);

  time_t timep;
  struct tm *p;

  struct tm St, Sto;
  char q1[20],q2[20],q3[20];
  strptime(temp->t_start, "%Y-%m-%d %H:%M:%S", &St);//類似scanf
  strftime(q1, 20, "%Y-%m-%d %H:%M:%S", &St);//類似sprintf
  printf("%s\n",q1);
  printf ("驗證 %d-%d-%d ", (St.tm_year),( St.tm_mon), St.tm_mday);
  printf("%d:%d:%d\n",St.tm_hour, St.tm_min, St.tm_sec);
  
  strptime(temp->t_stop, "%Y-%m-%d %H:%M:%S", &Sto);//類似scanf
  strftime(q2, 20, "%Y-%m-%d %H:%M:%S", &Sto);//類似sprintf
  printf("%s\n",q2);


  time_t timest, timesto;
  timest = mktime(&St);
  timesto = mktime(&Sto);
  printf("%d\n", timest);
  printf("%d\n", timesto);
while(1){
    time_t timep;
    struct tm *p;
    time(&timep);
    p=localtime(&timep); 
    timep=mktime(p);
   //printf("%d\n", timep);
   if(timep >= timest && timep <= timesto)
  {
    	//time(&timep);
    	//p=localtime(&timep); 

    	time(&timep);
    	p=localtime(&timep);

	sprintf (q3,"%d-%d-%d %d:%d:%d\n", (1900+p->tm_year),( 1+p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
        //printf("%s\n",q3);
	
 



    modbus_t            *ctx;
    struct timeval      timeout;
    int                 ret, i, rc, ii;
    int nb_pointers;
   // unit16_t *tab_rp_registers;
    uint16_t        regs[MODBUS_MAX_READ_REGISTERS] = {0};
    char            regs2[MODBUS_MAX_READ_REGISTERS]={0};
    FILE *fp = NULL;

    int err, rows, cols;

    err = opendb();
    if(err)
    {
        /*Database is not open*/
        return err;
    }
    int q_length = strlen(query3);
    int sch_id_size = strlen(temp->sch_id);
    char sc_id[sch_id_size];
    strcpy(sc_id, temp->sch_id);
    if(sch_id_size<5 && sch_id_size > 0){
        memmove( query3+46, "    ", 4);
        memmove( query3+46, sc_id, sch_id_size);
    }
        /* copy station_no to query[] */
        
        int sta_no_size = strlen(temp->sta_no);
        char sta_no[3]; 
        strcpy(sta_no, temp->sta_no);
        if(sta_no_size < 5 && sta_no_size > 0){
                memmove( query3+65, "    ", 4);
                memmove( query3+65, sta_no, sta_no_size);
        }       
        printf("%s\n",query3);
        
        /* copy address to query[] */
        int addre_size = strlen(temp->addr);
        char addre[addre_size];
        strcpy(addre, temp->addr);
        
        memmove(query3 + (q_length - 5 - addre_size) , addre, addre_size);
        printf("%s\n",query3);
        
                if( mysql_query(dp, query3))
        {       
                fprintf(stderr, "%s\n", mysql_error(dp));
        }       
        
        qp = mysql_store_result(dp);
        if(qp != NULL){
                /*所有執行結果都放在RAM中,由qp指著*/
                
                rows = mysql_num_rows(qp);
                cols = mysql_num_fields(qp);
                printf("row=%d: col=%d\n",rows,cols);
                
                mysql_free_result(qp);
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
    else
    {
        printf("Serial mode = RS485\n");
        ret = modbus_rtu_set_serial_mode(ctx, MODBUS_RTU_RS485);
        if(ret < 0)
            fprintf(stderr, "modbus_rtu_set_serial_mode() error: %s\n", strerror(errno));
    }
   
    /* set slave device ID */
    modbus_set_slave(ctx, strtol(temp->sta_no, NULL, 10));

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





    //Get Data
    rc = modbus_read_registers(ctx, strtol(temp->addr, NULL, 16), strtol("1", NULL, 10), regs);
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
		if(row==0){
                	sql_write(temp->sch_id,temp->sta_no, temp->addr, regs2, q3);
		}else
		{
                	sql_write2(temp->sta_no, temp->addr, regs2);
		}

            }
                /*if( mysql_query(dp, query))
                {
                        fprintf(stderr, "%s\n", mysql_error(dp));
                }
                qp = mysql_store_result(dp);
                mysql_free_result(qp);*/



    }
    	fclose(fp);
    	/* Close the connection */
    	modbus_close(ctx);
    	modbus_free(ctx);

    	sleep(temp->period);
    }
    else if(timep > timesto) {
        mysql_close(dp);
    	exit(1);
    }

    return NULL;
}

//char query[QSIZE] = "INSERT INTO Schedule_data (schedule_id, station_no, address, data, time) VALUES (   1, 10, 1523, '00000000', '2015-04-24 00:00:00')";
//char query2[QSIZE] = "UPDATE Schedule_data SET data= '13' where address=1522 and schedule_id=1 and station_no=11";
//char query3[QSIZE] = "INSERT INTO Schedule_data (schedule_id, station_no, address, data, time) VALUES (   1, 10, 1523, '00000000', '2015-04-24 00:00:00')";

void sql_write(char *sch_id, char *station_no, char *address, char *data, char *time)
{
        int q_length = strlen(query);

        int sch_id_size = strlen(sch_id);
        char sc_id[sch_id_size];
        strcpy(sc_id, sch_id);
        if(sch_id_size<5 && sch_id_size > 0){
                memmove( query+81, "    ", 4);
                memmove( query+81, sc_id, sch_id_size);
        }

        printf("%s\n",query);

        /* copy station_no to query[] */

        int sta_no_size = strlen(station_no);
        char sta_no[sta_no_size];
        strcpy(sta_no, station_no);
        if(sta_no_size < 5){
                memmove( query+86, "    ", 4);
                memmove( query+86, sta_no, sta_no_size);
        }
        printf("%s\n",query);

        /* copy address to query[] */

        int addre_size = strlen(address);
        char addre[addre_size];
        strcpy(addre, address);
        if(addre_size > 4)
        {
                int distance = addre_size - 4;
                memmove(query + (q_length - 36 + distance),query+(q_length - 36), 36);
                memmove(query + (q_length - 36 - addre_size) , addre, addre_size);
        }
        else
        {
                memmove(query + (q_length - 36 - addre_size) , addre, addre_size);
        }
        printf("%s\n",query);

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

                memmove(query+99, empty, dis_emp);
                memmove(query+99+dis_emp, data_tmp, data_size);
        }
        else if(data_size == 8)
        {
                memmove(query+99, data_tmp, data_size);

        }
        int time_size = strlen(time);
        char t_ime[time_size];
        strcpy(t_ime, time);
                memmove(query + (q_length - 2 - time_size) , t_ime, time_size);

        printf("%s\n",query);
}
void sql_write2(char *station_no, char *address, char *data)
{
        int q_length = strlen(query2);

        /* copy station_no to query[] */

        int sta_no_size = strlen(station_no);
        char sta_no[3];
        strcpy(sta_no, station_no);
        memmove( query2+(q_length-2), sta_no, sta_no_size);
        printf("%s\n",query2);

        /* copy address to query[] */

        int addre_size = strlen(address);
        char addre[addre_size];
        strcpy(addre, address);
        if(addre_size > 4)
        {
                int distance = addre_size - 4;
                memmove(query2 + (q_length - 36 + distance),query2+(q_length - 36), 36);
                memmove(query2 + (q_length - 37 - addre_size) , addre, addre_size);
        }
        else
        {
                memmove(query2 + (q_length - 37 - addre_size) , addre, addre_size);
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

                //memmove(query2+33+data_size+4 ,query2+(q_length- 20 - addre_size - 16 ), 36+addre_size);

                memmove(query2+32, empty, dis_emp);
                memmove(query2+32+dis_emp, data_tmp, data_size);
        }
        else if(data_size == 8)
        {
                memmove(query2+32, data_tmp, data_size);

        }
        printf("%s\n",query2);
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


