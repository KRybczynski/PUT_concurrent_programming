#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <unistd.h>
#include <time.h>

enum direction {
    north, south
};
int semid = 0;
int counter_id = 0;
int * counter = NULL;

// odpalenie programu gcc most.c -o program
// program.out [ilosc aut N] [ilosc aut S]

//operacje semaforowe
static struct sembuf buf;
void semV(int semid, int s_num);
void semP(int semid, int s_num);

void semV(int semid, int s_num) {
    buf.sem_num = s_num;
    buf.sem_op = 1;
    buf.sem_flg = 0;
    if (semop(semid, &buf, 1) == -1) {
        perror("V operation");
        exit(1);
    }
}

void semP(int semid, int s_num) {
    buf.sem_num = s_num;
    buf.sem_op = -1;
    buf.sem_flg = 0;
    if (semop(semid, &buf, 1) == -1) {
        perror("P operation");
        exit(1);
    }
}


int driving(enum direction direction) {
    for(int bruh = 0; bruh < 3; bruh++) {
        //zwiększenie ilości samochodów
        semP(semid, 0); // blokowanie semafora odpowiedzialnego za dostep do sekcji krytycznej
        // W przypadku gdy zaden samochod nie jest obecnie na moscie, proces blokuje semafor odpowiedni dla
        // kierunku, w ktorym samochod sie porusza
        counter[direction]++;
        if (counter[north] == 0 && counter[south] == 1) //samochód pojawia się jako pierwszy na moście
            semP(semid, 1); // proces blokuje semafor odpowiadajacy za ruch w danym kierunku
        if (counter[north] == 1 && counter[south] == 0) //samochód pojawia się jako pierwszy na moście
            semP(semid, 2); // proces blokuje semafor odpowiadajacy za ruch w danym kierunku

        semV(semid, 0);

        //próba wjazdu na most
        semP(semid, 1 + direction);
        semV(semid, 1 + direction); //wjezdza i mowi kolejnemu ze moze wjechac
        switch (direction) {
            case north:
                printf("Przejazd na polnoc\n");
                break;
            case south:
                printf("Przejazd na poludnie\n");
                break;
        }

        semP(semid, 0); // blokuje aby operowac na liczniku
        counter[direction]--;
        //ostatni samochód wyjeżdża, zezwala na wjazd samochodów w drugim kierunku
        if (counter[direction] == 0)
            semV(semid, 2 - direction); // jezeli jest ostatnim samochodem to podnosi dla przeciwnego kierunku
        semV(semid, 0);  // podnosi aby mogl modyfikowac sekcje krytyczna


    }
    return 1;
}

int main(int argc,
         char **argv) { 
    srand(time(NULL));
    if (argc < 3) {
        printf("argument error\n");
        return 1;
    }
    int cars[2];
    cars[north] = atoi(argv[1]);
    cars[south] = atoi(argv[2]);

    //semafory
    semid = semget(IPC_PRIVATE, 3, IPC_CREAT | 0600);
    if (semid == -1) {
        perror("semaphore creation");
        return 1;
    }
    semctl(semid, 0, SETVAL, 1); //semafor odpowiadający za dostęp do sekcji krytycznej zliczającej ilość samochodów jadących na północ i południe
    semctl(semid, 1, SETVAL, 1); //semafor blokujący jazdę na północ
    semctl(semid, 2, SETVAL, 1); //semafor blokujący jazdę na południe

    //pamięć współdzielona
    counter_id = shmget(IPC_PRIVATE, 2 * sizeof(int), IPC_CREAT | 0666);
    //n[0] - samochody jadące na północ
    //n[1] - samochody jadące na południe
    if (counter_id == -1) {
        perror("shared memory creation");
        return 1;
    }
    counter = (int *) shmat(counter_id, NULL, 0);
    if (counter == NULL) {
        perror("shared memory addition ");
        return 1;
    }
    counter[north] = 0;
    counter[south] = 0;

    for(int i = 0; i < cars[south]; i++){ // puszczanie procesow dla aut jadacych na poludnie
        if(fork() == 0){
            driving(south);
            return 1;
        }
    }

    for(int i = 0; i < cars[north]; i++){ // puszczanie procesow dla aut jadacych na poludnie
        if(fork() == 0){
            driving(north);
            return 1;
        }
    }

    return 1;
}




