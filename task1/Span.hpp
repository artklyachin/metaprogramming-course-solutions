#include <algorithm>
#include <span>
#include <concepts>
#include <cstdlib>
#include <iterator>
#include <vector>
#include <cassert>

namespace detail {

  template <std::size_t extent>
  struct SpanSize {
    SpanSize(std::size_t size) {
      assert(size <= extent);
    }
  };

  template <>
  struct SpanSize<std::dynamic_extent> {
    std::size_t size_;
    SpanSize(std::size_t size) : size_(size) {}
  };

}

template <class T, std::size_t extent = std::dynamic_extent>
class Span : private detail::SpanSize<extent> {
public:
  using element_type = T;
  using value_type = std::decay_t<T>;
  using reference = T&;
  using const_reference = const T&;
  using pointer = T*;
  using size_type = std::size_t;
  using const_pointer = const T*; 
  using iterator = T*;
  using reverse_iterator = std::reverse_iterator<T*>;
  using difference_type = std::ptrdiff_t;

  // Reimplement the standard span interface here
  // (some more exotic methods are not checked by the tests and can be sipped)
  // Note that unliike std, the methods name should be Capitalized!
  // E.g. instead of subspan, do Subspan.
  // Note that this does not apply to iterator methods like begin/end/etc.

  Span() requires (extent == 0 || extent == std::dynamic_extent) {}
  constexpr Span( const Span& other ) noexcept = default;

  template <std::contiguous_iterator It>
  explicit(extent != std::dynamic_extent)
  constexpr Span(It first, size_type count = extent)
    : detail::SpanSize<extent>(count), data_(std::to_address(first)) {}

  template <class U>
  constexpr Span( std::vector<U>& arr) noexcept 
    : detail::SpanSize<extent>(arr.size()), data_(arr.data()) {}

  template< class U, std::size_t N >
  constexpr Span( std::array<U, N>& arr ) noexcept 
    : detail::SpanSize<extent>(arr.size()), data_(arr.data()) {}

  template< class U, std::size_t N >
  constexpr Span(const std::array<U, N>& arr ) noexcept
    : detail::SpanSize<extent>(arr.size()), data_(arr.data()) {}

  template <class R>
  explicit(extent != std::dynamic_extent)
  constexpr Span(R&& range) : detail::SpanSize<extent>(std::ranges::size(range)), data_(std::data(range)) {

  }

  ~Span() = default;

  constexpr Span& operator=( const Span& other ) noexcept = default;

  constexpr reference operator[](size_t index) const {
    assert(index < Size());
    return *(data_ + index);
  }

  constexpr iterator begin() const noexcept {
    return data_;
  }

  constexpr iterator end() const noexcept {
    return data_ + Size();
  }

  constexpr reverse_iterator rbegin() const noexcept {
    return std::make_reverse_iterator(end() - 1);
  }

  constexpr reverse_iterator rend() const noexcept {
    return std::make_reverse_iterator(begin() - 1);
  }

  constexpr reference Front() const {
    assert(Size() > 0);
    return *data_;
  }

  constexpr reference Back() const {
    assert(Size() > 0);
    return *(data_ + Size() - 1);
  }

  constexpr pointer Data() const noexcept {
    return data_;
  }

  constexpr size_t Size() const noexcept {
    if constexpr (extent == std::dynamic_extent) {
      return this->size_;
    } else {
      return extent;
    }
  }

  template <std::size_t Count>
  constexpr auto First() const {
    assert(Count <= Size());
    return Span<T, Count>(data_, Count);
  }

  constexpr auto First(size_t Count) const {
    assert(Count <= Size());
    return Span<T> (data_, Count);
  }

  template< std::size_t Count >
  constexpr auto Last() const {
    assert(Count <= Size());
    return Span<T, Count> (data_ + Size() - Count, Count);
  }

  constexpr auto Last(size_t Count) const {
    assert(Count <= Size());
    return Span<T> (data_ + Size() - Count, Count);
  }

private:
  pointer data_ = nullptr;
};

template <typename R>
Span(R&&) -> Span<std::remove_reference_t<std::ranges::range_reference_t<R>>, std::dynamic_extent>;

template <std::contiguous_iterator It>
Span(It, std::size_t) -> Span<typename std::iterator_traits<It>::value_type, std::dynamic_extent>;

template <typename T, std::size_t N>
Span(std::array<T, N>&) -> Span<T, N>;

template <typename T, std::size_t N>
Span(const std::array<T, N>&) -> Span<const T, N>;