#include "processor.h"


void processor::schedule() {
   // Just for now:
   // (1) The Execution Lanes are split evenly between the integer and floating-point Issue Queues.
   //     For now, this means the issue_width must be even.
   // (2) Within each domain (integer / floating-point), an Execution Lane is universal.

   unsigned int half;
   
   assert((issue_width & 1) == 0);	// issue_width must be even (for now)
   half = (issue_width >> 1);		// issue_width divided by 2

   IQ_INT.select_and_issue(half, &(Execution_Lanes[0]));	// Issue instructions from integer IQ to the lower half of the Execution Lanes.
   IQ_FP.select_and_issue(half, &(Execution_Lanes[half]));	// Issue instructions from floating-point IQ to the upper half of the Execution Lanes.
}
