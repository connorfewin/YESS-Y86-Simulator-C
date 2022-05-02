//class to perform the combinational logic of
//the Fetch stage
class FetchStage: public Stage
{
   private:
      bool F_stall_var;
      bool D_stall_var;
      void setDInput(D * dreg, uint64_t stat, uint64_t icode, uint64_t ifun, 
                     uint64_t rA, uint64_t rB,
                     uint64_t valC, uint64_t valP);
      uint64_t selectPC(F * freg, M * mreg, W * wreg);
      bool needRegIds(uint64_t f_icode);
      bool needValC(uint64_t f_icode);
      uint64_t predictPC(uint64_t f_icode, uint64_t f_valC, uint64_t f_valP);
      uint64_t PCincrement(uint64_t  f_pc, bool needRegIds, bool needValC);
      void getRegIds(uint64_t f_pc, uint64_t * rA, uint64_t * rB);
      uint64_t buildValC(int32_t f_pc, bool needsRegId);
      bool instruction_valid(uint64_t f_icode);
      uint64_t f_stat(uint64_t icode, bool memoryError, bool instruction_valid);
      bool F_stall(uint64_t D_icode, uint64_t M_icode, uint64_t E_icode, uint64_t E_dstM, uint64_t d_srcA, uint64_t d_srcB);
      bool D_stall(uint64_t D_icode, uint64_t M_icode, uint64_t E_icode, uint64_t E_dstM, uint64_t d_srcA, uint64_t d_srcB);
      void calculateControlSignals(PipeReg ** pregs, Stage ** stages);
   public:
      bool doClockLow(PipeReg ** pregs, Stage ** stages);
      void doClockHigh(PipeReg ** pregs);

};
