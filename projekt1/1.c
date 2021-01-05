#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<wait.h>
#include<sys/types.h>
void info(){
	printf("PID: %d ",getpid());
	printf("PPID: %d ",getppid());
	printf("UID: %d ",getuid());
	printf("GID: %d \n",getgid());	
}
int main(){
	info();

	return 0;
}

