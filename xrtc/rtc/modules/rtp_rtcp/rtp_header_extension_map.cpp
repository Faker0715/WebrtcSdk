#include "xrtc/rtc/modules/rtp_rtcp/rtp_header_extension_map.h"

#include <absl/strings/string_view.h>
#include <rtc_base/arraysize.h>
#include <rtc_base/logging.h>

#include "xrtc/rtc/modules/rtp_rtcp/rtp_header_extensions.h"

namespace xrtc {

namespace {

struct ExtensionInfo {
    RTPExtensionType type;
    absl::string_view uri;
};

template <typename Extension>
const ExtensionInfo CreateExtensionInfo() {
    return { Extension::kId, Extension::Uri() };
}

const ExtensionInfo kExtensions[] = {
    CreateExtensionInfo<TransportSequenceNumber>(),
};

static_assert(arraysize(kExtensions) ==
    static_cast<int>(kRtpExtensionNumberOfExtensions) - 1,
    "kExtensions expect to list all known extensions");

} // namespace

RtpHeaderExtensionMap::RtpHeaderExtensionMap() {
    for (auto& id : ids_) {
        id = kInvalidId;
    }
}

bool RtpHeaderExtensionMap::RegisterUri(int id, absl::string_view uri) {
    // 注册的uri，必须是我们已经支持的头部扩展的uri
    for (const ExtensionInfo& extension : kExtensions) {
        if (extension.uri == uri) {
            return Register(id, extension.type, extension.uri);
        }
    }

    RTC_LOG(LS_WARNING) << "unknown extension uri: " << uri
        << ", id: " << id;
    return false;
}

RTPExtensionType RtpHeaderExtensionMap::GetType(int id) const {
    for (int type = kRtpExtensionNone; type < kRtpExtensionNumberOfExtensions;
        ++type) 
    {
        if (ids_[type] == id) {
            return static_cast<RTPExtensionType>(type);
        }
    }

    return kRtpExtensionNone;
}

uint8_t RtpHeaderExtensionMap::GetId(RTPExtensionType type) const {
    return ids_[type];
}

bool RtpHeaderExtensionMap::Register(int id, RTPExtensionType type, 
    absl::string_view uri) 
{
    // id必须是在[1, 255]范围内
    if (id < webrtc::RtpExtension::kMinId || id > webrtc::RtpExtension::kMaxId) {
        RTC_LOG(LS_WARNING) << "Failed to register extension uri: " << uri
            << " with invlaid id: " << id;
        return false;
    }

    // 检查id是否已经注册了
    RTPExtensionType registered_type = GetType(id);
    if (registered_type == type) { // 之前已经注册过，不需要重复注册
        RTC_LOG(LS_INFO) << "Extension already registered, uri: " << uri
            << ", id: " << id
            << ", type:" << static_cast<int>(type);
        return true;
    }

    // type已经被其它的id占用了，不能再注册了
    if (registered_type != kRtpExtensionNone) {
        RTC_LOG(LS_WARNING) << "Extension type already used, uri: " << uri
            << ", id: " << id
            << ", type: " << static_cast<int>(type);
        return false;
    }

    ids_[type] = static_cast<uint8_t>(id);

    return true;
}

} // namespace xrtc