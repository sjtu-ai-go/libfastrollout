#include <gtest/gtest.h>
#include "fastrollout/fastrollout.hpp"
#include <utility>
#include <vector>
#include <board.hpp>

using GraphItem = std::pair<int, board::Player>;
const GraphItem O = GraphItem(0, board::Player::B);
// Convert
// 1 2
// 4 3
// to (0, 0), (0, 1), (1, 1), (1, 0)
template<std::size_t W, std::size_t H, std::size_t ArrW, std::size_t ArrH>
std::vector<std::pair<board::GridPoint<W, H>, board::Player>> graphToPoint(GraphItem (&arr)[ArrW][ArrH])
        {
                static_assert(ArrW <= W && ArrH <= H, "Size of array must <= size of (W, H)");
                std::map<int, std::pair<board::GridPoint<W, H>, board::Player>> m;
                for (std::size_t i=0; i<ArrW; ++i)
                for (std::size_t j=0; j<ArrH; ++j)
                {
                    if (arr[i][j] != O)
                        m.insert(
                                std::make_pair(arr[i][j].first,
                                               std::make_pair(board::GridPoint<W, H>(i, j), arr[i][j].second)
                                ));
                }
                std::vector<std::pair<board::GridPoint<W, H>, board::Player>> ans;
                std::for_each(m.cbegin(), m.cend(), [&ans](typename decltype(m)::value_type v) {
                    ans.push_back(v.second);
                });
                return ans;
        };

std::pair<int, board::Player> operator"" _w(unsigned long long ord)
{
    return std::make_pair(ord, board::Player::W);
};

std::pair<int, board::Player> operator"" _b(unsigned long long ord)
{
    return std::make_pair(ord, board::Player::B);
};

TEST(RandomRolloutTest, TestRandomRollout1)
{
    using namespace board;

    Board<5, 5> b;
    using BT = Board<5, 5>;
    using PT = typename BT::PointType;
    GraphItem graph[5][5] = {
            {O, 1_b, 2_w, 3_w, O},
            {O, 4_b, 5_w, O, 6_w},
            {7_b, 8_b, 9_b, 10_w, 11_w},
            {O, 12_b, O, 13_b, 14_b},
            {O, O, 15_b, 16_w, O}
    };
    auto points = graphToPoint<5, 5>(graph);
    std::for_each(points.begin(), points.end(), [&](std::pair<board::GridPoint<5, 5>, board::Player> item) {
        b.place(item.first, item.second);
    });
    fastrollout::RandomRolloutPolicy<5, 5> p(50, 2);
    EXPECT_LT(p.run(b, 2.5, Player::B), 0.5);
}