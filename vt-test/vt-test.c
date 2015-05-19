/*
Procucer-consumer task solution. To show ability to use threads, mutexes and conditions, based on pthreads library.
The application must be stopped by pressing Ctrl+C

To build release run:
	make
or:
	make rel
To build non-stripped version with debug info and additional tracings run:
	make deb
To create source code and Makefile archive run:
	make tgz

*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#define MIN_PROD_COUNT			1
#define MAX_PROD_COUNT			10
#define MIN_CONS_COUNT			1
#define MAX_CONS_COUNT			10

#define QUEUE_SIZE				100
#define THRESHOLD				80

#ifdef ENABLE_TRACE
	#define TRACE				fprintf
#else
	#define TRACE(...)
#endif

//Queue object
struct VT_QUEUE {
	int buff[QUEUE_SIZE];									//data buffer, used as cyclic. Destination for producer and source for consumer
	int idx_prod;											//write position for producer
	int idx_cons;											//read position for consumer
	int count;												//number of integers currently available for consumer
	int saturation;											//maximum number of elements in buffer. Higher suspends producer threads
	pthread_mutex_t queue_locker;							//mutex for all queue related operations
	pthread_cond_t	prod_locker;							//locks the producer when upper limit is riched, resumed by consumers signal
	pthread_cond_t	cons_locker;							//locks the consumer when buffer is empty, resumed by producers signal
	int run;												//breaks consumer/producer infinite loop when Ctrl+C is pressed
};

//Consumer context
struct VT_CONSUMER_CTX {
	struct VT_QUEUE* queue;									//pointer to the queue
	FILE* file;												//output file
};

//Queue initialization
static void vt_init_queue(struct VT_QUEUE* queue) {
	queue->idx_prod = queue->idx_cons = 0;					//write and read positions are at the beginning of the buffer
	queue->count = 0;										//buffer is initially empty
	queue->saturation = QUEUE_SIZE;							//number of elements in buffer that suspends producer
	queue->run = 1;											//running state, Ctrl+C will change this to 0
	pthread_mutex_init(&queue->queue_locker, NULL);
	pthread_cond_init(&queue->prod_locker, NULL);
	pthread_cond_init(&queue->cons_locker, NULL);
}

//Queue freeing
static void vt_free_queue(struct VT_QUEUE* queue) {
	pthread_cond_destroy(&queue->prod_locker);
	pthread_cond_destroy(&queue->cons_locker);
	pthread_mutex_destroy(&queue->queue_locker);
}

//Lock queue for short term manipulations (long suspends are using prod_locker and cons_locker)
static void vt_lock_queue(struct VT_QUEUE* queue) {
	pthread_mutex_lock(&queue->queue_locker);
}

//Unlock queue
static void vt_unlock_queue(struct VT_QUEUE* queue) {
	pthread_mutex_unlock(&queue->queue_locker);
}

//Producer thread. Produces random numbers with random delays, puts to buffer, suspends when saturation value is riched
void* producer(void* ctx) {
	struct VT_QUEUE* queue = (struct VT_QUEUE*)ctx;				//passed from main(), tried not to have globals
	TRACE(stderr, "Producer started\n");
	while(queue->run) {
		usleep((rand() % 101) * 1000);							//0-100 ms (inclusive) in us
		vt_lock_queue(queue);									//enter critical section for all queue operations
		if(queue->count >= queue->saturation) {					//check if the buffer full. Either 100% or 80%, depending on prev. state
			TRACE(stderr, "P:F:%d\n", queue->count);
			queue->saturation = THRESHOLD;						//once 100% filled, 80% is marked as full
			pthread_cond_wait(&queue->prod_locker, &queue->queue_locker);//suspend - queue is saturated (consumers signal will resume)
			vt_unlock_queue(queue);								//consumers signal arrived
			continue;
		}
		queue->saturation = QUEUE_SIZE;							//once the space to write was available, set upper limit to 100%
		queue->buff[queue->idx_prod ++] = rand() % 100 + 1;		//new "product"
		queue->idx_prod %= QUEUE_SIZE;							//the index is cyclic - 0 after last
		queue->count ++;										//new "product" occupies single "unit" space
		pthread_cond_signal(&queue->cons_locker);				//wake up any of suspended consumers - there is fresh data to read
		vt_unlock_queue(queue);									//leave the critical section
	}
	TRACE(stderr, "Producer stopping\n");
	return 0;
}

void* consumer(void* ctx) {
	struct VT_CONSUMER_CTX* cc = (struct VT_CONSUMER_CTX*)ctx;	//passed from main(), tried not to have globals
	struct VT_QUEUE* queue = cc->queue;
	TRACE(stderr, "Consumer started\n");
	while(1) {
		usleep((rand() % 101) * 1000);							//0-100 ms (inclusive) in us
		vt_lock_queue(queue);									//enter critical section
		if(queue->count < 1) {									//queue is empty
			TRACE(stderr, "C:E:%d\n", queue->count);
			if(queue->run) {									//the program is still running -
				pthread_cond_wait(&queue->cons_locker, &queue->queue_locker);//suspend - queue is empty (producers signal will resume)
				vt_unlock_queue(queue);							//producers signal arrived
				continue;
			}
			else {												//queue is empty because the program is terminating, break and give up
				vt_unlock_queue(queue);
				break;
			}
		}
		//get new item  and write to file
		//!!! file operation is in critical section, so it is safe, but...
		//the real life solution is to remember the value, leave the critical section quickly,
		//and write to file outside of the critical section. In this case file lock can be used to
		//avoid concurent writings from other consumers. Needs some research, maybe fprintf already contains flock?
		//maybe flock works only for processes with different process ids?
		//Anyway, in our case file operation is relatively fast because we have up to 100 ms functional delays.
		fprintf(cc->file, "%d,", queue->buff[queue->idx_cons ++]);
		queue->idx_cons %= QUEUE_SIZE;							//the index is cyclic - 0 after last
		queue->count --;										//removed "product" occupied single "unit" space
		pthread_cond_signal(&queue->prod_locker);				//wake up any of suspended producers - new free space is available
		vt_unlock_queue(queue);									//leave the critical section
	}
	TRACE(stderr, "Consumer stopping\n");
	return 0;
}

//reads pair of integers from stdin, normalizes if out of range or invalid
static void vt_get_params(int* prod_count, int* cons_count) {
	fprintf(stderr, "Enter space separated number of producers and consumers (1..10). Out of range values will be normalized... ");
	*prod_count = *cons_count = 0;
	if(2 != scanf("%d%d", prod_count, cons_count)) {
		fprintf(stderr, "Invalid number of inputs. Taking default(s)\n");
	}
	if(*prod_count < MIN_PROD_COUNT) *prod_count = 1;
	if(*prod_count > MAX_PROD_COUNT) *prod_count = 10;
	if(*cons_count < MIN_CONS_COUNT) *cons_count = 1;
	if(*cons_count > MAX_CONS_COUNT) *cons_count = 10;
}

//scans threads, sends signals when alive thread is found
//only condition can lock thread for long duration
//this can happen only when the queue is full or empty
static int vt_wakeup_suspended_workers(pthread_t* first, pthread_t* end, struct VT_QUEUE* queue) {
	int count = 0;
	while(first < end) {
		//check if thread is alive. Signal "0" is not sent actually, return value of pthread_kill shows if thread is alive
		if(!pthread_kill(*first ++, 0)) {
			count ++;
			//send signal to both lockers because we don't know the type of found alive thread
			pthread_cond_signal(&queue->cons_locker);
			pthread_cond_signal(&queue->prod_locker);
		}
	}
	return count;
}

//waits for all child threads to die
static void vt_join_all_workers(pthread_t* first, pthread_t* end) {
	while(first < end) {
		pthread_join(*first ++, 0);
	}
}

//main loop of the main thread.
//reports queue item count every second, can be terminated only by Ctrl+C
static void vt_wait_for_stop_and_report(struct VT_QUEUE* queue) {
	//while running print report every 1 second
	while(queue->run) {
		sleep(1);														//signal interrupts the sleep, run=0 breaks the loop
		fprintf(stderr, "============Count: %d============\n", queue->count);
	}
}

int main() {
	static const char* fname = "/tmp/data.txt";							//consumers output
	struct VT_QUEUE queue;												//data queue, also context for producer and consumer
	struct VT_CONSUMER_CTX cons_ctx = {&queue, fopen(fname, "w")};		//contains also consumer-specific information (file)
	int prod_count, cons_count, i;
	pthread_t thread_pool[MAX_PROD_COUNT + MAX_CONS_COUNT];				//threads, used for checking and waiting at the end
	pthread_t* pp = thread_pool;
	if(!cons_ctx.file) {												//consumer output file could not be opened, stop
		perror(fname);
		return 1;
	}
	void ctrl_c(int sig) {												//program runs infinitely, can be stopped using Ctrl+C
		queue.run = 0;
	}
	srand(time(0));
	vt_get_params(&prod_count, &cons_count);
	vt_init_queue(&queue);

	fprintf(stderr, "To terminate the progran use Ctrl+C.\nOutput file is: %s\n", fname);
	signal(SIGINT, ctrl_c);

	//create prod_count production threads:
	for(i = 0; queue.run && i < prod_count; i ++) {
		if(pthread_create(pp++, 0, producer, &queue)) {					//if failed starting any of threads, stop everything
			perror("Create producer thread");
			queue.run = 0;
		}
	}


	//create cons_count consumer threads:
	for(i = 0; queue.run && i < cons_count; i ++) {
		if(pthread_create(pp++, 0, consumer, &cons_ctx)) {				//if failed starting any of threads, stop everything
			perror("Create consumer thread");
			queue.run = 0;
		}
	}

	//while running print report every 1 second
	vt_wait_for_stop_and_report(&queue);

	//try to wake up while any of threads didn't end
	while(vt_wakeup_suspended_workers(thread_pool, pp, &queue));
	
	//wait for all threads to  end
	vt_join_all_workers(thread_pool, pp);

	//destroy mutex, etc.
	vt_free_queue(&queue);

	fseek(cons_ctx.file, -1, SEEK_END);	//last symbol is ','
	fprintf(cons_ctx.file, "\n"); //put new line at the end of file
	fclose(cons_ctx.file);
	TRACE(stderr, "END\n");
	return 0;
}

