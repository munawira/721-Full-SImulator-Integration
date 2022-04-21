
class pipeline_register {

public:

   bool valid;				// valid instruction
   unsigned int index;			// index into instruction payload buffer
   unsigned long long branch_mask;	// branches that this instruction depends on

   pipeline_register();	// constructor

};
