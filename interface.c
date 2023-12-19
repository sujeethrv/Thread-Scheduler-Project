#include "interface.h"
#include "scheduler.h"
#include <pthread.h>
#include <math.h>
#include <stdint.h>
// Interface implementation
int globalTime = 0;
int totalThreads = 0;
int threadsEnqueued = 0;
pthread_mutex_t mutexEnqueue, mutexFullQueue, mutexCpuBurst; //mutexDequeue;
pthread_cond_t condEnqueue, condFullQueue, condCpuBurst; //condDequeue;


//creating a struct with boolean value. This struct will be used to create an array of structs which will store boolean values for each thread if it was called by cpu_me or not
struct tidBool {
    // int tid;
    int called;
};

struct tidBool *tidBoolArray;

struct queue_thread {
    int tid;
    int remaining_time;
    float arrival_time;
};

struct queue_thread *queue;

int front = -1;
int rear = -1;


// IO device 1 Interface implementation
int front_IO1 = -1;
int rear_IO1 = -1;
//int current_Pointer_IO1 = -1;

int globalTime_IO1 = 0;
int totalThreads_IO1 = 0;
int threadsEnqueued_IO1 = 0;
pthread_mutex_t mutexEnqueue_IO1, mutexFullQueue_IO1, mutexIO1Burst; //mutexDequeue;
pthread_cond_t condEnqueue_IO1, condFullQueue_IO1, condIO1Burst; //condDequeue;
struct queue_thread_IO1 {
    int tid;
    int device_id;
    float arrival_time;
    int end_time;
};

struct queue_thread_IO1 *queue_IO1;


// IO device 2 Interface implementation
int front_IO2 = -1;
int rear_IO2 = -1;
//int current_Pointer_IO2 = -1;

int globalTime_IO2 = 0;
int totalThreads_IO2 = 0;
int threadsEnqueued_IO2 = 0;
pthread_mutex_t mutexEnqueue_IO2, mutexFullQueue_IO2, mutexIO2Burst; //mutexDequeue;
pthread_cond_t condEnqueue_IO2, condFullQueue_IO2, condIO2Burst; //condDequeue;
struct queue_thread_IO2 {
    int tid;
    int device_id;
    float arrival_time;
    int end_time;
};

struct queue_thread_IO2 *queue_IO2;

//implementing a circular FIFO queue. FIFO stands for First In First Out. 
void enqueueFIFO(float arrivalTime, int tid,int remaining_time) {
    /*
    inserts an element to the queue based on arrival time. lower arrival time is given higher priority
    */
    if (rear == -1) {
        front = 0;
        rear = 0;
    } 
    else {
        rear++;
    }

    int i = front;
    while (arrivalTime > queue[i].arrival_time && i < rear) {
        i++;
    }
    // if arrival times are same, then the one with lower tid is given higher priority
    if (arrivalTime == queue[i].arrival_time) {
        while (tid > queue[i].tid && i < rear) {
            i++;
        }
    }
    //moving all the queue elements one position ahead
    for (int j = rear; j > i; j--) {
        queue[j] = queue[j - 1];
    }
    //inserting the element
    queue[i].tid = tid;
    queue[i].remaining_time = remaining_time;
    queue[i].arrival_time = arrivalTime;

    printf("Thread %d enqueued at time %d with remaining time %d and arrival time %f \n", tid, globalTime, remaining_time, arrivalTime);
    // display();
}

void dequeueFIFO(int tid) {
    /*
    removes element with the given tid from the queue
    */
    if (front == -1) {
        printf("Queue is empty\n");
        return;
    }
    bool found = false;
    //searching for the element with the given tid in the queue and moving all the queue elements from that position one position ahead
    for (int i = front; i <= rear; i++) {
        if (queue[i].tid == tid) {
            found = true;
            for (int j = i; j < rear; j++) {
                queue[j] = queue[j + 1];
            }
            break;
        }
    }
    //decrementing rear
    if (found) {
        rear--;
    }
    // rear--;
    //if rear is -1 then queue is empty
    if (rear == -1) {
        front = -1;
    }
    // printf("Thread %d dequeued at time %d \n", tid, globalTime);
    // display();


}
    // if (front == -1) {
    //     printf("Queue is empty\n");
    //     return;
    // }
    // //moving all the queue elements one position ahead
    // for (int i = front; i < rear; i++) {
    //     queue[i] = queue[i + 1];
    // }
    // //decrementing rear
    // rear--;
    // //if rear is -1 then queue is empty
    // if (rear == -1) {
    //     front = -1;
    // }

//implementing a priority queue. higher priority is given to the thread with lower remaining time
void enqueuePriority(float arrivalTime, int tid,int remaining_time) {
    /*
    inserts an element to the queue based on remaining time. lower remaining time is given higher priority.
    */
    if (rear == -1) {
        front = 0;
        rear = 0;
    } 
    else {
        rear++;
    }
    int i = front;
    while (remaining_time > queue[i].remaining_time && i < rear) {
        i++;
    }
    // if remaining times are same, then the one with lower tid is given higher priority
    if (remaining_time == queue[i].remaining_time) {
        while (tid > queue[i].tid && i < rear) {
            i++;
        }
    }
    //moving all the queue elements one position ahead
    for (int j = rear; j > i; j--) {
        queue[j] = queue[j - 1];
    }
    //inserting the element
    queue[i].tid = tid;
    queue[i].remaining_time = remaining_time;
    queue[i].arrival_time = arrivalTime;
    printf("Thread %d enqueued at time %d with remaining time %d and arrival time %f \n", tid, globalTime, remaining_time, arrivalTime);
    printf("Inside enqueuePriority - front: %d, rear: %d \n", front, rear);
    display();
}

void updateQueue(int tid, int remaining_time) {
    /*
    updates the remaining time of the thread with the given tid
    */
    float temp_arrival_time;
    int blocked_until;
    for (int i = front; i <= rear; i++) {
        if (queue[i].tid == tid) {
            temp_arrival_time = queue[i].arrival_time;
            break;
        }
    }
    
    printf("updating queue for thread %d with remaining time %d \n", tid, remaining_time);
    dequeuePriority(tid);
    enqueuePriority(temp_arrival_time, tid, remaining_time);
    printf("updated queue for thread %d with remaining time %d \n", tid, remaining_time);


    display();
    
}

void dequeuePriority(int tid) {
    /*
    removes element with the given tid from the queue
    */
    if (front == -1) {
        printf("Queue is empty\n");
        return;
    }
    //searching for the element with the given tid in the queue and moving all the queue elements from that position one position ahead
    for (int i = front; i <= rear; i++) {
        if (queue[i].tid == tid) {
            for (int j = i; j < rear; j++) {
                queue[j] = queue[j + 1];
            }
            break;
        }
    }
    //decrementing rear
    rear--;
    //if rear is -1 then queue is empty
    if (rear == -1) {
        front = -1;
    }
    // printf("Thread %d dequeued at time %d \n", tid, globalTime);
    // display();
}

//reseting the queue after decreasing the remaining time of the first element by 1


void display() {
    /*
    displays the queue with the given tid, remaining time and arrival time in a single line
    */
    if (front == -1) {
        printf("Queue is empty \n");
        return;
    }
    for (int i = front; i <= rear; i++) {
        printf("(tid:%d,rt:%d,at:%f) || ", queue[i].tid, queue[i].remaining_time, queue[i].arrival_time);
    }
    printf("\n");
    return;
}

void display_IO1() {
    /*
    displays the queue with the given tid, remaining time and arrival time in a single line
    */
    if (front == -1) {
        printf("Queue is empty \n");
        return;
    }
    for (int i = front_IO1; i <= rear_IO1; i++) {
        printf("IO1: (tid:%d,et:%d,at:%f) || ", queue_IO1[i].tid, queue_IO1[i].end_time, queue_IO1[i].arrival_time);
    }
    printf("\n");
    return;
}

bool checkQueue(int tid){
    if (front == -1) {
        printf("Queue is empty\n");
        return false;
    }
    for (int i = front; i <= rear; i++) {
        // printf("tid: %d, queue[i].tid: %d \n i: %d, front: %d, rear: %d\n", tid, queue[i].tid, i, front, rear);
        if(queue[i].tid == tid){
            return true;
        }
    }
    return false;
}

int peek() {
    if (front == -1) {
        printf("Queue is empty\n");
        return -1;
    }
    return queue[front].tid;
}

int peekPriority() {
    /*
    returns the tid of the thread with the shortest remaining time and 
    arrival time <= globalTime. Incase, there is no such thread with 
    arrival time <= globalTime, then it returns the tid of the thread 
    with the least arrival time greater than globalTime and 
    shortest remaining time and set the globalTime to the arrival time 
    of that thread using mutex
    */
    if (front == -1) {
        printf("Peek Priority: Queue is empty\n");
        return -1;
    }
    int i = front;
    while (i <= rear) {
        if (queue[i].arrival_time <= globalTime && !check_performing_io(queue[i].tid)) { // && queue[i].remaining_time > 0
            return queue[i].tid;
        }
        i++;
    }
    // if no thread with arrival time <= globalTime is found, then the thread with the least arrival time greater than globalTime is returned
    i = front;
    int minArvlIdx = i;
    float min_Arv = queue[minArvlIdx].arrival_time;
    if(check_performing_io(queue[minArvlIdx].tid)){
        min_Arv = check_performing_io(queue[minArvlIdx].tid);
    }
    while (i <= rear) {
        float arrival = queue[i].arrival_time;
        if(check_performing_io(queue[i].tid)){
            arrival = check_performing_io(queue[i].tid);
        }
        if (arrival > globalTime) {// && queue[i].remaining_time > 0
            if (arrival < min_Arv) {
                minArvlIdx = i;
                min_Arv = arrival;
            }
            // pthread_mutex_lock(&mutexCpuBurst); // Check if this is needed!!!! mutex to set globalTime to the arrival time of the thread
            
            // globalTime = ceil(queue[i].arrival_time);
            // printf("In peekPriority globalTime set to %d \n", globalTime);
            // pthread_mutex_unlock(&mutexCpuBurst);
            // return queue[i].tid;
        }
        i++;
    }

    globalTime = ceil(min_Arv);
    printf("In peekPriority globalTime set to %d for tid %d\n", globalTime,queue[minArvlIdx].tid);
    return queue[minArvlIdx].tid;
    // return -1;
}



void init_scheduler(int thread_count) {
    // TODO: Implement this
    totalThreads = thread_count;
    int temp_thread_count = 1000*thread_count;
    queue = (struct queue_thread *) calloc(temp_thread_count, sizeof(*queue));
    if (!queue) {
        perror("malloc() error");
        return;
    }
    memset(queue, 0, sizeof(*queue) * temp_thread_count);

    tidBoolArray = (struct tidBool *) calloc(thread_count, sizeof(*tidBoolArray));
    if (!tidBoolArray) {
        perror("malloc() error");
        return;
    }
    memset(tidBoolArray, 0, sizeof(*tidBoolArray) * thread_count);

    pthread_mutex_init(&mutexEnqueue, NULL);
    pthread_mutex_init(&mutexFullQueue, NULL);
    pthread_mutex_init(&mutexCpuBurst, NULL);
    // pthread_mutex_init(&mutexDequeue, NULL);
    pthread_cond_init(&condEnqueue, NULL);
    pthread_cond_init(&condFullQueue, NULL);
    pthread_cond_init(&condCpuBurst, NULL);
    // pthread_cond_init(&condDequeue, NULL);

    //Duplicate code, init for IO (1)
    totalThreads_IO1 = temp_thread_count;
    queue_IO1 = (struct queue_thread_IO1 *) calloc(temp_thread_count, sizeof(*queue_IO1));
    if (!queue_IO1) {
        perror("malloc() error");
        return;
    }
    memset(queue_IO1, 0, sizeof(*queue_IO1) * temp_thread_count);
    pthread_mutex_init(&mutexEnqueue_IO1, NULL);
    pthread_mutex_init(&mutexFullQueue_IO1, NULL);
    pthread_mutex_init(&mutexIO1Burst, NULL);
    // pthread_mutex_init(&mutexDequeue, NULL);
    pthread_cond_init(&condEnqueue_IO1, NULL);
    pthread_cond_init(&condFullQueue_IO1, NULL);
    pthread_cond_init(&condIO1Burst, NULL);

    totalThreads_IO2 = thread_count;
    queue_IO2 = (struct queue_thread_IO2 *) calloc(temp_thread_count, sizeof(*queue_IO2));
    if (!queue_IO2) {
        perror("malloc() error");
        return;
    }
    memset(queue_IO2, 0, sizeof(*queue_IO2) * temp_thread_count);
    pthread_mutex_init(&mutexEnqueue_IO2, NULL);
    pthread_mutex_init(&mutexFullQueue_IO2, NULL);
    pthread_mutex_init(&mutexIO2Burst, NULL);
    // pthread_mutex_init(&mutexDequeue, NULL);
    pthread_cond_init(&condEnqueue_IO2, NULL);
    pthread_cond_init(&condFullQueue_IO2, NULL);
    pthread_cond_init(&condIO2Burst, NULL);
}



int cpu_me(float current_time, int tid, int remaining_time) {
    // TODO: Implement this
    // printf("--------------------------------------------\n");
    printf("Beginning globalTime: %d, tid: %d \n", globalTime, tid);
    tidBoolArray[tid].called = 1;
    // Enqueue the thread if it is not already in the queue.
    pthread_mutex_lock(&mutexEnqueue);
    if (checkQueue(tid) == false){
        printf("inside if mutexEnqueue locked. tid: %d, remaining_time: %d, globalTime: %d\n", tid, remaining_time, globalTime);
        enqueuePriority(current_time, tid, remaining_time);
        threadsEnqueued++;
    }
    else{
        printf("inside else mutexEnqueue locked. tid: %d, remaining_time: %d, globalTime: %d\n", tid, remaining_time, globalTime);
        // int tidBefore = peekPriority();
        updateQueue(tid, remaining_time);
        // int tidAfter = peekPriority();
        // if (tidBefore != tidAfter){
        printf("inside if tidBefore != tidAfter. tid: %d, remaining_time: %d, globalTime: %d\n", tid, remaining_time, globalTime);
        pthread_cond_signal(&condCpuBurst);
        // }

    }
    // printf("mutexEnqueue locked after if. tid: %d, remaining_time: %d, globalTime: %d\n", tid, remaining_time, globalTime);
    pthread_mutex_unlock(&mutexEnqueue);
    // make all the threads wait using pthread_cond_wait until the size of the queue is equal to the number of threads "thread_count"
    
    pthread_mutex_lock(&mutexFullQueue);
    if (threadsEnqueued < totalThreads){
        printf("inside if cpu_me. tid: %d, remaining_time: %d, globalTime: %d\n", tid, remaining_time, globalTime);
        pthread_cond_wait(&condFullQueue, &mutexFullQueue);// is this correct? can we have pthread_cond_wait not enclosed in mutext lock and unlock?
    }
    else{
        printf("inside else cpu_me. tid: %d, remaining_time: %d, globalTime: %d\n", tid, remaining_time, globalTime);
        pthread_cond_broadcast(&condFullQueue);
        pthread_cond_signal(&condCpuBurst);
    }
    pthread_mutex_unlock(&mutexFullQueue);

    printf("after if else cpu_me. tid: %d, remaining_time: %d, globalTime: %d\n", tid, remaining_time, globalTime);
    // make all the threads wait using pthread_cond_wait until the thread with the lowest arrival time is processed
    pthread_mutex_lock(&mutexEnqueue);
    while (peekPriority() != tid){
        display();
        printf("inside while loop cpu_me . tid: %d, remaining_time: %d, globalTime: %d, peekPriority(): %d\n", tid, remaining_time, globalTime, peekPriority());
        pthread_cond_wait(&condCpuBurst, &mutexEnqueue);
    }
    // if (current_time>globalTime){
    //     globalTime = ceil(current_time);
    // }
    if (remaining_time>0 ){
        globalTime++;
        
        updateQueue(tid, remaining_time-1);
        // queue[peekPriority()].remaining_time--;
    }
    int endTime = globalTime;
    printf("after while loop before pthread_cond_broadcast. tid: %d, remaining_time: %d, globalTime: %d\n", tid, remaining_time, globalTime);
    pthread_cond_broadcast(&condCpuBurst);
    printf("after pthread_cond_broadcast - condCpuBurst. tid: %d, remaining_time: %d, globalTime: %d\n", tid, remaining_time, globalTime);
    pthread_mutex_unlock(&mutexEnqueue);
    printf("after mutexCpuBurst unlock. tid: %d, remaining_time: %d, globalTime: %d\n", tid, remaining_time, globalTime);
    // pthread_mutex_unlock(&mutexDequeue);
    // pthread_cond_broadcast(&condDequeue);
    printf("Ending globalTime: %d, tid: %d, endTime: %d \n", globalTime, tid, endTime);
    
    return endTime;
}


/* *********************************************************
    // This is the code for io_me. Half of this stuff is
    // straight up a copy and paste from cpu_me's FIFO


********************************************************* */

//implementing a circular FIFO queue. FIFO stands for First In First Out. 
void enqueueFIFO_IO1(int arrivalTime, int tid,int device_id) {
    /*
    inserts an element to the queue based on arrival time. lower arrival time is given higher priority
    */
    if (rear_IO1 == -1) {
        front_IO1 = 0;
        //current_Pointer_IO1 = 0;
        rear_IO1 = 0;
    } 
    else {
        rear_IO1++;
    }

    int i = front_IO1;
    while (arrivalTime > queue_IO1[i].arrival_time && i < rear_IO1) {
        i++;
    }
    // if arrival times are same, then the one with lower tid is given higher priority
    if (arrivalTime == queue_IO1[i].arrival_time) {
        while (tid > queue_IO1[i].tid && i < rear_IO1) {
            i++;
        }
    }
    //moving all the queue elements one position ahead
    for (int j = rear_IO1; j > i; j--) {
        queue_IO1[j] = queue_IO1[j - 1];
    }
    //inserting the element
    queue_IO1[i].tid = tid;
    queue_IO1[i].arrival_time = arrivalTime;
    //queue_IO1[i].arrival_time = arrivalTime;
    // display();
}

void dequeueFIFO_IO1(int tid) {
    /*
    removes element with the given tid from the queue
    */
    if (front_IO1 == -1) {
        printf("Dequeue: IO Queue is empty\n");
        return;
    }
    //searching for the element with the given tid in the queue and moving all the queue elements from that position one position ahead
    for (int i = front_IO1; i <= rear_IO1; i++) {
        if (queue_IO1[i].tid == tid) {
            for (int j = i; j < rear_IO1; j++) {
                queue_IO1[j] = queue_IO1[j + 1];
            }
            break;
        }
    }
    //decrementing rear
    rear_IO1--;
    //if rear is -1 then queue is empty
    if (rear_IO1 == -1) {
        front_IO1 = -1;
    }
    // printf("Thread %d dequeued at time %d \n", tid, globalTime);
    // display();


}


bool checkQueue_IO1(int tid){
    if (front_IO1 == -1) {
        printf("Check %d: IO Queue is empty\n", tid);
        return false;
    }
    for (int i = front_IO1; i <= rear_IO1; i++) {
        // printf("tid: %d, queue[i].tid: %d \n i: %d, front: %d, rear: %d\n", tid, queue[i].tid, i, front, rear);
        if(queue_IO1[i].tid == tid){
            return true;
        }
    }
    return false;
}

int peek_IO1() {
    if (front_IO1 == -1) {
        printf("Queue is empty\n");
        return -1;
    }
    for(int i = front_IO1;i<=rear_IO1;i++){
        if(queue_IO1[i].end_time == 0){
            return queue_IO1[i].tid;
        }
    }
    //return queue_IO1[current_Pointer_IO1].tid;
}

int update_IO1_operating(int tid){

    if (front_IO1 == -1) {
        printf("Queue is empty\n");
        return false;
    }
    int largest_end_time = globalTime;
    for (int i = front_IO1; i <= rear_IO1; i++) {
        // printf("tid: %d, queue[i].tid: %d \n i: %d, front: %d, rear: %d\n", tid, queue[i].tid, i, front, rear);
        if(queue_IO1[i].end_time >= largest_end_time){
            largest_end_time = queue_IO1[i].end_time;
        }
    }
    for (int i = front_IO1; i <= rear_IO1; i++) {
        // printf("tid: %d, queue[i].tid: %d \n i: %d, front: %d, rear: %d\n", tid, queue[i].tid, i, front, rear);
        if(queue_IO1[i].end_time == 0 && queue_IO1[i].tid == tid){
            queue_IO1[i].end_time = largest_end_time+IO_DEVICE_1_TICKS;
            return queue_IO1[i].end_time;
        }
    }
    pthread_cond_broadcast(&condCpuBurst); // New thing got blocked, recheck
    printf("Tid %d not found", tid);
    return false;
}



/* *********************************************************
    // This is the code for io_me, for device 2. Half of this stuff is
    // straight up a copy and paste from cpu_me's FIFO


********************************************************* */



//implementing a circular FIFO queue. FIFO stands for First In First Out. 
void enqueueFIFO_IO2(int arrivalTime, int tid,int device_id) {
    /*
    inserts an element to the queue based on arrival time. lower arrival time is given higher priority
    */
    if (rear_IO2 == -1) {
        front_IO2 = 0;

        rear_IO2 = 0;
    } 
    else {
        rear_IO2++;
    }

    int i = front_IO2;
    while (arrivalTime > queue_IO2[i].arrival_time && i < rear_IO2) {
        i++;
    }
    // if arrival times are same, then the one with lower tid is given higher priority
    if (arrivalTime == queue_IO2[i].arrival_time) {
        while (tid > queue_IO2[i].tid && i < rear_IO2) {
            i++;
        }
    }
    //moving all the queue elements one position ahead
    for (int j = rear_IO2; j > i; j--) {
        queue_IO2[j] = queue_IO2[j - 1];
    }
    //inserting the element
    queue_IO2[i].tid = tid;
    queue_IO2[i].arrival_time = arrivalTime;
    // display();
}

void dequeueFIFO_IO2(int tid) {
    /*
    removes element with the given tid from the queue
    */
    if (front_IO2 == -1) {
        printf("Dequeue: IO Queue is empty\n");
        return;
    }
    //searching for the element with the given tid in the queue and moving all the queue elements from that position one position ahead
    for (int i = front_IO2; i <= rear_IO2; i++) {
        if (queue_IO2[i].tid == tid) {
            for (int j = i; j < rear_IO2; j++) {
                queue_IO2[j] = queue_IO2[j + 1];
            }
            break;
        }
    }
    //decrementing rear
    rear_IO2--;
    //if rear is -1 then queue is empty
    if (rear_IO2 == -1) {
        front_IO2 = -1;
    }
    // printf("Thread %d dequeued at time %d \n", tid, globalTime);
    // display();


}


bool checkQueue_IO2(int tid){
    if (front_IO2 == -1) {
        printf("Check %d: IO Queue is empty\n", tid);
        return false;
    }
    for (int i = front_IO2; i <= rear_IO2; i++) {
        // printf("tid: %d, queue[i].tid: %d \n i: %d, front: %d, rear: %d\n", tid, queue[i].tid, i, front, rear);
        if(queue_IO2[i].tid == tid){
            return true;
        }
    }
    return false;
}

int peek_IO2() {
    if (front_IO2 == -1) {
        printf("Queue is empty\n");
        return -1;
    }
    for(int i = front_IO2;i<=rear_IO2;i++){
        if(queue_IO2[i].end_time == 0){
            return queue_IO2[i].tid;
        }
    }
    return -1;
}

int update_IO2_operating(int tid){

    if (front_IO2 == -1) {
        printf("Queue is empty\n");
        return false;
    }
    int largest_end_time = globalTime;
    for (int i = front_IO2; i <= rear_IO2; i++) {
        // printf("tid: %d, queue[i].tid: %d \n i: %d, front: %d, rear: %d\n", tid, queue[i].tid, i, front, rear);
        if(queue_IO2[i].end_time >= largest_end_time){
            largest_end_time = queue_IO2[i].end_time;
        }
    }
    for (int i = front_IO2; i <= rear_IO2; i++) {
        // printf("tid: %d, queue[i].tid: %d \n i: %d, front: %d, rear: %d\n", tid, queue[i].tid, i, front, rear);
        if(queue_IO2[i].end_time == 0 && queue_IO2[i].tid == tid){
            queue_IO2[i].end_time = largest_end_time+IO_DEVICE_2_TICKS;
            return queue_IO2[i].end_time;
        }
    }
    pthread_cond_broadcast(&condCpuBurst); // New thing got blocked, recheck
    printf("Tid %d not found", tid);
    return false;
}


int io_me(float current_time, int tid, int device_id) {
     // TODO: Implement this
    // printf("--------------------------------------------\n");
    // printf("Beginning globalTime: %d \n", globalTime);
    // Enqueue the thread if it is not already in the queue.
    int endTime;
    printf("device_id %d", device_id);
    if(device_id == 1){
        pthread_mutex_lock(&mutexEnqueue_IO1);
        if (checkQueue_IO1(tid) == false){
            enqueueFIFO_IO1(current_time, tid, device_id);
            threadsEnqueued_IO1++;
        }
        printf("IO mutexEnqueue locked after if. tid: %d, device_id: %d, globalTime: %d\n", tid, device_id, globalTime);
        pthread_mutex_unlock(&mutexEnqueue_IO1);

        // make all the threads wait using pthread_cond_wait until the thread with the lowest arrival time is processed
        pthread_mutex_lock(&mutexIO1Burst);
        while (peek_IO1() != tid){
            display_IO1();
            printf("inside while loop io_me . tid: %d, device_id: %d, globalTime: %d, peek: %d\n", tid, device_id, globalTime, peek_IO1());
            pthread_cond_wait(&condIO1Burst, &mutexIO1Burst);

        }
        
        // TODO: WE TOTALLY HAVEN'T CONSIDERED CASES WHEN IO DEVICE IS IN USE!!!
        endTime = update_IO1_operating(tid);
        //current_Pointer_IO1++;
        // printf("after while loop before pthread_cond_broadcast. tid: %d, remaining_time: %d, globalTime: %d\n", tid, remaining_time, globalTime);
        pthread_cond_broadcast(&condIO1Burst);
        
        // printf("after pthread_cond_broadcast - condCpuBurst. tid: %d, remaining_time: %d, globalTime: %d\n", tid, remaining_time, globalTime);
        pthread_mutex_unlock(&mutexIO1Burst);
        // printf("after mutexCpuBurst unlock. tid: %d, remaining_time: %d, globalTime: %d\n", tid, remaining_time, globalTime);
        // pthread_mutex_unlock(&mutexDequeue);
        // pthread_cond_broadcast(&condDequeue);
        // printf("Ending globalTime: %d \n", globalTime);
    } else if(device_id ==2) {
        pthread_mutex_lock(&mutexEnqueue_IO2);
        if (checkQueue_IO2(tid) == false){
            enqueueFIFO_IO2(current_time, tid, device_id);
            threadsEnqueued_IO2++;
        }
        printf("IO2 mutexEnqueue locked after if. tid: %d, device_id: %d, globalTime: %d\n", tid, device_id, globalTime);
        pthread_mutex_unlock(&mutexEnqueue_IO2);

        // make all the threads wait using pthread_cond_wait until the thread with the lowest arrival time is processed
        pthread_mutex_lock(&mutexIO2Burst);
        while (peek_IO2() != tid){
            // display();
            printf("inside while loop io_me . tid: %d, device_id: %d, globalTime: %d, peek: %d\n", tid, device_id, globalTime, peek());
            pthread_cond_wait(&condIO2Burst, &mutexIO2Burst);

        }
        
        // TODO: WE TOTALLY HAVEN'T CONSIDERED CASES WHEN IO DEVICE IS IN USE!!!
        endTime = update_IO2_operating(tid);
        // printf("after while loop before pthread_cond_broadcast. tid: %d, remaining_time: %d, globalTime: %d\n", tid, remaining_time, globalTime);
        pthread_cond_broadcast(&condIO2Burst);
        
        // printf("after pthread_cond_broadcast - condCpuBurst. tid: %d, remaining_time: %d, globalTime: %d\n", tid, remaining_time, globalTime);
        pthread_mutex_unlock(&mutexIO2Burst);
        // printf("after mutexCpuBurst unlock. tid: %d, remaining_time: %d, globalTime: %d\n", tid, remaining_time, globalTime);
        // pthread_mutex_unlock(&mutexDequeue);
        // pthread_cond_broadcast(&condDequeue);
        // printf("Ending globalTime: %d \n", globalTime);
    }
    
    return endTime;
    return 0;
}


int check_performing_io(int tid){

    if (front_IO1 == -1 && front_IO2 == -1) {
        printf("IO Queue is empty\n");
        return 0;
    }
    bool checkIO1 = true;
    bool checkIO2 = true;
    if (front_IO1 == -1){
        checkIO1 = false;
    }
    if (front_IO2 == -1){
        checkIO2 = false;
    }
    int stuckValue1 = 0;
    int stuckValue2 = 0;
    if(checkIO1){
        for (int i = front_IO1; i <= rear_IO1; i++) {
            // printf("tid: %d, queue[i].tid: %d \n i: %d, front: %d, rear: %d\n", tid, queue[i].tid, i, front, rear);
            if(queue_IO1[i].tid == tid){
                if(queue_IO1[i].end_time <=globalTime || queue_IO1[i].arrival_time >globalTime){
                    break;
                }
                stuckValue1 = queue_IO1[i].end_time;
                break;
            }
        }
    }
    if(checkIO2){
        for (int i = front_IO2; i <= rear_IO2; i++) {
            // printf("tid: %d, queue[i].tid: %d \n i: %d, front: %d, rear: %d\n", tid, queue[i].tid, i, front, rear);
            if(queue_IO2[i].tid == tid){
                if(queue_IO2[i].end_time <=globalTime || queue_IO1[i].arrival_time >globalTime){
                    break;
                }
                stuckValue2 = queue_IO2[i].end_time;
                break;
            }
        }
    }
    if (stuckValue1 > stuckValue2){
        return stuckValue1;
    }
    return stuckValue2;
}

void end_me(int tid) {
    // TODO: Implement this
    printf("inside end_me. tid: %d, globalTime: %d\n", tid, globalTime);
    pthread_mutex_lock(&mutexEnqueue);
    printf("inside end_me and inside mutexEnqueue lock. tid: %d, globalTime: %d\n", tid, globalTime);
    dequeueFIFO(tid);
    if (tidBoolArray[tid].called!=1){
        threadsEnqueued++;
        tidBoolArray[tid].called = 1;
        if (threadsEnqueued == totalThreads){
            pthread_cond_broadcast(&condFullQueue);
            // pthread_cond_broadcast(&condFullQueue);
            // pthread_cond_signal(&condCpuBurst);
        }

    }
    printf("inside end_me and after dequeueFIFO. tid: %d, globalTime: %d\n", tid, globalTime);
    pthread_cond_broadcast(&condCpuBurst);
    printf("inside end_me and after pthread_cond_broadcast. tid: %d, globalTime: %d\n", tid, globalTime);
    pthread_mutex_unlock(&mutexEnqueue);
    printf("inside end_me and after mutexEnqueue unlock. tid: %d, globalTime: %d\n", tid, globalTime);


    pthread_mutex_lock(&mutexIO1Burst);
    //dequeueFIFO_IO1(tid);
    pthread_cond_broadcast(&condIO1Burst);
    pthread_mutex_unlock(&mutexIO1Burst);

    pthread_mutex_lock(&mutexIO2Burst);
    //dequeueFIFO_IO2(tid);
    pthread_cond_broadcast(&condIO2Burst);
    pthread_mutex_unlock(&mutexIO2Burst);

}
