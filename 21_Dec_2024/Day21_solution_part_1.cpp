
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_1)

using namespace NAoc__MR;


NAoc__MR::TResult day21Part1(std::shared_ptr<std::istream> inputStream)
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
                                      [](char ch){return std::isdigit(ch) || (ch == 'A');} ) ; it != line.cend() ){
         throw std::invalid_argument(msgLine + "unexpected char: " + std::string(1,*it));
      }
      if (const auto occs = getSubStrPositions(line,"A"); (occs.size() != 1U) || (occs[0] != line.size()-1U)){
         throw std::invalid_argument(msgLine + "not just a single occurrence of 'A'");
      }
      if (getAllValues<TResult>(line).size() != 1U){
         throw std::invalid_argument(msgLine + "not just a single number in line");
      }
      //  --- END LINE CHECKS ---

      lines.push_back(std::move(line));
   }

   std::vector<std::string> linesNumKeyPad{"789","456","123"," 0A"};
   Field2D numKeypad(linesNumKeyPad);
   PointUnsign pAnum{3,2};

   std::vector<std::string> linesDirKeyPad{" ^A","<v>"};
   Field2D dirKeypad(linesDirKeyPad);
   PointUnsign pAdir{0,2};

   std::vector<PointUnsign> psOnNum(11, PointUnsign{});
   //std::vector<std::vector<CoordUnsign>> distNum(11, std::vector<CoordUnsign>(11,0) );
   for(int i = 0; i <= 10; ++i){
      auto [p1, found_] = numKeypad.find_first( (i == 10)? 'A' : ('0'+i));
      psOnNum[i] = p1;

      for(int j = i+1; j <= 10; ++j){
         auto [p2, found_] = numKeypad.find_first( (i == 10)? 'A' : ('0'+i));

         //distNum[i][j] = distNum[j][i] = manDistance(p1,p2);
      }
   }

   std::vector<PointUnsign> psOnDir(FourDir4+1U, PointUnsign{});
   //std::vector<std::vector<CoordUnsign>> distDir(FourDir4+1, std::vector<CoordUnsign>(FourDir4+1,0) );
   for(Dir4Under d1 = 0; d1 < FourDir4+1; ++d1){
      auto [p1, found_] = dirKeypad.find_first( (d1 == FourDir4)? 'A' : dir4ToArrow(Direction4{d1}).first );
      psOnDir[d1] = p1;
      for(Dir4Under d2 = d1+1; d2 < FourDir4+1; ++d2){
         auto [p2, found_] = dirKeypad.find_first( (d2 == FourDir4)? 'A' : Arrows4[d2]);

         //distDir[d1][d2] = distDir[d1][d2] = manDistance(p1,p2);
      }
   }

   static constexpr auto arrowsToMove = [](const PointUnsign p1, const PointUnsign p2, bool xFirst, const PointUnsign pAvoid){
      std::string seqArrows;

      for(int twice = 0; twice < 2; ++twice){
         bool bad = (p2 == pAvoid);
         auto pMove = p1;

         for(int k = 0; k < 2; ++k){
            auto isX = ((k == 0) == xFirst);
            auto iCoord = isX ? 0 : 1;

            while (pMove[iCoord] < p2[iCoord]){
               if (pMove == pAvoid){
                  bad = true;
               }
               seqArrows += std::string(1, isX ? '>' : 'v'); // TODO: use field ?
               pMove[iCoord]++;
            }
            while (pMove[iCoord] > p2[iCoord]){
               if (pMove == pAvoid){
                  bad = true;
               }
               seqArrows += std::string(1, isX ? '<' : '^'); // TODO: use field ?
               pMove[iCoord]--;
            }
         }

         if (bad){
            seqArrows.clear();
            xFirst = !xFirst;
         } else break;
      }

      return seqArrows;
   };

   const auto seqFromTo_OnNum = [pAnum, &psOnNum = std::as_const(psOnNum)](int i1, int i2, bool xFirst){
      auto p1 = psOnNum[i1];
      auto p2 = psOnNum[i2];
      static const PointUnsign pAvoid{0,3};
      return arrowsToMove(p1,p2,xFirst,pAvoid);
   };

   const auto seqFromTo_OnDir = [pAnum, &psOnDir = std::as_const(psOnDir)](int i1, int i2, bool xFirst){
      auto p1 = psOnDir[i1];
      auto p2 = psOnDir[i2];
      static const PointUnsign pAvoid{0,0};
      return arrowsToMove(p1,p2,xFirst,pAvoid);
   };

   using BestSeqsForDir = std::vector< std::vector< std::string > >;

   constexpr auto NumRobotDirPads = 2U;
   std::vector< BestSeqsForDir > bestSeqsByLevel(NumRobotDirPads, BestSeqsForDir{FourDir4+1, std::vector(FourDir4+1,""s)});

   if (NumRobotDirPads > 0U){
      auto& bestSeqsForDir = bestSeqsByLevel[NumRobotDirPads-1U];

      for(int i = 0; i <= FourDir4; ++i){
         for(int j = 0; j <= FourDir4; ++j){
            auto seq1 = seqFromTo_OnDir(i,j,true) + "A";
            auto seq2 = seqFromTo_OnDir(i,j,false) + "A";

            bestSeqsForDir[i][j] = std::move( (seq1.size() <= seq2.size())? seq1 : seq2 );
         }
      }
   }

   const auto generateBestSeqFromSequence = [seqFromTo_OnDir](const BestSeqsForDir& bestSeqs, const std::string& seqInput ){
      std::string result;

      auto k1 = FourDir4; // for 'A'
      for(auto ch : seqInput){
         const auto k2 = (ch == 'A')? FourDir4 : Dir4Under(dir4FromArrow(ch).first);

         result += bestSeqs[k1][k2];
         k1 = k2;
      }

      return result;
   };

   const auto generateBestSeqsBasedOnPrevious =
              [ &bestSeqsByLevel, seqFromTo_OnDir, generateBestSeqFromSequence ](std::size_t levelIdx){

      auto& bestSeqsForDir = bestSeqsByLevel[levelIdx];

      for(int i = 0; i <= FourDir4; ++i){
         for(int j = 0; j <= FourDir4; ++j){
            const auto seqSource1 = seqFromTo_OnDir(i,j,true) + "A";
            const auto seqSource2 = seqFromTo_OnDir(i,j,false) + "A";
            // intermediate sequences do not matter, because consecutive equivalent
            // chars will become consecutive A's for previous keyboard in the chain.

            const std::string seq1 = (levelIdx+1U < bestSeqsByLevel.size()) // NumRobotDirPads
                                     ? generateBestSeqFromSequence(bestSeqsByLevel[levelIdx+1U],seqSource1)
                                     : std::move(seqSource1);
            const std::string seq2 = (levelIdx < bestSeqsByLevel.size()) // NumRobotDirPads
                                     ? generateBestSeqFromSequence(bestSeqsByLevel[levelIdx+1U],seqSource2)
                                     : std::move(seqSource2);

            bestSeqsForDir[i][j] = std::move( (seq1.size() <= seq2.size())? seq1 : seq2 );
         }
      }
   };

   for(int levelIdx = NumRobotDirPads-1U; levelIdx > 0;){
      generateBestSeqsBasedOnPrevious(--levelIdx);
      bestSeqsByLevel[levelIdx+1U].clear(); // not needed anymore
   }

   for(const auto& code : lines){
      auto p1 = pAnum;
      char k1 = 10; // for 'A'
      std::string seq;
      for(char ch : code){
         const auto k2 = (ch == 'A')? 10 : (ch - '0');

         const auto seqSource1 = seqFromTo_OnNum(k1,k2,true) + "A";
         const auto seqSource2 = seqFromTo_OnNum(k1,k2,false) + "A";
         // intermediate sequences do not matter, because consecutive equivalent
         // chars will become consecutive A's for previous keyboard in the chain.

         auto seq1 = (NumRobotDirPads > 0U)
                     ? generateBestSeqFromSequence(bestSeqsByLevel[0],seqSource1)
                     : std::move(seqSource1);
         auto seq2 = (NumRobotDirPads > 0U)
                     ? generateBestSeqFromSequence(bestSeqsByLevel[0],seqSource2)
                     : std::move(seqSource2);

         seq += std::move( (seq1.size() <= seq2.size())? seq1 : seq2 );
         k1 = k2;
      }

      auto codeVal = getAllValues<TResult>(code);

      checkProdResult(codeVal[0], seq.size());
      auto add = (codeVal[0] * seq.size());

      std::cout << code << " -> " << seq << std::endl;

      checkSumResult(sum, add);
      sum += add;
   }

   std::cout << "Number of lines: " << lineCount << std::endl;
   std::cout << "\nResult P1: " << sum << std::endl;
   std::cout << std::endl;

   return sum;
}