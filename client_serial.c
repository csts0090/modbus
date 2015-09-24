#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
 
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
 

   if(argc != 3){
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
    modbus_set_slave(ctx, MODBUS_DEVICE_ID);
 
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



    rc = modbus_read_registers(ctx, strtol(argv[1], NULL, 16), strtol(argv[2], NULL, 10), regs);
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
            }
    }

    fclose(fp);

    /* Close the connection */
    modbus_close(ctx);

    modbus_free(ctx);
 
    return 0;
}
