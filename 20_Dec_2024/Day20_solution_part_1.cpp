
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_1)

using namespace NAoc__MR;


NAoc__MR::TResult day20Part1(std::shared_ptr<std::istream> inputStream)
{
   std::cout << "----- PART 1 -----" << std::endl;

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

   auto graphWalls = field.getConnectionGraphByEdge< NGraph__MR::GraphKind::Indirect >(
      [&field](PointUnsign source, PointUnsign dest){
         return std::make_pair( (field[source] == Field2D::WallTile) ^ (field[dest] == Field2D::WallTile), 1U); },
         [](PointUnsign){return false;} );

   auto wallEdgeSet = graphWalls.edgeSet();


   auto [bestPath, prevs_, noLoop_] = graph.findMinPath( pStart, false, endSingleton);
   // TODO: redundant with distancesFrom/distancesTo.

   auto distMapFromStart = graph.distancesFrom(pStart);
   auto distMapToEnd = graph.distancesTo(pEnd);

   auto bestCheat = bestPath;
   
   const auto minSave = 100U;

   std::size_t isolatedWallNumber = 0U;

   for(const auto& wallNode : graphWalls){
      if (field[wallNode] != Field2D::WallTile) continue;

      auto localWallEdgeSet = graphWalls.edgeSetFrom(wallNode);

      if (localWallEdgeSet.size() < 2) continue; // boundary walls -> useless to check

      using EdgeIt = typename decltype(graph)::EdgeIt;
      using PointIt = typename decltype(graph)::NodeIt;
      using EdgeSet = std::unordered_set<EdgeIt>;
      using PointSet = std::unordered_set<PointIt>;

      //select couples on the same direction
      PointSet pSet1, pSet2;
      bool twice = false;

      auto p1 = *(localWallEdgeSet.begin()->second);
      auto p2 = *(std::next(localWallEdgeSet.begin())->second);

      if (localWallEdgeSet.size() > 2){ // 3 or 4
         // remove the one not aligned
         auto p3 = *(std::next(localWallEdgeSet.begin(),2U)->second);

         if ((p1.x == p3.x) || (p1.y == p3.y)){
            std::swap(p2,p3);
         } else
         if ((p2.x == p3.x) || (p2.y == p3.y)){
            std::swap(p1,p3);
         }

         if (localWallEdgeSet.size() > 3){
            ++isolatedWallNumber;

            auto p4 = *(std::next(localWallEdgeSet.begin(),3U)->second);

            pSet2.insert(graph.refNode(p3));
            pSet2.insert(graph.refNode(p4));

            twice = true;
         }
      }

      pSet1.insert(graph.refNode(p1));
      pSet1.insert(graph.refNode(p2));
      
      bool alreadyCheat = false;
      for(auto i = 0; i < 2; ++i){
         auto itP1 = *(pSet1.begin());
         auto itP2 = *(std::next(pSet1.begin()));

         auto it1S = distMapFromStart.find(itP1);
         auto it2S = distMapFromStart.find(itP2);

         auto it1E = distMapToEnd.find(itP1);
         auto it2E = distMapToEnd.find(itP2);

         bool okEdge = false;
         std::size_t minPath = bestPath;

         if ( (it1S != distMapFromStart.end()) && (it2E != distMapToEnd.end()) ){
            auto candidate = it1S->second + it2E->second + 2U;
            if (candidate < minPath){
               minPath = candidate;
            }
            okEdge = true;
         }
         if ( (it2S != distMapFromStart.end()) && (it1E != distMapToEnd.end()) ){
            auto candidate = it2S->second + it1E->second + 2U;
            if (candidate < minPath){
               minPath = candidate;
            }
            okEdge = true;
         }

         if (okEdge){
            if (minPath <= bestPath-minSave){
               if (!alreadyCheat){ // there are up to two sets for a single removed wall
                  ++count;
                  alreadyCheat = true;
               }
            }

            if (minPath <= bestCheat){
               bestCheat = minPath;
            }
         }

         if (twice){
            //eSet1 = eSet2;
            pSet1 = pSet2;
         } else break;
      }
   }

   std::cout << "Number of lines: " << lineCount << std::endl;
   std::cout << "\nResult P1: " << count << std::endl;
   std::cout << "\n... that save at least: " << minSave << std::endl;
   std::cout << "\nbestPath: " << bestPath << std::endl;
   std::cout << "\nbestCheat: " << bestCheat << std::endl;
   std::cout << "\nn. isolated walls: " << isolatedWallNumber << std::endl;

   std::cout << std::endl;

   return count;
}