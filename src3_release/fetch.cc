#include "processor.h"


void processor::fetch() {
	unsigned int i;
	bool stop;
	SS_INST_TYPE inst;
	unsigned int pred_tag;
	unsigned int next_pc;
	unsigned int direct_target;
	bool conf;				// dummy
	bool fm;				// dummy
	unsigned int history_reg;

	unsigned int line1;				// I$
	unsigned int line2;				// I$
	bool hit1;					// I$
	bool hit2;					// I$
	SS_TIME_TYPE resolve_cycle1;			// I$
	SS_TIME_TYPE resolve_cycle2;			// I$

	unsigned int index;


	/////////////////////////////
	// Stall logic.
	/////////////////////////////

	// Stall the Fetch Stage if either:
	// 1. The Decode Stage is stalled.
	// 2. An I$ miss has not yet resolved.
	if ((DECODE[0].valid) ||			// Decode Stage is stalled.
	    (cycle < next_fetch_cycle)) 		// I$ miss has not yet resolved.
	   return;

	/////////////////////////////
	// I-cache access.
	/////////////////////////////

	if (!PERFECT_ICACHE) {
	   if (IC_INTERLEAVED) {
	      // Access two consecutive lines.
	      line1 = (pc >> L1_IC_LINE_SIZE);
	      line2 = (pc >> L1_IC_LINE_SIZE) + 1;
	      resolve_cycle1 = IC.Access(Tid, cycle, (line1 << L1_IC_LINE_SIZE), false, &hit1);
	      resolve_cycle2 = IC.Access(Tid, cycle, (line2 << L1_IC_LINE_SIZE), false, &hit2);

	      if (!hit1) {
		 if (!hit2)
	            next_fetch_cycle = MAX(resolve_cycle1, resolve_cycle2);	// Both lines missed.
		 else
		    next_fetch_cycle = resolve_cycle1;				// Line1 missed, line2 hit.

	         return;
	      }
	      else if (!hit2) {
		 next_fetch_cycle = resolve_cycle2;				// Line1 hit, line2 missed.
		 return;
	      }
	   }
	   else {
	      // Access only one line.
	      line1 = (pc >> L1_IC_LINE_SIZE);
	      resolve_cycle1 = IC.Access(Tid, cycle, (line1 << L1_IC_LINE_SIZE), false, &hit1);

	      if (!hit1) {
		 next_fetch_cycle = resolve_cycle1;				// Line1 missed.
		 return;
	      }
	   }
	}

	/////////////////////////////
	// Compose fetch bundle.
	/////////////////////////////

	i = 0;
	stop = false;
	while ((i < fetch_width) && (PERFECT_FETCH || !stop)) {
	      //////////////////////////////////////////////////////
	      // Initialize some important flags.
	      //////////////////////////////////////////////////////
	      pred_tag = 0;
	      history_reg = 0xFFFFFFFF;

	      //////////////////////////////////////////////////////
	      // Fetch instruction from the binary.
	      //////////////////////////////////////////////////////
	      THREAD[Tid]->fetch(pc, inst);

	      //////////////////////////////////////////////////////
	      // Set next_pc and the prediction tag.
	      //////////////////////////////////////////////////////

	      switch(SS_OPCODE(inst)) {
		 case JUMP: case JAL:
	            direct_target = ((pc & 036000000000) | (TARG << 2));

		    next_pc =
			 (PERFECT_BRANCH_PRED ?
			  (wait_for_trap ? direct_target : THREAD[Tid]->pop_pc()) :
			  BP.get_pred(history_reg, pc, inst, direct_target, &pred_tag, &conf, &fm));
		    assert(next_pc == direct_target);

		    stop = true;
		    break;

		 case JR: case JALR:
		    next_pc =
			 (PERFECT_BRANCH_PRED ?
			  (wait_for_trap ? (pc + 8) : THREAD[Tid]->pop_pc()) :
			  BP.get_pred(history_reg, pc, inst, 0, &pred_tag, &conf, &fm));

		    stop = true;
		    break;

		 case BEQ : case BNE : case BLEZ: case BGTZ:
		 case BLTZ: case BGEZ: case BC1F: case BC1T:
		    direct_target = (pc + 8 + (OFS << 2));

		    next_pc =
			 (PERFECT_BRANCH_PRED ?
			  (wait_for_trap ? (pc + 8) : THREAD[Tid]->pop_pc()) :
			  BP.get_pred(history_reg, pc, inst, direct_target, &pred_tag, &conf, &fm));
		    assert(next_pc == direct_target || next_pc == pc + 8);

		    if (next_pc != pc + 8)
		       stop = true;
		    break;

		 default:
		    next_pc = (PERFECT_BRANCH_PRED ?
				(wait_for_trap ? (pc + 8) : THREAD[Tid]->pop_pc()) :
				(pc + 8));
		    break;
	      }

	      // This is needed for perfect branch prediction to work properly when fetching past trap instructions.
	      // Specifically, wait_for_trap prevents popping PCs from the functional simulator after a trap instruction.
	      if ((SS_OPCODE(inst) == SYSCALL) || (SS_OPCODE(inst) == BREAK))
		 wait_for_trap = PERFECT_BRANCH_PRED;

	      // If not already stopped:
	      // Stop if the I$ is not interleaved and if a line boundary is crossed.
	      if (!stop && !IC_INTERLEAVED) {
	         line1 = (pc >> L1_IC_LINE_SIZE);
	         line2 = (next_pc >> L1_IC_LINE_SIZE);
		 stop = (line1 != line2);
	      }

	      // Put the instruction's information into PAY.
	      index = PAY.push();
	      PAY.buf[index].inst.a = inst.a;
	      PAY.buf[index].inst.b = inst.b;
	      PAY.buf[index].pc = pc;
	      PAY.buf[index].next_pc = next_pc;
	      PAY.buf[index].pred_tag = pred_tag;

	      // Link the instruction to corresponding instruction in functional simulator.
	      PAY.map_to_actual(index, Tid);

	      // Latch instruction into fetch-decode pipeline register.
	      DECODE[i].valid = true;
	      DECODE[i].index = index;

	      // Keep count of number of fetched instructions.
	      i++;

	      // Go to next PC.
	      pc = next_pc;
	}			// while()
}				// fetch()


#if 0
void fetcher::fix(unsigned int Tid,
		  unsigned int pred_tag,
		  unsigned int recover_pc) {
	// Redirect the fetch unit.
	BP[Tid]->fix_pred(pred_tag, recover_pc);
	pc[Tid] = recover_pc;
}


void fetcher::verify(unsigned int Tid,
		     unsigned int pred_tag,
		     unsigned int next_pc) {
	// Update branch predictor.
	BP[Tid]->verify_pred(pred_tag, next_pc, false);
}

#endif
