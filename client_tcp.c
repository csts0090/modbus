#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include "modbus.h"
 
#define MODBUS_CLIENT_IP            "192.168.1.5"
#define MODBUS_CLIENT_PORT          502
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
    char            regs2[MODBUS_MAX_READ_REGISTERS] = {0};
    unsigned int    regs3;
    char *flo_byte;
   // unsigned int dec_float;
    uint16_t        little_endian[2]={0};
    uint16_t        index;
    float f, *_float;
    FILE *fp = NULL;
    
  /* if(argc != 3){
	printf("INsufficient argument");
	return 1;
    }  */  
 
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
    fp = fopen("test.txt", "a+");
    if(fp == NULL)
    {
        printf("fail to open file!\n");
        return -1;
    }
 
    
 
 
    /* read holding registers (0x03 function code) */
/*
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
*/

   ret = modbus_read_registers(ctx, 0x1521, 7,regs);	
   if(ret < 0){
   	fprintf(stderr, "%s\n", modbus_strerror(errno));
   }
   else{
	index = 0x1521;
   	printf("HOLDING REGISTER:\n");
	for(ii=0; ii < ret; ii++, index++){
		sprintf(regs2, "%d", regs[ii]);
		fprintf(fp, "%x",index);
		fputs(":", fp);
		fputs(regs2, fp);
		if(ii < ret-1)
			fputs(",", fp);
		if(ii == ret-1)
			fputs("\n", fp);
           	printf("[%d]=%d\n", ii, regs[ii]);
	
	}
   }


	f = 60.0;
	flo_byte = (char *)&f;

/*	big_endian[0] = flo_byte[2]*256 + flo_byte[3];
	big_endian[1] = flo_byte[0]*256 + flo_byte[1];*/

	little_endian[0] = flo_byte[1]*256 + flo_byte[0];
	little_endian[1] = flo_byte[3]*256 + flo_byte[2];

	ret = modbus_write_registers(ctx, strtol(argv[1], NULL, 16), 2, &little_endian[0]);
   	 if (ret < 0) {
        	fprintf(stderr, "%s\n", modbus_strerror(errno));
    	 }
   
  	 ret = modbus_read_registers(ctx, 0x1066, 2,regs);	
  	 if(ret < 0){
   		fprintf(stderr, "%s\n", modbus_strerror(errno));
   	 }
   	 else{
   		printf("HOLDING REGISTER:\n");

		/*INT轉float*/
	
		regs3 = (regs[1] * 65536) + regs[0];	
		_float = (float *)(&regs3);
	
		
		
		fprintf(fp, "%f",*_float);
		fputs("\n", fp);
           	printf("%f\n", *_float);
	
	
  	     }



   ret = modbus_read_registers(ctx, 0x10c8, 100,regs);
   if(ret < 0){
        fprintf(stderr, "%s\n", modbus_strerror(errno));
   }
   else{
        index = 0x10c8;
        printf("HOLDING REGISTER:\n");
        for(ii=0; ii < ret; ii++, index++){
                sprintf(regs2, "%d", regs[ii]);
                fprintf(fp, "%x",index);
                fputs(":", fp);
                fputs(regs2, fp);
                if(ii < ret-1)
                        fputs(",", fp);
                if(ii == ret-1)
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
