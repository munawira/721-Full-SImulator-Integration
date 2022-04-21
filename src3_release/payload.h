#define PAYLOAD_BUFFER_SIZE  2048

typedef
enum {
SEL_IQ_INT,		// Select integer IQ.
SEL_IQ_FP,		// Select floating-point IQ.
SEL_IQ_NONE,		// Skip IQ: mark completed right away.
SEL_IQ_NONE_EXCEPTION	// Skip IQ with exception: mark completed and exception right away.
} sel_iq;

union DOUBLE_WORD {
   unsigned long long dw;
   SS_WORD_TYPE w[2];
   float f[2];
   double d;
};

typedef
struct {

   ////////////////////////
   // Set by Fetch Stage.
   ////////////////////////

   SS_INST_TYPE inst;           // The simplescalar instruction.
   unsigned int pc;		// The instruction's PC.
   unsigned int next_pc;	// The next instruction's PC. (I.e., the PC of
				// the instruction fetched after this one.)
   unsigned int pred_tag;       // If the instruction is a branch, this is its
				// index into the Branch Predictor's
				// branch queue.

   bool good_instruction;	// If 'true', this instruction has a
				// corresponding instruction in the
				// functional simulator. This implies the
				// instruction is on the correct control-flow
				// path.
   debug_index db_index;	// Index of corresponding instruction in the
				// functional simulator
				// (if good_instruction == 'true').
				// Having this index is useful for obtaining
				// oracle information about the instruction,
				// for various oracle modes of the simulator.

   ////////////////////////
   // Set by Decode Stage.
   ////////////////////////

   unsigned int flags;          // Operation flags: can be used for quickly
				// deciphering the type of instruction
				// (we used this in Project Part 1).
   enum ss_fu_class fu;         // Operation functional unit class (ignore:
				// not currently used).
   unsigned int latency;        // Operation latency (ignore: not currently
				// used).
   bool split;			// Instruction is split into two.
   bool upper;			// If 'true': this instruction is the upper
				// half of a split instruction.
				// If 'false': this instruction is the lower
				// half of a split instruction.
   bool checkpoint;		// If 'true', this instruction is a branch
				// that needs a checkpoint.
   bool split_store;		// Floating-point stores (S_S and S_D) are
				// implemented as split-stores because they
				// use both int and fp regs.

   // Source register A.
   bool A_valid;		// If 'true', the instruction has a
				// first source register.
   bool A_int;			// If 'true', the source register is an
				// integer register, else it is a
				// floating-point register.
   unsigned int A_log_reg;	// The logical register specifier of the
				// source register.

   // Source register B.
   bool B_valid;		// If 'true', the instruction has a
				// second source register.
   bool B_int;			// If 'true', the source register is an
				// integer register, else it is a
				// floating-point register.
   unsigned int B_log_reg;	// The logical register specifier of the
				// source register.

   // Destination register C.
   bool C_valid;		// If 'true', the instruction has a
				// destination register.
   bool C_int;			// If 'true', the destination register is an
				// integer register, else it is a
				// floating-point register.
   unsigned int C_log_reg;	// The logical register specifier of the
				// destination register.

   // IQ selection.
   sel_iq iq;			// The value of this enumerated type indicates
				// whether to place the instruction in the
				// integer issue queue, floating-point issue
				// queue, or neither issue queue.
				// (The 'sel_iq' enumerated type is also
				// defined in this file.)

   // Details about loads and stores.
   unsigned int size;		// Size of load or store (1, 2, 4, or 8 bytes).
   bool is_signed;		// If 'true', the loaded value is signed,
				// else it is unsigned.
   bool left;			// LWL or SWL instruction.
   bool right;			// LWR or SWR instruction.

   ////////////////////////
   // Set by Rename Stage.
   ////////////////////////

   // Physical registers.
   unsigned int A_phys_reg;	// If there exists a first source register (A),
				// this is the physical register specifier to
				// which it is renamed.
   unsigned int B_phys_reg;	// If there exists a second source register (B),
				// this is the physical register specifier to
				// which it is renamed.
   unsigned int C_phys_reg;	// If there exists a destination register (C),
				// this is the physical register specifier to
				// which it is renamed.

   // Branch ID, for checkpointed branches only.
   unsigned int branch_ID;	// When a checkpoint is created for a branch,
				// this is the branch's ID (its bit position
				// in the Global Branch Mask).

   ////////////////////////
   // Set by Dispatch Stage.
   ////////////////////////

   unsigned int AL_index_int;	// Index into integer Active List.
   unsigned int AL_index_fp;	// Index into floating-point Active List.
   unsigned int LQ_index;	// Indices into LSU. Only used by loads, stores, and branches.
   bool LQ_phase;
   unsigned int SQ_index;
   bool SQ_phase;

   //unsigned int lane;		// Execution lane chosen for the instruction
				// (to be implemented later).

   ////////////////////////
   // Set by Reg. Read Stage.
   ////////////////////////

   // Source values.
   DOUBLE_WORD A_value;		// If there exists a first source register (A),
				// this is its value. To reference the value as
				// an unsigned long long, use "A_value.dw".
   DOUBLE_WORD B_value;		// If there exists a second source register (B),
				// this is its value. To reference the value as
				// an unsigned long long, use "B_value.dw".

   ////////////////////////
   // Set by Execute Stage.
   ////////////////////////

   // Load/store address calculated by AGEN unit.
   unsigned int addr;

   // Resolved branch target. (c_next_pc: computed next program counter)
   unsigned int c_next_pc;

   // Destination value.
   DOUBLE_WORD C_value;		// If there exists a destination register (C),
				// this is its value. To reference the value as
				// an unsigned long long, use "C_value.dw".

} payload_t;
 

class payload {
	public:
		////////////////////////////////////////////////////////////////////////
		//
		// The new 721sim explicitly models all processor queues and
		// pipeline registers so that it is structurally the same as
		// a real pipeline. To maintain good simulation efficiency,
		// however, the simulator holds all "payload" information about
		// an instruction in a centralized data structure and only
		// pointers (indices) into this data structure are actually
		// moved through the pipeline. It is not a real hardware
		// structure but each entry collectively represents an instruction's
		// payload bits distributed throughout the pipeline.
		//
		// Each instruction is allocated two consecutive entries,
		// even and odd, in case the instruction is split into two.
		//
		////////////////////////////////////////////////////////////////////////
		payload_t    buf[PAYLOAD_BUFFER_SIZE];
		unsigned int head;
		unsigned int tail;
		int          length;
 
		payload();		// constructor
		unsigned int push();
		void pop();
		void clear();
		void split(unsigned int index);
		void map_to_actual(unsigned int index, unsigned int Tid);
		void rollback(unsigned int index);
};
