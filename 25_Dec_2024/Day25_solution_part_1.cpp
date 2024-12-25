
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_1)

using namespace NAoc__MR;


NAoc__MR::TResult day25Part1(std::shared_ptr<std::istream> inputStream)
{
   std::cout << "----- PART 1 -----" << std::endl;

   unsigned lineCount{0U};

   TResult count{0U};

   std::vector<std::string> lines;

   std::vector<Field2D> keys, locks;
   bool ended = false;

   std::size_t X = 0U, Y = 0U;

   while (true) {
      //  --- BEGIN LINE EXTRACTION ---
      auto [line, lineStream, flags] = lineExtraction<MaxLineLength>(inputStream);

      if (flags.endedLines){
         ended = true;
         line.clear();
         flags.emptyLine = true;
         // postpone break, to get last element.
      }

      std::string msgLine
         = "Input at the line n. " + std::to_string(++lineCount) + " : ";

      if (flags.emptyLine){
         if (!lines.empty()){
            Field2D field{lines};

            if (X == 0U){
               X = field.dimX();
               Y = field.dimY();
            } else
            if (field.dimX() != X){
               throw std::invalid_argument(getString(" ", msgLine, "unexpected non-uniform line-length:", field.dimX(), "different from", X));
            } else
            if (field.dimY() != Y){
               throw std::invalid_argument(getString(" ", msgLine, "unexpected non-uniform element-extension:", field.dimY(), "different from", Y));
            }

            if (auto p = field[0].find(Field2D::FreeTile); p == std::string::npos){
               if (auto p = field[0].find(Field2D::FreeTile); p != std::string::npos){
                  throw std::invalid_argument(msgLine + "unexpected non-uniform line: " + line);
               }
               locks.push_back(std::move(field));
            } else{
               if (auto p = field[Y-1].find(Field2D::FreeTile); p != std::string::npos){
                  throw std::invalid_argument(msgLine + "unexpected non-uniform line: " + line);
               }
               keys.push_back(std::move(field));
            }

            lines.clear();

            if (ended) break;
         } else{
            std::cout << msgLine << "WARNING: empty line\n";
         }

         continue;
      }

      if (flags.tooLongLine){
         throw std::invalid_argument(msgLine + "too long Line");
      }
      //  --- END LINE EXTRACTION ---

      //  --- BEGIN LINE CHECKS ---
      if (auto it = std::find_if_not( line.cbegin(), line.cend(),
                                      [](char ch){return (ch == Field2D::WallTile) || (ch == Field2D::FreeTile);} ) ; it != line.cend() ){
         throw std::invalid_argument(msgLine + "unexpected char: " + std::string(1,*it));
      }
      //  --- END LINE CHECKS ---

      lines.push_back(std::move(line));
   }

   /* alternative to check dimension consistency
   { // check of uniform dimensions
      Field3D{locks};
      Field3D{keys};
   } */

   std::vector<std::vector<std::size_t>> cKeys(keys.size(), std::vector<std::size_t>{}),
                                         cLocks(locks.size(), std::vector<std::size_t>{} );

   std::size_t i = 0;
   for(const auto& lock : locks){
      for(std::size_t x = 0; x < lock.dimX(); ++x){
         std::size_t y = 1;
         for(; y < lock.dimY(); ++y){
            if (lock(x,y) == Field2D::FreeTile){
               break;
            }
         }
         cLocks[i].push_back(y-1);
      }
      ++i;
   }

   i = 0U;
   for(const auto& key : keys){
      for(std::size_t x = 0; x < key.dimX(); ++x){
         std::size_t y = key.dimY()-1U;
         bool found = false;
         for(; y > 0U; ){
            --y;
            if (key(x,y) == Field2D::FreeTile){
               found = true;
               break;
            }
         }
         cKeys[i].push_back(key.dimY()-1 - (found? y+1 : 0) );
      }
      ++i;
   }
   
   i = 0U;
   for(const auto& cLock : cLocks){
      for(const auto& cKey : cKeys){
         bool ok = true;
         for(auto x = 0U; x < X; ++x){
            if (cKey[x] + cLock[x] + 2 > Y){
               ok = false;
               break;
            }
         }

         if (ok){
            checkSumResult(count,TResult{1U});
            ++count;
         }
      }
   }

   std::cout << "\nNumber of lines: " << lineCount << std::endl;
   std::cout << "\nResult P1: " << count << std::endl;
   std::cout << std::endl;

   return count;
}
