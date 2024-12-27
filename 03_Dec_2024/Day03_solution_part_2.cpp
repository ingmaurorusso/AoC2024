
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_2)

using namespace NAoc__MR;

NAoc__MR::TResult day03Part2(std::shared_ptr<std::istream> inputStream)
{
   std::cout << "----- PART 2 -----" << std::endl;

   unsigned lineCount{0U};

   TResult sum{0U};

   const std::regex doR(R"__($|(do\(\)))__");
   const std::regex dontR(R"__($|(don\'t\(\)))__");
   const std::regex mulR(R"__(mul\([0-9]{1,3}\,[0-9]{1,3}\))__");
   const std::regex valR(R"__([0-9]{1,3})__");

   bool doMul = true;

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
      
      auto itAllDo = line.cbegin();
      //std::match_results<decltype(it)> match{}; same as smatch
      std::smatch matchDo{};

      while (std::regex_search(itAllDo, line.cend(), matchDo, doMul ? dontR : doR)){
         auto itMul = itAllDo;

         itAllDo = matchDo.suffix().first; // == match[0].second
         // it updated for next cycle

         if (doMul){   
            std::smatch matchMul{};

            while (std::regex_search(itMul, itAllDo, matchMul, mulR)){
               itMul = matchMul.suffix().first; // == matchMul[0].second
               // it updated for next cycle

               const std::string_view mulExpr{matchMul[0].first, itMul};

               std::sregex_iterator itVal{matchMul[0].first, itMul, valR};
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

                  checkProdResult(prod,v);

                  prod *= v;
                  itVal++;
               }

               checkSumResult(sum,prod);
               sum += prod;
            }
         }

         // check if end-line was reached or switch-do
         if (matchDo[matchDo.size()-1].first != matchDo[matchDo.size()-1].second){
            // non-empty match: switch-do
            doMul = !doMul;
         }

         // Need to break as '$' would match an empty input
         if (itAllDo == line.cend()) break;
      }
   }

   std::cout << "Number of lines: " << lineCount << std::endl;
   std::cout << "\nResult P2: " << sum << std::endl;
   std::cout << std::endl;

   return sum;
}
