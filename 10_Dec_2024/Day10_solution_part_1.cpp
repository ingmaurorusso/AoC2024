
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_1)

using namespace NAoc__MR;

NAoc__MR::TResult day10Part1(std::shared_ptr<std::istream> inputStream)
{
   std::cout << "----- PART 1 -----" << std::endl;

   unsigned lineCount{0U};

   TResult sum{0U};

   std::vector<std::string> lines;
   std::unordered_set<PointUnsign> zeros;

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
      if ( auto it = std::find_if(std::cbegin(line), std::cend(line), [](char ch){return !std::isdigit(ch);});
           it != std::end(line) ){
         throw std::invalid_argument(msgLine + "unexpected non-digits: " + std::string(1,*it));
      }
      //  --- END LINE CHECKS ---

      
      for(auto x0 = line.find("0"); x0 != std::string::npos; x0 = line.find("0", x0+1)){
         zeros.insert(PointUnsign{x0,lineCount-1U});
      }

      lines.emplace_back(std::move(line));
   }

   Field2D field{lines};

   Graph<PointUnsign, GraphKind::Direct> graph;

   graph = field.getConnectionGraphByEdge<GraphKind::Direct>(
               [&field = std::as_const(field)](PointUnsign pSource, PointUnsign pDest)
               { return std::make_pair(field[pDest] == field[pSource]+1U, 1U); },
               [](PointUnsign pIsolated){return false;} );


   // compute reachable '9' from each '0'
   for(auto zero : zeros){
      auto add = graph.countReachablesFrom( zero,
                  [&field = std::as_const(field)](const PointUnsign& point){
                     return field[point] == '9';}
                 ).first;

      checkSumResult(sum, add);
      sum += add;
   }

   std::cout << "Number of lines: " << lineCount << std::endl;
   std::cout << "\nResult P1: " << sum << std::endl;
   std::cout << std::endl;

   return sum;
}