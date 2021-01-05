#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

#define MAX_SIZE 32764
#define KLIENT 1
#define SERVER 2
pthread_t id_watku_01, id_watku_02;
int id;
int licznik = 0;
int koniec = 0;
void* status;

struct komunikat{
	long mtype;
        int mpid;
	char mtext[MAX_SIZE];
};

int pobierz_klucz_glowny(){
    int klucz = ftok(".", 'A');
    if(klucz < 0){
        printf("Nie mozna wygenerowac klucza glownego\n");
        exit(EXIT_FAILURE);
    }
    return klucz;
}
int utworz_kolejke(int klucz){
    int id = msgget(klucz, IPC_CREAT | 0600);

    if(id < 0){
        if(errno == ENOMEM)
            perror("Blad podczas tworzenia kolejki, zwolnij miesce w pamieci: ");
        else if(errno == ENOSPC)
            perror("Blad podczas tworzenia kolejki, brak miejsca za kolejna kolejke: ");
        else
            perror("Blad podczas tworzenia kolejki: ");

        exit(EXIT_FAILURE);
    }

    return id;
}
int czy_istnieje_kolejka(){
    key_t key = ftok(".", 'A');
    if(key < 0){
        printf("Nie mozna utworzyc klucza\n");
        exit(EXIT_FAILURE);
    }

    int id = msgget(key, IPC_CREAT | IPC_EXCL | 0600);

    if(id < 0)
        return 1;
    return 0;
}
void dodaj_komunikat(int id, struct komunikat kom){
    int komSize = strlen(kom.mtext) + 1 + sizeof(int);
    //if(komSize > MAX_SIZE + sizeof(int)){
    //    printf("Komunikat jest za dlugi zostanie obciety do %d znakow.\n", MAX_SIZE);
        komSize = MAX_SIZE + sizeof(int);
        kom.mtext[MAX_SIZE - 1] = '\0';
    //}

    //printf("DK: %s, %d, %ld, D: %d\n", kom.mtext, kom.mpid, kom.mtype, komSize);

    int wysylka = msgsnd(id, (struct msgbuf*)&kom, komSize, IPC_NOWAIT);  //0 -> Czekamy na miejsce w kolejce
    if(wysylka < 0){
        if(errno == EINTR){
            perror("Perwanie podczas dodawania komunikatu: ");
            printf("Ponawiam probe dodania komunikatu...\n");
            dodaj_komunikat(id, kom);
        }
        if(errno == ENOMEM){
            perror("Blad podczas dodawania komunikatu. Brak miejsca w pamieci: ");
            printf("Komunikat zostaje usuniety\n");
            return;
        }
        else
            perror("Blad podczas dodawania komunikatu do kolejki: ");
            //pthread_exit(status);
            koniec = -1;
	    licznik--;
	    //exit(EXIT_FAILURE);
    }
}
struct komunikat pobierz_komunikat(int id, int type){
    struct komunikat kom;
    
    int pobranie = msgrcv(id, (struct msgbuf*)&kom, MAX_SIZE + sizeof(int), type, 0 | MSG_NOERROR);
    if(pobranie < 0){
        if(errno == EINTR){
            perror("Perwanie podczas pobierania komunikatu: ");
            printf("Ponawiam probe pobierania komunikatu...\n");
            pobierz_komunikat(id, type);
        }
        if(errno == EIDRM){
            perror("Blad podczas pobierania komunikatu do kolejki: ");
            printf("Serwer zakonczyl prace\n");
        }

        else
            perror("Blad podczas pobierania komunikatu do kolejki: ");
            pthread_exit(status);
	    koniec = -1;
	    //exit(EXIT_FAILURE);
    }

    return kom;
}
void usun_kolejke(int id){
    int usuniecie = msgctl(id, IPC_RMID, 0);
    if(usuniecie < 0){
        if(errno == EIDRM)
          perror("Bload podczas usuwania kolejki. Kolejka juz ostala usunieta: ");
        else
            perror("Bload podczas usuwania kolejki.: ");
        exit(EXIT_FAILURE);
    }
}
