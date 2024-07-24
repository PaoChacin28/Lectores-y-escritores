#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Variables Globales */
typedef struct datos {
    char nombre[20];
    char apellido[20];
    int edad;
} Datos;

unsigned int num_escritores, num_lectores;
FILE *baseDatos;
sem_t mutex, db;
int rc = 0;

/* Prototipos de Funciones */
void *lector(void *p);
void *escritor(void *p);
void escribirenbasedatos();
void leerenbasedatos();
void inicializar_semaforos();
void destruir_semaforos();
void abrir_base_datos();

int main() {
    printf("Ingrese el número de escritores: ");
    scanf("%u", &num_escritores);
    printf("Ingrese el número de lectores: ");
    scanf("%u", &num_lectores);

    int eid[num_escritores], lid[num_lectores];
    pthread_t escritora[num_escritores], lectora[num_lectores];

    abrir_base_datos();
    inicializar_semaforos();

    for (unsigned int i = 0; i < num_lectores; i++) {
        lid[i] = i;
        if (pthread_create(&lectora[i], NULL, lector, (void *)&lid[i]) != 0) {
            perror("Error al crear hilo lector");
            exit(EXIT_FAILURE);
        }
    }

    for (unsigned int i = 0; i < num_escritores; i++) {
        eid[i] = i;
        if (pthread_create(&escritora[i], NULL, escritor, (void *)&eid[i]) != 0) {
            perror("Error al crear hilo escritor");
            exit(EXIT_FAILURE);
        }
    }

    for (unsigned int i = 0; i < num_lectores; i++) {
        pthread_join(lectora[i], NULL);
    }
    for (unsigned int i = 0; i < num_escritores; i++) {
        pthread_join(escritora[i], NULL);
    }

    destruir_semaforos();
    return 0;
}

void abrir_base_datos() {
    baseDatos = fopen("baseDatos.txt", "a+");
    if (baseDatos == NULL) {
        perror("Error al abrir la base de datos");
        exit(EXIT_FAILURE);
    }
    fclose(baseDatos);
}

void inicializar_semaforos() {
    sem_init(&mutex, 0, 1);
    sem_init(&db, 0, 1);
}

void destruir_semaforos() {
    sem_destroy(&mutex);
    sem_destroy(&db);
}

void escribirenbasedatos() {
    Datos p;
    printf("Introduce el nombre: ");
    scanf("%19s", p.nombre);
    printf("Introduce el apellido: ");
    scanf("%19s", p.apellido);
    printf("Introduce la edad: ");
    scanf("%d", &p.edad);

    baseDatos = fopen("baseDatos.txt", "a");
    if (baseDatos == NULL) {
        perror("Error al abrir la base de datos para escritura");
        return;
    }

    fprintf(baseDatos, "%s\n%s\n%d\n", p.nombre, p.apellido, p.edad);
    fclose(baseDatos);
}

void leerenbasedatos() {
    Datos p;
    baseDatos = fopen("baseDatos.txt", "r");
    if (baseDatos == NULL) {
        perror("Error al abrir la base de datos para lectura");
        return;
    }

    while (fscanf(baseDatos, "%19s %19s %d", p.nombre, p.apellido, &p.edad) == 3) {
        printf("\nNombre: %s\nApellido: %s\nEdad: %d\n", p.nombre, p.apellido, p.edad);
    }

    fclose(baseDatos);
}

void *escritor(void *p) {
    int *id_es = (int *)p;
    while (1) {
        sleep(rand() % 5 + 1); // Simula tiempo de espera entre escrituras
        sem_wait(&db);
        printf("\nEscritor %d Escribiendo...\n", *id_es);
        escribirenbasedatos();
        printf("Escritor %d ha terminado de escribir.\n", *id_es);
        sem_post(&db);
    }
    pthread_exit(NULL);
}

void *lector(void *p) {
    int *id_le = (int *)p;
    while (1) {
        sleep(rand() % 5 + 1); // Simula tiempo de espera entre lecturas
        printf("\nLector %d solicitando leer...\n", *id_le);
        sem_wait(&mutex);
        rc++;
        if (rc == 1)
            sem_wait(&db); // Bloquear escritura mientras haya lectores
        sem_post(&mutex);

        printf("\nLector %d Leyendo...\n", *id_le);
        leerenbasedatos();

        sem_wait(&mutex);
        rc--;
        if (rc == 0)
            sem_post(&db); // Permitir escritura si no hay lectores
        sem_post(&mutex);
    }
    pthread_exit(NULL);
}
