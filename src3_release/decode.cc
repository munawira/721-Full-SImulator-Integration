#include "processor.h"


void processor::decode() {
   unsigned int i;
   unsigned int index;
   SS_INST_TYPE inst;

   // Stall the Decode Stage if there is not enough space in the Fetch Queue for 2x the fetch bundle width.
   // The factor of 2x assumes that each instruction in the fetch bundle is split, in the worst case.

   // Count the number of instructions in the fetch bundle.
   for (i = 0; i < fetch_width; i++)
      if (!DECODE[i].valid)
         break;

   // Stall if there is not enough space in the Fetch Queue.
   if (!FQ.enough_space(i<<1))
      return;
   

   for (i = 0; i < fetch_width; i++) {

      if (DECODE[i].valid)
         DECODE[i].valid = false;		// Valid instruction: Decode it and remove it from the pipeline register.
      else
         break;					// Not a valid instruction: Reached the end of the fetch bundle so exit loop.

      index = DECODE[i].index;

      // Get instruction from payload buffer.
      inst.a = PAY.buf[index].inst.a;
      inst.b = PAY.buf[index].inst.b;

      // Set checkpoint flag.
      switch (SS_OPCODE(inst)) {
         case JR: case JALR:
	 case BEQ: case BNE: case BLEZ: case BGTZ: case BLTZ: case BGEZ: case BC1F: case BC1T:
	    PAY.buf[index].checkpoint = true;
	    break;

         default:
	    PAY.buf[index].checkpoint = false;
	    break;
      }

      // Set flags
      switch (SS_OPCODE(inst)) {
         case JUMP: case JAL: case JR: case JALR:
	    PAY.buf[index].flags = (F_CTRL|F_UNCOND);
	    break;

	 case BEQ: case BNE: case BLEZ: case BGTZ: case BLTZ: case BGEZ: case BC1F: case BC1T:
	    PAY.buf[index].flags = (F_CTRL|F_COND);
	    break;

	 case LB: case LBU: case LH: case LHU: case LW: case DLW: case L_S: case L_D: case LWL: case LWR:
	    PAY.buf[index].flags = (F_MEM|F_LOAD|F_DISP);
	    break;

	 case SB: case SH: case SW: case DSW: case DSZ: case S_S: case S_D: case SWL: case SWR:
	    PAY.buf[index].flags = (F_MEM|F_STORE|F_DISP);
	    break;

	 case ADD: case ADDI: case ADDU: case ADDIU: case SUB: case SUBU:
	 case MFHI: case MTHI: case MFLO: case MTLO:
	 case AND_: case ANDI: case OR: case ORI: case XOR: case XORI: case NOR:
	 case SLL: case SLLV: case SRL: case SRLV: case SRA: case SRAV:
	 case SLT: case SLTI: case SLTU: case SLTIU:
	 case LUI:
	 case MFC1: case DMFC1: case MTC1: case DMTC1:
	 case NOP:
	    PAY.buf[index].flags = (F_ICOMP);
	    break;

	 case MULT: case MULTU: case DIV: case DIVU:
	    PAY.buf[index].flags = (F_ICOMP|F_LONGLAT);
	    break;

	 case FADD_S: case FADD_D: case FSUB_S: case FSUB_D:
	 case FABS_S: case FABS_D: case FNEG_S: case FNEG_D: case FMOV_S: case FMOV_D:
	 case CVT_S_D: case CVT_S_W: case CVT_D_S: case CVT_D_W: case CVT_W_S: case CVT_W_D:
	 case C_EQ_S: case C_EQ_D: case C_LT_S: case C_LT_D: case C_LE_S: case C_LE_D:
	    PAY.buf[index].flags = (F_FCOMP);
	    break;

	 case FMUL_S: case FMUL_D: case FDIV_S: case FDIV_D: case FSQRT_S: case FSQRT_D:
	    PAY.buf[index].flags = (F_FCOMP|F_LONGLAT);
	    break;

	 case SYSCALL: case BREAK:
	    PAY.buf[index].flags = (F_TRAP);
	    break;

         default:
	    assert(0);
	    break;
      }

      // Set register operands and split instructions.
      // Select IQ.

      // Default values.
      PAY.buf[index].split = false;
      PAY.buf[index].split_store = false;
      PAY.buf[index].A_valid = false;
      PAY.buf[index].B_valid = false;
      PAY.buf[index].C_valid = false;

      switch (SS_OPCODE(inst)) {

	 case JUMP: case NOP:
	    // Select IQ.
	    PAY.buf[index].iq = SEL_IQ_NONE;
	    break;

         case JAL:
	    // dest register
	    PAY.buf[index].C_valid = true;
	    PAY.buf[index].C_int = true;
	    PAY.buf[index].C_log_reg = 31;
	    // Select IQ.
	    PAY.buf[index].iq = SEL_IQ_INT;
            break;

         case JR:
	    // source register
	    PAY.buf[index].A_valid = true;
	    PAY.buf[index].A_int = true;
	    PAY.buf[index].A_log_reg = (RS);
	    // Select IQ.
	    PAY.buf[index].iq = SEL_IQ_INT;
            break;

         case JALR:
	    // source register
	    PAY.buf[index].A_valid = true;
	    PAY.buf[index].A_int = true;
	    PAY.buf[index].A_log_reg = (RS);
	    // dest register
	    PAY.buf[index].C_valid = true;
	    PAY.buf[index].C_int = true;
	    PAY.buf[index].C_log_reg = (RD);
	    // Select IQ.
	    PAY.buf[index].iq = SEL_IQ_INT;
            break;

	 case BEQ: case BNE:
	    // first source register
	    PAY.buf[index].A_valid = true;
	    PAY.buf[index].A_int = true;
	    PAY.buf[index].A_log_reg = (RS);
	    // second source register
	    PAY.buf[index].B_valid = true;
	    PAY.buf[index].B_int = true;
	    PAY.buf[index].B_log_reg = (RT);
	    // Select IQ.
	    PAY.buf[index].iq = SEL_IQ_INT;
	    break;

	 case BLEZ: case BGTZ: case BLTZ: case BGEZ:
	    // first source register
	    PAY.buf[index].A_valid = true;
	    PAY.buf[index].A_int = true;
	    PAY.buf[index].A_log_reg = (RS);
	    // Select IQ.
	    PAY.buf[index].iq = SEL_IQ_INT;
	    break;

	 case BC1F: case BC1T:
	    // first source register
	    PAY.buf[index].A_valid = true;
	    PAY.buf[index].A_int = true;
	    PAY.buf[index].A_log_reg = (FCC_ID_NEW);
	    // Select IQ.
	    PAY.buf[index].iq = SEL_IQ_INT;
	    break;

	 case LB: case LBU: case LH: case LHU: case LW:
	    // base register for AGEN
	    PAY.buf[index].A_valid = true;
	    PAY.buf[index].A_int = true;
	    PAY.buf[index].A_log_reg = (BS);
	    // dest register
	    PAY.buf[index].C_valid = true;
	    PAY.buf[index].C_int = true;
	    PAY.buf[index].C_log_reg = (RT);
	    // Select IQ.
	    PAY.buf[index].iq = SEL_IQ_INT;
            break;

	 case DLW:
	    // Split the instruction.
	    PAY.split(index);

	    // base register for AGEN
	    PAY.buf[index].A_valid = true;
	    PAY.buf[index].A_int = true;
	    PAY.buf[index].A_log_reg = (BS);

	    PAY.buf[index+1].A_valid = true;
	    PAY.buf[index+1].A_int = true;
	    PAY.buf[index+1].A_log_reg = (BS);

	    // dest register
	    PAY.buf[index].C_valid = true;
	    PAY.buf[index].C_int = true;
	    PAY.buf[index].C_log_reg = (RT);

	    PAY.buf[index+1].C_valid = true;
	    PAY.buf[index+1].C_int = true;
	    PAY.buf[index+1].C_log_reg = ((RT)+1);

	    // Select IQ.
	    PAY.buf[index].iq = SEL_IQ_INT;
	    PAY.buf[index+1].iq = SEL_IQ_INT;
            break;

	 case L_S: case L_D:
	    // base register for AGEN
	    PAY.buf[index].A_valid = true;
	    PAY.buf[index].A_int = true;
	    PAY.buf[index].A_log_reg = (BS);
	    // dest register
	    PAY.buf[index].C_valid = true;
	    PAY.buf[index].C_int = false;
	    PAY.buf[index].C_log_reg = (FT);
	    // Select IQ.
	    PAY.buf[index].iq = SEL_IQ_INT;
            break;

	 case LWL: case LWR:
	    // base register for AGEN
	    PAY.buf[index].A_valid = true;
	    PAY.buf[index].A_int = true;
	    PAY.buf[index].A_log_reg = (BS);
	    // source register (same as dest register)
	    PAY.buf[index].B_valid = true;
	    PAY.buf[index].B_int = true;
	    PAY.buf[index].B_log_reg = (RT);
	    // dest register
	    PAY.buf[index].C_valid = true;
	    PAY.buf[index].C_int = true;
	    PAY.buf[index].C_log_reg = (RT);
	    // Select IQ.
	    PAY.buf[index].iq = SEL_IQ_INT;
	    break;

	 case SB: case SH: case SW: case SWL: case SWR:
	    // base register for AGEN
	    PAY.buf[index].A_valid = true;
	    PAY.buf[index].A_int = true;
	    PAY.buf[index].A_log_reg = (BS);
	    // source register
	    PAY.buf[index].B_valid = true;
	    PAY.buf[index].B_int = true;
	    PAY.buf[index].B_log_reg = (RT);
	    // Select IQ.
	    PAY.buf[index].iq = SEL_IQ_INT;
	    break;

	 case DSW:
	    // Split the instruction.
	    PAY.split(index);

	    // base register for AGEN
	    PAY.buf[index].A_valid = true;
	    PAY.buf[index].A_int = true;
	    PAY.buf[index].A_log_reg = (BS);

	    PAY.buf[index+1].A_valid = true;
	    PAY.buf[index+1].A_int = true;
	    PAY.buf[index+1].A_log_reg = (BS);

	    // source register
	    PAY.buf[index].B_valid = true;
	    PAY.buf[index].B_int = true;
	    PAY.buf[index].B_log_reg = (RT);

	    PAY.buf[index+1].B_valid = true;
	    PAY.buf[index+1].B_int = true;
	    PAY.buf[index+1].B_log_reg = ((RT)+1);

	    // Select IQ.
	    PAY.buf[index].iq = SEL_IQ_INT;
	    PAY.buf[index+1].iq = SEL_IQ_INT;
	    break;

	 case DSZ:
	    // base register for AGEN
	    PAY.buf[index].A_valid = true;
	    PAY.buf[index].A_int = true;
	    PAY.buf[index].A_log_reg = (BS);
	    // Select IQ.
	    PAY.buf[index].iq = SEL_IQ_INT;
	    break;

	 case S_S: case S_D:
	    // Implement split-stores for these, due to referencing both int and fp source regs.
	    PAY.buf[index].split_store = true;

	    // Split the instruction.
	    PAY.split(index);

	    // base register for AGEN
	    PAY.buf[index].A_valid = true;
	    PAY.buf[index].A_int = true;
	    PAY.buf[index].A_log_reg = (BS);

	    // source register
	    PAY.buf[index+1].A_valid = true;
	    PAY.buf[index+1].A_int = false;
	    PAY.buf[index+1].A_log_reg = (FT);

	    // Select IQ.
	    PAY.buf[index].iq = SEL_IQ_INT;
	    PAY.buf[index+1].iq = SEL_IQ_FP;
            break;

	 case ADD: case ADDU: case SUB: case SUBU:
	 case AND_: case OR: case XOR: case NOR:
	 case SLLV: case SRLV: case SRAV:
	 case SLT: case SLTU:
	    // first source register
	    PAY.buf[index].A_valid = true;
	    PAY.buf[index].A_int = true;
	    PAY.buf[index].A_log_reg = (RS);
	    // second source register
	    PAY.buf[index].B_valid = true;
	    PAY.buf[index].B_int = true;
	    PAY.buf[index].B_log_reg = (RT);
	    // dest register
	    PAY.buf[index].C_valid = true;
	    PAY.buf[index].C_int = true;
	    PAY.buf[index].C_log_reg = (RD);
	    // Select IQ.
	    PAY.buf[index].iq = SEL_IQ_INT;
            break;

	 case SLL: case SRL: case SRA:
	    // source register
	    PAY.buf[index].A_valid = true;
	    PAY.buf[index].A_int = true;
	    PAY.buf[index].A_log_reg = (RT);
	    // dest register
	    PAY.buf[index].C_valid = true;
	    PAY.buf[index].C_int = true;
	    PAY.buf[index].C_log_reg = (RD);
	    // Select IQ.
	    PAY.buf[index].iq = SEL_IQ_INT;
            break;

	 case ADDI: case ADDIU:
	 case ANDI: case ORI: case XORI:
	 case SLTI: case SLTIU:
	    // source register
	    PAY.buf[index].A_valid = true;
	    PAY.buf[index].A_int = true;
	    PAY.buf[index].A_log_reg = (RS);
	    // dest register
	    PAY.buf[index].C_valid = true;
	    PAY.buf[index].C_int = true;
	    PAY.buf[index].C_log_reg = (RT);
	    // Select IQ.
	    PAY.buf[index].iq = SEL_IQ_INT;
            break;

	 case MULT: case MULTU: case DIV: case DIVU:
	    // Split the instruction.
	    PAY.split(index);

	    // first source register
	    PAY.buf[index].A_valid = true;
	    PAY.buf[index].A_int = true;
	    PAY.buf[index].A_log_reg = (RS);

	    PAY.buf[index+1].A_valid = true;
	    PAY.buf[index+1].A_int = true;
	    PAY.buf[index+1].A_log_reg = (RS);

	    // second source register
	    PAY.buf[index].B_valid = true;
	    PAY.buf[index].B_int = true;
	    PAY.buf[index].B_log_reg = (RT);

	    PAY.buf[index+1].B_valid = true;
	    PAY.buf[index+1].B_int = true;
	    PAY.buf[index+1].B_log_reg = (RT);

	    // destination register
	    PAY.buf[index].C_valid = true;
	    PAY.buf[index].C_int = true;
	    PAY.buf[index].C_log_reg = (HI_ID_NEW);

	    PAY.buf[index+1].C_valid = true;
	    PAY.buf[index+1].C_int = true;
	    PAY.buf[index+1].C_log_reg = (LO_ID_NEW);

	    // Select IQ.
	    PAY.buf[index].iq = SEL_IQ_INT;
	    PAY.buf[index+1].iq = SEL_IQ_INT;
	    break;

	 case MFHI:
	    // source register
	    PAY.buf[index].A_valid = true;
	    PAY.buf[index].A_int = true;
	    PAY.buf[index].A_log_reg = (HI_ID_NEW);
	    // dest register
	    PAY.buf[index].C_valid = true;
	    PAY.buf[index].C_int = true;
	    PAY.buf[index].C_log_reg = (RD);
	    // Select IQ.
	    PAY.buf[index].iq = SEL_IQ_INT;
            break;

	 case MTHI:
	    // source register
	    PAY.buf[index].A_valid = true;
	    PAY.buf[index].A_int = true;
	    PAY.buf[index].A_log_reg = (RS);
	    // dest register
	    PAY.buf[index].C_valid = true;
	    PAY.buf[index].C_int = true;
	    PAY.buf[index].C_log_reg = (HI_ID_NEW);
	    // Select IQ.
	    PAY.buf[index].iq = SEL_IQ_INT;
            break;

	 case MFLO:
	    // source register
	    PAY.buf[index].A_valid = true;
	    PAY.buf[index].A_int = true;
	    PAY.buf[index].A_log_reg = (LO_ID_NEW);
	    // dest register
	    PAY.buf[index].C_valid = true;
	    PAY.buf[index].C_int = true;
	    PAY.buf[index].C_log_reg = (RD);
	    // Select IQ.
	    PAY.buf[index].iq = SEL_IQ_INT;
            break;

	 case MTLO:
	    // source register
	    PAY.buf[index].A_valid = true;
	    PAY.buf[index].A_int = true;
	    PAY.buf[index].A_log_reg = (RS);
	    // dest register
	    PAY.buf[index].C_valid = true;
	    PAY.buf[index].C_int = true;
	    PAY.buf[index].C_log_reg = (LO_ID_NEW);
	    // Select IQ.
	    PAY.buf[index].iq = SEL_IQ_INT;
            break;

	 case FADD_S: case FADD_D: case FSUB_S: case FSUB_D: case FMUL_S: case FMUL_D: case FDIV_S: case FDIV_D:
	    // first source register
	    PAY.buf[index].A_valid = true;
	    PAY.buf[index].A_int = false;
	    PAY.buf[index].A_log_reg = (FS);
	    // second source register
	    PAY.buf[index].B_valid = true;
	    PAY.buf[index].B_int = false;
	    PAY.buf[index].B_log_reg = (FT);
	    // dest register
	    PAY.buf[index].C_valid = true;
	    PAY.buf[index].C_int = false;
	    PAY.buf[index].C_log_reg = (FD);
	    // Select IQ.
	    PAY.buf[index].iq = SEL_IQ_FP;
            break;

	 case FABS_S: case FABS_D: case FMOV_S: case FMOV_D: case FNEG_S: case FNEG_D:
	 case CVT_S_D: case CVT_S_W: case CVT_D_S: case CVT_D_W: case CVT_W_S: case CVT_W_D:
	 case FSQRT_S: case FSQRT_D:
	    // source register
	    PAY.buf[index].A_valid = true;
	    PAY.buf[index].A_int = false;
	    PAY.buf[index].A_log_reg = (FS);
	    // dest register
	    PAY.buf[index].C_valid = true;
	    PAY.buf[index].C_int = false;
	    PAY.buf[index].C_log_reg = (FD);
	    // Select IQ.
	    PAY.buf[index].iq = SEL_IQ_FP;
            break;

	 case C_EQ_S: case C_EQ_D: case C_LT_S: case C_LT_D: case C_LE_S: case C_LE_D:
	    // first source register
	    PAY.buf[index].A_valid = true;
	    PAY.buf[index].A_int = false;
	    PAY.buf[index].A_log_reg = (FS);
	    // second source register
	    PAY.buf[index].B_valid = true;
	    PAY.buf[index].B_int = false;
	    PAY.buf[index].B_log_reg = (FT);
	    // dest register
	    PAY.buf[index].C_valid = true;
	    PAY.buf[index].C_int = true;
	    PAY.buf[index].C_log_reg = (FCC_ID_NEW);
	    // Select IQ.
	    PAY.buf[index].iq = SEL_IQ_FP;
            break;

	 case MFC1:
	    // source register
	    PAY.buf[index].A_valid = true;
	    PAY.buf[index].A_int = false;
	    PAY.buf[index].A_log_reg = (FS);
	    // dest register
	    PAY.buf[index].C_valid = true;
	    PAY.buf[index].C_int = true;
	    PAY.buf[index].C_log_reg = (RT);
	    // Select IQ.
	    PAY.buf[index].iq = SEL_IQ_FP;
	    break;

	 case DMFC1:
	    // Split the instruction.
	    PAY.split(index);

	    // source register
	    PAY.buf[index].A_valid = true;
	    PAY.buf[index].A_int = false;
	    PAY.buf[index].A_log_reg = (FS);

	    PAY.buf[index+1].A_valid = true;
	    PAY.buf[index+1].A_int = false;
	    PAY.buf[index+1].A_log_reg = (FS);

	    // dest register
	    PAY.buf[index].C_valid = true;
	    PAY.buf[index].C_int = true;
	    PAY.buf[index].C_log_reg = (RT);

	    PAY.buf[index+1].C_valid = true;
	    PAY.buf[index+1].C_int = true;
	    PAY.buf[index+1].C_log_reg = ((RT)+1);

	    // Select IQ.
	    PAY.buf[index].iq = SEL_IQ_FP;
	    PAY.buf[index+1].iq = SEL_IQ_FP;
            break;

	 case MTC1:
	    // source register
	    PAY.buf[index].A_valid = true;
	    PAY.buf[index].A_int = true;
	    PAY.buf[index].A_log_reg = (RT);
	    // dest register
	    PAY.buf[index].C_valid = true;
	    PAY.buf[index].C_int = false;
	    PAY.buf[index].C_log_reg = (FS);
	    // Select IQ.
	    PAY.buf[index].iq = SEL_IQ_INT;
	    break;

	 case DMTC1:
	    // first source register
	    PAY.buf[index].A_valid = true;
	    PAY.buf[index].A_int = true;
	    PAY.buf[index].A_log_reg = (RT);
	    // second source register
	    PAY.buf[index].B_valid = true;
	    PAY.buf[index].B_int = true;
	    PAY.buf[index].B_log_reg = ((RT)+1);
	    // dest register
	    PAY.buf[index].C_valid = true;
	    PAY.buf[index].C_int = false;
	    PAY.buf[index].C_log_reg = (FS);
	    // Select IQ.
	    PAY.buf[index].iq = SEL_IQ_INT;
            break;

	 case SYSCALL: case BREAK:
	    // Select IQ.
	    PAY.buf[index].iq = SEL_IQ_NONE_EXCEPTION;
            break;

	 case LUI:
	    // dest register
	    PAY.buf[index].C_valid = true;
	    PAY.buf[index].C_int = true;
	    PAY.buf[index].C_log_reg = (RT);
	    // Select IQ.
	    PAY.buf[index].iq = SEL_IQ_INT;
            break;

         default:
	    assert(0);
	    break;
      }

      // Decode some details about loads and stores:
      // size and sign of data, and left/right info.
      switch (SS_OPCODE(inst)) {
         case DLW: case DSW:
	    assert(PAY.buf[index].split && PAY.buf[index].upper);
	    PAY.buf[index].size = 4;
	    PAY.buf[index].is_signed = false;
	    PAY.buf[index].left = false;
	    PAY.buf[index].right = false;

	    assert(PAY.buf[index+1].split && !PAY.buf[index+1].upper);
	    PAY.buf[index+1].size = 4;
	    PAY.buf[index+1].is_signed = false;
	    PAY.buf[index+1].left = false;
	    PAY.buf[index+1].right = false;
	    break;

	 case L_D: case S_D: case DSZ:
            PAY.buf[index].size = 8;
	    PAY.buf[index].is_signed = false;
	    PAY.buf[index].left = false;
	    PAY.buf[index].right = false;
            break;

         case LWL: case SWL:
	    PAY.buf[index].size = 4;
	    PAY.buf[index].is_signed = false;
	    PAY.buf[index].left = true;
	    PAY.buf[index].right = false;
	    break;

         case LWR: case SWR:
	    PAY.buf[index].size = 4;
	    PAY.buf[index].is_signed = false;
	    PAY.buf[index].left = false;
	    PAY.buf[index].right = true;
	    break;

         case LB:
	    PAY.buf[index].size = 1;
	    PAY.buf[index].is_signed = true;
	    PAY.buf[index].left = false;
	    PAY.buf[index].right = false;
	    break;

         case LH:
	    PAY.buf[index].size = 2;
	    PAY.buf[index].is_signed = true;
	    PAY.buf[index].left = false;
	    PAY.buf[index].right = false;
	    break;

         case LBU: case SB:
	    PAY.buf[index].size = 1;
	    PAY.buf[index].is_signed = false;
	    PAY.buf[index].left = false;
	    PAY.buf[index].right = false;
 	    break;

         case LHU: case SH:
	    PAY.buf[index].size = 2;
	    PAY.buf[index].is_signed = false;
	    PAY.buf[index].left = false;
	    PAY.buf[index].right = false;
 	    break;

         case LW: case L_S:
         case SW: case S_S:
	    PAY.buf[index].size = 4;
	    PAY.buf[index].is_signed = false;
	    PAY.buf[index].left = false;
	    PAY.buf[index].right = false;
	    break;

         default:
            break;
      }

      // Insert one or two instructions into the Fetch Queue (indices).
      FQ.push(index);
      if (PAY.buf[index].split) {
	 assert(PAY.buf[index+1].split);
	 assert(PAY.buf[index].upper);
	 assert(!PAY.buf[index+1].upper);
         FQ.push(index+1);
      }
   }
}
