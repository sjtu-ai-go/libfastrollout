//
// Created by lz on 12/25/16.
//

#ifndef FASTROLLOUT_CNN_V1_HPP
#define FASTROLLOUT_CNN_V1_HPP

#include "message.pb.h"
#include "logger.hpp"
#include <boost/asio.hpp>
#include <string>
#include <cstdint>
#include <vector>
#include <algorithm>

namespace fastrollout
{
    namespace detail
    {
        using namespace boost::asio;
        class CNNFastRolloutServiceBase
        {
        protected:
            std::shared_ptr<spdlog::logger> logger = {getGlobalLogger()};
            io_service service;
            ip::tcp::endpoint ep;

            ip::tcp::socket sock;

            static void config_socket(ip::tcp::socket &sock)
            {
                static ip::tcp::socket::reuse_address ra(true);
                static ip::tcp::no_delay nd(true);
                sock.set_option(ra);
                sock.set_option(nd);
            }
        public:
            CNNFastRolloutServiceBase(const std::string &addr, unsigned short port):
                    ep(ip::address::from_string(addr), port), sock(service)
            {
                logger->trace("Start fastrollout service");
                sock.connect(ep);
                config_socket(sock);
            }

            ~CNNFastRolloutServiceBase()
            {
                logger->trace("Stop fastrollout service");
                const std::int64_t len = 0;
                sock.write_some(buffer(&len, 8));
                sock.close();
            }

            std::string sync_call(const std::string &message)
            {
                logger->trace("Start a call");

                std::int64_t len = message.size();

                logger->trace("Start writing len");
                sock.write_some(buffer(&len, 8));
                logger->trace("Start writing msg");
                sock.write_some(buffer(message));

                std::int64_t resp_len = 0;
                logger->trace("Start reading resp len");
                sock.read_some(buffer(&resp_len, 8));
                std::vector<char> result(resp_len);
                logger->trace("Start read msg with len {}", resp_len);
                sock.read_some(buffer(result, resp_len));
                std::string result_s;
                std::copy(result.cbegin(), result.cend(), std::back_inserter(result_s));
                return result_s;
            }
        };

        class RequestV1Service: protected CNNFastRolloutServiceBase
        {
        public:
            RequestV1Service(const std::string &addr, unsigned short port):
                    CNNFastRolloutServiceBase(addr, port)
            {}

            gocnn::ResponseV1 sync_call(const gocnn::RequestV1 &reqV1)
            {
                std::string resp = CNNFastRolloutServiceBase::sync_call(reqV1.SerializeAsString());
                gocnn::ResponseV1 respV1;
                respV1.ParseFromString(resp);
                return respV1;
            }
        };

        class RequestV2Service: protected CNNFastRolloutServiceBase
        {
        public:
            RequestV2Service(const std::string &addr, unsigned short port):
                    CNNFastRolloutServiceBase(addr, port)
            {}

            gocnn::ResponseV2 sync_call(const gocnn::RequestV2 &reqV2)
            {
                auto pair = std::get_temporary_buffer<char>(38 * reqV2.board_size() + 4 * reqV2.board_size());
                auto pchar = pair.first;
                std::copy(reqV2.stone_color_our().begin(), reqV2.stone_color_our().end(), pchar);
                std::copy(reqV2.stone_color_oppo().begin(), reqV2.stone_color_oppo().end(), pchar + reqV2.board_size());
                std::copy(reqV2.stone_color_empty().begin(), reqV2.stone_color_empty().end(), pchar + 2 * reqV2.board_size());

                std::copy(reqV2.turns_since_one().begin(), reqV2.turns_since_one().end(), pchar + 3 * reqV2.board_size());
                std::copy(reqV2.turns_since_two().begin(), reqV2.turns_since_two().end(), pchar + 4 * reqV2.board_size());
                std::copy(reqV2.turns_since_three().begin(), reqV2.turns_since_three().end(), pchar + 5 * reqV2.board_size());
                std::copy(reqV2.turns_since_four().begin(), reqV2.turns_since_four().end(), pchar + 6 * reqV2.board_size());
                std::copy(reqV2.turns_since_five().begin(), reqV2.turns_since_five().end(), pchar + 7 * reqV2.board_size());
                std::copy(reqV2.turns_since_six().begin(), reqV2.turns_since_six().end(), pchar + 8 * reqV2.board_size());
                std::copy(reqV2.turns_since_seven().begin(), reqV2.turns_since_seven().end(), pchar + 9 * reqV2.board_size());
                std::copy(reqV2.turns_since_more().begin(), reqV2.turns_since_more().end(), pchar + 10 * reqV2.board_size());

                std::copy(reqV2.liberties_our_one().begin(), reqV2.liberties_our_one().end(), pchar + 11 * reqV2.board_size());
                std::copy(reqV2.liberties_our_two().begin(), reqV2.liberties_our_two().end(), pchar + 12 * reqV2.board_size());
                std::copy(reqV2.liberties_our_three().begin(), reqV2.liberties_our_three().end(), pchar + 13 * reqV2.board_size());
                std::copy(reqV2.liberties_our_more().begin(), reqV2.liberties_our_more().end(), pchar + 14 * reqV2.board_size());

                std::copy(reqV2.liberties_oppo_one().begin(), reqV2.liberties_oppo_one().end(), pchar + 15 * reqV2.board_size());
                std::copy(reqV2.liberties_oppo_two().begin(), reqV2.liberties_oppo_two().end(), pchar + 16 * reqV2.board_size());
                std::copy(reqV2.liberties_oppo_three().begin(), reqV2.liberties_oppo_three().end(), pchar + 17 * reqV2.board_size());
                std::copy(reqV2.liberties_oppo_more().begin(), reqV2.liberties_oppo_more().end(), pchar + 18 * reqV2.board_size());

                std::copy(reqV2.capture_size_one().begin(), reqV2.capture_size_one().end(), pchar + reqV2.board_size() * 19);
                std::copy(reqV2.capture_size_two().begin(), reqV2.capture_size_two().end(), pchar + 20 * reqV2.board_size());
                std::copy(reqV2.capture_size_three().begin(), reqV2.capture_size_three().end(), pchar + 21 * reqV2.board_size());
                std::copy(reqV2.capture_size_four().begin(), reqV2.capture_size_four().end(), pchar + 22 * reqV2.board_size());
                std::copy(reqV2.capture_size_five().begin(), reqV2.capture_size_five().end(), pchar + 23 * reqV2.board_size());
                std::copy(reqV2.capture_size_six().begin(), reqV2.capture_size_six().end(), pchar + 24 * reqV2.board_size());
                std::copy(reqV2.capture_size_seven().begin(), reqV2.capture_size_seven().end(), pchar + 25 * reqV2.board_size());
                std::copy(reqV2.capture_size_more().begin(), reqV2.capture_size_more().end(), pchar + 26 * reqV2.board_size());

                std::copy(reqV2.self_atari_one().begin(), reqV2.self_atari_one().end(), pchar + 27 * reqV2.board_size());
                std::copy(reqV2.self_atari_two().begin(), reqV2.self_atari_two().end(), pchar + 28 * reqV2.board_size());
                std::copy(reqV2.self_atari_three().begin(), reqV2.self_atari_three().end(), pchar + 29 * reqV2.board_size());
                std::copy(reqV2.self_atari_four().begin(), reqV2.self_atari_four().end(), pchar + 30 * reqV2.board_size());
                std::copy(reqV2.self_atari_five().begin(), reqV2.self_atari_five().end(), pchar + 31 * reqV2.board_size());
                std::copy(reqV2.self_atari_six().begin(), reqV2.self_atari_six().end(), pchar + 32 * reqV2.board_size());
                std::copy(reqV2.self_atari_seven().begin(), reqV2.self_atari_seven().end(), pchar + 33 * reqV2.board_size());
                std::copy(reqV2.self_atari_more().begin(), reqV2.self_atari_more().end(), pchar + 34 * reqV2.board_size());

                std::copy(reqV2.sensibleness().begin(), reqV2.sensibleness().end(), pchar + 35 * reqV2.board_size());
                std::copy(reqV2.ko().begin(), reqV2.ko().end(), pchar + 36 * reqV2.board_size());
                std::copy(reqV2.border().begin(), reqV2.border().end(), pchar + 37 * reqV2.board_size());

                float *start = (float *)(pchar + 38 * reqV2.board_size());
                std::copy(reqV2.position().begin(), reqV2.position().end(), start);

                std::string s;
                std::copy(pchar, pchar + 38 * reqV2.board_size() + 4 * reqV2.board_size(), std::back_inserter(s));

                std::string resp = CNNFastRolloutServiceBase::sync_call(s);
                std::return_temporary_buffer(pchar);

                gocnn::ResponseV2 respV2;
                respV2.ParseFromString(resp);
                return respV2;
            }
        };
    }
}

#endif //LIBUCT_CNN_V1_HPP
