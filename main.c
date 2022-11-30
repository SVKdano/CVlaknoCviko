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

}DATAZ;

//spravca
void* naplnValov(void* arg){
    //1.napln valov -> odosielam signal kobylam
    //2.cakam na zaerdzanie -> napln valov a signalizuj

    DATAZ* data = (DATAZ*)arg;
    printf("Spravca sa spustil\n");

    int zasoby = data->pocetKobyl;
    printf("Spravca 1. doplna\n");
    pthread_mutex_lock(data->mutexValov);
    usleep(1000);
    for (int i = 0; i < data->pocetPrvkov; ++i) {
        data->valov[i] = 1;
        zasoby--;
    }

    data->index = 0;
    pthread_mutex_unlock( data->mutexValov);
    //ak nejaka ostala cakat tak  tymto sa zobudza
    pthread_cond_broadcast(data->napln);
    printf("Spravca naplnil valov, ostava %d zasob\n", zasoby);

    while (zasoby > 0) {
        printf("Ide pozriet na valov\n");
        pthread_mutex_lock(data->mutexValov);

        while (data->index < data->pocetPrvkov) {
            //spravca caka pokial sa nezaerdzi
            pthread_cond_wait(data->zaergi, data->mutexValov);
        }

        printf("Spravca ide doplnit valov\n");
        usleep(1000);

        for (int i = 0; i < data->pocetPrvkov; i++) {
            data->valov[i] = 1;
            zasoby--;
        }

        data->index = 0;
        pthread_mutex_unlock(data->mutexValov);

        pthread_cond_broadcast(data->napln);
        usleep(1000);
        printf("Doplnil valovy a zasoby su %d\n", zasoby);

    }

    printf("Spravca koniec\n");
    return NULL;
}

//kobyla 1x
void* vyprazdnujValov(void* arg) {
    //1.ak je nalozene -> nazerie sa
    //2.ak je prazdny -> erdzi a caka

    DATAZ* data = (DATAZ*)arg;
    printf("Kobyla start\n");

    pthread_mutex_lock(data->mutexValov);
    while (data->index > data->pocetPrvkov-1) {
        printf("Kobyla caka\n");
        usleep(1000);
        pthread_cond_wait(data->napln, data->mutexValov);
    }
    data->valov[data->index++]=0;
    pthread_mutex_unlock(data->mutexValov);

    if (data->index == data->pocetPrvkov) {
        pthread_cond_signal(data->zaergi);
    }

}


int main() {
    int pocet = 10;
    int velkostValova = 2;
    int valov[velkostValova];

    pthread_t spravca;
    pthread_t kobyly[pocet];

    pthread_mutex_t mutex;
    pthread_cond_t cond1, cond2;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond1, NULL);
    pthread_cond_init(&cond2, NULL);

     DATAZ data = {
             velkostValova,
             pocet,
             valov,
             velkostValova,
             &mutex,
             &cond1,
             &cond2
     };

    printf("===========Zaciatok===========\n");

    //zaciatok vlakien
    pthread_create(&spravca, NULL, &naplnValov, &data);
    for (int i = 0; i < pocet; i++) {
        pthread_create(&kobyly[i], NULL, &vyprazdnujValov, &data);
    }

    //cakanie na vlakna
    pthread_join(spravca,NULL);
    for (int i = 0; i < pocet; i++){
        pthread_join(kobyly[i],NULL);
    }

    //koniec vlakien
    pthread_cond_destroy(&cond1);
    pthread_cond_destroy(&cond2);
    pthread_mutex_destroy(&mutex);
    return 0;
}
