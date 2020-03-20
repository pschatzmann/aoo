#pragma once

#include "aoo.h"
#include "lfqueue.hpp"
#include "time_dll.hpp"

#include <memory>

namespace aoo {

// NOTE: aoo::isource and aoo::isink don't define virtual destructors
// and have to be destroyed with their respective destroy() method.
// We provide a custom deleter and shared pointer to automate this task.
//
// The absence of a virtual destructor allows for ABI independent
// C++ interfaces on Windows (where the vtable layout is stable
// because of COM) and usually also on other platforms.
// (Compilers use different strategies for virtual destructors,
// some even put more than 1 entry in the vtable.)
// Also, we only use standard C types as function parameters
// and return types.
//
// In practice this means you only have to build 'aoo' once as a
// shared library and can then use it with the C++ interface in
// applications built with different compilers resp. compiler versions.
//
// If you want to be on the safe safe, use the C interface :-)

/*//////////////////////// AoO source ///////////////////////*/

class isource {
public:
    class deleter {
    public:
        void operator()(isource *x){
            destroy(x);
        }
    };
    // smart pointer for AoO source instance
    using pointer = std::unique_ptr<isource, deleter>;

    // create a new AoO source instance
    static isource * create(int32_t id);

    // destroy the AoO source instance
    static void destroy(isource *src);

    // Call from any thread - synchronize with network and audio thread!
    virtual int32_t setup(const aoo_source_settings& settings) = 0;

    // Call from any thread - synchronize with network and audio thread!
    virtual int32_t add_sink(void *sink, int32_t id, aoo_replyfn fn) = 0;

    // Call from any thread - synchronize with network and audio thread!
    virtual int32_t remove_sink(void *sink, int32_t id) = 0;

    // Call from any thread - synchronize with network and audio thread!
    virtual void remove_all() = 0;

    // Call from the network thread.
    virtual int32_t handle_message(const char *data, int32_t n,
                                void *endpoint, aoo_replyfn fn) = 0;

    // Call from the network thread.
    virtual int32_t send() = 0;

    // Call from the audio thread.
    // data:        array of channel data (non-interleaved)
    // nsamples:    number of samples per channel
    // t:           current NTP timestamp (see aoo_osctime_get)
    virtual int32_t process(const aoo_sample **data,
                            int32_t nsamples, uint64_t t) = 0;

    // Call from any thread - always thread safe!
    virtual int32_t events_available() = 0;

    // Call from any thread - always thread safe!
    virtual int32_t handle_events() = 0;

    //---------------------- options ----------------------//
    // Call from any thread - synchronize with network and audio thread!

    int32_t set_format(aoo_format& f){
        return set_option(aoo_opt_format, AOO_ARG(f));
    }

    int32_t get_format(aoo_format_storage& f){
        return get_option(aoo_opt_format, AOO_ARG(f));
    }

    int32_t set_buffersize(int32_t n){
        return set_option(aoo_opt_buffersize, AOO_ARG(n));
    }

    int32_t get_buffersize(int32_t& n){
        return get_option(aoo_opt_buffersize, AOO_ARG(n));
    }

    int32_t set_timefilter_bandwidth(float f){
        return set_option(aoo_opt_timefilter_bandwidth, AOO_ARG(f));
    }

    int32_t get_timefilter_bandwidth(float& f){
        return get_option(aoo_opt_timefilter_bandwidth, AOO_ARG(f));
    }

    int32_t set_packetsize(int32_t n){
        return set_option(aoo_opt_packetsize, AOO_ARG(n));
    }

    int32_t get_packetsize(int32_t& n){
        return get_option(aoo_opt_packetsize, AOO_ARG(n));
    }

    int32_t set_resend_buffersize(int32_t n){
        return set_option(aoo_opt_resend_buffersize, AOO_ARG(n));
    }

    int32_t set_resend_buffersize(int32_t& n){
        return get_option(aoo_opt_resend_buffersize, AOO_ARG(n));
    }

    virtual int32_t set_option(int32_t opt, void *ptr, int32_t size) = 0;
    virtual int32_t get_option(int32_t opt, void *ptr, int32_t size) = 0;

    //--------------------- sink options --------------------------//

    int32_t set_sink_channelonset(void *endpoint, int32_t id, int32_t onset){
        return set_sinkoption(endpoint, id, aoo_opt_channelonset, AOO_ARG(onset));
    }

    int32_t get_sink_channelonset(void *endpoint, int32_t id, int32_t& onset){
        return get_sinkoption(endpoint, id, aoo_opt_channelonset, AOO_ARG(onset));
    }

    virtual int32_t set_sinkoption(void *endpoint, int32_t id,
                                   int32_t opt, void *ptr, int32_t size) = 0;
    virtual int32_t get_sinkoption(void *endpoint, int32_t id,
                                   int32_t opt, void *ptr, int32_t size) = 0;
protected:
    ~isource(){} // non-virtual!
};

inline isource * isource::create(int32_t id){
    return aoo_source_new(id);
}

inline void isource::destroy(isource *src){
    aoo_source_free(src);
}

/*//////////////////////// AoO sink ///////////////////////*/

class isink {
public:
    class deleter {
    public:
        void operator()(isink *x){
            destroy(x);
        }
    };
    // smart pointer for AoO sink instance
    using pointer = std::unique_ptr<isink, deleter>;

    // create a new AoO sink instance
    static isink * create(int32_t id);

    // destroy the AoO sink instance
    static void destroy(isink *sink);

    // Call from any thread - synchronize with network and audio thread!
    virtual int32_t setup(const aoo_sink_settings& settings) = 0;

    // Call from the network thread.
    virtual int32_t handle_message(const char *data, int32_t n,
                                   void *endpoint, aoo_replyfn fn) = 0;

    // Call from the audio thread.
    virtual int32_t process(uint64_t t) = 0;

    // Call from any thread - always thread safe!
    virtual int32_t events_available() = 0;

    // Call from any thread - always thread safe!
    virtual int32_t handle_events() = 0;

    //---------------------- options ----------------------//
    // Call from any thread - synchronize with network and audio thread!

    int32_t reset(){
        return set_option(aoo_opt_reset, AOO_ARGNULL);
    }

    int32_t set_buffersize(int32_t n){
        return set_option(aoo_opt_buffersize, AOO_ARG(n));
    }

    int32_t get_buffersize(int32_t& n){
        return get_option(aoo_opt_buffersize, AOO_ARG(n));
    }

    int32_t set_timefilter_bandwidth(float f){
        return set_option(aoo_opt_timefilter_bandwidth, AOO_ARG(f));
    }

    int32_t get_timefilter_bandwidth(float& f){
        return get_option(aoo_opt_timefilter_bandwidth, AOO_ARG(f));
    }

    int32_t set_packetsize(int32_t n){
        return set_option(aoo_opt_packetsize, AOO_ARG(n));
    }

    int32_t get_packetsize(int32_t& n){
        return get_option(aoo_opt_packetsize, AOO_ARG(n));
    }

    int32_t set_ping_interval(int32_t n){
        return set_option(aoo_opt_ping_interval, AOO_ARG(n));
    }

    int32_t get_ping_interval(int32_t& n){
        return get_option(aoo_opt_ping_interval, AOO_ARG(n));
    }

    int32_t set_resend_limit(int32_t n){
        return set_option(aoo_opt_resend_limit, AOO_ARG(n));
    }

    int32_t get_resend_limit(int32_t& n){
        return get_option(aoo_opt_resend_limit, AOO_ARG(n));
    }

    int32_t set_resend_interval(int32_t n){
        return set_option(aoo_opt_resend_interval, AOO_ARG(n));
    }

    int32_t get_resend_interval(int32_t& n){
        return get_option(aoo_opt_resend_interval, AOO_ARG(n));
    }

    int32_t set_resend_maxnumframes(int32_t n){
        return set_option(aoo_opt_resend_maxnumframes, AOO_ARG(n));
    }

    int32_t get_resend_maxnumframes(int32_t& n){
        return get_option(aoo_opt_resend_maxnumframes, AOO_ARG(n));
    }

    virtual int32_t set_option(int32_t opt, void *ptr, int32_t size) = 0;
    virtual int32_t get_option(int32_t opt, void *ptr, int32_t size) = 0;

    //----------------- source options -------------------//

    int32_t reset_source(void *endpoint, int32_t id){
        return set_sourceoption(endpoint, id, aoo_opt_reset, AOO_ARGNULL);
    }

    int32_t get_source_format(void *endpoint, int32_t id, aoo_format_storage& f){
        return get_sourceoption(endpoint, id, aoo_opt_format, AOO_ARG(f));
    }

    virtual int32_t set_sourceoption(void *endpoint, int32_t id,
                                   int32_t opt, void *ptr, int32_t size) = 0;
    virtual int32_t get_sourceoption(void *endpoint, int32_t id,
                                   int32_t opt, void *ptr, int32_t size) = 0;
protected:
    ~isink(){} // non-virtual!
};

inline isink * isink::create(int32_t id){
    return aoo_sink_new(id);
}

inline void isink::destroy(isink *sink){
    aoo_sink_free(sink);
}

} // aoo
