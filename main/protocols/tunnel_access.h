#pragma once

#include <string>
#include <vector>

struct TunnelAccess {
    std::string url;
    std::string token;
    std::string client_id;
    std::string client_secret;
};

struct NamedHeader {
    std::string name;
    std::string value;
};

std::string FirstNonEmpty(const std::string& preferred, const std::string& fallback);
std::string HostOf(const std::string& url);
bool SameTunnelHost(const std::string& left_url, const std::string& right_url);
std::vector<NamedHeader> TunnelHandshakeHeaders(const TunnelAccess& access);
std::vector<NamedHeader> OtaTunnelHeaders(const TunnelAccess& access, const std::string& ota_url);
