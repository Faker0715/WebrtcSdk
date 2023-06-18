//
// Created by faker on 2023/6/18.
//

#ifndef XRTCSDK_STREAM_PARAMS_H
#define XRTCSDK_STREAM_PARAMS_H
#include <string>
#include <vector>

namespace xrtc {

    struct SsrcGroup {
        std::string semantics;
        std::vector<uint32_t> ssrcs;
    };

    struct StreamParams {
        std::string id;
        std::string stream_id;
        std::string cname;
        std::vector<uint32_t> ssrcs;
        std::vector<SsrcGroup> ssrc_groups;
    };

}
#endif //XRTCSDK_STREAM_PARAMS_H
