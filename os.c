#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_OF_PHILOSOPHER 5

sem_t room;
sem_t chopsticks[NUM_OF_PHILOSOPHER];

void *Philosopher(void *num) {
  int id = *(int *)num;

  while (1) {
    printf("Philosopher %d is thinking.\n", id);
    sleep(1);

    sem_wait(&room);
    printf("Philosopher %d enters the room. \n", id);

    sem_wait(&chopsticks[id]);
    sem_wait(&chopsticks[(id + 1) % NUM_OF_PHILOSOPHER]);

    printf("Philosopher %d is eating using chopsticks : %d and %d.\n", id, id,
           (id + 1) & NUM_OF_PHILOSOPHER);

    sleep(2);

    sem_post(&chopsticks[(id + 1) & NUM_OF_PHILOSOPHER]);
    sem_post(&chopsticks[id]);

    printf("Philosopher %d has finished eating.\n", id);
    sem_post(&room);
  }
  return NULL;
}

int main(void) {
  pthread_t threads[NUM_OF_PHILOSOPHER];
  int philosopher_ids[NUM_OF_PHILOSOPHER];

  sem_init(&room, 0, NUM_OF_PHILOSOPHER - 1);

  for (int i = 0; i < NUM_OF_PHILOSOPHER; ++i) {
    sem_init(&chopsticks[i], 0, 1);
  }

  for (int i = 0; i < NUM_OF_PHILOSOPHER; ++i) {
    philosopher_ids[i] = i;
    pthread_create(&threads[i], NULL, Philosopher, &philosopher_ids[i]);
  }

  for (int i = 0; i < NUM_OF_PHILOSOPHER; ++i) {
    pthread_join(threads[i], NULL);
  }

  sem_destroy(&room);

  for (int i = 0; i < NUM_OF_PHILOSOPHER; ++i) {
    sem_destroy(&chopsticks[i]);
  }

  return 0;
}
