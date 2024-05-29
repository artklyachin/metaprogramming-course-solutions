#pragma once

#include <type_traits>
#include <value_types.hpp>
#include <type_lists.hpp>

using type_lists::Iterate;
using value_types::ValueTag;

template <class ValTag>
using Increase = ValueTag<ValTag::Value + 1>;

using type_lists::Cons;

template<int Last, int New>
struct FibSeq {
    using Head = ValueTag<Last>;
    using Tail = FibSeq<New, Last + New>;
};

template<int N, type_lists::TypeList PrimeS>
struct IsPrime {
    static constexpr int Res = 1;
};

template<int N, type_lists::TypeSequence PrimeS>
struct IsPrime<N, PrimeS> {
    static constexpr int Res = 0; 
};

template<int N, type_lists::TypeSequence PrimeS>
requires(N % PrimeS::Head::Value != 0)
struct IsPrime<N, PrimeS> {
    static constexpr int Res = IsPrime<N, typename PrimeS::Tail>::Res;
};

template<int N, type_lists::TypeList PrevPrimesSeq>
struct PrimesSeq
    : PrimesSeq<N + 1, PrevPrimesSeq> {};

template<int N, type_lists::TypeList PrevPrimesSeq>
requires(IsPrime<N, PrevPrimesSeq>::Res == 1)
struct PrimesSeq<N, PrevPrimesSeq> { 
    using Head = ValueTag<N>;
    using Tail = PrimesSeq<N + 1, Cons<ValueTag<N>, PrevPrimesSeq>>;
};

using Nats = Iterate<Increase, ValueTag<0>>;
using Fib = FibSeq<0, 1>;
using Primes = PrimesSeq<2, type_lists::Nil>;

