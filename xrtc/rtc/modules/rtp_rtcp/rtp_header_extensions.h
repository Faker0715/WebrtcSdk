#ifndef XRTCSDK_XRTC_RTC_MODULES_RTP_RTCP_RTP_HEADER_EXTENSIONS_H_
#define XRTCSDK_XRTC_RTC_MODULES_RTP_RTCP_RTP_HEADER_EXTENSIONS_H_

#include <api/rtp_parameters.h>

#include "xrtc/rtc/modules/rtp_rtcp/rtp_rtcp_defines.h"

namespace xrtc {

class TransportSequenceNumber {
public:
    static const RTPExtensionType kId = kRtpExtensionTransportSequenceNumber;
    static const size_t kValueSizeBytes = 2;
    static const absl::string_view Uri() {
        return webrtc::RtpExtension::kTransportSequenceNumberUri;
    }
};

} // namespace xrtc

#endif // XRTCSDK_XRTC_RTC_MODULES_RTP_RTCP_RTP_HEADER_EXTENSIONS_H_