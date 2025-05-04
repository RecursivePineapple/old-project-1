#pragma once

namespace error {
constexpr int Success = 0;
struct Rest {
    static constexpr int NotJson = 10000;
    static constexpr int MissingFields = 10001;
    static constexpr int InvalidFormat = 10002;
};

struct CreatePlayer {
    static constexpr int UnknownError = 20000;
    static constexpr int InsertionFailure = 20001;
};

struct PlayerAuth {
    static constexpr int UnknownError = 30000;
    static constexpr int InvalidCredentials = 30001;
    static constexpr int ConcurrentSessionStart = 30002;
};

struct Session {
    static constexpr int UnknownError = 40000;
    static constexpr int InvalidFormat = 40001;
    static constexpr int MissingSession = 40002;
    static constexpr int InvalidSession = 40003;
    static constexpr int InvalidSessionClient = 40004;
    static constexpr int SessionAlreadyRunning = 40005;
    static constexpr int AddrFetchTimeout = 40006;
    static constexpr int AddrFetchError = 40007;
};

const char* GetErrorString(int code);
}
