#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

/*Variables Globales*/

	int rc=0;

	typedef struct datos{
		char nombre[20];
		char apellido[20];
		int edad;
	}Datos;

	unsigned int num_escritores, num_lectores;
	FILE * baseDatos;

/*Semaforos*/

	sem_t mutex,db;

/*Funciones*/

	void *lector(void *p);
	void *escritor(void *p);
	void escribirenbasedatos();
	void leerenbasedatos();

int main(int argc, char *argv[]){

	int estado,i;
	extern FILE * baseDatos;
	baseDatos=fopen("baseDatos.txt","w");
	fclose(baseDatos);

	 if ( argc < 3 ) {
                printf("Numero de parametros incorrecto. Indicar el numero de datos a producir.\n") ;
                exit(1);              
        }
	
	num_escritores = atoi(argv[1]);
	num_lectores = atoi(argv[2]);

	int eid[num_escritores],lid[num_lectores];

	/*Declaración de las dos hebras*/

	pthread_t escritora[num_escritores],lectora[num_lectores];

	/*Inicialización de Semáforos*/

	sem_init(&mutex,0,1);
	sem_init(&db,0,1);

	for(i=0;i<num_lectores;i++){
		lid[i]=i;
		if((estado=pthread_create(&lectora[i],NULL,lector,(void *) &lid[i])))
			exit(estado);
	}

	for(i=0;i<num_escritores;i++){
		eid[i]=i;
		if((estado=pthread_create(&escritora[i],NULL,escritor,(void *) &eid[i])))
			exit(estado);
	}

	/*La hebra principal esperara a que terminen las otras 2 hebras.*/

	for(i=0;i<num_lectores;i++)
	{
		pthread_join(lectora[i],NULL);
	}
	for(i=0;i<num_escritores;i++)
	{
		pthread_join(escritora[i],NULL);
	}
	/*Destrucción de los semaforos*/

	sem_destroy(&mutex);
	sem_destroy(&db);

	return 0;
}
void escribirenbasedatos(){

	extern FILE *baseDatos;
	char c;
	baseDatos=fopen("baseDatos.txt","a+");

	while ((c=getc(stdin))!=EOF){
		fputc(c,baseDatos);
	}
	fclose(baseDatos);	

}
void leerenbasedatos(){
	
	extern FILE *baseDatos;
	Datos p;        
	baseDatos=fopen("baseDatos.txt","r");

	while (fscanf(baseDatos,"%s\n%s\n%d\n",p.nombre,p.apellido,&p.edad)==3){

		printf("\nNombre:");
		puts(p.nombre);
		printf("Apellidos:");
		puts(p.apellido);
		printf("Edad:\n");
		printf("%d",p.edad);
	}

	fclose(baseDatos);

}


void *escritor(void *p){

		extern sem_t mutex,db;
		int *id_es;
        	id_es=(int *)p;

		/*Sección Residual*/

		sem_wait(&db);
		printf("\nEscritor %d Escribiendo...\n",*id_es);
		escribirenbasedatos();
		sem_post(&db);
		pthread_exit(NULL);

}

void *lector(void *p){

		extern sem_t mutex,db;
		extern int rc;
		int *id_le;

        	id_le=(int *)p;
		
		sem_wait(&mutex);
		rc++;
		if(rc == 1)
		   sem_wait(&db);

		
		printf("\nLector %d Leyendo...\n",*id_le);
		leerenbasedatos();
		sem_post(&mutex);
		sem_wait(&mutex);
		rc--;

		if(rc == 0)
		  sem_post(&db);

		sem_post(&mutex);
		pthread_exit(NULL);
}