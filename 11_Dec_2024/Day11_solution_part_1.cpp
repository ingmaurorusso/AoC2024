
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_1)

using namespace NAoc__MR;

NAoc__MR::TResult day11Part1(std::shared_ptr<std::istream> inputStream)
{
   std::cout << "----- PART 1 -----" << std::endl;

   unsigned lineCount{0U};

   TResult count{0U};

   while (true) {
      //  --- BEGIN LINE EXTRACTION ---
      auto [line, lineStream, flags] = lineExtraction<MaxLineLength>(inputStream);

      if (flags.endedLines) break;

      std::string msgLine
         = "Input at the line n. " + std::to_string(++lineCount) + " : ";

      if (flags.emptyLine){
         std::cout << msgLine << "WARNING: empty line\n";
         continue;
      }

      if (flags.tooLongLine){
         throw std::invalid_argument(msgLine + "too long Line");
      }
      //  --- END LINE EXTRACTION ---

      //  --- BEGIN LINE CHECKS ---
      if ( auto it = std::find_if(std::cbegin(line), std::cend(line), [](char ch){return !(std::isdigit(ch) || std::isspace(ch));});
           it != std::end(line) ){
         throw std::invalid_argument(msgLine + "unexpected non-digits: " + std::string(1,*it));
      }
      if (lineCount > 1){
         throw std::invalid_argument(msgLine + "more than 1 line!");
      }
      //  --- END LINE CHECKS ---

      using TValue = unsigned long;
      std::list<TValue> values;
      TValue v;
      while(lineStream >> v){
         values.push_back(v);
      }

      constexpr auto nBlink = 25U;

      for(auto i = 0; i < nBlink; ++i){
         // printIterable(values, " ");
         // std::cout << values.size() << std::endl;
         for(auto it = values.begin(); it != values.end(); ++it){

            std::string vStr = std::to_string(*it);
            if (*it == 0) *it = 1;
            else if (vStr.size()%2 == 0){
               TValue v1;
               std::from_chars(vStr.c_str(), vStr.c_str() + vStr.size()/2, v1);

               TValue v2;
               std::from_chars(vStr.c_str() + vStr.size()/2, vStr.c_str() + vStr.size(), v2);

               *it = v1;
               it = values.insert(++it,v2); // 'it' on the new value
            } else{
               checkProdResult(*it, 2024);
               *it *= 2024;
            }
         }
      }

      count = values.size();
   }

   std::cout << "Number of lines: " << lineCount << std::endl;
   std::cout << "\nResult P1: " << count << std::endl;
   std::cout << std::endl;

   return count;
}