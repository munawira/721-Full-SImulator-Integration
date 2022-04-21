#include "processor.h"


// constructor
issue_queue::issue_queue(unsigned int size) {
   // Initialize the issue queue.
   q = new issue_queue_entry_t[size];
   this->size = size;
   length = 0;
   for (unsigned int i = 0; i < size; i++)
      q[i].valid = false;

   // Initialize the issue queue's free list.
   fl = new unsigned int[size];
   fl_head = 0;
   fl_tail = 0;
   fl_length = size;
   for (unsigned int i = 0; i < size; i++)
      fl[i] = i;
}

bool issue_queue::stall(unsigned int bundle_inst) {
   assert((length + fl_length) == size);
   return(fl_length < bundle_inst);
}

void issue_queue::dispatch(unsigned int index, unsigned long long branch_mask, bool A_valid, bool A_ready, unsigned int A_tag, bool B_valid, bool B_ready, unsigned int B_tag) {
   unsigned int free;

   // Assert there is a free issue queue entry.
   assert(fl_length > 0);
   assert(length < size);

   // Pop a free issue queue entry.
   free = fl[fl_head];
   fl_head = MOD_S((fl_head + 1), size);
   fl_length--;

   // Dispatch the instruction into the free issue queue entry.
   length++;
   assert(!q[free].valid);
   q[free].valid = true;
   q[free].index = index;
   q[free].branch_mask = branch_mask;
   q[free].A_valid = A_valid;
   q[free].A_ready = A_ready;
   q[free].A_tag = A_tag;
   q[free].B_valid = B_valid;
   q[free].B_ready = B_ready;
   q[free].B_tag = B_tag;
}

void issue_queue::wakeup(unsigned int tag) {
   // Broadcast the tag to every entry in the issue queue.
   // If the broadcasted tag matches a valid tag:
   // (1) Assert that the ready bit is initially false.
   // (2) Set the ready bit.
   for (unsigned int i = 0; i < size; i++) {
      if (q[i].valid) {					// Only consider valid issue queue entries.
         if (q[i].A_valid && (tag == q[i].A_tag)) {	// Check first source operand.
	    assert(!q[i].A_ready);
	    q[i].A_ready = true;
	 }
         if (q[i].B_valid && (tag == q[i].B_tag)) {	// Check second source operand.
	    assert(!q[i].B_ready);
	    q[i].B_ready = true;
	 }
      }
   }
}

void issue_queue::select_and_issue(unsigned int num_lanes, lane *Execution_Lanes) {
   unsigned int i;
   unsigned int num_issued;

   // Attempt to issue as many instructions as there are Execution Lanes.
   for (i = 0, num_issued = 0; (i < size) && (num_issued < num_lanes); i++) {
      if (q[i].valid && (!q[i].A_valid || q[i].A_ready) && (!q[i].B_valid || q[i].B_ready)) {
	 // Issue the instruction to the Register Read Stage within the next Execution Lane.
	 assert(!Execution_Lanes[num_issued].rr.valid);
	 Execution_Lanes[num_issued].rr.valid = true;
	 Execution_Lanes[num_issued].rr.index = q[i].index;
	 Execution_Lanes[num_issued].rr.branch_mask = q[i].branch_mask;
         num_issued++;

         // Remove the instruction from the issue queue.
	 remove(i);
      }
   }
}

void issue_queue::remove(unsigned int i) {
	 assert(length > 0);
	 assert(fl_length < size);

         // Remove the instruction from the issue queue.
	 q[i].valid = false;
	 length--;

         // Push the issue queue entry back onto the free list.
	 fl[fl_tail] = i;
	 fl_tail = MOD_S((fl_tail + 1), size);
	 fl_length++;
}

void issue_queue::flush() {
   length = 0;
   for (unsigned int i = 0; i < size; i++)
      q[i].valid = false;

   fl_head = 0;
   fl_tail = 0;
   fl_length = size;
   for (unsigned int i = 0; i < size; i++)
      fl[i] = i;
}

void issue_queue::clear_branch_bit(unsigned int branch_ID) {
   for (unsigned int i = 0; i < size; i++)
      CLEAR_BIT(q[i].branch_mask, branch_ID);
}

void issue_queue::squash(unsigned int branch_ID) {
   for (unsigned int i = 0; i < size; i++) {
      if (q[i].valid && BIT_IS_ONE(q[i].branch_mask, branch_ID))
         remove(i);
   }
}
