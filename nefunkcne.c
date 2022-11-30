#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

typedef struct ThrData{

    const int pocetKobyl;
    const int kapacitaNadoby;

    int zasoby;
    int indexKobyly;
    int stavNadoby;

    pthread_mutex_t * mutex;
    pthread_cond_t * cond_spravca;
    pthread_cond_t * cond_kobyly;

}ThrData;

void* spravcaWork(void* sdata){
    ThrData* data = (ThrData*) sdata;
    while (data->zasoby > 0){
        //caka kym bude valov prazdny
        pthread_mutex_lock(data->mutex);
        while(data->stavNadoby > 0){
            pthread_cond_wait(data->cond_spravca, data->mutex);
        }
        pthread_mutex_unlock(data->mutex);
        fprintf(stderr,"Spravca ide doplnit nadobu\n");
        usleep(500);
        //doplni nadobu
        fprintf(stderr,"Spravca doplnil nadobu\n");
        usleep(500);
        pthread_mutex_lock(data->mutex);
        data->stavNadoby=data->kapacitaNadoby;
        data->zasoby-=data->kapacitaNadoby;
        pthread_mutex_unlock(data->mutex);

        //zavola cond_kobyly, ze valov je plny
        fprintf(stderr,"Spravca zobudza kobyly\n");
        usleep(500);
        pthread_cond_broadcast(data->cond_kobyly);

    }


    return NULL;
}
void* kobylaWork(void* sdata){
    ThrData* data = (ThrData*) sdata;
    pthread_mutex_lock(data->mutex);
    int indexKobyly = data->indexKobyly++;
    pthread_mutex_unlock(data->mutex);
    fprintf(stderr,"Kobyla %d vytvorena\n", indexKobyly);
    usleep(500);

    while (data->zasoby > 0){
        //ak je nadoba prazdna, tak cakaj
        pthread_mutex_lock(data->mutex);
        while(data->stavNadoby == 0){
            pthread_cond_wait(data->cond_kobyly, data->mutex);
        }
        pthread_mutex_unlock(data->mutex);
        //najedz sa
        fprintf(stderr,"Kobyla %d sa najedla \n", indexKobyly);
        usleep(500);
        pthread_mutex_lock(data->mutex);
        data->stavNadoby--;
        pthread_mutex_unlock(data->mutex);

        // ak je nadoba prazdna, tak zavolaj spravcu
        pthread_cond_signal(data->cond_spravca);
    }

    return NULL;
}



int main() {
    fprintf(stderr,"Main zacal\n");

    pthread_mutex_t  mutex;
    pthread_cond_t  cond_spravca, cond_kobyly;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_spravca, NULL);
    pthread_cond_init(&cond_kobyly, NULL);

    ThrData data = {
            10,
            19,
            200,
            1,
            20,
            &mutex,
            &cond_spravca,
            &cond_kobyly
    };

    pthread_t spravca;
    pthread_t kobyly[data.pocetKobyl];

    pthread_create(&spravca, NULL, &spravcaWork,&data);
    for (int i = 0; i < data.pocetKobyl; ++i) {
        pthread_create(&kobyly[i], NULL, &kobylaWork,&data);
    }


    pthread_join(spravca, NULL);
    for (int i = 0; i < data.pocetKobyl; ++i) {
        pthread_join(kobyly[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_spravca);
    pthread_cond_destroy(&cond_kobyly);
    return 0;
}