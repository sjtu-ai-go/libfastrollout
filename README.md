# libfastrollout
[![Build Status](https://travis-ci.org/sjtu-ai-go/libfastrollout.svg)](https://travis-ci.org/sjtu-ai-go/libfastrollout)
[![GNU3 License](https://img.shields.io/github/license/sjtu-ai-go/libfastrollout.svg)](https://github.com/sjtu-ai-go/libfastrollout/blob/master/LICENSE)

```cpp
template<std::size_t W, std::size_t H>
class FastRolloutPolicy
{
    public:
        // Pass a board as argument. The return value will be (0, 1): 0: Black dominates; 1: White dominates
        virtual double run(const board::Board<W, H> &, double komi, board::Player start_player) = 0;
        virtual ~FastRolloutPolicy() {}
};

template<std::size_t W, std::size_t H>
class RandomRolloutPolicy: FastRolloutPolicy<W, H>
{
    public:
    explicit RandomRolloutPolicy(std::size_t rollout_count = 1, std::size_t parallel_threads = 1);
    virtual double run(const board::Board<W, H> &, double komi, board::Player start_player) override;
    virtual ~RandomRolloutPolicy() override;
};
```

## Usage
```
git submodule add {{repo_url}} vendor/libfastrollout
git submodule update --recursive --init
```
Then, in `CMakeLists.txt`:
```
add_subdirectory(vendor/libfastrollout)
include_directories(${libfastrollout_INCLUDE_DIR})

# After add_executable(your_prog)
target_link_libraries(your_prog fastrollout)
```

Enable test with `libfastrollout_enable_tests`, default `OFF`.
