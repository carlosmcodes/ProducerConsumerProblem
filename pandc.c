#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>
#include <stdbool.h>
#include <sys/time.h>
#include <math.h>

bool checkUserError(int argc);

void *producer_thread(void *arg);

void *consumer_thread(void *arg);

void print_stats(char **argv);

int sleepVal(int arg);

bool checkFraction(double i);

int numberOf_itemsProducer_produces_perCycle;
int threadSpecific_sleepTime;
sem_t semaphore;
sem_t semaphoreOut;
pthread_mutex_t mutex;
int count, in, out = 0;
int *buffer;
int N_elements;
int number_Of_Producers;
int number_Of_Consumers;
int overFlowValue;

/*
 N is the number of buffers to maintain. - 1

 P is the number of producer threads. - 2

 C is the number of consumer threads. - 3

 X is the number of items each producer thread will produce. - 4

 Ptime is the how long (in seconds) each producer thread will sleep after
producing an item. - 5

 Ctime is the how long (in seconds) each consumer thread will sleep after
consuming an item. - 6
 */
int main(int argc, char *argv[]) {
    time_t start_t = time(0);

    // check user input error
    if (!checkUserError(argc))
        return 0;

    time_t start = time(NULL);
    printf("Current Time: %s\n\n", asctime(localtime(&start)));
    print_stats(argv);
    // arg 1 - creating buffer
    N_elements = atoi(argv[1]);
    buffer = malloc(N_elements * sizeof(int));

    // if you delete this line the program will explode
    sem_init(&semaphore, 0, *buffer);
    sem_init(&semaphoreOut, 0, 0);

    // arg 4
    numberOf_itemsProducer_produces_perCycle = atoi(argv[4]);

    // PRODUCER
    number_Of_Producers = atoi(argv[2]);
    pthread_t producer_threads[number_Of_Producers];
    threadSpecific_sleepTime = atoi(argv[5]);
    for (int i = 0; i < number_Of_Producers; i++) {
        pthread_create(&producer_threads[i], NULL, producer_thread, (void *) &threadSpecific_sleepTime);
    }
    // PRODUCER

    // CONSUMER
    number_Of_Consumers = atoi(argv[3]);
    pthread_t consumer_threads[number_Of_Consumers];
    threadSpecific_sleepTime = atoi(argv[6]);
    for (int i = 0; i < number_Of_Consumers; i++) {
        pthread_create(&consumer_threads[i], NULL, consumer_thread, (void *) &threadSpecific_sleepTime);
    }
    // CONSUMER

//    for (int i = 0; i < number_Of_Producers; i++) {
//        pthread_join(producer_threads[i], NULL);
//    }
//    for (int i = 0; i < number_Of_Consumers; i++) {
//        pthread_join(consumer_threads[i], NULL);
//    }

    time_t end = time(0);
    time_t ext = end - start_t;
    printf("Total Runtime:  %ld secs\n", ext);
}

int sleepVal(int arg_int) {
    sleep(arg_int);
//    struct timespec time = {arg_int, 0};
//    nanosleep(&time, NULL);
    return arg_int;
}


/*
 * Function to remove item.
 * Item removed is returned
 */
int dequeue_item() {
    pthread_mutex_lock(&mutex);
    if (count != 0) {
        out = (out + 1) % N_elements;
        count--;
    }
    pthread_mutex_unlock(&mutex);
    return out;
}

/*
 * Function to add item.
 * Item added is returned.
 * It is up to you to determine
 * how to use the return value.
 * If you decide to not use it, then ignore
 * the return value, do not change the
 * return type to void.
 */
int enqueue_item(int item) {
    pthread_mutex_lock(&mutex);
    if (count != N_elements) {
        buffer[in] = item;
        in = (in + 1) % N_elements;
        count++;
    }
    pthread_mutex_unlock(&mutex);
    return item;
}

bool checkUserError(int argc) {
    if (argc != 7) {
        puts("For this program to work properly please enter in 6 arguments seperated by spaces");
        puts("Argument 1: Number of buffers");
        puts("Argument 2: Number of Producer threads to create");
        puts("Argument 3: Number of Consumer threads to create");
        puts("Argument 4: Number of items each producer thread will produce");
        puts("Argument 5: times in seconds each producer thread will sleep after producing an item");
        puts("Argument 6: times in seconds each consumer thread will sleep after consuming an item");
        puts("Lets try again...");
        return false;
    }
    return true;
}

void print_stats(char **argv) {
    printf("                        Number Of Buffers:     %s\n", argv[1]);
    printf("                      Number Of Producers:     %s\n", argv[2]);
    printf("                      Number Of Consumers:     %s\n", argv[3]);
    printf("Number Of items produced by each Producer:     %s\n", argv[4]);
    printf("      Time each Producer Sleeps (seconds):     %s\n", argv[5]);
    printf("      Time each Consumer Sleeps (seconds):     %s\n", argv[6]);
}

void *producer_thread(void *arg) {
    sem_wait(&semaphoreOut);
    int sleepTime = *((int *) arg);
    for (int i = 0; i < numberOf_itemsProducer_produces_perCycle; i++) {
        enqueue_item(i);
        sleepVal(sleepTime);
        printf("%lu was produced by producer->%d", pthread_self(), i);
    }
    sem_post(&semaphore);
}

void *consumer_thread(void *arg) {
    // locks the in semaphore
    sem_wait(&semaphore);
    int sleepTime = *((int *) arg);
    double sequence = (number_Of_Producers *
                       numberOf_itemsProducer_produces_perCycle)
                      / number_Of_Consumers;
    if (!checkFraction(sequence)) {
        overFlowValue = 1;
    }
    for (int i = 0; i < (int) sequence; i++) {
        dequeue_item();
        sleepVal(sleepTime);
        printf("%lu was produced by consumer->%d", pthread_self(), i);
    }
    // signals out semaphore-- that its been updated?....
    sem_post(&semaphoreOut);
    overFlowValue = 0;
}

bool checkFraction(double i) {
    int val = (int) i;
    (i == (double) val) ? true : false;
}