#include <stdio.h>
#include <mysql/mysql.h>
#include <unistd.h>
#include<string.h>

#define MyHOST "127.0.0.1"
#define MyNAME "root"
#define MyDB "SystemControl"
#define QSIZE 256

MYSQL *dp;
MYSQL_RES *qp;
MYSQL_ROW row;

int opendb(void)
{
	char pass[128];/*避免使用strings指令去得知密碼*/
	pass[0]='g';
        pass[1]='h';
        pass[2]='3';
        pass[3]='7';
        pass[4]='1';
        pass[5]='4';
        pass[6]='3';
        pass[7]='7';
        pass[8]='1';
        pass[9]='5';

	dp = mysql_init(NULL);
	if(dp == NULL)
	{
		fprintf(stderr,"mysql_init() faild\n");
		return 2;
	}

	mysql_real_connect(dp, MyHOST, MyNAME, pass, MyDB, 0, NULL, 0);
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

int main(int argc, char* argv[])
{
	int i;
	int err, rows, cols;
	err = opendb();
	if(err)
	{
		/*Database is not open*/
		return err;
	}
	
	char query[QSIZE];
	strcpy(query, argv[1]);
	if( mysql_query(dp, query))
	{
		fprintf(stderr, "%s\n", mysql_error(dp));
	}
	char query2[QSIZE] = "SELECT * from DeviceDataField";
	if( mysql_query(dp, query2))
	{
		fprintf(stderr, "%s\n", mysql_error(dp));
	}
	
	qp = mysql_store_result(dp);
	if(qp != NULL){
		/*所有執行結果都放在RAM中,由qp指著*/
		
		rows = mysql_num_rows(qp);
		cols = mysql_num_fields(qp);

		while((row = mysql_fetch_row(qp)) != NULL)
		{
			for(i=0; i<cols; i++)
			{
				printf("%s\t", row[i]);
			}
			putchar('\n');
		}
		
		mysql_free_result(qp);
	}
	
	mysql_close(dp);
	return 0;
}

