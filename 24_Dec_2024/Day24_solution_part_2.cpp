
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_2)

using namespace NAoc__MR;


NAoc__MR::TResult day24Part2(std::shared_ptr<std::istream> inputStream)
{
   std::cout << "----- PART 2 -----" << std::endl;

   unsigned lineCount{0U};

   std::vector<std::string> strGates;


   std::unordered_map<std::string, std::pair<bool,bool> > wireValues; //ready-value
   using It = decltype(wireValues)::iterator;
   using SetIt = std::unordered_set<It>;

   enum class BitsOp {OR, AND, XOR};

   std::unordered_map<It, std::pair<BitsOp, std::pair<It,It>> > gatesConn;

   std::unordered_map<It, SetIt> gatesConnFw;

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

   for(const auto& strGate : strGates){
      auto forGate = split(strGate, R"__(->)__", true);

      wireValues[forGate[1]] = std::make_pair(false, false); // not ready
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
      auto itWin2 = wireValues.find(gateElems[2]);

      gatesConn[itOut] =
         std::make_pair(
            op,
            std::make_pair(itIn1, itWin2)
         );

      gatesConnFw[itIn1].insert(itOut);
      gatesConnFw[itWin2].insert(itOut);
   }


   SetIt initTrigger; // both 'x' and 'y'
   SetIt allZ;
   for(auto itW = wireValues.begin(); itW != wireValues.end(); ++itW){
      switch (itW->first[0]){
      case 'x' : case 'y' : initTrigger.insert(itW); break;
      case 'z' : allZ.insert(itW);
      }
   }

   auto xBitNumer = initTrigger.size() / 2;
   std::vector<It> xIts, yIts, zIts;
   xIts.reserve(xBitNumer);
   yIts.reserve(xBitNumer);
   zIts.reserve(allZ.size()); // == xBitNumer+1U in the real input

   {
      auto vName = "x"s;
      auto refIts = std::ref(xIts);
      for(int k = 0; k < 2; ++k){
         for(int b = 0; b < xBitNumer; ++b){
            refIts.get().push_back(wireValues.find(vName + std::to_string(b/10) + std::to_string(b%10)));
         }

         vName = "y"s;
         refIts = std::ref(yIts);
      }

      for(int b = 0; b < allZ.size(); ++b){ // == xBitNumer+1U in the real input
         zIts.push_back(wireValues.find("z"s + std::to_string(b/10) + std::to_string(b%10)));
      }
   }
   
   const auto resetXYwires = [ &wireValues, &initTrigger = std::as_const(initTrigger),
                               &xIts = std::as_const(xIts),
                               &yIts = std::as_const(yIts),
                               &zIts = std::as_const(zIts) ] (TResult x, TResult y){
      for(auto& wire : wireValues){
         wire.second.first = false;

      }
      for(auto itXY : initTrigger){
         itXY->second.first = true;
      }

      const auto xBitNumer = xIts.size();

      auto v = x;
      auto refIts = std::cref(xIts);
      for(int k = 0; k < 2; ++k){
         std::size_t bitIdx = 0;
         for(int b = 0; b < xBitNumer; ++b){
            bool bitV = v & 1;

            refIts.get()[bitIdx]->second.second = bitV;

            v >>= 1;
            ++bitIdx;
         }

         v = y;
         refIts = std::cref(yIts);
      }
   };

   const auto findResult = [ &wireValues = std::as_const(wireValues),
                             &initTrigger = std::as_const(initTrigger),
                             &allZ = std::as_const(allZ),
                             &gatesConn = std::as_const(gatesConn),
                             &gatesConnFw = std::as_const(gatesConnFw),
                             &zIts = std::as_const(zIts) ] (){
      SetIt readyZ;

      SetIt trigger = initTrigger;

      std::size_t antiLoopN = 0;

      // newTrigger empty with readyZ not yet full, because of gates to be swapped.
      while ( // (readyZ.size() < allZ.size()) && -> commented, as might need multiple propagations
              !trigger.empty() ){
         if (wireValues.size() <= ++antiLoopN){
            return std::pair(TResult{0U},false);
         }

         SetIt newTrigger;

         for(auto itW : trigger){
            if (auto itFw = gatesConnFw.find(itW); itFw != gatesConnFw.end()){
               for(auto itOut : itFw->second){
                  auto gateInfo = gatesConn.find(itOut)->second;

                  auto itIn1 = gateInfo.second.first;
                  auto itIn2 = gateInfo.second.second;

                  if (itIn1->second.first && itIn2->second.first){ // both ready
                     bool bit1 = itIn1->second.second;
                     bool bit2 = itIn2->second.second;
                     bool v;
                     switch(gateInfo.first){
                     case BitsOp::XOR: v = (bit1 ^ bit2); break;
                     case BitsOp::AND: v = (bit1 && bit2); break;
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
         }

         trigger = std::move(newTrigger);
      }

      TResult res = 0U;
      for(auto idx = zIts.size(); idx > 0U; ){
         --idx;
         (res <<= 1) |= zIts[idx]->second.second;
      }

      return std::pair(res, readyZ.size() == allZ.size());
   };

   const auto test = [&resetXYwires, &findResult](TResult x, TResult y){
      resetXYwires(x,y);
      return findResult();
   };

   //auto zDebugEx1 = test(13,31);

   const auto fullTest = [&test, xBitNumer](std::size_t iBit){
      TResult x = 0;
      TResult y = 1;

      TResult maskSingle = 1U;
      TResult maskOnes = 1U;
      std::unordered_set<std::size_t> bit_problems;
      for(int b = 0; b <= iBit; ++b){
         if (b < xBitNumer){
            auto [z1, okConnections] = test(maskSingle, 0U);
            auto [z2, ok_] = test(0U, maskSingle);

            if ( (!okConnections) ||
                 (z1 != maskSingle) || (z2  != maskSingle) ){
               return std::pair(false,okConnections);
            }
         }

         if (b > 0){
            // another test is for carries
            auto [z3, okConnections] = test(maskOnes, 1U);
            auto [z4, ok___] = test(1U, maskOnes);

            if ( (!okConnections) ||
                 (z3 != maskSingle) || (z4 != maskSingle) ){
               return std::pair(false,okConnections);
            }

            (maskOnes <<= 1) |= 1;
         }

         maskSingle <<= 1;
      }

      return std::pair(true,true);
   };


   const auto extremeTest = [&test, xBitNumer](std::size_t iBitStart, std::size_t iBitEnd){
      if (iBitEnd > xBitNumer) iBitEnd = xBitNumer;

      const auto ub = TResult{1ul} << iBitEnd;
      const auto lb = 1ul << iBitStart;

      for(TResult x = 0; x < ub; x+=lb){
         for(TResult y = x; y < ub; y+=lb){
            const auto s = x+y;

            auto [z1, okConnections] = test(x, y);
            auto [z2, ok_] = test(y, x);

            if ( (z1 != s) || (z2 != s) || !okConnections ){
               return false;
            }
         }
      }

      return true;
   };

   const auto finalTest = [&extremeTest, xBitNumer](){
      for(std::size_t iFinalCheck = 0; iFinalCheck < xBitNumer; iFinalCheck += 2){
         if (!extremeTest(iFinalCheck, iFinalCheck+6)) return false;
      }
      return true;
   };

   //if (IsShortTest) return 0;

   if (finalTest()){
      std::cout << "Net already working! \n";
      return 0;
   }

   using GraphGates = Graph<It, GraphKind::Direct>;
   GraphGates graphGates;
   using NodeSetRef = typename GraphGates::NodeSetRef;

   for(const auto& [out, pair] : gatesConn){
      graphGates.addEdge(pair.second.first, out);
      graphGates.addEdge(pair.second.second, out);
   }

   /* // Try to look for loops! -> no one was there
   for(auto node : graphGates){
      auto [count_, noLoop] = graphGates.countPathsFrom(node);
      if (!noLoop){
         callDebug();
      }
   } */

   /*
   NodeSetRef candidates;

   // Look incrementaly by increasing bits: reachable back from Zj should
   // be all Xi/Yi with i <= j, and not for j > i.
   for(std::size_t i = 0; i <= xBitNumer; ++i){
      auto zIt = zIts[i];
      auto [sources, noLoop_] = graphGates.getReachablesTo(zIt);
      sources.insert(graphGates.refNode(zIt));

      for(std::size_t j = i+1; j < xBitNumer; ++j){
         auto xIt = xIts[j];
         auto [destX, noLoop_] = graphGates.getReachablesFrom(xIt);
         destX.insert(graphGates.refNode(xIt));

         // among all shared nodes, there are candidates to be swapped
         reduceContainer(destX, sources, true);

         // the same on Y
         auto yIt = yIts[j];
         auto [destY, noLoop__] = graphGates.getReachablesFrom(yIt);
         destY.insert(graphGates.refNode(yIt));

         // among all shared nodes, there are candidates to be swapped
         reduceContainer(destY, sources, true);

         candidates.merge(destX);
         candidates.merge(destY);
      }
   }
   
   found out no candidate from this code

   -> therefore, look only for couples that both are involved
      in the same couple Xi->Zk or Yj->Zk

      that is, for each node involved in any path Xi/Yi->Zk,
      and not in any other Xi->Zl with l < k, it may
      be swapped only with similar nodes.
   */

   std::vector<NodeSetRef> candidatesBack;
   candidatesBack.reserve(xBitNumer+1U);
   {
      // NodeSetRef cumulativeCandidates;

      NodeSetRef nodesXY;
      transformContainer(nodesXY, initTrigger, [&graphGates = std::as_const(graphGates)](auto it)
                                               {return graphGates.refNode(it);} );

      for(std::size_t i = 0; i <= xBitNumer; ++i){
         auto zIt = zIts[i];
         auto [sources, noLoop_] = graphGates.getReachablesTo(zIt);
         sources.insert(graphGates.refNode(zIt));

         // exclude all the ones in previous-indexes candidates.
         // reduceContainer(sources, cumulativeCandidates);

         // cumulativeCandidates.merge(NodeSetRef{sources});

         reduceContainer(sources, nodesXY); // avoid swapping x and y
         candidatesBack.push_back(std::move(sources));
      }
   }

   std::vector<NodeSetRef> candidatesFront(xBitNumer+1U);
   {
      NodeSetRef nodesXY;
      transformContainer(nodesXY, initTrigger, [&graphGates = std::as_const(graphGates)](auto it)
                                               {return graphGates.refNode(it);} );

      NodeSetRef residualCandidates = graphGates.getNodesRef();
      reduceContainer(residualCandidates, nodesXY); // avoid swapping x and y

      for(std::size_t i = xBitNumer; i > 0; ){
         --i;

         candidatesFront[i] = residualCandidates;

         auto xIt = xIts[i];
         auto [dest, noLoop_] = graphGates.getReachablesFrom(xIt);
         // dest.insert(graphGates.refNode(xIt)); avoid swapping x and y

         auto yIt = yIts[i];
         auto [dest2, noLoop__] = graphGates.getReachablesFrom(yIt);
         // dest2.insert(graphGates.refNode(yIt)); avoid swapping x and y

         dest.merge(dest2);

         reduceContainer(residualCandidates, dest);
      }

      if (!residualCandidates.empty()){
         throw std::runtime_error("Some loop, generalize the code");
      }

      candidatesFront[xBitNumer] = candidatesFront[xBitNumer-1U]; // there are no more bits in input.
   }

   bool found = false;
   SetIt gates4;

   std::vector<std::size_t> seedSeq;
   std::vector<It> cIts;
   std::vector<std::size_t> candSetIdx; // index in candidates where cIts[] belongs
   std::unordered_map<It, std::size_t> cItIdxs;
   seedSeq.reserve(gatesConn.size());
   cIts.reserve(gatesConn.size());
   candSetIdx.reserve(gatesConn.size());
   {
      std::size_t i = 0;
      for(const auto& [cIt, second__] : gatesConn){
         seedSeq.push_back(i);
         cIts.push_back(cIt);
         cItIdxs[cIt] = i;
         candSetIdx.push_back(xBitNumer+2); // upper-bound, set here below.

         ++i;
      }

      i = 0;
      for (const auto& nodeSetIt : candidatesBack){
         for(auto nodeIt : nodeSetIt){
            candSetIdx[cItIdxs[*nodeIt]] = i;
         }
         ++i;
      }
   }

   // look incrementally
   std::function<std::pair<SetIt,bool>(std::size_t)> f_findCouples;

   SetIt incrementalGates8;
   std::vector<std::size_t> debugMissingChecks(xBitNumer+1U, 0U);

   constexpr auto NumGatesToSearch = 8U;

   f_findCouples = [&](std::size_t iBitStart){
      {
         bool fullyOk = false;

         bool alreadySize = (incrementalGates8.size() >= NumGatesToSearch);
         if (iBitStart > xBitNumer){
            fullyOk = true;
         }

         if (alreadySize){
            fullyOk = fullTest(xBitNumer).first;
         }

         if (fullyOk){
            fullyOk = finalTest();

            return std::pair( SetIt{}, fullyOk );
         }
      }

      //auto debugN = candidatesBack[iBitStart].size() * candidatesBack[iBitStart].size();
      //debugMissingChecks[iBitStart] = debugN;

      if (fullTest(iBitStart).first){
         // first try not to change anything...
         auto [setRes, ok] = f_findCouples(iBitStart+1U);
         if (ok){
            return std::pair(std::move(setRes),true);
         }
      }
      //... then try switching couples.

      // check all couples in candidates[i]
      for(auto node1It : candidatesBack[iBitStart]){
         auto cIt1 = *node1It;
         if ( // initTrigger.contains(cIt1) || // don't swap x and y -> avoided already during construction
              incrementalGates8.contains(cIt1) ){ // avoid repeat some already swapped
            //debugMissingChecks[iBitStart] -= candidatesBack[iBitStart].size();
            continue;
         }

         constexpr bool useFronAsSecondCandidates = true;

         auto candidates2 = std::cref(candidatesBack);
         if constexpr(useFronAsSecondCandidates){
            candidates2 = std::cref(candidatesFront);
         }

         for(auto node2It : candidates2.get()[iBitStart]){
            //debugMissingChecks[iBitStart]--;

            auto cIt2 = *node2It;
            if ( // initTrigger.contains(cIt2) || // don't swap x and y -> avoided already during construction
                 incrementalGates8.contains(cIt2) ){ // avoid repeat some already swapped
               continue;
            }

            bool mix12 = true;
            if constexpr(useFronAsSecondCandidates){
               mix12 = candidatesBack[iBitStart].contains(node2It) &&
                       candidatesFront[iBitStart].contains(node1It);
            }

            if (mix12 && (cItIdxs[cIt1] >= cItIdxs[cIt2])){
               continue; // avoid uselessly double the number of tries
            }

            auto& gate1 = gatesConn[cIt1];
            auto& gate2 = gatesConn[cIt2];

            auto itIn11 = gate1.second.first;
            auto& fw11 = gatesConnFw[itIn11];
            auto itIn12 = gate1.second.second;
            auto& fw12 = gatesConnFw[itIn12];
            auto itIn21 = gate2.second.first;
            auto& fw21 = gatesConnFw[itIn21];
            auto itIn22 = gate2.second.second;
            auto& fw22 = gatesConnFw[itIn22];

            std::swap(gate1, gate2);
            fw11.erase(cIt1);
            fw12.erase(cIt1);
            fw21.erase(cIt2);
            fw22.erase(cIt2);

            fw11.insert(cIt2);
            fw12.insert(cIt2);
            fw21.insert(cIt1);
            fw22.insert(cIt1);

            if (fullTest(iBitStart).first){
               incrementalGates8.insert(cIt1);
               incrementalGates8.insert(cIt2);

               auto [setRes, ok] = f_findCouples(iBitStart+1U);

               if (ok){
                  setRes.insert(cIt1);
                  setRes.insert(cIt2);
                  return std::pair(std::move(setRes),true);
               } // else: swap-back

               incrementalGates8.erase(cIt1);
               incrementalGates8.erase(cIt2);
            }

            // restores
            std::swap(gate1, gate2);
            fw11.erase(cIt2);
            fw12.erase(cIt2);
            fw21.erase(cIt1);
            fw22.erase(cIt1);

            fw11.insert(cIt1);
            fw12.insert(cIt1);
            fw21.insert(cIt2);
            fw22.insert(cIt2);
         }
      }

      return std::pair(SetIt{},false);
   };

   auto [gates8, okRecursion] = f_findCouples(0);

   std::vector<std::string> names;
   if (okRecursion){
      transformContainer(names, gates8, [](auto it){return it->first;});
   }
   std::sort(names.begin(), names.end());

   std::cout << "\nNumber of lines: " << lineCount << std::endl;
   std::cout << "\nResult P2: " << getString(names,",") << std::endl;
   std::cout << std::endl;

   return names.size();
}
