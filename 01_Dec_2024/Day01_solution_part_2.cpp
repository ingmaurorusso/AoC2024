
#include "AOC.h"
#include INCLUDE_FILE(HEADER_PART_2)

using namespace NAoc__MR;

NAoc__MR::TResult day01Part2(std::shared_ptr<std::istream> inputStream)
{
    std::cout << "----- PART 2 -----" << std::endl;

    unsigned lineCount{0U};

    std::list<unsigned long> l1, l2;

    //std::string msgLine, s1, s2;

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
        unsigned long v1{}, v2{};
        lineStream >> v1 >> v2;

        if ( (v1 == std::numeric_limits<decltype(v1)>::max()) ||
             (v2 == std::numeric_limits<decltype(v2)>::max()) ){
            // more specific error, compared to next exception.
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

    std::size_t c1{}, c2{};
    TResult v1, v2{};

    auto it1 = l1.cbegin(), it1_end = l1.cend();
    auto it2 = l2.cbegin(), it2_end = l2.cend();
    if (!l2.empty()){
        v2 = *(it1++);
    }

    while ((it1 != it1_end) && (it2 != it2_end)){
        v1 = *(it1++);

        if (v1 > v2){
            it2 = std::find_if( it2, it2_end, [v1](auto v){return v >= v1;});
            if (it2 == it2_end) break;
            v2 = *(it2++);
        }

        if (v1 == v2){
            auto new_it1 =
                std::find_if( it1, it1_end, [v1](auto v){return v > v1;});
            auto d1 = std::distance(it1, new_it1) + 1U;
            it1 = new_it1;

            auto new_it2 =
                std::find_if( it2, it2_end, [v2](auto v){return v > v2;});
            auto d2 = std::distance(it2, new_it2) + 1U;
            it2 = new_it2;

            if (sum > std::numeric_limits<TResult>::max()/v1/d1/d2){
                throw std::runtime_error("Overflow: TResult to extend");
            }
            sum += (v1*d1*d2);
        }

        if (v1 < v2){
            it1 = std::find_if( it1, it2_end, [v2](auto v){return v >= v2;});
            if (it1 == it1_end) break;
        }
    }

    std::cout << "Number of lines: " << lineCount << std::endl;
    std::cout << "\nResult P2: " << sum << std::endl;
    std::cout << std::endl;

    return sum;
}
