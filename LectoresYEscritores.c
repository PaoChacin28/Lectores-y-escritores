#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
//hola alex 
#define NUM_READERS 2
#define NUM_WRITERS 4
#define NUM_OPERATIONS 5

sem_t rw_mutex;
sem_t mutex;
int read_count = 0;
int total_operations = 0;

void* reader(void* arg) {
    int reader_id = *((int*)arg);
    while (1) {
        printf("[Lector-%d] Ha solicitado leer\n", reader_id);

        sem_wait(&mutex);
        read_count++;
        if (read_count == 1) {
            sem_wait(&rw_mutex);
        }
        sem_post(&mutex);

        printf("[Lector-%d] Leyendo...\n", reader_id);
        sleep(rand() % 3 + 1); // Simulación de tiempo de lectura
        printf("[Lector-%d] Ha terminado de leer\n", reader_id);

        sem_wait(&mutex);
        read_count--;
        if (read_count == 0) {
            sem_post(&rw_mutex);
        }
        sem_post(&mutex);

        sem_wait(&mutex);
        total_operations++;
        sem_post(&mutex);

        if (total_operations >= NUM_OPERATIONS) {
            break;
        }

        sleep(rand() % 5 + 1); // Simulación de tiempo antes de la próxima lectura
    }
    return NULL;
}

void* writer(void* arg) {
    int writer_id = *((int*)arg);
    while (1) {
        printf("[Escritor-%d] Ha solicitado escribir\n", writer_id);

        sem_wait(&rw_mutex);
        printf("[Escritor-%d] Escribiendo...\n", writer_id);
        sleep(rand() % 3 + 1); // Simulación de tiempo de escritura
        printf("[Escritor-%d] Ha terminado de escribir\n", writer_id);
        sem_post(&rw_mutex);

        sem_wait(&mutex);
        total_operations++;
        sem_post(&mutex);

        if (total_operations >= NUM_OPERATIONS) {
            break;
        }

        sleep(rand() % 5 + 1); // Simulación de tiempo antes de la próxima escritura
    }
    return NULL;
}

int main() {
    pthread_t readers[NUM_READERS], writers[NUM_WRITERS];
    int reader_ids[NUM_READERS], writer_ids[NUM_WRITERS];

    sem_init(&rw_mutex, 0, 1);
    sem_init(&mutex, 0, 1);

    for (int i = 0; i < NUM_READERS; i++) {
        reader_ids[i] = i + 1;
        pthread_create(&readers[i], NULL, reader, &reader_ids[i]);
    }

    for (int i = 0; i < NUM_WRITERS; i++) {
        writer_ids[i] = i + 1;
        pthread_create(&writers[i], NULL, writer, &writer_ids[i]);
    }

    for (int i = 0; i < NUM_READERS; i++) {
        pthread_join(readers[i], NULL);
    }

    for (int i = 0; i < NUM_WRITERS; i++) {
        pthread_join(writers[i], NULL);
    }

    sem_destroy(&rw_mutex);
    sem_destroy(&mutex);

    return 0;
}
