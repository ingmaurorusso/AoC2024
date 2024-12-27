
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_1)

using namespace NAoc__MR;


NAoc__MR::TResult day14Part1(std::shared_ptr<std::istream> inputStream)
{
   std::cout << "----- PART 1 -----" << std::endl;

   unsigned lineCount{0U};

   std::vector<std::string> lines;

   std::unordered_map<TResult, std::pair<PointSign,PointSign>> posAndSpeedByRobot;

   CoordSign X = IsShortTest ? 11 : 101;
   CoordSign Y = IsShortTest ? 7 : 103;

   constexpr CoordSign times = 100;

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
      std::smatch matchInfo{};
      if (!std::regex_search(line.cbegin(), line.cend(), matchInfo, std::regex{R"__((.*?p=.*?)(-?[\d]+),(-?[\d]+)(.*?v=.*?)(-?[\d]+),(-?[\d]+).*)__"})){
         throw std::invalid_argument(msgLine + "unmatched syntax");
      }
      //  --- END LINE CHECKS ---

      auto values = getAllValues<CoordSign>(line);
      if (values.size() != 4U){
         throw std::invalid_argument(msgLine + "not exactly 4 numbers in the line");
      }

      CoordSign px = values[0];
      CoordSign py = values[1];
      if ((px < 0) || (py < 0)){
         throw std::runtime_error("negative initial position");
      }

      CoordSign vx = values[2];
      while (vx < 0) vx += X;
      CoordSign vy = values[3];
      while (vy < 0) vy += Y;

      posAndSpeedByRobot[lineCount] = {{px,py},{vx,vy}};


      lines.emplace_back(std::move(line)); //TODO: remove when not needed
   }

   for(auto& [k_, p_v] : posAndSpeedByRobot){
      auto& [p,v] = p_v;

      checkProdResult(std::max(v.x,v.y), times);
      checkProdResult(std::max(p.x,p.y), std::max(v.x,v.y)*times);
      p += (v*times);

      p.x %= X;
      p.y %= Y;
   }


   const auto middleX = X/2; // indexes are from 0
   const auto middleY = Y/2;

   std::array<TResult,4> qs = {0,0,0,0};

   for(auto& [k_, p_v] : posAndSpeedByRobot){
      auto& [p,v_] = p_v;

      if ((p.x != middleX) && (p.y != middleY)){
         unsigned idxQ = 0;
         if (p.y > middleY) idxQ += 2;
         if (p.x > middleX) idxQ++;

         qs[idxQ]++;
      }
   }

   TResult res = std::accumulate(qs.begin(), qs.end(), 1, [](auto acc, auto q){return acc*q;});

   std::cout << "Number of lines: " << lineCount << std::endl;
   std::cout << "\nResult P1: " << res << std::endl;
   std::cout << std::endl;

   return res;
}