
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_2)

using namespace NAoc__MR;

using TNode = unsigned;
using TRank = unsigned;
using TRelations = std::unordered_map<TNode, std::set<TNode> >;
using TLabels = std::unordered_map<TNode, TRank>;

// cannot manage cycles -> would loop forever.
TRank countConnected( TNode startingNode,
                      const TRelations& localPredecessors, const TRelations& localSuccessors ){
   std::set<TNode> allPred, newPred;
   if (auto itp = localPredecessors.find(startingNode); itp != localPredecessors.cend()){
      newPred = itp->second;
   }
   while(!newPred.empty()){
      std::set<TNode> oldPred{std::move(newPred)};

      for(auto elem : oldPred){
         if (auto itp = localPredecessors.find(elem); itp != localPredecessors.cend()){
            newPred.merge(std::set<TNode>{itp->second});
         }
      }

      allPred.merge(oldPred);
   }

   std::set<TNode> allSucc, newSucc;
   if (auto its = localSuccessors.find(startingNode); its != localSuccessors.cend()){
      newSucc = its->second;
   }
   while(!newSucc.empty()){
      std::set<TNode> oldSucc{std::move(newSucc)};

      for(auto elem : oldSucc){
         if (auto its = localSuccessors.find(elem); its != localSuccessors.cend()){
            newSucc.merge(std::set<TNode>{its->second});
         }
      }

      allSucc.merge(oldSucc);
   }

   return allPred.size() + allSucc.size() + 1U;
}

template<class SortedContainer>
auto filterRelations( const TRelations& globalPredecessors, const TRelations& globalSuccessors,
                      const SortedContainer& focusedElements ){
   TRelations localPredecessors, localSuccessors;

   for(const auto node : focusedElements){
      if (auto itp = globalPredecessors.find(node); itp != globalPredecessors.cend()){
         for(const auto pred : itp->second){
            if (focusedElements.contains(pred)){
               localPredecessors[node].insert(pred);
               localSuccessors[pred].insert(node);
            }
         }
      }
   }

   return std::make_pair(std::move(localPredecessors),std::move(localSuccessors));
}

template<class SortedContainer>
bool buildGlobalLabelling( TLabels &labelsOut, bool &uniqueOrderingOut,
                           const TRelations& withPredecessors, const TRelations& withSuccessors,
                           const SortedContainer& focusedElements){
   // Algorithm: start from sources or sinks, and propagate.
   // Start labelling the nodes that have no predecessors
   TRank idx{};

   if (withPredecessors.empty()){ // => even withSuccessors is empty
      labelsOut.clear();
      uniqueOrderingOut = (focusedElements.size() == 1U);
      // labels are not set for the elements without predecessors/successors.
      // When focusedElements is empty, there is no way to know how many values
      // may be later involved, therefore uniqueOrderingOut is not meaningful
      // in this case.
      return true; // no cycles.
   }

   using TSources = std::unordered_set<unsigned>; // to check for circular references.

   constexpr bool UseLazyCircularDiscovery = true; // less memory, likely also faster when no circle exists
   const TRank thresholdCirc = focusedElements.empty()? withPredecessors.size() : focusedElements.size();
   

   std::unordered_map<TNode, TSources> toPropagate;
   std::unordered_map<TNode, TRank> maxDistance; // to support confirming uniqueOrderingOut

   uniqueOrderingOut = true;

   bool anyCircular = true;

   // start searching sources.
   {
      labelsOut.clear();
      toPropagate.clear();
      maxDistance.clear();
      idx = 0U;

      for (const auto& [v, succs_] : withSuccessors){
         if (!withPredecessors.count(v)){
            labelsOut[v] = ++idx;
            toPropagate[v]; //insert with empty TSources{}
            maxDistance[v] = 0U;
         }
      }

      anyCircular = labelsOut.empty() && !withPredecessors.empty();

      uniqueOrderingOut &= (labelsOut.size() == 1U);
      // For the unique ordering, it is needed to have one single source and one single sink
   }

   TRank maxIdx = idx;

   while (!(toPropagate.empty() || anyCircular)){
      auto [v, sources] = std::move(*(toPropagate.cbegin()));
      toPropagate.erase(toPropagate.cbegin());
      idx = labelsOut[v];
      if (idx > maxIdx){
         maxIdx = idx;
      }

      if constexpr(!UseLazyCircularDiscovery){
         if (!sources.insert(v).second){
            // found a circular reference propagation !
            anyCircular = true;
         }
      }

      // update all its successors with values higher than the one of 'v'.
      if (auto its = withSuccessors.find(v); its != withSuccessors.cend()){
         unsigned countSucc{0U};
         for (auto succ : its->second){
            if (!focusedElements.empty()){
               // only the successors within focusedElements are considered
               if (!focusedElements.count(succ))
                  continue;
            }

            ++countSucc;
            labelsOut[succ] = ++idx;

            if constexpr(!UseLazyCircularDiscovery){
               toPropagate[succ].merge(TSources{sources});
               // sources must not be modified, it is needed
               // for next successors -> a copy is used.
            } else
               toPropagate[succ];

            // update distances to support computing uniqueOrderingOut
            auto& maxDist = maxDistance[succ];
            const auto vDist = maxDistance[v];
            if (maxDist <= vDist){
               maxDist = vDist+1;

               if constexpr(UseLazyCircularDiscovery){
                  anyCircular |= (maxDist >= thresholdCirc);
               }
            }
         }
      }
   }

   if (anyCircular){
      uniqueOrderingOut = false;
   } else
   if (uniqueOrderingOut){
      std::set<TRank> already;
      for (auto& [v__, rank] : maxDistance)
         if (!already.insert(rank).second){
            uniqueOrderingOut = false;
            break;
         }
   }

   if (uniqueOrderingOut){
      // better to re-set labels as from 1 to maxDist
      std::map<TRank, TNode> inverse;
      std::for_each(labelsOut.cbegin(), labelsOut.cend(), [&inverse](auto labelAndRank){
         inverse[labelAndRank.second] = labelAndRank.first;
      });

      TRank idx{0U};
      std::for_each(inverse.cbegin(), inverse.cend(), [&labelsOut, &idx](auto rankAndLabel){
         labelsOut[rankAndLabel.second] = ++idx;
      });
   }

   return !anyCircular;
}


NAoc__MR::TResult day05Part2(std::shared_ptr<std::istream> inputStream)
{
   std::cout << "----- PART 2 -----" << std::endl;

   unsigned lineCount{0U};

   TResult sum{0U};

   TRelations withPredecessors;
   TRelations withSuccessors;
   TLabels globalLabel;

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

         withPredecessors[v2].insert(v1);
         withSuccessors[v1].insert(v2);
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

            bool uniqueOrdering;
            anyCircular = !buildGlobalLabelling( globalLabel, uniqueOrdering,
                                                 withPredecessors, withSuccessors, std::set<TNode>{} );

            if (anyCircular){
               std::cout << "WARNING: circular references for no number with only successors\n";
               std::cout << "         Will proceed rebuilding local labelling for each line-update\n";
            } else
            if (uniqueOrdering){
               std::cout << "WOW: unique ordering for the global labelling!\n";
            }
         }

         // new update found
         std::vector<TNode> update;
         update.push_back(v1);
         if (!oneNumberOnly)
            do{
               if ((ch != ',')){
                  throw std::invalid_argument(msgLine + "unexpected separator: " + std::string(1,ch)); // LINE CHECK
               }

               unsigned v2;
               getValue(lineStream,v2);

               update.push_back(v2);
            } while(lineStream >> ch);

         if ( update.size()%2 == 0 ){
            throw std::invalid_argument(msgLine + "update list with even number of values"); // LINE CHECK
         }

         constexpr auto AlwaysReorder = true;
         // preferred the solution where reordering always is in place

         decltype(update) originalUpdate{};

         bool goodUpdate;
         if constexpr(!AlwaysReorder){
            goodUpdate = true;
            for(auto i = 0U; (i < update.size()-1U) && goodUpdate; ++i){
               try{
                  const auto& prevs = withPredecessors.at(update[i]);
                  for(auto j = i+1U; j < update.size(); ++j){
                     if (prevs.count(update[j])){
                        goodUpdate = false;
                        break;
                     }
                  }
               }catch(std::out_of_range){
                  // from .at() -> update[i] has no precedence rule.
               }
            }
         } else{
            originalUpdate = update;
            goodUpdate = false;
         }

         if (!goodUpdate){
            // reorder

            /* (_1_): this worked,... fortunately
            std::sort(update.begin(),update.end(), [&withPredecessors](const auto v1, const auto v2){
               const auto it2 = withPredecessors.find(v2);
               return (it2 != withPredecessors.cend()) && (it2->second.count(v1) > 0); // v1|v2 is in the rule list (unless v1 is zero)
               //return ruleSet.count(std::make_pair(v1,v2)) > 0;
            }); */

            constexpr auto ForceUniqueResult = true;
            bool uniqueOrdering{};

            std::set<TNode> updateSet;
            for(auto elem : update) updateSet.insert(elem); //don't care about repeatitions.

            auto [filteredPredecessors, filteredSuccessors] =
               filterRelations( withPredecessors, withSuccessors, updateSet );

            // if (anyCircular)
            // removed condition in order to get warnings for the cases of
            // non-unique possible ordering.
            {
               // rebuild globalLabel from scratch, focusing only on the update-line

               if (!buildGlobalLabelling( globalLabel, uniqueOrdering,
                                          filteredPredecessors, filteredSuccessors, updateSet )){
                  throw std::invalid_argument(msgLine + "impossible to decide: circular reference in the update: " + line); // LINE CHECK
               }
               
               if (!uniqueOrdering){ // LINE CHECK
                  std::cout << "WARNING: non-unique ordering for: " << line << std::endl;
                  // still possible to recovery if at the interesting position there is a unique choice.
               }
            }

            TLabels originalRank;
            if constexpr(!ForceUniqueResult){ // else: not used
               for(TNode idx = 0U; idx < update.size(); ++idx){
                  originalRank[update[idx]] = idx+1;
               }
            }

            std::stable_sort(update.begin(),update.end(),
               [&globalLabel = std::as_const(globalLabel), &originalRank = std::as_const(originalRank)]
               (const auto v1, const auto v2){
               const auto it1 = globalLabel.find(v1);
               const auto it2 = globalLabel.find(v2);
               
               if constexpr(!ForceUniqueResult){ //else: useless, as later we throw an exception
                  if (bool no1 = (it1 == globalLabel.cend()), no2 = (it2 == globalLabel.cend());
                     no1 || no2){
                     // keys with no predecessors nor successors, may be considered to stay first,
                     // but in order to preserve the stability, let's use the original ordering.
                     return originalRank.at(v1) < originalRank.at(v2);
                  }
               }
               return it1->second < it2->second;
            });           
            
            bool increaseSum;
            if constexpr(AlwaysReorder){
               goodUpdate = (update == originalUpdate);
               increaseSum = !goodUpdate;
            } else{
               increaseSum = true;
            }

            if constexpr(ForceUniqueResult){
               if (!uniqueOrdering){ // LINE CHECK
                  // check for the unicity of the choice at the position update.size()/2U.
                  // If the selected node is connected to all other nodes, then it is ok.
                  if (countConnected(update[update.size()/2U], filteredPredecessors, filteredSuccessors) != update.size()){ // => is less
                     throw std::invalid_argument(msgLine + "non-unique choice for the target element in the update: " + line); // LINE CHECK
                  }
               }
            }

            if (increaseSum){
               auto add = update[update.size()/2U];
               if (sum > std::numeric_limits<TResult>::max() - add){
                  throw std::runtime_error("Overflow: TResult size to be increased");
               }
               sum += add;
            }
         }

         // printIterable(update,",");

      } else{
         throw std::runtime_error(msgLine + "separator: " + std::string(1,ch));
      }
   }

   if (withPredecessors.empty()){
      std::cout << "WARNING: no predecessors rules";
   }

   std::cout << "Number of lines: " << lineCount << std::endl;
   std::cout << "\nResult P2: " << sum << std::endl;
   std::cout << std::endl;

   return sum;

   // ricordati di rimuovere unsigned sum se non usato
}
