#pragma once

#include <tuple>
#include <utility>

namespace NHash__MR{

    template<typename T>
    struct HashWrapper{
        HashWrapper(T v) : value{std::move(v)}{};

        T value;
    };

    template<typename T>
    inline auto operator|(std::size_t cumulated, const HashWrapper<T>& op) noexcept{
        return cumulated ^ (std::hash<std::decay_t<T>>{}(op.value) << 1);
    }

    class CombineHash{
    // idea of HashWrapper from
    // https://stackoverflow.com/questions/27582862/fold-expressions-with-arbitrary-callable
    private:
        struct Pushable{ // need to define before using, otherwise begin, end are not visible
                         // from inside the functions used in operator()
            std::vector<std::size_t> elements;

            void operator<<(std::size_t elem){elements.push_back(elem);}

            // methods to satisfy Iterable concept
            auto begin(){return elements.begin();}
            auto begin() const {return elements.begin();}

            auto end(){return elements.end();}
            auto end() const {return elements.end();}

            using iterator = decltype(elements)::iterator;
            using const_iterator = decltype(elements)::const_iterator;
        };
    public:
        template<NConcept__MR::Iterable Iter>
    #ifdef __cpp_static_call_operator
        static // -> only from C++23
    #endif
        auto operator()(const Iter& iter) noexcept{
            std::size_t res = 0U; // no effect on XOR
            int i = 0;

            static auto hasher = std::hash<std::decay_t<NConcept__MR::IterValueType<Iter>>>{};
            for(const auto& elem : iter){
                // res ^= (hasher(elem) << (15 & ++i));

                // hash combination copied from:
                // https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x
                // in turn. copied from boost.
                //res ^= (hasher(elem) + 0x9e3779b9 + (res<<6) + (res>>2));

                res ^= (hasher(elem) + (res << (6+(7 & i))) + (res >> (2+(3 & i))));
                ++i;
            }
            return res;
        }

        template<typename First, typename... Ts>
        requires ((sizeof...(Ts) > 0U) || (!NConcept__MR::Iterable<First>))
    #ifdef __cpp_static_call_operator
        static // -> only from C++23
    #endif
        auto operator()(First&& first, Ts&&... args) noexcept{
            //return (std::hash<std::decay_t<First>>{}(first) ^ ... ^ HashWrapper<std::decay_t<Ts>>{args}); poor hash combination

            Pushable pushable;

            pushable << (std::hash<std::decay_t<First>>{}(first));
            (pushable << ... << std::hash<std::decay_t<Ts>>{}(args));

            return operator()(pushable);
        }

    #ifndef __cpp_static_call_operator
        template<typename First, typename... Ts>
        requires ((sizeof...(Ts) > 0U) || (!NConcept__MR::Iterable<First>))
        //support to be used for std::apply before C++23
        static auto static_exec(First&& first, Ts&&... args) noexcept{
            return CombineHash{}.operator()<First&&, Ts&&...>(first, args...);
        }
    #endif
    };
} // namespace NHash__MR

template<typename T1, typename T2> // outside namespace NHash__MR
struct std::hash<std::pair<T1,T2>>
{
    inline std::size_t operator()(const std::pair<T1,T2>& pair) const noexcept
    { return NHash__MR::CombineHash{}(pair.first, pair.second); }
};

template<typename... Ts> // outside namespace NHash__MR
struct std::hash<std::tuple<Ts...>>
{
    inline std::size_t operator()(const std::tuple<Ts...>& tuple) const noexcept
#ifdef __cpp_static_call_operator
    {return std::apply( &NHash__MR::CombineHash{}.operator()<Ts&...>, tuple );}
#else
    { return std::apply( NHash__MR::CombineHash::static_exec<Ts&...>, tuple ); }
#endif
};

template<NConcept__MR::IsStdMapIter FwdIter>
requires requires(FwdIter it) {std::next(it); it->first;}

struct std::hash<FwdIter>
{
    inline std::size_t operator()(const FwdIter& it) const noexcept
    { return std::hash<std::decay_t<decltype(it->first)>>{}(it->first); }
};

template<typename FwdIter>
requires requires(FwdIter it) {std::next(it); *it;} && (!NConcept__MR::IsStdMapIter<FwdIter>)
         /*&& -> TODO: generates compilation troubles, insuccessfully tried to solve with std::decay_t
                       in IsStdMapIter and IsStdSetIter.
         ( // explicit accept the case of ->first and ->second only if it is a std::set iterator
            (!requires(FwdIter it) {it->first; it->second;})
            ||
            NConcept__MR::IsStdSetIter<FwdIter>
         )*/
struct std::hash<FwdIter>
{
    inline std::size_t operator()(const FwdIter& it) const noexcept
    { return std::hash<std::decay_t<decltype(*it)>>{}(*it); }
};