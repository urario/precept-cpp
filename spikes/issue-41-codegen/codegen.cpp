#include <cstddef>

namespace spike {

template<class T, std::size_t N>
class aligned_ptr {
public:
    constexpr explicit aligned_ptr(T* ptr) noexcept : ptr_(ptr) {}

    [[nodiscard]] constexpr T* get() const noexcept {
        return ptr_;
    }

private:
    T* ptr_;
};

} // namespace spike

extern "C" int raw_load(const int* ptr) {
    return *ptr;
}

extern "C" int wrapped_load(spike::aligned_ptr<const int, 64> ptr) {
    return *ptr.get();
}

extern "C" void raw_store(int* ptr, int value) {
    *ptr = value;
}

extern "C" void wrapped_store(spike::aligned_ptr<int, 64> ptr, int value) {
    *ptr.get() = value;
}
