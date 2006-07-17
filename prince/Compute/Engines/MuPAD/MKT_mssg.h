#ifndef __MKT_mssg_h__
#define __MKT_mssg_h__

// MKT_msg.h -- Portable part of the interface of the API used to communicate
//     with the MuPAD kernel.
//
// PLEASE NOTE: If this interface is changed one must also update the external
// doxygen files in $CVSROOT/DEV/CLIENTS/MUP_LIB/DOC !

#include <stdio.h>
#include "MKT_stream.h"
#include "MCO_encodings.h"

//---------------------------------------------------------------------------

/* Debugger command and info types.
 *
 * This types are described in $CVSROOT/DEV/CLIENTS/MUP_LIB/DOC/prot_dbg.h.
 */
const unsigned char MKdc_clear           = 'C';
const unsigned char MKdc_clear_all       = 'a';
const unsigned char MKdc_continue        = 'c';
const unsigned char MKdc_next            = 'n';
const unsigned char MKdc_step            = 's';
const unsigned char MKdc_finish          = 'f';
const unsigned char MKdc_where           = 'w';
const unsigned char MKdc_up              = 'u';
const unsigned char MKdc_down            = 'd';
const unsigned char MKdc_display         = 'D';
const unsigned char MKdc_undisplay       = 'U';
const unsigned char MKdc_undisplay_all   = 'N';
const unsigned char MKdc_list            = 'b';
const unsigned char MKdc_list_proc       = 'l';
const unsigned char MKdc_autolist        = 't';
const unsigned char MKdc_stop_at         = 'S';
const unsigned char MKdc_quit            = 'q';
const unsigned char MKdc_print           = 'p';
const unsigned char MKdc_pretty_print    = 'P';
const unsigned char MKdc_goto_proc       = 'g';
const unsigned char MKdc_execute         = 'e';
const unsigned char MKdc_tty_help        = '?';
const unsigned char MKdc_level           = 'L';
const unsigned char MKdc_breakpoints     = 'X';
const unsigned char MKdc_display_list    = 'Y';
const unsigned char MKdc_call_stack      = 'Z';
const unsigned char MKdc_activate        = 'x';
const unsigned char MKdc_deactivate      = 'y';

//---------------------------------------------------------------------------
/**@defgroup mup_io MuPAD Input and Output

@brief Constants and Classes to handle MuPAD input and output.

MuPAD input and output may have different formats, output may have
different types. The data itself is stored in byte streams.

This section describes the classes and constants needed to handle the data
formats and typed output.

@see MTstream abstract base class for byte streams used to store the data.
*/
//@{

//---------------------------------------------------------------------------
/**@defgroup out_types Output Types

@brief Constants for the different types of MuPAD oputput.

The kernel has several predefined types of output, encoded by this
string constants.

In the future it will be possible to define new output types via the
MuPAD language and to use these types for example with the MuPAD
function \c print.
*/
//@{
/// Result of a MuPAD command/statement executed.
const char *const MKot_result = "_res";
/// A side effect output, usually produced with the MuPAD function \a print.
const char *const MKot_print = "_prt";
/// An error message after an evaluation or syntax error.
const char *const MKot_error = "_err";
/// A warning message.
const char *const MKot_warning = "_wrn";
/// Debugger protocol output (not pretty-printed).
const char *const MKot_debug = "_dbg";
/// Debugger error message.
const char *const MKot_dbg_err = "_der";
/// Debugger protocol output (pretty-printed).
const char *const MKot_dbg_pretty = "_dpr";
/// Unidentified text output.
const char *const MKot_other = "_oth";
/** Debugging text messages.
 * Usually trace output or output of the kernel C-function \c debug.
 * (The name \c MKot_SYSERROR is misleading.) */
const char *const MKot_SYSERROR = "_ERR";
// Protocol output. This is a special output type which is used
// inside the kernel only for output into a protocol file.
// Should \em not be seen or used by clients.
const char *const MKot_PROTOCOL = "_PRT";
//@}


//---------------------------------------------------------------------------
/**@defgroup data_fmts Data Format Types

@brief Constants for the different formats of MuPAD input and output.

MuPAD input and output may be encoded in different formats. This string
constants encode the types of the predefined formats.

Currently valid output types are: #MKfn_null, #MKfn_text, #MKfn_mcode_tree
and #MKfn_mcode_content. Valid types of input are: #MKfn_mupad_lang and
#MKfn_mcode_tree.

In the future it will be possible to define new data formats for input and
output at the MuPAD language level.
*/
//@{
/// Empty format: The output is ignored.
const char *const MKfn_null = "_nul";
/** Free-form text output of unknown semantics.
 * This is the format of the kernels text output, because it is not valid
 * input in some cases (even if not pretty-printed). */
const char *const MKfn_text = "_txt";
/// Expression tree serialized as a binary MCode stream.
const char *const MKfn_mcode_tree = "_tre";
/** Layout tree serialized as a binary MCode stream.
 * Output for 2-dimensional TeX-like expression layout is encoded in special
 * MuPAD objects. These are serialized into MCode. */
const char *const MKfn_mcode_content = "_con";
/// Text containing MuPAD language statement.
const char *const MKfn_mupad_lang = "_lan";
//@}

//---------------------------------------------------------------------------

/** Specifies format of MuPAD input and output.
 *
 * The format name is not stored, the #name string must be allocated
 * and freed by the user of the objects.
 */
class MKT_DECL_LIB MTformat {
public:
    /** Type of encoding.
     * Currently only #Ascii and #MCode are used. */
    enum Encoding {
        /// ASCII text string
        Ascii,
        /// MCode stream
        MCode,
        /// arbitrary binary data
        Binary
    };

    /** Create format specifier.
     * The argument name is \em not copied.
     * @param name format name string (not copied)
     * @param enc encoding used for this format. */
    MTformat(const char* name=0, Encoding enc=Ascii);

    /** Get format name.
     * @return pointer to formats name string. */
    virtual const char* name() const;

    /** Set format name.
     * The string is \em not copied. */
    virtual void setName(const char*);

    /** Get encoding of data.
     * @return encoding used for this format. */
    virtual Encoding encoding() const;

    /// Set encoding of data.
    virtual void setEncoding(Encoding);

protected:
    const char* name_;  // pointer to name string, not allocated by object
    Encoding enc_;      // encoding
};

/** Formatted MuPAD data used to communicate with kernel.
 *
 * MuPAD data used to communicate with the kernel is stored in byte streams
 * plus additional format informations.
 *
 * This is a base class which is not used directly. Usually the
 * class MTmemoryData is used to handle the data because one needs the
 * properties of a MTmemoryStream for useful operations on the datas
 * stream.
 *
 * The format name string and data stream are not copied. They must be
 * allocated and freed by the user of the objetcs.
 */
class MKT_DECL_LIB MTdata : public MTformat {
public:
    /** Create data specifier.
     * The arguments are not copied.
     * @param name data formats name string (not copied)
     * @param enc data formats encoding
     * @param str stream containing the data (not copied) */
    MTdata(const char* name=0, Encoding enc=Ascii, MTstream* str=0);

    /** Get ASCII C-string containing data.
     * May return 0 if data is not an ASCII string or the stream does not
     * support random positioning. The default implementation always
     * returns 0.
     * @return pointer to C-string data */
    virtual const char* c_str() const;

    /** Get stream containing data.
     * @return stream containing the data */
    virtual MTstream* stream();

protected:
    MTstream* stream_;
};

/** Formatted MuPAD data stored in main memory.
 *
 * MuPAD data used to communicate with the kernel. The data is stored in byte
 * streams of class MTmemoryStream stored in main memory. Additionally fomat
 * informations are provided.
 *
 * The format name string and data stream are not copied. They must be allocated and
 * freed by the user of the objects.
 */
class MKT_DECL_LIB MTmemoryData : public MTdata {
public:
    /** Create data specifier.
     * The arguments are not copied.
     * @param name data format name string (not copied)
     * @param enc data format encoding
     * @param str memory stream holding the data (not copied) */
    MTmemoryData(const char* name=0, Encoding enc=Ascii, MTmemoryStream* str=0);

    /** Get ASCII C-string containing data.
     * Returns 0 if data is not an ASCII string.
     * @return C-string holding the data, 0 if encoding is not #Ascii */
    virtual const char* c_str() const;

    /** Get memory stream containing data.
     * @return pointer to memory stream holding the data */
    virtual MTmemoryStream* mem_stream();
};

//@}

//---------------------------------------------------------------------------
/**@defgroup Kernel Objects and Message Handlers

@brief Kernel objects represent the kernel at the clients side.

At the client side a kernel is represented by an object of a subclass
of MTkernel. This kernel object uses a message handler (an object of
a subclass of MTclientMessageHandler) to handle incoming messages from
the kernel.

SciFace provides concrete kernel subclasses for the different OSes.
The client has to implement message handler subclasses which handle
the messages it is interested in.
*/

//@{

//---------------------------------------------------------------------------

class MTkernel;

/** Base class for client message handlers.
 *
 * Message handlers are used by the client to handle incoming messages from
 * the kernel. They must be subclasses of this class. A message handler must
 * be 'attached' to a kernel object (subclass of MTkernel) in order to handle
 * the messages of the kernel it represents.
 *
 * The message handler methods get called by their kernel object
 * when the corresponding message is received from the kernel.
 *
 * The default implementations generally do nothing useful, but will at
 * least send default 'answers' back to the kernel (where apropriate) in
 * order to avoid that the kernel will block and wait.
 *
 * One must override those methods one is interested in.
 *
 * The file names used in the debugging messages may name stdio files or MuPAD
 * library tar file entries. One may read those files using the MTlibraryStream
 * class.
 */
class MKT_DECL_LIB MTclientMessageHandler {
public:
    /// Constructor.
    MTclientMessageHandler();

    /// Destructor.
    virtual ~MTclientMessageHandler();

    /** Init wanted message.
     * The default implementation sends an empty argument list to the
     * kernel. */
    virtual void on_init_wanted();

    /** Kernel ready message.
     * The default implementation does nothing. */
    virtual void on_kernel_ready();

    /** Output message.
     * The default implementation does nothing.
     * @param cmd_cookie cookie of the command which produced this output
     * @param output_type output type
     * @param output the output */
    virtual void on_output(void* cmd_cookie, const char* output_type,
                           MTmemoryData* output);

    /** Finished message.
     * The default implementation does nothing.
     * @param cmd_cookie cookie of the command which has been executed */
    virtual void on_finished(void* cmd_cookie);

    /** Complete command message.
     * The default implementation does nothing.
     * @param cmd_cookie cookie of the command which is to be completed */
    virtual void on_complete_cmd(void* cmd_cookie);

    /** Input wanted message.
     * The default implementation sends a canceled input message to
     * the kernel.
     * @param cmd_cookie cookie of the command which caused the execution
     *          of the input function
     * @param text_input free-form text or MuPAD command wanted as input
     * @param prompt prompt which should be displayed */
    virtual void on_input_wanted(void* cmd_cookie, bool text_input,
                                 MTmemoryData* prompt);

    /** Callback message.
     * The default implementation sends an error result back to
     * the kernel.
     * @param cmd_cookie cookie of the command which caused the execution
     *          of the #clientCall function
     * @param call MCode stream containing the arguments of the call */
    virtual void on_callback(void* cmd_cookie, MTmemoryStream* call);

    /** Help message.
     * The default implementation does nothing.
     * @param topic help topic */
    virtual void on_help(const char* topic);

    /** System message.
     * The default implementation does not execute the command
     * but sends a system_result(1) message back to the kernel.
     * @param cmd system command to be executed */
    virtual void on_system(const char* cmd);

    /** Frame changed message.
     * The default implementation does nothing.
     * @param frame new frame of client. */
    virtual void on_frame_changed(const char* frame);

    /** Textwidth changed message.
     * The default implementation does nothing.
     * @param width new value of \c TEXTWIDTH */
    virtual void on_textwidth_changed(long width);

    /** Pretty-print changed message.
     * The default implementation does nothing.
     * @param pretty new value of \c PRETTYPRINT */
    virtual void on_pretty_print_changed(bool pretty);

    /** Format changed message.
     * The default implementation does nothing.
     * @param output_type output type
     * @param format new format of the output of the given type */
    virtual void on_format_changed(const char* output_type,
                                   const MTformat* format);

    /** Select format message.
     * The default implementation randomly selects the first format.
     * @param output_type output type for which to select a format
     * @param num_formats number of formats to choose from
     * @param formats array of formats to choose from */
    virtual void on_select_format(const char* output_type, size_t num_formats,
                                  MTformat* formats[]);

    /** Status message.
     * The default implementation does nothing.
     * @param used_mem number of bytes allocated logically
     * @param res_mem number of bytes allocated physically
     * @param sec computation time */
    virtual void on_status(size_t used_mem, size_t res_mem, size_t sec);

    /** Input completion message.
     * The list string encodes the possible completions. Possible cases are:
     * <DL>
     * <DT>An empty string </DT> <DD>Only one possible completion exists, which is
     *    given by \a completion.
     * <DT>The string "\\007" </DT> <DD>No completions exist. \a completion is empty.
     * <DT>A string containing a comma-separated list of names </DT> <DD>The given
     *    completions are possible. \a completion contains their common prefix.
     * </DL>
     * The default implementation does nothing.
     * @param list list of possible completions (may be empty)
     * @param completion common prefix of all possible completions */
    virtual void on_input_completion(const char* list, const char* completion);

    /** Plot result file message.
     * The default implementation does nothing.
     * @param cmd_cookie cookie of plot command which created the data file
     * @param fname name of plot data file */
    virtual void on_plot_result_file(void* cmd_cookie, const char* fname);

    /** Debug command wanted message.
     * The default implementation stops debugging by sending a continue
     * debug message to the kernel.
     * @param cmd_cookie cookie of the command which caused the execution
     *          of the \c debug function */
    virtual void on_dbg_cmd_wanted(void* cmd_cookie);

    /** Debug info message.
     * The default implementation does nothing.
     * @param cmd type of info
     * @param fname file name (may be 0 or empty)
     * @param pos line number in given file (may be 0)
     * @param expr string containing MuPAD expression
     * @param level actual procedure stack frame level
     * @param depth actual depth of procedure call stack */
    virtual void on_dbg_info(unsigned char cmd, const char* fname,
                             long pos, const char* expr, long level, long depth);

    /** Debugger display list message.
     * The default implementation does nothing.
     *
     * Each variable is given by (\a names[i], \a values[i]) ---
     * variable \a i with name \a names[i] and value \a values[i].
     *
     * @param num_vars number of display variables
     * @param names array of \a num_vars variable names
     * @param values array of \a num_vars actual variable values */
    virtual void on_display_list(size_t num_vars, char* names[],
                                 char* values[]);

    /** Debugger breakpoint list message.
     * The default implementation does nothing.
     *
     * Each breakpoint is given by (\a fnames[i], \a lines[i], \a conditions[i]) ---
     * breakpoint \a i in source file \a fnames[i] at line \a lines[i] with
     * condition \a conditions[i]. The condition may be empty.
     *
     * @param num_bpts number of breakpoints
     * @param fnames array of \a num_bpts source file names
     * @param lines array of \a num_bpts source file line numbers
     * @param conditions array of \a num_bpts condition expressions */
    virtual void on_breakpoint_list(size_t num_bpts, char* fnames[],
                                    long lines[], char* conditions[]);

    /** Debugger call stack message.
     * The default implementation does nothing.
     *
     * For each stack frame in the procedure call stack the information
     * (\a fnames[i], \a lines[i], \a procnames[i]) is sent -- frame \a i
     * executing procedure named \a procnames[i] in source file
     * \a fnames[i] at line \a lines[i].
     *
     * @param level actual procedure call stack level
     * @param num_calls number of procedure stack frames
     * @param fnames array of \a num_calls source file names
     * @param lines array of \a num_calls source file line numbers
     * @param procnames array of \a num_calls procedure names */
    virtual void on_call_stack(size_t level, size_t num_calls, char* fnames[],
                                    long lines[], char* procnames[]);

    /** Reset message.
     * The default implementation does nothing. */
    virtual void on_reset();

    /** Quit message.
     * The default implementation does nothing. */
    virtual void on_quit();

protected:
    /** Get kernel sending messages to this handler.
     * @return kernel of this handler */
    MTkernel* kernel();

private:
    /* Define the kernel calling the message handler.
     * Is called by the kernel when it is created or teared down. */
    virtual void set_kernel(MTkernel* kernel);

    MTkernel* kernel_;

    friend class MTkernel;
};

//---------------------------------------------------------------------------
/**@defgroup client_features Client features

@brief Flags describing the (user interface) features a client implements.

Different clients may implement different features (mainly user interface
features). These flags encode the features a client may want to implement 
and the kernel must know about. Use the bitwise or operator to combine
the feature flags.
*/

//@{
/// Type to encode client features.
typedef long MTclientFeature;

/// 0 signals non-feature.
const MTclientFeature MKcf_none = 0;

/// Kernel should stay alive after the client exits.
const MTclientFeature MKcf_stay_alive = 1 << 0;
/// Client allows user to complete incomplete command lines.
const MTclientFeature MKcf_complete_commands = 1 << 1;
/// Client can debug.
const MTclientFeature MKcf_can_debug = 1 << 2;
/// Client can start debug mode after initialization.
const MTclientFeature MKcf_can_lazy_debug = 1 << 3;
/// Client has debugger user-interface.
const MTclientFeature MKcf_debugger_ui = 1 << 4;
/// Client can receive and render plotting data.
const MTclientFeature MKcf_can_plot = 1 << 5;
/// Client can plot binary VCam file.
const MTclientFeature MKcf_can_plot_bin = 1 << 6;
/// Client can create Raster file from binary VCam file.
const MTclientFeature MKcf_can_plot_raster = 1 << 7;
/// Client can create GIF file from binary VCam file.
const MTclientFeature MKcf_can_plot_gif = 1 << 8;
/// Client can create PostScript file from binary VCam file.
const MTclientFeature MKcf_can_plot_postscript = 1 << 9;
/// Client can query \c input from user.
const MTclientFeature MKcf_can_input = 1 << 10;
/// Client can query \c textinput from user.
const MTclientFeature MKcf_can_textinput = 1 << 11;
/// Client can show help.
const MTclientFeature MKcf_can_help = 1 << 12;
/// Client can show dvi hypertext help.
const MTclientFeature MKcf_can_help_dvi = 1 << 13;
/// Client has help browser.
const MTclientFeature MKcf_can_browse_help = 1 << 14;
/// Client can execute system command.
const MTclientFeature MKcf_can_exec_system = 1 << 15;
/// Client is allowed to quit the kernel via MuPAD \c quit command.
const MTclientFeature MKcf_can_quit = 1 << 16;
/// Client can execute callbacks via the kernels \c callback message.
const MTclientFeature MKcf_can_callback = 1 << 17;
/// Client can print in typesetting mode.
const MTclientFeature MKcf_can_typesetting = 1 << 18;
//@}

//---------------------------------------------------------------------------
/**@defgroup exec_flags Command execution flags

@brief Constants describing special features used during command execution.

When executing a command via the MTkernel::send_cmd member function one may
specify additional execution features by combining this flags.
*/

//@{
/// Type to encode command execution flags.
typedef long MTexecutionFlag;

/// 0 signals default command execution.
const MTexecutionFlag MKef_default = 0;

/** Execution flag signalling that the command and result should not
 * be inserted into the command history. */
const MTexecutionFlag MKef_no_history = 0x1;
//@}

//---------------------------------------------------------------------------

/** MuPAD kernel objects base class.
 *
 * Abstract base class defining the clients interface to the kernel (a kernel
 * object represents a kernel on the clients side). Concrete subclasses exist
 * for the different platforms.
 *
 * Incoming messages are handled via message handler objects (subclass of
 * MTclientMessageHandler).
 *
 * Most methods of this class are used to send messages to the kernel. This
 * methods generally return true if the message was sent successfully.
 */
class MKT_DECL_LIB MTkernel {
public:
    /** Create kernel object.
     * @param mssg_handler message handler for the kernel */
    MTkernel(MTclientMessageHandler* mssg_handler);

    /// Destructor.
    virtual ~MTkernel();

    /** Get the ID of this client.
     * The ID returned is the ID this client has on the kernels side.
     * @return the client ID, a non-negative number, or -1 if this
     *   object is not connected to the kernel */
    virtual long get_client_id() const = 0;

    /** Send a new client message.
     * A new client is registered with the kernel. This method must be called
     * exactly once for each kernel object. The kernel will send an \c init_wanted
     * message if this is its first client. When the kernel is ready to receive
     * commands it will send a \c kernel_ready message.
     * @param name name of this client (used in the kernel to identify the client)
     * @param frame actual frame (context) of the client after initialization
     * @param long_fmt format of longs in MCode streams used for communication
     * @param float_fmt format of floats in MCode streams
     * @param coding type of encoding to be used in MCode streams
     * @param features client feature flags
     * @return true if client was registered successfully */
    virtual bool send_new_client(const char* name=0,
				 const char* frame=0,
				 MTlongFormat long_fmt=MCstringLong,
				 MTfloatFormat float_fmt=MCieeeFloat,
				 MTcoding coding=MCflatCoding|MCidentCoding,
				 MTclientFeature features=0) = 0;

    /** Send initialization arguments.
     * The format of the arguments is that of command line arguments.
     * @param argc number of initialization arguments
     * @param argv array of \a argc strings used as initialization arguments
     * @return true if message was sent successfully */
    virtual bool send_init(size_t argc=0, char* argv[]=0) = 0;

    /** Send a MuPAD command.
     * Currently a MuPAD command may have the format #MKfn_mupad_lang
     * (MuPAD language) or #MKfn_mcode_tree (MCode expression tree).
     *
     * The parameter \a flags may be used to control the execution of the
     * command. See the \c MKef_... constants for possible values.
     *
     * The parameter \a format may be used to define the output format of
     * the result messages to be returned (independent of the actual
     * format settings). If 0 or an empty string is entered the actual
     * output format for results is used. One will not receive any
     * \c format_changed messages if the format given is not the same as
     * the actual format for results.
     *
     * @param cmd_cookie cookie which may be used by the client to identify
     *    the results of the command
     * @param cmd MuPAD command
     * @param flags evaluation flags (no special handling if 0)
     * @param format output format to be used for result messages
     *    (the actual format is used if 0 or the empty string)
     * @return true if message was sent successfully */
    virtual bool send_cmd(void* cmd_cookie, MTmemoryData* cmd,
                          MTexecutionFlag flags=0, const char* format=0) = 0;

    /** Send a completion wanted message.
     * @param prefix prefix of name to be completed
     * @return true if message was sent successfully */
    virtual bool send_completion_wanted(const char* prefix) = 0;

    /** Send debug command.
     * @param cmd the debugger command
     * @param fname file name
     * @param fpos line number in file
     * @param expr expression (as MuPAD language statement)
     * @return true if message was sent successfully */
    virtual bool send_dbg_cmd(unsigned char cmd, const char* fname=0,
                              long fpos=0, const char* expr=0) = 0;

    /** Send a response for a MuPAD \c system() function call.
     * @param result result code of the system call
     * @return true if message was sent successfully */
    virtual bool send_system_result(int result) = 0;

    /** Send a response for a MuPAD \c input() function call.
     * @param canceled true if the input was canceled by the user.
     * @param input the input entered by the user.
     * @return true if message was sent successfully */
    virtual bool send_input(bool canceled, MTmemoryData* input) = 0;

    /** Send a response for a MuPAD \c textinput() function call.
     * @param canceled true if the input was canceled by the user.
     * @param input the input entered by the user.
     * @return true if message was sent successfully */
    virtual bool send_textinput(bool canceled, const char* input) = 0;

    /** Send a response for a MuPAD \c clientCall() function call.
     * @param error true if an error occured when executing the callback function
     * @param result result of the callback function (encoded as MCode stream)
     * @return true if message was sent successfully */
    virtual bool send_call_result(bool error, MTmemoryStream* result) = 0;

    /** Send a new textwidth.
     * @param width new value of \c TEXTWIDTH
     * @return true if message was sent successfully */
    virtual bool send_textwidth(long width) = 0;

    /** Send a new pretty-print value.
     * @param pretty new value of \c PRETTYPRINT
     * @return true if message was sent successfully */
    virtual bool send_pretty_print(bool pretty) = 0;

    /** Send a format wanted for a output type.
     * @param output_type output type (one of the \c MKot_... constants)
     * @param format output format (one of the \c MKfn_... constants)
     * @return true if message was sent successfully */
    virtual bool send_format(const char* output_type, const char* format) = 0;

    /** Send a status request.
     * @return true if message was sent successfully */
    virtual bool send_status_wanted() = 0;

    /** Send a stop signal.
     * @return true if message was sent successfully */
    virtual bool send_stop() = 0;

    /** Send a quit message.
     * @return true if message was sent successfully */
    virtual bool send_quit() = 0;

protected:
    // disconnects this kernel from its message handler
    void quit_mssg_handler();
    MTclientMessageHandler* mssg_handler_;
};

//@}

#endif // !__MKT_mssg_h__
