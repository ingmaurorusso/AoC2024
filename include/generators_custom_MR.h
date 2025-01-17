#pragma once

#include <algorithm>
#include <exception>
#include <tuple>
#include <utility>
#include <vector>

#include "enum_custom_MR.h"

#include "concept_custom_MR.h"

namespace NGen__MR{

    // TODO: exploit C++20 ranges
    template<NConcept__MR::Iterable Iter>
    class Sequences{
    public:
        using SequencesException = std::string;

        using Iterator = decltype(std::cbegin(std::declval<Iter>()));
        using Sequence = std::vector<Iterator>;

        using TSeqResult = std::tuple<Sequence,std::size_t,bool>;

        explicit Sequences(Iter opt, std::size_t seqLength = 1U) : m_allOptions{std::move(opt)} {
            reset(seqLength);
        }

        // in case of calling nextLexicographic after reset, second sequence
        // is provided first, but this mens undefined behaviour if
        // beginLexicographic() returns false (for no sequence existing).
        // -> call always first beginLexicographic().
        void reset(std::size_t seqLength) {
            if (!seqLength){
                throw SequencesException("no way to generate empty sequences");
            }

            m_positions.resize(seqLength);
            static_cast<void>(beginLexicographic()); // avoids problem if nextLexicographic is called.
        }

        TSeqResult beginLexicographic() {
            for(auto& p : m_positions) p = std::cbegin(m_allOptions);
            m_lastChangeIdx = 0U;

            return std::make_tuple(m_positions,m_lastChangeIdx,!m_allOptions.empty());
        }

        // first: new sequence
        // second: minimum index modified compared to previous sequence (0 the first time)
        // third: boolean is false if no new sequence existed (and previous elements make no sense)
        // Undefined behaviour if forceChangeNonLast==true and forceChangeIdx is out-of-range,
        // or if beginLexicographic() returned false (for no sequence existing).
        TSeqResult nextLexicographic(std::size_t forceChangeIdx = 0U, bool forceChangeNonLast = false) {
            bool ended = false;

            std::size_t i = forceChangeNonLast? forceChangeIdx : m_positions.size()-1U;
            while(std::next(m_positions[i]) == std::cend(m_allOptions)){
                if (i == 0){
                    ended = true;
                    break;
                }
                --i;
            }

            m_lastChangeIdx = i;
            if (!ended){
                m_positions[i] = std::next(m_positions[i]);
                ++i;

                while(i < m_positions.size()){
                    m_positions[i++] = std::cbegin(m_allOptions);
                }
            } //else: no change on m_positions so that
              // nextLexicographic may be called again with no risk.

            return std::make_tuple(m_positions,m_lastChangeIdx,!ended);
        }

        std::size_t getIndexAt(std::size_t idx) const{
            return std::distance(m_allOptions.cbegin(), m_positions[idx]);
        }

        void force(std::size_t idx, std::size_t pos){
            m_positions[idx] = std::next(m_allOptions.cbegin(),pos);
        }

    private:
        Iter m_allOptions;

        Sequence m_positions;
        std::size_t m_lastChangeIdx;
    };

} // namespace NSpace__MR
