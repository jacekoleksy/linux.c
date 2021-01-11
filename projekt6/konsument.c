#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/resource.h>

FILE *fp_in;
#define NAZWA_FIFO "./k_fifo"

void readFifo(int potok, FILE* plik);
FILE* generateFile();
void argument_checker(int argc, char *argv[], int* l_k);
void process_checker(int l_k);


int main(int argc, char *argv[]){
    //Sprawdzenie argumetnow
    int l_k;
    argument_checker(argc, argv, &l_k);

    process_checker(l_k);    

    //Otwarcie potoku
    int potok = open(NAZWA_FIFO, O_RDONLY);
	if(potok < 0){
		perror("Nie wykryto kolejki FIFO. Uruchom producenta pierwszego: \n");
		exit(EXIT_FAILURE);
	}
    
    //Petla tworzaca konsumentow
    printf("WY:\n");
    for (int i = 0; i < l_k; i++){
        switch (fork()){
            case -1:
                    printf("Blad podczas tworzenia procesu producenta\n");
                    exit(EXIT_FAILURE);
                    break;
            case 0:
                {
		    //printf("%d\t\t", i+1);
                    FILE* plik = generateFile();
                    readFifo(potok, plik);
                }
            break;

            default:
            break;
        }
    }

    //Czekanie na zakonczenie procesow
    for (int i = 0; i < l_k; i++){
        int potomny, status;
        potomny = wait(&status);
    }

    close(potok);
    unlink(NAZWA_FIFO);
    printf("Ilosc znakow w plikach WY:\n");
    system("wc -c wy* | nl");

    return 0;
}


//Funkcja generujaca i zapisujaca znaku
void readFifo(int potok, FILE* plik){
    char buffor[1];

    while (1){
        int stan = read(potok,buffor,1);
        if(stan < 0){
            perror("Blad podczas zapisu\n");
            exit(EXIT_FAILURE);
        }
        else if(stan == 0){
            close(potok);
            break;
        }
        else
            fputc(buffor[0], plik);
    }
    
    close(potok);
    exit(EXIT_SUCCESS);
}



//Funkcja generujaca plik do zapisu dla danego procesu
FILE* generateFile(){
    //Tworzenie pliku-----------
    srand(getpid());

    char filename[256] = "wy_";
    char pid[16];
    sprintf(pid, "%d", getpid());
    strcat(filename, pid);
    strcat(filename, ".txt");

    FILE* plik = fopen(filename, "w");

    //printf("%s\n",filename);

    return plik;
}



//Funkcja sprawdzajaca poprawnosc argymetnow
void argument_checker(int argc, char *argv[], int* l_k){
    if(argc != 2){
        printf("Blad zla ilosc argumentow\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < strlen(argv[1]); i++){
        if(!isdigit(argv[1][i])){
            printf("Blad argumentow! Wartosc nie jest liczba\n");
            exit(EXIT_FAILURE);
        }
    }

    long temp_k = strtol(argv[1], NULL, 10);
    if(temp_k < 0 || temp_k > INT_MAX){
        //if(errno == ERANGE)
        printf("Blad argumentow\n");
        exit(EXIT_FAILURE);
    }

    *l_k = temp_k;
}


//Funkcja sprawdzajaca ilosc wolnych procesow
void process_checker(int l_k){
    const int size = 256;
    char buf[size];
    int liczbaProducentow;
    FILE *plik;
    if ((plik=fopen("liczbaProcesow.txt", "r")) == NULL){
        printf("Nie moge otworzyc pliku liczbaProcesow.txt do odczytu!\n");
        exit(-1);
    }
    fscanf(plik, "%d", &liczbaProducentow);
    fclose(plik);


    fp_in = popen("ps ux | grep ^4638 | wc -l", "r");			//licze uruchomione procesy
    fgets(buf,size,fp_in);
    int liczbaUruchomionychProcesow = atoi(buf);
    if(pclose(fp_in) == -1){
	printf("PClose error\n");
	exit(EXIT_FAILURE);
    }
    
    fp_in=popen("ulimit -u", "r");					//sprawdzam limit procesow 
    fgets(buf,size,fp_in);
    int limitProcesow = atoi(buf);
    if(pclose(fp_in) == -1){
        printf("PClose error\n");
        exit(EXIT_FAILURE);
    }

    printf("\tLiczba uruchomionych procesow (ps ux): %d\n\tLimit procesow (unlimit -u): %d\n\tLiczba producentow: %d\n", liczbaUruchomionychProcesow, limitProcesow, liczbaProducentow);
    
    if(limitProcesow - liczbaUruchomionychProcesow - liczbaProducentow < l_k){
        printf("Limit error: Zbyt maly limit dostepnych procesow\n");   //sprawdzam czy argumenty nie sa za duze
	printf("Limit: %d\n", limitProcesow - liczbaUruchomionychProcesow - liczbaProducentow);
        exit(2);
    }
}

