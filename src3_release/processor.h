//////////////////////////////////////////////////////////////////////////////

#include "Thread.h"		// simplescalar + thread functional simulator(s)
#include "parameters.h"		// simulator configuration

//////////////////////////////////////////////////////////////////////////////

#include "bpred_interface.h"	// referenced by fetch unit
#include "memory_macros.h"	// referenced by memory system
#include "histogram.h"		// referenced by fetcn unit and memory system
#include "cache.h"		//	"
#include "dcache.h"		//	"

//////////////////////////////////////////////////////////////////////////////

#include "payload.h"		// instruction payload buffer

#include "pipeline_register.h"	// PIPELINE REGISTERS

#include "fetch_queue.h"	// FETCH QUEUE

#include "renamer.h"		// REGISTER RENAMER + REGISTER FILE

#include "lane.h"		// EXECUTION LANES

#include "issue_queue.h"	// ISSUE QUEUE

#include "mem_interface.h"	// MEMORY SYSTEM

//////////////////////////////////////////////////////////////////////////////

#define TOTAL_FP_REGS		32
#define TOTAL_GP_INT_REGS	32
#define TOTAL_INT_REGS		35

#define HI_ID_NEW		32
#define LO_ID_NEW		33
#define FCC_ID_NEW		34

//////////////////////////////////////////////////////////////////////////////

#define IS_BRANCH(flags)	((flags) & (F_CTRL))
#define IS_LOAD(flags)          ((flags) & (F_LOAD))
#define IS_STORE(flags)         ((flags) & (F_STORE))
#define IS_MEM_OP(flags)        ((flags) & (F_MEM))

//////////////////////////////////////////////////////////////////////////////

#define BIT_IS_ZERO(x,i)	(((x) & (((unsigned long long)1) << i)) == 0)
#define BIT_IS_ONE(x,i)		(((x) & (((unsigned long long)1) << i)) != 0)
#define SET_BIT(x,i)		(x |= (((unsigned long long)1) << i))
#define CLEAR_BIT(x,i)		(x &= ~(((unsigned long long)1) << i))

//////////////////////////////////////////////////////////////////////////////


class processor {
	private:
		/////////////////////////////////////////////////////////////
		// Instruction payload buffer.
		/////////////////////////////////////////////////////////////
		payload	PAY;

		/////////////////////////////////////////////////////////////
		// Pipeline widths.
		/////////////////////////////////////////////////////////////
		unsigned int fetch_width;	// fetch, decode width
		unsigned int dispatch_width;	// rename, dispatch width
		unsigned int issue_width;	// issue width (number of universal execution lanes)
		unsigned int retire_width;	// retire width

		/////////////////////////////////////////////////////////////
		// Fetch unit.
		/////////////////////////////////////////////////////////////
		unsigned int pc;		// Speculative program counter.
		SS_TIME_TYPE next_fetch_cycle;	// Support for I$ miss stalls.
		bool wait_for_trap;		// Needed to override perfect branch prediction after fetching a syscall.
		bpred_interface BP;		// Branch predictor.
		CacheClass IC;			// Instruction cache.

		/////////////////////////////////////////////////////////////
		// Pipeline register between the Fetch and Decode Stages.
		/////////////////////////////////////////////////////////////
		pipeline_register *DECODE;

		/////////////////////////////////////////////////////////////
		// Fetch Queue between the Decode and Rename Stages.
		/////////////////////////////////////////////////////////////
		fetch_queue FQ;

		/////////////////////////////////////////////////////////////
		// Pipeline register between the Rename1 and Rename2
		// sub-stages (within the Rename Stage).
		/////////////////////////////////////////////////////////////
		pipeline_register *RENAME2;

		/////////////////////////////////////////////////////////////
		// Register renaming modules.
		/////////////////////////////////////////////////////////////
		renamer *REN_INT;
		renamer *REN_FP;

		/////////////////////////////////////////////////////////////
		// Pipeline register between the Rename and Dispatch Stages.
		/////////////////////////////////////////////////////////////
		pipeline_register *DISPATCH;

		/////////////////////////////////////////////////////////////
		// Issue Queues.
		/////////////////////////////////////////////////////////////
		issue_queue IQ_INT;
		issue_queue IQ_FP;

		/////////////////////////////////////////////////////////////
		// Execution Lanes.
		/////////////////////////////////////////////////////////////
		lane *Execution_Lanes;

		/////////////////////////////////////////////////////////////
		// Load and Store Unit.
		/////////////////////////////////////////////////////////////
		mem_interface LSU;
				

		//////////////////////
		// PRIVATE FUNCTIONS
		//////////////////////

		void copy_reg();
		void agen(unsigned int index);
		void alu(unsigned int index);
		void squash_complete(unsigned int offending_PC);
		void resolve(unsigned int branch_ID, bool correct);
		void checker();
		void check_single(SS_WORD_TYPE proc, SS_WORD_TYPE func, char *desc);
		void check_double(SS_WORD_TYPE proc0, SS_WORD_TYPE proc1, SS_WORD_TYPE func0, SS_WORD_TYPE func1, char *desc);


	public:
		// The thread id.
		unsigned int Tid;

		// The simulator cycle.
		SS_TIME_TYPE cycle;

		// Number of instructions retired.
		unsigned int num_insn;
		unsigned int num_insn_split;

#if 0
		unsigned int BREAKPOINT_TIME;
#endif

		processor(unsigned int fq_size,
			  unsigned int num_chkpts,
			  unsigned int rob_size,
			  unsigned int iq_size,
			  unsigned int lq_size,
			  unsigned int sq_size,
			  unsigned int fetch_width,
			  unsigned int dispatch_width,
			  unsigned int issue_width,
			  unsigned int retire_width,
			  unsigned int Tid);
		
		// Functions for pipeline stages.
		void fetch();
		void decode();
		void rename1();
		void rename2();
		void dispatch();
		void schedule();
		void register_read(unsigned int lane_number);
		void execute(unsigned int lane_number);
		void writeback(unsigned int lane_number);
		void retire();

		// Miscellaneous other functions.
		void next_cycle();
		void stats(FILE *fp);
		void skip(unsigned int skip_amt);
};
