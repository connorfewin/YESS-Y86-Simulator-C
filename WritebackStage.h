//class to perform the combinational logic of
//the Writeback stage
class WritebackStage: public Stage
{
   //Writeback doesn't need setInput method since it isn't writing to memory.
   // private:
   //    void setDInput(D * dreg, uint64_t stat, uint64_t icode, uint64_t ifun, 
   //                   uint64_t rA, uint64_t rB,
   //                   uint64_t valC, uint64_t valP);
   public:
      bool doClockLow(PipeReg ** pregs, Stage ** stages);
      void doClockHigh(PipeReg ** pregs);

};