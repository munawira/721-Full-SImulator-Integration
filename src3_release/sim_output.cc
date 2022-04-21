#include "processor.h"
#include "globals.h"
#include <time.h>

/* set to non-zero when simulator should dump statistics */
int sim_dump_stats = FALSE;

void
sim_stats(FILE *stream)
{
  extern time_t start_time;
  time_t elapsed = MAX(time((time_t *)NULL) - start_time, 1);


  for (unsigned int i = 0; i < NumThreads; i++) {
     INFO("sim: simulation time: %s (%f insts/sec)",
          elapsed_time(elapsed), (double)PROC[i]->num_insn/(double)elapsed);
     INFO("sim: number of instructions: %.0f", (double)PROC[i]->num_insn);
     INFO("sim: number of cycles: %.0f", (double)PROC[i]->cycle);
     INFO("sim: IPC: %.2f", (double)PROC[i]->num_insn/(double)PROC[i]->cycle);
     INFO("sim: number of split instructions: %.0f", (double)PROC[i]->num_insn_split);
     INFO("sim: eff-IPC: %.2f", (double)(PROC[i]->num_insn - PROC[i]->num_insn_split)/(double)PROC[i]->cycle);

     PROC[i]->stats(fp_info);
  }
}
