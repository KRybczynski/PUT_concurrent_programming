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
static pthread_cond_t cnd_full = PTHREAD_COND_INITIALIZER, cnd_empty_big = PTHREAD_COND_INITIALIZER, cnd_empty_small = PTHREAD_COND_INITIALIZER;

void setBuff(){
    buf = (int*)malloc(BUFFSIZE * sizeof(int));
    for(int i =0; i < BUFFSIZE; i++){
        buf[i] = 0;
    }
}

void showBuf(){
    int temp = 0;
    for(int i = 0; i < BUFFSIZE; i++){
        //temp += buf[i];
        printf("%d, ", buf[i]);
    }
    printf("\n");
    //printf("weight %d\n", temp);
}

//funkcja licząca wagę paczek na taśmie
int countBuf(){
    int sum = 0;
    for(int i = 0; i < BUFFSIZE; i++){
        sum += buf[i];
    }
    return sum;
}

//funkcja dodająca paczkę na taśmę
void put(int package){
    item_counter++;
    for(int i = item_counter; i > 0; i--){
        buf[i] = buf[i - 1];
    }    
    buf[0] = package;
}

//funkcja usuwająca ostatni element z taśmy
int take(){
    int out = 0 ;
    item_counter--;
    if(item_counter < 0)
        return out;
    out = buf[item_counter]; 
    buf[item_counter] = 0;
    return out;
}



void* upWorker(){
    auto int temp = 0;
    while (true){
        //wejście do sekcji krytycznej - stanu taśmy
        pthread_mutex_lock(&mtx);
            //warunek zakończenia pracy
            if(n_small + n_big <= 0  && item_counter < 0){
                pthread_cond_signal(&cnd_full);
                pthread_mutex_unlock(&mtx);
                break;
            }
            //oczekiwanie na kolejne paczki   
            if(item_counter <= 0 && n_big + n_small > 0)        
                pthread_cond_wait(&cnd_full, &mtx);
            if(n_small + n_big <= 0  && item_counter < 0){ //warunek zakończenia pracy
                pthread_cond_signal(&cnd_full);
                pthread_mutex_unlock(&mtx);
                break;
            }
            //zjęcie paczki z taśmy
            temp = take();
            printf("odkładam element o indeksie %d, obciazenie tasmy -> %d\n", item_counter, countBuf()); fflush(stdout);
            showBuf();
            //sygnalizacja zdjęcia paczki dla wątków oczekujących na miejsce, w zależnośći od tego ile miejsca się zwolniło
            if(temp == 2 && n_big > 0)
                pthread_cond_signal(&cnd_empty_big);
            else
                pthread_cond_signal(&cnd_empty_small);
        //wyjście z sekcji krytycznej 
        pthread_mutex_unlock(&mtx);
    }
}

void *downWorker(){
    while(true){
        pthread_mutex_lock(&mtx); //wejscie do sekcji krytycznej - edycja stanu taśmy
        if(n_big + n_small <= 0){ //warunek zakonczenia pracy
                pthread_mutex_unlock(&mtx);          
                break;
        }
        if(n_big > 0){ //załadowanie większej paczki
            if(countBuf() + 2 > 10)
                pthread_cond_wait(&cnd_empty_big, &mtx);
            if(n_big + n_small <= 0){ //warunek zakonczenia pracy
                pthread_mutex_unlock(&mtx);            
                break;
            }
            n_big--;
            put(2);
            printf("wkładam 2kg, ilosc elementow -> %d. Pozostalo %d duzych i %d malych paczek\n",item_counter, n_big, n_small); fflush(stdout);
            showBuf();
            pthread_cond_signal(&cnd_full);
        } else if(n_small > 0){ //załadowanie mniejszej paczki
            if(countBuf() + 1 > 10)
                pthread_cond_wait(&cnd_empty_small, &mtx);
            
            if(n_big + n_small <= 0){
                pthread_mutex_unlock(&mtx);            
                break;
            }
            n_small--;
            put(1);
            printf("wkładam 1kg, obciazenie tasmy -> %d. Pozostalo %d duzych i %d malych paczek\n", countBuf(), n_big, n_small); fflush(stdout);
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
    //utworzenie tablicy z id wątków
    up_t_id = (pthread_t *) malloc(n_up * sizeof(pthread_t ));
    down_t_id = (pthread_t*) malloc(n_down * sizeof(pthread_t));

    //wątki realizujące robotników na dole
    for(int i = 0; i < n_down; i++){
        if(pthread_create(&(down_t_id[i]), NULL, downWorker, NULL) == -1){
            perror("thread creation");
        }
    }
    //wątki realizujące robotników na górze
    for(int i = 0; i < n_up; i++){
        if(pthread_create(&(up_t_id[i]), NULL, upWorker, NULL) == -1){
            perror("thread creation");
        }
    }


    //czekanie na zakończenie wszystkich wątków
    for(int i = 0; i < n_up; i++)
        pthread_join(up_t_id[i], NULL);
    for(int i = 0; i < n_down; i++)
        pthread_join(down_t_id[i], NULL);
    
    return 1;
}
