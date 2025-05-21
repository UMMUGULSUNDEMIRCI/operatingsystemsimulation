#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/wait.h>

#define KAT_SAYISI 10
#define DAIRE_SAYISI 4

pthread_mutex_t vinç_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t asansor_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mikser_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t daire_mutex[KAT_SAYISI][DAIRE_SAYISI];
sem_t tesisat_sem[KAT_SAYISI];

void* daire_icerik_isleri(void* arg) {
    int* ids = (int*)arg;
    int kat = ids[0];
    int daire = ids[1];

    // Tesisat ve pencere/kapı işlemleri sırayla
    pthread_mutex_lock(&daire_mutex[kat][daire]);
    printf("Kat %d Daire %d: Su ve elektrik tesisatına başlandı.\n", kat+1, daire+1);
    sleep(1);
    printf("Kat %d Daire %d: Tesisat tamamlandı.\n", kat+1, daire+1);

    printf("Kat %d Daire %d: Pencere ve kapı kurulumu başladı.\n", kat+1, daire+1);
    sleep(1);
    printf("Kat %d Daire %d: Pencere ve kapı tamamlandı.\n", kat+1, daire+1);
    pthread_mutex_unlock(&daire_mutex[kat][daire]);

    // Paralel yapılabilen işlemler
    printf("Kat %d Daire %d: Alçı ve boya yapılıyor.\n", kat+1, daire+1);
    sleep(1);
    printf("Kat %d Daire %d: Zemin kaplama yapılıyor.\n", kat+1, daire+1);
    sleep(1);
    printf("Kat %d Daire %d: İç mekan düzenlemeleri yapılıyor.\n", kat+1, daire+1);
    sleep(1);

    pthread_exit(NULL);
}

void kat_islemleri(int kat) {
    // Vinç ortak kaynak - iskelet kurulumunda kullanılıyor
    pthread_mutex_lock(&vinç_mutex);
    printf("Kat %d: İskele kuruluyor (vinç kullanılıyor)...\n", kat+1);
    sleep(2);
    pthread_mutex_unlock(&vinç_mutex);

    // Beton mikseri ortak kaynak - kolon dökümü
    pthread_mutex_lock(&mikser_mutex);
    printf("Kat %d: Kolon ve beton dökümü...\n", kat+1);
    sleep(2);
    pthread_mutex_unlock(&mikser_mutex);

    printf("Kat %d: Duvarlar örülüyor...\n", kat+1);
    sleep(1);

    pthread_t daire_thread[DAIRE_SAYISI];
    int daire_id[DAIRE_SAYISI][2];

    // Her daire için mutex ve thread
    for (int i = 0; i < DAIRE_SAYISI; i++) {
        daire_id[i][0] = kat;
        daire_id[i][1] = i;
        pthread_mutex_init(&daire_mutex[kat][i], NULL);
    }

    for (int i = 0; i < DAIRE_SAYISI; i++) {
        pthread_create(&daire_thread[i], NULL, daire_icerik_isleri, (void*)&daire_id[i]);
        pthread_join(daire_thread[i], NULL); // sıralı başlatmak için join
    }
}

int main() {
    printf("Temel atılıyor...\n");
    sleep(2);
    printf("Temel tamamlandı.\n");

    pid_t pid[KAT_SAYISI];

    for (int i = 0; i < KAT_SAYISI; i++) {
        if ((pid[i] = fork()) == 0) {
            // Child process: ilgili kat işlemleri
            kat_islemleri(i);
            exit(0);
        } else {
            // Parent process
            if (i > 0)
                waitpid(pid[i - 1], NULL, 0); // önceki kat tamamlanmadan başlamasın
        }
    }

    // Tüm çocuklar tamamlanana kadar bekle
    for (int i = 0; i < KAT_SAYISI; i++) {
        waitpid(pid[i], NULL, 0);
    }

    printf("Çatı kuruluyor...\n");
    sleep(2);
    printf("Dış cephe boyanıyor...\n");
    sleep(2);
    printf("Bina inşaatı tamamlandı.\n");

    return 0;
}
