/**
 * Names:Kyle Kennedy and Connor Fewin
 * Team: Kyle and Connor 2
*/
#include <iostream>
#include <fstream>
#include <string.h>
#include <ctype.h>

#include "Loader.h"
#include "Memory.h"
#include "Tools.h"

//first column in file is assumed to be 0
#define ADDRBEGIN 2   //starting column of 3 digit hex address 
#define ADDREND 4     //ending column of 3 digit hex address
#define DATABEGIN 7   //starting column of data bytes
#define COMMENT 28    //location of the '|' character 

/**
 * Loader constructor
 * Opens the .yo file named in the command line arguments, reads the contents of the file
 * line by line and loads the program into memory.  If no file is given or the file doesn't
 * exist or the file doesn't end with a .yo extension or the .yo file contains errors then
 * loaded is set to false.  Otherwise loaded is set to true. Dr. Wilkes was here.
 *
 * @param argc is the number of command line arguments passed to the main; should
 *        be 2
 * @param argv[0] is the name of the executable
 *        argv[1] is the name of the .yo file
 */
Loader::Loader(int argc, char * argv[])
{
   loaded = false;

   //Start by writing a method that opens the file (checks whether it ends 
   //with a .yo and whether the file successfully opens; if not, return without 
   //loading)
   if (argc != 2){
      return;
   }
   else{
      char *filename = argv[1];
      if(isValidFileName(filename)){
         if (!readFile(filename)) {
            return;
         }
         //after we read file, we have to check if there were errors.
         //if there were errors, we do not set loaded to true.
      } else {
         return;
      }
   }

   loaded = true;
}

/**
 * isLoaded
 * returns the value of the loaded data member; loaded is set by the constructor
 *
 * @return value of loaded (true or false)
 */
bool Loader::isLoaded()
{
   return loaded;
}


//You'll need to add more helper methods to this file.  Don't put all of your code in the
//Loader constructor.  When you add a method here, add the prototype to Loader.h in the private
//section.
bool Loader::isValidFileName(char *filename){
    bool flag = true;
    int count = 1;
    while(flag){
        if(filename[count] == ' '){
            flag = false;
        } 
        else{
            if(count > 0 && filename[count] == '.'){
                flag = false;
                if(filename[count+1] == 'y' && filename[count+2] == 'o'){
                    return true;                
                }
            }
        }
        count++;
    }
   return false;
}

//Reads the file line by line
bool Loader::readFile(char *filename){
   std::ifstream inf(filename);
   std::string strInput;
   std::getline(inf, strInput);
   //std::cout << strInput << "\n";
   int lineNumber = 1;
   std::string prevLine;

   while(inf){
      std::getline(inf, strInput);
      lineNumber++;
      //if this line doesn't have errors, load it into memory.
      if (!hasErrors(strInput, prevLine)) {
            //implement loadline. Call it here. Definitely look at the getAddress comment. It's worth it ;)
            //make sure an empty string is not getting added to the first memory location.
            if (strInput.compare("") != 0) loadLine(strInput);
      }
      else {
           std::cout << "Error on line " << std::dec << lineNumber
              << ": " << strInput << std::endl;
            return false;
       }
      prevLine = strInput;
   }
   return true;
      //std::cout << strInput << "\n";
}


bool Loader::hasAddress1(std::string line){
   if(line[0] == '0'){
      return true;
   }
   else{
      return false;
   }
}

bool Loader::hasAddress2(std::string line){
   if(line[1] == 'x'){
      return true;
   }
   else{
      return false;
   }
}

bool Loader::hasColon(std::string line){
   if(line[5] == ':'){
      //check if there is a space after the colon.
      if (line[6] == ' '){
         return true;
      }
   }
      return false;
}

bool Loader::hasComment(std::string line){
   if(line[COMMENT] == '|'){
      return true;
   }
   else{
      return false;
   }
}

bool Loader::hasData(std::string line){
   if(line[DATABEGIN] > 'f' || (line[DATABEGIN] < 'a' && line[DATABEGIN] > '9') || line[DATABEGIN] < '0'){
      return false;
   }
   else{
      return true;
   }
}

/*
   This might need to return a 64bit number. If you look at Memory.h the value i an unsigned 64bit integer
   We are gonna go with a trial and error style on this one. Might need a seperate convert function, but
   that seems kind of stupid. Like...what's the point, you know? Anyway, I hope this comment finds you well
   and you are still in good spirits. In case you are not, connor and/or kyle, you are talented and you are
   loved. 
*/
uint32_t Loader::getAddress(std::string line){
   return convert(line, ADDRBEGIN, ADDREND);
}

uint64_t Loader::getData(std::string line){
   int end = 0;
   for(int i = DATABEGIN; i < COMMENT; i++){
      if(line[i] == ' '){
         end = i - 1;
         i = COMMENT;
      }
   }
   return convert(line, DATABEGIN, end);
}

//Use put byte, loop through data starting at address and incrementing address and the data.
void Loader::loadLine(std::string line){
   int byte = DATABEGIN;
   uint8_t byteData;
   bool imem_error = false;
   //get the address
   uint32_t address = convert(line,ADDRBEGIN,ADDREND);
   //get the data
   while(isxdigit(line.c_str()[byte]) && isxdigit(line.c_str()[byte + 1])) {
      byteData = convert(line, byte, byte + 1);
      Memory::getInstance()->putByte(byteData, address, imem_error);
      
      byte += 2;

      address+=1;
   }
}

uint32_t Loader::convert(std::string line, int start, int end) {
    //TODO: write check bounds method, to verify return type doesnt overflow.
    std::string data = "";
    for (int i = start; i <= end; i++) {
        data += line.c_str()[i];
    }
    return strtol(data.c_str(), NULL, 16);
}

/**
 * Takes instruction from current line and calls other error check methods.
 * Look at address and data of the line for errors.
 * 
 * @return true if there are errors in the instruction, else false.
 **/
 bool Loader::hasErrors(std::string line, std::string prevLine){
    //check each error one at a time, returning if true with line number.
    //check if the line is all blanks, if it is, no error, return.
    if (isBlanks(line)){
       if(!hasComment(line)){
          //if blank and doesn't have a comment, check if whole line is blank. if not, error.)
          if(!isAllBlanks(line)){
             return true;
          }
       }
    }
    //error1.
   else{
      if (!hasColon(line)){
         return true;
      } 
      
      if (!hasAddress1(line)){
         return true;
      }

      if (!hasAddress2(line)){
         return true;
      }

      if (!hasComment(line)){
         return true;
      }
      if(!hasDataError(line)){
         return true;
      }
      if(!findFirstBlank(line)){
         return true;
      }
      if(!hasHalfByte(line)){
         return true;
      }
      if(!validAddress(line)){
         return true;
      }
      if(!prevLineError(line, prevLine)){
         return true;
      }
   
   }
   //if line passes all error checking methods, there are no errors, return false.

      return false;
}

/**
 * this function returns true if the data part of an input line has errors.
 **/
bool Loader::hasDataError(std::string line){
   for(int i = DATABEGIN; i < COMMENT; i++){
      if(line.c_str()[i] > 'f'){
         return false;
      }
      if(line.c_str()[i] < 'a' && line.c_str()[i] > '9'){
         return false;
      }
      if(line.c_str()[i] < '0' && line.c_str()[i] > ' '){
         return false;
      }
      if(line.c_str()[i] < ' '){
         return false;
      }
   }
   return true;
}  

bool Loader::isBlanks(std::string line){
      std::string subString = line.substr(0, COMMENT - 1);

      if (subString.find_first_not_of(' ') != std::string::npos){
         return false;
      }
      else{
         return true;
      }
}

bool Loader::isAllBlanks(std::string line){
   if (line.find_first_not_of(' ') != std::string::npos){
      return false;
   }
   else{
      return true;
   }
}


bool Loader::findFirstBlank(std::string line){
   int start = DATABEGIN;
   if(hasData(line)){
      for(int i = DATABEGIN+1; i < COMMENT; i++){
         if(line.c_str()[i] == ' '){
            start = i;
            i = COMMENT;
         }
      }
      //printf("start = %d\n", start);
      return isSpecificBlanks(line, start);
   }
   else{
      return true;
   }
   
   
}

bool Loader::isSpecificBlanks(std::string line, int start){
      
      for(int i = start; i < COMMENT; i++){
         if(line.c_str()[i] != ' '){
            return false;
         }
      }
      return true;
}

bool Loader::hasHalfByte(std::string line){
   int count = 0;
   if(line.c_str()[DATABEGIN]==' ' && line.c_str()[DATABEGIN + 1]!=' '){
      return false;
   }
   for(int i = DATABEGIN; i < COMMENT; i++){
      if(line.c_str()[i] != ' '){
         count++;
      }
      else{
         i = COMMENT;
      }
   }
   if(count % 2 != 0){
      return false;
   }
   else{
      return true;
   }
}

bool Loader::validAddress(std::string line){
   uint64_t addr = convert(line, 2, 4);
   uint64_t count = 0;
   for(int i = DATABEGIN; i < COMMENT; i++){
      if(line.c_str()[i] != ' '){
         count++;
      }
      else{
         i = COMMENT;
      }
   }
   count /= 0x002;
   addr = 0xfff - addr + 1;
   if (count > addr){
      return false;
   }
   else{
      return true;
   }
}

bool Loader::prevLineError(std::string line, std::string prevLine){
   uint64_t currAddr = convert(line, 2, 4);
   uint64_t prevAddr = convert(prevLine, 2, 4);
   uint64_t prevData = 0x0;
   if(hasData(prevLine) && hasData(line)){
      for(int i = DATABEGIN; i < COMMENT; i++){
         if(prevLine.c_str()[i] != ' '){
            prevData++;
         }
         else{
            i = COMMENT;
         }
      }
      prevData /= 0x002;
      if(prevAddr + prevData > currAddr){
         return false;
      }
      else{
         return true;
      }
   }
   else{
      return true;
   }
}