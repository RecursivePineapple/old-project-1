#pragma target server

#include "Common/ErrorCodes.hpp"

namespace error {

const char* GetErrorString(int code)
{
    switch(code)
    {
        default: {
            return nullptr;
        }
        case ::error::Success: {
            return "Success";
        }
        case ::error::Rest::NotJson: {
            return "A protocol error occurred";
        }
        case ::error::Rest::MissingFields: {
            return "A protocol error occurred";
        }
        case ::error::Rest::InvalidFormat: {
            return "A protocol error occurred";
        }
        case ::error::CreatePlayer::UnknownError: {
            return "An unknown error occured while creating player";
        }
        case ::error::CreatePlayer::InsertionFailure: {
            return "Could not create player due to unknown error";
        }
        case ::error::PlayerAuth::UnknownError: {
            return "Unknown error while authenticating";
        }
        case ::error::PlayerAuth::InvalidCredentials: {
            return "Invalid credentials";
        }
        case ::error::PlayerAuth::ConcurrentSessionStart: {
            return "Unknown error while authenticating";
        }
        case ::error::Session::UnknownError: {
            return "Unknown error while loading session";
        }
        case ::error::Session::InvalidFormat: {
            return "Unknown error while loading session";
        }
        case ::error::Session::MissingSession: {
            return "Session does not exist, or is invalid";
        }
        case ::error::Session::InvalidSession: {
            return "Session does not exist, or is invalid";
        }
        case ::error::Session::InvalidSessionClient: {
            return "Session does not exist, or is invalid";
        }
        case ::error::Session::SessionAlreadyRunning: {
            return "Session does not exist, or is invalid";
        }
        case ::error::Session::AddrFetchTimeout: {
            return "Unknown error while loading session";
        }
        case ::error::Session::AddrFetchError: {
            return "Unknown error while loading session";
        }
    }
}
}
