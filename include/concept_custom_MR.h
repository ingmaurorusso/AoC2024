#pragma once

#include <algorithm>
#include <concepts>
#include <utility>

namespace NConcept__MR{

    template<typename Iter>
    concept Iterable = requires(std::decay_t<Iter> iter){std::next(std::begin(iter)) != std::end(iter); *(std::begin(iter));};
    //TODO: study how to use Iter as array type. As parameter in requires clause, it decays to pointer.
    //Note also that std::next dows not work in case of array (++ should be used, but ++ fails with string_view::iterator -> +1 works, which fails for other stuff).
    //Using alternative with lambda get troubles for classes without default constructor.
                       //requires(std::add_lvalue_reference_t<Iter> iter){std::begin(iter)++ != std::end(iter); *(std::begin(iter));};
                       //([](){std::remove_reference_t<Iter> iter; std::begin(iter)++ != std::end(iter); *(std::begin(iter)); return true;}());

    template<typename>
    struct IterValueInfo;

    template<typename T, std::size_t N>
    struct IterValueInfo<T[N]>{using type = T;};

    template<Iterable T> requires requires (T t){t.begin();}
    struct IterValueInfo<T>{using type = typename T::iterator::value_type;};

    template<typename T>
    using IterValueType = typename IterValueInfo<T>::type;


    template<typename T>
    concept SummAndOrdinable = requires(T t){{t+t}; {t < t}; {t <= t};};


    template<typename FwdIter>
    concept IsStdMapIter = requires(FwdIter it) {std::next(it); it->first; it->second;} &&
                            [](){
                                using KeyT = std::decay_t<decltype(std::declval<FwdIter>()->first)>;
                                using ValueT = std::decay_t<decltype(std::declval<FwdIter>()->second)>;
                                return std::same_as< typename std::map<KeyT,ValueT>::iterator, FwdIter > ||
                                       std::same_as< typename std::map<KeyT,ValueT>::const_iterator, FwdIter > ||
                                       std::same_as< typename std::unordered_map<KeyT,ValueT>::iterator, FwdIter > ||
                                       std::same_as< typename std::unordered_map<KeyT,ValueT>::const_iterator, FwdIter > ||
                                       std::same_as< typename std::multimap<KeyT,ValueT>::iterator, FwdIter > ||
                                       std::same_as< typename std::multimap<KeyT,ValueT>::const_iterator, FwdIter > ||
                                       std::same_as< typename std::unordered_multimap<KeyT,ValueT>::iterator, FwdIter > ||
                                       std::same_as< typename std::unordered_multimap<KeyT,ValueT>::const_iterator, FwdIter >;
                            }();


    template<typename FwdIter>
    concept IsStdSetIter = requires(FwdIter it) {std::next(it); *it;} &&
                            [](){
                                using KeyT = std::decay_t<decltype(std::declval<FwdIter>()->first)>;
                                using ValueT = std::decay_t<decltype(std::declval<FwdIter>()->second)>;
                                return std::same_as< typename std::set<KeyT,ValueT>::iterator, FwdIter > ||
                                       std::same_as< typename std::set<KeyT,ValueT>::const_iterator, FwdIter > ||
                                       std::same_as< typename std::unordered_set<KeyT,ValueT>::iterator, FwdIter > ||
                                       std::same_as< typename std::unordered_set<KeyT,ValueT>::const_iterator, FwdIter > ||
                                       std::same_as< typename std::multiset<KeyT,ValueT>::iterator, FwdIter > ||
                                       std::same_as< typename std::multiset<KeyT,ValueT>::const_iterator, FwdIter > ||
                                       std::same_as< typename std::unordered_multiset<KeyT,ValueT>::iterator, FwdIter > ||
                                       std::same_as< typename std::unordered_multiset<KeyT,ValueT>::const_iterator, FwdIter >;
                            }();


    // copied from https://en.cppreference.com/w/cpp/language/constraints
    template<typename T>
    concept Hashable = requires(T a)
    {
        { std::hash<T>{}(a) } -> std::convertible_to<std::size_t>;
    };

    // copied from https://stackoverflow.com/questions/54182239/c-concepts-checking-for-template-instantiation
    // and extended for non-type template parmeters after up to 3 tpyenames.
    template <typename T, template <typename...> class Z>
    struct is_specialization_of : std::false_type {};

    template <typename... Args, template <typename...> class Z>
    struct is_specialization_of<Z<Args...>, Z> : std::true_type {};

    // cannot have the same name, as it is a class. TODO: study other solutions ?
    template <typename T, template <typename, auto...> class Z>
    struct is_specialization_of_1 : std::false_type {};

    template <typename T1, auto... args, template <typename, auto...> class Z>
    struct is_specialization_of_1<Z<T1, args...>, Z> : std::true_type {};

    template <typename T, template <typename, typename, auto...> class Z>
    struct is_specialization_of_2 : std::false_type {};

    template <typename T1, typename T2, auto... args, template <typename, typename, auto...> class Z>
    struct is_specialization_of_2<Z<T1, T2, args...>, Z> : std::true_type {};

    template <typename T, template <typename, typename, typename, auto...> class Z>
    struct is_specialization_of_3 : std::false_type {};

    template <typename T1, typename T2, typename T3, auto... args, template <typename, typename, typename, auto...> class Z>
    struct is_specialization_of_3<Z<T1, T2, T3, args...>, Z> : std::true_type {};


    //template <typename T, template <typename...> class Z>
    //inline constexpr bool is_specialization_of_v = is_specialization_of<T,Z>::value;

    template<typename T, template <typename...> class Z>
    concept Specializes = is_specialization_of<T,Z>::value;

    //template <typename T, template <typename, auto...> class Z>
    //inline constexpr bool is_specialization_of_v_1 = is_specialization_of_1<T,Z>::value;

    template<typename T, template <typename, auto...> class Z>
    concept Specializes_1 = is_specialization_of_1<T,Z>::value;

    //template <typename T, template <typename, typename, auto...> class Z>
    //inline constexpr bool is_specialization_of_v_2 = is_specialization_of_2<T,Z>::value;

    template<typename T, template <typename, typename, auto...> class Z>
    concept Specializes_2 = is_specialization_of_2<T,Z>::value;

    //template <typename T, template <typename, typename, typename, auto...> class Z>
    //inline constexpr bool is_specialization_of_v_3 = is_specialization_of_3<T,Z>::value;

    template<typename T, template <typename, typename, typename, auto...> class Z>
    concept Specializes_3 = is_specialization_of_3<T,Z>::value;


    template<typename Callee, typename Result, typename... Params>
    concept Callable = requires (Params... params){ {std::declval<Callee>()(params...)} -> std::convertible_to<Result>; };

    template<typename EvalCombiner, typename Value, typename Result = Value, typename... Others>
    concept computerCombiner2 = requires (Value v, Others... others){ {std::declval<EvalCombiner>()(v,v,others...)} -> std::convertible_to<Result>; };

    template<typename EvalCombiner, typename Value, typename... Others>
    concept voidCombiner2 = computerCombiner2<EvalCombiner, Value, void, Others...>;


    template<typename UnaryPredicate, typename Type>
    concept IsUnaryPredFor = std::convertible_to<std::invoke_result_t<UnaryPredicate,Type>, bool>;

    template<typename BinaryPredicate, typename Type>
    concept IsBinaryPredFor = std::convertible_to<std::invoke_result_t<BinaryPredicate,Type,Type>, bool>;


    inline constexpr auto idemExtractor = [] (auto&& elem) -> auto&&{return elem;};
} // namespace NConcept__MR