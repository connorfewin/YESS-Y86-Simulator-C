#include <string>
#include <cstdint>
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "F.h"
#include "D.h"
#include "M.h"
#include "W.h"
#include "E.h"
#include "Stage.h"
#include "Instructions.h"
#include "DecodeStage.h"
#include "ExecuteStage.h"
#include "MemoryStage.h"
#include "Status.h"
#include "Debug.h"


/*
 * doClockLow:
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool DecodeStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
   //valA and dstE cannot be obtained from D register. Set to none.
   D * dreg = (D*)pregs[DREG];
   E * ereg = (E*)pregs[EREG];
   W * wreg = (W*)pregs[WREG];
   M * mreg = (M*)pregs[MREG];
   

   //values that can be pulled from d register.
   uint64_t stat = dreg->getstat()->getOutput();
   uint64_t icode = dreg->geticode()->getOutput();
   uint64_t ifun = dreg->getifun()->getOutput();
   uint64_t valC = dreg->getvalC()->getOutput();

   d_srcA_var = getSrcA(dreg, icode);
   d_srcB_var = getSrcB(dreg, icode);
   
   uint64_t dstE = getdstE(dreg, icode);
   uint64_t dstM = getdstM(dreg, icode);
   uint64_t srcA = getSrcA(dreg, icode);
   uint64_t srcB= getSrcB(dreg, icode);

   //these methods call RegisterFile::readFile
   uint64_t valA = getvalA(stages, mreg, wreg, dreg, srcA);
   uint64_t valB = getvalB(stages, mreg, wreg, dreg, srcB);

   setEInput(ereg, stat, icode, ifun, valC, valA, valB, dstE, dstM, srcA, srcB);
   return false;
}

/* doClockHigh
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void DecodeStage::doClockHigh(PipeReg ** pregs)
{
  // D * dreg = (D *)pregs[DREG];
   E * ereg = (E *) pregs[EREG];

   ereg->getstat()->normal();
   ereg->geticode()->normal();
   ereg->getifun()->normal();
   ereg->getvalC()->normal();
   ereg->getvalA()->normal();
   ereg->getvalB()->normal();
   ereg->getdstM()->normal();
   ereg->getdstE()->normal();
   ereg->getsrcA()->normal();
   ereg->getsrcB()->normal();
   
}

/**setEInput
 * called at end of doClockLow method to set the input values
 * for the E pipeline register.
 * 
 * Param for each pipe register field in E pipe reg.
 * Has call to setInput method for each of these.
 */
void DecodeStage::setEInput(E * ereg, uint64_t stat, uint64_t icode, 
                           uint64_t ifun, uint64_t valC, uint64_t valA,
                           uint64_t valB, uint64_t dstE, uint64_t dstM,
                           uint64_t srcA, uint64_t srcB)
{
   ereg->getstat()->setInput(stat);
   ereg->geticode()->setInput(icode);
   ereg->getifun()->setInput(ifun);
   ereg->getvalC()->setInput(valC);
   ereg->getvalA()->setInput(valA);
   ereg->getvalB()->setInput(valB);
   ereg->getdstE()->setInput(dstE);
   ereg->getdstM()->setInput(dstM);
   ereg->getsrcA()->setInput(srcA);
   ereg->getsrcB()->setInput(srcB);
}

uint64_t DecodeStage::getSrcA(D * dreg, uint64_t D_icode){
   //D * dreg = (D *) pregs[DREG];

   if (D_icode == IRRMOVQ || D_icode == IRMMOVQ || 
         D_icode == IOPQ || D_icode == IPUSHQ){
            return dreg->getrA()->getOutput();

   }
   else if (D_icode == IPOPQ || D_icode == IRET){
      return RSP;
   }
   else{
      return RNONE;
   }

}

uint64_t DecodeStage::getSrcB(D * dreg, uint64_t D_icode){
   //D * dreg = (D *) pregs[DREG];
   if (D_icode == IOPQ || D_icode == IRMMOVQ || D_icode == IMRMOVQ){
      return dreg->getrB()->getOutput();
   }
   else if(D_icode == IPUSHQ || D_icode == IPOPQ || D_icode == ICALL || D_icode == IRET){
      return RSP;
   }
   else{
      return RNONE;
   }
}

uint64_t DecodeStage::getdstE(D * dreg, uint64_t D_icode){
   //D * dreg = (D *) pregs[DREG];

   if(D_icode == IRRMOVQ || D_icode == IIRMOVQ || D_icode == IOPQ){
      return dreg->getrB()->getOutput();
   }
   else if(D_icode == IPUSHQ || D_icode == IPOPQ || D_icode == ICALL || D_icode == IRET){
      return RSP;
   }
   else{
      return RNONE;
   }
}

uint64_t DecodeStage::getdstM(D * dreg, uint64_t D_icode){

   if(D_icode == IMRMOVQ || D_icode == IPOPQ){
      return dreg->getrA()->getOutput();
   }
   else{
      return RNONE;
   }
}

//Sel + FwdA
uint64_t DecodeStage::getvalA(Stage ** stages, M * m_reg, W * w_reg, D * d_reg, uint64_t d_srcA){
   
   ExecuteStage * e_stage = (ExecuteStage*)stages[ESTAGE];
   MemoryStage * m_stage = (MemoryStage*)stages[MSTAGE];
   uint64_t e_dstE = e_stage->get_dstE();
   uint64_t M_dstE = m_reg->getdstE()->getOutput();
   uint64_t W_dstE = w_reg->getdstE()->getOutput();
   uint64_t M_dstM = m_reg->getdstM()->getOutput();
   uint64_t W_dstM = w_reg->getdstM()->getOutput();
   uint64_t D_icode = d_reg->geticode()->getOutput();

   if (D_icode == ICALL || D_icode == IJXX){
      return d_reg->getvalP()->getOutput();
   }
   if (d_srcA == RNONE){
      return 0;
   } 
   if(d_srcA == e_dstE){
      return e_stage->get_valE();
   }
   if (d_srcA == M_dstM) {
        return m_stage->get_valM();
    }
   if(d_srcA == M_dstE){
      return m_reg->getvalE()->getOutput();
   }
   if (d_srcA == W_dstM) {
        return w_reg->getvalM()->getOutput();
   }
   if(d_srcA == W_dstE){
      return w_reg->getvalE()->getOutput();
   }
   
   RegisterFile* regFile = RegisterFile::getInstance();   
   bool error;
   return regFile->readRegister(d_srcA, error);
   
}

uint64_t DecodeStage::getvalB(Stage ** stages, M * m_reg, W * w_reg, D * d_reg,  uint64_t d_srcB){
   
   ExecuteStage * e_stage = (ExecuteStage*)stages[ESTAGE];
   MemoryStage * m_stage = (MemoryStage*)stages[MSTAGE];
   uint64_t e_dstE = e_stage->get_dstE();
   uint64_t M_dstE = m_reg->getdstE()->getOutput();
   uint64_t M_dstM = m_reg->getdstM()->getOutput();
   uint64_t W_dstE = w_reg->getdstE()->getOutput();
   uint64_t W_dstM = w_reg->getdstM()->getOutput();
   
   if (d_srcB == RNONE) {
      return 0;
   }
   if(d_srcB == e_dstE){
      return e_stage->get_valE();
   }
   if(d_srcB == M_dstE){
      return m_reg->getvalE()->getOutput();
   }
   if (d_srcB == M_dstM) {
        return m_stage->get_valM();
   }
   if (d_srcB == W_dstM) {
        return w_reg->getvalM()->getOutput();
   }
   if(d_srcB == W_dstE){
      return w_reg->getvalE()->getOutput();
   }
   
   RegisterFile* regFile = RegisterFile::getInstance();
   bool error;
   return regFile->readRegister(d_srcB, error);
}

/**
 * getter for d_srcA variable.
 */
uint64_t DecodeStage::getd_srcA(){
   return d_srcA_var;
}

/**
 * getter for d_srcB variable.
 */
uint64_t DecodeStage::getd_srcB(){
   return d_srcB_var;
}