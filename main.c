#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

typedef struct dataZrebcin {
    //konecna mnozina zasob, kapacita nadoby
    const int pocetPrvkov;
    //pocet kobyl
    const int pocetKobyl;

    int* valov;
    int index;

    pthread_mutex_t* mutexValov;
    pthread_cond_t* napln;
    pthread_cond_t* zaergi;

}THRDATA;

//spravca
void* naplnValov(void* arg){
    //1.napln valov -> odosielam signal kobylam
    //2.cakam na zaerdzanie -> napln valov a signalizuj

}

//kobyla 1x
void* vyprazdnujValov(void* arg) {
    //1.ak je nalozene -> nazerie sa
    //2.ak je prazdny -> erdzi a caka

}


int main() {
    int pocet = 10;
    int velkostValova = 2;
    int valov[velkostValova];

    pthread_t spravca;
    pthread_t kobyly[pocet];
    pthread_mutex_t mutex;

    return 0;
}
