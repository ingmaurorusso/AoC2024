
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_1)

using namespace NAoc__MR;

using TNode = unsigned;

NAoc__MR::TResult day05Part1(std::shared_ptr<std::istream> inputStream)
{
   std::cout << "----- PART 1 -----" << std::endl;

   unsigned lineCount{0U};

   TResult sum{0U};

   std::unordered_map<TNode, std::unordered_set<TNode> > predecessors;

   bool readingRules = true;

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

      if ( std::find_if(std::begin(line), std::end(line), [](char ch){return !(std::isdigit(ch) || (ch == '|') || (ch == ','));})
           != std::end(line) ){
         throw std::invalid_argument(msgLine + "unexpected non-digits or kind of separators");
      }
      // ... other checks later along the code
      //  --- END LINE CHECKS ---

      TNode v1{};

      getValue(lineStream,v1);

      char ch{};
      bool oneNumberOnly = !(lineStream >> ch);
      if ((ch == '|') && !oneNumberOnly){
         if (!readingRules){
            throw std::invalid_argument(msgLine + "another rule after the empty line"); // LINE CHECK
         }
         TNode v2;
         getValue(lineStream,v2);

         if (v1 == v2){
            throw std::invalid_argument(msgLine + "rule with two equivalent values"); // LINE CHECK
         }

         predecessors[v2].insert(v1);
      } else
      if (oneNumberOnly || (ch == ',')){
         if (readingRules){
            if constexpr(ForceExpectEmptyLine){
               throw std::invalid_argument(msgLine + "an update before the empty line"); // LINE CHECK
            } else{
               readingRules = false; // from now on, only update
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

               TNode v2;
               getValue(lineStream,v2);

               update.push_back(v2);
            } while(lineStream >> ch);
         
         if ( update.size()%2 == 0 ){
            throw std::invalid_argument(msgLine + "update list with even number of values"); // LINE CHECK
         }

         bool goodUpdate = true;
         for(auto i = 0U; (i < update.size()-1U) && goodUpdate; ++i){
            try{
               const auto& prevs = predecessors.at(update[i]);
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

         if (goodUpdate){
            auto add = update[update.size()/2U];
            if (sum > std::numeric_limits<TResult>::max() - add){
               throw std::runtime_error("Overflow: TResult size to be increased");
            }
            sum += add;
         }
      } else{
         throw std::runtime_error(msgLine + "separator: " + std::string(1,ch));
      }
   }

   if (predecessors.empty()){
      std::cout << "WARNING: no predecessors rules";
   }

   std::cout << "Number of lines: " << lineCount << std::endl;
   std::cout << "\nResult P1: " << sum << std::endl;
   std::cout << std::endl;

   return sum;
}