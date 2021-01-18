#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <sys/shm.h>

FILE *fp_in;

static void ustaw_semafor(int);
static void usun_semafor(int);
void upd(void);
void dpd(void);
void odlacz_pamiec(void);

int semid; //id semafora
int pamiec; //id pamieci dzielonej
int *adres; //wskaznik do pamieci dzielonej
//Funkcja wywolana po SIGINT
void sighandler(int signum, siginfo_t *info, void *ptr){
  if (signum == 2){
    printf("[Program glowny] Sygnal SIGINT!\n");
    usun_semafor(semid);
    odlacz_pamiec();
    exit(0);
  }
}

int main(int argc, char *argv[]){
  //Obsluga sygnalu
  struct sigaction sigg;
  sigg.sa_sigaction = sighandler;
  sigaction(SIGINT, &sigg, NULL);
  //Sprawdzanie ilosci argumentow
  if (argc != 4){
    printf("[Program glowny] Za malo argumentow na wejsciu\n");
    exit(1);
  }
  //Sprawdzanie poprawnosci argumentow
  char *test;
    long liczbaPis = strtol(argv[1], &test, 10);
  if (errno == ERANGE){
    printf("[Program glowny] Nieprawidlowy zakres danych\n");
    exit(2);
  }
  //else if (test == argv[1]){
    //printf("[Program glowny] Podano bledny format zmiennej (inne niz liczba)");
    //exit(2);
  //}
  for (int i = 0; i < strlen(argv[1]); i++){
    if(!isdigit(argv[1][i])){
      printf("[Program glowny] Blad argumentow! Wartosc nie jest liczba\n");
      exit(EXIT_FAILURE);
    }
  }

  long liczbaCzyt = strtol(argv[2], &test, 10);
  if (errno == ERANGE){
    printf("[Program glowny] Nieprawidlowy zakres danych\n");
    exit(2);
  }
  for (int i = 0; i < strlen(argv[2]); i++){
    if(!isdigit(argv[2][i])){
      printf("[Program glowny] Blad argumentow! Wartosc nie jest liczba\n");
      exit(EXIT_FAILURE);
    }
  }


  long liczbaMiej = strtol(argv[3], &test, 10);
  if (errno == ERANGE){
    printf("[Program glowny] Nieprawidlowy zakres danych\n");
    exit(2);
  }
  for (int i = 0; i < strlen(argv[3]); i++){
    if(!isdigit(argv[3][i])){
      printf("[Program glowny] Blad argumentow! Wartosc nie jest liczba\n");
      exit(EXIT_FAILURE);
    }
  }

  const int size = 256;
  char buf[size];
  //Sprawdzam limity i uruchomione procesy
  fp_in = popen("ps ux | grep ^4638 | wc -l", "r");
  fgets(buf, size, fp_in);
  long liczbaUruchomionychProcesow = strtol(buf, NULL, 10);
  if (errno == ERANGE){
    printf("[Program glowny] Blad pobrania uruchommionych procesow\n");
    exit(2);
  }
  pclose(fp_in);

  fp_in = popen("ulimit -u", "r");
  fgets(buf, size, fp_in);
  long limit = strtol(buf, NULL, 10);
  if (errno == ERANGE){
    printf("[Program glowny] Blad pobrania limitu procesow\n");
    exit(2);
  }
  pclose(fp_in);

  if ((limit - liczbaUruchomionychProcesow) < (liczbaPis + liczbaCzyt)){
    printf("[Program glowny] Limit error: Zbyt maly limit dostepnych procesow\n");
    printf("[Program glowny] Limit: %d\n", limit - liczbaUruchomionychProcesow);
    exit(2);
  }
  //Tworze i uzyskuje dostep do kolejki semaforow
  key_t key = ftok(".", 3);
  semid = semget(key, 2, IPC_CREAT | 0600);
  if (semid == -1){
    perror("Semget");
    printf("[Program glowny] Nie udalo sie utworzyc semaformow");
    exit(3);
  }

  ustaw_semafor(semid); //Ustawienie - Inicjowanie semaforow
  upd(); //Utworzenie pamieci dzielonej
  dpd(); //Dolaczenie pamieci dzielonej
  adres[0] = 0;
  adres[1] = liczbaMiej;

  int zarodek, i = 0;
  time_t tt;
  zarodek = time(&tt);

  for (; i < liczbaPis; i++){
    int id_procesu = fork();
    switch (id_procesu){
    case -1:
      perror("Fork");
      usun_semafor(semid);
      odlacz_pamiec();
      exit(2);

    case 0:{
      int pidExecl = execl("./p", "", NULL);
      if (pidExecl < 0){
        perror("Execl failed");
        usun_semafor(semid);
        exit(1);
      }
    }
    }
    //sleep(rand() % 3);
  }

  for (i = 0; i < liczbaCzyt; i++){
    int id_procesu = fork();
    switch (id_procesu){
    case -1:
      perror("Fork");
      usun_semafor(semid);
      odlacz_pamiec();
      exit(2);

    case 0:{
      int pidExecl = execl("./c", "", NULL);
      if (pidExecl < 0){
        perror("Execl failed");
        usun_semafor(semid);
        exit(1);
      }
    }
    }
    //sleep(rand() % 3); //
  }

  while (1)
    ;
}
//Ustawienie - Inicjowanie semaforow
static void ustaw_semafor(int semafor){
  int ustaw_sem;
  for (int i = 0; i < 2; i++){
    ustaw_sem = semctl(semafor, i, SETVAL, 1); 
    if (ustaw_sem == -1){
      printf("[Program glowny] Nie mozna ustawic semafora.\n");
      exit(EXIT_FAILURE);
    }
    else{
      printf("[Program glowny] Semafor numer %d zostal ustawiony.\n", i);
    }
  }
}
//Usuwanie semaforow
static void usun_semafor(int k){
  int sem;
  sem = semctl(k, 0, IPC_RMID);
  if (sem == -1){
    perror("Semctl");
    printf("[Program glowny] Nie mozna usunac semafora \n");
    exit(EXIT_FAILURE);
  }
  else{
    printf("[Program glowny] Semafory zostaly usuniete : %d\n", sem);
  }
}
//Utworzenie pamieci dzielonej
void upd(){
  key_t key = ftok(".", 6);
  pamiec = shmget(key, 2 * sizeof(int), 0600 | IPC_CREAT | IPC_EXCL);
  if (pamiec == -1){
    pamiec = shmget(key, 2 * sizeof(int), 0700);
    if (pamiec == -1){
      perror("Shmget");
      printf("[Program glowny] Problemy z utworzeniem pamieci dzielonej.\n");
      usun_semafor(semid);
      exit(EXIT_FAILURE);
    }
  }
  else
    printf("[Program glowny] Pamiec dzielona zostala utworzona: %d\n", pamiec);
}
//Dolaczenie pamieci dzielonej
void dpd(){
  adres = shmat(pamiec, 0, 0);
  if (*adres == -1){
    perror("Shmat");
    printf("[Program glowny] Problem z przydzieleniem adresu.\n");
    usun_semafor(semid);
    exit(EXIT_FAILURE);
  }
  else
    printf("[Program glowny] Przestrzen adresowa zostala przyznana.\n");
}
//Odlaczenie pamieci dzielonej
void odlacz_pamiec(){
  int odlaczenie1;
  int odlaczenie2;
  odlaczenie1 = shmctl(pamiec, IPC_RMID, 0);
  odlaczenie2 = shmdt(adres);
  if (odlaczenie1 == -1 || odlaczenie2 == -1){
    perror("Shmctl|shmdt");
    printf("[Program glowny] Problemy z odlaczeniem pamieci dzielonej.\n");
    exit(EXIT_FAILURE);
  }
  else
    printf("[Program glowny] Pamiec dzielona zostala odlaczona.\n");
}
