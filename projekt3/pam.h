#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <stdlib.h>
int pamiec_init(){
    key_t klucz = ftok(".", 'A');
    int pam = shmget(klucz, 1, 0600|IPC_CREAT);
    if(pam == -1)
    {
        perror("Blad podczas wydzielania pamieci: ");
        exit(EXIT_FAILURE);
    }
    
    return pam;
}

char* pamiec_dolacz(int pam_id){
    char* adress = shmat(pam_id, 0, 0);
    if(*adress < 0){
        perror("Blad podczas dolaczania pamieci: ");
        exit(EXIT_FAILURE);
    }
    return adress;
}

void pamiec_odlacz(char* adres, int pam_id){
    int usuwanie = shmctl(pam_id, IPC_RMID, 0);
    if(usuwanie < 0){
        perror("Blad z usunieciem pamieci: ");
    }

    int odlaczenie = shmdt(adres);
    if(odlaczenie < 0){
        perror("Blad z odlaczeniem pamieci: ");
    }
}

