#include "tunnel_access.h"

namespace {

bool ParseHextet(const std::string& text, uint16_t& value) {
    if (text.empty() || text.size() > 4) {
        return false;
    }
    value = 0;
    for (char c : text) {
        value = static_cast<uint16_t>(value << 4);
        if (c >= '0' && c <= '9') {
            value = static_cast<uint16_t>(value + (c - '0'));
        } else if (c >= 'a' && c <= 'f') {
            value = static_cast<uint16_t>(value + (c - 'a' + 10));
        } else if (c >= 'A' && c <= 'F') {
            value = static_cast<uint16_t>(value + (c - 'A' + 10));
        } else {
            return false;
        }
    }
    return true;
}

bool SplitHextets(const std::string& part, uint16_t* out, size_t& count) {
    count = 0;
    if (part.empty()) {
        return true;
    }
    size_t start = 0;
    while (start <= part.size()) {
        auto colon = part.find(':', start);
        std::string piece = part.substr(start, colon == std::string::npos ? std::string::npos : colon - start);
        if (count >= 8 || !ParseHextet(piece, out[count])) {
            return false;
        }
        count++;
        if (colon == std::string::npos) {
            return true;
        }
        start = colon + 1;
        if (start > part.size()) {
            return false;
        }
    }
    return false;
}

std::string CanonicalIpv6(const std::string& host) {
    if (host.find(':') == std::string::npos) {
        return host;
    }
    auto gap = host.find("::");
    if (gap != std::string::npos && host.find("::", gap + 2) != std::string::npos) {
        return host;
    }
    std::string left = gap == std::string::npos ? host : host.substr(0, gap);
    std::string right = gap == std::string::npos ? std::string() : host.substr(gap + 2);
    uint16_t head[8] = {};
    uint16_t tail[8] = {};
    size_t head_count = 0;
    size_t tail_count = 0;
    if (!SplitHextets(left, head, head_count) || !SplitHextets(right, tail, tail_count)) {
        return host;
    }
    uint16_t full[8] = {};
    if (gap == std::string::npos) {
        if (head_count != 8) {
            return host;
        }
        for (size_t i = 0; i < 8; i++) {
            full[i] = head[i];
        }
    } else if (head_count + tail_count > 7) {
        return host;
    } else {
        size_t index = 0;
        for (size_t i = 0; i < head_count; i++) {
            full[index++] = head[i];
        }
        index += 8 - head_count - tail_count;
        for (size_t i = 0; i < tail_count; i++) {
            full[index++] = tail[i];
        }
    }
    std::string out;
    for (size_t i = 0; i < 8; i++) {
        if (i != 0) {
            out.push_back(':');
        }
        uint16_t value = full[i];
        char digits[4];
        int n = 0;
        if (value == 0) {
            digits[n++] = '0';
        } else {
            while (value > 0) {
                int d = value & 0xf;
                digits[n++] = static_cast<char>(d < 10 ? '0' + d : 'a' + (d - 10));
                value = static_cast<uint16_t>(value >> 4);
            }
        }
        while (n > 0) {
            out.push_back(digits[--n]);
        }
    }
    return out;
}

}  // namespace

std::string FirstNonEmpty(const std::string& preferred, const std::string& fallback) {
    if (!preferred.empty()) {
        return preferred;
    }
    return fallback;
}

std::string HostOf(const std::string& url) {
    std::string rest = url;
    auto scheme = rest.find("://");
    if (scheme != std::string::npos) {
        rest = rest.substr(scheme + 3);
    }
    size_t authority_end = rest.size();
    for (size_t i = 0; i < rest.size(); i++) {
        char c = rest[i];
        if (c == '/' || c == '?' || c == '#') {
            authority_end = i;
            break;
        }
    }
    std::string authority = rest.substr(0, authority_end);
    auto at = authority.rfind('@');
    if (at != std::string::npos) {
        authority = authority.substr(at + 1);
    }
    if (!authority.empty() && authority.front() == '[') {
        auto end = authority.find(']');
        if (end != std::string::npos) {
            authority = authority.substr(1, end - 1);
        }
    } else {
        auto colon = authority.rfind(':');
        if (colon != std::string::npos) {
            authority = authority.substr(0, colon);
        }
    }
    for (char& c : authority) {
        if (c >= 'A' && c <= 'Z') {
            c = static_cast<char>(c - 'A' + 'a');
        }
    }
    return CanonicalIpv6(authority);
}

bool SameTunnelHost(const std::string& left_url, const std::string& right_url) {
    std::string left = HostOf(left_url);
    std::string right = HostOf(right_url);
    return !left.empty() && left == right;
}

namespace {

void AddHeader(std::vector<NamedHeader>& headers, const std::string& name, const std::string& value) {
    if (value.empty()) {
        return;
    }
    headers.push_back(NamedHeader{name, value});
}

std::string AuthorizationValue(const std::string& token) {
    if (token.empty()) {
        return "";
    }
    if (token.find(' ') == std::string::npos) {
        return "Bearer " + token;
    }
    return token;
}

}  // namespace

std::vector<NamedHeader> TunnelHandshakeHeaders(const TunnelAccess& access) {
    std::vector<NamedHeader> headers;
    AddHeader(headers, "Authorization", AuthorizationValue(access.token));
    AddHeader(headers, "CF-Access-Client-Id", access.client_id);
    AddHeader(headers, "CF-Access-Client-Secret", access.client_secret);
    return headers;
}

std::vector<NamedHeader> OtaTunnelHeaders(const TunnelAccess& access, const std::string& ota_url) {
    if (!SameTunnelHost(access.url, ota_url)) {
        return {};
    }
    return TunnelHandshakeHeaders(access);
}
