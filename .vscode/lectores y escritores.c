#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Variables Globales */
int rc = 0; // Contador de lectores activos
unsigned int num_escritores, num_lectores;
FILE *baseDatos;

/* Semáforos */
sem_t mutex, db;

/* Estructura de Datos para la Base de Datos */
typedef struct datos {
    char nombre[20];
    char apellido[20];
    int edad;
} Datos;

/* Prototipos de Funciones */
void *lector(void *p);
void *escritor(void *p);
void escribirenbasedatos();
void leerenbasedatos();

int main(int argc, char *argv[]) {
    int estado, i;
    if (argc < 3) {
        printf("Número de parámetros incorrecto. Indicar el número de escritores y lectores.\n");
        exit(1);
    }
    
    num_escritores = atoi(argv[1]);
    num_lectores = atoi(argv[2]);

    int eid[num_escritores], lid[num_lectores];

    /* Declaración de los hilos */
    pthread_t escritora[num_escritores], lectora[num_lectores];

    /* Inicialización de Semáforos */
    sem_init(&mutex, 0, 1);
    sem_init(&db, 0, 1);

    /* Creación de hilos lectores */
    for (i = 0; i < num_lectores; i++) {
        lid[i] = i;
        if ((estado = pthread_create(&lectora[i], NULL, lector, (void *) &lid[i]))) {
            exit(estado);
        }
    }

    /* Creación de hilos escritores */
    for (i = 0; i < num_escritores; i++) {
        eid[i] = i;
        if ((estado = pthread_create(&escritora[i], NULL, escritor, (void *) &eid[i]))) {
            exit(estado);
        }
    }

    /* Esperar a que terminen todos los hilos */
    for (i = 0; i < num_lectores; i++) {
        pthread_join(lectora[i], NULL);
    }
    for (i = 0; i < num_escritores; i++) {
        pthread_join(escritora[i], NULL);
    }

    /* Destrucción de los semáforos */
    sem_destroy(&mutex);
    sem_destroy(&db);

    return 0;
}

void escribirenbasedatos() {
    extern FILE *baseDatos;
    Datos p;
    baseDatos = fopen("baseDatos.txt", "a+");
    if (!baseDatos) {
        printf("Error al abrir el archivo para escribir.\n");
        return;
    }

    printf("Escriba nombre, apellido y edad. Escriba 'fin' para terminar:\n");
    while (1) {
        printf("Nombre: ");
        scanf("%s", p.nombre);
        if (strcmp(p.nombre, "fin") == 0) {
            break;
        }

        printf("Apellido: ");
        scanf("%s", p.apellido);
        if (strcmp(p.apellido, "fin") == 0) {
            break;
        }

        printf("Edad: ");
        scanf("%d", &p.edad);
        if (p.edad == -1) {  // Opcional: puedes usar una señal especial para terminar
            break;
        }

        fprintf(baseDatos, "%s\n%s\n%d\n", p.nombre, p.apellido, p.edad);
    }

    fclose(baseDatos);
}



void leerenbasedatos() {
    extern FILE *baseDatos;
    Datos p;
    baseDatos = fopen("baseDatos.txt", "r");
    if (!baseDatos) {
        printf("Error al abrir el archivo para leer.\n");
        return;
    }

    printf("Leyendo datos de la base de datos:\n");
    while (fscanf(baseDatos, "%s\n%s\n%d\n", p.nombre, p.apellido, &p.edad) == 3) {
        printf("\nNombre: %s\nApellidos: %s\nEdad: %d\n", p.nombre, p.apellido, p.edad);
    }
    fclose(baseDatos);
}

void *escritor(void *p) {
    extern sem_t mutex, db;
    int *id_es = (int *) p;

    sem_wait(&db); // Bloquear la base de datos para escritura
    printf("\nEscritor %d escribiendo...\n", *id_es);
    escribirenbasedatos();
    printf("Escritor %d terminó de escribir.\n", *id_es);
    sem_post(&db); // Liberar la base de datos
    pthread_exit(NULL);
}

void *lector(void *p) {
    extern sem_t mutex, db;
    extern int rc;
    int *id_le;
    id_le = (int *)p;

    sem_wait(&mutex);
    rc++;
    if(rc == 1) {
        sem_wait(&db); // El primer lector bloquea a los escritores
    }
    sem_post(&mutex);

    printf("\nLector %d leyendo...\n", *id_le);
    leerenbasedatos();

    sem_wait(&mutex);
    rc--;
    if(rc == 0) {
        sem_post(&db); // El último lector libera el acceso para los escritores
    }
    sem_post(&mutex);

    pthread_exit(NULL);
}

