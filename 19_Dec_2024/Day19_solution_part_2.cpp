
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_2)

using namespace NAoc__MR;

static
TResult checkStringSumIter( const std::string& line, std::map<std::string, std::size_t>& towelsMap,
                            std::size_t maxTowelLen, bool recomputeIndexes ){
   const auto Ntow = towelsMap.size();
   const auto N = line.size();

   if (recomputeIndexes){
      std::size_t i = 0;
      for( auto& [towel, idx] : towelsMap ){
         idx = i++; // to support (_1_)
      }
   }

   //TODO: ordered set towels and search by lower_bound
   Sequences generator(towelsMap, N); // TODO: scale by minimum length of towels.

   auto genRes = generator.beginLexicographic();

   auto& [seqItMap, changedIdx, ok] = genRes;

   std::vector<std::size_t> partialBefore(N, 0);

   std::unordered_map<std::size_t, TResult> countGoodAfter;

   while(ok){
      auto consume = partialBefore[changedIdx];
      std::size_t i = changedIdx;

      std::size_t changeIdx;

      for(; (i < N) && ok; ++i){
         auto sIt = seqItMap[i];
         if (line.substr(consume, sIt->first.size()) == sIt->first){
            consume += sIt->first.size();

            if (i < N){
               partialBefore[i+1] = consume;
            }

            auto [itGood, newPos] = countGoodAfter.insert({consume, !!(consume == N)});
            if ((consume == N) || !newPos){
               auto alreadyKnownAfter = itGood->second;

               if (alreadyKnownAfter > 0U){
                  for(std::size_t j = 0; j <= i; ++j){
                     if ( countGoodAfter[partialBefore[j]] >
                          std::numeric_limits<TResult>::max() - alreadyKnownAfter ){
                        throw std::runtime_error("TResult to be wider");
                     }
                     countGoodAfter[partialBefore[j]] += alreadyKnownAfter;
                  }
               }

               changeIdx = i;
               ok = false; // to force pushing changing soon.

               if (consume == N){
                  // check:
                  std::string sCheck;
                  if (!std::any_of( seqItMap.begin(), seqItMap.end(),
                                 [&sCheck,line](auto sIt)
                                 {
                                 sCheck += sIt->first;
                                 return (sCheck == line);
                                 }))
                     throw "error";
               }
            }
         } else{
            changeIdx = i;
            ok = false;
         }
      }

      // speed-up: look for a good string.
      std::size_t currentTried = generator.getIndexAt(changeIdx);
      std::size_t nextToTry = currentTried;
      auto sIt = seqItMap[changeIdx];

      if (ok){ //did not consume all, look for any longer
         // TODO: test this part, never hit.
         while((++nextToTry), towelsMap.cend() != ++sIt){
            if (sIt->first.size() == line.size()-partialBefore[N-1]){
               break;
            }
         }
      } else{
         consume = partialBefore[changeIdx];

         bool found = false;
         for(std::size_t l = 1; l <= std::min(line.size()-consume, maxTowelLen); ++l){
            auto itCandidate = towelsMap.find(line.substr(consume,l));
            if (itCandidate != towelsMap.end()){
               if (itCandidate->second > currentTried){
               // based on the fact that towels was sorted <-> same order as in the map.
                  if ((!found) || (itCandidate->second < nextToTry)){
                     nextToTry = itCandidate->second; //(_1_)
                     found = true;
                     break; // because shorter strings come first.
                  }
               }
            }
         }
         if (!found){
            nextToTry = Ntow;
         }
      }

      if (nextToTry == Ntow){
         bool impossibleBack = false;

         if (changeIdx == 0U){
            impossibleBack = true;
         } else --changeIdx;

         if (ok){ //did not consume all, look back for any longer in a faster way.
            // TODO: test this part, never hit.
            while ( line.size()-partialBefore[changeIdx] >
                     (N-changeIdx)*maxTowelLen ){
               if (changeIdx == 0U){
                  impossibleBack = true;
                  break;
               } else --changeIdx;
            }
         }

         if (impossibleBack) break;
      } else{
         generator.force(changeIdx, nextToTry-1U);
      }

      genRes = generator.nextLexicographic(changeIdx,!ok);
   }

   return countGoodAfter[0];
}

NAoc__MR::TResult day19Part2(std::shared_ptr<std::istream> inputStream)
{
   std::cout << "----- PART 2 -----" << std::endl;

   unsigned lineCount{0U};

   TResult count{0U};
   TResult bad = 0;

   std::vector<std::string> lines;
   std::vector<std::string> towels;
   std::map<std::string, std::size_t> towelsMap;
   std::size_t maxTowelLen;
   std::size_t Ntow;
   std::size_t nRemovedTowel = 0U;

   bool squencesBegun = false;

   while (true) {
      //  --- BEGIN LINE EXTRACTION ---
      auto [line, lineStream, flags] = lineExtraction<MaxLineLength>(inputStream);

      if (flags.endedLines) break;

      std::string msgLine
         = "Input at the line n. " + std::to_string(++lineCount) + " : ";

      if (flags.emptyLine){
         if (squencesBegun){
            std::cout << msgLine << "WARNING: empty line\n";
         }
         continue;
      }

      if (flags.tooLongLine){
         throw std::invalid_argument(msgLine + "too long Line");
      }
      //  --- END LINE EXTRACTION ---

      //  --- BEGIN LINE CHECKS ---

      if (lineCount == 1){
         towels = split(line, ",", true);

         maxTowelLen = std::accumulate( towels.begin(), towels.end(), std::size_t(0U),
                                        [](auto acc, auto towel){return std::max(acc,towel.size());} );

         Ntow = towels.size();

         std::sort(towels.begin(), towels.end());
         for(std::size_t i = 0; i < Ntow; ++i){
            if (towels[i].empty()){
               throw std::invalid_argument(msgLine + "some empty towel !");
            }
            towelsMap[towels[i]] = i;
         }
      } else{
         squencesBegun = true;

         std::unordered_map<std::size_t, TResult> alreadyAhead;

         std::function<std::size_t(std::string_view)> checkStringSumRec;
         checkStringSumRec =
            [&towels = std::as_const(towels), &alreadyAhead, &line = std::as_const(line), &checkStringSumRec]( std::string_view residualLine ){
               auto idxStarView = std::distance(&*(line.cbegin()), &*(residualLine.begin()));

               if (auto itAhead = alreadyAhead.find(idxStarView); itAhead != alreadyAhead.end()){
                  return itAhead->second;
               }

               auto& count = alreadyAhead.insert({idxStarView,!!residualLine.empty()}).first->second;
               for(const auto& towel : towels){
                  if (residualLine.find(towel) == 0U){
                     const auto add = checkStringSumRec(std::string_view(std::next(residualLine.begin(),towel.size()), residualLine.end()));
                     if (count > std::numeric_limits<TResult>::max() - add){
                        throw std::runtime_error("TResult to be wider");
                     }
                     count += add;
                  }
               }

               return count;
            };


         auto add = checkStringSumRec(std::string_view{line});

         /*if (checkStringSumIter(line, towelsMap, maxTowelLen, false) != add){
            throw "Fix code!";
         }*/

         if (add > 0U){
            count += add;
         } else{
            if (IsShortTest) std::cout << "lineCount = " << lineCount << " : " << line << std::endl;
            ++bad;
         }
      }

      lines.push_back(std::move(line));
   }

   std::cout << "Number of lines: " << lineCount << std::endl;
   std::cout << "\nResult P2: " << count  << std::endl;
   std::cout << "\nn. bad: " << bad << std::endl;
   std::cout << "\nn. removed useless towels: " << nRemovedTowel << std::endl;

   //TODO: check why without towel reduction we get 307 instead of 322 !

   std::cout << std::endl;

   return count;
}