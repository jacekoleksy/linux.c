#include "sem.h"
#include "pam.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
int main(){

    //Otwarcie pliku i zainicjalizowanie semaforów oraz pamieci
    FILE* plik = fopen("dane", "r");
    if(plik == NULL)
        printf("Nie mozna otworzyc pliku z danymi!\n");

    int sem_id;
    if(semafor_sprawdz() > -1){ //Czy semafory zostaly stworzone
        sem_id = semafor_init();    //Nowe semafory
        semafor_ustaw(sem_id);
        semafor_v(sem_id, 1);
        printf("Oczekiwanie na konsumenta...\n");
        semafor_p(sem_id, 0);
    }
    else{
        sem_id = semafor_init();
        semafor_p(sem_id, 0);
        printf("Producent przylaczony\n");
        semafor_v(sem_id, 1);
    }

    int pam_id = pamiec_init();

    //Przdzial pamieci i ustawienie semaforów
    char* adres = pamiec_dolacz(pam_id);

    //Wczytywanie do pamieci z pliku
    char temp = 0;
    while (temp != EOF){
        semafor_p(sem_id, 0);   //Oczekiwanie na wyslanie
        temp = fgetc(plik);
        *adres = temp;          //Wysylka
        printf("Surowiec: %c\t", temp);
        printf("Wyslane: %c\n", *adres);
	srand(time(NULL));
	int random = rand()%3+1;
        sleep(random);
        semafor_v(sem_id, 1);   //Informacja o wysylce
    }

    semafor_p(sem_id, 0);

    fclose(plik);                   //Zamkniecie pliku
    pamiec_odlacz(adres, pam_id);   //Odlaczenie pamieci
    usun_semafor(sem_id);
    printf("Plik z danymi:\n\t");
    char *argv[]={"cat","dane",0};
    execvp(argv[0],argv);
    
    return 0;
}   
