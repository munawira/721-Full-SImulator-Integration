#include "processor.h"


void processor::execute(unsigned int lane_number) {
   unsigned int index;
   bool hit;		// load value available

   // Check if there is an instruction in the Execute Stage of the specified Execution Lane.
   if (Execution_Lanes[lane_number].ex.valid) {

      //////////////////////////////////////////////////////////////////////////////////////////////////////////
      // Get the instruction's index into PAY.
      //////////////////////////////////////////////////////////////////////////////////////////////////////////
      index = Execution_Lanes[lane_number].ex.index;

      //////////////////////////////////////////////////////////////////////////////////////////////////////////
      // Execute the instruction.
      // * Load and store instructions use the AGEN and Load/Store Units.
      // * All other instructions use the ALU.
      //////////////////////////////////////////////////////////////////////////////////////////////////////////
      if (IS_MEM_OP(PAY.buf[index].flags)) {
         // Perform AGEN to generate address.
	 if (!PAY.buf[index].split_store || PAY.buf[index].upper)
	    agen(index);

	 // Execute the load or store in the LSU.
	 if (IS_LOAD(PAY.buf[index].flags)) {
	    hit = LSU.load_addr(cycle,
				PAY.buf[index].addr,
				PAY.buf[index].LQ_index,
				PAY.buf[index].SQ_index, PAY.buf[index].SQ_phase,
				PAY.buf[index].B_value.w[0],	// LWL/LWR
				PAY.buf[index].C_value.w[0],
				PAY.buf[index].C_value.w[1]);

	    // FIX_ME #13
	    // If the load hit:
	    // (1) Broadcast its destination tag to the appropriate IQ to wakeup its dependent instructions.
	    // (2) Set the corresponding ready bit in the Physical Register File Ready Bit Array.
	    // (3) Write the value into the appropriate Physical Register File. Doing this here, instead of in WB,
	    //     properly simulates the bypass network.
	    //
	    // If it didn't hit, it will get replayed later from within the LSU ("load_replay").
	    //
	    // Tips:
	    // 1. At this point of the code, 'index' is the instruction's index into PAY.buf[] (payload).
	    // 2. Background: The code above attempts to execute the load instruction in the LSU.
	    //    The load may hit (a value is obtained from either the SQ or D$) or not hit (disambiguation stall or D$ miss stall).
	    //    The local variable 'hit' indicates which case occurred. Recall, since we didn't know in the Register Read Stage
	    //    whether or not the load would hit, we didn't speculatively wakeup dependent instructions in that Stage. Now we know
	    //    if it hit or not. If it did hit, we need to not only write the load value into the Physical Register File, but also
	    //    wakeup dependent instructions and set the destination register's ready bit -- two tasks that were deferred until we
	    //    knew for certain that we could.
	    // 3. ONLY if the load instruction has hit (check local variable 'hit', already declared and set above):
	    //    a. Wakeup dependents in the relevant IQ using its wakeup() port (see issue_queue.h for arguments
	    //       to the wakeup port). The relevant IQ is determined by the type of the instruction's
	    //       destination register (integer vs. floating-point).
	    //    b. Set the destination register's ready bit. Remember to use the relevant renamer module,
	    //       determined by the type of the instruction's destination register (integer vs. floating-point).
	    //    c. Write the doubleword value of the destination register (now available in the instruction's payload, which was
	    //       provided by the LSU via the code above) into the relevant Physical Register File. The relevant Physical
	    //       Register File is determined by the type of the instruction's destination register (integer vs. floating-point).
	    //       Note: Values in the payload use a union type (can be referenced as either a single doubleword or as two words
	    //       separately); see the comments in file payload.h regarding referencing a value as a single doubleword.



	 }
	 else {
	    assert(IS_STORE(PAY.buf[index].flags));

	    if (PAY.buf[index].split_store) {
	       assert(PAY.buf[index].split);
	       if (PAY.buf[index].upper)
	          LSU.store_addr(cycle, PAY.buf[index].addr, PAY.buf[index].SQ_index, PAY.buf[index].LQ_index, PAY.buf[index].LQ_phase); // upper op: address
	       else
		  LSU.store_value(PAY.buf[index].SQ_index, PAY.buf[index].A_value.w[0], PAY.buf[index].A_value.w[1]);		// lower op: float.pt. value
	    }
	    else {
	       // If not a split-store, then the store has both the address and the value.
	       LSU.store_addr(cycle, PAY.buf[index].addr, PAY.buf[index].SQ_index, PAY.buf[index].LQ_index, PAY.buf[index].LQ_phase);
	       if (SS_OPCODE(PAY.buf[index].inst) == DSZ)
	          LSU.store_value(PAY.buf[index].SQ_index, (SS_WORD_TYPE)0, (SS_WORD_TYPE)0);
	       else
	          LSU.store_value(PAY.buf[index].SQ_index, PAY.buf[index].B_value.w[0], PAY.buf[index].B_value.w[1]);
	    }
	 }
      }
      else {
	 // Execute the ALU type instruction on the ALU.
	 alu(index);

	 // FIX_ME #14
	 // If the ALU type instruction has a destination register (not a branch),
	 // then write its result value into the appropriate Physical Register File.
	 // Doing this here, instead of in WB, properly simulates the bypass network.
	 //
	 // Tips:
	 // 1. At this point of the code, 'index' is the instruction's index into PAY.buf[] (payload).
	 // 2. If the ALU type instruction has a destination register, then write the doubleword value of the
	 //    destination register (now available in the instruction's payload, which was provided by alu()
	 //    via the code above) into the relevant Physical Register File. The relevant Physical Register
	 //    File is determined by the type of the instruction's destination register (integer vs. floating-point).
	 //    Note: Values in the payload use a union type (can be referenced as either a single doubleword or as two words
	 //    separately); see the comments in file payload.h regarding referencing a value as a single doubleword.



      }

      ////////////////////////////////////////////////////////////////////////////////////////////////////
      // NOTE: Setting the completed bit in the Active List is deferred to the Writeback Stage.
      // NOTE: Resolving branches is deferred to the Writeback Stage.
      ////////////////////////////////////////////////////////////////////////////////////////////////////

      //////////////////////////////////////////////////////////////////////////////////////////////////////////
      // Advance the instruction to the Writeback Stage.
      //////////////////////////////////////////////////////////////////////////////////////////////////////////

      // There must be space in the Writeback Stage because Execution Lanes are free-flowing.
      assert(!Execution_Lanes[lane_number].wb.valid);

      // Copy instruction to Writeback Stage.
      // BUT: Stalled loads should not advance to the Writeback Stage.
      if (!IS_LOAD(PAY.buf[index].flags) || hit) {
         Execution_Lanes[lane_number].wb.valid = true;
         Execution_Lanes[lane_number].wb.index = Execution_Lanes[lane_number].ex.index;
         Execution_Lanes[lane_number].wb.branch_mask = Execution_Lanes[lane_number].ex.branch_mask;
      }

      // Remove instruction from Execute Stage.
      Execution_Lanes[lane_number].ex.valid = false;
   }
}
