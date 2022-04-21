#include "processor.h"


void processor::retire() {
   unsigned int i;
   bool committed1, committed2;
   bool load1, load2;
   bool store1, store2;
   bool branch1, branch2;	// ERIC_FIX_ME
   bool exception1, exception2;
   unsigned int offending_PC1, offending_PC2;

   for (i = 0; i < retire_width; i++) {
      // FIX_ME #17
      // Call the commit() function of both the integer and floating-point renamer modules.
      // This tells each renamer module to examine the instruction at the head of its Active List,
      // and to take appropriate action, for example: do nothing if not yet complete, commit the
      // instruction if it is completed and exception-free, or recover if it is completed with a
      // posted exception. Moreover, the commit() function returns six arguments that THIS
      // function must examine and act upon.
      //
      // Tips:
      // 1. Call the integer renamer module's commit() function with the following arguments
      //    in this order: committed1, load1, store1, branch1, exception1, offending_PC1.
      //    These are local variables, already declared above.
      // 2. Call the floating-point renamer module's commit() function with the following arguments
      //    in this order: committed2, load2, store2, branch2, exception2, offending_PC2.
      //    These are local variables, already declared above.
      // 3. Study the code that follows the code that you added, and simply note the following observations:
      //    * We assert that the two committed flags match, since the Active Lists are managed redundantly.
      //    * We assert that the dual load, store, and branch flags match, but only if the instruction was committed.
      //    * We assert that the two exception flags match, since the Active Lists are managed redundantly.
      //    * We assert that the two offending_PCs match, but only if there was a posted exception.
      //    * If the instruction was committed -- "if (committed1)" -- and if the committed instruction
      //      is a memory operation (load or store) -- "if (load1 || store1)" -- we signal the LSU to commit the
      //      memory operation. For loads, this just means removing it from the LSU. For stores,
      //      this means removing it from the LSU and committing it to the L1 Data Cache.
      //    * If the instruction was not committed but it was an exception -- "else if (exception1)" --
      //      notice we perform a full pipeline flush and service the system call (the only cause of
      //      exceptions in our benchmarks).
      //    * If the instruction was not committed nor was it an exception -- "else" -- we stall retirement.



      assert(committed1 == committed2);
      if (committed1) {assert(load1 == load2); assert(store1 == store2); assert(branch1 == branch2);}
      assert(exception1 == exception2);
      if (exception1) assert(offending_PC1 == offending_PC2);

      if (committed1) {
	 // If the committed instruction is a load or store, signal the LSU to commit its oldest load or store.
	 if (load1 || store1)
	    LSU.commit(load1);

	 // If the committed instruction is a branch, signal the branch predictor to commit its oldest branch.
	 // ERIC_FIX_ME:
	 // 1. Add a branch flag to the Active List so that *it* does the signaling.
	 // 2. Change the branch predictor interface as follows: BP.commit().
	 if (!PERFECT_BRANCH_PRED && IS_BRANCH(PAY.buf[PAY.head].flags))
	    BP.verify_pred(PAY.buf[PAY.head].pred_tag, ((SS_OPCODE(PAY.buf[PAY.head].inst) != JUMP) ? PAY.buf[PAY.head].c_next_pc : PAY.buf[PAY.head].next_pc), false);

         // Check results.
	 checker();

	 // Keep track of the number of retired instructions.
	 num_insn++;

         // Pop the instruction from PAY.
	 if (PAY.buf[PAY.head].split) {
            PAY.pop();
	    // Keep track of the number of split instructions.
	    if (PAY.buf[PAY.head].upper)
	       num_insn_split++;	// ERIC_FIX_ME
	 }
	 else {
            PAY.pop();
            PAY.pop();
         }
      }
      else if (exception1) {
	 assert((SS_OPCODE(PAY.buf[PAY.head].inst) == SYSCALL) || (SS_OPCODE(PAY.buf[PAY.head].inst) == BREAK));

         // Check results.
	 checker();

	 // Keep track of the number of retired instructions.
	 num_insn++;	// syscall or break instruction

	 ////////////////////////////
	 // Service the trap.
	 ////////////////////////////

	 // Step 1: Squash the pipeline.
	 squash_complete(offending_PC1);

	 // Step 2:
	 // Func. sim. is stalled waiting to execute the trap.
	 // Signal it to proceed with the trap.
	 THREAD[Tid]->trap_now(PAY.buf[PAY.head].inst);

	 // Step 3:
	 // Registers of the timing simulator are now stale.
	 // Copy values from the functional simulator.
	 copy_reg();
	   
	 // Step 4:
	 // The memory state of the timing simulator is now stale.
	 // Copy values from the functional simulator.
	 LSU.copy_mem(THREAD[Tid]->get_mem_table());
		 	
	 // Step 5:
         // Func. sim. is waiting to resume after the trap.
         // Signal it to resume.
         THREAD[Tid]->trap_resume();

         // Flush PAY.
         PAY.clear();

	 // Stall the retire stage.
         break;
      }
      else {
	 // Neither committed nor exception: stall the Retire Stage.
         break;
      }
   }


   //////////////////////////////
   // FIX_ME #18
   // Replay stalled loads.
   //
   // There is an autonomous engine that replays one stalled load each cycle in the LSU,
   // to determine if it can unstall. The code, below, implements the autonomous replay engine.
   // If the replay succeeds, it means the load finally has a value and we can
   // (1) wakeup its dependents,
   // (2) set the ready bit of its destination register, and
   // (3) write its value into the Physical Register File.
   // We already did these three steps for loads that hit on the first attempt;
   // here we are doing them for loads that stalled and have finally become unstalled.
   //////////////////////////////
   unsigned int index;
   SS_WORD_TYPE value0, value1;
   if (LSU.load_replay(cycle, index, value0, value1)) {
      // Load has resolved.
      assert(IS_LOAD(PAY.buf[index].flags));
      assert(PAY.buf[index].C_valid);
      PAY.buf[index].C_value.w[0] = value0;
      PAY.buf[index].C_value.w[1] = value1;

      // FIX_ME #18a
      // Tips:
      // 1. At this point of the code, 'index' is the instruction's index into PAY.buf[] (payload).
      // 2. See #13 (in execute.cc), and implement steps 3a,3b,3c.



      // FIX_ME #18b
      // Set completed bits in Active Lists.
      //
      // Tips:
      // 1. At this point of the code, 'index' is the instruction's index into PAY.buf[] (payload).
      // 2. Set the completed bit for this instruction in both the integer Active List and floating-point Active List.



   }

}
