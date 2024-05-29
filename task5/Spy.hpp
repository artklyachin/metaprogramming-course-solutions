#pragma once
#include <concepts>
#include <unistd.h>
#include <variant>
#include <utility>
#include <memory>

template <bool From, bool To>
concept Implies = !From || To;

struct PointerCounter {
  void reset() { 
    count_ = 0; 
    expr_finished_ = false; 
  }
  unsigned int count_ = 0;
  bool expr_finished_ = false;
};

struct IStorage {
  virtual void operator()(unsigned int) = 0;
  virtual IStorage* self_copy() = 0;
  virtual ~IStorage() = default;
};

template<class F>
struct Storage : IStorage
{
  void operator()(unsigned int args) override { return f(args); };
  IStorage* self_copy() override {
    return new Storage<F>{std::forward<F>(f)};
  }
  Storage(F &&other_f) : f{std::move(other_f)} {}
  ~Storage() override = default;

  F f;
};

template <class T> /*, class Allocator = std::allocator<std::byte>*/ 
class Spy 
{

  template <class U>
  class Proxy 
  {
    public:
      Proxy(Spy<U>* spy) : spy_{spy} {}
      
      U* operator->() {
        ++spy_->pcounter_.count_;
        return &(spy_->value_);
      }

      ~Proxy() {
        if (spy_->logger_ != nullptr && !(spy_->pcounter_.expr_finished_)) {
          (*spy_->logger_)(spy_->pcounter_.count_);
        }
        spy_->pcounter_.expr_finished_ = true;
      }

    private:
      Spy<U>* spy_;
  };

public:
  // default constructor
  Spy() = default;

  // copy construction
  Spy(const T& value)
  requires std::copyable<T>
  : value_(value)
  {}

  // move construction
  Spy(T&& value) 
  requires std::movable<T>
  : value_{std::move(value)} 
  {}

  // copy construction
  Spy(const Spy& other) 
  requires std::copyable<T>
  : value_(other.value_)
  {

    if (other.logger_) {
      logger_.reset(other.logger_->self_copy());
    } else {
      logger_ = nullptr;
    }
  }

  // move construction
  Spy(Spy&& other) 
  requires std::movable<T>
  : value_{std::move(other.value_)} 
  {
    if (other.logger_) {
      logger_.reset(other.logger_->self_copy());
    } else {
      logger_ = nullptr;
    }
  }

  //copy assignment
  Spy<T>& operator=(const Spy<T>& other) 
  requires std::copyable<T>
  {
    if (this == &other) 
      return *this;

    value_ = other.value_;

    if (other.logger_) {
      logger_.reset(other.logger_->self_copy());
    } else {
      logger_ = nullptr;
    }
    pcounter_.reset();

    return *this;
  }

  //move assignment
  Spy<T>& operator=(Spy<T>&& other) 
  requires std::movable<T>
  {
    if (this == &other) 
      return *this;

    value_ = std::move(other.value_);

    if (other.logger_) {
      logger_.reset(other.logger_->self_copy());
    } else {
      logger_ = nullptr;
    }

    pcounter_ = PointerCounter(other.pcounter_);
    other.pcounter_.reset(); 
    return *this;
  }

  Proxy<T> operator->() 
  {
    if (pcounter_.expr_finished_)
      pcounter_.reset();
    return Proxy<T>(this);
  }

  T& operator *() { return value_; }
  const T& operator *() const { return value_; }

  // equality operators
  constexpr bool operator==(const Spy<T>& other) const
    requires std::equality_comparable<T>
  {
    return value_ == other.value_;
  }

  // destructor
  ~Spy() = default;

  /*
   * if needed (see task readme):
   *   default constructor
   *   copy and move construction
   *   copy and move assignment
   *   equality operators
   *   destructor
  */

  // Resets logger
  template <std::invocable<unsigned int> Logger> /* see task readme */
  requires (Implies<std::copyable<T>, std::copyable<std::remove_reference_t<Logger>>>) && 
            (Implies<std::movable<T>, std::movable<std::remove_reference_t<Logger>>>)
  void setLogger(Logger&& other_logger)
  {
    logger_.reset(new Storage<std::remove_cvref_t<Logger>>{std::forward<std::remove_reference_t<Logger>>(other_logger)});
  }
private:

  T value_;
  PointerCounter pcounter_; 

  std::unique_ptr<IStorage> logger_{nullptr};

  // Allocator allocator_;
};