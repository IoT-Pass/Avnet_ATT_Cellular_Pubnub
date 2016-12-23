#include "pubnub.h"

#include "mbed.h"
#include <string>
#include "wnc_control.h"

#include <stdio.h>


pubnub_ctx::pubnub_ctx(char const* pub_key, char const *key_sub)
    : d_pub_key(pub_key)
    , d_key_sub(key_sub)
    , d_token("0")
{
}


pubnub_ctx::~pubnub_ctx()
{
}


static void append_epilogue(std::string &s, std::string const& uuid, std::string const& auth)
{
    s += "?pnsdk=AvnetATTmbed";
    if (!uuid.empty()) {
        s += "&uuid=";
        s += uuid;
    }
    if (!auth.empty()) {
        s += "&auth=";
        s += auth;
    }
    s += " HTTP/1.1\r\nHost: pubsub.pubnub.com\r\n\r\n";
}


pubnub_ctx::result pubnub_ctx::publish(char const* channel, char const* message)
{
    char const *pmessage = message;
    std::string s("GET /publish/");
    s += d_pub_key; s += "/";
    s += d_key_sub; s += "/0/";
    s += channel; s += "/0/";

    while (pmessage[0]) {
        /* RFC 3986 Unreserved characters plus few
         * safe reserved ones. */
        size_t okspan = strspn(pmessage, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_.~" ",=:;@[]");
        if (okspan > 0) {
            s.append(pmessage, okspan);
            pmessage += okspan;
        }
        if (pmessage[0]) {
            /* %-encode a non-ok character. */
            s.append(1, '%');
            s.append(1, "0123456789ABCDEF"[pmessage[0] / 16]);
            s.append(1, "0123456789ABCDEF"[pmessage[0] % 16]);
            ++pmessage;
        }
    }
    
    append_epilogue(s, d_uuid, d_auth);

    sockwrite_mdm(s.c_str());
    string response;
    unsigned read = sockread_mdm(&response, 1024, 50);

    if (read < 5) {
        extern Serial pc;
        pc.printf("read = %d\r\n", read);
        return response_too_short;
    }
    char const* start = strchr(response.data(), '[');
    if (NULL == start) {
        return missing_open_bracket;
    }
    if (NULL == strrchr(response.data() + response.size() - 1, ']')) {
        return missing_close_bracket;
    }

    if (1 != strtol(start + 1, NULL, 10)) {
        return publish_failed;
    }

    return ok;
}

pubnub_ctx::result pubnub_ctx::subscribe(char const* channel, std::vector<std::string>& messages)
{
    std::string s("GET /subscribe/");
    s += d_key_sub; s += "/";
    s += channel; s += "/0/";
    s += d_token;;
    append_epilogue(s, d_uuid, d_auth);

    sockwrite_mdm(s.c_str());
    string response;
    unsigned read = sockread_mdm(&response, 1024, 50);

    if (read < 10) {
        return response_too_short;
    }
    char const* start = strchr(response.c_str(), '[');
    if (NULL == start) {
        return missing_open_bracket;
    }
    if (start[1] != '[') {
        return missing_open_bracket;
    }
    enum {
        idle,
        in_string,
        escape,
        done
    } state = idle;
    start += 2;
    char const* end;
    int bracket_level = 1;
    for (end = start; (*end != '\0') && (state != done); ++end) {
        char c = *end;
        switch (state) {
        case idle:
            switch (c) {
            case '"':
                state = in_string;
                break;
            case ',':
                if (bracket_level == 1) {
                    messages.push_back(std::string(start, end-start));
                    start = end + 1;
                }
                break;
            case '{':
            case '[':
                ++bracket_level;
                break;
            case '}':
                --bracket_level;
                break;
            case ']':
                if (--bracket_level <= 0) {
                    if (end-start-1 > 0) {
                        messages.push_back(std::string(start, end-start));
                    }
                    state = done;
                }
                break;
            }
            break;
        case in_string:
            switch (c) {
            case '"':
                state = idle;
                break;
            case '\\':
                state = escape;
                break;
            }
            break;
        case escape:
            state = in_string;
            break;
        default:
            state = done;
            break;
        }
    }
    if (state != done) {
        return format_error;
    }
    if ((end[0] != ',') && (end[1] != '"')) {
        return missing_time_token;
    }
    start = end + 2;
    end = strchr(start, '"');
    if (NULL == end) {
        return bad_time_token;
    }
    d_token.assign(start, end-start);

    return ok;
}
