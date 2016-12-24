#ifndef FASTROLLOUT_HEADER
#define FASTROLLOUT_HEADER
#include <functional>
#include <cstddef>
#include <memory>
#include <thread>
#include <random>
#include <board.hpp>

namespace board
{
    template<std::size_t W, std::size_t H>
    class Board;
}

namespace fastrollout
{
    template<std::size_t W, std::size_t H>
    class FastRolloutPolicy
    {
    public:
        // Pass a board as argument. The return value will be (0, 1): 0: Black dominates; 1: White dominates
        virtual double run(const board::Board<W, H> &, double komi, board::Player start_player) = 0;
        virtual ~FastRolloutPolicy() {}
    };

    template<std::size_t W, std::size_t H>
    class RandomRolloutPolicy;

    namespace detail
    {
        template<std::size_t W, std::size_t H>
        class RandomRolloutPolicyImpl
        {
            std::size_t rollout_cnt_, thread_cnt_;
            friend class RandomRolloutPolicy<W, H>;

            std::mt19937 gen;

            RandomRolloutPolicyImpl(std::size_t rollout_cnt, std::size_t thread_cnt):
                    rollout_cnt_(rollout_cnt), thread_cnt_(thread_cnt), gen(std::random_device()())
            {}

            // x: white - black + komi;
            // impact_factor: [0, 1]. The less the more close to 0.5
            static double logistic(double x, double impact_factor)
            {
                return 1.0 / (1.0 + std::exp((-impact_factor) * x));
            }

            double run_single(const board::Board<W, H> &b, double komi, board::Player start_player)
            {
                static constexpr std::size_t REFRESH_GOODPOS_INTERVAL = 10;
                //static constexpr std::size_t MAX_ROLLOUT_CNT = 100;
                board::Board<W, H> state(b);
                using BoardType = decltype(state);
                board::Player cur_player = start_player;

                std::vector<board::GridPoint<W, H>> valid_pos_w, valid_pos_b;
                std::size_t valid_pos_age = REFRESH_GOODPOS_INTERVAL;
                std::size_t rollout_cnt = 0;
                for (;;)
                {
                    /*
                    if (rollout_cnt > MAX_ROLLOUT_CNT)
                        break;
                    */
                    board::GridPoint<W, H> cur_point;
                    bool is_empty = false;
                    do
                    {
                        auto &cur_valid_pos_vec = cur_player == board::Player::B ? valid_pos_b : valid_pos_w;
                        if (valid_pos_age >= REFRESH_GOODPOS_INTERVAL || cur_valid_pos_vec.empty())
                        {
                            cur_valid_pos_vec = state.getAllGoodPosition(cur_player);
                            std::shuffle(cur_valid_pos_vec.begin(), cur_valid_pos_vec.end(), gen);
                            if (cur_valid_pos_vec.empty())
                            {
                                is_empty = true;
                                break;
                            }
                            valid_pos_age = 0;
                        }
                        cur_point = *cur_valid_pos_vec.rbegin();
                        cur_valid_pos_vec.pop_back();
                        ++valid_pos_age;
                    } while (state.getPosStatus(cur_point, cur_player) != BoardType::PositionStatus::OK);
                    if (is_empty)
                        break;
                    state.place(cur_point, cur_player);
                    ++rollout_cnt;
                    cur_player = board::getOpponentPlayer(cur_player);
                }
/*
                std::size_t black_lib = 0, white_lib = 0;
                for (auto it = state.groupBegin(); it != state.groupEnd(); ++it)
                {
                    switch(it->getPlayer())
                    {
                        case board::Player::B:
                            black_lib += it->getLiberty();
                            break;
                        case board::Player::W:
                            white_lib += it->getLiberty();
                            break;
                    }
                }
*/
                std::size_t black_field = 0, white_field = 0;
                for (std::size_t i = 0; i < W; ++i)
                    for (std::size_t j = 0; j < H; ++j)
                    {
                        board::GridPoint<W, H> cur_point(i, j);
                        switch (state.getPointState(board::GridPoint<W, H>(i, j)))
                        {
                            case board::PointState::B:
                                ++black_field;
                                break;
                            case board::PointState::W:
                                ++white_field;
                                break;
                            default:
                                std::size_t neighbor_b = 0, neighbor_w = 0;
                                cur_point.for_each_adjacent([&](board::GridPoint<W, H> adjP) {
                                    switch (state.getPointState(adjP)) {
                                        case board::PointState::B:
                                            ++neighbor_b;
                                            break;
                                        case board::PointState::W:
                                            ++neighbor_w;
                                            break;
                                        default:
                                            break;
                                    }
                                });

                                if (neighbor_b != 0 && neighbor_w == 0)
                                    ++black_field;
                                if (neighbor_b == 0 && neighbor_w != 0)
                                    ++white_field;
                                break;
                        }
                    }

                double impact_factor = std::pow(0.98, rollout_cnt);
                return logistic((int)white_field - (int)black_field + komi, impact_factor); // minus <=> black dominates <=> return 0
            }

            void thread_runner(std::size_t cnt, const board::Board<W, H>& b, double komi,
                                      board::Player start_player, double *presult_sum)
            {
                double result = 0;
                for (std::size_t i=0; i<cnt; ++i)
                    result += run_single(b, komi, start_player);
                *presult_sum = result;
            }

            double run(const board::Board<W, H> &b, double komi, board::Player start_player)
            {
                if (thread_cnt_ == 1)
                {
                    double result_sum;
                    thread_runner(rollout_cnt_, b, komi, start_player, &result_sum);
                    return result_sum / rollout_cnt_;
                }

                std::size_t remain_round = rollout_cnt_;
                std::vector<std::thread> threads;
                std::vector<double> result_sum(thread_cnt_);
                for (std::size_t i=0; i<thread_cnt_; ++i)
                {
                    using namespace std::placeholders;
                    std::size_t alloc_tasks = remain_round / (thread_cnt_ - i);
                    remain_round -= alloc_tasks;
                    if (remain_round > 0)
                        threads.push_back(
                                std::move(std::thread(
                                        std::bind(&RandomRolloutPolicyImpl::thread_runner, this, _1, _2, _3, _4, _5),
                                        alloc_tasks, b, komi, start_player, &result_sum[i]
                                ))
                        );
                }
                std::for_each(threads.begin(), threads.end(), [](std::thread &t) {
                    if (t.joinable())
                        t.join();
                });
                double final_sum = std::accumulate(result_sum.cbegin(), result_sum.cend(), 0);
                return final_sum / rollout_cnt_;
            }
        };
    }

    template<std::size_t W, std::size_t H>
    class RandomRolloutPolicy: public FastRolloutPolicy<W, H>
    {
        detail::RandomRolloutPolicyImpl<W, H> impl;
    public:
        explicit RandomRolloutPolicy(std::size_t rollout_count = 1, std::size_t parallel_threads = 1);
        virtual double run(const board::Board<W, H> &, double komi, board::Player start_player) override ;
        virtual ~RandomRolloutPolicy() override;
    };

    template<std::size_t W, std::size_t H>
    RandomRolloutPolicy<W, H>::RandomRolloutPolicy(size_t rollout_count, size_t parallel_threads):
            impl(rollout_count, parallel_threads)
    {}

    template<std::size_t W, std::size_t H>
    double RandomRolloutPolicy<W, H>::run(const board::Board<W, H> &b, double komi, board::Player start_player)
    {
        return impl.run(b, komi, start_player);
    }

    template<std::size_t W, std::size_t H>
    RandomRolloutPolicy<W, H>::~RandomRolloutPolicy()
    {}

    extern template class RandomRolloutPolicy<5, 5>;
    extern template class RandomRolloutPolicy<9, 9>;
    extern template class RandomRolloutPolicy<19, 19>;
}
#endif