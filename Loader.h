
class Loader
{
   private:
      bool loaded;        //set to true if a file is successfully loaded into memory
      std::ifstream inf;  //input file handle
   public:
      Loader(int argc, char * argv[]);
      bool isLoaded();
      bool isValidFileName(char *filename);
      bool readFile(char *filename);
      uint32_t convert(std::string line, int start, int end);
      bool hasAddress1(std::string line);
      bool hasAddress2(std::string line);
      bool hasColon(std::string line);
      bool hasComment(std::string line);
      bool hasData(std::string line);
      uint32_t getAddress(std::string line);
      uint64_t getData(std::string line);
      void loadLine(std::string line);
      bool hasErrors(std::string line, std::string prevLine);
      bool isBlanks(std::string line);
      bool isAllBlanks(std::string line);
      bool hasDataError(std::string line);
      bool isSpecificBlanks(std::string line, int start);
      bool findFirstBlank(std::string line);
      bool hasHalfByte(std::string line);
      bool validAddress(std::string line);
      bool prevLineError(std::string line, std::string prevLine);
};
