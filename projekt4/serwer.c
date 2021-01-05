#include "funkcje.h"
#include <ctype.h>
#include <signal.h>

void procesString(char* str){
    int j = 0;
    while (str[j]){
        str[j] = toupper(str[j]);
        j++; 
    }
}
void handler(int sig){
    int id = utworz_kolejke(pobierz_klucz_glowny());
    usun_kolejke(id);
    printf("Usuwanie kolejki serwera\n");
    exit(EXIT_SUCCESS);
}

int main(){
    int id = utworz_kolejke(pobierz_klucz_glowny());
    signal(SIGINT, handler);

    while (1){
        struct komunikat kom = pobierz_komunikat(id, SERVER);
        printf("Serwer pobieral od %d dane %s\n", kom.mpid, kom.mtext);
        
        procesString(kom.mtext);
        
        kom.mtype = kom.mpid;
        printf("Serwer wysyla do %d dane %s\n", kom.mpid, kom.mtext);
        dodaj_komunikat(id, kom);
    }

    return 0;
}
