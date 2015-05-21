#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include <time.h>
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
#define QSIZE 256

MYSQL *dp ;
MYSQL_RES *qp;

/*----------------database_end---------------------------*/


typedef struct schedule
{
	char sch_id[4];
	char sta_no[3];
	char addr[5];
        char t_start[20];
	char t_stop[20];
	char period[3];
	
}schedule_t;

void* schedule_thread(void *a);

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
        passwd[10]='\0';


        dp = mysql_init(NULL);


        if(dp == NULL)
        {
                fprintf(stderr,"mysql_init() faild\n");
                return 2;
        }
		/* char value = 1;
		 mysql_options(dp, MYSQL_OPT_RECONNECT, &value);*/

	

        mysql_real_connect(dp, MyHOST, MyNAME, passwd, MyDB, 0, NULL, 0);
        if(dp == NULL)
        {
                fprintf(stderr,"mysql_real_connect() faild\n");
                return 3;
        }
        	if (mysql_ping(dp)) {
		 char value = 1;
		 mysql_options(dp, MYSQL_OPT_RECONNECT, &value);
		printf("Ping dp error: %s\n", mysql_error(dp));
		} else {
		puts("Ping OK\n");
		}

/*       if(mysql_select_db(dp, MyDB) != 0)
        {
                fprintf(stderr, "mysql_select_db() failed\n");
                mysql_close(dp);
                return 4;
        }*/
        return 0;
}

/*void sql_write(char *sch_id, char *station_no, char *address, char *data, char *time)
{
        sprintf(query, "INSERT INTO Schedule_data (schedule_id, station_no, address, data) VALUES (  %s,  %s, %s, '%s')", sch_id, station_no, address, data); 
        printf("%s\n",query);
                if( mysql_query(dp, query))
        {       
                fprintf(stderr, "%s\n", mysql_error(dp));
        }       
}*/



int server (int client_socket)
{
    int length=10;
    char text[10];
    int ret;

    /* Allocate a buffer to hold the text.  */
    /* Read the text itself, and print it.  */
    ret = recv(client_socket, text , length, 0);
    if(ret==0) {
	printf ("peer close socket\n");
	return 1;
	}
    else if (ret<0) {
	 printf("socket error: %s\n", strerror(errno));
	 return 1;
	}
    else {
	  text[ret] = '\0';
	  printf ("%s\n", text);
	  return 1;
         }
													       

    /* Free the buffer.  */
    free (text);
    return 0;
}
 
void* listen_thread(void *id)
{
    	int *p_id = (int*) id;
        //Wait for php send
	char soc_n[20];
        sprintf(soc_n, "sch_%d",*p_id);
	printf("pid=%s\n",soc_n);
	const char* const socket_name="sch";
	int socket_fd;
	struct sockaddr_un name;
	int client_sent_quit_message;

  	/* Create the socket.  */
	socket_fd = socket (PF_LOCAL, SOCK_STREAM, 0);
  	/* Indicate this is a server.  */
  	name.sun_family = AF_LOCAL;
  	strcpy (name.sun_path, socket_name);
  	bind (socket_fd,  (struct sockaddr *)&name, sizeof(struct sockaddr_un));
  	/* Listen for connections.  */
  	listen (socket_fd, 20);

  	/* Repeatedly accept connections, spinning off one server() to deal
     	with each client.  Continue until a client sends a "quit" message.  */
  	do {
    	struct sockaddr_un client_name;
    	socklen_t client_name_len;
    	int client_socket_fd;

    	/* Accept a connection.  */
    	client_socket_fd = accept(socket_fd, (struct sockaddr *)&client_name, &client_name_len);
    	/* Handle the connection.  */
    	client_sent_quit_message = server(client_socket_fd);
    	/* Close our end of the connection.  */
    	close (client_socket_fd);
  	}
  	while (!client_sent_quit_message);

  	/* Remove the socket file.  */
  	close (socket_fd);
  	unlink (socket_name);
	return NULL;
}

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
	char *temp;
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
	schedule_t sch[num-1];
	int k=0;
	while(fgets(s, fileLength, fptr) != NULL)
	{
		if(i == 0){
			for(int j=0; j< num; j++){
				strcpy(sch[j].sch_id, s);
				printf("%d:%s\n",j,sch[j].sch_id);
			}
		}
		else{  
			temp = strtok(s, " ");
			for(int j=0; temp != NULL; j++)
			{
				if(j == 0)strcpy(sch[k].sta_no,temp); 
				if(j == 1)strcpy(sch[k].addr, temp); 
				if(j == 2)strcpy(sch[k].t_start, temp); 
				if(j == 3){
				strncat(sch[k].t_start," ", 1); 	
				strncat(sch[k].t_start,temp,strlen(temp)); 
				}
				if(j == 4)strcpy(sch[k].t_stop, temp); 
				if(j == 5){
				strncat(sch[k].t_stop, " ", 1); 
				strncat(sch[k].t_stop, temp,strlen(temp));
				} 
				if(j == 6){
				temp = strtok(temp, "\n");
				strcpy(sch[k].period, temp);
				}

				
				printf("%s\n", temp);
				temp = strtok(NULL, " ");
				
			}
			k++;
		     }
			//printf("csts:%s \n no:%s %s %s %s %s\n",sch[1].sch_id,sch[1].sta_no, sch[1].addr,sch[1].t_start,sch[1].t_stop,sch[1].period);
		i++;
	}
	//Write pid to sql
    	int err, rows, cols;
	//err = opendb();
	//if(err)
	//{
		/*Database is not open*/
	//	return err;
	//}
	
	int p_id=getpid();
	//char q[QSIZE] = "UPDATE Schedule SET pid=10000  where schedule_id=1";
        //sprintf(q, "UPDATE Schedule SET pid=%d  where schedule_id=%d", p_id, sch[1].sch_id);
	//if( mysql_query(dp, q))
	//{
	//	fprintf(stderr, "%s\n", mysql_error(dp));
	//}

			printf("num=%d\n",num);
			for(int k=0;k<=num-2;k++)
			printf("%s \n %s %s %s %s %s\n",sch[k].sch_id,sch[k].sta_no, sch[k].addr,sch[k].t_start,sch[k].t_stop,sch[k].period);
	
		pthread_t listen;
		pthread_create(&listen, NULL, listen_thread, (void*)&p_id);

		pthread_t threads[num-1];
		for(int k=0;k<num-1;k++)
		{
			pthread_create(&threads[k], NULL, schedule_thread, (void *)(sch+k));
		}
		void *result;
			pthread_join(listen, &result);
		//for(int k=0;k<num-1;k++)
		//	pthread_join(threads[k], &result);
		
		return 0;
		

}   

pthread_mutex_t modbus_lock = PTHREAD_MUTEX_INITIALIZER;	
pthread_mutex_t mysql_lock = PTHREAD_MUTEX_INITIALIZER;	
void* schedule_thread(void* parm)
{
    //char query[QSIZE]   = "INSERT INTO Schedule_data (schedule_id, station_no, address, data, time) VALUES ( 1,10,1523, '00000000', '2015-04-24 00:00:00')";
   
    MYSQL_ROW row;

    schedule_t* temp = (schedule_t*)parm;
    printf("v v: %s \n %s \n %s %s %s %s\n",temp->sch_id,temp->sta_no, temp->addr,temp->t_start,temp->t_stop,temp->period);
    int T=atoi(temp->period);


  struct tm time_start, time_stop;
  char q1[20],q2[20],q3[20];
  strptime(temp->t_start, "%Y-%m-%d %H:%M:%S", &time_start);//類似scanf
  strftime(q1, 20, "%Y-%m-%d %H:%M:%S", &time_start);//類似sprintf
  printf("%s\n",q1);
  printf ("驗證 %d-%d-%d ", (time_start.tm_year),( time_start.tm_mon), time_start.tm_mday);
  printf("%d:%d:%d\n",time_start.tm_hour, time_start.tm_min, time_start.tm_sec);
  
  strptime(temp->t_stop, "%Y-%m-%d %H:%M:%S", &time_stop);//類似scanf
  strftime(q2, 20, "%Y-%m-%d %H:%M:%S", &time_stop);//類似sprintf
  printf("%s\n",q2);


  time_t timestart, timestop;
  timestart = mktime(&time_start);
  timestop = mktime(&time_stop);
  printf("%d\n", timestart);
  printf("%d\n", timestop);

  struct timeval start;
  struct timeval end;
  struct timeval res;

while(1){
    time_t time_now;
    struct tm p;
    time(&time_now);
    printf("now_time:%d\n", time_now);
//    localtime_r(&time_now,&p); 
//    time_now=mktime(&p);
//   printf("now_time:%d\n", time_now);
   if(time_now >= timestart && time_now <= timestop)
  {
    	//time(&time_now);
    	//p=localtime(&time_now); 
	gettimeofday(&start, NULL);

    	//time(&time_now);
    	//p=localtime(&time_now);

//	sprintf (q3,"%d-%d-%d %d:%d:%d\n", (1900+p->tm_year),( 1+p->tm_mon), p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec);
        //printf("%s\n",q3);
	
 

    pthread_mutex_lock(&modbus_lock);
    modbus_t            *ctx;
    struct timeval      timeout;
    int                 ret, i, rc, ii;
   // int nb_pointers;
   // unit16_t *tab_rp_registers;
    uint16_t        regs[MODBUS_MAX_READ_REGISTERS] = {0};
    char            regs_8[MODBUS_MAX_READ_REGISTERS]={0};

    
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
    //Get Data
    rc = modbus_read_registers(ctx, strtol(temp->addr, NULL, 16), strtol("1", NULL, 10), regs);
    if (rc < 0) {
        fprintf(stderr, "%s\n", modbus_strerror(errno));
    	pthread_mutex_unlock(&modbus_lock);
    }
    else {
            printf("HOLDING REGISTERS:\n");
            for (ii=0; ii < rc; ii++) {
                sprintf(regs_8, "%d", regs[ii]);
                printf("[%d]=%d\n", ii, regs[ii]);

            }
    		/* Close the connection */
    		modbus_close(ctx);
    		modbus_free(ctx);
    		pthread_mutex_unlock(&modbus_lock);


    		pthread_mutex_lock(&mysql_lock);
    		int err, rows, cols;
    		err = opendb();
    		if(err)
    		{
        		/*Database is not open*/
        		return err;
    		}
	


  
    		//sql_write(temp->sch_id,temp->sta_no, temp->addr, regs_8, q3);
    		char query[QSIZE];
        	sprintf(query, "INSERT INTO Schedule_data (schedule_id, station_no, address, data) VALUES (%s,%s,%s,'%s')", temp->sch_id, temp->sta_no, temp->addr, regs_8); 
        	//strcpy(query, "select * from Schedule_data"); 
        	printf("%s\n",query);
                if( mysql_query(dp, query))
       		{       
                	fprintf(stderr, "%s\n", mysql_error(dp));
        	}       
        	if (mysql_ping(dp)) {
		printf("Ping error: %s\n", mysql_error(dp));
		} else {
		puts("Ping OK\n");
		}
		mysql_close(dp);
		//mysql_thread_end();
    		pthread_mutex_unlock(&mysql_lock);

    	}

	gettimeofday(&end, NULL);
	timersub(&end, &start, &res);

    	//pthread_mutex_unlock(&beers_lock);
    	usleep(T* 1000000  - (res.tv_sec * 1000000 + res.tv_usec));
    }
    else if(time_now > timestop) {
        mysql_close(dp);
    	pthread_exit(0);
    }
}

    return NULL;
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


