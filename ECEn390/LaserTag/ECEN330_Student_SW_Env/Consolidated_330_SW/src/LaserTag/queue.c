/*
 * queue.c
 *
 */
#include "queue.h"
#include <stdio.h>
#include <stdlib.h>


// Standard queue implementation that leaves one spot empty so easier to check for full/empty.
void queue_init(queue_t* q, queue_size_t size) {
  q->indexIn = 0;
  q->indexOut = 0;
  q->elementCount = 0;
  q->size = size+1;	// Add one additional location for the empty location.
  q->data = (queue_data_t *) malloc(size * sizeof(queue_data_t));
}

// Just free the malloc'd storage.
void gueue_garbageCollect(queue_t* q) {
  free(q->data);
}

// Returns the size of the queue..
queue_size_t queue_size(queue_t* q){
	return q->size;
}

// Returns true if the queue is full.
bool queueFull(queue_t* q){
	return q->elementCount ==  q->size;

}

// Returns true if the queue is empty.
bool queue_empty(queue_t* q){
	return !q->elementCount;

}

// Pushes a new element into the queue. Reports an error if the queue is full.
void queue_push(queue_t* q, queue_data_t value){

	if(queueFull()){
		printf("Error: Queue is full\n\r");
	}else{
		q->data[q->indexIn]= value;
		q->indexIn++;
		q->elementCount++;
	}
}

// Removes the oldest element in the queue.
queue_data_t queue_pop(queue_t* q){
	if(queue_empty()){
		printf("Error: the queue is empty");
	}else{
		q->data[q->indexOut]= 0;
		q->indexOut++;
	}
}

// Pushes a new element into the queue, making room by removing the oldest element.
void queue_overwritePush(queue_t* q, queue_data_t value){

	if(queueFull()){
		q->data[q->indexOut]= 0;
		q->indexOut++;
	}


}

// Provides random-access read capability to the queue.
// Low-valued indexes access older queue elements while higher-value indexes access newer elements
// (according to the order that they were added).
queue_data_t queue_readElementAt(queue_t* q, queue_index_t index){




}

// Returns a count of the elements currently contained in the queue.
queue_size_t queue_elementCount(queue_t* q){




}

// Prints the current contents of the queue. Handy for debugging.
void queue_print(queue_t* q){



}

// Performs a comprehensive test of all queue functions.
int queue_runTest(){




}

