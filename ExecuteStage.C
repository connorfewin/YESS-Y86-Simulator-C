#include <string>
#include <cstdint>
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "F.h"
#include "D.h"
#include "E.h"
#include "M.h"
#include "W.h"
#include "Stage.h"
#include "ExecuteStage.h"
#include "Status.h"
#include "Debug.h"
#include "ConditionCodes.h"
#include "Instructions.h"
#include "Tools.h"
#include "MemoryStage.h"


/*
 * doClockLow:
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool ExecuteStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
   E * ereg = (E *) pregs[EREG];
   M * mreg = (M *) pregs[MREG];
   W * wreg = (W *) pregs[WREG];

   uint64_t stat = ereg->getstat()->getOutput();
   uint64_t icode = ereg->geticode()->getOutput();
   //dstE = ereg->getdstE()->getOutput();
   uint64_t dstM = ereg->getdstM()->getOutput();
   uint64_t ifun = ereg->getifun()->getOutput();

   MemoryStage * mem = (MemoryStage*)stages[MSTAGE]; 
   uint64_t m_stat = mem->getm_stat();
   uint64_t W_stat = wreg->getstat()->getOutput();

   valE = ereg->getvalC()->getOutput();
   uint64_t e_valA = ereg->getvalA()->getOutput();
   
   uint64_t e_alufun = getaluFun(ereg, icode);
   uint64_t e_aluA = getaluA(ereg, icode);
   uint64_t e_aluB = getaluB(ereg, icode);
   valE = aluLogic(e_alufun, e_aluA, e_aluB, set_cc(icode, m_stat, W_stat));

   bool error = false;
   bool of = false;
   uint64_t sf = Tools::sign(valE);
   uint64_t zf = 0;
   if(valE == 0){
      zf = 1;
   }
   
   if(set_cc(icode, m_stat, W_stat)){
      ConditionCodes * codes = ConditionCodes::getInstance();
      codes->setConditionCode(sf, SF, error);
      codes->setConditionCode(zf, ZF, error);
      codes->setConditionCode(of, OF, error);
   }

   uint64_t Cnd = setCC(icode, ifun);
   dstE = getdstE(ereg, icode, Cnd);

   setMInput(mreg, stat, icode, Cnd, valE, e_valA, dstE, dstM);

   M_bubble = calcControlSignals(m_stat, W_stat);
   return false;
}

/* doClockHigh
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void ExecuteStage::doClockHigh(PipeReg ** pregs)
{
   //if M_bubble is true, bubbleM. Otherwise, keep normal.
   if (!M_bubble){
      normalM(pregs);
   }
   else{
      bubbleM(pregs);
   }
}

void ExecuteStage::setMInput(M * mreg, uint64_t stat, uint64_t icode, uint64_t Cnd,
   uint64_t valE, uint64_t valA, uint64_t dstE, uint64_t dstM){

   mreg->getstat()->setInput(stat);
   mreg->geticode()->setInput(icode);
   mreg->getCnd()->setInput(Cnd);
   mreg->getvalE()->setInput(valE);
   mreg->getvalA()->setInput(valA);
   mreg->getdstE()->setInput(dstE);
   mreg->getdstM()->setInput(dstM);
   

}

void ExecuteStage::normalM(PipeReg ** pregs){
   M * mreg = (M *) pregs[MREG];

   mreg->getstat()->normal();
   mreg->geticode()->normal();
   mreg->getCnd()->normal();
   mreg->getvalE()->normal();
   mreg->getvalA()->normal();
   mreg->getdstE()->normal();
   mreg->getdstM()->normal();
}

void ExecuteStage::bubbleM(PipeReg ** pregs){
   M * mreg = (M *) pregs[MREG];

   mreg->getstat()->bubble(SAOK);
   mreg->geticode()->bubble(INOP);
   mreg->getCnd()->bubble();
   mreg->getvalE()->bubble();
   mreg->getvalA()->bubble();
   mreg->getdstE()->bubble(RNONE);
   mreg->getdstM()->bubble(RNONE);
}

uint64_t ExecuteStage::getaluA(E * ereg, uint64_t E_icode){
   
   if (E_icode == IRRMOVQ || E_icode == IOPQ){
      return ereg->getvalA()->getOutput();
   }
   else if (E_icode == IIRMOVQ || E_icode == IRMMOVQ || E_icode == IMRMOVQ){
      return ereg->getvalC()->getOutput();
   }
   else if (E_icode == ICALL || E_icode == IPUSHQ){
      return -8;
   }
   else if (E_icode == IRET || E_icode == IPOPQ){
      return 8;
   }
   else{
      return 0;
   }
}

uint64_t ExecuteStage::getaluB(E * ereg, uint64_t E_icode){
   if (E_icode == IRMMOVQ || E_icode == IMRMOVQ || E_icode == IOPQ || E_icode == ICALL 
      || E_icode == IPUSHQ || E_icode == IRET || E_icode == IPOPQ){
         return ereg->getvalB()->getOutput();
   }
   else{
      return 0;
   }
}

uint64_t ExecuteStage::getaluFun(E * ereg, uint64_t E_icode){
   if (E_icode == IOPQ){
      return ereg->getifun()->getOutput();
   }
   return ADDQ;
}

bool ExecuteStage::set_cc(uint64_t E_icode, uint64_t m_stat, uint64_t W_stat){
   if((E_icode == IOPQ) && (m_stat != SADR && m_stat != SINS && m_stat != SHLT)
      && (W_stat != SADR && W_stat != SINS && W_stat != SHLT)){
      return true;
   }
   else{
      return false;
   }
}

uint64_t ExecuteStage::getdstE(E * ereg, uint64_t E_icode, uint64_t cnd){
   if (E_icode == IRRMOVQ && !cnd){
      return RNONE;
   }
   else{
      return ereg->getdstE()->getOutput();
   }
}

//methods for cc and alu combinational logic.
uint64_t ExecuteStage::setCC(uint64_t icode, uint64_t ifun){
   
   ConditionCodes * cc = ConditionCodes::getInstance();
   bool error;
   //ConditionCodes::setConditionCodes(bool value, int32_t ccNum, bool & error);
   uint8_t zf = cc->getConditionCode(ZF, error);
   uint8_t sf = cc->getConditionCode(SF, error);
   uint8_t of = cc->getConditionCode(OF, error);

   //if this is broken change it to ~ and make another method. lab10.
   if (icode == IJXX || icode == ICMOVXX){
      switch(ifun){
         case UNCOND:
            return 1;
         case EQUAL:
            return (zf);
         case NOTEQUAL:
            return (!zf);
         case LESS:
            return (sf ^ of);
         case LESSEQ:
            return ((sf ^ of) | zf);
         case GREATER:
            return (!(sf ^ of) & !zf);
         case GREATEREQ:
            return (!(sf ^ of)); 
      }
   }
   return 0;
}


//alu will perform add, sub, xor, and functions depending on value returned from alu fun. control unit.
uint64_t ExecuteStage::aluLogic(uint64_t ifun, uint64_t aluA, uint64_t aluB, bool setCC){

   bool error;
   //create overflow variable to store result of add and sub overflow tests from Tools.
   bool overflow = false;
   //create and initialize result variable to store the computed result.
   uint64_t result = 0;

   if (ifun == ADDQ){
      //compute aluB + aluA, return for storage. Update CC's.
      result = aluB + aluA;
      //check if overflow occured in the addition of valA and valB.
      overflow = Tools::addOverflow(aluA, aluB);

   } else if (ifun == SUBQ){
      //compute aluB - aluA. Update CC's.
      result = aluB - aluA;
      //check if overflow occured in the subtraction of valA and valB.
      overflow = Tools::subOverflow(aluA, aluB);
   } else if (ifun == ANDQ){
      //return bitwise AND. zf, sf update, of = 0.
      result = (aluA & aluB);
   }
   else if (ifun == XORQ){
      //return bitwise xor. update zf, sf. of = 0.
      result = (aluA ^ aluB);
   }

   //setCC method->if setCC is true, we need to update the condition codes.
   if (setCC){
      //get CC instance so we are updating the right codes.
      ConditionCodes  * cc = ConditionCodes::getInstance();

      //to set zero flag, check if the result is 0.
      cc->setConditionCode((result == 0), ZF, error);
      //to update overflow flag, check if overflow could be true.
      cc->setConditionCode(overflow, OF, error);
      //to update sign flag, get sign bit of result.
      cc->setConditionCode(Tools::sign(result), SF, error);
   }
   
   //return the result of the ALU computation.
   return result;
}
uint64_t ExecuteStage::get_dstE() {
    return dstE;
}
uint64_t ExecuteStage::get_valE() {
    return valE;
}

bool ExecuteStage::calcControlSignals(uint64_t m_stat, uint64_t W_stat){
   if ((m_stat == SADR || m_stat == SINS || m_stat == SHLT) 
      || (W_stat == SADR || W_stat == SINS || W_stat == SHLT)){
         return true;
   }
   else{
      return false;
   }
}
