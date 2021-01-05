#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>
FILE *fp_in;
int we = 0, wy = 0;

int main(int argc, char* argv[]) {
    const int size = 256;
    char buf[size];

    if(argc != 4){
        printf("Zla ilosc argumentow na wejsciu. Maja byc 3!\n");
        exit(1);
    }    

    int liczbaK = atoi(argv[1]);					//przypisanie argumentow do zmiennych
    int liczbaP = atoi(argv[2]);
    //int liczbaZnakow = atoi(argv[3]);
    int sumaProcesow = liczbaK + liczbaP;    

    errno = 0;
    long liczbaZnakow = strtol(argv[3], NULL, 10);					//naiwne sprawdzanie czy podana
    if (errno) {							//ilosc znakow nie zatrzyma programu
        perror("/t/tBlad konwersji!\n");
    } else if (liczbaZnakow < 0) {
        printf("\t\tZa mala ilosc znakow!\n");
        exit(EXIT_FAILURE);
    } else if (liczbaZnakow > 1000000) {
        printf("\t\tDuza ilosc znakow, proces moze zajac duzo czasu!\n");
        //if(liczbaZnakow > 1000000000){
	     printf("\t\tZa duza ilosc znakow, zamykam program!\n");
	     exit(EXIT_FAILURE);
	//}
    } else {
        printf("Ilosc znakow = %ld\n", liczbaZnakow);
    }

    printf("Usuwam wszystkie wczesniejsze pliki we i wy...");		//usuwamy wczesniejsze pliki we/wy
    system("rm -r w*");
    printf("\n");    

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

    if(limitProcesow - liczbaUruchomionychProcesow < sumaProcesow){
        printf("Limit error: Zbyt maly limit dostepnych procesow\n");   //sprawdzam czy argumenty nie sa za duze
	printf("Limit: %d", limitProcesow - liczbaUruchomionychProcesow);
        exit(2);
    }
    

    int id_procesu;
    int uchwyt_mac, uchwyt_pot;
    char bufor[BUFSIZ];
    int sprawdz_potok;
    int potok[2];
    int i, j;

    int zarodek;
    time_t tt;
    zarodek = time(&tt);
    
    FILE *fp;

    sprawdz_potok = pipe(potok);					//tworzenie potoku
    if(sprawdz_potok != 0){
        perror("Pipe: \n");
        exit(3);
    }

    printf("WE:\n");    
    for(i=0;i<liczbaP;i++){
        we++;
        id_procesu = fork();
        if(id_procesu == -1){
            close(potok[0]);
            close(potok[1]);
            perror("Fork: \n");
            exit(4);
        }
        else if(id_procesu == 0){
            srand(zarodek + getpid());
            close(potok[0]);
            int pid_p = getpid();
            sprintf(buf,"./we_%d.txt",pid_p);
            printf("%s\t\t%d\n", buf, we);
            fp = fopen(buf, "w");
            
            while(liczbaZnakow>0){
                int x = rand()%10+'a';					//losowanie znakow i wypelnianie pliku
                fputc(x, fp);
                uchwyt_mac = write(potok[1], &x, 1);			//zapisuje po jednym znaku do potoku
                if(uchwyt_mac == -1){
                    perror("Write: ");
                    exit(1);
                }
                liczbaZnakow--;
            }
            fclose(fp);
            if(close(potok[1]) == -1){					//zamykam plik i lacze (potok)
		printf("Close error\n");
		exit(-1);
	    }
            exit(0);
        }
    }

    sleep(1);

    printf("WY:\n");
    for(i=0;i<liczbaK;i++){
        wy++;
        id_procesu = fork();
        if(id_procesu == -1){
            close(potok[0]);
            close(potok[1]);
            perror("Fork: \n");
            exit(4);
        }
        else if(id_procesu == 0){
            close(potok[1]);
            int pid_p = getpid();
            sprintf(buf,"./wy_%d.txt",pid_p);
	    printf("%s\t\t%d\n", buf, wy);
            fp = fopen(buf, "w");
            int x,y;
            while(0!=(x=read(potok[0], &y, 1))){ 			//odczytuje po jednym znaku do potoku 
                if(x == -1){						//dopoki funkcja read nie zwroci 0 czyli
                    perror("Read: ");					//kiedy nie bedzie wiecej procesow a potok 
                    exit(1);						//jest pusty
                }
                fputc(y, fp);						//wstawiam po jednym znaku do pliku
            }
            fclose(fp);
            if(close(potok[0]) == -1){
	    	printf("Close error\n");
		exit(-1);
	    }
            exit(0);
        }
    }
    sleep(2);
    
    printf("Ilosc znakow w plikach WE:\n");
    system("wc -c we*");
    //printf("Ilosc znakow w plikach WY:\n");
    //system("wc -c wy*");
}
