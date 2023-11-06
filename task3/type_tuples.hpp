#pragma once


namespace type_tuples
{

template<class... Ts>
struct TTuple {};

template<class TT>
concept TypeTuple = requires(TT t) { []<class... Ts>(TTuple<Ts...>){}(t); };

template<TypeTuple TT, typename T>
using Prepend =
decltype([]<class... Ts>(TTuple<Ts...>) {
    return TTuple<T, Ts...>{};
} (TT{}));

template<TypeTuple TT, typename T>
using Append =
decltype([]<class... Ts>(TTuple<Ts...>) {
    return TTuple<Ts..., T>{};
} (TT{}));

} // namespace type_tuples
