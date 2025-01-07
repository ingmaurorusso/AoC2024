
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_1)

using namespace NAoc__MR;

using Equation = std::array<double,3>; //TODO: VectorN
struct System{
   Equation eq1;
   Equation eq2;
};

// prize multipliers
constexpr Equation eqMin{3,1,0}; // last 0 does not matter.

NAoc__MR::TResult day13Part1(std::shared_ptr<std::istream> inputStream)
{
   std::cout << "----- PART 1 -----" << std::endl;

   unsigned lineCount{0U};

   TResult count{0U};

   std::vector<std::string> lines;

   std::list<System> systems;
   System currSys;

   constexpr double Eps = 0.0000001;

   bool buttonA = false, buttonB = false;

   while (true) {
      //  --- BEGIN LINE EXTRACTION ---
      auto [line, lineStream, flags] = lineExtraction<MaxLineLength>(inputStream);

      if (flags.endedLines) break;

      std::string msgLine
         = "Input at the line n. " + std::to_string(++lineCount) + " : ";

      if (flags.emptyLine){
         if (buttonA || buttonB){
            std::cout << msgLine << "WARNING: empty line\n";
         }
         continue;
      }

      if (flags.tooLongLine){
         throw std::invalid_argument(msgLine + "too long Line");
      }
      //  --- END LINE EXTRACTION ---

      //  --- BEGIN LINE CHECKS ---
      //  --- END LINE CHECKS ---

      //std::match_results<decltype(it)> match{}; same as smatch
      std::smatch matchLine{};
      
      if (std::regex_search(line.cbegin(), line.cend(), matchLine, std::regex{R"__((^.*?)\:(.*?)X.([\d]+)(.*?)Y.([\d]+).*)__"})){
         if (matchLine.size() < 6){
            throw std::invalid_argument(msgLine + "lack of title (before column ':') or numbers");
         }

         std::string_view title{matchLine[1].first, matchLine[1].second};

         std::string_view X{matchLine[3].first, matchLine[3].second};

         std::string_view Y{matchLine[5].first, matchLine[5].second};

         unsigned moveX;
         unsigned moveY;

         switch(std::from_chars(X.cbegin(), X.cend(), moveX).ec){
         case std::errc::result_out_of_range:
         case std::errc::invalid_argument:
            throw std::runtime_error(msgLine + "fix code");
         }

         switch(std::from_chars(Y.cbegin(), Y.cend(), moveY).ec){
         case std::errc::result_out_of_range:
         case std::errc::invalid_argument:
            throw std::runtime_error(msgLine + "fix code");
         }

         if (title == "Button A"){
            currSys.eq1[0] = moveX;
            currSys.eq2[0] = moveY;

            if (buttonA){
               throw std::invalid_argument(msgLine + "repeated button A part");
            }

            if (buttonB){
               std::cout << msgLine << "WARNING: button A after button B\n";
            }
            buttonA = true;
         } else
         if (title == "Button B"){
            currSys.eq1[1] = moveX;
            currSys.eq2[1] = moveY;

            if (buttonB){
               throw std::invalid_argument(msgLine + "repeated button B part");
            }
            buttonB = true;
         } else{
            currSys.eq1[2] = moveX;
            currSys.eq2[2] = moveY;

            if (!(buttonA && buttonB)){
               throw std::invalid_argument(msgLine + "some button part missing");
            }
            buttonA = false;
            buttonB = false;

            systems.push_back(currSys);
         }
      }

      lines.emplace_back(std::move(line)); //TODO: remove when not needed
   }

   TResult nTooManyPressures = 0U;
   TResult nPrizes = 0U;
   constexpr unsigned MAX_PRESSURES = 100U;
   constexpr double ROUND = 0.0001;

   using PointD = Point<double>;
   for(auto sys : systems){
      SquareMatrix<2> matrix;
      Matrix<2,1> prize;

      matrix[0] = {sys.eq1[0], sys.eq1[1]};
      matrix[1] = {sys.eq2[0], sys.eq2[1]};
      prize[0] = {sys.eq1[2]};
      prize[1] = {sys.eq2[2]};
      auto prizeRow = prize.buildTranspost()[0];

      TResult maxSys = std::max(sys.eq1[2], sys.eq2[2]);

      if ( (sys.eq1[2] < std::max(sys.eq1[0],sys.eq1[1])) ||
           (sys.eq2[2] < std::max(sys.eq2[0],sys.eq2[1])) ){
         continue;
      }

      PointD px0{0.,0}, py0{0, 0.}, pInters;

      auto y1 = (sys.eq1[1] == 0)? maxSys : sys.eq1[2]/sys.eq1[1];
      auto y2 = (sys.eq2[1] == 0)? maxSys : sys.eq2[2]/sys.eq2[1];

      auto x1 = (sys.eq1[0] == 0)? maxSys : sys.eq1[2]/sys.eq1[0];
      auto x2 = (sys.eq2[0] == 0)? maxSys : sys.eq2[2]/sys.eq2[0];

      px0.y = std::min(y1,y2);

      py0.x = std::min(x1,x2);

      TResult Q;

      bool foundPoint = false;
      PointD bestPoint;

      auto [inv, det] = matrix.buildInverse();

      if (std::abs(det) > 0.5){
         auto solSys = (inv * prize).buildTranspost()[0];

         //pInters = PointD(solSys); -> uses a method that is ICE for gcc on problem 6
         pInters = PointD(solSys[0],solSys[1]);

         // pInters is the only point that solves the system.
         // If it is integer and positive, well.

         double integerPart;
         if ( (pInters.x >= 0) && (pInters.y >= 0) &&
              (modf(pInters.x+ROUND/2.,&integerPart) < ROUND) && (modf(pInters.y+ROUND/2.,&integerPart) < ROUND) ){
            if ((pInters.x <= MAX_PRESSURES + ROUND) && (pInters.y <= MAX_PRESSURES + ROUND)){
               bestPoint = pInters;
               foundPoint = true;
            } else{
               ++nTooManyPressures;
            }
         }
      } else{
         // the two boundaries do not intersect each other: do not
         // consider pInters and just consider the edge between Px0 and Py0.
         // However, if the two constraints-edge are only parallel but
         // not corresponding, then there is no solution.

         // included in the lcm check
         //if (det > std::numeric_limits<TResult>::max()){
         //   throw std::runtime_error("Cannot compute lcm for too high numbers");
         //}

         // check the system range (TODO: a method of Matrix)
         double det1 = prizeRow[0]*matrix[1][1] - prizeRow[1]*matrix[0][1];
         double det2 = matrix[0][0]*prizeRow[1] - matrix[1][0]*prizeRow[0];

         if ((std::abs(det1) > 0.5) || (std::abs(det2) > 0.5)){
            continue; // no solution.
         }

         // the two equations are equivalent.
         // Let's look on any integer point along the edge.

         // In general, solve a simplex.

         {
            auto d = TResult(det+ROUND);

            Q = d;
            for(int i = 0; i < 2; ++i)
               for(int j = 0; j < 2; ++j)
                  Q = std::lcm(Q, TResult(matrix[i][j]+ROUND));

            bool bad = (Q%d != 0);
            for(int i = 0; i < 2; ++i)
               for(int j = 0; j < 2; ++j)
                  bad |= (Q % TResult(matrix[i][j]+ROUND) != 0);
            
            if (bad){
               throw std::runtime_error("Cannot compute lcm for too high numbers");
            }
         }

         /* unused
         double lb = std::min( scalarProd(eqMin,px0), scalarProd(eqMin,py0) );
         if (useInter){
            lb = std::min( lb, scalarProd(eqMin,pInters) );
         }*/
      
         auto ph = px0; // highest
         auto pr = py0; // rightest

         // search on the edge between ph and pr
         auto diffX = pr.x - ph.x;
         auto diffY = ph.y - pr.y;

         if (diffX+diffY < ROUND){
            continue; // (0,0) is the only point.
         }

         bool StartX0 = (diffX > ROUND);

         // search for the first integer point.
         // Start from one of the extremes that is on an axis.
         PointD p_integ = StartX0 ? ph : pr;

         // The move along dX, -dY (or -dX, dY)
         // needs, for example, dX integer if start from ph,
         // therefore |dY| = dX*f (f = diffY/diffX) min must
         // make the other coordinate C also integer:
         // that is, C - dX*f must be integer: now,
         // C and f are multiple of the same minimum 1/Q
         // where Q is lcm('det',matrix_coefficients).
         // Therefore, after multiplying by Q the fractional
         // parts of C and f, they become integer and we can
         // find the gcd of the two numbers, let's say
         // C0 and f0 -> now, within Q/gcd times f0 is equivalent
         // to C0 modulo-(Q/gcd) TODO: look for a known algotihm
         // to know C1, I have used one (see the code).
         // After C1, the periodicy is by N where N is the
         // minimum that makes N*f integer.
         // Given f = P/Q, it is enough to compute
         // gcd(P,Q) and consider f = [P/gcd] / [Q/gcd] -> N = Q/gcd.
         double c = StartX0 ? ph.y : pr.x;
         double f = StartX0 ? diffY/diffX : (diffX/diffY);

         double intC;
         auto fracC = modf(c, &intC);

         double intF;
         auto fracF = modf(f, &intF);

         TResult P = TResult(fracF*Q + ROUND);
         auto gcdPQ = std::gcd(P,Q);
         auto N = Q/gcdPQ;

         TResult cI = TResult(fracC*Q + ROUND);
         TResult fI = TResult(fracF*Q + ROUND);

         TResult C1{};
         if (fI == 0){
            if (cI > 0){
               // continue: no first point exists... maybe not possible
               // ... or maybe possible as need some mixed (positive-negative)
               // couple of values...
               continue;
            }
            // p_integ has already integer coordinates.
            C1 = 0U;
         } else{
            auto gcdCF = std::gcd(std::gcd(cI, fI), Q);
            auto Q_over_gcd = Q/gcdCF;

            if ((cI%gcdCF != 0) || (fI%gcdCF != 0) || (Q%gcdCF != 0)){
               throw std::runtime_error("Fix the code!");
            }

            cI /= gcdCF;
            fI /= gcdCF;

            auto howMany = Q_over_gcd/fI;

            auto r = Q_over_gcd - fI*howMany;

            auto check = fI;
            TResult steps = 0U;
            while (check != cI){
               // first try to reach or overcome cI.
               if (check < cI){
                  auto deltaSteps = 1 + (cI - check -1)/fI;
                  steps += deltaSteps;

                  check = (check + deltaSteps*fI) % Q_over_gcd;
               }
               if (check > cI){
                  // apply Q_over_gcd/fI steps and go back by r.
                  steps += howMany;
                  check -= r;
                  //TODO: speed-up with a certain number of (howMany+1)
                  //to advance that number of (check-r).
               }
            }

            C1 = steps+1U;
         }

         if (StartX0){
            p_integ.y -= (C1*f);
            p_integ.x += C1;
         } else{
            p_integ.x -= (C1*f);
            p_integ.y += C1;
         }

         // p_integ is now the first point.
         TResult nTokens = maxSys+1U; // as upper-bound.
         
         while ( StartX0 ? (p_integ.y > pr.y-ROUND)
                           : (p_integ.x > ph.x-ROUND) ){
            TResult nTokensCand = TResult(scalarProd(p_integ,eqMin) + ROUND);
            if ( (nTokensCand < nTokens) &&
                 (p_integ.x <= MAX_PRESSURES + ROUND) && (p_integ.y <= MAX_PRESSURES + ROUND) ){
               nTokens = nTokensCand;
               bestPoint = p_integ;
               foundPoint = true;
            }

            if (StartX0){
               p_integ.y -= (N*diffY/diffX);
               p_integ.x += N;
            } else{
               p_integ.x -= (N*diffX/diffY);
               p_integ.y += N;
            }
         }
      }

      if (foundPoint){
         count += TResult(scalarProd(bestPoint,eqMin) + ROUND);
         ++nPrizes;
      }
   }

   if (buttonA || buttonB){
      throw std::invalid_argument("Missing prize part at the end");
   }

   std::cout << "Number of lines: " << lineCount << std::endl;
   std::cout << "Number of systems: " << systems.size() << std::endl;
   std::cout << "Number of prizes: " << nPrizes << std::endl;
   std::cout << "Number of prizes discarded for too many pressures: " << nTooManyPressures << std::endl;
   std::cout << "\nResult P1: " << count << std::endl;
   std::cout << std::endl;

   return count;
}