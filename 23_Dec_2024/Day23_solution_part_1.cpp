
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_1)

using namespace NAoc__MR;


NAoc__MR::TResult day23Part1(std::shared_ptr<std::istream> inputStream)
{
   std::cout << "----- PART 1 -----" << std::endl;

   unsigned lineCount{0U};

   TResult sum{0U};

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


   std::unordered_set<GraphPC::NodeIt> already;
   TResult n1 = 0U, n2 = 0U, n3 = 0U;
   for(const auto& pc : graph){
      if (pc[0] == 't'){
         auto connected = graph.edgeSetFrom(pc);
         unsigned couples = 0U;

         for(const auto pc1eIt : connected){
            for(const auto pc2eIt : connected){
               if (pc1eIt != pc2eIt){
                  if (graph.edgeExists(pc1eIt.second, pc2eIt.second)){
                     bool t1 = ((*pc1eIt.second)[0] == 't');
                     bool t2 = ((*pc2eIt.second)[0] == 't');

                     TResult n;
                     if (t1 && t2) n = ++n3;
                     else if (t1 || t2) n = ++n2;
                     else n = ++n1;

                     checkSumResult(n, TResult{1U}); // preventive.
                  }
               }
            }
         }
      }
   }

   checkSumResult(n1/2, n2/4);
   checkSumResult(n1/2 + n2/4, n3/6);

   sum = (n1 + n2/2 + n3/3) / 2;

   std::cout << "\nNumber of lines: " << lineCount << std::endl;
   std::cout << "\nResult P1: " << sum << std::endl;
   std::cout << std::endl;

   return sum;
}
