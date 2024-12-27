
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_1)

using namespace NAoc__MR;

NAoc__MR::TResult day04Part1(std::shared_ptr<std::istream> inputStream)
{
   std::cout << "----- PART 1 -----" << std::endl;

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

   using DeltaCoord = CoordSign;

   constexpr auto XMAS = "XMAS"sv;
   constexpr auto LenXS = CoordSign{XMAS.size()};

   const auto xmasCheck = [&lines = std::as_const(lines), nx, ny, &count, &XMAS]
                          (CoordSign i, CoordSign j, CoordSign dx, CoordSign dy){
      static std::string sample(LenXS, ' ');

      /*if constexpr(XMAS.size()){
      } else{
         throw "XMAS lost constexpr-ness"; does not happen
      }*/

      if ( (dx < 0) ? (i+1 >= -LenXS*dx) : (i <= nx - LenXS*dx) )
         if ( (dy < 0) ? (j+1 >= -LenXS*dy) : (j <= ny - LenXS*dy) ){ 
            for(auto k = 0; k < LenXS; ++k){
               sample[k] = lines[j][i];
               i += dx;
               j += dy;
            }

            if (sample == XMAS) return true;
            std::reverse(sample.begin(),sample.end());
            return (sample == XMAS);
         }
      
      return false;
   };

   enum class DirAhead{Up, Straight, Down};
   constexpr auto NumDir = 3U;

   for(CoordSign i = 0; i < nx; ++i)
      for(CoordSign j = 0; j < ny; ++j){
         //4-directions, 3 ahead + vertical
         int dx = 1;
         int dy{0U};
         for(auto dir = 0U; dir < NumDir; ++dir ){
            switch(static_cast<DirAhead>(dir)){
            case DirAhead::Up : dy = -1; break;
            case DirAhead::Straight : dy = 0; break;
            // case DirAhead::Down :
            default:
               dy = 1; break;
            }

            if (xmasCheck(i,j,dx,dy)){
               checkSumResult(count,1U);
               ++count;
            }
         }

         // vertical
         if (xmasCheck(i,j,0,1)){
            checkSumResult(count,1U);
            ++count;
         }
      }

   std::cout << "Number of lines: " << lineCount << std::endl;
   std::cout << "\nResult P1: " << count << std::endl;
   std::cout << std::endl;

   return count;
}