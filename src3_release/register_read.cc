#include "processor.h"


void processor::register_read(unsigned int lane_number) {
   unsigned int index;

   // Check if there is an instruction in the Register Read Stage of the specified Execution Lane.
   if (Execution_Lanes[lane_number].rr.valid) {

      //////////////////////////////////////////////////////////////////////////////////////////////////////////
      // Get the instruction's index into PAY.
      //////////////////////////////////////////////////////////////////////////////////////////////////////////
      index = Execution_Lanes[lane_number].rr.index;

      //////////////////////////////////////////////////////////////////////////////////////////////////////////
      // FIX_ME #11
      // If the instruction has a destination register:
      // (1) Broadcast its destination tag to the appropriate IQ to wakeup its dependent instructions.
      // (2) Set the corresponding ready bit in the Physical Register File Ready Bit Array.
      //
      // HOWEVER: Load instructions conservatively delay broadcasting their destination tags until
      // their data are available, because they may stall in the Execute Stage. I.e., in the current
      // simulator implementation, loads do NOT speculatively wakeup their dependent instructions.
      //
      // Tips:
      // 1. At this point of the code, 'index' is the instruction's index into PAY.buf[] (payload).
      // 2. The easiest way to tell if this instruction is a load or not, is to test the instruction's
      //    flags (in its payload) via the IS_LOAD() macro (see processor.h).
      // 3. If the instruction is not a load and has a destination register, then:
      //    a. Wakeup dependents in the relevant IQ using its wakeup() port (see issue_queue.h for arguments
      //       to the wakeup port). The relevant IQ is determined by the type of the instruction's
      //       destination register (integer vs. floating-point).
      //    b. Set the destination register's ready bit. Remember to use the relevant renamer module,
      //       determined by the type of the instruction's destination register (integer vs. floating-point).
      //////////////////////////////////////////////////////////////////////////////////////////////////////////
 


      //////////////////////////////////////////////////////////////////////////////////////////////////////////
      // FIX_ME #12
      // Read source register(s) from the appropriate Physical Register File.
      //
      // Tips:
      // 1. At this point of the code, 'index' is the instruction's index into PAY.buf[] (payload).
      // 2. If the instruction has a first source register (A), then read its doubleword value from
      //    the relevant Physical Register File. The relevant one is determined by the type of the
      //    source register (integer vs. floating-point).
      // 3. If the instruction has a second source register (B), follow the same procedure for it.
      // 4. Be sure to record any doubleword value(s) in the instruction's payload, for use in the
      //    subsequent Execute Stage. The values in the payload use a union type (can be referenced
      //    as either a single doubleword or as two words separately); see the comments in file
      //    payload.h regarding referencing a value as a single doubleword.
      //////////////////////////////////////////////////////////////////////////////////////////////////////////



      //////////////////////////////////////////////////////////////////////////////////////////////////////////
      // Advance the instruction to the Execution Stage.
      //////////////////////////////////////////////////////////////////////////////////////////////////////////

      // There must be space in the Execute Stage because Execution Lanes are free-flowing.
      assert(!Execution_Lanes[lane_number].ex.valid);

      // Copy instruction to Execute Stage.
      Execution_Lanes[lane_number].ex.valid = true;
      Execution_Lanes[lane_number].ex.index = Execution_Lanes[lane_number].rr.index;
      Execution_Lanes[lane_number].ex.branch_mask = Execution_Lanes[lane_number].rr.branch_mask;

      // Remove instruction from Register Read Stage.
      Execution_Lanes[lane_number].rr.valid = false;
   }
}
