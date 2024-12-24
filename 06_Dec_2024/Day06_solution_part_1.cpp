
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_1)

using namespace NAoc__MR;

constexpr char Obstacle = '#';
constexpr char FreePos = '.';
constexpr std::string Expected{Arrows4 + std::string(1,Obstacle) + std::string(1,FreePos)};

NAoc__MR::TResult day06Part1(std::shared_ptr<std::istream> inputStream)
{
   std::cout << "----- PART 1 -----" << std::endl;

   unsigned lineCount{0U};

   std::vector<std::string> lines;

   Point guardPos;
   bool foundPos = false;

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
      if (!lines.empty()){
         if (line.size() != lines[0].size()){
            throw std::invalid_argument(msgLine + "non-uniform lines length");
         }
      }

      if (auto p = line.find_first_not_of(Expected); p != std::string::npos){
         throw std::invalid_argument(msgLine + "unexpected char: " + std::string(1,line[p]));
      }

      if (auto p1 = line.find_first_of(Arrows4); p1 != std::string::npos){
         auto p2 = line.find_last_of(Arrows4);

         if (foundPos || (p1 != p2)){
            throw std::invalid_argument(msgLine + "multiple starting positions");
         }

         guardPos = {p1, lineCount-1U};
         foundPos = true;
      }
      //  --- END LINE CHECKS ---

      lines.emplace_back(std::move(line));
   }

   if (!foundPos){
      throw std::invalid_argument("No initial position");
   }

   std::set<PointUnsign> passedPositions{};
   passedPositions.insert(guardPos);

   Field2D field{std::move(lines)};

   Direction4 guardDir = dir4FromArrow(field[guardPos]).first;

   bool ended = false;
   while(!ended){
      auto oldPos = guardPos;
      auto oldDir = guardDir;

      ended = !field.movePoint(guardPos, guardDir);

      if (ended) break;

      if (field[guardPos] == Obstacle){
         guardPos = oldPos;
         guardDir = oldDir;
         dir4rotate(guardDir,false);
         // false=anti-clockwise, because Down here represents
         // reducing 'y', that is 'up' in relaity, and the puzzle
         // wants clockwise in reality.
         continue;
      }

      passedPositions.insert(guardPos);
   }
   TResult count = passedPositions.size();

   std::cout << "Number of lines: " << lineCount << std::endl;
   std::cout << "\nResult P1: " << count << std::endl;
   std::cout << std::endl;

   return count;
}