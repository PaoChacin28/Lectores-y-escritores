#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

// Definición del número de lectores, escritores y operaciones totales
#define NUM_READERS 3
#define NUM_WRITERS 2
#define NUM_OPERATIONS 5

// Semáforos para controlar el acceso a la región crítica
sem_t rw_mutex; // Controla el acceso de los escritores
sem_t mutex;    // Controla el acceso a la variable read_count
int read_count = 0; // Cuenta el número de lectores activos
int total_operations = 0; // Cuenta el número total de operaciones de lectura/escritura

// Función que representa el comportamiento de los lectores
void* reader(void* arg) {
    int reader_id = *((int*)arg); // ID del lector
    while (1) {
        // Solicita leer
        printf("[Reader-%d] Requesting to read\n", reader_id);

        // Sección crítica para actualizar read_count
        sem_wait(&mutex);
        read_count++;
        if (read_count == 1) {
            sem_wait(&rw_mutex); // Si es el primer lector, bloquea a los escritores
        }
        sem_post(&mutex);

        // Sección de lectura
        printf("[Reader-%d] Reading...\n", reader_id);
        sleep(rand() % 3 + 1); // Simulación de tiempo de lectura
        printf("[Reader-%d] Finished reading\n", reader_id);

        // Sección crítica para actualizar read_count
        sem_wait(&mutex);
        read_count--;
        if (read_count == 0) {
            sem_post(&rw_mutex); // Si es el último lector, desbloquea a los escritores
        }
        sem_post(&mutex);

        // Actualiza el número total de operaciones
        sem_wait(&mutex);
        total_operations++;
        sem_post(&mutex);

        // Si se alcanzó el número máximo de operaciones, sale del bucle
        if (total_operations >= NUM_OPERATIONS) {
            break;
        }

        // Simulación de tiempo antes de la próxima lectura
        sleep(rand() % 5 + 1);
    }
    return NULL;
}

// Función que representa el comportamiento de los escritores
void* writer(void* arg) {
    int writer_id = *((int*)arg); // ID del escritor
    while (1) {
        // Solicita escribir
        printf("[Writer-%d] Requesting to write\n", writer_id);

        // Sección crítica para la escritura
        sem_wait(&rw_mutex);
        printf("[Writer-%d] Writing...\n", writer_id);
        sleep(rand() % 3 + 1); // Simulación de tiempo de escritura
        printf("[Writer-%d] Finished writing\n", writer_id);
        sem_post(&rw_mutex);

        // Actualiza el número total de operaciones
        sem_wait(&mutex);
        total_operations++;
        sem_post(&mutex);

        // Si se alcanzó el número máximo de operaciones, sale del bucle
        if (total_operations >= NUM_OPERATIONS) {
            break;
        }

        // Simulación de tiempo antes de la próxima escritura
        sleep(rand() % 5 + 1);
    }
    return NULL;
}

int main() {
    pthread_t readers[NUM_READERS], writers[NUM_WRITERS];
    int reader_ids[NUM_READERS], writer_ids[NUM_WRITERS];

    // Inicializa los semáforos
    sem_init(&rw_mutex, 0, 1);
    sem_init(&mutex, 0, 1);

    // Crea hilos de lectores
    for (int i = 0; i < NUM_READERS; i++) {
        reader_ids[i] = i + 1;
        pthread_create(&readers[i], NULL, reader, &reader_ids[i]);
    }

    // Crea hilos de escritores
    for (int i = 0; i < NUM_WRITERS; i++) {
        writer_ids[i] = i + 1;
        pthread_create(&writers[i], NULL, writer, &writer_ids[i]);
    }

    // Espera a que todos los hilos de lectores terminen
    for (int i = 0; i < NUM_READERS; i++) {
        pthread_join(readers[i], NULL);
    }

    // Espera a que todos los hilos de escritores terminen
    for (int i = 0; i < NUM_WRITERS; i++) {
        pthread_join(writers[i], NULL);
    }

    // Destruye los semáforos
    sem_destroy(&rw_mutex);
    sem_destroy(&mutex);

    return 0;
}
