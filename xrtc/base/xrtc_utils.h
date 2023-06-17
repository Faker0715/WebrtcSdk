//
// Created by faker on 2023/6/17.
//

#ifndef XRTCSDK_XRTC_UTILS_H
#define XRTCSDK_XRTC_UTILS_H
#include <string>
#include <map>
namespace xrtc {

    bool ParseUrl(const std::string& url,
                  std::string& protocol,
                  std::string& host,
                  std::string& action,
                  std::map<std::string, std::string>& request_params);

}

#endif //XRTCSDK_XRTC_UTILS_H
