#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
int main()
{
	long long test=39;
	for(int i=0;i<64;i++)
	{
		if(test%2==0)
		{
			printf("%d \n",0);
		}
		else 
		{
			printf("%d \n",1);
		}
		test=test/2;
	}
}
