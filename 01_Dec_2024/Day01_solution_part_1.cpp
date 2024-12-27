
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_1)

using namespace NAoc__MR;

NAoc__MR::TResult day01Part1(std::shared_ptr<std::istream> inputStream)
{
    std::cout << "----- PART 1 -----" << std::endl;

    unsigned lineCount{0U};

    std::list<TResult> l1, l2;

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

        std::string s1, s2;
        TResult v1, v2{};
        lineStream >> v1 >> v2;

        if ( (v1 == std::numeric_limits<decltype(v1)>::max()) ||
             (v2 == std::numeric_limits<decltype(v2)>::max()) ){
            throw std::invalid_argument(msgLine + "Too wide number in: <" + line + ">");
        }

        if (unsigned long v3; (!lineStream) || (lineStream >> v3)){
            throw std::invalid_argument(msgLine + "Not exactly two numbers in: <" + line + ">");
        }
        //  --- END LINE CHECKS ---

        l1.push_back(v1);
        l2.push_back(v2);
    }

    TResult sum{};
    l1.sort();
    l2.sort();

    auto it2 = l2.cbegin();

    for(auto v1 : l1){
        auto v2 = *(it2++);

        auto diff = (v2 > v1)? (v2 - v1) : (v1 - v2);

        checkSumResult(sum,diff);
        sum += diff;
    }

    std::cout << "Number of lines: " << lineCount << std::endl;
    std::cout << "\nResult P1: " << sum << std::endl;
    std::cout << std::endl;

    return sum;
}