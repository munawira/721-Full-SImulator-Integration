

typedef
struct {

// Valid bit for the issue queue entry as a whole.
// If true, it means an instruction occupies this issue queue entry.
bool valid;

// Index into the instruction payload buffer.
unsigned int index;

// Branches that this instruction depends on.
unsigned long long branch_mask;

// Execution lane that this instruction wants.
//unsigned int lane;

// Valid bit, ready bit, and tag of first operand (A).
bool A_valid;		// valid bit (operand exists)
bool A_ready;		// ready bit (operand is ready)
unsigned int A_tag;	// physical register name

// Valid bit, ready bit, and tag of second operand (B).
bool B_valid;		// valid bit (operand exists)
bool B_ready;		// ready bit (operand is ready)
unsigned int B_tag;	// physical register name

} issue_queue_entry_t;


class issue_queue {
	private:
		issue_queue_entry_t *q;		// This is the issue queue.
		unsigned int size;		// Issue queue size.
		int length;			// Number of instructions in the issue queue.

		unsigned int *fl;		// This is the list of free issue queue entries, i.e., the "free list".
		unsigned int fl_head;		// Head of issue queue's free list.
		unsigned int fl_tail;		// Tail of issue queue's free list.
		int fl_length;			// Length of issue queue's free list.

		void remove(unsigned int i);	// Remove the instruction in issue queue entry 'i' from the issue queue.

	public:
		issue_queue(unsigned int size);	// constructor
		bool stall(unsigned int bundle_inst);
		void dispatch(unsigned int index, unsigned long long branch_mask, bool A_valid, bool A_ready, unsigned int A_tag, bool B_valid, bool B_ready, unsigned int B_tag);
		void wakeup(unsigned int tag);
		void select_and_issue(unsigned int num_lanes, lane *Execution_Lanes);
		void flush();
		void clear_branch_bit(unsigned int branch_ID);
		void squash(unsigned int branch_ID);
};
