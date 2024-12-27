
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_1)

using namespace NAoc__MR;

NAoc__MR::TResult day12Part1(std::shared_ptr<std::istream> inputStream)
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

      //  --- BEGIN LINE CHECKS ---
      //  --- END LINE CHECKS ---

      lines.emplace_back(std::move(line));
   }

   Field2D field{lines};

   using GraphIndir = Graph<PointUnsign, GraphKind::Indirect>;

   GraphIndir graph = field.getConnectionGraphByEdge<GraphKind::Indirect>(
                           [&field = std::as_const(field)](PointUnsign pSource, PointUnsign pDest){
                              return std::make_pair(field[pSource] == field[pDest], 1U);
                           } ); // ok any isolated node added.

   TResult sum = 0U;

   std::size_t nodeCount = 0U;
   std::size_t areaCount = 0U;

   GraphIndir::NodeSetRef reachedAlready;

   for(auto itNode = graph.cbegin(); itNode != graph.cend(); ++itNode){
      ++nodeCount;

      if (!reachedAlready.contains(itNode)){
         auto region = graph.getReachablesFrom(*itNode).first;
         region.insert(itNode);

         // find area and perimenter of the region.

         auto area = region.size();

         auto perimeter = decltype(area){0};

         auto ch = field[*itNode];
         for(auto itNodeRegion : region){
            Point p = *itNodeRegion;

            for (std::underlying_type_t<Direction4> d = 0; d < FourDir4; ++d){
               Point p2 = p;

               checkSumResult(perimeter, 1U);
               // ... well, maybe 'perimeter' would not have been increased.

               if (field.movePoint(p2,Direction4(d))){
                  if (field[p2] != ch){
                     ++perimeter;
                  }
               } else{
                  ++perimeter;
               }
            }
         }

         checkProdResult(area, perimeter);
         checkSumResult(sum, area*perimeter);
         sum += (area*perimeter);
         ++areaCount;

         reachedAlready.merge(region);
      }
   }

   std::cout << "Number of lines: " << lineCount << std::endl;
   std::cout << "Number of areas: " << areaCount << std::endl;
   std::cout << "\nResult P1: " << sum << std::endl;
   std::cout << std::endl;

   return sum;
}
