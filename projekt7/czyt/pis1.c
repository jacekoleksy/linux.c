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

int semafor;

FILE *f;

static void sem_wait(int);
static void sem_sig(int);

void pisz(int pid, int nr_kol){
  f = fopen("./czytelnia.txt", "w");
  fprintf(f, "Picarz %d, pisze. Nr petli: %d\n", pid, nr_kol);
  fclose(f);
  printf("Pisarz %d, pisze.\n", pid);
  //sleep(rand() % 3);
}

int main(){
  int iterator = 0;
  //Uzyskuje dostep do kolejki semaforow
  key_t key = ftok(".", 3);
  semafor = semget(key, 2, 0600);
  if (semafor == -1){
    printf("[Pisarz] Nie moglem utworzyc nowego semafora.\n");
    exit(EXIT_FAILURE);
  }
  while (1){
    iterator++;
    sem_wait(0); //sem_op = -1
    pisz(getpid(), iterator); //pisarz pisze
    sem_sig(0); //sem_op = 1
  }
}
//sem_op = -1
static void sem_wait(int nr){
  int semi;
  struct sembuf buforr;
  buforr.sem_num = nr;
  buforr.sem_op = -1;
  buforr.sem_flg = SEM_UNDO;
  semi = semop(semafor, &buforr, 1);
  if (zmien_sem == -1){
    perror("Semop");
    printf("[Czytelnik] Nie moglem otworzyc semafora.\n");
    exit(EXIT_FAILURE);
  }
}
//sem_op = 1
static void sem_sig(int nr){
  int semi;
  struct sembuf buforr;
  buforr.sem_num = nr;
  buforr.sem_op = 1;
  buforr.sem_flg = SEM_UNDO;
  semi = semop(semafor, &buforr, 1);
  if (zmien_sem == -1){
    perror("Semop");
    printf("[Czytelnik] Nie moglem otworzyc semafora.\n");
    exit(EXIT_FAILURE);
  }
}
