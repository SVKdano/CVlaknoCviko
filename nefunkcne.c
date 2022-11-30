#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

typedef struct ThrData {
    //v zrebcine je niekolko kobyl
    const int pocetKobyl;

    //ked vyhladne ide sa najest k valovu
    const int kapacitaNadoby;

    int zasoby;
    int indexKobyly;
    int stavNadoby;

    pthread_mutex_t* mutex;
    pthread_cond_t* cond_spravca;
    pthread_cond_t* cond_kobyly;

}THRDATA;

void* spravcaWork(void* sData) {
    //dostane data v strukture ktoru musim pretypovat
    THRDATA* data = (THRDATA*)sData;

    while (data->zasoby > 0) {
        //cakanie na vyprazdnenie valova -> zamknem mutex aby ho nemohol zmenit
        pthread_mutex_lock(data->mutex);

        while (data->stavNadoby > 0) {
            //pokial nema co robit tak caka
            pthread_cond_wait(data->cond_spravca, data->mutex);
        }

        //presiel podmienku kedy je nieco vo valove (je prazdny) -> ide ho doplnat -> musi mat k nemu pristup
        pthread_mutex_unlock(data->mutex);
        fprintf(stderr, "Spravca ide doplnit nadobu\n");
        usleep(1000);

        pthread_mutex_lock(data->mutex);
        //doplnanie nadoby
        data->stavNadoby = data->kapacitaNadoby;
        data->zasoby -= data->kapacitaNadoby;
        pthread_mutex_unlock(data->mutex);
        fprintf(stderr, "Spravca doplnil nadobu\n");

        usleep(500);
        fprintf(stderr, "Spravca ide zobudit kobyly\n");
        //zobudzanie kobyl
        pthread_cond_broadcast(data->cond_kobyly);
    }

    return NULL;
};

void* kobylaWork(void* kData) {
    THRDATA* data = (THRDATA*)kData;

    pthread_mutex_lock(data->mutex);
    //vytvaranie kobyl
    int indexKobyly = data->indexKobyly++;
    pthread_mutex_unlock(data->mutex);
    fprintf(stderr, "Kobyla %d bola vytvorena\n", data->indexKobyly);
    usleep(500);

    while (data->zasoby > 0) {

        pthread_mutex_lock(data->mutex);
        //ak je nadoba prazdna tak cakaj
        while (data->stavNadoby == 0) {
            pthread_cond_wait(data->cond_kobyly, data->mutex);
        }
        pthread_mutex_unlock(data->mutex);

        fprintf(stderr, "Kobyla %d sa najedla\n", data->indexKobyly);
        usleep(500);
        pthread_mutex_lock(data->mutex);
        data->stavNadoby--;
        pthread_mutex_unlock(data->mutex);

        //zalarmuje spravcu ze je prazdno
        pthread_cond_signal(data->cond_spravca);
    }
}

static int main() {
    pthread_mutex_t mutex;
    pthread_cond_t cond_spravca, cond_kobyly;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_spravca, NULL);
    pthread_cond_init(&cond_kobyly, NULL);

    THRDATA data = {
            10,
            20,
            200,
            1,
            20,
            &mutex,
            &cond_spravca,
            &cond_kobyly

    };

    pthread_t spravca;
    pthread_t kobyly[data.pocetKobyl];

    pthread_create(&spravca, NULL, &spravcaWork, &data);
    for (int i = 0; i < data.pocetKobyl; i++) {
        pthread_create(&kobyly[i], NULL, &kobylaWork, &data);
    }

    pthread_join(spravca, NULL);
    for (int i = 0; i < data.pocetKobyl;i++) {
        pthread_join(kobyly[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_spravca);
    pthread_cond_destroy(&cond_kobyly);
    return 0;
}