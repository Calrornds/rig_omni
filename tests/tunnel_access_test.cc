#include "tunnel_access.h"

#include <iostream>
#include <string>

namespace {

int failures = 0;

void Expect(bool condition, const char* name) {
    if (condition) {
        std::cout << "ok " << name << "\n";
        return;
    }
    std::cout << "FAIL " << name << "\n";
    failures++;
}

bool HasHeader(const std::vector<NamedHeader>& headers, const std::string& name) {
    for (const auto& header : headers) {
        if (header.name == name) {
            return true;
        }
    }
    return false;
}

const std::string* HeaderValue(const std::vector<NamedHeader>& headers, const std::string& name) {
    for (const auto& header : headers) {
        if (header.name == name) {
            return &header.value;
        }
    }
    return nullptr;
}

bool SameHeaders(const std::vector<NamedHeader>& left, const std::vector<NamedHeader>& right) {
    if (left.size() != right.size()) {
        return false;
    }
    for (size_t i = 0; i < left.size(); i++) {
        if (left[i].name != right[i].name || left[i].value != right[i].value) {
            return false;
        }
    }
    return true;
}

}  // namespace

int main() {
    TunnelAccess bare{"wss://bridge.example/ws", "", "", ""};
    auto no_cf = TunnelHandshakeHeaders(bare);
    Expect(!HasHeader(no_cf, "CF-Access-Client-Id"), "empty id and secret omit CF-Access-Client-Id");
    Expect(!HasHeader(no_cf, "CF-Access-Client-Secret"), "empty id and secret omit CF-Access-Client-Secret");
    Expect(no_cf.empty(), "empty token id and secret emit no headers");

    TunnelAccess bearer{"wss://bridge.example/ws", "abc", "", ""};
    auto bearer_headers = TunnelHandshakeHeaders(bearer);
    const std::string* bearer_value = HeaderValue(bearer_headers, "Authorization");
    Expect(bearer_headers.size() == 1 && bearer_value != nullptr && *bearer_value == "Bearer abc",
           "token abc becomes Authorization Bearer abc");

    TunnelAccess already{"wss://bridge.example/ws", "Bearer abc", "", ""};
    auto already_headers = TunnelHandshakeHeaders(already);
    const std::string* already_value = HeaderValue(already_headers, "Authorization");
    Expect(already_value != nullptr && *already_value == "Bearer abc",
           "token Bearer abc stays Bearer abc");

    TunnelAccess full{"wss://bridge.example/ws", "abc", "client-id", "client-secret"};
    auto full_headers = TunnelHandshakeHeaders(full);
    Expect(full_headers.size() == 3, "token id and secret emit three headers");
    Expect(full_headers.size() == 3 && full_headers[0].name == "Authorization" && full_headers[0].value == "Bearer abc",
           "Authorization is first");
    Expect(full_headers.size() == 3 && full_headers[1].name == "CF-Access-Client-Id" && full_headers[1].value == "client-id",
           "CF-Access-Client-Id is present");
    Expect(full_headers.size() == 3 && full_headers[2].name == "CF-Access-Client-Secret" &&
               full_headers[2].value == "client-secret",
           "CF-Access-Client-Secret is present");

    auto other_host = OtaTunnelHeaders(full, "https://xl-api.luwudynamics.ai/xiaolu/ota/");
    Expect(other_host.empty(), "ota host that differs from the bridge gets no tunnel headers");

    auto same_host = OtaTunnelHeaders(full, "https://bridge.example/xiaolu/ota/");
    Expect(SameHeaders(same_host, full_headers), "matching hosts ignore scheme and path");

    TunnelAccess secret_only{"wss://bridge.example/ws", "", "", "client-secret"};
    auto secret_headers = TunnelHandshakeHeaders(secret_only);
    Expect(secret_headers.size() == 1 && secret_headers[0].name == "CF-Access-Client-Secret",
           "secret without token or id emits only CF-Access-Client-Secret");

    Expect(HostOf("https://user:pass@Example.COM:443/x") == "example.com",
           "HostOf strips scheme userinfo port and path and lowercases");
    Expect(HostOf("https://[2001:db8::1]/ota") == "2001:db8:0:0:0:0:0:1",
           "HostOf keeps an IPv6 literal");
    Expect(SameTunnelHost("wss://bridge.example:443/ws", "https://bridge.example/ota"),
           "ports do not split one host");
    Expect(HostOf("https://bridge.example/xiaolu/ota/?e=dev@other.example") == "bridge.example",
           "an at sign in the query is not the host");
    Expect(OtaTunnelHeaders(full, "https://other.example/xiaolu/ota/?e=dev@bridge.example").empty(),
           "an at sign in another host query does not attach tunnel headers");
    Expect(HostOf("https://bridge.example#/xiaolu/ota/") == "bridge.example",
           "a fragment is not part of the host");
    Expect(SameTunnelHost("wss://[2001:db8::1]:443/ws",
                          "https://[2001:0db8:0000:0000:0000:0000:0000:0001]/xiaolu/ota/"),
           "compressed and expanded IPv6 are one host");
    Expect(SameTunnelHost("wss://[::1]:443/ws", "https://[0:0:0:0:0:0:0:1]/ota/"),
           "IPv6 loopback spellings are one host");
    Expect(FirstNonEmpty("", "fallback") == "fallback" && FirstNonEmpty("preferred", "fallback") == "preferred",
           "FirstNonEmpty keeps the first non-empty string");

    if (failures != 0) {
        std::cout << failures << " failed\n";
        return 1;
    }
    std::cout << "tunnel_access_test passed\n";
    return 0;
}
