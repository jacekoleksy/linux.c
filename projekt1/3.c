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
void czekaj(){
	int status;
	if(wait(&status)==-1){
		perror("Wait error: ");
		exit(3);
	}
	printf("Proces dziecka zakonczyl sie kodem: %d\n", status);
}
int main(){
	int mac, id;
	mac = getpid();
	char cmd[30];
	char* args[]={"./program1",NULL};
	sprintf(cmd, "pstree -p %d", mac);
	printf("ID procesu macierzystego: %d \n", mac);
	for(int i=0; i<3; i++){
		switch(id = fork()){
			case -1:
				perror("Fork error: ");
				exit(1);
			case 0:
				if(execvp(args[0],args)==-1){
					perror("Exec error: ");
					exit(2);
				}
			default:
				continue;
		}
	}
	
	if(mac==getpid())
		system(cmd);
	for(int j=0; j<3; j++){
		czekaj();
	}	

	sleep(3);
	
	return 0;
}

