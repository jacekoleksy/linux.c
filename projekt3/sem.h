#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/errno.h>
int semafor_init(){
    key_t klucz = ftok(".", 'B');
    if(klucz == -1){
        printf("Blad podczas tworzenia klucza\n");
        exit(EXIT_FAILURE);
    }

    int semafor = semget(klucz, 2, IPC_CREAT | 0600);
    if(semafor == -1){
        perror("Blad podczas uzsykiwania dostepu do semafora: ");
        exit(EXIT_FAILURE);
    }

    return semafor;
}

int semafor_sprawdz(){
    key_t klucz = ftok(".", 'B');
    return semget(klucz, 2, IPC_CREAT | IPC_EXCL | 0600);
}

void semafor_ustaw(int id){
    for (int i = 0; i < 2; i++){
        int ustaw_sem;
        ustaw_sem=semctl(id, i, SETVAL, 0);
        if (ustaw_sem==-1){
            perror("Nie mozna ustawic semafora: ");
            exit(EXIT_FAILURE);
        }
    }
}

void semafor_p(int id, int sem){
    int zmien_sem;
    struct sembuf bufor_sem;
    bufor_sem.sem_num = sem;
    bufor_sem.sem_op = -1;
    bufor_sem.sem_flg=SEM_UNDO;
    zmien_sem=semop(id,&bufor_sem,1);
    if (zmien_sem==-1){
        if(errno == EINTR){
            perror("Przerwanie podczas zamykania: ");
            printf("Ponawiam probe zamkniecia...\n");
            semafor_p(id, sem);
        }
        else{
            perror("Nie moglem zamknac semafora: ");
            exit(EXIT_FAILURE);
        }
    }
}

void semafor_v(int id, int sem){
    int zmien_sem;
    struct sembuf bufor_sem;
    bufor_sem.sem_num = sem;
    bufor_sem.sem_op = 1;
    bufor_sem.sem_flg=IPC_NOWAIT;
    zmien_sem=semop(id,&bufor_sem,1);

    if (zmien_sem==-1){
        if(errno == EAGAIN){
            perror("Blad zmienna errno: ");
            perror("Ponawiam probe otwarcia...\n");
            semafor_v(id, sem);
        }

        if(errno == EINTR){
            perror("Przerwanie podczas otwierania: ");
            printf("Ponawiam probe otwarcia...\n");
            semafor_p(id, sem);
        }
        else{
            perror("Nie mozna otworzyc semafora: ");
            exit(EXIT_FAILURE);   
        }
    }      
}

void usun_semafor(int id){
    int sem;
    sem=semctl(id,0,IPC_RMID);
    if (sem==-1){
        perror("Nie mozna usunac semafora: ");
        exit(EXIT_FAILURE);
    }
}
