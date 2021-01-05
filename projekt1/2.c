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
	int mac, id;
	mac = getpid();
	char cmd[30];
	sprintf(cmd, "pstree -p %d", mac);
	printf("ID procesu macierzystego: %d \n", mac);
	for(int i=0; i<3; i++){
		id = fork();
		printf("*");
	}
	printf("Proces potomny: ");
	info();
	
	if(mac==getpid())
		system(cmd);
	sleep(3);
	
	return 0;
}

