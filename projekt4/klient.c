#include "funkcje.h"
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>

void* wyslij(void* arg){
    id = utworz_kolejke(pobierz_klucz_glowny());
    struct komunikat kom;
    while (1){
        kom.mtype = SERVER;
        kom.mpid = getpid();
	//printf("Podaj ciag znakow. MAX_SIZE = %d\n",MAX_SIZE);
        fgets(kom.mtext, MAX_SIZE, stdin);
        
        if(strlen(kom.mtext) > 0 && (kom.mtext[strlen(kom.mtext) - 1] == '\n'))
            kom.mtext[strlen(kom.mtext) - 1] = '\0';
        
        printf("Klient: %d wysyła %s\n", kom.mpid, kom.mtext);
        dodaj_komunikat(id, kom);
        licznik++;
    }
}
void* odbierz(void* arg){
    struct komunikat kom;
    kom.mtype = getpid();
    kom.mpid = getpid();
    while(1){
	//printf("koniec = %d, licznik = %d", koniec, licznik);
	if(koniec == -1 && licznik == 0){
	    printf("Koniec\n");
            int kasacja = pthread_cancel(id_watku_02);
	    exit(-1);
	}
	  //srand(time(NULL));
	  //int random = rand()%3+1;
          //sleep(random);
        licznik--;
        if(koniec == 1 && licznik == 0){
            break;
	}

        kom = pobierz_komunikat(id, kom.mtype);
        printf("\t\tKlient: %d odbiera %s\n", kom.mpid, kom.mtext);
    }
}
void handler(int sig){
    if(koniec){
        int kasacja = pthread_cancel(id_watku_02);
        if(kasacja < 0){
            printf("Blad kasacji watku_2 %ld, kod bledu %d", id_watku_02, kasacja);
            exit(-1);
        }
        return;
    }

    int kasacja = pthread_cancel(id_watku_01);
    if(kasacja < 0){
        printf("Blad kasacji watku_1 %ld, kod bledu %d", id_watku_01, kasacja);
        exit(-1);
    }
    koniec = 1;

    printf("Zakoncze program po wyczyszczeniu kolejki lub kliknij jeszcze raz ctr + c by zakonczyc teraz\n");
}

int main(){
    signal(SIGINT, handler);

    int tworzenie = pthread_create(&id_watku_01,NULL, wyslij, NULL);
    if (tworzenie < 0){
        printf("Blad tworzenia watku_1 %ld, kod bledu %d", id_watku_01, tworzenie);
        exit(-1);
    }
    tworzenie = pthread_create(&id_watku_02,NULL, odbierz, NULL);
    if (tworzenie < 0){
        printf("Blad tworzenia watku_2 %ld, kod bledu %d", id_watku_02, tworzenie);
        exit(-1);
    }

    int przylaczenie = pthread_join(id_watku_01,NULL);
    if (przylaczenie < 0){
        printf("Blad przyczepiania watku_1 %ld, kod bledu %d", id_watku_01, przylaczenie);
        exit(-1);
    }
    przylaczenie = pthread_join(id_watku_02,NULL);
    if (przylaczenie < 0){
        printf("Blad przyczepiania watku_2 %ld, kod bledu %d", id_watku_02, przylaczenie);
        exit(-1);
    }

    int odlaczenie = pthread_detach(id_watku_01);
    if (odlaczenie < 0){
        printf("Blad przyczepiania watku_1 %ld, kod bledu %d", id_watku_01, odlaczenie);
        exit(-1);
    }
    odlaczenie = pthread_detach(id_watku_02);
    if (odlaczenie < 0){
        printf("Blad przyczepiania watku_2 %ld, kod bledu %d", id_watku_02, odlaczenie);
        exit(-1);
    }

    return 0;
}
