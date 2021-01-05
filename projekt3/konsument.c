#include "sem.h"
#include "pam.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
int main(){
    //Otwarcie pliku i zainicjalizowanie semaforow oraz pamieci
    key_t klucz = ftok(".", 'A');
    FILE* plik = fopen("wynik", "w");
    if(plik == NULL)
        printf("Nie mozna otworzyc pliku wynikowego!\n");

    int sem_id;
    if(semafor_sprawdz() > -1){ //Czy semafory zostaly stworzone
        sem_id = semafor_init();    //Nowe semafory
        semafor_ustaw(sem_id);
        semafor_v(sem_id, 0);
        printf("Oczekiwanie na producenta...\n");
        semafor_p(sem_id, 1);
    }
    else{
        sem_id = semafor_init();
        semafor_p(sem_id, 1);
        printf("Konsument przylaczony\n");
        semafor_v(sem_id, 0);
    }

    int pam_id = pamiec_init();
    
    //Przdzial pamieci
    char* adres = pamiec_dolacz(pam_id);

    //Gotowosc do przyjmowania informacji
    semafor_v(sem_id, 0);

    //Wczytywanie wartosci z adresu
    while (1){
        semafor_p(sem_id, 1);   //Oczekiwanie na nadejscie
        if(*adres == EOF)       //Sprawdzenie czy to nie jest koniec pliku
        {
            semafor_v(sem_id, 0);   //Informacja o zakonczeniu wczytywania
            break;
        }
	printf("Zapisane: %c\n", *adres);
        fprintf(plik, "%c", *adres);    //Zapisanie informacji do pliku
        semafor_v(sem_id, 0);           //Informacja otrzymaniu informacji
    }

    fclose(plik);                   //Zamkniecie pliku
    pamiec_odlacz(adres, pam_id);   //Odlaczenie pamieci
    printf("Plik wynikowy:\n\t");
    char *argv[]={"cat","wynik",0};
    execvp(argv[0],argv);

    return 0;
}   
