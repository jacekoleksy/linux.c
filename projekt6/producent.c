#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/resource.h>
#include <time.h>
#include <ctype.h>

FILE *fp_in;
#define NAZWA_FIFO "./k_fifo"

void generate(int l_z, int potok, FILE* plik);
FILE* generateFile();
void argument_checker(int argc, char *argv[], int* l_p, int* l_z);
void process_checker(int l_p, int l_z);

int main(int argc, char *argv[]){    
    //Sprawdzenie argumetnow
    int l_p, l_z;
    argument_checker(argc, argv, &l_p, &l_z);
    //printf("l_p: %d, l_z: %d\n", l_p, l_z);
    process_checker(l_p, l_z);

    //przekazuje argument l_p (liczba producentow) do pliku z ktorego odczytam te wartosc w konsument.c
    FILE *plik;
    if ((plik=fopen("liczbaProcesow.txt", "w")) == NULL){
	printf("Nie moge otworzyc pliku liczbaProcesow.txt do zapisu!\n");
	exit(-1);
    }
    fprintf(plik, "%d", l_p);
    fclose(plik);

    //usuwamy wczesniejsze pliki we/wy
    printf("Usuwam wszystkie wczesniejsze pliki we i wy...");
    system("rm -r w*");
    printf("\n");    

    srand(time(NULL));

    //Tworzenie potoku fifo
    if(access(NAZWA_FIFO, F_OK) == -1){
        int res = mkfifo(NAZWA_FIFO, 0600);
        if(res != 0){
            perror("Blad podczas tworzenia kolejki fifio: ");
            exit(EXIT_FAILURE);
        }
    }
    //else exit(-2);

    //Otwarcie potoku
    printf("Program czeka na podpiecie klientow...\n");
    int potok=open(NAZWA_FIFO, O_WRONLY);    //NON_BLOCK
    printf("Wykryto klientow\n");

    system("rm -r liczbaProcesow.txt");

    //Petla tworzaca producentow
    printf("WE:\n");
    for (int i = 0; i < l_p; i++){
        switch (fork()){
            case -1:
                    printf("Blad podczas tworzenia watku producenta\n");
                    exit(EXIT_FAILURE);
                    break;
            case 0:
            {
		    //printf("%d\t\t", i+1);
                    FILE* plik = generateFile();
                    generate(l_z, potok, plik);
            }
            break;

            default:
            break;
        }
    }
    
    //Czekanie na zakonczenie procesów
    for (int i = 0; i < l_p; i++){
        int potomny, status;
        potomny = wait(&status);
    }


    close(potok);
    unlink(NAZWA_FIFO);
    printf("Ilosc znakow w plikach WE:\n");
    system("wc -c we* | nl");

    return 0;
}


//Funkcja generujaca i zapisujaca znaku
void generate(int l_z, int potok, FILE* plik){
    char c_zapis[1];
    for (int i = 0; i < l_z; i++){
        c_zapis[0] = rand() % 93 + 33;
        //printf("Wygenerowano: %s\n", c_zapis);
        if(write(potok, c_zapis, 1)  < 0){
            if(errno == EINTR)
                generate(l_z, potok, plik);
            else{
                perror("Blad podczas zapisu:\n");
                exit(EXIT_FAILURE);
            }
        }
        else
            fputc(c_zapis[0], plik);
    }
    
    close(potok);
    fclose(plik);
    exit(EXIT_SUCCESS);
}


//Funkcja generujaca plik do zapisu dla danego procesu
FILE* generateFile(){
    //Tworzenie pliku
    srand(getpid());

    char filename[256] = "we_";
    char pid[16];
    sprintf(pid, "%d", getpid());
    strcat(filename, pid);
    strcat(filename, ".txt");

    FILE* plik = fopen(filename, "w");
    
    //printf("%s\n", filename);    

    return plik;
}



//Funkcja sprawdzajaca poprawnosc argumetntow
void argument_checker(int argc, char *argv[], int* l_p, int* l_z){
    if(argc != 3){
        printf("Blad zla ilosc argumentow\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < strlen(argv[1]); i++){
        if(!isdigit(argv[1][i])){
            printf("Blad argumentow! Wartosc nie jest liczba\n");
            exit(EXIT_FAILURE);
        }
    }
    
    for (int i = 0; i < strlen(argv[2]); i++){
        if(!isdigit(argv[2][i])){
            printf("Blad argumentow! Wartosc nie jest liczba\n");
            exit(EXIT_FAILURE);
        }
    }

    long temp_p = strtol(argv[1], NULL, 10);
    long temp_z = strtol(argv[2], NULL, 10);
    if(temp_p < 0 || temp_p > 1000000 || temp_z < 0 || temp_z > 1000000){	//INT_MAX
        //if(errno == ERANGE)
        printf("Blad argumentow\n");
        exit(EXIT_FAILURE);
    }

    *l_p = temp_p;
    *l_z = temp_z;
}



//Funkcja sprawdzajaca ilosc wolnych procesow
void process_checker(int l_p, int l_z){
    const int size = 256;
    char buf[size];
    fp_in = popen("ps ux | grep ^4638 | wc -l", "r");			
    fgets(buf,size,fp_in);
    int liczbaUruchomionychProcesow = atoi(buf);
    if(pclose(fp_in) == -1){
	printf("PClose error\n");
	exit(EXIT_FAILURE);
    }
    
    fp_in=popen("ulimit -u", "r");
    fgets(buf,size,fp_in);
    int limitProcesow = atoi(buf);
    if(pclose(fp_in) == -1){
        printf("PClose error\n");
        exit(EXIT_FAILURE);
    }
    printf("\tLiczba uruchomionych procesow (ps ux): %d\n\tLimit procesow (unlimit -u): %d\n", liczbaUruchomionychProcesow, limitProcesow);

    if(limitProcesow - liczbaUruchomionychProcesow - 2 < l_p){
        printf("Limit error: Zbyt maly limit dostepnych procesow\n");   
	printf("Limit: %d\n", limitProcesow - liczbaUruchomionychProcesow - 2);
        exit(2);
    }
    if(l_p * l_z > 1500000)
	printf("\t\tProces może zajac bardzo duzo czasu...\n");
}
