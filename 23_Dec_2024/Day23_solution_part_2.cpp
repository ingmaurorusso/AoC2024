
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_2)

using namespace NAoc__MR;


NAoc__MR::TResult day23Part2(std::shared_ptr<std::istream> inputStream)
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
      if (auto it = std::find_if_not( line.cbegin(), line.cend(),
                                      [](char ch){return std::isalpha(ch) || (ch == '-');} ) ; it != line.cend() ){
         throw std::invalid_argument(msgLine + "unexpected char: " + std::string(1,*it));
      }
      //  --- END LINE CHECKS ---

      lines.push_back(std::move(line));
   }

   using GraphPC = Graph<std::string, GraphKind::Indirect>;
   GraphPC graph;

   for(const auto& line : lines){
      auto cps = split(line, "-", false);

      graph << GraphPC::Edge{cps[0],cps[1]};
   }


   /*
   std::size_t best = 0U;;
   GraphPC::NodeSetRef bestClique;

   GraphPC::NodeSetRef alreadyVisited;

   for(const auto& pc : graph){
      auto pcIt = graph.refNode(pc);

      auto connected = graph.edgeSetFrom(pc);

      // speed-up not to consider the nodes already visited.
      //TODO: implement as https://www.sciencedirect.com/science/article/pii/S0166218X01002906
      for(auto itC = connected.begin(); itC != connected.end();){
         if (alreadyVisited.contains(itC->second)){
            itC = connected.erase(itC);
         } else ++itC;
      }

      std::size_t ub = connected.size()+1U;

      std::unordered_set<GraphPC::NodeIt> bestCliqueForPc;

      for(const auto pc1eIt : connected){
         if (ub <= best){
            break; // speed-up, useless to continue
         }

         bool ok = true;
         for(auto cliquePcIt : bestCliqueForPc){
            if (!graph.edgeExists(pc1eIt.second, cliquePcIt)){
               ok = false;
               --ub;
               break;
            }
         }

         if (ok){
            bestCliqueForPc.insert(pc1eIt.second);
         }
      }

      bestCliqueForPc.insert(pcIt); // can be done at the end, it's faster not to check for it
      alreadyVisited.insert(pcIt);

      if (bestCliqueForPc.size() > best){
         best = bestCliqueForPc.size();

         bestClique.clear();
         for(auto pcIt : bestCliqueForPc){
            bestClique.insert(pcIt);
         }
      }
   }
   */

   auto bestClique = graph.getMaxClique();

   std::vector<std::string> pcs;
   /*for(auto pcIt : bestClique){
      pcs.push_back(*pcIt);
   }*/
   transformContainer(pcs, bestClique, [](auto pcIt){return *pcIt;});

   std::sort(pcs.begin(), pcs.end());

   std::string pwd = join(pcs,",");

   std::cout << "\nNumber of lines: " << lineCount << std::endl;
   std::cout << "\nResult P2: " << pwd << std::endl;
   std::cout << std::endl;

   return bestClique.size();
}
