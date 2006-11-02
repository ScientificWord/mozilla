#ifndef MUPKERNEL_H_INCLUDED
#define MUPKERNEL_H_INCLUDED
/*
Copyright (c) 2005 SciFace Software GmbH & Co KG
All Rights Reserverd.
*/

/** @file mupkernel.h 

The MuPAD kernel client interface.
A plain C API for communicating with the MuPAD kernel.
*/

#ifdef WIN32

#  include <stddef.h>

#  ifndef MUP_STATIC
#    ifdef MUP_EXPORTS
#      define MUP_EXTERN extern __declspec(dllexport)
#    else
#      define MUP_EXTERN extern __declspec(dllimport)
#    endif
#    define MUP_API __stdcall
#    define MUP_CALLBACK __stdcall
#  endif  /* not MUP_STATIC */

#endif /* WIN32 */

#ifndef MUP_EXTERN
#  define MUP_EXTERN extern
#endif

#ifndef MUP_API
#  define MUP_API /* as nothing */
#endif

#ifndef MUP_CALLBACK
#  define MUP_CALLBACK /* as nothing */
#endif

/** @def MUP_CALLBACK
 * Calling convention of callback functions.
 * Must be used to declare callback functions.
 * Under Windows this expands to \c __stdcall. */

#ifdef __cplusplus
extern "C" {
#endif

/* ----------------------------------------------------------------------- */
/** @defgroup base_types Basic Types 

@brief Basic data types.

Strings declared as plain \c char * are always ASCII encoded.

Strings declared as #MUP_Char * may have different encodings.
The following combinations exist:

<UL>
 <LI>If \c MUP_UNICODE is \em not defined these strings are single-
    or multibyte strings encoded in the current users locale of the
    calling process and <CODE>sizeof(MUP_Char) == 1</CODE> holds.</LI>

 <LI>If \c MUP_UNICODE is defined these strings are UNICODE strings.</LI>
 <UL>
    <LI>If \c MUP_UTF16 is defined strings are UFT-16 encoded UNICODE 
        strings, one character is 2 bytes long, i.e. 
        <CODE>sizeof(MUP_Char) == 2</CODE> holds.
        \c MUP_UTF16 implies \c MUP_UNICODE.</LI>

    <LI>If \c MUP_UNICODE is defined, but \em not \c MUP_UTF16 the strings
        are UTF-8 encoded UNICODE strings and
        <CODE>sizeof(MUP_Char) == 1</CODE> holds.</LI>

    <LI>If \c MUP_UTF16_WCHAR_T is defined this implies that \c MUP_UTF16
        and thus also \c MUP_UNICODE is defined. Further it is assumed that
        <CODE>sizeof(wchar_t) == 2</CODE> and \c wchar_t is used to define
        MUP_Char.</LI>
  </UL>
</UL>
All strings are assumed to be 0-terminated.

<B>Note:</B> The MuPAD kernel is \em no UNICODE application, even if 
UNICODE encoded strings are handled via the MuPAD kernel client 
interface. UNICODE strings are always converted to the users locale 
by the kernel. (The kernel can handle multi-byte strings, thus Japanese
strings are handled correctly under a Japanese locale for example.)
*/

/* @{ */

/** @var typedef struct MUP_KernelImpl * MUP_Kernel
 * An opaque handle to a MuPAD kernel.
 * This handle represents a single clients connection to a MuPAD kernel.
 * Different MUP_Kernel variables may exist which connect to the
 * same kernel. */
#ifdef __cplusplus
class MUP_KernelImpl;
typedef class MUP_KernelImpl * MUP_Kernel;
#else
typedef struct MUP_KernelImpl * MUP_Kernel;
#endif

/** A byte. */
typedef unsigned char MUP_Byte;

/** A signed 32bit integer. */
typedef int MUP_Int;

/** A boolean encoded as signed 32bit integer. */
typedef int MUP_Bool;

/** A cookie handled between caller and callback. */
typedef void * MUP_Cookie;

#ifdef MUP_UTF16_WCHAR_T

#  ifndef MUP_UNICODE
#    define MUP_UNICODE
#  endif
#  ifndef MUP_UTF16
#    define MUP_UTF16
#  endif

#  include <stddef.h>

typedef wchar_t MUP_Char;

#else /* not MUP_UTF16_WCHAR_T */

#  ifdef MUP_UTF16

#    ifndef MUP_UNICODE
#      define MUP_UNICODE
#    endif

typedef unsigned short MUP_Char;

#  else /* not MUP_UTF16 */

/** @var typedef char MUP_Char
 * Characters of non-ASCII strings.
 * Their econding depends on the macros \c MUP_UNICODE, \c MUP_UTF16 and 
 * \c MUP_UTF16_WCHAR_T. All plain \c char strings are ASCII encoded. */
typedef char MUP_Char;

#  endif /* not MUP_UTF16 */

#endif /* not MUP_UTF16_WCHAR_T */

/* @} */

/* ----------------------------------------------------------------------- */
/** @defgroup out_types Output Types

@brief Constants to identify the different types of MuPAD output.

The kernel produces several predefined types of output, encoded by this 
string constants.
*/

/* @{ */

/** Result of a MuPAD command executed. */
const char * const MUP_ot_result = "_res";

/** A side effect output, usually produced with the MuPAD function \c print. */
const char * const MUP_ot_print = "_prt";

/** An error message after an evaluation or syntax error. */
const char * const MUP_ot_error = "_err";

/** A warning message. */
const char * const MUP_ot_warning = "_wrn";

/** Debugger protocol output (not pretty-printed). */
const char * const MUP_ot_debug = "_dbg";

/** Debugger protocol output (pretty-printed). */
const char * const MUP_ot_dbg_pretty = "_dpr";

/** Debugger error message. */
const char * const MUP_ot_dbg_err = "_der";

/** Other text output, not identified. */
const char * const MUP_ot_other = "_oth";

/* @} */

/* ----------------------------------------------------------------------- */
/** @defgroup data_fmts Input and Output Data Formats

@brief Constants to identify data formats used for MuPAD input and output.

MuPAD input and output may be encoded in different formats. The string
constants below encode the types of the predefined formats.

Data may be encoded either as text strings or binary data. The only binary
encoding which currently exists is MCode, a binary serialization of MuPAD 
expression trees.

Valid output data types are: #MUP_fn_null, #MUP_fn_text, #MUP_fn_mcode_tree 
and #MUP_fn_mcode_content.

Valid input data types are: #MUP_fn_mupad_lang and #MUP_fn_mcode_tree.

The formats #MUP_fn_text and #MUP_fn_mupad_lang are encoded as
text strings. The formats #MUP_fn_mcode_tree and #MUP_fn_mcode_content
are encoded as binary MCode data. Binary data is handled via MUP_Data.
*/

/* @{ */

/** Empty format: The output is ignored. */
const char * const MUP_fn_null = "_nul";

/** Free-form text output of unknown semantics.
 * This is the format of the kernels text output, because it is not valid
 * MuPAD input in some cases (even if not pretty-printed). */
const char * const MUP_fn_text = "_txt";

/** Text containing MuPAD language statements. */
const char * const MUP_fn_mupad_lang = "_lan";

/** General expression tree serialized as a binary MCode data. */
const char * const MUP_fn_mcode_tree = "_tre";

/** Layout tree serialized as a binary MCode data.
 * Output for 2-dimensional TeX-like expression layout is encoded in special
 * MuPAD trees. These are serialized into MCode data. */
const char * const MUP_fn_mcode_content = "_con";


/** Binary data transferred between client and kernel.
 * Currently MCode is the only form of binary data supported.
 */
typedef struct MUP_DataTAG {
    const char * format;   /**< Format of the data.
                            * Must be either #MUP_fn_mcode_tree 
                            * or #MUP_fn_mcode_content. */
    const MUP_Byte * data; /**< Data bytes. */
    MUP_Int len;           /**< Lenght of the data in bytes. */
} MUP_Data;

/* @} */

/* ----------------------------------------------------------------------- */
/** @defgroup client_features Client Features

@brief Flags describing the (user interface) features a client supports.

Different clients may support different features (mainly user interface
features). These flags encode the features a client may want to support 
and the kernel must know about. Use the bitwise or operator to combine
the feature flags.
*/

/* @{ */

/** Type to encode client features. */
typedef MUP_Int MUP_ClientFeatures;

/** 0 signals void features. */
const MUP_ClientFeatures MUP_cf_none = 0;

/** Kernel should stay alive after the client exits. */
const MUP_ClientFeatures MUP_cf_stay_alive = 0x01;
/** Client allows user to complete incomplete command lines. */
const MUP_ClientFeatures MUP_cf_complete_commands = 0x02;
/** Client can debug. */
const MUP_ClientFeatures MUP_cf_can_debug = 0x04;
/** Client can start debug mode after initialization. */
const MUP_ClientFeatures MUP_cf_can_lazy_debug = 0x08;
/** Client has debugger user-interface. */
const MUP_ClientFeatures MUP_cf_debugger_ui = 0x10;
/** Client can receive and render plotting data. */
const MUP_ClientFeatures MUP_cf_can_plot = 0x20;
/** Client can query \c input from user. */
const MUP_ClientFeatures MUP_cf_can_input = 0x0400;
/** Client can query \c textinput from user. */
const MUP_ClientFeatures MUP_cf_can_textinput = 0x0800;
/** Client can show help. */
const MUP_ClientFeatures MUP_cf_can_help = 0x1000;
/** Client can show hypertext help. */
const MUP_ClientFeatures MUP_cf_can_help_dvi = 0x2000;
/** Client has help browser. */
const MUP_ClientFeatures MUP_cf_can_browse_help = 0x4000;
/** Client can execute system command. */
const MUP_ClientFeatures MUP_cf_can_exec_system = 0x8000;
/** Client is allowed to quit the kernel via MuPAD \c quit command. */
const MUP_ClientFeatures MUP_cf_can_quit = 0x010000;
/** Client can execute callbacks via the kernels \c callback message. */
const MUP_ClientFeatures MUP_cf_can_callback = 0x020000;
/** Client can print expressions in typesetting mode. */
const MUP_ClientFeatures MUP_cf_can_typeset = 0x040000;

/* @} */

/* ----------------------------------------------------------------------- */

/* Debugger command and info types.
 *
 * This types are described in $CVSROOT/DEV/CLIENTS/MUP_LIB/DOC/prot_dbg.h.
 */

const MUP_Byte MUP_dc_clear           = 'C';
const MUP_Byte MUP_dc_clear_all       = 'a';
const MUP_Byte MUP_dc_continue        = 'c';
const MUP_Byte MUP_dc_next            = 'n';
const MUP_Byte MUP_dc_step            = 's';
const MUP_Byte MUP_dc_finish          = 'f';
const MUP_Byte MUP_dc_where           = 'w';
const MUP_Byte MUP_dc_up              = 'u';
const MUP_Byte MUP_dc_down            = 'd';
const MUP_Byte MUP_dc_display         = 'D';
const MUP_Byte MUP_dc_display_replace = 'R';
const MUP_Byte MUP_dc_undisplay       = 'U';
const MUP_Byte MUP_dc_undisplay_all   = 'N';
const MUP_Byte MUP_dc_list            = 'b';
const MUP_Byte MUP_dc_list_proc       = 'l';
const MUP_Byte MUP_dc_autolist        = 't';
const MUP_Byte MUP_dc_autodisplay     = 'T';
const MUP_Byte MUP_dc_stop_at         = 'S';
const MUP_Byte MUP_dc_quit            = 'q';
const MUP_Byte MUP_dc_print           = 'p';
const MUP_Byte MUP_dc_pretty_print    = 'P';
const MUP_Byte MUP_dc_goto_proc       = 'g';
const MUP_Byte MUP_dc_execute         = 'e';
const MUP_Byte MUP_dc_tty_help        = '?';
const MUP_Byte MUP_dc_level           = 'L';
const MUP_Byte MUP_dc_breakpoints     = 'X';
const MUP_Byte MUP_dc_display_list    = 'Y';
const MUP_Byte MUP_dc_call_stack      = 'Z';
const MUP_Byte MUP_dc_activate        = 'x';
const MUP_Byte MUP_dc_deactivate      = 'y';

/* ----------------------------------------------------------------------- */
/** @defgroup exec_flags Command Execution Flags

@brief Constants describing special features modifying command execution.

When executing a command via MUP_send_cmd_ext() one may
specify additional execution features by combining this flags.
*/

/* @{ */

/** Type to encode command execution flags. */
typedef MUP_Int MUP_ExecutionFlags;

/** 0 signals default command execution. */
const MUP_ExecutionFlags MUP_ef_default = 0;

/** Do not insert command and result into history table.
 * Usually any command and its result is inserted into the kernels
 * history table, see the MuPAD function \c history. Specify this flag 
 * to avoid insertion into the history. */
const MUP_ExecutionFlags MUP_ef_no_history = 0x1;

/* @} */

/* ----------------------------------------------------------------------- */
/** @defgroup mssg_hdl Types of Message Handlers 

@brief Types of callbacks which handle kernel messages.

Messages sent by the kernel to the client must be handled by callback
funtions. The possible callback function types are described here.

Use the <A HREF="group__set__mssg__hdl.html"><CODE>MUP_set_..._handler</CODE> 
functions</A> to set message handler callbacks for a kernel handle.

<B><A HREF="prot.html#kernel">&gt;&gt; Description of the existing kernel 
messages.</A></B>
*/

/* @{ */

/** Init wanted message handler.
 * The default handler sends an empty argument list to the
 * kernel.
 * @param kernel kernel which sent the message */
typedef void (MUP_CALLBACK * MUP_on_init_wanted_handler)(
                                    MUP_Kernel kernel);

/** Kernel ready message handler.
 * The default handler does nothing.
 * @param kernel kernel which sent the message */
typedef void (MUP_CALLBACK * MUP_on_kernel_ready_handler)(
                                    MUP_Kernel kernel);

/** Output message handler for text output.
 * Is called on text output from the kernel.
 * The default handler does nothing.
 * @param kernel kernel which sent the message
 * @param cmd_cookie cookie of the command which produced this output
 * @param output_type output type
 * @param output_format output format
 * @param output_text output text */
typedef void (MUP_CALLBACK * MUP_on_output_handler)(
                                    MUP_Kernel kernel,
                                    MUP_Cookie cmd_cookie, 
                                    const char * output_type,
                                    const char * output_format,
                                    const MUP_Char * output_text);

/** Output message handler for binary output.
 * Is called on binary non-text output from the kernel.
 * The default handler does nothing.
 * @param kernel kernel which sent the message
 * @param cmd_cookie cookie of the command which produced this output
 * @param output_type output type
 * @param output the output */
typedef void (MUP_CALLBACK * MUP_on_output_data_handler)(
                                    MUP_Kernel kernel,
                                    MUP_Cookie cmd_cookie, 
                                    const char * output_type,
                                    const MUP_Data * output);

/** Finished message handler.
 * The default handler does nothing.
 * @param kernel kernel which sent the message
 * @param cmd_cookie cookie of the command which has been executed */
typedef void (MUP_CALLBACK * MUP_on_finished_handler)(
                                    MUP_Kernel kernel,
                                    MUP_Cookie cmd_cookie);

/** Complete command message handler.
 * The default handler does nothing.
 * @param kernel kernel which sent the message
 * @param cmd_cookie cookie of the command which is to be completed */
typedef void (MUP_CALLBACK * MUP_on_complete_cmd_handler)(
                                    MUP_Kernel kernel,
                                    MUP_Cookie cmd_cookie);

/** Input wanted message handler.
 * The default handler sends a canceled input message to
 * the kernel.
 * @param kernel kernel which sent the message
 * @param cmd_cookie cookie of the command which caused the execution
 *          of the input function
 * @param text_input free-form text or MuPAD command wanted as input
 * @param prompt prompt text which should be displayed */
typedef void (MUP_CALLBACK * MUP_on_input_wanted_handler)(
                                    MUP_Kernel kernel,
                                    MUP_Cookie cmd_cookie,
                                    MUP_Bool text_input,
                                    const MUP_Char * prompt);

/** Callback message handler.
 * The default handler sends an error result back to
 * the kernel.
 * @param kernel kernel which sent the message
 * @param cmd_cookie cookie of the command which caused the execution
 *          of the #clientCall function
 * @param args MCode data containing the arguments of the callback */
typedef void (MUP_CALLBACK * MUP_on_callback_handler)(
                                    MUP_Kernel kernel,
                                    MUP_Cookie cmd_cookie,
                                    const MUP_Data * args);

/** Help message handler.
 * The default handler does nothing.
 * @param kernel kernel which sent the message
 * @param topic help topic */
typedef void (MUP_CALLBACK * MUP_on_help_handler)(
                                    MUP_Kernel kernel,
                                    const MUP_Char * topic);

/** System message handler.
 * The default handler does not execute the command
 * but sends a system_result(1) message back to the kernel.
 * @param kernel kernel which sent the message
 * @param cmd system command to be executed */
typedef void (MUP_CALLBACK * MUP_on_system_handler)(
                                    MUP_Kernel kernel,
                                    const MUP_Char * cmd);

/** Frame changed message handler.
 * The default handler does nothing.
 * @param kernel kernel which sent the message
 * @param frame new frame of client. */
typedef void (MUP_CALLBACK * MUP_on_frame_changed_handler)(
                                    MUP_Kernel kernel,
                                    const MUP_Char * frame);

/** Textwidth changed message handler.
 * The default handler does nothing.
 * @param kernel kernel which sent the message
 * @param width new value of \c TEXTWIDTH */
typedef void (MUP_CALLBACK * MUP_on_textwidth_changed_handler)(
                                    MUP_Kernel kernel,
                                    MUP_Int width);

/** Pretty-print changed message handler.
 * The default handler does nothing.
 * @param kernel kernel which sent the message
 * @param pretty new value of \c PRETTYPRINT */
typedef void (MUP_CALLBACK * MUP_on_pretty_print_changed_handler)(
                                    MUP_Kernel kernel,
                                    MUP_Bool pretty);

/** Format changed message handler.
 * The default handler does nothing.
 * @param kernel kernel which sent the message
 * @param output_type output type
 * @param format new format of the output of the given type */
typedef void (MUP_CALLBACK * MUP_on_format_changed_handler)(
                                    MUP_Kernel kernel,
                                    const char * output_type,
                                    const char * format);

/** Select format message handler.
 * The default handler randomly selects the first format.
 * @param kernel kernel which sent the message
 * @param output_type output type for which to select a format
 * @param num_formats number of formats to choose from
 * @param formats array of formats to choose from */
typedef void (MUP_CALLBACK * MUP_on_select_format_handler)(
                                    MUP_Kernel kernel,
                                    const char * output_type,
                                    MUP_Int num_formats,
                                    const char ** formats);

/** Status message handler.
 * The default handler does nothing.
 * @param kernel kernel which sent the message
 * @param used_mem number of bytes allocated logically
 * @param res_mem number of bytes allocated physically
 * @param sec computation time */
typedef void (MUP_CALLBACK * MUP_on_status_handler)(
                                    MUP_Kernel kernel,
                                    MUP_Int used_mem,
                                    MUP_Int res_mem,
                                    MUP_Int sec);

/** Input completion message handler.
 * The \a list string encodes the possible completions. Possible cases are:
 * <DL>
 * <DT>An empty string </DT> <DD>Only one possible completion exists, which is
 *    given by \a completion.
 * <DT>The string "\\007" </DT> <DD>No completions exist. \a completion is empty.
 * <DT>A string containing a comma-separated list of names </DT> <DD>The given
 *    completions are possible. \a completion contains their common prefix.
 * </DL>
 * The default handler does nothing.
 * @param kernel kernel which sent the message
 * @param list list of possible completions (may be empty)
 * @param completion common prefix of all possible completions */
typedef void (MUP_CALLBACK * MUP_on_input_completion_handler)(
                                    MUP_Kernel kernel,
                                    const MUP_Char * list,
                                    const MUP_Char * completion);

/** Plot result file message handler.
 * The default handler does nothing.
 * @param kernel kernel which sent the message
 * @param cmd_cookie cookie of plot command which created the data file
 * @param fname name of plot data file */
typedef void (MUP_CALLBACK * MUP_on_plot_result_file_handler)(
                                    MUP_Kernel kernel,
                                    MUP_Cookie cmd_cookie,
                                    const MUP_Char * fname);

/** Debug command wanted message handler.
 * The default handler stops debugging by sending a continue
 * debug message to the kernel.
 * @param kernel kernel which sent the message
 * @param cmd_cookie cookie of the command which caused the execution
 *          of the \c debug function */
typedef void (MUP_CALLBACK * MUP_on_dbg_cmd_wanted_handler)(
                                    MUP_Kernel kernel,
                                    MUP_Cookie cmd_cookie);

/** Debug info message handler.
 * The default handler does nothing.
 * @param kernel kernel which sent the message
 * @param cmd type of info
 * @param fname file name (may be NULL or empty)
 * @param pos line number in given file (may be 0)
 * @param expr string containing MuPAD expression
 * @param level actual procedure stack frame level
 * @param depth actual depth of procedure call stack */
typedef void (MUP_CALLBACK * MUP_on_dbg_info_handler)(
                                    MUP_Kernel kernel,
                                    MUP_Int cmd,
                                    const MUP_Char * fname,
                                    MUP_Int pos,
                                    const MUP_Char * expr,
                                    MUP_Int level,
                                    MUP_Int depth);

/** Debugger display list message handler.
 * The default handler does nothing.
 *
 * Each variable is given by (\a names[i], \a values[i]) ---
 * variable \a i with name \a names[i] and value \a values[i].
 *
 * @param kernel kernel which sent the message
 * @param num_vars number of display variables
 * @param names array of \a num_vars variable names
 * @param values array of \a num_vars actual variable values */
typedef void (MUP_CALLBACK * MUP_on_display_list_handler)(
                                    MUP_Kernel kernel,
                                    MUP_Int num_vars, 
                                    const MUP_Char ** names,
                                    const MUP_Char ** values);

/** Debugger breakpoint list message handler.
 * The default handler does nothing.
 *
 * Each breakpoint is given by (\a fnames[i], \a lines[i], \a conditions[i]) ---
 * breakpoint \a i in source file \a fnames[i] at line \a lines[i] with
 * condition \a conditions[i]. A condition may be empty.
 *
 * @param kernel kernel which sent the message
 * @param num_bpts number of breakpoints
 * @param fnames array of \a num_bpts source file names
 * @param lines array of \a num_bpts source file line numbers
 * @param conditions array of \a num_bpts condition expressions */
typedef void (MUP_CALLBACK * MUP_on_breakpoint_list_handler)(
                                    MUP_Kernel kernel,
                                    MUP_Int num_bpts, 
                                    const MUP_Char ** fnames,
                                    const MUP_Int * lines,
                                    const MUP_Char ** conditions);

/** Debugger call stack message handler.
 * The default handler does nothing.
 *
 * For each stack frame in the procedure call stack the information
 * (\a fnames[i], \a lines[i], \a procnames[i]) is sent -- frame \a i
 * executing procedure named \a procnames[i] in source file
 * \a fnames[i] at line \a lines[i].
 *
 * @param kernel kernel which sent the message
 * @param level actual procedure call stack level
 * @param num_calls number of procedure stack frames
 * @param fnames array of \a num_calls source file names
 * @param lines array of \a num_calls source file line numbers
 * @param procnames array of \a num_calls procedure names */
typedef void (MUP_CALLBACK * MUP_on_call_stack_handler)(
                                    MUP_Kernel kernel,
                                    MUP_Int level, 
                                    MUP_Int num_calls, 
                                    const MUP_Char ** fnames,
                                    const MUP_Int * lines,
                                    const MUP_Char ** procnames);

/** Reset message handler.
 * The default handler does nothing.
 * @param kernel kernel which sent the message */
typedef void (MUP_CALLBACK * MUP_on_reset_handler)(
                                    MUP_Kernel kernel);

/** Quit message handler.
 * The default handler does nothing.
 * @param kernel kernel which sent the message */
typedef void (MUP_CALLBACK * MUP_on_quit_handler)(
                                    MUP_Kernel kernel);

/* @} */

/* ----------------------------------------------------------------------- */

/** @defgroup lib_krnl Creating a Shared Library Kernel
 *
 * Creating kernel handles connecting to a shared library kernel.
 *
 * A shared library kernel is a kernel implemented as an in-process server 
 * housed in a DLL or shared library.
 *
 * For each client process there may exist several objects of this class,
 * all connected to exactly one common kernel. This implies that there
 * may exist only one global kernel state in the client process (ie. one
 * single state which holds all MuPAD variables).
 *
 * This kernel handle type is \em not thread safe, it should not be called
 * from different threads.
 */

/* @{ */

/** Get a new client connection to the library kernel.
 *
 * The \a path argument is used to set up defaults for the MuPAD
 * library path etc. One may use NULL for it, but in this case one must be 
 * shure to set up the library path correctly during the initialization of 
 * the first client via the #MUP_on_init_wanted_handler handler attached 
 * to the kernel handle.
 *
 * If a \a path is given the MuPAD library is searched for in the
 * folder <tt>../lib/</tt> relative to it. This folder must contain
 * the files <tt>sysinit.mu</tt> and <tt>lib.tar</tt>. If one wants
 * to store the library at a different location one must use the '<tt>-l</tt>'
 * option via the #MUP_on_init_wanted_handler handler attached to the kernel
 * handle.
 *
 * The kernel is initialized when the first client connection is created
 * via MUP_send_new_client().
 *
 * @param path full path name of executable using the library or NULL
 * @return handle to kernel, \c NULL in case of an error
 */
MUP_EXTERN MUP_Kernel MUP_API MUP_get_lib_kernel(
                                const MUP_Char * path);

/** Event handler callback for kernel.
 * @param call_data call data passed through from client */
typedef void (MUP_CALLBACK * MUP_event_handler)(MUP_Cookie user_data);

/** Fatal exit callback for kernel. */
typedef void (MUP_CALLBACK * MUP_fatal_exit_handler)();

/** Set event handler for kernel library.
 * This installs a callback which is called quite often by the kernel
 * during longer calculations. It may be used to keep a user interface
 * reactive for example. 
 * @param handler event handler callback
 * @param user_data user data passed through to callback
 * @return previous event handler */
MUP_EXTERN MUP_event_handler MUP_API MUP_set_event_handler(
                                    MUP_event_handler handler,
                                    MUP_Cookie user_data);

/** Set fatal exit handler for kernel library.
 * This installs a callback which is executed when the kernel
 * causes a fatal error. The previous exit handler is returned.
 * The default handler does nothing.
 *
 * When the \a handler returns to the caller \c exit is called in any
 * case. This is because the kernel is in the middle of nowhere
 * and execution of kernel code can not be resumed at this point.
 *
 * One must execute a \c longjmp or similar in the exit handler
 * to shut down the kernel library gracefully.
 * @param handler fatal exit handler callback
 * @return previous exit handler */
MUP_EXTERN MUP_fatal_exit_handler MUP_API MUP_set_fatal_exit_handler(
                                    MUP_fatal_exit_handler handler);

/** Tear down MuPAD kernel library.
 * Frees any global resources allocated by the kernel library. One my 
 * \em not create kernel handles after calling this function. May be 
 * called several times.
 *
 * Under Windows you do not need to call this function, it is called when
 * the kernel DLL is unloaded. */
MUP_EXTERN void MUP_API MUP_exit_lib();

/* @} */

#ifdef WIN32
#  ifdef MUP_COM_KERNEL

/** @defgroup com_krnl Creating a COM Kernel
 *
 * Creating kernel handles connecting to a COM server kernel under Windows.
 *
 * A kernel handle created with this functions represents a client connection
 * to a kernel which is implemented as a COM out-of-process server. The 
 * communication to a COM kernel uses the COM-based protocol introduced with
 * MuPAD 3.0. Kernels may be started on remote servers using DCOM.
 *
 * For each client process there may exist several handles of this type,
 * connected to one or more kernel processes. One may identify the kernels
 * by name and enumerate them. Note that the kernel names are visible only
 * in the current process, one may \em not enumerate kernel processes
 * which are not connected to the current process.
 *
 * This handles are single-appartment thread safe (ie. can be used in 
 * single-appartment COM objects).
 *
 * If an error occures in a COM call the \c HRESULT of the call is either
 * returned or can be obtained via MUP_get_com_hresult().
 *
 * \b Note: One must define the macro \c MUP_COM_KERNEL before including
 * \c mupkernel.h in order to be able to use the COM-related functions.
 *
 * \b Note: The calling application must initialize the COM library by
 * calling \c CoInitialize or \c CoInitializeEx before calling any of the
 * COM-related functions.
 */

/* @{ */

/** Start and connect to new COM kernel.
 *
 * The kernel name may be NULL, in this case a new unique default name is choosen.
 *
 * If \a server is NULL or \c L"" the kernel will be started on a default server,
 * which may be a remote server depending on the \c RemoteServerName
 * registry setting for the kernel. If the server name is \c L"." a local
 * server will be started in any case.
 *
 * \c NULL is returned if a kernel with the name given already existed.
 *
 * @param clsid_kernel the CLSID of the COM kernel server
 * @param name name of the kernel, may be NULL
 * @param server name of server where to launch the kernel (may be NULL)
 * @return handle to kernel, \c NULL in case of an error
 */
MUP_EXTERN MUP_Kernel MUP_API MUP_new_com_kernel(
                                const CLSID & clsid_kernel,
                                const MUP_Char * name,
                                const wchar_t * server);

/** Get a new client connection to the named COM kernel.
 *
 * If a kernel with the given name already exists a connection to that kernel is
 * established. A new kernel is started if the named one does not exist and \a start
 * is true. The kernel is started on the server \a server.
 *
 * If \a server is NULL or \c L"" the kernel will be started on a default server,
 * which may be a remote server depending on the \c RemoteServerName
 * registry setting for the kernel. If the server name is \c L"." a local
 * server will be started in any case.
 *
 * \c NULL is returned if no kernel with the name exists.
 *
 * @param clsid_kernel the CLSID of the COM kernel server
 * @param name name of the kernel to connect to
 * @param start start new kernel if named one does not exist
 * @param server name of server where to launch the kernel (may be NULL)
 * @return handle to kernel, \c NULL in case of an error
 */
MUP_EXTERN MUP_Kernel MUP_API MUP_get_com_kernel(
                                const CLSID & clsid_kernel,
                                const MUP_Char * name,
                                MUP_Bool start,
                                const wchar_t * server);

/** Get a new client connection to a COM kernel given by interface.
 *
 * A new client connection to a running kernel given by a COM 
 * interface pointer is established. One may obtain an appropriate interface
 * pointer via MUP_get_com_interface().
 *
 * @param interf COM interface of a running kernel
 * @param name name of the kernel, may be NULL
 * @return handle to kernel, \c NULL in case of an error
 */
MUP_EXTERN MUP_Kernel MUP_API MUP_get_com_kernel_from_interface(
                                IUnknown * interf,
                                const MUP_Char * name);

/** Get the names of the COM kernels which are currently connected to the
 * clients process. 
 * Returns an array of C-strings where the last entry is \c NULL.
 * Both the strings and the array must be freed by the caller using MUP_free() !
 * @return array of strings, last entry is \c NULL, 
 *         or \c NULL in case of an error
 */
MUP_EXTERN MUP_Char ** MUP_API MUP_get_com_kernel_names();


/** Returns true if this machine has DCOM installed.
 * If DCOM is installed one may access or start a kernel on a remote
 * server.
 * @return true if DCOM is available */
MUP_EXTERN MUP_Bool MUP_API MUP_has_dcom();

/** Get \c HRESULT of last COM call.
 * May be used directly after a failed function call to obtain the \c HRESULT
 * of the last COM call.
 * @param kernel kernel
 * @return HRESULT of the last COM call */
MUP_EXTERN HRESULT MUP_API MUP_get_com_hresult(
                                MUP_Kernel kernel);

/** Query \c IUnknown COM interface from kernel.
 * @param kernel kernel
 * @param interf returns \c IUnknown COM interface of kernel
 * @return \c S_OK in case of success, \c HRESULT of failing COM call otherwise
 */
MUP_EXTERN HRESULT MUP_API MUP_get_com_interface(
                                MUP_Kernel kernel,
                                IUnknown** interf);

/* @} */

#  endif /* MUP_COM_KERNEL */

#endif /* WIN32 */

/** @defgroup wmsg_krnl Creating a Windows Messages Kernel
 *
 * Creating kernel handles connecting to a Windows messages server kernel.
 *
 * A kernel handle created with this functions represents a client connection
 * to a kernel which is implemented as an out-of-process server under Windows.
 * The communication to a Windows messages kernel uses Windows messages. This
 * protocol has been introduced with MuPAD 3.1.
 *
 * In order to send and receive messages from the kernel process the kernels
 * handle must live in a UI thread which has a message loop. A hidden window
 * is created for the UI thread which receives Windows messages from the 
 * kernel. Thus communication between this object and the kernel process
 * can only happen if Windows messages are processed by the message loop.
 *
 * For each client process there may exist several kernel handles of this
 * type, connected to one or more kernel server processes. One may identify
 * the kernels by name and enumerate them.
 *
 * This class is single-appartment thread safe (i.e. can be used in single-
 * appartment COM objects).
 */

/* @{ */

/** Start and connect to new Windows messages kernel.
 *
 * The kernel name may be \c NULL, in this case a new unique default name is 
 * choosen.
 *
 * \c NULL is returned if a kernel with the name given already existed.
 *
 * @param kernel_path full path of kernel exe to start
 * @param name name of the kernel, may be \c NULL
 * @return handle to kernel, \c NULL in case of an error
 */
MUP_EXTERN MUP_Kernel MUP_API MUP_new_wmsg_kernel(
                                const MUP_Char * kernel_path,
                                const MUP_Char * name);

/** Get a new client connection to the named Windows messages kernel.
 *
 * A new kernel is started if the named one does not exist and \a start
 * is true.
 *
 * \c NULL is returned if no kernel with the name exists.
 *
 * @param kernel_path full path of kernel exe to start
 * @param name name of the kernel to connect to
 * @param start start new kernel if named one does not exist
 * @return handle to kernel, \c NULL in case of an error
 */
MUP_EXTERN MUP_Kernel MUP_API MUP_get_wmsg_kernel(
                                const MUP_Char * kernel_path,
                                const MUP_Char * name, 
                                MUP_Bool start);

/** Get the names of the kernels the client process is connected to.
 * Returns an array of C-strings where the last entry is \c NULL. The strings
 * and the array must be freed by the caller using MUP_free() !
 * @return array of strings, last entry is \c NULL, 
 *         or \c NULL in case of an error
 */
MUP_EXTERN MUP_Char ** MUP_API MUP_get_wmsg_kernel_names();

/* @} */

/** @defgroup uds_krnl Creating a UNIX Domain Socket Kernel
 *
 * Creating kernel handles connecting to a UNIX domain socket server kernel.
 *
 * A kernel handle created with this functions represents a client connection
 * to a kernel which is implemented as an out-of-process server under UNIX or
 * Linux. The communication with such a kernel uses a protocol based in UNIX 
 * domain sockets (it has been introduced with MuPAD 3.1).
 *
 * This kernel handle communicates with its kernel process via sockets.
 * In order to send and receive messages from the kernel process the
 * function MUP_handle_uds_input() must be called regularly, for example in
 * the X event loop of an application with user interface.
 *
 * MUP_handle_uds_input() uses \c select to query for incoming messages from
 * the kernels socket. If one has another means to call \c select for the 
 * sockets one may also use MUP_read_uds_input() to read and handle incoming
 * messages.
 *
 * For each client process there may exist several handles of this type,
 * connected to one or more kernel processes. One may identify the kernels
 * by name and enumerate them.
 *
 * This class is not thread safe, you should not use these handles from
 * different threads.
 */

/* @{ */

/** Start and connect to new UDS kernel.
 *
 * A new UDS kernel process is created. The method blocks until the kernel
 * process can handle messages. (The \c SIGUSR1 signal is used internally
 * to signal that the kernel process is ready.)
 *
 * The kernel name may be \c NULL, in this case a new unique default name
 * is choosen.
 *
 * \c NULL is returned if a kernel with the name given already existed.
 *
 * @param kernel_path path name of kernel executable to start
 * @param name name of the kernel, may be \c NULL
 * @return handle to kernel, \c NULL in case of an error
 */
MUP_EXTERN MUP_Kernel MUP_API MUP_new_uds_kernel(
                                const char * kernel_path,
                                const MUP_Char * name);

/** Get a new client connection to the named UDS kernel.
 *
 * A new kernel is started if the named one does not exist and \a start
 * is true.
 *
 * \c NULL is returned if no kernel with the name exists.
 *
 * @param kernel_path path name of kernel executable to start
 * @param name name of the kernel to connect to
 * @param start start new kernel if named one does not exist
 * @return handle to kernel, \c NULL in case of an error
 */
MUP_EXTERN MUP_Kernel MUP_API MUP_get_uds_kernel(
                                const char * kernel_path,
                                const MUP_Char * name,
                                MUP_Bool start);

/** Get a new client connection to running UDS kernel.
 *
 * Connect to a running kernel process which listens on the given socket
 * path for new connections.
 *
 * \c NULL is returned if the kernel could not be contacted.
 *
 * @param listen_path socket path name of kernel to connect to
 * @param name name of the kernel to connect to
 * @return handle to kernel, \c NULL in case of an error
 */
MUP_EXTERN MUP_Kernel MUP_API MUP_connect_uds_kernel(
                                const char * listen_path,
                                const MUP_Char * name);

/** Get the names of the kernels the client process is connected to.
 * Returns an array of C-strings where the last entry is \c NULL. The strings
 * and the array must be freed by the caller using MUP_free() !
 * @return array of C-strings, last entry is \c NULL,
 *         or \c NULL in case of an error
 */
MUP_EXTERN MUP_Char ** MUP_API MUP_get_uds_kernel_names();

/** Receive messages from kernel and handle it.
 * Call this method regularly in order to receive messages from the
 * kernel processes. This method calls \c select to see if data is
 * available from some kernel.
 * @param block block until data is available */
MUP_EXTERN void MUP_API MUP_handle_uds_input(
                                MUP_Bool block);

/** Read and process kernel messages from the given file descriptor.
 * This call will \em block if no data is available. 
 * This method may be useful if you want to call \c select on the kernel
 * file descriptors from some 'outer' infrastructure instead of calling
 * handle_uds_input(). */
MUP_EXTERN void MUP_API MUP_read_uds_input(
                                int fd);

/** Get file descriptor for reading kernel messages.
 * May be used to call \c select to handle incoming messages. 
 * Please note that all clients of the same kernel share this descriptor.
 * @param kernel kernel */
MUP_EXTERN int MUP_API MUP_get_client_fd(
                                MUP_Kernel kernel);

/** Get socket path name where the kernel process listens for new 
 * connections.
 * @param kernel kernel */
MUP_EXTERN const char * MUP_API MUP_get_listen_path(
                                MUP_Kernel kernel);

/* @} */

/* ----------------------------------------------------------------------- */
/** @defgroup manage_krnl Managing the Kernel */

/* @{ */

/** Get the ID of this client.
 * The ID returned is the ID this client has on the kernels side.
 * @param kernel kernel handle
 * @return the client ID, a non-negative number, or -1 if this
 *   object is not connected to the kernel */
MUP_EXTERN MUP_Int MUP_API MUP_get_client_id(
                                MUP_Kernel kernel);

/** Set client data associated with kernel handle.
 * The \a client_data is stored with the kernel handle. It may be used
 * to associate arbitary data with the kernel handle.
 * @param kernel kernel handle
 * @param client_data client data
 * @return true if data was set successfully */
MUP_EXTERN MUP_Bool MUP_API MUP_set_client_data(
                                MUP_Kernel kernel,
                                MUP_Cookie client_data);

/** Get client data associated with kernel handle.
 * @param kernel kernel handle
 * @return client data */
MUP_EXTERN MUP_Cookie MUP_API MUP_get_client_data(
                                MUP_Kernel kernel);

/** Set kernel name.
 * Set name of the kernel. This is a local name (i.e. visible in the
 * client process only) which allows to identify different kernels
 * of the running process.
 * @param kernel kernel handle
 * @param name (local) name of kernel handle, may be NULL
 * @return true if name was set successfully */
MUP_EXTERN MUP_Bool MUP_API MUP_set_kernel_name(
                                MUP_Kernel kernel,
                                const MUP_Char * name);

/** Get kernel name.
 * @param kernel kernel handle
 * @return kernels name, may be NULL */
MUP_EXTERN const MUP_Char * MUP_API MUP_get_kernel_name(
                                MUP_Kernel kernel);

/** Send a new client message.
 * A new client is registered with the kernel. This method must be called
 * exactly once for each kernel handle. The kernel will send an \c init_wanted
 * message if this is its first client. When the kernel is ready to receive
 * commands it will send a \c kernel_ready message.
 * @param kernel kernel handle
 * @param name name of this client (used in the kernel to identify the client)
 * @param frame frame (context) of the client after initialization
 * @param features client feature flags
 * @return true if client was registered successfully */
MUP_EXTERN MUP_Bool MUP_API MUP_send_new_client(
                                MUP_Kernel kernel,
                                const MUP_Char * name,
                                const MUP_Char * frame,
                                MUP_ClientFeatures features);

/** Send a new client message defining extended MCode options.
 * Rarely used to define non-standard options for the encoding of the MCode
 * data streams used for non-text input and output. Otherwise the same 
 * conditions as for MUP_send_new_client() hold.
 * @param kernel kernel handle
 * @param name name of this client (used in the kernel to identify the client)
 * @param frame frame (context) of the client after initialization
 * @param features client feature flags
 * @param long_fmt format of longs in MCode streams used for communication
 * @param float_fmt format of floats in MCode streams
 * @param coding type of encoding to be used in MCode streams
 * @return true if client was registered successfully */
MUP_EXTERN MUP_Bool MUP_API MUP_send_new_client_ext(
                                MUP_Kernel kernel,
                                const MUP_Char * name,
                                const MUP_Char * frame,
                                MUP_ClientFeatures features,
                                MUP_Int long_fmt,
                                MUP_Int float_fmt,
                                MUP_Int coding);

/** Tear down and delete kernel connection.
 * Must be called for each kernel handle created.
 * @param kernel kernel handle */
MUP_EXTERN void MUP_API MUP_delete_kernel(
                                MUP_Kernel kernel);

/** Free memory allocated by the MuPAD kernel API. 
 * @param bytes memory to free */
MUP_EXTERN void MUP_API MUP_free(void* bytes);

/* @} */

/* ----------------------------------------------------------------------- */
/** @defgroup send_mssg Sending Messages 

Functions to send messages to a kernel.

<B><A HREF="prot.html#client">&gt;&gt; Description of the existing client 
messages.</A></B>
*/

/* @{ */

/** Send initialization arguments.
 * The format of the arguments is that of command line arguments.
 *
 * @param kernel kernel handle
 * @param argc number of initialization arguments
 * @param argv array of \a argc strings used as initialization arguments
 * @return true if message was sent successfully */
MUP_EXTERN MUP_Bool MUP_API MUP_send_init(
                                MUP_Kernel kernel,
                                MUP_Int argc,
                                const MUP_Char ** argv);

/** Send a MuPAD text command.
 * Sends a MuPAD language string as command to the kernel.
 *
 * @param kernel kernel handle
 * @param cmd_cookie cookie which may be used by the client to identify
 *    the results of the command
 * @param cmd MuPAD command
 * @return true if message was sent successfully */
MUP_EXTERN MUP_Bool MUP_API MUP_send_cmd(
                                MUP_Kernel kernel,
                                MUP_Cookie cmd_cookie,
                                const MUP_Char * cmd);

/** Send a MuPAD text command with extended options.
 * Sends a MuPAD language string as command to the kernel with extended 
 * options.
 *
 * The parameter \a flags may be used to control the execution of the
 * command. See the \c MUP_ef_... constants for possible values.
 *
 * The parameter \a out_format may be used to define the output format of
 * the result messages to be returned (independent of the actual
 * format settings). If NULL or an empty string is given the actual
 * output format for results is used. One will not receive any
 * \c format_changed messages if the format given is not the same as
 * the actual format for results.
 *
 * @param kernel kernel handle
 * @param cmd_cookie cookie which may be used by the client to identify
 *    the results of the command
 * @param cmd MuPAD command
 * @param flags evaluation flags (no special handling if 0)
 * @param  out_format output format to be used for result messages
 *    (the actual format is used if NULL or the empty string)
 * @return true if message was sent successfully */
MUP_EXTERN MUP_Bool MUP_API MUP_send_cmd_ext(
                                MUP_Kernel kernel,
                                MUP_Cookie cmd_cookie,
                                const MUP_Char * cmd,
                                MUP_ExecutionFlags flags,
                                const char * out_format);

/** Send a MuPAD non-text command.
 * Currently a MuPAD non-text command must have the format
 * #MUP_fn_mcode_tree (MCode expression tree).
 *
 * The parameter \a flags may be used to control the execution of the
 * command. See the \c MUP_ef_... constants for possible values.
 *
 * The parameter \a out_format may be used to define the output format of
 * the result messages to be returned (independent of the actual
 * format settings). If NULL or an empty string is given the actual
 * output format for results is used. One will not receive any
 * \c format_changed messages if the format given is not the same as
 * the actual format for results.
 *
 * @param kernel kernel handle
 * @param cmd_cookie cookie which may be used by the client to identify
 *    the results of the command
 * @param cmd binary data of non-text MuPAD command
 * @param flags evaluation flags (no special handling if 0)
 * @param out_format output format to be used for result messages
 *    (the actual format is used if NULL or the empty string)
 * @return true if message was sent successfully */
MUP_EXTERN MUP_Bool MUP_API MUP_send_cmd_data(
                                MUP_Kernel kernel,
                                MUP_Cookie cmd_cookie,
                                const MUP_Data * cmd,
                                MUP_ExecutionFlags flags,
                                const char * out_format);

/** Send a completion wanted message.
 * @param kernel kernel handle
 * @param prefix prefix of name to be completed
 * @return true if message was sent successfully */
MUP_EXTERN MUP_Bool MUP_API MUP_send_completion_wanted(
                                MUP_Kernel kernel,
                                const MUP_Char * prefix);

/** Send debug command.
 * @param kernel kernel handle
 * @param cmd the debugger command
 * @param fname file name
 * @param fpos line number in file
 * @param expr expression (as MuPAD language statement)
 * @return true if message was sent successfully */
MUP_EXTERN MUP_Bool MUP_API MUP_send_dbg_cmd(
                                MUP_Kernel kernel,
                                MUP_Byte cmd,
                                const MUP_Char * fname,
                                MUP_Int fpos,
                                const MUP_Char * expr);

/** Send a response for a MuPAD \c system() function call.
 * @param kernel kernel handle
 * @param result result code of the system call
 * @return true if message was sent successfully */
MUP_EXTERN MUP_Bool MUP_API MUP_send_system_result(
                                MUP_Kernel kernel,
                                MUP_Int result);

/** Send a response for a MuPAD \c input() function call.
 * @param kernel kernel handle
 * @param canceled true if the input was canceled by the user.
 * @param input the input text entered by the user.
 * @return true if message was sent successfully */
MUP_EXTERN MUP_Bool MUP_API MUP_send_input(
                                MUP_Kernel kernel,
                                MUP_Bool canceled,
                                const MUP_Char * input);

/** Send a response for a MuPAD \c textinput() function call.
 * @param kernel kernel handle
 * @param canceled true if the input was canceled by the user.
 * @param input the input text entered by the user.
 * @return true if message was sent successfully */
MUP_EXTERN MUP_Bool MUP_API MUP_send_textinput(
                                MUP_Kernel kernel,
                                MUP_Bool canceled, 
                                const MUP_Char * input);

/** Send a response for a MuPAD \c clientCall() function call.
 * @param kernel kernel handle
 * @param error true if an error occured when executing the callback function
 * @param result result data of the callback function (format must be
 *          #MUP_fn_mcode_tree always)
 * @return true if message was sent successfully */
MUP_EXTERN MUP_Bool MUP_API MUP_send_call_result(
                                MUP_Kernel kernel,
                                MUP_Bool error,
                                const MUP_Data * result);

/** Send a new textwidth.
 * @param kernel kernel handle
 * @param width new value of \c TEXTWIDTH
 * @return true if message was sent successfully */
MUP_EXTERN MUP_Bool MUP_API MUP_send_textwidth(
                                MUP_Kernel kernel,
                                MUP_Int width);

/** Send a new pretty-print value.
 * @param kernel kernel handle
 * @param pretty new value of \c PRETTYPRINT
 * @return true if message was sent successfully */
MUP_EXTERN MUP_Bool MUP_API MUP_send_pretty_print(
                                MUP_Kernel kernel,
                                MUP_Bool pretty);

/** Send a format wanted for an output type.
 * @param kernel kernel handle
 * @param output_type output type (one of the \c MUP_ot_... constants)
 * @param format output format (one of the \c MUP_fn_... constants)
 * @return true if message was sent successfully */
MUP_EXTERN MUP_Bool MUP_API MUP_send_format(
                                MUP_Kernel kernel,
                                const char * output_type, 
                                const char * format);

/** Send a status request.
 * @param kernel kernel handle
 * @return true if message was sent successfully */
MUP_EXTERN MUP_Bool MUP_API MUP_send_status_wanted(
                                MUP_Kernel kernel);

/** Send a stop signal.
 * @param kernel kernel handle
 * @return true if message was sent successfully */
MUP_EXTERN MUP_Bool MUP_API MUP_send_stop(
                                MUP_Kernel kernel);

/** Send a quit message.
 * @param kernel kernel handle
 * @return true if message was sent successfully */
MUP_EXTERN MUP_Bool MUP_API MUP_send_quit(
                                MUP_Kernel kernel);

/* @} */

/* ----------------------------------------------------------------------- */
/** @defgroup set_mssg_hdl Setting Message Handlers

Attach callback functions for kernel messages.

This functions attach callback functions to a kernels handle. The
callbacks will be called for messages received from the kernel
associated with the handle.

One needs to define callbacks only for messages one is interested in.
One may remove a callback by setting it to \c NULL.

In order to feed user data through to the callbacks one may 
associate the data with the kernel handle via MUP_set_client_data().
The callback may then get access to this data via the kernel handle
when called.

The arguments of the callbacks are described
<A HREF="group__mssg__hdl.html">&gt;&gt;here</A>.

<B><A HREF="prot.html#kernel">&gt;&gt; Description of the existing kernel 
messages.</A></B>
*/

/* @{ */

/** Set init wanted message handler. */
MUP_EXTERN void MUP_API MUP_set_on_init_wanted_handler(
                                MUP_Kernel kernel,
                                MUP_on_init_wanted_handler handler);

/** Set kernel ready message handler. */
MUP_EXTERN void MUP_API MUP_set_on_kernel_ready_handler(
                                MUP_Kernel kernel,
                                MUP_on_kernel_ready_handler handler);

/** Set output message handler for text output. */
MUP_EXTERN void MUP_API MUP_set_on_output_handler(
                                MUP_Kernel kernel,
                                MUP_on_output_handler handler);

/** Set output message handler for binary output. */
MUP_EXTERN void MUP_API MUP_set_on_output_data_handler(
                                MUP_Kernel kernel,
                                MUP_on_output_data_handler handler);

/** Set finished message handler. */
MUP_EXTERN void MUP_API MUP_set_on_finished_handler(
                                MUP_Kernel kernel,
                                MUP_on_finished_handler handler);

/** Set complete command message handler. */
MUP_EXTERN void MUP_API MUP_set_on_complete_cmd_handler(
                                MUP_Kernel kernel,
                                MUP_on_complete_cmd_handler handler);

/** Set input wanted message handler. */
MUP_EXTERN void MUP_API MUP_set_on_input_wanted_handler(
                                MUP_Kernel kernel,
                                MUP_on_input_wanted_handler handler);

/** Set callback message handler. */
MUP_EXTERN void MUP_API MUP_set_on_callback_handler(
                                MUP_Kernel kernel,
                                MUP_on_callback_handler handler);

/** Set help message handler. */
MUP_EXTERN void MUP_API MUP_set_on_help_handler(
                                MUP_Kernel kernel,
                                MUP_on_help_handler handler);

/** Set system message handler. */
MUP_EXTERN void MUP_API MUP_set_on_system_handler(
                                MUP_Kernel kernel,
                                MUP_on_system_handler handler);

/** Set frame changed message handler. */
MUP_EXTERN void MUP_API MUP_set_on_frame_changed_handler(
                                MUP_Kernel kernel,
                                MUP_on_frame_changed_handler handler);

/** Set textwidth changed message handler. */
MUP_EXTERN void MUP_API MUP_set_on_textwidth_changed_handler(
                                MUP_Kernel kernel,
                                MUP_on_textwidth_changed_handler handler);

/** Set pretty-print changed message handler. */
MUP_EXTERN void MUP_API MUP_set_on_pretty_print_changed_handler(
                                MUP_Kernel kernel,
                                MUP_on_pretty_print_changed_handler handler);

/** Set format changed message handler. */
MUP_EXTERN void MUP_API MUP_set_on_format_changed_handler(
                                MUP_Kernel kernel,
                                MUP_on_format_changed_handler handler);

/** Set select format message handler. */
MUP_EXTERN void MUP_API MUP_set_on_select_format_handler(
                                MUP_Kernel kernel,
                                MUP_on_select_format_handler handler);

/** Set status message handler. */
MUP_EXTERN void MUP_API MUP_set_on_status_handler(
                                MUP_Kernel kernel,
                                MUP_on_status_handler handler);

/** Set input completion message handler. */
MUP_EXTERN void MUP_API MUP_set_on_input_completion_handler(
                                MUP_Kernel kernel,
                                MUP_on_input_completion_handler handler);

/** Set plot result file message handler. */
MUP_EXTERN void MUP_API MUP_set_on_plot_result_file_handler(
                                MUP_Kernel kernel,
                                MUP_on_plot_result_file_handler handler);

/** Set debug command wanted message handler. */
MUP_EXTERN void MUP_API MUP_set_on_dbg_cmd_wanted_handler(
                                MUP_Kernel kernel,
                                MUP_on_dbg_cmd_wanted_handler handler);

/** Set debug info message handler. */
MUP_EXTERN void MUP_API MUP_set_on_dbg_info_handler(
                                MUP_Kernel kernel,
                                MUP_on_dbg_info_handler handler);

/** Set debugger display list message handler. */
MUP_EXTERN void MUP_API MUP_set_on_display_list_handler(
                                MUP_Kernel kernel,
                                MUP_on_display_list_handler handler);

/** Set debugger breakpoint list message handler. */
MUP_EXTERN void MUP_API MUP_set_on_breakpoint_list_handler(
                                MUP_Kernel kernel,
                                MUP_on_breakpoint_list_handler handler);

/** Set debugger call stack message handler. */
MUP_EXTERN void MUP_API MUP_set_on_call_stack_handler(
                                MUP_Kernel kernel,
                                MUP_on_call_stack_handler handler);

/** Set reset message handler. */
MUP_EXTERN void MUP_API MUP_set_on_reset_handler(
                                MUP_Kernel kernel,
                                MUP_on_reset_handler handler);

/** Set quit message handler. */
MUP_EXTERN void MUP_API MUP_set_on_quit_handler(
                                MUP_Kernel kernel,
                                MUP_on_quit_handler handler);

/* @} */

#ifdef __cplusplus
}
#endif

#endif /* !MUPKERNEL_H_INCLUDED */
