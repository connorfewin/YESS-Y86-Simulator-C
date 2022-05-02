CC = g++
CFLAGS = -g -c -Wall -std=c++11 -O0
OBJ = yess.o Memory.o Loader.o Tools.o RegisterFile.o ConditionCodes.o D.o W.o M.o F.o E.o Simulate.o PipeRegField.o PipeReg.o FetchStage.o DecodeStage.o ExecuteStage.o MemoryStage.o WritebackStage.o

.C.o:
	$(CC) $(CFLAGS) $< -o $@

yess: $(OBJ)

yess.o: Debug.h Memory.h Loader.h RegisterFile.h ConditionCodes.h PipeReg.h Stage.h Simulate.h

Memory.o: Tools.h

Loader.o: Memory.h Tools.h

Tools.o:

RegisterFile.o: Tools.h

ConditionCodes.o: Tools.h

D.o: Instructions.h RegisterFile.h PipeReg.h PipeRegField.h Status.h

W.o: RegisterFile.h Instructions.h PipeRegField.h PipeReg.h Status.h

M.o: RegisterFile.h Instructions.h PipeRegField.h PipeReg.h Status.h

F.o: PipeRegField.h PipeReg.h

E.o: RegisterFile.h Instructions.h PipeRegField.h PipeReg.h Status.h

Simulate.o: PipeRegField.h PipeReg.h F.h D.h E.h M.h W.h Stage.h ExecuteStage.h MemoryStage.h DecodeStage.h FetchStage.h WritebackStage.h Memory.h RegisterFile.h ConditionCodes.h

PipeRegField.o: 

PipeReg.o: 

FetchStage.o: RegisterFile.h PipeRegField.h PipeReg.h F.h D.h M.h W.h E.h Stage.h Status.h Debug.h Instructions.h Memory.h Tools.h ExecuteStage.h DecodeStage.h

DecodeStage.o: RegisterFile.h PipeRegField.h PipeReg.h F.h D.h M.h W.h E.h Stage.h Instructions.h ExecuteStage.h MemoryStage.h Status.h Debug.h

ExecuteStage.o: RegisterFile.h PipeRegField.h PipeReg.h F.h D.h E.h M.h W.h Stage.h Status.h Debug.h Instructions.h Tools.h MemoryStage.h ConditionCodes.h

MemoryStage.o: RegisterFile.h PipeRegField.h PipeReg.h F.h D.h M.h W.h E.h Stage.h Status.h Debug.h Instructions.h Memory.h ExecuteStage.h

WritebackStage.o: RegisterFile.h PipeRegField.h PipeReg.h F.h D.h M.h W.h Stage.h Status.h Debug.h Instructions.h

clean:
	rm $(OBJ) yess

run:
	make clean
	make yess
	./run.sh

