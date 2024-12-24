
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_2)

using namespace NAoc__MR;

NAoc__MR::TResult day08Part2(std::shared_ptr<std::istream> inputStream)
{
   std::cout << "----- PART 2 -----" << std::endl;

   unsigned lineCount{0U};

   using TValue = TResult;

   std::vector<std::string> lines;

   std::map<char, std::set<PointSign> > antennas;
   std::map<PointSign,unsigned> antiNodes;
   std::set<PointSign> antennaPositionsPrint;

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
      if ( const auto it = std::find_if( line.cbegin(), line.cend(),
                                         [](char ch){return !((ch == '.') || std::isalnum(ch));} );
           it != line.cend() ){
         throw std::invalid_argument(msgLine + "unexpected character: " + std::string(1,*it));
      }
      // equivalent-size, done in the constructor of Field2DUnlimited.
      //  --- END LINE CHECKS ---

      auto i = 0U;
      for(char ch : line){
         if (ch != '.'){
            PointSign p{i , lineCount-1};
            antennas[ch].insert(p);

            antennaPositionsPrint.insert(p);
         }
         ++i;
      }

      lines.emplace_back(std::move(line));
   }

   Field2DUnlimited fieldAntenna{lines};

   for(const auto& [ant, points] : antennas){
      for(auto it1 = points.cbegin(); it1 != points.cend(); ++it1){
         PointSign p1 = *it1;

         for(auto it2 = std::next(it1); it2 != points.cend(); ++it2){
            PointSign p2 = *it2;
            PointSign anti1 = p1, anti2 = p2;

            VectorSign delta1 = p1 - p2;

            // lukily not needed on the given input, but needed in general,
            // as explained by Remus Schnell (during the evening).
            delta1 /= std::gcd(std::abs(delta1.x), std::abs(delta1.y));

            VectorSign delta2 = PointSign{0,0} - delta1;

            while(fieldAntenna.inBoundary(anti1)){
               antiNodes[anti1]++;
               anti1 += delta1; //post-move, as antenna positions count
            }

            while(fieldAntenna.inBoundary(anti2)){
               antiNodes[anti2]++;
               anti2 += delta2; //post-move, as antenna positions count
            }
         }
      }
   }

   TResult count = antiNodes.size();

   if (NAoc__MR::IsShortTest){
      for(int j = 0; j < fieldAntenna.dimY(); ++j){
         for(int i = 0; i < fieldAntenna.dimX(); ++i){
            // '-' for single-antennas (do not produce antinodes)
            // otherwise antennas correspond to anti-nodes
            std::cout << ( antiNodes.contains({i,j})
                           ? (antennaPositionsPrint.count({i,j})? fieldAntenna[{i,j}] : '#')
                           : (antennaPositionsPrint.count({i,j})? '-' : fieldAntenna[{i,j}]) );
         }
         std::cout << std::endl;
      }
   }


   std::cout << "Number of lines: " << lineCount << std::endl;
   std::cout << "\nResult P2: " << count << std::endl;
   std::cout << std::endl;

   return count;
}
