#ifndef XRTCSDK_XRTC_RTC_MODULES_RTP_RTCP_RTP_HEADER_EXTENSION_MAP_H_
#define XRTCSDK_XRTC_RTC_MODULES_RTP_RTCP_RTP_HEADER_EXTENSION_MAP_H_

#include "xrtc/rtc/modules/rtp_rtcp/rtp_rtcp_defines.h"

#include <absl/strings/string_view.h>

namespace xrtc {

class RtpHeaderExtensionMap {
public:
    static const uint8_t kInvalidId = 0;

    RtpHeaderExtensionMap();

    bool RegisterUri(int id, absl::string_view uri);
    RTPExtensionType GetType(int id) const;
    uint8_t GetId(RTPExtensionType type) const;

private:
    bool Register(int id, RTPExtensionType type, absl::string_view uri);

private:
    uint8_t ids_[kRtpExtensionNumberOfExtensions];
};

} // namespace xrtc

#endif // XRTCSDK_XRTC_RTC_MODULES_RTP_RTCP_RTP_HEADER_EXTENSION_MAP_H_