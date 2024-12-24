
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_1)

using namespace NAoc__MR;


NAoc__MR::TResult day24Part1(std::shared_ptr<std::istream> inputStream)
{
   std::cout << "----- PART 1 -----" << std::endl;

   unsigned lineCount{0U};

   std::vector<std::string> strGates;


   std::unordered_map<std::string, std::pair<bool,bool> > wireValues; //ready-value
   using It = decltype(wireValues)::iterator;

   enum class BitsOp {OR, AND, XOR};

   std::unordered_map<It, std::pair<BitsOp, std::pair<It,It>> > gatesConn;

   std::unordered_map<It, std::unordered_set<It>> gatesConnFw;

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
                                      [](char ch){return true;} ) ; it != line.cend() ){
         throw std::invalid_argument(msgLine + "unexpected char: " + std::string(1,*it));
      }
      //  --- END LINE CHECKS ---

      auto forWire = split(line, R"__(:)__", true);
      

      if (forWire.size() == 2){
         wireValues[forWire[0]] = std::make_pair(true, !!getAllValues<TResult>(forWire[1])[0]);
      } else{
         strGates.push_back(line);
      }
   }

   std::unordered_set<It> trigger;
   for(auto itW = wireValues.begin(); itW != wireValues.end(); ++itW){
      trigger.insert(itW);
   }

   for(const auto& strGate : strGates){
      auto forGate = split(strGate, R"__(->)__", true);

      wireValues[forGate[1]] = std::make_pair(false, false); // not ready
   }

   std::unordered_set<It> readyZ;
   std::unordered_set<It> allZ;
   
   for(auto itW = wireValues.begin(); itW != wireValues.end(); ++itW){
      const auto& [wireName, pair] = *itW;

      if (wireName[0] == 'z'){
         allZ.insert(itW);

         if (pair.first){
            readyZ.insert(itW);
         }
      } 
   }

   for(const auto& strGate : strGates){
      auto forGate = split(strGate, R"__(->)__", true);

      auto gateElems = split(forGate[0], R"__( )__", true);

      BitsOp op;
      if (gateElems[1] == "XOR"){
         op = BitsOp::XOR;
      } else if (gateElems[1] == "AND"){
         op = BitsOp::AND;
      } else op = BitsOp::OR;

      auto itOut = wireValues.find(forGate[1]);
      auto itIn1 = wireValues.find(gateElems[0]);
      auto itIn2 = wireValues.find(gateElems[2]);

      gatesConn[itOut] =
         std::make_pair(
            op,
            std::make_pair(itIn1, itIn2)
         );

      gatesConnFw[itIn1].insert(itOut);
      gatesConnFw[itIn2].insert(itOut);
   }

   while (readyZ.size() < allZ.size()){
      std::unordered_set<It> newTrigger;

      for(auto itW : trigger){
         for(auto itOut : gatesConnFw[itW]){

            auto gateInfo = gatesConn[itOut];

            auto itIn1 = gateInfo.second.first;
            auto itIn2 = gateInfo.second.second;

            if (itIn1->second.first && itIn2->second.first){ // both ready
               bool bit1 = itIn1->second.second;
               bool bit2 = itIn2->second.second;
               bool v;
               switch(gateInfo.first){
               case BitsOp::XOR: v = bit1 ^ bit2; break;
               case BitsOp::AND: v = bit1 && bit2; break;
               //case BitsOp::OR:
               default: v = bit1 || bit2; break;
               }

               if ((!itOut->second.first) || (itOut->second.second != v)){ // anti-loop
                  //itOut->second = std::make_pair(true, v);
                  itOut->second.first = true;
                  itOut->second.second = v;
                  newTrigger.insert(itOut);

                  if (itOut->first[0] == 'z'){
                     readyZ.insert(itOut);
                  }
               }
            }
         }
      }

      trigger = std::move(newTrigger);
   }

   std::vector<std::string> zNames;
   transformContainer( zNames, allZ, [](const auto &itW){return std::string{itW->first};} );

   std::sort(zNames.begin(), zNames.end());

   TResult res = 0U;
   for(auto itName = zNames.rbegin(); itName != zNames.rend(); ++itName){
      (res <<= 1) |= !!wireValues.find(*itName)->second.second;
   }

   std::cout << "\nNumber of lines: " << lineCount << std::endl;
   std::cout << "\nResult P1: " << res << std::endl;
   std::cout << std::endl;

   return res;
}
