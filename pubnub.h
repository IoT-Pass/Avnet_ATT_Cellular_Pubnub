#if !defined INC_PUBNUB
#define      INC_PUBNUB

#include <string>
#include <vector>


/** A pubnub context.
*/
class pubnub_ctx {
public:
    /** Give publish key as @p pub_key and subscribe key as @p key_sub
        to initialize the context.
    */
    pubnub_ctx(char const* pub_key, char const* key_sub);
    
    ~pubnub_ctx();
    
    /** Result of a Pubnub transaction */
    enum result {
        format_error,
        response_too_short,
        missing_open_bracket,
        missing_close_bracket,
        missing_time_token,
        bad_time_token,
        publish_failed,
        ok
    };
    
    /** Publish a JSON @p message to a pubnub @p channel (or channels,
        separated by commas).
        This is a synchronous call, so it will take a while to return
        (depending on your Internet connection, could be 100s of ms).
        */
    result publish(char const* channel, char const* message);
    
    /** Subscribe on a @p channel (or channels, separated by commas).
        Messages received (if any) are pushed back to the @p messages
        vector.
        
        This is a synchronous call, but it will not wait the full Pubnub ~5 min
        for a message to appear on the channel. It will be much less, ~1 s.
        
        @return vector of messages - empty if there are no messages (like on
        the very first subscribe)
    */
    result subscribe(char const* channel, std::vector<std::string>& messages);
    
    std::string uuid() const { return d_uuid; }
    void set_uuid(char const *s) { d_uuid = s; }

    std::string auth() const { return d_auth; }
    void set_auth(char const *s) { d_auth = s; }
    
private:
    /// The publish key to use
    std::string d_pub_key;
    
    /// The subscribe key to use
    std::string d_key_sub;
    
    /// the last timetoken
    std::string d_token;
    
    /// The UUID to use (empty - do not use)
    std::string d_uuid;
    
    /// The auth key to use (empty - do not use)
    std::string d_auth;
};

#endif // !defined INC_PUBNUB
