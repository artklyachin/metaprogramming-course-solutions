#pragma once
#include <concepts>
#include <type_tuples.hpp>

namespace type_lists {

template<class TL>
concept TypeSequence = requires {
    typename TL::Head;
    typename TL::Tail;
};

struct Nil {};

template<class TL>
concept Empty = std::derived_from<TL, Nil>;

template<class TL>
concept TypeList = Empty<TL> || TypeSequence<TL>;

namespace details {

template<TypeList TL>
constexpr std::size_t Length = 0;

template<TypeSequence TS>
constexpr std::size_t Length<TS> = 1 + Length<typename TS::Tail>;

};

// Cons<T, TL> -- список, в начале которого стоит T, а дальше -- элементы списка TL.

template<class T, TypeList TL>
struct Cons {
    using Head = T;
    using Tail = TL;
};

// FromTuple<TT> / ToTuple<TL> -- функции для конвертации между конечными списками и тюплами.

namespace details {

    template<class TT>
    struct FromTupleImpl;

    template<class T, class... Ts>
    struct FromTupleImpl<typename type_tuples::TTuple<T, Ts...>> {
        using Type = Cons<T, typename FromTupleImpl<typename type_tuples::TTuple<Ts...>>::Type>;
    };

    template<>
    struct FromTupleImpl<typename type_tuples::TTuple<>> {
        using Type = Nil;
    };

    using type_tuples::TTuple;
    using type_tuples::TypeTuple;
    template<TypeList TL, TypeTuple TT = TTuple<>>
    struct ToTupleImpl;
    
    template<Empty E, TypeTuple TT>
    struct ToTupleImpl<E, TT> {
        using Type = TT;
    };

    template<TypeSequence TS, TypeTuple TT>
    struct ToTupleImpl<TS, TT> {
        using Type = type_tuples::Prepend
            < typename ToTupleImpl<typename TS::Tail, TT>::Type
            , typename TS::Head>;
    };

}

template<class TT>
using FromTuple = typename details::FromTupleImpl<TT>::Type;

template<class TL>
using ToTuple = typename details::ToTupleImpl<TL>::Type;

// Repeat<T> -- бесконечный список из T.

template<class T>
struct Repeat {
    using Head = T;
    using Tail = Repeat<T>;
};

// Take<N, TL> --первые N элементов потенциально бесконечного списка TL.

template<std::size_t N, TypeList TL>
struct Take
    : Nil {};

template<std::size_t N, TypeSequence TS>
requires (N != 0)
struct Take<N, TS> {
    using Head = typename TS::Head;
    using Tail = Take<N - 1, typename TS::Tail>;
};

// Drop<N, TL> -- всё кроме первых N элементов списка TL.

template<std::size_t N, TypeList TL>
struct Drop
    : Nil {};

template<std::size_t N, TypeSequence TS>
struct Drop<N, TS>
    : Drop<N - 1, typename TS::Tail> {};

template<TypeSequence TS>
struct Drop<0, TS>
    : TS {};

// Replicate<N, T> -- список из N элементов равных T.

template<std::size_t N, class T>
struct Replicate
    : Nil {};

template<std::size_t N, class T>
requires (N > 0)
struct Replicate<N, T> {
    using Head = T;
    using Tail = Replicate<N - 1, T>;
};

// Map<F, TL> -- список из результатов применения F к элементам TL.

template<template<class> class F, TypeList TL>
struct Map;

template<template<class> class F, TypeSequence TS>
struct Map<F, TS> {
    using Head = F<typename TS::Head>;
    using Tail = Map<F, typename TS::Tail>;
};

template <template<class> class F, Empty TL>
struct Map<F, TL> : Nil {

};

//Filter<P, TL> --список лишь тех элементов TL, что удовлетворяют P<_>::Value. Относительный порядок элементов не должен меняться.

template<template<class> class P, TypeList TL>
struct Filter
    : Nil {};

template<template<class> class P, TypeSequence TS>
struct Filter<P, TS>
    : Filter<P, typename TS::Tail> {};

template<template<class> class P, TypeSequence TS>
requires (P<typename TS::Head>::Value)
struct Filter<P, TS> {
    using Head = typename TS::Head;
    using Tail = Filter<P, typename TS::Tail>;
};

// Iterate<F, T> -- список, в котором каждый следующий элемент является результатом применения метафункции F к предыдущему, а первый -- T.

template<template<class> class F, class T>
struct Iterate {
    using Head = T;
    using Tail = Iterate<F, F<T>>;
};


// Cycle<TL> -- бесконечный список, в котором раз за разом повторяется конечный список TL.

namespace details {

    template<TypeList TL, TypeList TLNext>
    struct CycleImpl 
        : Nil {};

    template<TypeSequence TL, TypeSequence TLNext>
    struct CycleImpl<TL, TLNext> {
        using Head = typename TL::Head;
        using Tail = CycleImpl<typename TL::Tail, TLNext>;
    };

    template<TypeSequence TLNext>
    struct CycleImpl<Nil, TLNext> {
        using Head = typename TLNext::Head;
        using Tail = CycleImpl<typename TLNext::Tail, TLNext>;
    };

} 

template<TypeList TL>
using Cycle = details::CycleImpl<TL, TL>;

// Inits<TL> -- список всех префиксов TL в порядке возрастания длины.

template <TypeList TL, typename... Ts>
struct Inits {
    using Head = FromTuple<type_tuples::TTuple<Ts...>>;
    using Tail = Inits<typename TL::Tail, Ts..., typename TL::Head>;
};

template <Empty TL, typename... Ts>
struct Inits<TL, Ts...> {
    using Head = FromTuple<type_tuples::TTuple<Ts...>>;
    using Tail = Nil;
};

// template<TypeList TL, std::size_t N = 0>
// struct Inits
//     : Nil{};

// template<TypeSequence TS, std::size_t N>
// requires (N == 0 || !Empty<Drop<N - 1, TS>>)
// struct Inits<TS, N> { 
//     using Head = Take<N, TS>;
//     using Tail = Inits<TS, N + 1>;
// };

// template<Empty E>
// struct Inits<E> {
//     using Head = Nil;
//     using Tail = Nil;
// };


//Tails<TL> --список всех суффиксов TL в порядке возрастания длины их дополнения до всего списка.

template<TypeList TL>
struct Tails {
    using Head = Nil;
    using Tail = Nil;
};

template<TypeSequence TS>
struct Tails<TS> {
    using Head = TS;
    using Tail = Tails<typename TS::Tail>;
};

template <Empty TL>
struct Tails<TL> {
    using Head = Nil;
    using Tail = Nil;
};

// Scanl<OP, T, TL> -- последовательность, в которой первый элемент -- T,
//  а каждый последующий получается путём применения OP<_, _>::Type к текущему и следующему элементу TL.

template<template<class,class> class OP, class T, TypeList TL>
struct Scanl
    : Cons<T, Nil> {};

template<template<class,class> class OP, class T, TypeSequence TS>
struct Scanl<OP, T, TS>
    : Cons<T, Scanl<OP, OP<T, typename TS::Head>, typename TS::Tail>> {};

// Foldl<OP, T, TL> -- тип, получаемый как OP<... OP<OP<T, TL[0]>, TL[1]> ... >.
// Если последовательность бесконечная, значение не определено.

namespace details {

template<template<class, class> class OP, class T, TypeList TL>
struct FoldlImpl {
    using Type = T;
};

template<template<class, class> class OP, class T, TypeSequence TS>
struct FoldlImpl<OP, T, TS> {
    using Type = FoldlImpl<OP, OP<T, typename TS::Head>, typename TS::Tail>::Type;
};

}

template<template<class, class> class OP, class T, TypeList TL>
using Foldl = details::FoldlImpl<OP, T, TL>::Type;

// Zip2<L, R> -- список пар из i - ых элементов списков L и R соответственно, идущих подряд.

template<TypeList L, TypeList R>
struct Zip2
    : Nil {};

template<TypeSequence L, TypeSequence R>
//requires (!Empty<typename L::Head> && !Empty<typename R::Head>)
struct Zip2<L, R> {
    using Head = type_tuples::TTuple<typename L::Head, typename R::Head>;
    using Tail = Zip2<typename L::Tail, typename R::Tail>;
};

// Zip<TL...> -- список тюплов по одному элементу фиксированного номера из каждого списка.

template<class... Ts> 
struct Zip;

template<TypeSequence... Ts>
struct Zip<Ts...> {
    using Head = type_tuples::TTuple<typename Ts::Head ...>;
    using Tail = Zip<typename Ts::Tail ...>;
};

template<TypeList... Ts>
requires (Empty<Ts> || ...)
struct Zip<Ts...>
    : Nil {};


// Бонусный уровень(+1 балл):
// GroupBy<EQ, TL> --список из списков подряд идущих элементов TL, "равных" последовательно

template<class T, TypeList TL>
struct Append
    : Cons<T, Nil> {};

template<class T, TypeSequence TS>
struct Append<T, TS>
    : Cons<typename TS::Head, Append<T, typename TS::Tail>> {};

template<template<class, class> class EQ, TypeList TL, TypeList TR = Nil>
struct GroupBy
    : Nil {};

template<template<class, class> class EQ, Empty E, TypeList TR>
requires (!Empty<TR>)
struct GroupBy<EQ, E, TR>
    : Cons<TR, Nil> {};

template<template<class, class> class EQ, TypeSequence TS, TypeList TR>
requires (Empty<TR> || EQ<typename TR::Head, typename TS::Head>::Value)
struct GroupBy<EQ, TS, TR>
    : GroupBy<EQ, typename TS::Tail, Append<typename TS::Head, TR>> {};

template<template<class, class> class EQ, TypeSequence TS, TypeList TR>
    requires (!EQ<typename TR::Head, typename TS::Head>::Value)
struct GroupBy<EQ, TS, TR> {
    using Head = TR;
    using Tail = GroupBy<EQ, TS>;
};

}
