
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_2)

using namespace NAoc__MR;

NAoc__MR::TResult day12Part2(std::shared_ptr<std::istream> inputStream)
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

      //  --- BEGIN LINE CHECKS ---
      //  --- END LINE CHECKS ---

      lines.emplace_back(std::move(line));
   }

   Field2D field{lines};

   using GraphIndir = Graph<PointUnsign, GraphKind::Indirect>;

   using GraphNodeIt = GraphIndir::NodeIt;

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
         region.insert(itNode); // needed for area and perimeter

         // find area and perimenter of the region.

         auto area = region.size();

         std::map<Direction4, GraphIndir::NodeSetRef > itPointsPerDir;

         auto ch = field[*itNode];
         for(auto itNodeRegion : region){
            Point p = *itNodeRegion;

            for (std::underlying_type_t<Direction4> d = 0; d < FourDir4; ++d){
               Point p2 = p;
               if (field.movePoint(p2,Direction4(d))){
                  if (field[p2] != ch){
                     itPointsPerDir[Direction4(d)].insert(itNodeRegion);
                  }
               } else{
                  itPointsPerDir[Direction4(d)].insert(itNodeRegion);
               }
            }
         }

         auto sideCount = decltype(area){0};

         for (std::underlying_type_t<Direction4> d = 0; d < FourDir4; ++d){
            // easier to filter first by nodes...
            auto subGraph = graph.buildSubGraph( [&itPoints = std::as_const(itPointsPerDir[Direction4(d)])]
                                                 (const GraphNodeIt& itNode)
                                                 {return itPoints.contains(itNode);} );

            // put together the points that are connected in the subgraph: for sure connections
            // parallel to the direction are excluded.

            GraphIndir::NodeSetRef subRreachedAlready;

            for(auto itPoint = subGraph.begin(); itPoint != subGraph.end(); ++itPoint){
               if (!subRreachedAlready.contains(itPoint)){
                  auto itPointsSameSide = subGraph.getReachablesFrom(*itPoint).first;

                  checkSumResult(sideCount, 1U);
                  ++sideCount;

                  subRreachedAlready.insert(itPoint); // not really needed
                  subRreachedAlready.merge(itPointsSameSide);
               }
            }
         }

         checkProdResult(area, sideCount);
         checkSumResult(sum, area*sideCount);
         sum += (area*sideCount);
         ++areaCount;

         reachedAlready.merge(region);
      }
   }

   std::cout << "Number of lines: " << lineCount << std::endl;
   std::cout << "Number of areas: " << areaCount << std::endl;
   std::cout << "\nResult P2: " << sum << std::endl;
   std::cout << std::endl;

   return sum;
}
