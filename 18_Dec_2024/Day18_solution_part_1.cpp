
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_1)

using namespace NAoc__MR;


NAoc__MR::TResult day18Part1(std::shared_ptr<std::istream> inputStream)
{
   std::cout << "----- PART 1 -----" << std::endl;

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

   Field2D field(dim,dim,'.');

   std::size_t i = 0;
   for(auto& p : bytesPos){
      if (fallNumber >= ++i){
         field[p] = Field2D::WallTile;
      }
   }

   using GraphP = Graph<PointUnsign,GraphKind::Indirect>;

   GraphP graph = field.getConnectionGraphByEdge<GraphKind::Indirect>(
      [&field = std::as_const(field)](PointUnsign pSource, PointUnsign pDest)
      {return std::make_pair( (field[pSource] != Field2D::WallTile) && (field[pDest] != Field2D::WallTile), 1U );},
      [](const auto p){return false;} // avoid isolated nodes
   );

   std::unordered_set<PointUnsign> ends;
   ends.insert(PointUnsign{dim-1U,dim-1U});

   

   auto [minPath, paths_, somePath] = graph.findMinPath( PointUnsign{0,0}, false,
                                                         [&ends = std::as_const(ends)](typename GraphP::NodeIt itNode){
                                                           return ends.contains(*itNode);
                                                         } );


   std::cout << "Number of lines: " << lineCount << std::endl;
   std::cout << "\nResult P1: " << (somePath? std::to_string(minPath) : "no path!") << std::endl;
   std::cout << std::endl;

   //std::stringstream ss;
   //printIterableOnStream(ss, output, ",");

   return minPath;
}