//
// Created by lz on 12/27/16.
//

#include <gtest/gtest.h>
#include "fastrollout/fastrollout_cnn.hpp"
#include "board.hpp"

TEST(FastrolloutCNNTest, DISABLED_TestFastrolloutCNNRemote)
{
    board::Board<19, 19> b;
    fastrollout::detail::RequestV2Service reqV2Service("127.0.0.1", 7598);

    board::Player cur_player = board::Player::B;
    std::vector<std::pair<board::GridPoint<19, 19>, double>> vPointW, vPointB;
    int age_w = 100, age_b = 100;
    for (;;)
    {
        using PT = board::GridPoint<19, 19>;
        using BT = board::Board<19, 19>;


        auto &vPoint = cur_player == board::Player::B ? vPointB : vPointW;
        auto &age = cur_player == board::Player::B ? age_b : age_w;

        if (age > 10) {
            auto reqV2 = b.generateRequestV2(cur_player);
            gocnn::ResponseV2 resp = reqV2Service.sync_call(reqV2);
            EXPECT_EQ(19 * 19, resp.board_size());

            vPoint.clear();
            for (int i = 0; i < resp.possibility_size(); ++i)
                vPoint.emplace_back(board::GridPoint<19, 19>(i / 19, i % 19), resp.possibility(i));
            std::sort(vPoint.begin(), vPoint.end(), [](std::pair<PT, double> a, std::pair<PT, double> b) {
                return a.second > b.second;
            });
            age = 0;
        }

        ++age;
        bool placed = false;
        for (auto &pair: vPoint)
            if (b.getPosStatus(pair.first, cur_player) == BT::PositionStatus::OK &&
                    !b.isTrueEye(pair.first, cur_player) && !b.isSelfAtari(pair.first, cur_player))
            {
                placed = true;
                b.place(pair.first, cur_player);
                break;
            }
        if (!placed)
            break;

        cur_player = board::getOpponentPlayer(cur_player);
    }
}
