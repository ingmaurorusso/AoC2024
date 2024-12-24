
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_2)

using namespace NAoc__MR;


NAoc__MR::TResult day16Part2(std::shared_ptr<std::istream> inputStream)
{
   std::cout << "----- PART 2 -----" << std::endl;

   unsigned lineCount{0U};

   TResult count{0U};

   std::vector<std::string> lines;

   PointUnsign start, end;

   constexpr char StartTile = 'S';
   constexpr char EndTile = 'E';

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
      static auto accepted = getStringWithChars({Field2D::FreeTile, Field2D::WallTile, StartTile, EndTile});
      if (auto it = std::find_if_not( line.cbegin(), line.cend(),
                                      [](char ch){return accepted.find(ch) != std::string::npos;} ) ; it != line.cend() ){
         throw std::invalid_argument(msgLine + "unexpected char: " + std::string(1,*it));
      }
      //  --- END LINE CHECKS ---

      if (auto p = line.find(StartTile); p != std::string::npos){
         start = PointUnsign{p, lineCount-1U};
      }
      if (auto p = line.find(EndTile); p != std::string::npos){
         end = PointUnsign{p, lineCount-1U};
      }

      lines.push_back(std::move(line));
   }

   Field2D field{lines};

   using EdgeCost = std::size_t;

   auto graph = field.getConnectionGraphWithExitDir<EdgeCost>(
                        getStringWithChars({StartTile}), getStringWithChars({Field2D::FreeTile}), getStringWithChars({Field2D::FreeTile, StartTile}),
                        getStringWithChars({EndTile,Field2D::FreeTile}), getStringWithChars({}), getStringWithChars({}),
                        [](const auto &edge){
                           auto dir1 = edge.first.second;
                           auto dir2 = edge.second.second;
                           bool sameAxis = (dir4IsVertical(dir1) == dir4IsVertical(dir2));
                           return sameAxis ? 1U : 1001U; });

   std::unordered_set<decltype(graph)::Node> endNodes;

   for(std::underlying_type_t<Direction4> d = 0; d < FourDir4; ++d){
      auto dir = Direction4{d};

      endNodes.insert({end,dir});
   }

   auto [minPath_, mapPaths, ok] = graph.findMinPath({start, Direction4::Right}, true, endNodes, true);

   if (ok){
      std::unordered_set<PointUnsign> setAll;
      for(const auto& [key,set] : mapPaths){
         for(auto pointAndDir_It : set){
            setAll.insert(pointAndDir_It->first);
         }
      }
      count = setAll.size() + 1U;
   }


   std::cout << "Number of lines: " << lineCount << std::endl;
   std::cout << "\nResult P2: " << count << std::endl;
   std::cout << std::endl;

   return count;
}
