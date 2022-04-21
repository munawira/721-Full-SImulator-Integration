#include "processor.h"


processor::processor(unsigned int fq_size,
		     unsigned int num_chkpts,
		     unsigned int rob_size,
		     unsigned int iq_size,
		     unsigned int lq_size,
		     unsigned int sq_size,
		     unsigned int fetch_width,
		     unsigned int dispatch_width,
		     unsigned int issue_width,
		     unsigned int retire_width,
		     unsigned int Tid):
FQ(fq_size),
IQ_INT(iq_size),
IQ_FP(iq_size),
LSU(lq_size, sq_size, Tid),
IC(L1_IC_SETS, L1_IC_ASSOC, L1_IC_LINE_SIZE, L1_IC_HIT_LATENCY, L1_IC_MISS_LATENCY, L1_IC_NUM_MHSRs, L1_IC_MISS_SRV_PORTS, L1_IC_MISS_SRV_LATENCY)
{
   // Initialize the thread id.
   this->Tid = Tid;

   // Initialize simulator time:
   cycle = 0;

   // Initialize number of retired instructions.
   num_insn = 0;
   num_insn_split = 0;

#if 0
		   BREAKPOINT_TIME = 0;
#endif

   // Skip.
   if (SKIP_AMT)
      skip(SKIP_AMT);

   /////////////////////////////////////////////////////////////
   // Pipeline widths.
   /////////////////////////////////////////////////////////////
   this->fetch_width = fetch_width;
   this->dispatch_width = dispatch_width;
   this->issue_width = issue_width;
   this->retire_width = retire_width;

   /////////////////////////////////////////////////////////////
   // Fetch unit.
   /////////////////////////////////////////////////////////////
   pc = THREAD[Tid]->get_arch_PC();
   next_fetch_cycle = (SS_TIME_TYPE)0;
   wait_for_trap = false;

   /////////////////////////////////////////////////////////////
   // Pipeline register between the Fetch and Decode Stages.
   /////////////////////////////////////////////////////////////
   DECODE = new pipeline_register[fetch_width];

   /////////////////////////////////////////////////////////////
   // Pipeline register between the Rename1 and Rename2
   // sub-stages (within the Rename Stage).
   /////////////////////////////////////////////////////////////
   RENAME2 = new pipeline_register[dispatch_width];

   ////////////////////////////////////////////////////////////
   // Set up the register renaming modules.
   ////////////////////////////////////////////////////////////

   REN_INT = new renamer(TOTAL_INT_REGS, (TOTAL_INT_REGS + rob_size), num_chkpts);
   REN_FP = new renamer(TOTAL_FP_REGS, (TOTAL_FP_REGS + rob_size), num_chkpts);

   // Initialize the physical register files from func. sim.
   copy_reg();

   /////////////////////////////////////////////////////////////
   // Pipeline register between the Rename and Dispatch Stages.
   /////////////////////////////////////////////////////////////
   DISPATCH = new pipeline_register[dispatch_width];

   /////////////////////////////////////////////////////////////
   // Execution Lanes.
   /////////////////////////////////////////////////////////////
   Execution_Lanes = new lane[issue_width];

   ///////////////////////////////////////////////////
   // Set up the memory system.
   ///////////////////////////////////////////////////
   // Memory system is declared above.
   // Initialize memory from functional simulator memory.
   LSU.copy_mem(THREAD[Tid]->get_mem_table());
}


#if 0
void processor::echo_config(FILE *fp) {
   fprintf(fp, "-----------\n");
   fprintf(fp, "SUPERSCALAR\n");
   fprintf(fp, "-----------\n");

   //fprintf(fp, "FETCH QUEUE SIZE: %d\n", FQ.get_size());
   //fprintf(fp, "ROB SIZE: %d\n", rob_size);
   //fprintf(fp, "IQ SIZE: %d\n", iq_size);
   //fprintf(fp, "LSQ SIZE: %d\n", mem_i.get_size());
   fprintf(fp, "FETCH_WIDTH: %d\n", fetch_width);
   fprintf(fp, "DISPATCH_WIDTH: %d\n", dispatch_width);
   fprintf(fp, "ISSUE_WIDTH: %d\n", issue_width);
   fprintf(fp, "RETIRE_WIDTH: %d\n", retire_width);

   fflush(fp);
}
#endif


void processor::copy_reg() {
   DOUBLE_WORD x;

   // Integer RF: general registers 0-31.
   for (unsigned int i = 0; i < TOTAL_GP_INT_REGS; i++)
      REN_INT->write(REN_INT->rename_rsrc(i), (unsigned long long)THREAD[Tid]->get_arch_reg_value(i));

   // Integer RF: HI, LO, FCC.
   REN_INT->write(REN_INT->rename_rsrc(HI_ID_NEW), (unsigned long long)THREAD[Tid]->get_arch_reg_value(HI_ID));
   REN_INT->write(REN_INT->rename_rsrc(LO_ID_NEW), (unsigned long long)THREAD[Tid]->get_arch_reg_value(LO_ID));
   REN_INT->write(REN_INT->rename_rsrc(FCC_ID_NEW), (unsigned long long)THREAD[Tid]->get_arch_reg_value(FCC_ID));

   // Floating-point RF.
   for (unsigned int i = 0; i < TOTAL_FP_REGS; i+=2) {
      x.w[0] = THREAD[Tid]->get_arch_reg_value(i + FPR_BASE);
      x.w[1] = THREAD[Tid]->get_arch_reg_value((i+1) + FPR_BASE);

      // single precision value or double precision value for F0
      REN_FP->write(REN_FP->rename_rsrc(i), x.dw);

      // single precision value for F1
      REN_FP->write(REN_FP->rename_rsrc(i+1), (unsigned long long)x.w[1]);
   }
}


void processor::next_cycle() {
   // Update the simulator cycle.
   cycle += 1;

#if 0
   if (cycle > BREAKPOINT_TIME) {
      printf("cycle = %.0f\n", (double)cycle);
      printf("Enter next cycle to stop at: ");
      scanf("%d", &BREAKPOINT_TIME);
      printf("next time: %d\n\n", BREAKPOINT_TIME);
   }
#endif
}


void processor::stats(FILE *fp) {
   BP.dump_stats(fp);
   LSU.stats(fp);
}


void processor::skip(unsigned int skip_amt) {
   debug_index i;
   db_t * db_ptr;
   unsigned int j;
   unsigned int pc;

   THREAD[Tid]->run_ahead();
   pc = THREAD[Tid]->get_arch_PC();

   j = 0;
   while (j < skip_amt) {
      i = THREAD[Tid]->first(pc);
      db_ptr = THREAD[Tid]->pop(i);
      THREAD[Tid]->pop_pc(); // Keep perf. branch pred. in sync
      pc = db_ptr->a_next_pc;
      j++;
      if (db_ptr->a_flags & F_TRAP) {
         THREAD[Tid]->trap_now(db_ptr->a_inst);
         THREAD[Tid]->trap_resume();
      }
   }

   while (THREAD[Tid]->cleanup_get_instr())
      THREAD[Tid]->pop_pc(); // Keep perf. branch pred. in sync

   fprintf(stderr, "Starting with Timing Simulator.\n");
}
