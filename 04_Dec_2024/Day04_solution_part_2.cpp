
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_2)

using namespace NAoc__MR;

NAoc__MR::TResult day04Part2(std::shared_ptr<std::istream> inputStream)
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


      lines.push_back(line);

      //  --- BEGIN LINE CHECKS ---
      if (line.size() != lines[0].size()){
         throw std::invalid_argument(msgLine + "lines with different length (idx 1 vs " + std::to_string(lineCount) + ")");
      }
      //  --- END LINE CHECKS ---
   }

   TResult count = 0U;

   if (lines.empty()){
      throw std::invalid_argument("no lines or only empty lines !");
   }

   CoordSign ny = lines.size();
   CoordSign nx = lines[0].size();

   constexpr auto MAS = "MAS"sv;
   constexpr auto LenXS = CoordSign{MAS.size()};

   const auto xmasPivotCheck = [&lines = std::as_const(lines), nx, ny, &count, &MAS](CoordSign i, CoordSign j){
      static std::string sample(LenXS, ' ');

      /*if constexpr(MAS.size()){
      } else{
         throw "MAS lost constexpr-ness"; does not happen
      }*/

      int f = -1;
      for(auto twice = 0; twice < 2; ++twice){
         auto m = 0U;
         for(auto k = -LenXS/2; k <= LenXS/2; ++k){
            sample[m++] = lines[j+k*f][i+k];
         }

         if (sample != MAS){
            std::reverse(sample.begin(),sample.end());
            if (sample != MAS) return false;
         }

         f = 1;
      }

      return true;
   };

   enum class DirAhead{Up, Straight, Down};
   constexpr auto NumDir = 3U;

   for(CoordSign i = 1; i < nx-1; ++i)
      for(CoordSign j = 1; j < ny-1; ++j)
         if ( (lines[j][i] == 'A') //speed-up
              && xmasPivotCheck(i,j) ){
            if (count == std::numeric_limits<TResult>::max()){
               throw std::runtime_error("Overflow");
            }
            ++count;
         }

   std::cout << "Number of lines: " << lineCount << std::endl;
   std::cout << "\nResult P2: " << count << std::endl;
   std::cout << std::endl;

   return count;
}
