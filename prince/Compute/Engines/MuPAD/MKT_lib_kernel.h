#ifndef __MKT_lib_kernel_h__
#define __MKT_lib_kernel_h__

#include "MKT_mssg.h"

/** Clients MuPAD kernel object for library kernel.
 *
 * An object of this class represents a client connection to a MuPAD
 * library kernel.
 *
 * For each client process there may exist several objects of this class,
 * connected to exactly one common kernel object.
 *
 * This class is not thread safe.
 */

class MKT_DECL_LIB MTlibKernel : public MTkernel {
public:
    /** Get a new client connection to the kernel.
     *
     * The \a path argument is used to set up defaults for the MuPAD
     * library path etc. One may use 0 for it, but in this case one must be shure
     * to set up the library path correctly during the initialization of the
     * first client via the send_init() method.
     *
     * If a \a path is given the MuPAD library is searched for in the
     * folder <tt>../lib/</tt> relative to it. This folder must contain
     * the files <tt>sysinit.mu</tt> and <tt>lib.tar</tt>. If one wants
     * to store the library at a different location one must use the <tt>-l</tt>
     * option via the send_init() method.
     *
     * The kernel is initialized when the first client connection is created
     * via send_new_client().
     *
     * Note: In the client messsage handler one \em must override the
     * MTclientMessageHandler::on_init_wanted method and call the kernels
     * send_init() methods with a user name and license key for the DLL.
     * The user name must be given via the <tt>'-N'</tt> option, the license
     * key via the <tt>'-K'</tt> option. A minimal implementation may look
     * like follows:
     * \code
     * void MyHandler::on_init_wanted()
     * {
     *     static char* argv[4] =
     *         { "-N", "<<your user name>>", "-K", "<<your license key>>" };
     *     m_Kernel->send_init(4, argv);
     * }
     * \endcode
     *
     * @param mssg_handler messsage handler for the kernel object returned
     * @param path full path name of executable using the library or 0
     * @param kernel returns pointer to kernel object
     * @return true in case of success, false otherwise
     */
    static bool get_kernel(MTclientMessageHandler* mssg_handler,
                           const char* path,
                           MTlibKernel** kernel);

    /** Set event handler for kernel.
     * This installs a callback which is called quite often by the kernel
     * during longer calculations. It may be used to keep a user interface
     * reactive for example. The previous event handler is returned. The
     * \a user_data is handled back to the handler. */
    static void (*event_handler(void (*handler)(void*), void* user_data))(void*);

    /** Set fatal exit handler for kernel.
     * This installs a callback which is executed when the kernel
     * causes a fatal error. The previous exit handler is returned.
     * The default handler does nothing.
     *
     * When the fatal_exit_handler() returns \c exit is called in any
     * case. This is because the kernel is in the middle of nowhere
     * and execution of kernel code can not be resumed at this point.
     *
	 * One must execute a \c longjmp or similar in the exit handler
	 * to shut down the kernel DLL gracefully.
     */
    static void (*fatal_exit_handler(void (*handler)()))();

    /** Destructor, releases connection to kernel.
     * If this is the last client of the kernel the kernel will be quit. */
    virtual ~MTlibKernel();

    /** Get the ID of this client.
     * The ID returned is the ID this client has on the kernels side.
     * @return the client ID, a non-negative number, or -1 if this
     *   object is not connected to the kernel */
    long get_client_id() const;

    // Send a new client message.
    bool send_new_client(const char* name=0,
			 const char* namesp=0,
			 MTlongFormat long_fmt=MCstringLong,
			 MTfloatFormat float_fmt=MCieeeFloat,
			 MTcoding coding=MCflatCoding|MCidentCoding,
			 MTclientFeature features=0);

    /// Get message handler of this client.
    MTclientMessageHandler* mssg_handler();

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

    // Send a response for a MuPAD input() function call.
    bool send_input(bool canceled, MTmemoryData* input=0);

    // Send a response for a MuPAD textinput() function call.
    bool send_textinput(bool canceled, const char* input=0);

    // Send a response for a MuPAD clientCall() function call.
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

private:
    // create kernel
    MTlibKernel(MTclientMessageHandler* mssg_handler, const char* path);

    long client_id_;           // the id of this client
    char* path_;               // path to executable using the library
};

#endif // !__MKT_lib_kernel_h__
