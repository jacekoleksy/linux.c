#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <ctype.h>
#include <string.h>

#define sem_w1 0
#define sem_sp 1
#define sem_sc 2
#define sem_w2 3
#define sem_w3 4

FILE *f;

static void sem_wait(int);
static void sem_sig(int);
void upd(void);
void dpd(void);
void odlacz_pamiec(void);

int semafor; //id semafora
int pamiec; //id pamieci dzielonej
int *lp; //wskaznik do pamieci dzielonej

void pisz(int pid, int nr_kol){
  f = fopen("./czytelnia.txt", "w");
  fprintf(f, "Pisarz %d, pisze. Nr petli: %d\n", pid, nr_kol);
  fclose(f);
  printf("Picarz %d, pisze.\n", pid);
  sleep(rand() % 3);
}

int main(){
  int iterator = 0;
  //Uzyskuje dostep do kolejki semaforow
  key_t key = ftok(".", 3);
  semafor = semget(key, 5, 0600);
  if (semafor == -1){
    printf("[Pisarz] Nie moglem utworzyc nowego semafora.\n");
    exit(EXIT_FAILURE);
  }

  upd(); //Utworzenie pamieci dzielonej
  dpd(); //Dolaczenie pamieci dzielonej
  int i = 10;
  while (i){
    iterator++;
    sem_wait(sem_w2); //sem_op = -1
    lp[2] += 1;

    if (lp[2] == 1)
      sem_wait(sem_sc); //sem_op = -1
    sem_sig(sem_w2); //sem_op = 1
    sem_wait(sem_sp); //sem_op = -1
    pisz(getpid(), iterator); //pisarz pisze
    sem_sig(sem_sp); //sem_op = 1
    sem_wait(sem_w2); //sem_op = -1
    lp[2] -= 1;
    if (lp[2] == 0)
      sem_sig(sem_sc); //sem_op = 1
    sem_sig(sem_w2); //sem_op = 1
    i--;
    sleep(1);
  }
  odlacz_pamiec(); //Odlaczenie pamieci dzielonej
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
//Utworzenie pamieci dzielonej
void upd(){
  key_t key = ftok(".", 6);
  pamiec = shmget(key, 3 * sizeof(int), 0600 | IPC_CREAT | IPC_EXCL);
  if (pamiec == -1){
    pamiec = shmget(key, 3 * sizeof(int), 0600);
    if (pamiec == -1){
      perror("Shmget");
      printf("[Pisarz] Problemy z utworzeniem pamieci dzielonej.\n");
      exit(EXIT_FAILURE);
    }
  }
}
//Dolaczenie pamieci dzielonej
void dpd(){
  lp = shmat(pamiec, 0, 0);
  if (*lp == -1){
    perror("Shmat");
    printf("[Pisarz] Problem z przydzieleniem adresu.\n");
    exit(EXIT_FAILURE);
  }
}
//Odlaczenie pamieci dzielonej
void odlacz_pamiec(){
  int odlaczenie2;
  odlaczenie2 = shmdt(lp);
  if (odlaczenie2 == -1){
    perror("shmdt");
    printf("[Pisarz] Problemy z odlaczeniem pamieci dzielonej.\n");
    exit(EXIT_FAILURE);
  }
}
