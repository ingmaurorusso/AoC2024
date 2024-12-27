
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_2)

using namespace NAoc__MR;


NAoc__MR::TResult day20Part2(std::shared_ptr<std::istream> inputStream)
{
   std::cout << "----- PART 2 -----" << std::endl;

   unsigned lineCount{0U};

   TResult count{0U};

   std::vector<std::string> lines;

   PointUnsign pStart, pEnd;
   bool startAlready = false, endAlready = false;

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
      static std::string check = getStringWithChars({Field2D::WallTile, Field2D::FreeTile, Field2D::StartTile, Field2D::EndTile});
      if (auto it = std::find_if_not( line.cbegin(), line.cend(),
                                      [](char ch){return check.find(ch) != std::string::npos;} ) ; it != line.cend() ){
         throw std::invalid_argument(msgLine + "unexpected char: " + std::string(1,*it));
      }

      if (auto p = line.find(Field2D::StartTile); p != std::string::npos){
         pStart = PointUnsign(p,lineCount-1);
         if (startAlready || (line.find(Field2D::StartTile, p+1U) != std::string::npos)){
            throw std::invalid_argument(msgLine + "multiple start points");
         }
         startAlready = true;
      }

      if (auto p = line.find(Field2D::EndTile); p != std::string::npos){
         pEnd = PointUnsign(p,lineCount-1);
         if (endAlready || (line.find(Field2D::EndTile, p+1U) != std::string::npos)){
            throw std::invalid_argument(msgLine + "multiple end points");
         }
         endAlready = true;
      }
      //  --- END LINE CHECKS ---

      lines.push_back(std::move(line));
   }

   std::unordered_set<PointUnsign> endSingleton;
   endSingleton.insert(pEnd);

   Field2D field{lines};

   auto graph = field.getConnectionGraphByEdge< NGraph__MR::GraphKind::Indirect >(
      [&field](PointUnsign source, PointUnsign dest){
         return std::make_pair( ((field[source] == Field2D::FreeTile) || (field[source] == 'S'))
                                &&
                                ((field[dest] == Field2D::FreeTile) || (field[dest] == 'E')), 1U); },
         [](PointUnsign){return false;} );

   auto [bestPath, prevs0_, noLoop0_] = graph.findMinPath( pStart, false, endSingleton); //field.dimX()*field.dimY() + 1U;

   auto distMapFromStart = graph.distancesFrom(pStart);
   auto distMapToEnd = graph.distancesTo(pEnd);

   auto bestCheat = bestPath;

   const auto minSave = IsShortTest ? 50U : 100U;

   std::size_t nCouplesWithinAllowedDist = 0U;

   for(const auto& p1 : graph){
      auto itP1 = graph.refNode(p1);
      auto it1S = distMapFromStart.find(itP1);
      auto it1E = distMapToEnd.find(itP1);

      bool existsStartToP1 = (it1S != distMapFromStart.end());
      bool existsEndFromP1 = (it1E != distMapToEnd.end());

      if (!(existsStartToP1 || existsEndFromP1)) continue;

      for(const auto& p2 : graph){
         if (p2 <= p1) continue; // avoid count twice

         auto itP2 = graph.refNode(p2);

         auto it2S = distMapFromStart.find(itP2);
         auto it2E = distMapToEnd.find(itP2);

         const auto manDist = manDistance(p1,p2);

         constexpr auto MaxCheatTime = 20U;

         if (manDist > MaxCheatTime) continue;

         ++nCouplesWithinAllowedDist;

         bool okCheat = false;
         std::size_t minPath = bestPath;
         if ( existsStartToP1 && (it2E != distMapToEnd.end()) ){
            auto candidate = it1S->second + it2E->second + manDist;
            if (candidate < minPath){
               minPath = candidate;
            }
            okCheat = true;
         }
         if ( (it2S != distMapFromStart.end()) && existsEndFromP1 ){
            auto candidate = it2S->second + it1E->second + manDist;
            if (candidate < minPath){
               minPath = candidate;
            }
            okCheat = true;
         }

         if (okCheat){
            if (minPath <= bestPath-minSave){
               ++count;
            }

            if (minPath < bestCheat){
               bestCheat = minPath;
            }
         }
      }
   }

   std::cout << "Number of lines: " << lineCount << std::endl;
   std::cout << "\nResult P2: " << count << std::endl;
   std::cout << "\n... that save at least: " << minSave << std::endl;
   std::cout << "\nbestPath: " << bestPath << std::endl;
   std::cout << "\nbestCheat: " << bestCheat << std::endl;
   std::cout << "\nn. total allowed cheats: " << nCouplesWithinAllowedDist << std::endl;
   std::cout << std::endl;

   return count;
}