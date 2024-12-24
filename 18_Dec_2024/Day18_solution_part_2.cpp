
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_2)

using namespace NAoc__MR;


NAoc__MR::TResult day18Part2(std::shared_ptr<std::istream> inputStream)
{
   std::cout << "----- PART 2 -----" << std::endl;

   unsigned lineCount{0U};

   TResult sum{0U};

   std::vector<std::string> lines;

   std::list<PointUnsign> bytesPos;

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
      auto v = getAllValues<CoordUnsign>(line);

      if (v.size() != 2){
         throw std::invalid_argument(msgLine + "not exactly 2 values");
      }
      if (getString(v, ",") != line){
         throw std::invalid_argument(msgLine + "not exactly only comma in the middle");
      }
      //  --- END LINE CHECKS ---

      bytesPos.push_back(PointUnsign{v[0],v[1]});

      lines.push_back(std::move(line));
   }

   CoordUnsign dim = IsShortTest ? 7U : 71U;

   CoordUnsign fallNumber = IsShortTest ? 12U : 1024U;

   Field2D field(dim, dim, Field2D::FreeTile );

   std::size_t i = 0;
   for(auto& p : bytesPos){
      if (fallNumber >= ++i){
         field[p] = '#';
      }
   }

   using GraphP = Graph<PointUnsign,GraphKind::Indirect>;

   GraphP graph = field.getConnectionGraphByEdge<GraphKind::Indirect>(
      [&field = std::as_const(field)](PointUnsign pSource, PointUnsign pDest)
      {return std::make_pair( (field[pSource] != Field2D::WallTile) && (field[pDest] != Field2D::WallTile), 1U );},
      [](const auto p){return false;} // avoid isolated nodes
   );

   auto end = PointUnsign{dim-1U,dim-1U};

   std::unordered_set<PointUnsign> ends;
   ends.insert(end);

   std::unordered_set<PointUnsign> addedWalls;

   i = 0;
   PointUnsign pRes{};
   std::size_t firstPpos = 0U;
   auto endRef = graph.refNode(end);
   for(auto& p : bytesPos){
      if (fallNumber < ++i){
         addedWalls.insert(p);
         auto [reached, noLoop_] = graph.getReachablesFrom( PointUnsign{0,0}, GraphP::predAnyNodeIt,
                                    [&addedWalls](typename GraphP::NodeIt itNode1, typename GraphP::NodeIt itNode2){
                                        return !(addedWalls.contains(*itNode1) || addedWalls.contains(*itNode2));
                                    } );

         if (!reached.contains(endRef)){
            pRes = p;
            firstPpos = i;
            break;
         }
      }
   }

   std::cout << "Number of lines: " << lineCount << std::endl;
   std::cout << "\nResult P2: " << ((firstPpos > 0)? pointToStr(pRes) : "no point!") << std::endl;
   std::cout << "The point was in position n. " << firstPpos << std::endl;
   std::cout << std::endl;

   return 0;
}