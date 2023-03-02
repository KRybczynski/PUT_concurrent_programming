#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <unistd.h>
#include <time.h>


enum direction{north, south};
int semid = 0;
int n_S_id = 0;
int n_N_id = 0;


static struct sembuf buf;

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

int process(enum direction direction){
    int* n = (int*)shmat()
}


int naPolnoc(){
    int* poludnie = (int*)shmat(n_S_id, NULL, 0);      
    if(poludnie == NULL){
        perror("shared memory addition ");
        exit(1);
    }
    semP(semid, 0);
        (*poludnie)++;
    semV(semid, 0);

    semP(semid, 2);
    semV(semid, 2);
    printf("jade na polnoc");
    
    semP(semid, 0);
        (*poludnie)--;
        if(*poludnie == 0){
            semV(semid, 1);
        }
    semV(semid, 0);
    return 1;
}

int naPoludnie(){
    int* n_N = (int*)shmat(n_N_id, NULL, 0);
    if(poludnie == NULL){
        perror("shared memory addition ");
        exit(1);
    }
    semP(semid, 1)



    return 1;
}





int main(int argc, char *argv[]){
    srand(time(NULL));
    if(argc < 2){
        exit(1);
    }

    int n = atoi(argv[1]);
    if(n < 0){
        printf("bledna ilosc samochodow");
        exit(1);
    }

    semid = semget(45282, 4 * sizeof(int), IPC_CREAT | 0600);
    semctl(semid, 0, SETVAL, (int)1);
    semctl(semid, 1, SETVAL, (int)1);
    semctl(semid, 2, SETVAL, (int)0);
    semctl(semid, 3, SETVAL, (int)1);


    n_S_id = shmget(45281, 2 * sizeof(int), IPC_CREAT | 0600);
    if(n_S_id == -1){
        perror("shared memory creation");
        exit(1);
    }
    int* n_S = (int*)shmat(n_S_id, NULL, 0);
    if(n_S == NULL){
        perror("shared memory addition ");
        exit(1);
    }
    *n_S = 0;


    n_N_id = shmget(45282, sizeof(int), IPC_CREAT | 0600);
    if(n_N_id == -1){
        perror("shared memory creation");
        exit(1);
    }
    int* n_N = (int*)shmat(n_N_id, NULL, 0);
    if(n_N == NULL){
        perror("shared memory addition ");
        exit(1);
    }
    *n_N = 0;


    /*
    for(int i = 0; i < n; i++){
        if(fork() == 0){
            naPolnoc();
            return 1;
        }
    }
    */
   naPolnoc();

    return 1;
}