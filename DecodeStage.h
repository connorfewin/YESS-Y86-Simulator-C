//class to perform the combinational logic of
//the Decode stage
class DecodeStage: public Stage
{
   private:
      uint64_t d_srcA_var;
      uint64_t d_srcB_var;
      void setEInput(E * ereg, uint64_t stat, uint64_t icode, 
                           uint64_t ifun, uint64_t valC, uint64_t valA,
                           uint64_t valB, uint64_t dstE, uint64_t dstM,
                           uint64_t srcA, uint64_t srcB);
      uint64_t getSrcA(D * dreg, uint64_t D_icode);
      uint64_t getSrcB(D * dreg, uint64_t D_icode);
      uint64_t getdstE(D * dreg, uint64_t D_icode);
      uint64_t getdstM(D * dreg, uint64_t D_icode);
      uint64_t getvalA(Stage** stages, M * m_reg, W * w_reg, D * d_reg, uint64_t d_srcA);
      uint64_t getvalB(Stage ** stages, M * m_reg, W * w_reg, D * d_reg, uint64_t d_srcB);
   public:
      bool doClockLow(PipeReg ** pregs, Stage ** stages);
      void doClockHigh(PipeReg ** pregs);
      uint64_t getd_srcA();
      uint64_t getd_srcB();

};