
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_1)

using namespace NAoc__MR;

enum class BitsOp {OR, AND, XOR};

MR__ENUM_MAP_BEGIN_EXPAND(BitsOp)(OR)(AND)MR__ENUM_MAP_END_EXPAND(XOR);

inline auto bitOpToStr(BitsOp op){
   auto s = std::string{MR__ENUM_GET_NAMEq(BitsOp, op)};
   if (s.empty()){
      throw std::runtime_error("inconsistent gate-operation");
   }
   return s;
}

inline auto bitOpFromStr(const std::string& s){
   auto [e, ok] = MR__ENUM_GET_VALUE(BitsOp, s);
   if (ok) return e;
   
   throw std::runtime_error("inconsistent gate-operation");
}

NAoc__MR::TResult day24Part1(std::shared_ptr<std::istream> inputStream)
{
   std::cout << "----- PART 1 -----" << std::endl;

   unsigned lineCount{0U};

   std::vector<std::string> strGates;


   std::unordered_map<std::string, std::pair<bool,bool> > wireValues; //ready-value
   using It = decltype(wireValues)::iterator;

   std::unordered_map<It, std::pair<BitsOp, std::pair<It,It>> > gatesConn;

   std::unordered_map<It, std::unordered_set<It>> gatesConnFw;

   static const auto gateExtraction = [](const std::string& gate) -> std::vector<std::string> {
      const auto forGate = split(gate, R"__(->)__", true);
      if (forGate.size() != 2U) return std::vector<std::string>{};

      const auto gateElems = split(forGate[0], R"__( )__", true);
      if (gateElems.size() != 3U) return std::vector<std::string>{};

      static_cast<void>(bitOpFromStr(gateElems[1])); // check

      return std::vector{std::move(gateElems[0]), std::move(gateElems[1]), std::move(gateElems[2]), forGate[1]};
   };

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
      static_assert(__cplusplus > 202002L, "Use C++23 for std::string::contains, or disable the following line-check");

      if (auto it = std::find_if_not( line.cbegin(), line.cend(), // needed C++23 for string::contains
                                      [](char ch){return std::isalnum(ch) || (": ->"s.contains(ch));} ) ; it != line.cend() ){
         throw std::invalid_argument(msgLine + "unexpected char: " + std::string(1,*it));
      }
      //  --- END LINE CHECKS ---

      auto forWire = split(line, R"__(:)__", true);
      
      if (forWire.size() == 2){
         auto values = getAllValues<TResult>(forWire[1]);
         if ((values.size() != 1U) || (values[0] < 0U) || (values[0] > 1U)){
            throw std::invalid_argument(msgLine + "unexpected part of wire value: '" + forWire[1] + "'");
         }
         wireValues[forWire[0]] = std::make_pair(true, !!values[0]);
      } else{
         if (gateExtraction(line).empty()){
            throw std::invalid_argument(msgLine + "bad gate format");
         }

         strGates.push_back(std::move(line));
      }
   }

   std::unordered_set<It> trigger;
   for(auto itW = wireValues.begin(); itW != wireValues.end(); ++itW){
      trigger.insert(itW);
   }

   for(const auto& strGate : strGates){
      const auto forGate = split(strGate, R"__(->)__", true);

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
      auto gateAllElems = gateExtraction(strGate);

      BitsOp op = bitOpFromStr(gateAllElems[1]);

      auto itOut = wireValues.find(gateAllElems[3]);
      auto itIn1 = wireValues.find(gateAllElems[0]);
      auto itIn2 = wireValues.find(gateAllElems[2]);

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
   checkResultExtension<TResult>(allZ.size());
   std::size_t i = 0;
   for(auto itName = zNames.rbegin(); itName != zNames.rend(); ++itName){
      const auto& name = *itName;
      (res <<= 1) |= !!wireValues.find(name)->second.second;

      auto val = getAllValues<std::size_t>(std::string_view{std::next(name.begin()), name.end()});
      if ((val.size() != 1U) && (val[0] != zNames.size()-1U-i)){
         throw std::invalid_argument("Missing soem intermediate 'z' wires");
      }
   }

   std::cout << "\nNumber of lines: " << lineCount << std::endl;
   std::cout << "\nResult P1: " << res << std::endl;
   std::cout << std::endl;

   return res;
}
