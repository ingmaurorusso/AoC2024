
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_2)

using namespace NAoc__MR;


NAoc__MR::TResult day22Part2(std::shared_ptr<std::istream> inputStream)
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

   //std::unordered_map<VectorNSign<4>, TResult> bananas;
   std::map<VectorNSign<4>, TResult> bananas; // faster, 6 vs 39 secs

   constexpr auto NSeq = 2000U;

   for(const auto& line : lines){
      auto secret = getAllValues<TResult>(line)[0];

      // might use a representative index for the vector of last 4 changes,
      // in order to simplify keys management: 2 decimal digits for each change-index,
      // and +10 if corresponding change is negative (values from 0 to 19, 10 excluded).
      // This was done in the Python implementation.
      std::map<VectorNSign<4>, TResult> buyerBananas;

      std::array<int,NSeq> changes;
      int oldPrice = secret%10;
      if (IsShortTest) std::cout << std::endl;
      for(int i = 0; i < NSeq; ++i){
         if (IsShortTest && (i <= 10)) std::cout << "  " << secret << std::endl;

         transform(secret);

         const int newPrice = secret%10;

         changes[i] = newPrice - oldPrice;

         if (i >= 3){
            VectorNSign<4> fourChanges;
            for(int k = 0; k < 4; ++k)
               fourChanges[3-k] = changes[i-k];

            if (auto& buyerPrice = buyerBananas[fourChanges]; buyerPrice == 0){
               buyerPrice = newPrice;
            }
         }

         oldPrice = newPrice;
      }

      if (IsShortTest) std::cout << " -> " << secret << std::endl;

      for(const auto& [changes4, firstPrice] : buyerBananas){
         bananas[changes4] += firstPrice;
      }
   }

   TResult best = 0;
   VectorNSign<4> bestComb;
   for(const auto& [changes4, number] : bananas){
      if (number > best){
         best = number;
         bestComb = changes4;
      }
   }

   std::cout << "\nNumber of lines: " << lineCount << std::endl;
   std::cout << "\nResult P2: " << best << std::endl;
   std::cout << "\n  best changes[4]: " << bestComb << std::endl;
   std::cout << std::endl;

   return best;
}