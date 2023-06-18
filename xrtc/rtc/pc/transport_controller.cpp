//
// Created by faker on 2023/6/18.
//

#include "xrtc/rtc/pc/transport_controller.h"

#include "xrtc/base/xrtc_global.h"
#include "xrtc/rtc/pc/session_description.h"
//#include "xrtc/rtc/modules/rtp_rtcp/rtp_utils.h"

namespace xrtc {

    TransportController::TransportController() :
            ice_agent_(new ice::IceAgent(XRTCGlobal::Instance()->network_thread(),
                                         XRTCGlobal::Instance()->port_allocator()))
    {
        ice_agent_->SignalIceState.connect(this,
                                           &TransportController::OnIceState);
        ice_agent_->SignalReadPacket.connect(this,
                                             &TransportController::OnReadPacket);
    }

    TransportController::~TransportController() {
        if (ice_agent_) {
            ice_agent_->Destroy();
            ice_agent_ = nullptr;
        }
    }

    int TransportController::SetRemoteSDP(SessionDescription* desc) {
        if (!desc) {
            return -1;
        }

        for (auto content : desc->contents()) {
            std::string mid = content->mid();
            if (desc->IsBundle(mid) && mid != desc->GetFirstBundleId()) {
                continue;
            }

            // 创建ICE transport
            // RTCP, 默认开启a=rtcp:mux
            ice_agent_->CreateIceTransport(mid, 1); // 1: RTP

            // 设置ICE param
            auto td = desc->GetTransportInfo(mid);
            if (td) {
                ice_agent_->SetRemoteIceParams(mid, 1, ice::IceParameters(
                        td->ice_ufrag, td->ice_pwd));
            }

            // 设置ICE candidate
            for (auto candidate : content->candidates()) {
                ice_agent_->AddRemoteCandidate(mid, 1, candidate);
            }
        }

        return 0;
    }

    int TransportController::SetLocalSDP(SessionDescription* desc) {
        if (!desc) {
            return -1;
        }

        for (auto content : desc->contents()) {
            std::string mid = content->mid();
            if (desc->IsBundle(mid) && mid != desc->GetFirstBundleId()) {
                continue;
            }

            auto td = desc->GetTransportInfo(mid);
            if (td) {
                ice_agent_->SetIceParams(mid, 1,
                                         ice::IceParameters(td->ice_ufrag, td->ice_pwd));
            }
        }

        ice_agent_->GatheringCandidate();

        return 0;
    }

    int TransportController::SendPacket(const std::string& transport_name,
                                        const char* data, size_t len)
    {
        return ice_agent_->SendPacket(transport_name, 1, data, len);
    }

    void TransportController::OnIceState(ice::IceAgent*,
                                         ice::IceTransportState ice_state)
    {
        SignalIceState(this, ice_state);
    }

    void TransportController::OnReadPacket(ice::IceAgent*, const std::string&, int,
                                           const char* data, size_t len, int64_t ts)
    {
//        auto array_view = rtc::MakeArrayView<const uint8_t>((const uint8_t*)data, len);
//        RtpPacketType packet_type = InferRtpPacketType(array_view);
//        if (RtpPacketType::kUnknown == packet_type) {
//            return;
//        }
//
//        if (RtpPacketType::kRtcp == packet_type) {
//            SignalRtcpPacketReceived(this, data, len, ts);
//        }
//        else if (RtpPacketType::kRtp == packet_type) {
//            SignalRtpPacketReceived(this, data, len, ts);
//        }
    }

} // namespace xrtc

