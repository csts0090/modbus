#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include "modbus.h"
 
#define MODBUS_CLIENT_IP            "140.127.196.105"
#define MODBUS_CLIENT_PORT          5502
#define MODBUS_DEVICE_ID            10
#define MODBUS_TIMEOUT_SEC          3
#define MODBUS_TIMEOUT_USEC         0
#define MODBUS_DEBUG                ON
 
#define MODUBS_HOLDING_ADDR         0
#define MODUBS_HOLDING_LEN          64
 
int main(int argc, char *argv[])
{
    modbus_t        *ctx;
    struct timeval  timeout;
    int             ret, ii;
    /*uint8_t       bits[MODBUS_MAX_READ_BITS] = {0};*/
    uint16_t        regs[MODBUS_MAX_READ_REGISTERS] = {0};
    char            regs2[MODBUS_MAX_READ_REGISTERS]={0};
    FILE *fp = NULL;
    
   if(argc != 3){
	printf("INsufficient argument");
	return 1;
    }    
 
    ctx = modbus_new_tcp(MODBUS_CLIENT_IP, MODBUS_CLIENT_PORT);
 
    /* set device ID */
    modbus_set_slave(ctx, MODBUS_DEVICE_ID);
 
    /* Debug mode */
    modbus_set_debug(ctx, MODBUS_DEBUG);
 
    /* set timeout */
    timeout.tv_sec = MODBUS_TIMEOUT_SEC;
    timeout.tv_usec = MODBUS_TIMEOUT_USEC;
    modbus_get_byte_timeout(ctx, &timeout);
 
    timeout.tv_sec = MODBUS_TIMEOUT_SEC;
    timeout.tv_usec = MODBUS_TIMEOUT_USEC;
    modbus_set_response_timeout(ctx, &timeout);
    
    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "Connexion failed: %s\n",
                modbus_strerror(errno));
        modbus_free(ctx);
        exit(-1);
    }

     /* write data in test.txt */
    fp = fopen("test.txt", "w");
    if(fp == NULL)
    {
        printf("fail to open file!\n");
        return -1;
    }
 

 
 
    /* read holding registers (0x03 function code) */

    ret = modbus_read_registers(ctx, strtol(argv[1], NULL, 16), strtol(argv[2], NULL, 10), regs);
    if (ret < 0) {
        fprintf(stderr, "%s\n", modbus_strerror(errno));
    }
    else {
            printf("HOLDING REGISTERS:\n");
            for (ii=0; ii < ret; ii++) {
		sprintf(regs2, "%d", regs[ii]);
		fputs(regs2, fp);
		fputs("\n", fp);
           	printf("[%d]=%d\n", ii, regs[ii]);
            }
    }
	
    

    fclose(fp);

    /* Close the connection */
    modbus_close(ctx);
    modbus_free(ctx);
 
    exit(0);
}
