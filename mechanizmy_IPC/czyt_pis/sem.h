#include<stdio.h>
#include<stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include<sys/shm.h>
#include<unistd.h>
#include <time.h>
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