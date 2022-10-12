#include <span>
#include <concepts>
#include <cstdlib>
#include <array>
#include <iterator>

inline constexpr std::ptrdiff_t dynamic_stride = -1;

namespace {

template<std::size_t extent>
class ExtentStorage {
 public:
  explicit ExtentStorage(std::size_t) {}

  [[nodiscard]] constexpr std::size_t Size() const {
    return extent;
  }
};

template<>
class ExtentStorage<std::dynamic_extent> {
 public:
  explicit ExtentStorage(std::size_t extent) : extent_(extent) {}

  [[nodiscard]] constexpr std::size_t Size() const {
    return extent_;
  }

 private:
  std::size_t extent_;
};

template<std::ptrdiff_t stride>
class StrideStorage {
 public:
  explicit StrideStorage(std::ptrdiff_t) {}

  [[nodiscard]] constexpr std::ptrdiff_t Stride() const {
    return stride;
  }
};

template<>
class StrideStorage<dynamic_stride> {
 public:
  explicit StrideStorage(std::ptrdiff_t stride) : stride_(stride) {}

  [[nodiscard]] constexpr std::ptrdiff_t Stride() const {
    return stride_;
  }

 private:
  std::ptrdiff_t stride_;
};

template<typename U>
concept Container = requires(U container) {
  container.size();
  container.data();
};

template<typename T, typename U>
constexpr auto divide_round_up(T a, U b) {
  return (a + b - 1) / b;
}

} // namespace

template
    <class T, std::size_t extent = std::dynamic_extent, std::ptrdiff_t stride = 1>
class Slice : public ExtentStorage<extent>, public StrideStorage<stride> {
 public:
  Slice()
      : ExtentStorage<extent>(extent),
        StrideStorage<stride>(stride),
        data_(nullptr) {}

  template<Container U>
  Slice(U &container)
      : ExtentStorage<extent>(container.size()),
        StrideStorage<stride>(stride),
        data_(container.data()) {}

  template<std::contiguous_iterator It>
  Slice(It first, std::size_t count, std::ptrdiff_t skip)
      : ExtentStorage<extent>(count), StrideStorage<stride>(skip), data_(std::to_address(first)) {}

  class iterator;

  using value_type = typename iterator::value_type;
  using element_type = T;
  using size_type = std::size_t;
  using pointer = typename iterator::pointer;
  using const_pointer = const T *;
  using reference = typename iterator::reference;
  using const_reference = const element_type &;
  using difference_type = typename iterator::difference_type;

  T &operator[](size_type index) const {
    return *(data_ + index * this->Stride());
  }

  template<class OtherT, std::size_t other_extent, std::ptrdiff_t other_stride>
  bool operator==(const Slice<OtherT, other_extent, other_stride> &other) const {
    return std::equal(this->begin(), this->end(), other.begin());
  }

  template<class OtherT, std::size_t other_extent, std::ptrdiff_t other_stride>
  bool operator!=(const Slice<OtherT, other_extent, other_stride> &other) const {
    return !(*this == other);
  }

  template<class OtherT, std::size_t other_extent, std::ptrdiff_t other_stride>
  requires std::convertible_to<T, OtherT>
      && (other_extent <= extent || other_extent == std::dynamic_extent)
      && (stride == other_stride || other_stride == dynamic_stride)
      && (!std::is_same_v<Slice, Slice<OtherT, other_extent, other_stride>>)
  operator Slice<OtherT, other_extent, other_stride>() const {
    return {data_, this->Size(), this->Stride()};
  }

  class iterator : private StrideStorage<stride> {
   public:
    using iterator_category = std::contiguous_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = std::remove_cv_t<T>;
    using pointer = T *;
    using reference = element_type &;

    friend Slice;

    iterator() : StrideStorage<stride>(stride), ptr_(nullptr) {}

    reference operator*() const {
      return *ptr_;
    }

    pointer operator->() const {
      return ptr_;
    }

    reference operator[](difference_type n) const {
      return ptr_[n * this->Stride()];
    }

    iterator &operator++() {
      ptr_ += this->Stride();
      return *this;
    }

    iterator operator++(int) {
      iterator copy(*this);
      ++(*this);
      return copy;
    }

    iterator &operator--() {
      ptr_ -= this->Stride();
      return *this;
    }

    iterator operator--(int) {
      iterator copy(*this);
      --(*this);
      return copy;
    }

    iterator &operator+=(difference_type n) {
      ptr_ += n;
      return *this;
    }

    iterator operator+(difference_type n) const {
      iterator copy(*this);
      copy += n;
      return copy;
    }

    friend iterator operator+(difference_type n, const iterator &other) {
      iterator copy(other);
      copy += n;
      return copy;
    }

    iterator &operator-=(difference_type n) {
      ptr_ -= n;
      return *this;
    }

    iterator operator-(difference_type n) const {
      iterator copy(*this);
      copy -= n;
      return copy;
    }

    difference_type operator-(const iterator &other) const {
      return this->ptr_ - other.ptr_;
    }

    bool operator<(const iterator &other) const {
      return this->ptr_ < other.ptr_;
    }

    bool operator>(const iterator &other) const {
      return other < *this;
    }

    bool operator>=(const iterator &other) const {
      return !(*this < other);
    }

    bool operator<=(const iterator &other) const {
      return !(other < *this);
    }

    bool operator==(const iterator &other) const {
      return this->ptr_ == other.ptr_;
    }

   private:
    iterator(pointer ptr, std::ptrdiff_t skip) : StrideStorage<stride>(skip), ptr_(ptr) {}

    pointer ptr_;
  };

  using reverse_iterator = std::reverse_iterator<iterator>;

  iterator begin() const {
    return iterator(data_, this->Stride());
  }

  iterator end() const {
    return iterator(data_ + this->Size() * this->Stride(), this->Stride());
  }

  reverse_iterator rbegin() const {
    return std::make_reverse_iterator(end());
  }

  reverse_iterator rend() const {
    return std::make_reverse_iterator(begin());
  }

  friend iterator begin(Slice &slice) {
    return slice.begin();
  }

  friend iterator end(Slice &slice) {
    return slice.end();
  }

  [[nodiscard]] constexpr T *Data() const {
    return data_;
  }

  Slice<T, std::dynamic_extent, stride> First(std::size_t count) const {
    return {data_, std::min(count, this->Size()), this->Stride()};
  }

  template<std::size_t count>
  Slice<T, count, stride> First() const {
    return {data_, std::min(count, this->Size()), this->Stride()};
  }

  Slice<T, std::dynamic_extent, stride> Last(std::size_t count) const {
    auto new_extent = std::min(this->Size(), count);
    return {data_ + (this->Size() - count) * this->Stride(), new_extent, this->Stride()};
  }

  template<std::size_t count>
  Slice<T, count, stride> Last() const {
    auto new_extent = std::min(this->Size(), count);
    return {data_ + (this->Size() - count) * this->Stride(), new_extent, this->Stride()};
  }

  Slice<T, std::dynamic_extent, stride> DropFirst(std::size_t count) const {
    return {data_ + count * this->Stride(), this->Size() - count, this->Stride()};
  }

  template<std::size_t count>
  Slice<T, extent - count, stride> DropFirst() const {
    return {data_ + count * this->Stride(), this->Size() - count, this->Stride()};
  }

  Slice<T, std::dynamic_extent, stride> DropLast(std::size_t count) const {
    return {data_, this->Size() - count, this->Stride()};
  }

  template<std::size_t count>
  Slice<T, extent - count, stride> DropLast() const {
    return {data_, this->Size() - count, this->Stride()};
  }

  Slice<T, std::dynamic_extent, dynamic_stride> Skip(std::ptrdiff_t skip) const {
    return {data_, divide_round_up(this->Size(), skip), this->Stride() * skip};
  }

  template<std::ptrdiff_t skip>
  Slice<T,
        (extent == std::dynamic_extent || stride == dynamic_stride) ? std::dynamic_extent
                                                                    : divide_round_up(extent, skip),
        stride == dynamic_stride ? stride : (stride * skip)> Skip() const {
    return {data_, divide_round_up(this->Size(), skip), this->Stride() * skip};
  }

 private:
  T *data_;
};

template<class T, std::size_t N>
Slice(std::array<T, N> &) -> Slice<T, N>;

template<class U>
Slice(U &) -> Slice<typename U::value_type, std::dynamic_extent>;

template<std::contiguous_iterator It>
Slice(It, std::size_t, std::ptrdiff_t) -> Slice<typename It::value_type,
                                                std::dynamic_extent,
                                                dynamic_stride>;
