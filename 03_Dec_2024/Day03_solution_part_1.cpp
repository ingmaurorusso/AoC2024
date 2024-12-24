
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_1)

using namespace NAoc__MR;

NAoc__MR::TResult day03Part1(std::shared_ptr<std::istream> inputStream)
{
   std::cout << "----- PART 1 -----" << std::endl;

   unsigned lineCount{0U};

   TResult sum{0U};
   const std::regex mulR(R"__(mul\([0-9]{1,3}\,[0-9]{1,3}\))__");
   const std::regex valR(R"__([0-9]{1,3})__");

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
      // No check to do.
      //  --- END LINE CHECKS ---
      
      auto itMul = line.cbegin();
      //std::match_results<decltype(it)> match{}; same as smatch
      std::smatch match{};

      while (std::regex_search(itMul, line.cend(), match, mulR)){
         itMul = match.suffix().first; // == match[0].second
         // it updated for next cycle

         const std::string_view mulExpr{match[0].first, itMul};

         std::sregex_iterator itVal{match[0].first, itMul, valR};
         std::sregex_iterator itValEnd{};

         TResult prod = 1;
         while(itVal != itValEnd){
            TResult v;
            switch(std::from_chars(&*((*itVal)[0].first), &*((*itVal)[0].second), v).ec){
            case std::errc::invalid_argument :
               // unexpected for 3 digits
               throw std::runtime_error(msgLine + "mul part invalid: " + std::string{mulExpr});
            case std::errc::result_out_of_range :
               // unexpected for 3 digits
               throw std::runtime_error(msgLine + "mul part out-of-range: " + std::string{mulExpr});
            }

            if (prod > std::numeric_limits<TResult>::max()/v){
               throw std::runtime_error("Overflow: TResult size not enough");
            }
            prod *= v;
            itVal++;
         }

         if (sum > std::numeric_limits<TResult>::max() - prod){
            throw std::runtime_error("Overflow: TResult size not enough");
         }

         sum += prod;
      }
   }

   std::cout << "Number of lines: " << lineCount << std::endl;
   std::cout << "\nResult P1: " << sum << std::endl;
   std::cout << std::endl;

   return sum;
}