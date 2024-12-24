
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_1)

using namespace NAoc__MR;


NAoc__MR::TResult day22Part1(std::shared_ptr<std::istream> inputStream)
{
   std::cout << "----- PART 1 -----" << std::endl;

   unsigned lineCount{0U};

   TResult sum{0U};

   std::vector<std::string> lines;

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
      if (auto it = std::find_if_not( line.cbegin(), line.cend(),
                                      [](char ch){return std::isdigit(ch);} ) ; it != line.cend() ){
         throw std::invalid_argument(msgLine + "unexpected char: " + std::string(1,*it));
      }
      //  --- END LINE CHECKS ---

      lines.push_back(std::move(line));
   }

   static constexpr auto mix = [](TResult val, TResult& secret){
      secret = val ^ secret;
   };

   static constexpr auto prune = [](TResult& secret){
      //secret %= 16777216; // 0x1000000
      secret &= 0xFFFFFF;
   };

   static constexpr auto transform = [](TResult& secret){
      TResult res = secret << 6; // *64;
      mix(res,secret);
      prune(secret);

      res = secret >> 5; // / 32;
      mix(res,secret);
      prune(secret);

      res = secret << 11; // *2048;
      mix(res,secret);
      prune(secret);
   };

   constexpr auto NSeq = 2000U;

   for(const auto& line : lines){
      auto secret = getAllValues<TResult>(line)[0];

      for(int i = 0; i < NSeq; ++i){
         if (IsShortTest && (i <= 10)) std::cout << "  " << secret << std::endl;

         transform(secret);
      }

      if (IsShortTest) std::cout << " -> " << secret << std::endl;

      sum += secret;
   }

   std::cout << "\nNumber of lines: " << lineCount << std::endl;
   std::cout << "\nResult P1: " << sum << std::endl;
   std::cout << std::endl;

   return sum;
}