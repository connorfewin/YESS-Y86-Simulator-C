//class to perform the combinational logic of
//the Execute stage
class ExecuteStage: public Stage
{
   private:
      uint64_t dstE, valE;
      bool M_bubble;
      void setMInput(M * mreg, uint64_t stat, uint64_t icode, uint64_t Cnd,
                           uint64_t valE, uint64_t valA, uint64_t dstE, uint64_t dstM);
      void bubbleM(PipeReg ** pregs);
      void normalM(PipeReg ** pregs);                     
      uint64_t getaluA(E * ereg, uint64_t E_icode);
      uint64_t getaluB(E * ereg, uint64_t E_icode);
      uint64_t getaluFun(E * ereg, uint64_t E_icode);
      bool set_cc(uint64_t E_icode, uint64_t m_stat, uint64_t W_stat);
      uint64_t getdstE(E * ereg, uint64_t E_icode, uint64_t cnd);
      uint64_t setCC(uint64_t icode, uint64_t ifun);
      uint64_t aluLogic(uint64_t ifun, uint64_t aluA, uint64_t aluB, bool setCC);
   public:
      bool doClockLow(PipeReg ** pregs, Stage ** stages);
      void doClockHigh(PipeReg ** pregs);
      uint64_t get_dstE();
      uint64_t get_valE();
      bool calcControlSignals(uint64_t m_stat, uint64_t W_stat);

};