/*
20 - Esercitazione su produttore, consumatore
Un file su disco ha il seguente formato:
<numero_record><record 1><record 2>…

dove:

-        <numero_record> è un intero rappresentante il numero di record attualmente presenti all’interno del file

-        <record1><record2>, …, sono ognuno un numero intero.

Il file è acceduto da due thread: un produttore ed un consumatore, ed è gestito come se fosse una pila: i nuovi elementi vengono accodati al termine del file, e la lettura (con contestuale rimozione) degli elementi avviene dall’ultimo elemento del file. Il file non deve contenere più di 10 record oltre all’indicatore iniziale del numero di record presenti.

I due thread, produttore e consumatore, hanno il seguente comportamento:

1)     Il produttore, in un ciclo infinito:

Deve attendere una quantità di tempo casuale inferiore al secondo
Una volta scaduta l’attesa, se la pila contenuta nel file è piena, deve attendere che qualche elemento venga rimosso dal consumatore
Quando si libera dello spazio nella pila, deve inserire un numero casuale di elementi (senza andare in overflow rispetto alle dimensioni della pila) ed aggiornare il contatore all’inizio del file
Deve segnalare al consumatore la disponibilità di nuovi elementi da consumare
2)     Il consumatore, in un ciclo infinito:

Deve attendere una quantità di tempo casuale inferiore al secondo
Una volta scaduta l’attesa, se la pila è vuota, deve attendere che qualche elemento venga inserito dal produttore
Quando la pila non è vuota, deve leggere un numero casuale di elementi (inferiore o uguale al numero di elementi presenti nello stack), sostituirne il valore con il numero 0 ed aggiornare il valore all’inizio del file.
Deve segnalare al produttore la possibilità di inserire nuovi elementi
 

Suggerimento

Quando si esegue una read() o una write() su un file, viene spostato un cursore in avanti del numero di byte letti o scritti sul file. Ad esempio, se un file contenesse la stringa “ciaopino” e venisse effettuata una read() di 4 byte, questa leggerebbe “ciao”. Un’eventuale seconda read() di 4 byte leggerebbe invece “pino”. Allo stesso modo, se si eseguisse una prima lettura di 4 byte e successivamente una scrittura di 4 byte della stringa “anno”, il file conterrebbe la stringa “ciaoanno” al termine dell’esecuzione delle read() e delle write().
E’ possibile spostare il cursore anche senza necessariamente effettuare una read() o una write(), utilizzando la funzione lseek() – usa il manuale per scoprire come usare lseek().
*/


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SIZE 10
//int buffer[SIZE];
int fd;
int count;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

/*
int pop() {
	int ret = buffer[count-1];
	buffer[count-1]=0;
	count--;
	return ret;
}

void push(int v) {
	buffer[count] = v;
	count++;
}
*/
void f_push(int v) {
	lseek(fd,(count+1)*sizeof(int),SEEK_SET);
	write(fd,&v,sizeof(int));
	count++;
	lseek(fd,0,SEEK_SET);
	write(fd,&count,sizeof(int));
}

int f_pop() {
	int zero=0;
	int ret;
	lseek(fd,count*sizeof(int),SEEK_SET);
	read(fd, &ret, sizeof(int));
	lseek(fd,count*sizeof(int),SEEK_SET);
	write(fd, &zero, sizeof(int));
	count--;
	lseek(fd,0,SEEK_SET);
	write(fd,&count,sizeof(int));
	return ret;
}

void f_count() {
	lseek(fd,0,SEEK_SET);
	read(fd,&count,sizeof(int));
}

void* producer(void* arg) {
	while(1) {
		long sleep_time = random() % 1000000;
		usleep(sleep_time);
		pthread_mutex_lock(&mutex);

		while (count == SIZE) {
			pthread_cond_wait(&cond, &mutex);
		}
		long elements = random() % (SIZE - count);
		for (int i=0; i<elements;i++) {
			int r = (int)random();
			f_push(r);
			printf("Producer - Posizione %d: %d\n", count, r );
		}
		pthread_cond_signal(&cond);
		pthread_mutex_unlock(&mutex);
	}
	return NULL;
}

void* consumer(void* arg) {
	while (1) {
		long sleep_time = random() % 1000000;
		usleep(sleep_time);
		pthread_mutex_lock(&mutex);
		while (count == 0) {
			pthread_cond_wait(&cond, &mutex);
		}
		long elements = random() % count;
		for (int i=0;i<elements;i++) {
			printf("Consumer - Posizione %d: %d\n", count+1, f_pop());
		}
		pthread_cond_signal(&cond);
		pthread_mutex_unlock(&mutex);	
	}
	return NULL;
}

void stampa_file() {
	int fd = open("file",  O_RDONLY);
	int buf;
	for (int i=0; i<SIZE+1; i++) {
		read(fd,&buf,sizeof(int));
		printf("%d\t", buf);
	}
	printf("\n");
	close(fd);
}

int main(int argc, char** argv) {

	if (argc>1) {
		stampa_file();
		exit(EXIT_SUCCESS);
	}

	//Inizializzazione file
	fd = open("file",  O_CREAT|O_RDWR|O_TRUNC, S_IRUSR|S_IWUSR);
	int buf=0;
	for (int i=0; i<SIZE+1;i++) {
		write(fd,&buf,sizeof(int));
	}

	pthread_t producer_t, consumer_t;
	pthread_create(&producer_t, NULL, producer, NULL);
	pthread_create(&consumer_t, NULL, consumer, NULL);
	pthread_join(producer_t, NULL);
	pthread_join(consumer_t, NULL);
	
	return 0;
}
--------------------------
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#define SIZE 10
//int buffer[SIZE];
int* buffer;
int fd;
int count=0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;


int pop() {
	int ret = buffer[count];
	buffer[count]=0;
	count--;
	buffer[0]=count;
	return ret;
}

void push(int v) {
	buffer[count+1] = v;
	count++;
	buffer[0]=count;
}

void f_push(int v) {
	lseek(fd,(count+1)*sizeof(int),SEEK_SET);
	write(fd,&v,sizeof(int));
	count++;
	lseek(fd,0,SEEK_SET);
	write(fd,&count,sizeof(int));
}

int f_pop() {
	int zero=0;
	int ret;
	lseek(fd,count*sizeof(int),SEEK_SET);
	read(fd, &ret, sizeof(int));
	lseek(fd,count*sizeof(int),SEEK_SET);
	write(fd, &zero, sizeof(int));
	count--;
	lseek(fd,0,SEEK_SET);
	write(fd,&count,sizeof(int));
	return ret;
}

void f_count() {
	lseek(fd,0,SEEK_SET);
	read(fd,&count,sizeof(int));
}

void* producer(void* arg) {
	while(1) {
		long sleep_time = random() % 1000000;
		usleep(sleep_time);
		pthread_mutex_lock(&mutex);

		while (count == SIZE) {
			pthread_cond_wait(&cond, &mutex);
		}
		long elements = random() % (SIZE - count);
		for (int i=0; i<elements;i++) {
			int r = (int)random();
			push(r);
			msync(buffer,(SIZE+1)*sizeof(int),MS_SYNC);
			printf("Producer - Posizione %d: %d\n", count, r );
		}
		pthread_cond_signal(&cond);
		pthread_mutex_unlock(&mutex);
	}
	return NULL;
}

void* consumer(void* arg) {
	while (1) {
		long sleep_time = random() % 1000000;
		usleep(sleep_time);
		pthread_mutex_lock(&mutex);
		while (count == 0) {
			pthread_cond_wait(&cond, &mutex);
		}
		long elements = random() % count;
		for (int i=0;i<elements;i++) {
			printf("Consumer - Posizione %d: %d\n", count+1, pop());
			msync(buffer,(SIZE+1)*sizeof(int),MS_SYNC);
		}
		pthread_cond_signal(&cond);
		pthread_mutex_unlock(&mutex);	
	}
	return NULL;
}

void stampa_file() {
	int fd = open("file",  O_RDONLY);
	int buf;
	for (int i=0; i<SIZE+1; i++) {
		read(fd,&buf,sizeof(int));
		printf("%d\t", buf);
	}
	printf("\n");
	close(fd);
}

int main(int argc, char** argv) {

	if (argc>1) {
		stampa_file();
		exit(EXIT_SUCCESS);
	}

	//Inizializzazione file
	fd = open("file",  O_CREAT|O_RDWR|O_TRUNC, S_IRUSR|S_IWUSR);
	int buf=0;
	for (int i=0; i<SIZE+1;i++) {
		write(fd,&buf,sizeof(int));
	}
	lseek(fd,0,SEEK_SET);
	buffer = mmap(NULL, (SIZE+1)*sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

	pthread_t producer_t, consumer_t;
	pthread_create(&producer_t, NULL, producer, NULL);
	pthread_create(&consumer_t, NULL, consumer, NULL);
	pthread_join(producer_t, NULL);
	pthread_join(consumer_t, NULL);
	
	return 0;
}
