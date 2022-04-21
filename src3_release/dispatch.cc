#include "processor.h"


void processor::dispatch() {
   unsigned int i;
   unsigned int bundle_inst_int, bundle_inst_fp, bundle_load, bundle_store;
   unsigned int index;
   bool load_flag;
   bool store_flag;
   bool A_ready;
   bool B_ready;
   db_t *actual;
   unsigned int oracle_address;

   // Stall the Dispatch Stage if either:
   // (1) There isn't a dispatch bundle.
   // (2) There aren't enough AL entries for the dispatch bundle.
   // (3) There aren't enough IQ entries for the dispatch bundle.
   // (4) There aren't enough LQ/SQ entries for the dispatch bundle.

   // First stall condition: There isn't a dispatch bundle.
   if (!DISPATCH[0].valid)
      return;

   // FIX_ME #6
   // Second stall condition: There aren't enough AL entries for the dispatch bundle.
   // Check if the Dispatch Stage must stall due to either Active List not having sufficient entries for the dispatch bundle. (The current implementation keeps the two ALs redundant.)
   // * If either the integer Active List or floating-point Active List does not have enough entries for the *whole* dispatch bundle (which is always comprised of 'dispatch_width' instructions),
   //   then stall the Dispatch Stage. Stalling is achieved by returning from this function ('return').
   // * Else, don't stall the Dispatch Stage. This is achieved by doing nothing and proceeding to the next statements.
   //
   // Tips:
   // 1. If you need to stall, you do it by hard exiting this function with 'return'. Otherwise do nothing.



   //
   // Third and fourth stall conditions:
   // - There aren't enough IQ entries for the dispatch bundle.
   // - There aren't enough LQ/SQ entries for the dispatch bundle.
   //
   bundle_inst_int = 0;
   bundle_inst_fp = 0;
   bundle_load = 0;
   bundle_store = 0;
   for (i = 0; i < dispatch_width; i++) {
      assert(DISPATCH[i].valid);
      index = DISPATCH[i].index;

      // Check IQ requirement.
      switch (PAY.buf[index].iq) {
         case SEL_IQ_INT:
	    // Increment number of instructions to be dispatched to integer IQ.
	    bundle_inst_int++;
	    break;

	 case SEL_IQ_FP:
	    // Increment number of instructions to be dispatched to flt. pt. IQ.
	    bundle_inst_fp++;
	    break;

	 case SEL_IQ_NONE: case SEL_IQ_NONE_EXCEPTION:
	    // Skip IQ altogether.
	    break;

	 default:
	    assert(0);
            break;
      }

      // Check LQ/SQ requirement.
      if (IS_LOAD(PAY.buf[index].flags)) {
         bundle_load++;
      }
      else if (IS_STORE(PAY.buf[index].flags)) {
         // Special cases:
	 // S_S and S_D are split-stores, i.e., they are split into an addr-op and a value-op.
	 // The two ops share a SQ entry to "rejoin". Therefore, only the first op should check
	 // for and allocate a SQ entry; the second op should inherit the same entry.
	 if (!PAY.buf[index].split_store || PAY.buf[index].upper)
	    bundle_store++;
      }
   }

   // Now, check for available entries in the integer IQ, flt. pt. IQ, and LQ/SQ.
   if (IQ_INT.stall(bundle_inst_int) || IQ_FP.stall(bundle_inst_fp) || LSU.stall(bundle_load, bundle_store))
      return;


   //
   // Making it this far means we have all the required resources to dispatch the dispatch bundle.
   //
   for (i = 0; i < dispatch_width; i++) {
      assert(DISPATCH[i].valid);
      index = DISPATCH[i].index;

      // FIX_ME #7
      // Dispatch the instruction into both the integer and floating-point Active Lists, redundantly.
      // The type of the destination register determines which Active List records and manages it.
      //
      // Tips:
      // 1. At this point of the code, 'index' is the instruction's index into PAY.buf[] (payload).
      // 2. The renamer class' function for dispatching an instruction into its Active List takes seven different arguments.
      //    Four of these arguments are control flags:
      //    a. dest_valid: The instruction's payload has information about whether or not the instruction
      //       has a destination register. In addition, as mentioned above, only one of the Active Lists
      //       should record and manage the destination register: if the type of the destination register
      //       is integer, then only the integer Active List should perceive a valid destination register;
      //       if it is floating-point, then only the floating-point Active List should perceive a valid
      //       destination register.
      //    b. load: You can efficiently detect loads by testing the instruction's flags with the IS_LOAD() macro,
      //       as shown below. Use the local variable 'load_flag' (already declared):
      //       load_flag = IS_LOAD(PAY.buf[index].flags);
      //    c. store: You can efficiently detect stores by testing the instruction's flags with the IS_STORE() macro,
      //       as shown below. A complication is that split-stores (implemented for floating-point stores) take
      //       two entries in the Active List but a unified entry in the LSU. Therefore, only the second entry should
      //       signal retirement of the split-store to the LSU because that is when the split-store
      //       as a whole may leave the LSU. Because of these intricate details, the 'store' flag is
      //       calculated for you below. Use the local variable 'store_flag' (already declared):
      //       store_flag = IS_STORE(PAY.buf[index].flags) && !(PAY.buf[index].split_store && PAY.buf[index].upper);
      //    d. branch: This is not currently used so just pass in 'false'. (We'll adjust later.)
      // 3. When you dispatch the instruction into both Active Lists, remember to *update* the instruction's
      //    payload with its integer Active List index and its floating-point Active List index.



      // FIX_ME #8
      // Determine initial ready bits for the instruction's two source registers.
      // These will be used to initialize the instruction's ready bits in the Issue Queue.
      //
      // Tips:
      // 1. At this point of the code, 'index' is the instruction's index into PAY.buf[] (payload).
      // 2. When you implement the logic for determining readiness of the two source registers, A and B,
      //    put the results of your calculations into the local variables 'A_ready' and 'B_ready'.
      //    These local variables are already declared for you. You can then use these ready flags
      //    when you dispatch the instruction into one of the Issue Queues (that code is coming up shortly).
      // 3. If the instruction doesn't have a given source register, then it should be declared ready
      //    since the Issue Queue must not wait for a non-existent register. On the other hand, if the
      //    instruction does have a given source register, then you must consult the appropriate renamer
      //    module to determine whether or not the register is ready: the type of the source register
      //    dictates which renamer module to consult (integer vs. floating-point).



      // FIX_ME #9
      // Clear the ready bit of the instruction's destination register.
      // This is needed to synchronize future consumers.
      //
      // (TANGENT: Alternatively, this could be done when the physical register is freed. This would
      // ensure newly-allocated physical registers are initially marked as not-ready, obviating the
      // need to clear their ready bits in the Dispatch Stage. It was less complex to implement this
      // alternative in the FabScalar library, by the way.)
      //
      // Tips:
      // 1. At this point of the code, 'index' is the instruction's index into PAY.buf[] (payload).
      // 2. If the instruction has a destination register, then clear its ready bit; otherwise do nothing.
      //    Remember to use the renamer module corresponding to the destination register's type.



      // FIX_ME #10
      // Dispatch the instruction into its corresponding Issue Queue (IQ_INT or IQ_FP),
      // or circumvent the Issue Queues altogether and update status in both Active Lists.
      //
      // Tips:
      // 1. At this point of the code, 'index' is the instruction's index into PAY.buf[] (payload).
      // 2. The Decode Stage has already set the 'iq' field of the instruction's payload. This field
      //    indicates which Issue Queue the instruction should use. It is an enumerated type with
      //    four possible values (refer to payload.h), corresponding to:
      //    * Use the integer Issue Queue, because source operands are integer.
      //    * Use the floating-point Issue Queue, because source operands are floating-point.
      //    * Skip the Issue Queues and early-complete the instruction, because it has no source or destination registers (JUMP).
      //    * Skip the Issue Queues and early-complete the instruction, moreover, post an exception: this is for system calls (SYSCALL and BREAK).
      //    The switch statement below enumerates the four cases for you. You must implement the code for each case.
      switch (PAY.buf[index].iq) {
         case SEL_IQ_INT:
	    // FIX_ME #10a
	    // Dispatch the instruction into the integer IQ.
	    //
	    // Tips:
	    // 1. At this point of the code, 'index' is the instruction's index into PAY.buf[] (payload).
	    // 2. You only need to implement one statement: a call to the Issue Queue's dispatch function.
	    //    See file issue_queue.h to determine the arguments that need to be passed in. Here is some clarification:
	    //    * 'index' argument: the instruction's index into PAY.buf[]
	    //    * 'branch_mask' argument: pass in the branch_mask of the instruction currently being dispatched
	    //      from the DISPATCH pipeline register, i.e., DISPATCH[i].branch_mask
	    //    * 'A_valid', 'A_ready', and 'A_tag': Valid bit, ready bit (calculated above), and physical register of first source register.
	    //    * 'B_valid', 'B_ready', and 'B_tag': Valid bit, ready bit (calculated above), and physical register of second source register.
	    // 3. Remember that this case is for instructions with integer source registers, therefore,
	    //    dispatch the instruction into the integer IQ (IQ_INT), only. As you can see in
	    //    file processor.h, the IQ_INT variable is the Issue Queue itself, NOT a pointer to it.



	    break;

	 case SEL_IQ_FP:
	    // FIX_ME #10b
	    // Dispatch the instruction into the flt. pt. IQ.
	    //
	    // Tips:
	    // 1. Same as 10a above but use IQ_FP.



	    break;

	 case SEL_IQ_NONE:
	    // FIX_ME #10c
	    // Skip IQ altogether.
	    // Set completed bit in both ALs.
	    //
	    // Tips:
	    // 1. At this point of the code, 'index' is the instruction's index into PAY.buf[] (payload).
	    // 2. Set the completed bit for this instruction in both the integer Active List and floating-point Active List.



	    break;

	 case SEL_IQ_NONE_EXCEPTION:
	    // FIX_ME #10d
	    // Skip IQ altogether.
	    // Set completed bit in both ALs.
	    // Set exception bit in both ALs.
	    //
	    // Tips:
	    // 1. At this point of the code, 'index' is the instruction's index into PAY.buf[] (payload).
	    // 2. Set the completed bit for this instruction in both the integer Active List and floating-point Active List.
	    // 3. Set the exception bit for this instruction in both the integer Active List and floating-point Active List.



	    break;

	 default:
	    assert(0);
            break;
      }

      // Dispatch loads and stores into the LQ/SQ and record their LQ/SQ indices.
      if (IS_MEM_OP(PAY.buf[index].flags)) {
	 if (!PAY.buf[index].split_store || PAY.buf[index].upper) {
	    LSU.dispatch(IS_LOAD(PAY.buf[index].flags),
			 PAY.buf[index].size,
			 PAY.buf[index].left,
			 PAY.buf[index].right,
			 PAY.buf[index].is_signed,
			 index,
			 PAY.buf[index].LQ_index, PAY.buf[index].LQ_phase,
			 PAY.buf[index].SQ_index, PAY.buf[index].SQ_phase);
	    // The lower part of a split-store should inherit the same LSU indices.
	    if (PAY.buf[index].split_store) {
	       assert(PAY.buf[index+1].split && !PAY.buf[index+1].upper);
	       PAY.buf[index+1].LQ_index = PAY.buf[index].LQ_index;
	       PAY.buf[index+1].LQ_phase = PAY.buf[index].LQ_phase;
	       PAY.buf[index+1].SQ_index = PAY.buf[index].SQ_index;
	       PAY.buf[index+1].SQ_phase = PAY.buf[index].SQ_phase;
	    }

	    // Oracle memory disambiguation support.
	    if (ORACLE_DISAMBIG && PAY.buf[index].good_instruction && IS_STORE(PAY.buf[index].flags)) {
	       // Get pointer to the corresponding instruction in the functional simulator.
	       actual = THREAD[Tid]->peek(PAY.buf[index].db_index);

	       // Determine the oracle store address.
	       if (SS_OPCODE(PAY.buf[index].inst) == DSW) {
		  assert(PAY.buf[index].split);
		  oracle_address = (PAY.buf[index].upper ?  actual->a_addr : actual->a_addr + 4);
	       }
	       else {
	          oracle_address = actual->a_addr;
	       }
	
	       // Place oracle store address into SQ before all subsequent loads are dispatched.
	       // This policy ensures loads only stall on truly-dependent stores.
	       LSU.store_addr(cycle, oracle_address, PAY.buf[index].SQ_index, PAY.buf[index].LQ_index, PAY.buf[index].LQ_phase);
	    }
	 }
      }

      // Checkpointed branches must record information for restoring the LQ/SQ when a branch misprediction resolves.
      if (PAY.buf[index].checkpoint)
         LSU.checkpoint(PAY.buf[index].LQ_index, PAY.buf[index].LQ_phase, PAY.buf[index].SQ_index, PAY.buf[index].SQ_phase);
   }


   // Remove the dispatch bundle from the Dispatch Stage.
   for (i = 0; i < dispatch_width; i++) {
      assert(DISPATCH[i].valid);
      DISPATCH[i].valid = false;
   }
}
