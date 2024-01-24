/*19 - Produttore, Consumatore
Una pila di 10 elementi interi è condivisa tra due thread: un produttore ed un consumatore

1)     Il produttore deve essere implementato secondo la seguente logica. In un ciclo infinito:

Deve attendere una quantità di tempo casuale inferiore al secondo
Una volta scaduta l’attesa, se la pila è piena, deve attendere che qualche elemento venga rimosso dal consumatore
Quando si libera dello spazio nello stack, deve inserire un numero casuale di elementi (senza andare in overflow)
Deve segnalare al consumatore la disponibilità di nuovi elementi da consumare
2)     Il consumatore deve essere implementato secondo la seguente logica. In un ciclo infinito:

Deve attendere una quantità di tempo casuale inferiore al secondo
Una volta scaduta l’attesa, se lo stack è vuoto, deve attendere che qualche elemento venga inserito dal produttore
Quando lo stack non è vuoto, deve leggere un numero casuale di elementi (inferiore o uguale al numero di elementi presenti nello stack)
Deve segnalare al produttore la possibilità di inserire nuovi elementi
 

Suggerimenti:

-        Lo stack può essere implementato con un array di interi, un contatore di elementi già inseriti, e con due funzioni: push() e pop()

-        Alcune funzioni utili: random() e usleep()*/
-----------------------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define SIZE 10
int buffer[SIZE];
int count = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

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
			push((int)random());
			printf("Producer - buffer[%d]=%d\n", count-1, buffer[count-1]);
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
			printf("Consumer - buffer[%d]=%d\n", count, pop());
		}
		pthread_cond_signal(&cond);
		pthread_mutex_unlock(&mutex);	
	}
	return NULL;
}

int main() {

	pthread_t producer_t, consumer_t;
	pthread_create(&producer_t, NULL, producer, NULL);
	pthread_create(&consumer_t, NULL, consumer, NULL);
	pthread_join(producer_t, NULL);
	pthread_join(consumer_t, NULL);
	
	return 0;
}
