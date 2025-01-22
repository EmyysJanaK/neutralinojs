#ifndef NEU_ROUTER_H
#define NEU_ROUTER_H

#include <string>

#include <websocketpp/server.hpp>

#include "lib/json/json.hpp"

using namespace std;
using json = nlohmann::json;

namespace router {

typedef json (*NativeMethod)(const json &);

struct Response {
    websocketpp::http::status_code::value status = websocketpp::http::status_code::ok;
    string contentType = "application/octet-stream";
    string data;
};

struct NativeMessage {
    string id;
    string method;
    string accessToken;
    json data;
};

inline bool operator==(const NativeMessage &lhs, const NativeMessage &rhs) {
    return lhs.id == rhs.id && lhs.method == rhs.method && lhs.accessToken == rhs.accessToken && lhs.data == rhs.data;
}

router::Response serve(string path);
router::NativeMessage executeNativeMethod(const router::NativeMessage &request);
router::Response getAsset(string path, const string &prependData = "");
map<string, router::NativeMethod> getMethodMap();

} // namespace router

#endif // #define NEU_ROUTER_H
