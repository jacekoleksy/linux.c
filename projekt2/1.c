//gcc -pthread 1.c
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<time.h>
#define REENTRANT
int utworz_watek;
int przylacz_watek;
int odlacz_watek;
int ilosc1;
int ilosc2;
typedef struct _Wat{
	int nr;
	int suma;
	int* tab;
}Watek;
void losowanie(int tab[2][10]){
	srand(time(NULL));
	for(int i=0; i<2; i++){
		for(int j=0; j<10; j++){
			tab[i][j]=rand()%100;
		}
	}

}
void wypisz(int tab[2][10]){
	printf("\nTablica liczb losowych:\n");
	for(int i=0; i<2; i++){
		printf("%d: ",i);
		for(int j=0; j<10; j++){
			printf("%d ",tab[i][j]);
		}
	printf("\n");
	}
	printf("\n");
}
void* suma(void* _wat){
	Watek* wat = (Watek*)_wat;
	for(int j=0; j<10; j++)		
		wat->suma+=wat->tab[j];
	//sleep(10);
	if(pthread_self()==-1){
		perror("Error self: \n");
		exit(-4);
	}
	printf("Watek %d (id: %lld) zliczyl elementy %d wiersza tablicy. Suma = %d\n",wat->nr,pthread_self(),wat->nr,wat->suma);
	pthread_exit(0);
}
void przylacz(pthread_t id_watku){
	ilosc1++;
	printf("[SYSTEM]Przylaczam watek %d...\n",ilosc1);
        przylacz_watek=pthread_join(id_watku,NULL);
        if(przylacz_watek==-1){
        	perror("Error join: \n");
        	exit(-2);
        }
}
void odlacz(pthread_t id_watku){
	ilosc2++;
	printf("[SYSTEM]Odlaczam watek %d...\n",ilosc2);
   	odlacz_watek=pthread_detach(id_watku);
   	if (odlacz_watek==-1){
		perror("Error detach: \n");
	        exit(-3);
	}
}
int main(){
	pthread_t id_watku1;
	pthread_t id_watku2;

        Watek wat1;
	wat1.nr=1;
	wat1.suma=0;

	Watek wat2;
	wat2.nr=2;
	wat2.suma=0;

	printf("\nIdentyfikator procesu: %d\n",getpid());
	int tab[2][10];
	losowanie(tab);
	wypisz(tab);

	wat1.tab=(int *)malloc(10*sizeof(int));
	wat2.tab=(int *)malloc(10*sizeof(int));
	wat1.tab=tab[0];
	wat2.tab=tab[1];
	
	printf("[SYSTEM]Tworze watek 1...\n");
        utworz_watek=pthread_create(&id_watku1,NULL,suma,&wat1);
        if(utworz_watek==-1){
	        perror("Error create: \n");
       		exit(-1);
        }
	printf("[SYSTEM]Tworze watek 2...\n");
        utworz_watek=pthread_create(&id_watku2,NULL,suma,&wat2);
        if(utworz_watek==-1){
	        perror("Error create: \n");
       		exit(-1);
        }

	przylacz(id_watku1);
	przylacz(id_watku2);
	//sleep(10);
	odlacz(id_watku1);
	odlacz(id_watku2);

	int wynik=wat1.suma+wat2.suma;
	printf("\nSuma 2ch wierzy tablicy, zliczona przez watki wynosi: %d\n\n",wynik);
	

	printf("[PROGRAM]Koncze dzialanie\n");
	exit(0);
}
