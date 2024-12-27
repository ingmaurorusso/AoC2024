
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_1)

using namespace NAoc__MR;


NAoc__MR::TResult day16Part1(std::shared_ptr<std::istream> inputStream)
{
   std::cout << "----- PART 1 -----" << std::endl;

   unsigned lineCount{0U};

   TResult sum{0U};

   std::vector<std::string> lines;

   PointUnsign start, end;
   bool foundStart = false, foundEnd = false;

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
         if (foundStart){
            throw std::invalid_argument(msgLine + "unexpected second start");
         }
         foundStart = true;
      }
      if (auto p = line.find(EndTile); p != std::string::npos){
         end = PointUnsign{p, lineCount-1U};
         if (foundEnd){
            throw std::invalid_argument(msgLine + "unexpected second end");
         }
         foundEnd = true;
      }

      lines.push_back(std::move(line));
   }

   if (!(foundStart && foundEnd)){
      throw std::invalid_argument("Absent start or end");
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
   using GraphPD = decltype(graph);

   std::unordered_set<typename GraphPD::Node> endNodes;

   for(Dir4Under d = 0; d < FourDir4; ++d){
      auto dir = Direction4{d};

      endNodes.insert({end,dir});
   }
   
   /*graph.edgeSetFrom({PointUnsign(7,5),Direction4::Right}); // TODO: remove
   graph.edgeSetTo({PointUnsign(7,5),Direction4::Right}); // TODO: remove

   graph.implode( endNodes,
      [&graph](){
         if (graph.edgeSetTo({PointUnsign(7,5),Direction4::Right}).empty()){
            throw 0;
         }
      } );*/

   //endNodes.insert({PointUnsign{23,133}, Direction4::Right});
   //endNodes.insert({PointUnsign{23,132}, Direction4::Down});
   auto [minPath, mapPaths_, ok] = graph.findMinPath(typename GraphPD::Node{start, Direction4::Right}, true, endNodes, true );
   //TODO: switch second parameter getPaths to false.

   std::cout << "Number of lines: " << lineCount << std::endl;
   std::cout << "\nResult P1: " << minPath << std::endl;
   std::cout << std::endl;

   return minPath;
}