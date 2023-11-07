#include "xrtc/rtc/modules/rtp_rtcp/rtp_header_extensions.h"

#include <modules/rtp_rtcp/source/byte_io.h>

namespace xrtc {

    bool TransportSequenceNumber::Write(rtc::ArrayView<uint8_t> data,
                                        uint16_t transport_sequence_number)
    {
        webrtc::ByteWriter<uint16_t>::WriteBigEndian(data.data(),
                                                     transport_sequence_number);
        return true;
    }

} // namespace xrtc