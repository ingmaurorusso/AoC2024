
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_1)

using namespace NAoc__MR;

NAoc__MR::TResult day02Part1(std::shared_ptr<std::istream> inputStream)
{
    std::cout << "----- PART 1 -----" << std::endl;

    unsigned lineCount{0U};

    TResult nSafeReports{};

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
        if ( std::find_if(std::begin(line), std::end(line), [](char ch){return !(std::isdigit(ch) || std::isspace(ch));})
             != std::end(line) ){
            throw std::invalid_argument(msgLine + "Not only numbers in: <" + line + ">");
        }
        //  --- END LINE CHECKS ---

        bool first = true, second{};
        bool increase{};
        unsigned val{}, oldVal{};
        bool badReport = false;

        while (lineStream >> val){
            if (!first){
                bool newInc = (val > oldVal);
                if (second){
                    increase = newInc;
                    second = false;
                } else
                if (increase != newInc){
                    badReport = true;
                }

                const auto diff = increase? val-oldVal: (oldVal - val);
                if ((diff == 0) || (diff > 3)){
                    badReport = true;
                }
            } else{
                first = false;
                second = true;
            }

            oldVal = val;
        }

        if (val == std::numeric_limits<decltype(val)>::max()){
            throw std::invalid_argument(msgLine + "Too wide number in: <" + line + ">");
        }

        if (!badReport){
            ++nSafeReports;
        }
    }

    std::cout << "Number of lines: " << lineCount << std::endl;
    std::cout << "\nResult P1: " << nSafeReports << std::endl;
    std::cout << std::endl;

    return nSafeReports;
}