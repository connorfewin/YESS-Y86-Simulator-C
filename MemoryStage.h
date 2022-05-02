//class to perform the combinational logic of
//the Memory stage
class MemoryStage: public Stage
{
   private:
      uint64_t m_valM;
      uint64_t stat;
      void setWInput(W * wreg, uint64_t stat, uint64_t icode, uint64_t valE,
         uint64_t valM, uint64_t dstE, uint64_t dstM);
      uint32_t getAddr(M * mreg, uint64_t M_icode);
      bool mem_read(uint64_t M_icode);
      bool mem_write(uint64_t M_icode);
   public:
      bool doClockLow(PipeReg ** pregs, Stage ** stages);
      void doClockHigh(PipeReg ** pregs);
      uint64_t get_valM();
      uint64_t getm_stat();

};