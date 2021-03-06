#include "processor.h"


// constructor
fetch_queue::fetch_queue(unsigned int size) {
   q = new unsigned int[size];
   this->size = size;
   head = 0;
   tail = 0;
   length = 0;
}

// returns true if the fetch queue has enough space for i instructions
bool fetch_queue::enough_space(unsigned int i) {
   assert(length <= size);
   return((size - length) >= i);
}

// push an instruction (its payload buffer index) into the fetch queue
void fetch_queue::push(unsigned int index) {
   assert(length < size);

   q[tail] = index;

   tail = MOD_S((tail + 1), size);
   length += 1;
}

// returns true if the fetch queue has enough instructions to form a bundle of i instructions
bool fetch_queue::bundle_ready(unsigned int i) {
   return(length >= i);
}

// pop an instruction (its payload buffer index) from the fetch queue
unsigned int fetch_queue::pop() {
   unsigned int index;

   assert(length > 0);

   index = q[head];
   
   head = MOD_S((head + 1), size);
   length -= 1;

   return(index);
}

// flush the fetch queue (make it empty)
void fetch_queue::flush() {
   head = 0;
   tail = 0;
   length = 0;
}
