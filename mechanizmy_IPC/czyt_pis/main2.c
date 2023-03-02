#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <unistd.h>
#include <time.h>

#include"sem.h"

enum mode{relax, libary};

struct buf_elem{
    long mtype;
    int mvalue;
};
struct buf_elem elem;


int semid = 0;
int read_count_id = 0;
int msg_id = 0;

int reader(int semid, int* read_count){


    semP(semid, 1);
        (*read_count)++;
        if(*read_count == 1)
            semP(semid, 0);
    semV(semid, 1);
    if (msgrcv(msg_id, &elem, sizeof(elem.mvalue), 0, IPC_NOWAIT ) == -1){
        perror("odbior komunikatu");
        exit(1);
    }
    printf("reading - %d\n", elem.mvalue);
    semP(semid, 1);
        (*read_count)--;
        if((*read_count) == 0)
            semV(semid, 0);
    semV(semid, 1);

    return 1;
}

int writer(int semid, int* read_count, int to_send){
    
    elem.mtype = 1;
    elem.mvalue = to_send;
    semP(semid, 0);
    printf("writing\n");
    if(msgsnd(msg_id, &elem, sizeof(elem.mvalue), 0) == -1){
        perror("sending message");
    }
    semV(semid, 0);
    return 1;
}

int process(int limit, int semid){
    //printf("ulala\n");
    enum mode curr = relax;    
    int* read_count = (int*)shmat(read_count_id, NULL, 0);
    if(read_count == NULL){
        perror("shared memory addition ");
        exit(1);
    }

    for(int i = 0; i < limit; i++){
        curr = (curr + 1) % 2;
        if(curr == libary){
            if(rand() % 2 == 0)
                reader(semid, read_count);
            else
                writer(semid, read_count, i);
        }
    }
    return 1;
}


int main(int argc, char *argv[]){
    int n = atoi(argv[1]);
    srand(time(NULL));
    semid = semget(45282, 2, IPC_CREAT | 0600);
    semctl(semid, 0, SETVAL, (int)1);
    semctl(semid, 1, SETVAL, (int)1);




    read_count_id = shmget(45281, sizeof(int), IPC_CREAT | 0600);
    if(read_count_id == -1){
        perror("shared memory creation");
        exit(1);
    }
    int* read_count = (int*)shmat(read_count_id, NULL, 0);
    if(read_count == NULL){
        perror("shared memory addition ");
        exit(1);
    }
    *read_count = 0;

    //message que
    msg_id = msgget(257421, IPC_CREAT | IPC_EXCL|0600);
    if(msg_id == -1){
        msg_id = msgget(257421, IPC_CREAT |0600);
        if(msg_id == -1){
            perror("message queue creation");
            exit(1);
        }        
    }
    elem.mtype = 1;
    elem.mvalue = 2137;
    if(msgsnd(msg_id, &elem, sizeof(elem.mvalue), 0) == -1){
        perror("main sending message");
    }


    for(int i = 0; i < n; i++){
        if(fork() == 0){
            process(5, semid);
            //printf("koniec\n");
            return 1;
        }
    }
    return 1;
}