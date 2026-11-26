#pragma once
#include "display.h"
#include <memory>
#include <string>
class Http {
public:
    void SetHeader(const char*, const char*) {}
    void SetHeader(const char*, const std::string&) {}
    bool Open(const char*, const std::string&) { return false; }
    int Write(const char*, size_t) { return 0; }
    int GetStatusCode() { return 0; }
    std::string ReadAll() { return ""; }
    void Close() {}
};
class Network {
public:
    std::unique_ptr<Http> CreateHttp(int) { return std::unique_ptr<Http>(new Http()); }
};
class Board {
public:
    static Board& GetInstance() {
        static Board board;
        return board;
    }
    Display* GetDisplay() { return nullptr; }
    std::string GetUuid() { return "uuid"; }
    Network* GetNetwork() {
        static Network network;
        return &network;
    }
};
