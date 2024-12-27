
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_2)

using namespace NAoc__MR;

using TNode = unsigned;

NAoc__MR::TResult day05Part2(std::shared_ptr<std::istream> inputStream)
{
   std::cout << "----- PART 2 -----" << std::endl;

   unsigned lineCount{0U};

   TResult sum{0U};
   TResult nReorderedUpdates{0U};

   using GraphV = Graph<TResult, GraphKind::Direct>;
   GraphV graph;

   bool readingRules = true, firstUpdate = true;
   bool anyCircular = false;

   constexpr auto ForceExpectEmptyLine = true;

   while (true) {
      //  --- BEGIN LINE EXTRACTION ---
      auto [line, lineStream, flags] = lineExtraction<MaxLineLength>(inputStream);

      if (flags.endedLines) break;

      std::string msgLine
         = "Input at the line n. " + std::to_string(++lineCount) + " : ";

      if (flags.emptyLine){
         if (!readingRules){
            std::cout << msgLine << "WARNING: empty line\n";
         } // else: expected empty line between rules and update
         readingRules = false;
         continue;
      }

      if (flags.tooLongLine){
         throw std::invalid_argument(msgLine + "too long Line");
      }
      //  --- END LINE EXTRACTION ---

      //  --- BEGIN LINE CHECKS ---
      const auto getValue = [msgLine = std::as_const(msgLine)](auto &lineStream, auto &v){
         if (!(lineStream >> v)){ // LINE CHECK
            if (v < std::numeric_limits<std::decay_t<decltype(v)>>::max()){
               throw std::invalid_argument(msgLine + "not starting with a number");
            } else{
               throw std::invalid_argument(msgLine + "too high number");
            }
         }
      };

      if ( std::find_if(std::cbegin(line), std::cend(line), [](char ch){return !(std::isdigit(ch) || (ch == '|') || (ch == ','));})
           != std::end(line) ){
         throw std::invalid_argument(msgLine + "unexpected non-digits or kind of separators");
      }
      // ... other checks later along the code
      //  --- END LINE CHECKS ---

      unsigned v1{};

      getValue(lineStream,v1);

      char ch{};
      bool oneNumberOnly = !(lineStream >> ch);
      if ((ch == '|') && !oneNumberOnly){
         if (!readingRules){
            throw std::invalid_argument(msgLine + "another rule after the empty line"); // LINE CHECK
         }
         unsigned v2;
         getValue(lineStream,v2);

         if (v1 == v2){
            throw std::invalid_argument(msgLine + "rule with two equivalent values"); // LINE CHECK
         }

         // withPredecessors[v2].insert(v1);
         // withSuccessors[v1].insert(v2);
         graph << GraphV::Edge{v1,v2};
      } else
      if (oneNumberOnly || (ch == ',')){
         if (firstUpdate){
            if (readingRules)
               if constexpr(ForceExpectEmptyLine){
                  throw std::invalid_argument(msgLine + "not a rule before the empty line"); // LINE CHECK
               }
            
            readingRules = false; // from now on, only update
            firstUpdate = false;
            
            // need to propagate predecessors rules (the context worked even just doing the order
            // by relying on a 'direct' predecessor rule: see (_1_))

            auto [nodesOrder, noLoop, uniqueOrdering] = graph.topologicalOrderForDirect();

            if (!noLoop){
               std::cout << "WARNING: loop in the original set of rules\n";
               std::cout << "         Will proceed rebuilding local ordering for each line-update\n";
            } else
            if (uniqueOrdering){
               std::cout << "WOW: unique ordering for the global set of rules!\n";
            }
         }

         // new 'update' found
         std::vector<TNode> update;
         std::unordered_set<TNode> updateSet;
         update.push_back(v1);
         updateSet.insert(v1);
         if (!oneNumberOnly)
            do{
               if ((ch != ',')){
                  throw std::invalid_argument(msgLine + "unexpected separator: " + std::string(1,ch)); // LINE CHECK
               }

               unsigned v2;
               getValue(lineStream,v2);

               update.push_back(v2);
               updateSet.insert(v2);
            } while(lineStream >> ch);

         if ( update.size()%2 == 0 ){
            throw std::invalid_argument(msgLine + "update list with even number of values"); // LINE CHECK
         }

         bool goodUpdate = true;

         for(auto i = 0U; (i < update.size()-1U) && goodUpdate; ++i){
            for(auto j = i+1U; (j < update.size()) && goodUpdate; ++j){
               if (graph.hasEdge(update[j], update[i])){
                  goodUpdate = false;
               }
            }
         }

         if (!goodUpdate){
            // reorder

            auto [nodesOrder, noLoop, uniqueOrdering] = graph.topologicalOrderForDirect(updateSet); // updateSet as node set filter

            /* (_1_): this worked,... fortunately
            std::sort(update.begin(),update.end(), [&withPredecessors](const auto v1, const auto v2){
               const auto it2 = withPredecessors.find(v2);
               return (it2 != withPredecessors.cend()) && (it2->second.count(v1) > 0); // v1|v2 is in the rule list (unless v1 is zero)
               //return ruleSet.count(std::make_pair(v1,v2)) > 0;
            }); */

            if (!noLoop){
               throw std::invalid_argument(msgLine + "impossible to decide: circular precedence rules in the update: " + line); // LINE CHECK
            } else
            if (!uniqueOrdering){
               auto [connSet, noLoop_] = graph.getConnectedNodes(nodesOrder[nodesOrder.size()/2], updateSet);

               if ( connSet.size() != update.size()-1U ){ // => '<'
                  throw std::invalid_argument(msgLine + "not unique order for the target middle element: " + line); // LINE CHECK
               }
            }

            auto add = *(nodesOrder[nodesOrder.size()/2]);
            checkSumResult(sum, add);

            sum += add;
            ++nReorderedUpdates;
         }

         // printIterable(update,",");

      } else{
         throw std::runtime_error(msgLine + "separator: " + std::string(1,ch));
      }
   }

   if (graph.empty()){
      std::cout << "WARNING: no predecessors rules";
   }

   std::cout << "Number of lines: " << lineCount << std::endl;
   std::cout << "\nResult P2: " << sum << std::endl;
   std::cout << "\n n. reordered updates: " << nReorderedUpdates << std::endl;
   std::cout << std::endl;

   return sum;
}
