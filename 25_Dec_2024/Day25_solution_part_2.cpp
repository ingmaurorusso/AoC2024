
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_1)

using namespace NAoc__MR;


NAoc__MR::TResult day25Part2(std::shared_ptr<std::istream> inputStream)
{
   std::cout << "----- PART 2 -----" << std::endl;

   unsigned lineCount{0U};

   std::vector<std::string> lines;

   while (true) {
      //  --- BEGIN LINE EXTRACTION ---
      auto [line, lineStream, flags] = lineExtraction<MaxLineLength>(inputStream);

      if (flags.endedLines) break;

      std::string msgLine
         = "Input at the line n. " + std::to_string(++lineCount) + " : ";

      if (flags.emptyLine){
         // std::cout << msgLine << "WARNING: empty line\n";
         continue;
      }

      if (flags.tooLongLine){
         throw std::invalid_argument(msgLine + "too long Line");
      }
      //  --- END LINE EXTRACTION ---

      //  --- BEGIN LINE CHECKS ---
      if (auto it = std::find_if_not( line.cbegin(), line.cend(),
                                      [](char ch){return true;} ) ; it != line.cend() ){
         throw std::invalid_argument(msgLine + "unexpected char: " + std::string(1,*it));
      }
      //  --- END LINE CHECKS ---

      lines.push_back(std::move(line));
   }

   std::cout << "\nNumber of lines: " << lineCount << std::endl;
   std::cout << "\nResult P2: rest in the Lord :-)" << std::endl;
   std::cout << std::endl;

   return 2024;
}
