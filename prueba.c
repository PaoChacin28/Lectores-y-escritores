#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define NUM_READERS 5
#define NUM_WRITERS 3

sem_t resourceAccess;  // Controla el acceso al recurso compartido
sem_t readCountAccess; // Controla el acceso a readCount
int readCount = 0;     // Contador de lectores

void *reader(void *param);
void *writer(void *param);

int main() {
    pthread_t readers[NUM_READERS], writers[NUM_WRITERS];

    // Inicializar semáforos
    sem_init(&resourceAccess, 0, 1);
    sem_init(&readCountAccess, 0, 1);

    // Crear hilos de lectores
    for (int i = 0; i < NUM_READERS; i++) {
        pthread_create(&readers[i], NULL, reader, (void*)(long)i);
    }

    // Crear hilos de escritores
    for (int i = 0; i < NUM_WRITERS; i++) {
        pthread_create(&writers[i], NULL, writer, (void*)(long)i);
    }

    // Esperar a que terminen los hilos
    for (int i = 0; i < NUM_READERS; i++) {
        pthread_join(readers[i], NULL);
    }
    for (int i = 0; i < NUM_WRITERS; i++) {
        pthread_join(writers[i], NULL);
    }

    // Destruir semáforos
    sem_destroy(&resourceAccess);
    sem_destroy(&readCountAccess);

    return 0;
}

void *reader(void *param) {
    int reader_id = (int)(long)param;

    printf("Reader %d wants to read.\n", reader_id);
    sem_wait(&readCountAccess);  // Entrar en sección crítica para modificar readCount
    readCount++;
    if (readCount == 1) {
        printf("Reader %d is the first reader, blocking writers.\n", reader_id);
        sem_wait(&resourceAccess); // Primer lector bloquea a los escritores
    }
    sem_post(&readCountAccess);  // Salir de sección crítica para modificar readCount

    // Lectura del recurso compartido
    printf("Reader %d is reading.\n", reader_id);
    sleep(1); // Simular el tiempo de lectura

    sem_wait(&readCountAccess);  // Entrar en sección crítica para modificar readCount
    readCount--;
    if (readCount == 0) {
        printf("Reader %d is the last reader, releasing writers.\n", reader_id);
        sem_post(&resourceAccess); // Último lector permite a los escritores
    }
    sem_post(&readCountAccess);  // Salir de sección crítica para modificar readCount

    printf("Reader %d has finished reading.\n", reader_id);
    return NULL;
}

void *writer(void *param) {
    int writer_id = (int)(long)param;

    printf("Writer %d wants to write.\n", writer_id);
    sem_wait(&resourceAccess); // Esperar acceso exclusivo al recurso

    // Escritura en el recurso compartido
    printf("Writer %d is writing.\n", writer_id);
    sleep(1); // Simular el tiempo de escritura

    sem_post(&resourceAccess); // Liberar el recurso
    printf("Writer %d has finished writing.\n", writer_id);

    return NULL;
}
