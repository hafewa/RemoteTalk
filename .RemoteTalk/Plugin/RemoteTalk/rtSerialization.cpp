#include "pch.h"
#include "rtSerialization.h"
#include "rtTalkInterface.h"
#include "rtTalkServer.h"
#include "picojson/picojson.h"

namespace rt {

void Print(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
#ifdef _WIN32
    char buf[1024];
    vsnprintf(buf, sizeof(buf), fmt, args);
    ::OutputDebugStringA(buf);
#else
    vprintf(fmt, args);
#endif
    va_end(args);
}

void Print(const wchar_t *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
#ifdef _WIN32
    wchar_t buf[1024];
    _vsnwprintf(buf, sizeof(buf), fmt, args);
    ::OutputDebugStringW(buf);
#else
    vwprintf(fmt, args);
#endif
    va_end(args);
}


std::string ToUTF8(const char *src)
{
#ifdef _WIN32
    // to UTF-16
    const int wsize = ::MultiByteToWideChar(CP_ACP, 0, (LPCSTR)src, -1, nullptr, 0);
    std::wstring ws;
    ws.resize(wsize-1);
    ::MultiByteToWideChar(CP_ACP, 0, (LPCSTR)src, -1, (LPWSTR)ws.data(), wsize);

    // to UTF-8
    const int u8size = ::WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)ws.data(), -1, nullptr, 0, nullptr, nullptr);
    std::string u8s;
    u8s.resize(u8size-1);
    ::WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)ws.data(), -1, (LPSTR)u8s.data(), u8size, nullptr, nullptr);
    return u8s;
#else
    return src;
#endif
}
std::string ToUTF8(const std::string& src)
{
    return ToUTF8(src.c_str());
}

std::string ToANSI(const char *src)
{
#ifdef _WIN32
    // to UTF-16
    const int wsize = ::MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)src, -1, nullptr, 0);
    std::wstring ws;
    ws.resize(wsize-1);
    ::MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)src, -1, (LPWSTR)ws.data(), wsize);

    // to ANSI
    const int u8size = ::WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)ws.data(), -1, nullptr, 0, nullptr, nullptr);
    std::string u8s;
    u8s.resize(u8size-1);
    ::WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)ws.data(), -1, (LPSTR)u8s.data(), u8size, nullptr, nullptr);
    return u8s;
#else
    return src;
#endif
}
std::string ToANSI(const std::string& src)
{
    return ToANSI(src.c_str());
}

std::string ToMBS(const wchar_t * src)
{
    using converter_t = std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>;
    return converter_t().to_bytes(src);
}
std::string ToMBS(const std::wstring& src)
{
    return ToMBS(src.c_str());
}

std::wstring ToWCS(const char * src)
{
    using converter_t = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>;
    return converter_t().from_bytes(src);
}
std::wstring ToWCS(const std::string & src)
{
    return ToWCS(src.c_str());
}


template<> std::string to_string(const int& v)
{
    char buf[32];
    sprintf(buf, "%d", v);
    return buf;
}
template<> std::string to_string(const short& v)
{
    return to_string((int)v);
}
template<> std::string to_string(const bool& v)
{
    return to_string((int)v);
}
template<> std::string to_string(const float& v)
{
    char buf[32];
    sprintf(buf, "%.3f", v);
    return buf;
}
template<> std::string to_string(const picojson::value& v)
{
    return v.serialize(true);
}

template<> int from_string(const std::string& v)
{
    return std::atoi(v.c_str());
}
template<> short from_string(const std::string& v)
{
    return (short)from_string<int>(v) != 0;
}
template<> bool from_string(const std::string& v)
{
    return from_string<int>(v) != 0;
}
template<> float from_string(const std::string& v)
{
    return (float)std::atof(v.c_str());
}


using namespace picojson;

template<> picojson::value to_json(const bool& v)
{
    return value(v);
}
template<> bool from_json(bool& dst, const picojson::value& v)
{
    if (!v.is<bool>())
        return false;
    dst = v.get<bool>();
    return true;
}

template<> picojson::value to_json(const short& v)
{
    return value((float)v);
}
template<> bool from_json(short& dst, const picojson::value& v)
{
    if (!v.is<float>())
        return false;
    dst = (short)v.get<float>();
    return true;
}

template<> picojson::value to_json(const uint16_t& v)
{
    return value((float)v);
}
template<> bool from_json(uint16_t& dst, const picojson::value& v)
{
    if (!v.is<float>())
        return false;
    dst = (uint16_t)v.get<float>();
    return true;
}

template<> picojson::value to_json(const int& v)
{
    return value((float)v);
}
template<> bool from_json(int& dst, const picojson::value& v)
{
    if (!v.is<float>())
        return false;
    dst = (int)v.get<float>();
    return true;
}

template<> picojson::value to_json(const float& v)
{
    return value(v);
}
template<> bool from_json(float& dst, const picojson::value& v)
{
    if (!v.is<float>())
        return false;
    dst = v.get<float>();
    return true;
}

template<> picojson::value to_json(const std::string& v)
{
    return value(v);
}
template<> bool from_json(std::string& dst, const picojson::value& v)
{
    if (!v.is<std::string>())
        return false;
    dst = v.get<std::string>();
    return true;
}

template<> picojson::value to_json(const TalkParams& v)
{
    object t;
    t["mute"] = to_json(v.mute);
    t["force_mono"] = to_json(v.force_mono);
    t["cast"] = to_json(v.cast);
    {
        object params;
        for (int i = 0; i < TalkParams::MaxParams; ++i) {
            if(v.isSet(i))
                params[to_string(i)] = value(v[i]);
        }
        t["params"] = value(params);
    }
    return value(std::move(t));
}
template<> bool from_json(TalkParams& dst, const picojson::value& v)
{
    if (!v.is<object>())
        return false;

    const auto& o = v.get<object>();
    int n = 0;
    if (from_json(dst.mute, v.get("mute"))) ++n;
    if (from_json(dst.force_mono, v.get("force_mono"))) ++n;
    if (from_json(dst.cast, v.get("cast"))) ++n;
    {
        auto it = o.find("params");
        if (it != o.end() && it->second.is<object>()) {
            for (auto& kvp : it->second.get<object>()) {
                if (kvp.second.is<float>()) {
                    int idx = from_string<int>(kvp.first);
                    if (idx >= 0 && idx < TalkParams::MaxParams) {
                        float val;
                        if (from_json(val, kvp.second))
                            dst[idx] = val;
                    }
                }
            }
        }
    }
    return n >= 3;
}

template<> picojson::value to_json(const TalkParamInfo& v)
{
    object o;
    o["name"] = to_json(v.name);
    o["value"] = to_json(v.value);
    o["range_min"] = to_json(v.range_min);
    o["range_max"] = to_json(v.range_max);
    return value(std::move(o));
}
template<> bool from_json(TalkParamInfo& dst, const picojson::value& v)
{
    if (!v.is<object>())
        return false;

    int n = 0;
    if (from_json(dst.name, v.get("name"))) ++n;
    if (from_json(dst.value, v.get("value"))) ++n;
    if (from_json(dst.range_min, v.get("range_min"))) ++n;
    if (from_json(dst.range_max, v.get("range_max"))) ++n;
    return n >= 4;
}

template<> picojson::value to_json(const CastInfo& v)
{
    object o;
    o["id"] = to_json(v.id);
    o["name"] = to_json(v.name);

    if (!v.params.empty()) {
        array params;
        for (auto& p : v.params)
            params.push_back(to_json(p));
        o["params"] = value(params);
    }

    return value(std::move(o));
}
template<> bool from_json(CastInfo& dst, const picojson::value& v)
{
    if (!v.is<object>())
        return false;

    auto& o = v.get<object>();
    int n = 0;
    if (from_json(dst.id, v.get("id"))) ++n;
    if (from_json(dst.name, v.get("name"))) ++n;
    {
        auto it = o.find("params");
        if (it != o.end() && it->second.is<array>()) {
            auto& params = it->second.get<array>();
            dst.params.clear();
            for (auto& p : params) {
                TalkParamInfo tmp;
                if (from_json(tmp, p))
                    dst.params.push_back(std::move(tmp));
            }
        }
    }
    return n >= 2;
}

template<> picojson::value to_json(const CastList& v)
{
    array t(v.size());
    for (size_t i = 0; i < v.size(); ++i)
        t[i] = to_json(v[i]);
    return value(std::move(t));
}
template<> bool from_json(CastList& dst, const picojson::value& v)
{
    if (!v.is<array>())
        return false;

    for (auto& e : v.get<array>()) {
        CastInfo ai;
        if(from_json(ai, e))
            dst.push_back(ai);
    }
    return true;
}

template<> picojson::value to_json(const TalkServerStats& v)
{
    object ret;
    ret["host"] = to_json(v.host);
    ret["plugin_version"] = to_json(v.plugin_version);
    ret["protocol_version"] = to_json(v.protocol_version);
    ret["params"] = to_json(v.params);
    ret["casts"] = to_json(v.casts);
    return value(std::move(ret));
}
template<> bool from_json(TalkServerStats& dst, const picojson::value& v)
{
    if (!v.is<object>())
        return false;

    int n = 0;
    if (from_json(dst.host, v.get("host"))) ++n;
    if (from_json(dst.plugin_version, v.get("plugin_version"))) ++n;
    if (from_json(dst.protocol_version, v.get("protocol_version"))) ++n;
    if (from_json(dst.params, v.get("params"))) ++n;
    if (from_json(dst.casts, v.get("casts"))) ++n;
    return n >= 5;
}

template<> picojson::value to_json(const std::map<std::string, std::string>& v)
{
    object t;
    for (auto& kvp : v)
        t[kvp.first] = value(kvp.second);
    return value(std::move(t));
}
template<> bool from_json(std::map<std::string, std::string>& dst, const picojson::value& v)
{
    if (!v.is<object>())
        return false;

    auto& o = v.get<object>();
    for (auto& kvp : o) {
        if (kvp.second.is<std::string>())
            dst[kvp.first] = kvp.second.get<std::string>();
    }
    return true;
}

template<> picojson::value to_json(const TalkServerSettings& v)
{
    object ret;
    ret["port"] = to_json(v.port);
    ret["max_queue"] = to_json(v.max_queue);
    ret["max_threads"] = to_json(v.max_threads);
    return value(std::move(ret));
}
template<> bool from_json(TalkServerSettings& dst, const picojson::value& v)
{
    if (!v.is<object>())
        return false;

    int n = 0;
    if (from_json(dst.port, v.get("port"))) ++n;
    if (from_json(dst.max_queue, v.get("max_queue"))) ++n;
    if (from_json(dst.max_threads, v.get("max_threads"))) ++n;
    return n >= 1;
}

template<> picojson::value to_json(const TalkServerSettingsTable& v)
{
    object t;
    for (auto& kvp : v)
        t[kvp.first] = to_json(kvp.second);
    return value(std::move(t));
}
template<> bool from_json(TalkServerSettingsTable& dst, const picojson::value& v)
{
    if (!v.is<object>())
        return false;

    auto& o = v.get<object>();
    for (auto& kvp : o) {
        if (kvp.second.is<object>()) {
            TalkServerSettings tmp;
            if (from_json(tmp, kvp.second))
                dst[kvp.first] = tmp;
        }
    }
    return true;
}

} // namespace rt

