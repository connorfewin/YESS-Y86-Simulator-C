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
#include "FetchStage.h"
#include "Status.h"
#include "Debug.h"
#include "Instructions.h"
#include "Memory.h"
#include "Tools.h"
#include "ExecuteStage.h"
#include "DecodeStage.h"


/*
 * doClockLow:
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool FetchStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
   F * freg = (F *) pregs[FREG];
   D * dreg = (D *) pregs[DREG];
   M * mreg = (M *) pregs[MREG];
   W * wreg = (W *) pregs[WREG];
   Memory * mem = Memory::getInstance();
   bool error;

   uint64_t f_pc = selectPC(freg, mreg, wreg);

   uint8_t instructionByte = mem->getByte(f_pc, error);

   uint64_t icode = Tools::getBits(instructionByte, 4, 7);
   uint64_t ifun = Tools::getBits(instructionByte, 0, 3);
   uint64_t valC = 0;
   uint64_t valP = PCincrement(f_pc, needRegIds(icode), needValC(icode));
   uint64_t rA = RNONE, rB = RNONE, stat = SAOK;
   //code missing here to select the value of the PC
   //and fetch the instruction from memory
   //Fetching the instruction will allow the icode, ifun,
   //rA, rB, and valC to be set.
   //The lab assignment describes what methods need to be
   //written.

   if (error){
      icode = INOP;
      ifun = FNONE;
   }

   bool id = needRegIds(icode);
   if (needRegIds(icode)){
      getRegIds(f_pc, &rA, &rB);
   }
   if(needValC(icode)){
      valC = buildValC(f_pc, id);
   }
   bool instrValid = instruction_valid(icode);
   stat = f_stat(icode, error, instrValid);

   //The value passed to setInput below will need to be changed
   freg->getpredPC()->setInput(predictPC(icode, valC, valP));

   calculateControlSignals(pregs, stages);
   //provide the input values for the D register
   setDInput(dreg, stat, icode, ifun, rA, rB, valC, valP);
   return false;
}

/* doClockHigh
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void FetchStage::doClockHigh(PipeReg ** pregs)
{
   F * freg = (F *) pregs[FREG];
   D * dreg = (D *) pregs[DREG];
   if (!F_stall_var){
      freg->getpredPC()->normal();
   }

   if(!D_stall_var){
      dreg->getstat()->normal();
      dreg->geticode()->normal();
      dreg->getifun()->normal();
      dreg->getrA()->normal();
      dreg->getrB()->normal();
      dreg->getvalC()->normal();
      dreg->getvalP()->normal();
   }
}

/* setDInput
 * provides the input to potentially be stored in the D register
 * during doClockHigh
 *
 * @param: dreg - pointer to the D register instance
 * @param: stat - value to be stored in the stat pipeline register within D
 * @param: icode - value to be stored in the icode pipeline register within D
 * @param: ifun - value to be stored in the ifun pipeline register within D
 * @param: rA - value to be stored in the rA pipeline register within D
 * @param: rB - value to be stored in the rB pipeline register within D
 * @param: valC - value to be stored in the valC pipeline register within D
 * @param: valP - value to be stored in the valP pipeline register within D
*/
void FetchStage::setDInput(D * dreg, uint64_t stat, uint64_t icode, 
                           uint64_t ifun, uint64_t rA, uint64_t rB,
                           uint64_t valC, uint64_t valP)
{
   dreg->getstat()->setInput(stat);
   dreg->geticode()->setInput(icode);
   dreg->getifun()->setInput(ifun);
   dreg->getrA()->setInput(rA);
   dreg->getrB()->setInput(rB);
   dreg->getvalC()->setInput(valC);
   dreg->getvalP()->setInput(valP);
}
     
//selectPC method: input is F, M, and W Registers
uint64_t FetchStage::selectPC(F * freg, M * mreg, W * wreg){
   uint64_t progCount = 0;
   uint64_t M_icode = mreg ->geticode()->getOutput();
   if(M_icode == IJXX && !mreg->getCnd()->getOutput()){
      return mreg->getvalA()->getOutput();
   }
   else if(wreg->geticode()->getOutput() == IRET){
      return wreg->getvalM()->getOutput();
   }
   else{
      return freg->getpredPC()->getOutput();
   }

   return progCount;
}

//needRegIds method: input is f_icode
bool FetchStage::needRegIds(uint64_t f_icode){   
   if(f_icode==IRRMOVQ||f_icode==IOPQ||f_icode==IPUSHQ||f_icode==IPOPQ||
      f_icode==IIRMOVQ||f_icode==IRMMOVQ||f_icode==IMRMOVQ){
         return true;
   }
   return false;   
}

//getRegIds method
void FetchStage::getRegIds(uint64_t f_pc, uint64_t * rA, uint64_t * rB){
   
   Memory * mem = Memory::getInstance();
   bool error;

   uint64_t rval = mem->getByte(f_pc + 1, error);
   
   uint8_t rAval = Tools::getBits(rval, 4, 7);
   uint8_t rBval = Tools::getBits(rval, 0, 3);
   
   *rA = rAval;
   *rB = rBval;
}

uint64_t FetchStage::buildValC(int32_t f_pc, bool needsRegId){
   Memory * mem = Memory::getInstance();
   //if need_valC is true, read 8 bytes from memory and builds and returns
   //the valC that is then used as input to the D register.
   int startOffset = f_pc + 1;
   if(needsRegId) {
        startOffset+=1;
    }

    bool error;
    uint8_t valC[8];

    for(int i = 0; i < 8; i++) {
        valC[i] = mem->getByte(startOffset + i, error);
    }

   return Tools::buildLong(valC);
}


//need valC method: input is f_icode
bool FetchStage::needValC(uint64_t f_icode){
   if(f_icode==IIRMOVQ||f_icode==IRMMOVQ||f_icode==IMRMOVQ||f_icode==IJXX||f_icode==ICALL){
         return true;
   }
   return false;  
}
//predictPC method: inputs are f_icode, f_valC, f_valP
uint64_t FetchStage::predictPC(uint64_t f_icode, uint64_t f_valC, uint64_t f_valP){
   if(f_icode == IJXX || f_icode == ICALL){
      return f_valC;
   }
   else{
      return f_valP;
   }
}

uint64_t FetchStage::PCincrement(uint64_t  f_pc, bool needRegIds, bool needValC){
   f_pc += 1;
   if(needRegIds){
      f_pc += 1;
   }
   if(needValC){
      f_pc += 8;
   }
   return f_pc;
}

bool FetchStage::instruction_valid(uint64_t f_icode){
   if (f_icode == INOP || f_icode == IHALT || f_icode == IRRMOVQ || 
      f_icode == IIRMOVQ || f_icode == IRMMOVQ || f_icode == IMRMOVQ || 
      f_icode == IOPQ || f_icode == IJXX || f_icode == ICALL || 
      f_icode == IRET || f_icode == IPUSHQ || f_icode == IPOPQ){
         return true;
   }
   return false;
}

uint64_t FetchStage::f_stat(uint64_t icode, bool memoryError, bool instruction_valid){
   if(memoryError){
      return SADR;
   }
   if(!instruction_valid){
      return SINS;
   }
   if(icode == IHALT){
      return SHLT;
   }
      return SAOK;
}

/**
 * determines if we need to stall the fetch register.
 */
bool FetchStage::F_stall(uint64_t D_icode, uint64_t M_icode, uint64_t E_icode, uint64_t E_dstM, uint64_t d_srcA, uint64_t d_srcB){
   if((E_icode == IMRMOVQ || E_icode == IPOPQ) && (E_dstM == d_srcA || E_dstM == d_srcB)){
      return true;
   }
   return false;
}
/**
 * determines if we need to stall the decode register.
 */
bool FetchStage::D_stall(uint64_t D_icode, uint64_t M_icode, uint64_t E_icode, uint64_t E_dstM, uint64_t d_srcA, uint64_t d_srcB){
   if((E_icode == IMRMOVQ || E_icode == IPOPQ) && (E_dstM == d_srcA || E_dstM == d_srcB)){
      return true;
   }
   return false;
}

/**
 * Calculates the f_stall variable and d_stall variable to see if we should stall or bubble.
 * 
 */
void FetchStage::calculateControlSignals(PipeReg ** pregs, Stage ** stages){
   DecodeStage * d_stage = (DecodeStage *)stages[DSTAGE];
   E * ereg = (E *) pregs[EREG];
   M * mreg = (M *) pregs[MREG];
   D * dreg = (D *) pregs[DREG];

   uint64_t E_icode = ereg->geticode()->getOutput();
   uint64_t M_icode = mreg->geticode()->getOutput();
   uint64_t D_icode = dreg->geticode()->getOutput();
   uint64_t E_dstM = ereg->getdstM()->getOutput();
   uint64_t d_srcA = d_stage->getd_srcA();
   uint64_t d_srcB = d_stage->getd_srcB();

   F_stall_var = F_stall(D_icode, M_icode, E_icode, E_dstM, d_srcA, d_srcB);
   D_stall_var = D_stall(D_icode, M_icode, E_icode, E_dstM, d_srcA, d_srcB);
}