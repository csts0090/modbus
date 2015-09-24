//檔案處理
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

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
   //pthread_mutex_lock(&beers_lock);
    struct f_ile* temp = (struct f_ile*)parm;
    printf("%s %s %s %s %s %s\n",(*temp).sch_id,temp->sta_no, temp->addr,temp->t_start,temp->t_stop,temp->period);
    //pthread_mutex_unlock(&beers_lock);
    return NULL;
}
