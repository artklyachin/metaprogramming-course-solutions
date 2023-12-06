#pragma once

#include <type_traits>
#include <cstdint>

#include <array>
#include <string_view>
#include <utility>
#include <limits>

namespace detail {

    template <class Enum, Enum T>
    static constexpr std::string_view getNameByValue() noexcept {
        constexpr const std::string_view name(__PRETTY_FUNCTION__);
        constexpr auto type_pos = name.find("T = ") + 4;
        if constexpr (name[type_pos] == '(') // T = (FOO)10
            return std::string_view("");
        // T = FOO::A;
        constexpr auto end_type_pos = 
            (name.find(';', type_pos) == std::string_view::npos) 
            ? name.find(']', type_pos) 
            : name.find(';', type_pos);
        constexpr auto type_str = name.substr(type_pos, end_type_pos - type_pos);
        if constexpr (type_str.find("::") != std::string_view::npos) {
            constexpr auto name_pos = type_str.find("::") + 2;
            return type_str.substr(name_pos, type_str.size() - name_pos);
        }
        return type_str;
    }

    template<class Enum, char sign, std::size_t max_val, std::size_t invalidVal, std::size_t start_pos>
    static constexpr std::size_t searchEnums(auto& names_arr) noexcept {
        std::size_t curr_pos = start_pos;
        [&]<std::size_t... I>(std::index_sequence<I...>) {
            ([&]() {
                constexpr auto ValEnumType = static_cast<Enum>(sign*(static_cast<int>(I)+1));
                names_arr[curr_pos].first = getNameByValue<Enum, ValEnumType>();
                if (names_arr[curr_pos].first == std::string_view("")) {
                    names_arr[curr_pos].second = static_cast<Enum>(invalidVal); 
                } else {
                    names_arr[curr_pos].second = ValEnumType;
                    curr_pos += sign;
                }
            }(), ...);
        }(std::make_index_sequence<max_val>{});
        std::size_t added_size = sign * (curr_pos - start_pos);
        return added_size;
    }

    template<class Enum, std::size_t MAXN>
    static constexpr auto getEnumInfo() noexcept {
        constexpr std::size_t neg_max = std::min(-static_cast<std::size_t>(std::numeric_limits<std::underlying_type_t<Enum>>::min()), MAXN);
        constexpr std::size_t pos_max = std::min( static_cast<std::size_t>(std::numeric_limits<std::underlying_type_t<Enum>>::max()), MAXN);
        constexpr std::size_t enum_size = neg_max + 1 + pos_max;
        constexpr std::size_t zero_pos = neg_max;
        std::array<std::pair<std::string_view, Enum>, enum_size> names_arr;
        
        constexpr auto zero_name = getNameByValue<Enum, static_cast<Enum>(0)>();
        constexpr std::size_t zero_exist = (zero_name == "") ? 0 : 1;
        if constexpr (zero_exist) {
            names_arr[zero_pos] = std::make_pair(zero_name, static_cast<Enum>(0));
        }
        constexpr std::size_t invalidVal = MAXN + 1;
        std::size_t pos_size = searchEnums<Enum, 1, pos_max, invalidVal, zero_pos + zero_exist>(names_arr);
        std::size_t neg_size = searchEnums<Enum, -1, neg_max, invalidVal, zero_pos - 1>(names_arr);
    
        std::size_t start_index = zero_pos - neg_size;
        std::size_t arr_size = neg_size + zero_exist + pos_size;
        return std::make_pair(std::make_pair(start_index, arr_size), names_arr);
    }
}

template<class Enum, std::size_t MAXN>
constexpr auto enum_info = detail::getEnumInfo<Enum, MAXN>();

template <class Enum, std::size_t MAXN = 512>
        requires std::is_enum_v<Enum>
struct EnumeratorTraits {
    static constexpr std::size_t size() noexcept {
        return enum_info<Enum, MAXN>.first.second;
    }

    static constexpr Enum at(const std::size_t i) noexcept {
        return enum_info<Enum, MAXN>.second[enum_info<Enum, MAXN>.first.first + i].second;
    }

    static constexpr std::string_view nameAt(const std::size_t i) noexcept {
        return enum_info<Enum, MAXN>.second[enum_info<Enum, MAXN>.first.first + i].first;
    }
};