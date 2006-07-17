#ifndef __MKT_xmsg_kernel_h__
#define __MKT_xmsg_kernel_h__

// MKT_xmsg_kernel.h -- client kernel communication class for XML protocol messages

#include "MKT_mssg.h"

/** Clients MuPAD kernel object base class for XML protocol messages.
 *
 * An object of this class represents a client connection to a MuPAD
 * kernel for the 'new' XML message protocol introduced with MuPAD 3.0.
 */
class MKT_DECL_LIB MTxmsgKernel : public MTkernel {
public:
    /// Destructor.
    virtual ~MTxmsgKernel();

    /** Send a new_client message.
     * This implementation does not call xend, thus the message
     * is not actually sent. It only outputs the message to the XML message 
     * stream. */
    virtual bool send_new_client(const char* name=0,
			                     const char* frame=0,
			                     MTlongFormat long_fmt=MCstringLong,
			                     MTfloatFormat float_fmt=MCieeeFloat,
			                     MTcoding coding=MCflatCoding|MCidentCoding,
			                     MTclientFeature features=0);

    // Send initialization data.
    bool send_init(size_t argc=0, char* argv[]=0);

    // Send a MuPAD command.
    bool send_cmd(void* cmd_cookie, MTmemoryData* cmd,
                  MTexecutionFlag flags=0, const char* format=0);

    // Send a completion wanted message.
    bool send_completion_wanted(const char* prefix);

    // Send debug command.
    bool send_dbg_cmd(unsigned char cmd, const char* fname=0,
                      long fpos=0, const char* expr=0);

    // Send a response for a MuPAD system() function call.
    bool send_system_result(int result);

    // Send a response for a MuPAD input() function call.
    bool send_input(bool canceled, MTmemoryData* input=0);

    // Send a response for a MuPAD textinput() function call.
    bool send_textinput(bool canceled, const char* input=0);

    // Send a response for a MuPAD client_call() function call.
    bool send_call_result(bool error, MTmemoryStream* result=0);

    // Send a new textwidth.
    bool send_textwidth(long width);

    // Send a new pretty-print value.
    bool send_pretty_print(bool pretty);

    // Send a format wanted for a output type.
    bool send_format(const char* output_type, const char* format);

    // Send a status request.
    bool send_status_wanted();

    // Send a stop signal.
    bool send_stop();

    // Send a quit message.
    bool send_quit();

protected:
    // create kernel
    MTxmsgKernel(MTclientMessageHandler* mssg_handler);

    virtual MTstream* xstream() = 0;
    virtual long get_client_id() const = 0;

    virtual void xbegin(const char*, size_t);
    virtual bool xend();

    void xput_cdata(const char*);
    void xput(const char*, size_t);
    void xput(void*);
    void xput(bool);
    void xput(long);
    void xput(const MTmemoryData*);

    MTstream::Stat stat_;

#ifdef _DEBUG
    // sendig messages is no reentrant
    bool in_send_;
#endif
};

#endif // ! __MKT_xmsg_kernel_h__
