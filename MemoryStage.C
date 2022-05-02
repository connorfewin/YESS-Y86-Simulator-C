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
#include "MemoryStage.h"
#include "Status.h"
#include "Debug.h"
#include "Instructions.h"
#include "Memory.h"


/*
 * doClockLow:
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool MemoryStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
   bool mem_error = false;
   M * mreg = (M *) pregs[MREG];
   W * wreg = (W *) pregs[WREG];
   Memory * mem = Memory::getInstance();

   stat = mreg->getstat()->getOutput();
   uint64_t icode = mreg->geticode()->getOutput();
   uint64_t valE = mreg->getvalE()->getOutput();
   uint64_t dstE = mreg->getdstE()->getOutput();
   uint64_t dstM = mreg->getdstM()->getOutput();
   uint64_t valA = mreg->getvalA()->getOutput();

   m_valM = 0;

   uint32_t addressToAccess = getAddr(mreg, icode);
   if (mem_read(icode)){
      //read a long at addressToAccess.
      m_valM = mem->getLong(addressToAccess,mem_error);
   }
   if (mem_write(icode)){
      //write a long at addressToAccess.
      mem->putLong(valA, addressToAccess, mem_error);
   }

   if(mem_error){
      stat = SADR;
   }
   setWInput(wreg, stat, icode, valE, m_valM, dstE, dstM);
   return false;
}

/* doClockHigh
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void MemoryStage::doClockHigh(PipeReg ** pregs)
{
   W * wreg = (W *) pregs[WREG];

   wreg->getstat()->normal();
   wreg->geticode()->normal();
   wreg->getvalE()->normal();
   wreg->getvalM()->normal();
   wreg->getdstE()->normal();
   wreg->getdstM()->normal();
}

void MemoryStage::setWInput(W * wreg, uint64_t stat, uint64_t icode, uint64_t valE,
   uint64_t valM, uint64_t dstE, uint64_t dstM){

   wreg->getstat()->setInput(stat);
   wreg->geticode()->setInput(icode);
   wreg->getvalE()->setInput(valE);
   wreg->getvalM()->setInput(valM);
   wreg->getdstE()->setInput(dstE);
   wreg->getdstM()->setInput(dstM);

}

uint64_t MemoryStage::get_valM() {
    return m_valM;
}

/**
 * getAddr - returns Address Component
 * 
 * @param: M_icode-icode from memory stage.
 * @param: M * mreg - instance of the m pipe register for access to return values.
 * 
 * @return: appropriate address value depending on icode.
 */

uint32_t MemoryStage::getAddr(M * mreg, uint64_t M_icode){
   if(M_icode == IRMMOVQ || M_icode == IPUSHQ || M_icode == ICALL || M_icode == IMRMOVQ){
      return mreg->getvalE()->getOutput();
   }
   else if(M_icode == IPOPQ || M_icode == IRET){
      return mreg->getvalA()->getOutput();
   }
   else return 0;
}

/**
 * Mem Read Methods - Boolean that return whether we need to read or write to mem.
 */
bool MemoryStage::mem_read(uint64_t M_icode){
   if(M_icode == IMRMOVQ || M_icode == IPOPQ || M_icode == IRET){
      return true;
   }
      return false;
}

bool MemoryStage::mem_write(uint64_t M_icode){
   if(M_icode == IRMMOVQ || M_icode == IPUSHQ || M_icode == ICALL){
      return true;
   }

   return false;
}

uint64_t MemoryStage::getm_stat(){
   return stat;
}