
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_2)

using namespace NAoc__MR;

NAoc__MR::TResult day11Part2(std::shared_ptr<std::istream> inputStream)
{
   std::cout << "----- PART 2 -----" << std::endl;

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

      using TValue = unsigned long long;
      using TTimes = TValue;
      std::map<TValue, TTimes> values;
      TValue v;
      while(lineStream >> v){
         values[v]++;
      }

      constexpr auto nBlink = 75U;

      for(auto i = 0; i < nBlink; ++i){
         //printIterable(values, " ");
         //std::cout << std::accumulate(values.cbegin(), values.cend(), 0U, [](auto acc, auto p){return acc + p.second;}) << std::endl;

         std::map<TValue, TTimes> newValues;
         for(auto it = values.begin(); it != values.end(); ++it){
            std::string vStr = std::to_string(it->first);
            if (it->first == TValue{0U}){
               newValues[TValue{1U}] += it->second;
            } else if (vStr.size()%2 == 0){
               TValue v1;
               std::from_chars(vStr.c_str(), vStr.c_str() + vStr.size()/2, v1);

               TValue v2;
               std::from_chars(vStr.c_str() + vStr.size()/2, vStr.c_str() + vStr.size(), v2);

               std::string s1 = std::to_string(v1);

               // if (v1 == v2) std::cout << "funny!\n";

               newValues[v1] += it->second;
               newValues[v2] += it->second;
            } else{
               if (it->first > std::numeric_limits<TValue>::max()/2024){
                  throw std::runtime_error("TValue needs to be a wider type");
               }

               newValues[it->first * 2024] += it->second;
            }
         }

         values = std::move(newValues);

         // mergeMapCombining( values, newValues, [](TTimes t1, Ttimes t2){return t1 + t2;} );
      }

      count = std::accumulate(values.cbegin(), values.cend(), TResult{0U}, [](auto acc, auto p){
         if (acc > std::numeric_limits<decltype(acc)>::max() - p.second){
            throw std::runtime_error("TValue needs to be a wider type");
         }
         return acc + p.second;
      });
   }

   std::cout << "Number of lines: " << lineCount << std::endl;
   std::cout << "\nResult P2: " << count << std::endl;
   std::cout << std::endl;

   return count;
}
