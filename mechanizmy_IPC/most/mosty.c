#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <unistd.h>
#include <time.h>

enum dir{north, south};
int semid = 0;
int n_id = 0;
int * n = NULL;


//operacje semaforowe
static struct sembuf buf;
void semV(int semid, int semnum);
void semP(int semid, int semnum);


int process(enum dir direction){
    //zwiększenie ilości samochodów
    semP(semid, 0);
        n[direction]++;
        //sytuacja gdzie samochód pojawia się jako pierwszy na moście
        if(n[north] == 0 && n[south] >= 1)
            semP(semid, 1);
        if(n[north] >= 1 && n[south] == 0)
            semP(semid, 2);   
    semV(semid, 0);
    
    //próba wjazdu na most
    semP(semid, 1 + direction);
    semV(semid, 1 + direction);
    switch (direction){
    case north:
        printf("jade na polnoc\n");
        break;    
    case south:
        printf("jade na poludnie\n");
        break;
    }

    semP(semid, 0);
        n[direction]--;
        //ostatni samochód wyjeżdża, zezwala na wiazd samochodów w drugim kierunku
        if(n[direction] <= 0)
            semV(semid, 2 - direction);
    semV(semid, 0);

    return 1;
}

int main(int argc, char *argv[]){ //wywołanie funkcji ./main [liczba samochodów jadąca na północ] [liczba samochodów jadących na południe]
    srand(time(NULL));
    if(argc < 3){
        printf("argument error\n");
        return 1;
    }
    int cars[2];
    cars[north] = atoi(argv[1]);
    cars[south] = atoi(argv[2]);

    //semafory
    semid = semget(IPC_PRIVATE, 3, IPC_CREAT | 0600);
    if(semid == -1){
        perror("semaphore creation");
        return 1;
    }
    semctl(semid, 0, SETVAL, (int)1); //semafor odpowiadający za dostęp do sekcji krytycznej zliczającej ilość samochodów jadących na północ i południe
    semctl(semid, 1, SETVAL, (int)1); //semafor blokujący jazdę na północ
    semctl(semid, 2, SETVAL, (int)1); //semafor blokujący jazdę na południe

    //pamięć współdzielona
    n_id = shmget(IPC_PRIVATE, 2 * sizeof(int), IPC_CREAT | 0666);
    //n[0] - liczba samochodów chcących jechać na północ 
    //n[1] - liczba samochodów chcących jechać  na południe
    if(n_id == -1){
        perror("shared memory creation");
        return 1;
    }   
    n = (int*)shmat(n_id, NULL, 0);
    if(n == NULL){
        perror("shared memory addition ");
        return 1;
    }
    n[north] = 0;
    n[south] = 0;


    //losowo samochodów z losowymi kierunkami
    while(0 < cars[north] + cars[south]){
        enum dir temp = rand() % 2;
        if(cars[temp] > 0){
            //printf("%d\n", temp);
            cars[temp]--;
            if(fork() == 0){
                process(temp);
                return 1;
            }
        }
    }
    return 1;
}



void semV(int semid, int semnum){
    buf.sem_num = semnum;
    buf.sem_op = 1;
    buf.sem_flg = 0;
    if(semop(semid, &buf, 1) == -1){
        perror("V sem operation");
        exit(1);
    }
}

void semP(int semid, int semnum){
    buf.sem_num = semnum;
    buf.sem_op = -1;
    buf.sem_flg = 0;
    if(semop(semid, &buf, 1) == -1){
        perror("P sem operation");
        exit(1);
    }
}