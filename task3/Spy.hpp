#pragma once
#include <concepts>
#include <memory>
#include <cassert>

namespace {
consteval bool implies(bool a, bool b) {
  return !a || b;
}
}

template<class T>
class Spy {
  struct LoggingProxy;
  friend LoggingProxy;

 public:
  explicit Spy() requires std::default_initializable<T> = default;
  explicit Spy(const T &value) requires std::copyable<T>: value_(value) {}
  explicit Spy(T &&value) requires std::movable<T>: value_(std::move(value)) {}

  Spy(const Spy &other) requires std::copyable<T>
      : value_(other.value_),
        logger_(other.cloneLoggerSafely()) {
  }

  Spy(Spy &&other) requires std::movable<T>
      : value_(std::move(other.value_)),
        logger_(std::move(other.logger_)) {

  }

  Spy &operator=(const Spy &other) requires std::copyable<T> {
    value_ = other.value_;
    logger_ = other.cloneLoggerSafely();

    return *this;
  }

  Spy &operator=(Spy &&other) requires std::movable<T> {
    value_ = std::move(other.value_);
    logger_ = std::move(other.logger_);

    return *this;
  }

  T &operator*() {
    return value_;
  }

  const T &operator*() const {
    return value_;
  }

  LoggingProxy operator->() {
    if (!logging_proxies_alive_) {
      consecutive_calls_ = 0;
    }

    return LoggingProxy{*this};
  }

  bool operator==(const Spy &other) const requires std::equality_comparable<T> {
    return value_ == other.value_;
  }

  // Resets logger
  void setLogger() {
    logger_.reset();
  }

  template<std::invocable<unsigned int> Logger>
  requires (
      implies(std::copyable<T>, std::copyable<std::remove_reference_t<Logger>>) &&
          implies(std::movable<T>, std::movable<std::remove_reference_t<Logger>>)
  )
  void setLogger(Logger &&logger) {
    logger_ = std::make_unique<LoggerHolder < std::remove_reference_t<Logger>>
        > (std::forward<Logger>(logger));
  }

 private:
  struct LoggingProxy {
    explicit LoggingProxy(Spy<T> &ptr) : ptr_(ptr) {
      ++ptr_.logging_proxies_alive_;
    }

    T *operator->() {
      ++ptr_.consecutive_calls_;
      return &ptr_.value_;
    }

    ~LoggingProxy() {
      if ((--ptr_.logging_proxies_alive_ == 0) && ptr_.logger_) {
        ptr_.logger_->callLog(ptr_.consecutive_calls_);
      }
    }
   private:
    Spy<T> &ptr_;
  };

  struct LoggerHolderBase {
    virtual void callLog(unsigned int) = 0;
    virtual std::unique_ptr<LoggerHolderBase> clone() const = 0;
    virtual ~LoggerHolderBase() = default;
  };

  template<std::invocable<unsigned int> Logger>
  struct LoggerHolder {
  };

  template<std::invocable<unsigned int> Logger> requires std::copyable<Logger>
  struct LoggerHolder<Logger> : public LoggerHolderBase {
    explicit LoggerHolder(const Logger &value) : value_(value) {}
    explicit LoggerHolder(Logger &&value) : value_(std::move(value)) {}

    void callLog(unsigned int x) override {
      value_(x);
    }

    std::unique_ptr<LoggerHolderBase> clone() const override {
      return std::make_unique<LoggerHolder>(value_);
    }

   private:
    Logger value_;
  };

  template<std::invocable<unsigned int> Logger> requires std::movable<Logger>
  struct LoggerHolder<Logger> : public LoggerHolderBase {
    explicit LoggerHolder(Logger &&value) : value_(std::move(value)) {}

    void callLog(unsigned int x) override {
      value_(x);
    }

    std::unique_ptr<LoggerHolderBase> clone() const override {
      assert(false || "Unreachable!");
    }

   private:
    Logger value_;
  };

 private:
  std::unique_ptr<LoggerHolderBase> cloneLoggerSafely() const {
    if (logger_) {
      return logger_->clone();
    } else {
      return nullptr;
    }
  }

 private:
  T value_;
  std::unique_ptr<LoggerHolderBase> logger_;
  unsigned int consecutive_calls_{};
  unsigned int logging_proxies_alive_{};
};