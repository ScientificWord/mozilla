#ifndef __MKT_com_kernel_h__
#define __MKT_com_kernel_h__

// MKT_com_kernel.h -- client kernel communication class for Windows/COM kernel

#include "MKT_xmsg_kernel.h"

struct IMssg;
class MTkernelEvent;

/** Clients MuPAD kernel object for the Windows/COM kernel server.
 *
 * An object of this class represents a client connection to a MuPAD
 * Windows/COM kernel for the 'new' COM protocol introduced with MuPAD 3.0.
 * Kernels may be started on remote servers using DCOM.
 *
 * For each client process there may exist several objects of this class,
 * connected to one or more kernel processes. One may identify the kernel
 * objects by name and enumerate them.
 *
 * This class is single-appartment thread safe (i.e. can be used in single-
 * appartment COM objects).
 *
 * If an error occures in a COM call the \c HRESULT of the call is either
 * returned (by the static member functions) or can be obtained via the
 * member function #get_hresult.
 */

class MKT_DECL_LIB MTcomKernel : public MTxmsgKernel {
public:
    /**@name Managing Kernels
     *
     * Each client process may hold connections to several kernels at each
     * time. The following static members may be used to start and connect
     * to a new kernel or connect to a running kernel.
     *
     * A MTcomKernel object may only be created by these static members,
     * its constructors are private.
     *
     * Each kernel has a name. The names of the kernels a client process is
     * connected to may be enumerated.
     */
    //@{
    /** Start and connect to new kernel.
     *
     * The kernel name may be 0, in this case a new unique default name is choosen.
     *
     * If \a server is 0 or \c L"" the kernel will be started on a default server,
     * which may be a remote server depending on the \c RemoteServerName
     * registry setting for the kernel. If the server name is \c L"." a local
     * server will be started.
     *
     * \c E_FAIL is returned if a kernel with the name given already existed.
     *
     * @param clsid_kernel the CLSID of the kernel object
     * @param mssg_handler messsage handler for the kernel object returned
     * @param kernel returns pointer to kernel object
     * @param name (local) name of the kernel, may be 0
     * @param server name of server where to launch the kernel (may be 0)
     * @return \c S_OK in case of success, \c HRESULT of failing COM call otherwise
     */
    static HRESULT new_kernel(const CLSID& clsid_kernel,
                              MTclientMessageHandler* mssg_handler,
                              MTcomKernel** kernel,
                              const char* name=0, wchar_t* server=0);

    /** Get a new client connection to the named kernel.
     *
     * A new kernel is started if the named one does not exist and \a start
     * is true. The kernel ist started on the server \a server.
     * Otherwise a connection to the named kernel is established.
     *
     * If \a server is 0 or \c L"" the kernel will be started on a default server,
     * which may be a remote server depending on the \c RemoteServerName
     * registry setting for the kernel. If the server name is \c L"." a local
     * server will be started.
     *
     * \c E_FAIL is returned if no kernel with the name exists.
     *
     * @param clsid_kernel the CLSID of the kernel object
     * @param mssg_handler messsage handler for the kernel object returned
     * @param kernel returns pointer to kernel object
     * @param name (local) name of the kernel to connect to
     * @param start start new kernel if named one does not exist
     * @param server name of server where to launch the kernel (may be 0)
     * @return \c S_OK in case of success, \c HRESULT of failing COM call otherwise
     */
    static HRESULT get_kernel(const CLSID& clsid_kernel,
                              MTclientMessageHandler* mssg_handler,
                              MTcomKernel** kernel,
                              const char* name, bool start=true,
                              wchar_t* server=0);

    /** Get a new client connection to a kernel given by interface.
     *
     * A new client connection to a running kernel given by a COM 
     * interface pointer is established.
     *
     * @param interf COM interface of a running kernel
     * @param mssg_handler messsage handler for the kernel object returned
     * @param kernel returns pointer to kernel object
     * @param name (local) name of the kernel, may be 0
     * @return \c S_OK in case of success, \c HRESULT of failing COM call otherwise
     */
    static HRESULT get_kernel(IUnknown* interf,
                              MTclientMessageHandler* mssg_handler,
                              MTcomKernel** kernel,
                              const char* name=0);

    /** Get the names of the kernels the client process is connected to.
     * Returns an array of C-strings where the last entry is 0. The strings
     * and the array must be freed by the caller using \c free !
     * @param names returns array of C-strings, last entry is 0
     * @return \c S_OK in case of success, \c HRESULT of failing COM call otherwise
     */
    static HRESULT get_kernel_names(char*** names);

    /** Returns true if this machine has DCOM installed.
     * If DCOM is installed one may access or start a kernel on a remote
     * server.
     * @return true if DCOM is available */
    static bool has_dcom();
    //@}

    /** Destructor, releases connection to kernel.
     * If this is the last client of the kernel and no client forced the
     * kernel to stay alive the kernel will be quit. */
    virtual ~MTcomKernel();

    /** Get \c HRESULT of last COM call.
     * May be used directly after a failed \c send_... call to obtain the
     * \c HRESULT of the last COM call. */
    HRESULT get_hresult() const;

    /** Query IUnknown COM interface from kernel.
     * @param interf returns IUnknown COM interface of kernel */
    HRESULT get_interface(IUnknown** interf);

    /** Get the name of the kernel.
     * The result string is allocated on the heap and must be freed by
     * the caller using \c free !
     * @return string holding the name of the kernel, may be 0 */
    char* get_name() const;

    /** Change the name of the kernel.
     * @param name new kernel name
     * @return true if name was changed */
    bool set_name(const char* name);

    /** Get the ID of this client.
     * This is the ID this client has on the kernels side.
     * @return the client ID, a non-negative number, or -1 if this
     *   object is not connected to the kernel */
    long get_client_id() const;

    // Send new_client message.
    bool send_new_client(const char* name=0, 
                         const char* frame=0, 
                         MTlongFormat long_fmt=MCstringLong,
                         MTfloatFormat float_fmt=MCieeeFloat, 
                         MTcoding coding=MCflatCoding|MCidentCoding,
                         MTclientFeature features=0);

private:
    // create kernel
    MTcomKernel(MTclientMessageHandler* mssg_handler, long cookie, IMssg* mssg,
                MTkernelEvent* event_sink);

    // remember the HRESULT
    HRESULT hr(HRESULT) const;

    virtual MTstream* xstream();
    virtual void xbegin(const char*, size_t);
    virtual bool xend();

    // Kernel interface.
    IMssg* kernel_;

    // Hides the COM event sink interface from the library user
    MTkernelEvent* event_sink_;

    long cookie_;           // the id of this kernel for manager
    mutable HRESULT hr_;    // the HRESULT of the last COM call
    long client_id_;        // the id of our client
    MTheapStream stream_;   // stream where XML messages are put
};

#endif // !__MKT_com_kernel_h__
