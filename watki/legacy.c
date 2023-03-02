#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<pthread.h>
#include <stdbool.h>


#define BUFFSIZE 10

static volatile bool buf_full;
static volatile int * buf;
static volatile int item_counter = 0;
static volatile int n_big = 0, n_small = 0;

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cnd_full = PTHREAD_COND_INITIALIZER, cnd_empty = PTHREAD_COND_INITIALIZER;

void setBuff(){
    buf = (int*)malloc(BUFFSIZE * sizeof(int));
    for(int i =0; i < BUFFSIZE; i++){
        buf[i] = 0;
    }
}

void showBuf(){
    int temp = 0;
    for(int i = 0; i < item_counter; i++){
        //temp += buf[i];
        printf("%d, ", buf[i]);
    }
    printf("\n");
    //printf("weight %d\n", temp);
}

int countBuf(){
    int sum = 0;
    for(int i = 0; i < BUFFSIZE; i++){
        sum += buf[i];
    }
    return sum;
}


void put(int package){
    item_counter++;
    for(int i = item_counter; i > 0; i--){
        buf[i] = buf[i - 1];
    }    
    buf[0] = package;
}

void take(){
    item_counter--;
    if(item_counter < 0)
        return;
    buf[item_counter] = 0;
}



void* upWorker(){
    while (true){
        pthread_mutex_lock(&mtx);
            if(n_small + n_big <= 0  && item_counter < 0){
                pthread_mutex_unlock(&mtx);
                pthread_cond_signal(&cnd_full);
                break;
            }            
            if(item_counter <= 0 && n_big + n_small > 0)        
                pthread_cond_wait(&cnd_full, &mtx);
            take();
            printf("odkładam element %d, obciazenie tasmy -> %d\n", item_counter, countBuf());
            showBuf();
            pthread_cond_signal(&cnd_empty);            
        pthread_mutex_unlock(&mtx);
    }
    //printf("koniec\n");    
}


void *downWorker(){
    while(true){
        pthread_mutex_lock(&mtx);
        if(n_big + n_small <= 0){
            pthread_mutex_unlock(&mtx);            
            break;
        }                    
        if(n_big > 0){
            if(countBuf() + 2 > 10)
                pthread_cond_wait(&cnd_empty, &mtx);
            n_big--;
            put(2);            
            printf("wkładam 2kg, ilosc elementow -> %d. Pozostalo %d duzych i %d malych paczek\n",item_counter, n_big, n_small);
            showBuf();
            pthread_cond_signal(&cnd_full);
        }
        if(n_big <= 0 && n_small > 0){   
            if(countBuf() + 1 > 10)
                pthread_cond_wait(&cnd_empty, &mtx);
            n_small--;
            put(1);     
            printf("wkładam 1kg, obciazenie tasmy -> %d. Pozostalo %d duzych i %d malych paczek\n", countBuf(), n_big, n_small);
            showBuf();
            pthread_cond_signal(&cnd_full);
        }
        pthread_mutex_unlock(&mtx);
    }
    
}


int main(int argc, char *argv[]){ //wywołanie funkcji ./main [liczba robotników na górze] [liczba robotników na dole] [liczba małych paczek] [liczba dużych paczek]
    srand(time(NULL));
    int n_up = 0, n_down = 0;
    pthread_t * up_t_id, *down_t_id;
    setBuff();
    
    if(argc < 3){
        printf("argument error\n");
        return 1;
    }
    n_up = atoi(argv[1]);
    n_down = atoi(argv[2]);
    n_small = atoi(argv[3]);
    n_big = atoi(argv[4]);

    up_t_id = (pthread_t *) malloc(n_up * sizeof(pthread_t ));
    down_t_id = (pthread_t*) malloc(n_down * sizeof(pthread_t));

    //put(2);
    //showBuf();
    //printf("%d\n", countBuf());
    
    for(int i = 0; i < n_down; i++){
        if(pthread_create(&(down_t_id[i]), NULL, downWorker, NULL) == -1){
            perror("thread creation");
        }
    }

    for(int i = 0; i < n_up; i++){
        if(pthread_create(&(up_t_id[i]), NULL, upWorker, NULL) == -1){
            perror("thread creation");
        }
    }


    for(int i = 0; i < n_up; i++)
        pthread_join(up_t_id[i], NULL);
    for(int i = 0; i < n_down; i++)
        pthread_join(down_t_id[i], NULL);
    
    return 1;
}
