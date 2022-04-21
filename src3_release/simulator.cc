#include "processor.h"
#include "globals.h"


// Change function names to be more meaningful:
//	     NEW	       OLD
#define simulator_init	trace_consume_init
#define simulator	trace_consume


void simulator_init() {
  ///////////////////////////////////////////
  // Create the processor(s).
  ///////////////////////////////////////////
  PROC = new processor *[NumThreads];
  for (unsigned int i = 0; i < NumThreads; i++) {
     PROC[i] = new processor(FETCH_QUEUE_SIZE,
			     NUM_CHECKPOINTS,
			     ACTIVE_LIST_SIZE,
			     ISSUE_QUEUE_SIZE,
			     LQ_SIZE,
			     SQ_SIZE,
			     FETCH_WIDTH,
			     DISPATCH_WIDTH,
			     ISSUE_WIDTH,
			     RETIRE_WIDTH,
			     i);
  }

  ///////////////////////////////////////////
  // Fill debug buffers for all threads.
  ///////////////////////////////////////////
  for (unsigned int i = 0; i < NumThreads; i++)
     THREAD[i]->run_ahead();
}	// simulator_init()


void simulator() {
   unsigned int i;
   unsigned int lane_number;

   ////////////////////////////////
   // Main simulator loop.
   ////////////////////////////////
   while (1) {
      for (i = 0; i < NumThreads; i++) {
	 /////////////////////////////////////////////////////////////
	 // Pipeline.
	 /////////////////////////////////////////////////////////////

	 PROC[i]->retire();						// Retire Stage
	 for (lane_number = 0; lane_number < ISSUE_WIDTH; lane_number++)
	    PROC[i]->writeback(lane_number);				// Writeback Stage
	 for (lane_number = 0; lane_number < ISSUE_WIDTH; lane_number++)
	    PROC[i]->execute(lane_number);				// Execute Stage
	 for (lane_number = 0; lane_number < ISSUE_WIDTH; lane_number++)
	    PROC[i]->register_read(lane_number);			// Register Read Stage
	 PROC[i]->schedule();						// Schedule Stage
	 PROC[i]->dispatch();						// Dispatch Stage
	 PROC[i]->rename2();						// Rename Stage
	 PROC[i]->rename1();						// Rename Stage
	 PROC[i]->decode();						// Decode Stage
	 PROC[i]->fetch();						// Fetch Stage

	 /////////////////////////////////////////////////////////////
	 // Miscellaneous stuff that must be processed every cycle.
	 /////////////////////////////////////////////////////////////

	 // Go to the next simulator cycle.
	 PROC[i]->next_cycle();

	 // For detecting deadlock, and monitoring progress.
	 if (MOD(PROC[i]->cycle, 0x400000) == 0) {
	    INFO("Thread %d: (cycle = %x) num_insn = %.0f\tIPC = %.2f",
		 i,
		 (unsigned int)PROC[i]->cycle,
		 (double)PROC[i]->num_insn,
		 (double)PROC[i]->num_insn/(double)PROC[i]->cycle);
	    fflush(fp_info);
	 }
      }
   }
}	// simulator()
