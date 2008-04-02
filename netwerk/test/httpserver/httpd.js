/* -*- Mode: Java; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et: */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is the MozJSHTTP server.
 *
 * The Initial Developer of the Original Code is
 * Mozilla Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Darin Fisher (v1, netwerk/test/TestServ.js)
 *   Christian Biesinger (v2, netwerk/test/unit/head_http_server.js)
 *   Jeff Walden <jwalden+code@mit.edu> (v3, netwerk/test/httpserver/httpd.js)
 *   Robert Sayre <sayrer@gmail.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

/*
 * An implementation of an HTTP server both as a loadable script and as an XPCOM
 * component.  See the accompanying README file for user documentation on
 * MozJSHTTP.
 */

const Cc = Components.classes;
const Ci = Components.interfaces;
const Cr = Components.results;
const Cu = Components.utils;
const CC = Components.Constructor;

/** True if debugging output is enabled, false otherwise. */
var DEBUG = false; // non-const *only* so tweakable in server tests

var gGlobalObject = this;

/**
 * Asserts that the given condition holds.  If it doesn't, the given message is
 * dumped, a stack trace is printed, and an exception is thrown to attempt to
 * stop execution (which unfortunately must rely upon the exception not being
 * accidentally swallowed by the code that uses it).
 */
function NS_ASSERT(cond, msg)
{
  if (DEBUG && !cond)
  {
    dumpn("###!!!");
    dumpn("###!!! ASSERTION" + (msg ? ": " + msg : "!"));
    dumpn("###!!! Stack follows:");

    var stack = new Error().stack.split(/\n/);
    dumpn(stack.map(function(val) { return "###!!!   " + val; }).join("\n"));
    
    throw Cr.NS_ERROR_ABORT;
  }
}

/** Constructs an HTTP error object. */
function HttpError(code, description)
{
  this.code = code;
  this.description = description;
}
HttpError.prototype =
{
  toString: function()
  {
    return this.code + " " + this.description;
  }
};

/**
 * Errors thrown to trigger specific HTTP server responses.
 */
const HTTP_400 = new HttpError(400, "Bad Request");
const HTTP_401 = new HttpError(401, "Unauthorized");
const HTTP_402 = new HttpError(402, "Payment Required");
const HTTP_403 = new HttpError(403, "Forbidden");
const HTTP_404 = new HttpError(404, "Not Found");
const HTTP_405 = new HttpError(405, "Method Not Allowed");
const HTTP_406 = new HttpError(406, "Not Acceptable");
const HTTP_407 = new HttpError(407, "Proxy Authentication Required");
const HTTP_408 = new HttpError(408, "Request Timeout");
const HTTP_409 = new HttpError(409, "Conflict");
const HTTP_410 = new HttpError(410, "Gone");
const HTTP_411 = new HttpError(411, "Length Required");
const HTTP_412 = new HttpError(412, "Precondition Failed");
const HTTP_413 = new HttpError(413, "Request Entity Too Large");
const HTTP_414 = new HttpError(414, "Request-URI Too Long");
const HTTP_415 = new HttpError(415, "Unsupported Media Type");
const HTTP_416 = new HttpError(416, "Requested Range Not Satisfiable");
const HTTP_417 = new HttpError(417, "Expectation Failed");

const HTTP_500 = new HttpError(500, "Internal Server Error");
const HTTP_501 = new HttpError(501, "Not Implemented");
const HTTP_502 = new HttpError(502, "Bad Gateway");
const HTTP_503 = new HttpError(503, "Service Unavailable");
const HTTP_504 = new HttpError(504, "Gateway Timeout");
const HTTP_505 = new HttpError(505, "HTTP Version Not Supported");

/** Creates a hash with fields corresponding to the values in arr. */
function array2obj(arr)
{
  var obj = {};
  for (var i = 0; i < arr.length; i++)
    obj[arr[i]] = arr[i];
  return obj;
}

/** Returns an array of the integers x through y, inclusive. */
function range(x, y)
{
  var arr = [];
  for (var i = x; i <= y; i++)
    arr.push(i);
  return arr;
}

/** An object (hash) whose fields are the numbers of all HTTP error codes. */
const HTTP_ERROR_CODES = array2obj(range(400, 417).concat(range(500, 505)));


/**
 * The character used to distinguish hidden files from non-hidden files, a la
 * the leading dot in Apache.  Since that mechanism also hides files from
 * easy display in LXR, ls output, etc. however, we choose instead to use a
 * suffix character.  If a requested file ends with it, we append another
 * when getting the file on the server.  If it doesn't, we just look up that
 * file.  Therefore, any file whose name ends with exactly one of the character
 * is "hidden" and available for use by the server.
 */
const HIDDEN_CHAR = "^";

/**
 * The file name suffix indicating the file containing overridden headers for
 * a requested file.
 */
const HEADERS_SUFFIX = HIDDEN_CHAR + "headers" + HIDDEN_CHAR;

/** Type used to denote SJS scripts for CGI-like functionality. */
const SJS_TYPE = "sjs";


/** dump(str) with a trailing "\n" -- only outputs if DEBUG */
function dumpn(str)
{
  if (DEBUG)
    dump(str + "\n");
}

/** Dumps the current JS stack if DEBUG. */
function dumpStack()
{
  // peel off the frames for dumpStack() and Error()
  var stack = new Error().stack.split(/\n/).slice(2);
  stack.forEach(dumpn);
}


/** The XPCOM thread manager. */
var gThreadManager = null;


/**
 * JavaScript constructors for commonly-used classes; precreating these is a
 * speedup over doing the same from base principles.  See the docs at
 * http://developer.mozilla.org/en/docs/Components.Constructor for details.
 */
const ServerSocket = CC("@mozilla.org/network/server-socket;1",
                        "nsIServerSocket",
                        "init");
const BinaryInputStream = CC("@mozilla.org/binaryinputstream;1",
                             "nsIBinaryInputStream",
                             "setInputStream");
const ScriptableInputStream = CC("@mozilla.org/scriptableinputstream;1",
                                 "nsIScriptableInputStream",
                                 "init");
const Pipe = CC("@mozilla.org/pipe;1",
                "nsIPipe",
                "init");
const FileInputStream = CC("@mozilla.org/network/file-input-stream;1",
                           "nsIFileInputStream",
                           "init");
const StreamCopier = CC("@mozilla.org/network/async-stream-copier;1",
                        "nsIAsyncStreamCopier",
                        "init");
const Pump = CC("@mozilla.org/network/input-stream-pump;1",
                "nsIInputStreamPump",
                "init");
const ConverterInputStream = CC("@mozilla.org/intl/converter-input-stream;1",
                                "nsIConverterInputStream",
                                "init");
const WritablePropertyBag = CC("@mozilla.org/hash-property-bag;1",
                               "nsIWritablePropertyBag2");
const SupportsString = CC("@mozilla.org/supports-string;1",
                          "nsISupportsString");


/**
 * Returns the RFC 822/1123 representation of a date.
 *
 * @param date : Number
 *   the date, in milliseconds from midnight (00:00:00), January 1, 1970 GMT
 * @returns string
 *   the representation of the given date
 */
function toDateString(date)
{
  //
  // rfc1123-date = wkday "," SP date1 SP time SP "GMT"
  // date1        = 2DIGIT SP month SP 4DIGIT
  //                ; day month year (e.g., 02 Jun 1982)
  // time         = 2DIGIT ":" 2DIGIT ":" 2DIGIT
  //                ; 00:00:00 - 23:59:59
  // wkday        = "Mon" | "Tue" | "Wed"
  //              | "Thu" | "Fri" | "Sat" | "Sun"
  // month        = "Jan" | "Feb" | "Mar" | "Apr"
  //              | "May" | "Jun" | "Jul" | "Aug"
  //              | "Sep" | "Oct" | "Nov" | "Dec"
  //

  const wkdayStrings = ["Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"];
  const monthStrings = ["Jan", "Feb", "Mar", "Apr", "May", "Jun",
                        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"];

  /**
   * Processes a date and returns the encoded UTC time as a string according to
   * the format specified in RFC 2616.
   *
   * @param date : Date
   *   the date to process
   * @returns string
   *   a string of the form "HH:MM:SS", ranging from "00:00:00" to "23:59:59"
   */
  function toTime(date)
  {
    var hrs = date.getUTCHours();
    var rv  = (hrs < 10) ? "0" + hrs : hrs;
    
    var mins = date.getUTCMinutes();
    rv += ":";
    rv += (mins < 10) ? "0" + mins : mins;

    var secs = date.getUTCSeconds();
    rv += ":";
    rv += (secs < 10) ? "0" + secs : secs;

    return rv;
  }

  /**
   * Processes a date and returns the encoded UTC date as a string according to
   * the date1 format specified in RFC 2616.
   *
   * @param date : Date
   *   the date to process
   * @returns string
   *   a string of the form "HH:MM:SS", ranging from "00:00:00" to "23:59:59"
   */
  function toDate1(date)
  {
    var day = date.getUTCDate();
    var month = date.getUTCMonth();
    var year = date.getUTCFullYear();

    var rv = (day < 10) ? "0" + day : day;
    rv += " " + monthStrings[month];
    rv += " " + year;

    return rv;
  }

  date = new Date(date);

  const fmtString = "%wkday%, %date1% %time% GMT";
  var rv = fmtString.replace("%wkday%", wkdayStrings[date.getUTCDay()]);
  rv = rv.replace("%time%", toTime(date));
  return rv.replace("%date1%", toDate1(date));
}

/**
 * Prints out a human-readable representation of the object o and its fields,
 * omitting those whose names begin with "_" if showMembers != true (to ignore
 * "private" properties exposed via getters/setters).
 */
function printObj(o, showMembers)
{
  var s = "******************************\n";
  s +=    "o = {\n";
  for (var i in o)
  {
    if (typeof(i) != "string" ||
        (showMembers || (i.length > 0 && i[0] != "_")))
      s+= "      " + i + ": " + o[i] + ",\n";
  }
  s +=    "    };\n";
  s +=    "******************************";
  dumpn(s);
}

/**
 * Instantiates a new HTTP server.
 */
function nsHttpServer()
{
  if (!gThreadManager)
    gThreadManager = Cc["@mozilla.org/thread-manager;1"].getService();

  /** The port on which this server listens. */
  this._port = undefined;

  /** The socket associated with this. */
  this._socket = null;

  /** The handler used to process requests to this server. */
  this._handler = new ServerHandler(this);

  /**
   * Indicates when the server is to be shut down at the end of the request.
   */
  this._doQuit = false;

  /**
   * True if the socket in this is closed (and closure notifications have been
   * sent and processed if the socket was ever opened), false otherwise.
   */
  this._socketClosed = true;
}
nsHttpServer.prototype =
{
  // NSISERVERSOCKETLISTENER

  /**
   * Processes an incoming request coming in on the given socket and contained
   * in the given transport.
   *
   * @param socket : nsIServerSocket
   *   the socket through which the request was served
   * @param trans : nsISocketTransport
   *   the transport for the request/response
   * @see nsIServerSocketListener.onSocketAccepted
   */
  onSocketAccepted: function(socket, trans)
  {
    dumpn("*** onSocketAccepted(socket=" + socket + ", trans=" + trans + ") " +
          "on thread " + gThreadManager.currentThread +
          " (main is " + gThreadManager.mainThread + ")");

    dumpn(">>> new connection on " + trans.host + ":" + trans.port);

    const SEGMENT_SIZE = 8192;
    const SEGMENT_COUNT = 1024;
    var input = trans.openInputStream(0, SEGMENT_SIZE, SEGMENT_COUNT)
                     .QueryInterface(Ci.nsIAsyncInputStream);
    var output = trans.openOutputStream(Ci.nsITransport.OPEN_BLOCKING, 0, 0);

    var conn = new Connection(input, output, this, socket.port);
    var reader = new RequestReader(conn);

    // XXX add request timeout functionality here!

    // Note: must use main thread here, or we might get a GC that will cause
    //       threadsafety assertions.  We really need to fix XPConnect so that
    //       you can actually do things in multi-threaded JS.  :-(
    input.asyncWait(reader, 0, 0, gThreadManager.mainThread);
  },

  /**
   * Called when the socket associated with this is closed.
   *
   * @param socket : nsIServerSocket
   *   the socket being closed
   * @param status : nsresult
   *   the reason the socket stopped listening (NS_BINDING_ABORTED if the server
   *   was stopped using nsIHttpServer.stop)
   * @see nsIServerSocketListener.onStopListening
   */
  onStopListening: function(socket, status)
  {
    dumpn(">>> shutting down server");
    this._socketClosed = true;
  },

  // NSIHTTPSERVER

  //
  // see nsIHttpServer.start
  //
  start: function(port)
  {
    if (this._socket)
      throw Cr.NS_ERROR_ALREADY_INITIALIZED;

    this._port = port;
    this._doQuit = this._socketClosed = false;

    var socket = new ServerSocket(this._port,
                                  true, // loopback only
                                  -1);  // default number of pending connections

    dumpn(">>> listening on port " + socket.port);
    socket.asyncListen(this);
    this._socket = socket;
  },

  //
  // see nsIHttpServer.stop
  //
  stop: function()
  {
    if (!this._socket)
      return;

    dumpn(">>> stopping listening on port " + this._socket.port);
    this._socket.close();
    this._socket = null;
    this._doQuit = false;

    // spin an event loop and wait for the socket-close notification
    var thr = gThreadManager.currentThread;
    while (!this._socketClosed || this._handler.hasPendingRequests())
      thr.processNextEvent(true);
  },

  //
  // see nsIHttpServer.registerFile
  //
  registerFile: function(path, file)
  {
    if (file && (!file.exists() || file.isDirectory()))
      throw Cr.NS_ERROR_INVALID_ARG;

    this._handler.registerFile(path, file);
  },

  //
  // see nsIHttpServer.registerDirectory
  //
  registerDirectory: function(path, directory)
  {
    // XXX true path validation!
    if (path.charAt(0) != "/" ||
        path.charAt(path.length - 1) != "/" ||
        (directory &&
         (!directory.exists() || !directory.isDirectory())))
      throw Cr.NS_ERROR_INVALID_ARG;

    // XXX determine behavior of non-existent /foo/bar when a /foo/bar/ mapping
    //     exists!

    this._handler.registerDirectory(path, directory);
  },

  //
  // see nsIHttpServer.registerPathHandler
  //
  registerPathHandler: function(path, handler)
  {
    this._handler.registerPathHandler(path, handler);
  },

  //
  // see nsIHttpServer.registerErrorHandler
  //
  registerErrorHandler: function(code, handler)
  {
    this._handler.registerErrorHandler(code, handler);
  },

  //
  // see nsIHttpServer.setIndexHandler
  //
  setIndexHandler: function(handler)
  {
    this._handler.setIndexHandler(handler);
  },

  //
  // see nsIHttpServer.registerContentType
  //
  registerContentType: function(ext, type)
  {
    this._handler.registerContentType(ext, type);
  },

  // NSISUPPORTS

  //
  // see nsISupports.QueryInterface
  //
  QueryInterface: function(iid)
  {
    if (iid.equals(Ci.nsIHttpServer) ||
        iid.equals(Ci.nsIServerSocketListener) ||
        iid.equals(Ci.nsISupports))
      return this;

    throw Cr.NS_ERROR_NO_INTERFACE;
  },


  // NON-XPCOM PUBLIC API

  /**
   * Returns true iff this server is not running (and is not in the process of
   * serving any requests still to be processed when the server was last
   * stopped after being run).
   */
  isStopped: function()
  {
    return this._socketClosed && !this._handler.hasPendingRequests();
  },

  
  // PRIVATE IMPLEMENTATION

  /**
   * Closes the passed-in connection.
   *
   * @param connection : Connection
   *   the connection to close
   */
  _endConnection: function(connection)
  {
    //
    // Order is important below: we must decrement handler._pendingRequests
    // BEFORE calling this.stop(), if needed, in connection.destroy().
    // this.stop() returns only when the server socket's closed AND all pending
    // requests are complete, which clearly isn't (and never will be) the case
    // if it were the other way around.
    //

    connection.close();

    NS_ASSERT(this == connection.server);

    this._handler._pendingRequests--;

    connection.destroy();
  },

  /**
   * Requests that the server be shut down when possible.
   */
  _requestQuit: function()
  {
    dumpn(">>> requesting a quit");
    dumpStack();
    this._doQuit = true;
  }

};


/**
 * Represents a connection to the server (and possibly in the future the thread
 * on which the connection is processed).
 *
 * @param input : nsIInputStream
 *   stream from which incoming data on the connection is read
 * @param output : nsIOutputStream
 *   stream to write data out the connection
 * @param server : nsHttpServer
 *   the server handling the connection
 * @param port : int
 *   the port on which the server is running
 */
function Connection(input, output, server, port)
{
  /** Stream of incoming data. */
  this.input = input;

  /** Stream for outgoing data. */
  this.output = output;

  /** The server associated with this request. */
  this.server = server;

  /** The port on which the server is running. */
  this.port = port;
  
  /** State variables for debugging. */
  this._closed = this._processed = false;
}
Connection.prototype =
{
  /** Closes this connection's input/output streams. */
  close: function()
  {
    this.input.close();
    this.output.close();
    this._closed = true;
  },

  /**
   * Initiates processing of this connection, using the data in the given
   * request.
   *
   * @param request : Request
   *   the request which should be processed
   */
  process: function(request)
  {
    NS_ASSERT(!this._closed && !this._processed);

    this._processed = true;

    this.server._handler.handleResponse(this, request);
  },

  /**
   * Initiates processing of this connection, generating a response with the
   * given HTTP error code.
   *
   * @param code : uint
   *   an HTTP code, so in the range [0, 1000)
   * @param metadata : Request
   *   incomplete data about the incoming request (since there were errors
   *   during its processing
   */
  processError: function(code, metadata)
  {
    NS_ASSERT(!this._closed && !this._processed);

    this._processed = true;

    this.server._handler.handleError(code, this, metadata);
  },

  /** Ends this connection, destroying the resources it uses. */
  end: function()
  {
    this.server._endConnection(this);
  },

  /** Destroys resources used by this. */
  destroy: function()
  {
    if (!this._closed)
      this.close();

    // If an error triggered a server shutdown, act on it now
    var server = this.server;
    if (server._doQuit)
      server.stop();
  }
};



/** Returns an array of count bytes from the given input stream. */
function readBytes(inputStream, count)
{
  return new BinaryInputStream(inputStream).readByteArray(count);
}



/** Request reader processing states; see RequestReader for details. */
const READER_INITIAL    = 0;
const READER_IN_HEADERS = 1;
const READER_IN_BODY    = 2;


/**
 * Reads incoming request data asynchronously, does any necessary preprocessing,
 * and forwards it to the request handler.  Processing occurs in three states:
 *
 *   READER_INITIAL             Haven't read the entire request line yet
 *   READER_IN_HEADERS          Reading headers in the request
 *   READER_IN_BODY             Finished reading all request headers (when body
 *                              support's added, will be reading the body)
 *
 * During the first two stages, initial metadata about the request is gathered
 * into a Request object.  Once the status line and headers have been processed,
 * we create a Response and hand it off to the ServerHandler to be given to the
 * appropriate request handler.
 *
 * XXX we should set up a stream to provide lazy access to the request body
 *
 * @param connection : Connection
 *   the connection for the request being read
 */
function RequestReader(connection)
{
  /** Connection metadata for this request. */
  this._connection = connection;

  /**
   * A container providing line-by-line access to the raw bytes that make up the
   * data which has been read from the connection but has not yet been acted
   * upon (by passing it to the request handler or by extracting request
   * metadata from it).
   */
  this._data = new LineData();

  /** The current state of parsing the incoming request. */
  this._state = READER_INITIAL;

  /** Metadata constructed from the incoming request for the request handler. */
  this._metadata = new Request(connection.port);

  /**
   * Used to preserve state if we run out of line data midway through a
   * multi-line header.  _lastHeaderName stores the name of the header, while
   * _lastHeaderValue stores the value we've seen so far for the header.
   *
   * These fields are always either both undefined or both strings.
   */
  this._lastHeaderName = this._lastHeaderValue = undefined;
}
RequestReader.prototype =
{
  // NSIINPUTSTREAMCALLBACK

  /**
   * Called when more data from the incoming request is available.  This method
   * then reads the available data from input and deals with that data as
   * necessary, depending upon the syntax of already-downloaded data.
   *
   * @param input : nsIAsyncInputStream
   *   the stream of incoming data from the connection
   */
  onInputStreamReady: function(input)
  {
    dumpn("*** onInputStreamReady(input=" + input + ") on thread " +
          gThreadManager.currentThread + " (main is " +
          gThreadManager.mainThread + ")");
    dumpn("*** this._state == " + this._state);

    var count = input.available();

    // Handle cases where we get more data after a request error has been
    // discovered but *before* we can close the connection.
    if (!this._data)
      return;

    var moreAvailable = false;

    switch (this._state)
    {
      case READER_INITIAL:
        moreAvailable = this._processRequestLine(input, count);
        break;

      case READER_IN_HEADERS:
        moreAvailable = this._processHeaders(input, count);
        break;

      case READER_IN_BODY:
        // XXX handle the request body!  until then, just stop reading
        break;

      default:
        NS_ASSERT(false);
    }

    if (moreAvailable)
      input.asyncWait(this, 0, 0, gThreadManager.currentThread);
  },

  //
  // see nsISupports.QueryInterface
  //
  QueryInterface: function(aIID)
  {
    if (aIID.equals(Ci.nsIInputStreamCallback) ||
        aIID.equals(Ci.nsISupports))
      return this;

    throw Cr.NS_ERROR_NO_INTERFACE;
  },


  // PRIVATE API

  /**
   * Reads count bytes from input and processes unprocessed, downloaded data as
   * a request line.
   *
   * @param input : nsIInputStream
   *   stream from which count bytes of data must be read
   * @param count : PRUint32
   *   the number of bytes of data which must be read from input
   * @returns boolean
   *   true if more data must be read from the request, false otherwise
   */
  _processRequestLine: function(input, count)
  {
    NS_ASSERT(this._state == READER_INITIAL);

    var data = this._data;
    data.appendBytes(readBytes(input, count));


    // servers SHOULD ignore any empty line(s) received where a Request-Line
    // is expected (section 4.1)
    var line = {};
    var readSuccess;
    while ((readSuccess = data.readLine(line)) && line.value == "")
      dumpn("*** ignoring beginning blank line...");

    // if we don't have a full line, wait until we do
    if (!readSuccess)
      return true;

    // we have the first non-blank line
    try
    {
      this._parseRequestLine(line.value);

      // do we have more header data to read?
      if (!this._parseHeaders())
        return true;

      // headers complete, do a data check and then forward to the handler
      this._validateRequest();
      return this._handleResponse();
    }
    catch (e)
    {
      this._handleError(e);
      return false;
    }
  },

  /**
   * Reads data from input and processes it, assuming it is either at the
   * beginning or in the middle of processing request headers.
   *
   * @param input : nsIInputStream
   *   stream from which count bytes of data must be read
   * @param count : PRUint32
   *   the number of bytes of data which must be read from input
   * @returns boolean
   *   true if more data must be read from the request, false otherwise
   */
  _processHeaders: function(input, count)
  {
    NS_ASSERT(this._state == READER_IN_HEADERS);

    // XXX things to fix here:
    //
    // - need to support RFC 2047-encoded non-US-ASCII characters
    // - really support absolute URLs (requires telling the server all its
    //   hostnames, beyond just localhost:port or 127.0.0.1:port)

    this._data.appendBytes(readBytes(input, count));

    try
    {
      // do we have all the headers?
      if (!this._parseHeaders())
        return true;

      // we have all the headers, continue with the body
      this._validateRequest();
      return this._handleResponse();
    }
    catch (e)
    {
      this._handleError(e);
      return false;
    }
  },

  /**
   * Does various post-header checks on the data in this request.
   *
   * @throws : HttpError
   *   if the request was malformed in some way
   */
  _validateRequest: function()
  {
    NS_ASSERT(this._state == READER_IN_BODY);

    dumpn("*** _validateRequest");

    var metadata = this._metadata;
    var headers = metadata._headers;

    var isHttp11 = metadata._httpVersion.equals(nsHttpVersion.HTTP_1_1);

    // 19.6.1.1 -- servers MUST report 400 to HTTP/1.1 requests w/o Host header
    if (isHttp11 && !headers.hasHeader("Host"))
      throw HTTP_400;
  },

  /**
   * Handles responses in case of error, either in the server or in the request.
   *
   * @param e
   *   the specific error encountered, which is an HttpError in the case where
   *   the request is in some way invalid or cannot be fulfilled; if this isn't
   *   an HttpError we're going to be paranoid and shut down, because that
   *   shouldn't happen, ever
   */
  _handleError: function(e)
  {
    var server = this._connection.server;
    if (e instanceof HttpError)
    {
      var code = e.code;
    }
    else
    {
      // no idea what happened -- be paranoid and shut down
      code = 500;
      server._requestQuit();
    }

    // make attempted reuse of data an error
    this._data = null;

    this._connection.processError(code, this._metadata);
  },

  /**
   * Now that we've read the request line and headers, we can actually hand off
   * the request to be handled.
   *
   * This method is called once per request, after the request line and all
   * headers have been received.
   *
   * @returns boolean
   *   true if more data must be read, false otherwise
   */
  _handleResponse: function()
  {
    NS_ASSERT(this._state == READER_IN_BODY);

    // XXX set up a stream for data in the request body here

    // We don't need the line-based data any more, so make attempted reuse an
    // error.
    this._data = null;

    this._connection.process(this._metadata);

    return false;
  },


  // PARSING

  /**
   * Parses the request line for the HTTP request associated with this.
   *
   * @param line : string
   *   the request line
   */
  _parseRequestLine: function(line)
  {
    NS_ASSERT(this._state == READER_INITIAL);

    dumpn("*** _parseRequestLine('" + line + "')");

    var metadata = this._metadata;

    // clients and servers SHOULD accept any amount of SP or HT characters
    // between fields, even though only a single SP is required (section 19.3)
    var request = line.split(/[ \t]+/);
    if (!request || request.length != 3)
      throw HTTP_400;

    metadata._method = request[0];

    // get the HTTP version
    var ver = request[2];
    var match = ver.match(/^HTTP\/(\d+\.\d+)$/);
    if (!match)
      throw HTTP_400;

    // determine HTTP version
    try
    {
      metadata._httpVersion = new nsHttpVersion(match[1]);
      if (!metadata._httpVersion.equals(nsHttpVersion.HTTP_1_0) &&
          !metadata._httpVersion.equals(nsHttpVersion.HTTP_1_1))
        throw "unsupported HTTP version";
    }
    catch (e)
    {
      // we support HTTP/1.0 and HTTP/1.1 only
      throw HTTP_501;
    }


    var fullPath = request[1];

    if (fullPath.charAt(0) != "/")
    {
      // XXX we don't really support absolute URIs yet -- a MUST for HTTP/1.1;
      //     for now just get the path and use that, ignoring hostport
      try
      {
        var uri = Cc["@mozilla.org/network/io-service;1"]
                    .getService(Ci.nsIIOService)
                    .newURI(fullPath, null, null);
        fullPath = uri.path;
      }
      catch (e) { /* invalid URI */ }
      if (fullPath.charAt(0) != "/")
      {
        this.errorCode = 400;
        return;
      }
    }

    var splitter = fullPath.indexOf("?");
    if (splitter < 0)
    {
      // _queryString already set in ctor
      metadata._path = fullPath;
    }
    else
    {
      metadata._path = fullPath.substring(0, splitter);
      metadata._queryString = fullPath.substring(splitter + 1);
    }

    // our work here is finished
    this._state = READER_IN_HEADERS;
  },

  /**
   * Parses all available HTTP headers in this until the header-ending CRLFCRLF,
   * adding them to the store of headers in the request.
   *
   * @throws
   *   HTTP_400 if the headers are malformed
   * @returns boolean
   *   true if all headers have now been processed, false otherwise
   */
  _parseHeaders: function()
  {
    NS_ASSERT(this._state == READER_IN_HEADERS);

    dumpn("*** _parseHeaders");

    var data = this._data;

    var headers = this._metadata._headers;
    var lastName = this._lastHeaderName;
    var lastVal = this._lastHeaderValue;

    var line = {};
    while (true)
    {
      NS_ASSERT(!((lastVal === undefined) ^ (lastName === undefined)),
                lastName === undefined ?
                  "lastVal without lastName?  lastVal: '" + lastVal + "'" :
                  "lastName without lastVal?  lastName: '" + lastName + "'");

      if (!data.readLine(line))
      {
        // save any data we have from the header we might still be processing
        this._lastHeaderName = lastName;
        this._lastHeaderValue = lastVal;
        return false;
      }

      var lineText = line.value;
      var firstChar = lineText.charAt(0);

      // blank line means end of headers
      if (lineText == "")
      {
        // we're finished with the previous header
        if (lastName)
        {
          try
          {
            headers.setHeader(lastName, lastVal, true);
          }
          catch (e)
          {
            dumpn("*** e == " + e);
            throw HTTP_400;
          }
        }
        else
        {
          // no headers in request -- valid for HTTP/1.0 requests
        }

        // either way, we're done processing headers
        this._state = READER_IN_BODY;
        return true;
      }
      else if (firstChar == " " || firstChar == "\t")
      {
        // multi-line header if we've already seen a header line
        if (!lastName)
        {
          // we don't have a header to continue!
          throw HTTP_400;
        }

        // append this line's text to the value; starts with SP/HT, so no need
        // for separating whitespace
        lastVal += lineText;
      }
      else
      {
        // we have a new header, so set the old one (if one existed)
        if (lastName)
        {
          try
          {
            headers.setHeader(lastName, lastVal, true);
          }
          catch (e)
          {
            dumpn("*** e == " + e);
            throw HTTP_400;
          }
        }

        var colon = lineText.indexOf(":"); // first colon must be splitter
        if (colon < 1)
        {
          // no colon or missing header field-name
          throw HTTP_400;
        }

        // set header name, value (to be set in the next loop, usually)
        lastName = lineText.substring(0, colon);
        lastVal = lineText.substring(colon + 1);
      } // empty, continuation, start of header
    } // while (true)
  }
};


/** The character codes for CR and LF. */
const CR = 0x0D, LF = 0x0A;

/**
 * Calculates the number of characters before the first CRLF pair in array, or
 * -1 if the array contains no CRLF pair.
 *
 * @param array : Array
 *   an array of numbers in the range [0, 256), each representing a single
 *   character; the first CRLF is the lowest index i where
 *   |array[i] == "\r".charCodeAt(0)| and |array[i+1] == "\n".charCodeAt(0)|,
 *   if such an |i| exists, and -1 otherwise
 * @returns int
 *   the index of the first CRLF if any were present, -1 otherwise
 */
function findCRLF(array)
{
  for (var i = array.indexOf(CR); i >= 0; i = array.indexOf(CR, i + 1))
  {
    if (array[i + 1] == LF)
      return i;
  }
  return -1;
}


/**
 * A container which provides line-by-line access to the arrays of bytes with
 * which it is seeded.
 */
function LineData()
{
  /** An array of queued bytes from which to get line-based characters. */
  this._data = [];
}
LineData.prototype =
{
  /**
   * Appends the bytes in the given array to the internal data cache maintained
   * by this.
   */
  appendBytes: function(bytes)
  {
    Array.prototype.push.apply(this._data, bytes);
  },

  /**
   * Removes and returns a line of data, delimited by CRLF, from this.
   *
   * @param out
   *   an object whose "value" property will be set to the first line of text
   *   present in this, sans CRLF, if this contains a full CRLF-delimited line
   *   of text; if this doesn't contain enough data, the value of the property
   *   is undefined
   * @returns boolean
   *   true if a full line of data could be read from the data in this, false
   *   otherwise
   */
  readLine: function(out)
  {
    var data = this._data;
    var length = findCRLF(data);
    if (length < 0)
      return false;

    //
    // We have the index of the CR, so remove all the characters, including
    // CRLF, from the array with splice, and convert the removed array into the
    // corresponding string, from which we then strip the trailing CRLF.
    //
    // Getting the line in this matter acknowledges that substring is an O(1)
    // operation in SpiderMonkey because strings are immutable, whereas two
    // splices, both from the beginning of the data, are less likely to be as
    // cheap as a single splice plus two extra character conversions.
    //
    var line = String.fromCharCode.apply(null, data.splice(0, length + 2));
    out.value = line.substring(0, length);

    return true;
  },

  /**
   * Retrieve any bytes we may have overread from the request's postdata.  After
   * this method is called, this must not be used in any way.
   *
   * @returns Array
   *   the bytes read past the CRLFCRLF at the end of request headers
   */
  purge: function()
  {
    var data = this._data;
    this._data = null;
    return data;
  }
};



/**
 * Creates a request-handling function for an nsIHttpRequestHandler object.
 */
function createHandlerFunc(handler)
{
  return function(metadata, response) { handler.handle(metadata, response); };
}


/**
 * The default handler for directories; writes an HTML response containing a
 * slightly-formatted directory listing.
 */
function defaultIndexHandler(metadata, response)
{
  response.setHeader("Content-Type", "text/html", false);

  var path = htmlEscape(decodeURI(metadata.path));

  //
  // Just do a very basic bit of directory listings -- no need for too much
  // fanciness, especially since we don't have a style sheet in which we can
  // stick rules (don't want to pollute the default path-space).
  //

  var body = '<html>\
                <head>\
                  <title>' + path + '</title>\
                </head>\
                <body>\
                  <h1>' + path + '</h1>\
                  <ol style="list-style-type: none">';

  var directory = metadata.getProperty("directory");
  NS_ASSERT(directory && directory.isDirectory());

  var fileList = [];
  var files = directory.directoryEntries;
  while (files.hasMoreElements())
  {
    var f = files.getNext().QueryInterface(Ci.nsIFile);
    var name = f.leafName;
    if (!f.isHidden() &&
        (name.charAt(name.length - 1) != HIDDEN_CHAR ||
         name.charAt(name.length - 2) == HIDDEN_CHAR))
      fileList.push(f);
  }

  fileList.sort(fileSort);

  for (var i = 0; i < fileList.length; i++)
  {
    var file = fileList[i];
    try
    {
      var name = file.leafName;
      if (name.charAt(name.length - 1) == HIDDEN_CHAR)
        name = name.substring(0, name.length - 1);
      var sep = file.isDirectory() ? "/" : "";

      // Note: using " to delimit the attribute here because encodeURIComponent
      //       passes through '.
      var item = '<li><a href="' + encodeURIComponent(name) + sep + '">' +
                   htmlEscape(name) + sep +
                 '</a></li>';

      body += item;
    }
    catch (e) { /* some file system error, ignore the file */ }
  }

  body    += '    </ol>\
                </body>\
              </html>';

  response.bodyOutputStream.write(body, body.length);
}

/**
 * Sorts a and b (nsIFile objects) into an aesthetically pleasing order.
 */
function fileSort(a, b)
{
  var dira = a.isDirectory(), dirb = b.isDirectory();

  if (dira && !dirb)
    return -1;
  if (dirb && !dira)
    return 1;

  var namea = a.leafName.toLowerCase(), nameb = b.leafName.toLowerCase();
  return nameb > namea ? -1 : 1;
}


/**
 * Converts an externally-provided path into an internal path for use in
 * determining file mappings.
 *
 * @param path
 *   the path to convert
 * @param encoded
 *   true if the given path should be passed through decodeURI prior to
 *   conversion
 * @throws URIError
 *   if path is incorrectly encoded
 */
function toInternalPath(path, encoded)
{
  if (encoded)
    path = decodeURI(path);

  var comps = path.split("/");
  for (var i = 0, sz = comps.length; i < sz; i++)
  {
    var comp = comps[i];
    if (comp.charAt(comp.length - 1) == HIDDEN_CHAR)
      comps[i] = comp + HIDDEN_CHAR;
  }
  return comps.join("/");
}


/**
 * Adds custom-specified headers for the given file to the given response, if
 * any such headers are specified.
 *
 * @param file
 *   the file on the disk which is to be written
 * @param metadata
 *   metadata about the incoming request
 * @param response
 *   the Response to which any specified headers/data should be written
 * @throws HTTP_500
 *   if an error occurred while processing custom-specified headers
 */
function maybeAddHeaders(file, metadata, response)
{
  var name = file.leafName;
  if (name.charAt(name.length - 1) == HIDDEN_CHAR)
    name = name.substring(0, name.length - 1);

  var headerFile = file.parent;
  headerFile.append(name + HEADERS_SUFFIX);

  if (!headerFile.exists())
    return;

  const PR_RDONLY = 0x01;
  var fis = new FileInputStream(headerFile, PR_RDONLY, 0444,
                                Ci.nsIFileInputStream.CLOSE_ON_EOF);

  var lis = new ConverterInputStream(fis, "UTF-8", 1024, 0x0);
  lis.QueryInterface(Ci.nsIUnicharLineInputStream);

  try
  {
    var line = {value: ""};
    var more = lis.readLine(line);

    if (!more && line.value == "")
      return;


    // request line

    var status = line.value;
    if (status.indexOf("HTTP ") == 0)
    {
      status = status.substring(5);
      var space = status.indexOf(" ");
      var code, description;
      if (space < 0)
      {
        code = status;
        description = "";
      }
      else
      {
        code = status.substring(0, space);
        description = status.substring(space + 1, status.length);
      }
    
      response.setStatusLine(metadata.httpVersion, parseInt(code, 10), description);

      line.value = "";
      more = lis.readLine(line);
    }

    // headers
    while (more || line.value != "")
    {
      var header = line.value;
      var colon = header.indexOf(":");

      response.setHeader(header.substring(0, colon),
                         header.substring(colon + 1, header.length),
                         false); // allow overriding server-set headers

      line.value = "";
      more = lis.readLine(line);
    }
  }
  catch (e)
  {
    dumpn("WARNING: error in headers for " + metadata.path + ": " + e);
    throw HTTP_500;
  }
}


/**
 * An object which handles requests for a server, executing default and
 * overridden behaviors as instructed by the code which uses and manipulates it.
 * Default behavior includes the paths / and /trace (diagnostics), with some
 * support for HTTP error pages for various codes and fallback to HTTP 500 if
 * those codes fail for any reason.
 *
 * @param server : nsHttpServer
 *   the server in which this handler is being used
 */
function ServerHandler(server)
{
  // FIELDS

  /**
   * The nsHttpServer instance associated with this handler.
   */
  this._server = server;

  /**
   * A variable used to ensure that all requests are fully complete before the
   * server shuts down, to avoid callbacks from compiled code calling back into
   * empty contexts.  See also the comment before this field is next modified.
   */
  this._pendingRequests = 0;

  /**
   * A FileMap object containing the set of path->nsILocalFile mappings for
   * all directory mappings set in the server (e.g., "/" for /var/www/html/,
   * "/foo/bar/" for /local/path/, and "/foo/bar/baz/" for /local/path2).
   *
   * Note carefully: the leading and trailing "/" in each path (not file) are
   * removed before insertion to simplify the code which uses this.  You have
   * been warned!
   */
  this._pathDirectoryMap = new FileMap();

  /**
   * Custom request handlers for the server in which this resides.  Path-handler
   * pairs are stored as property-value pairs in this property.
   *
   * @see ServerHandler.prototype._defaultPaths
   */
  this._overridePaths = {};

  /**
   * Custom request handlers for the error handlers in the server in which this
   * resides.  Path-handler pairs are stored as property-value pairs in this
   * property.
   *
   * @see ServerHandler.prototype._defaultErrors
   */
  this._overrideErrors = {};

  /**
   * Maps file extensions to their MIME types in the server, overriding any
   * mapping that might or might not exist in the MIME service.
   */
  this._mimeMappings = {};

  /**
   * The default handler for requests for directories, used to serve directories
   * when no index file is present.
   */
  this._indexHandler = defaultIndexHandler;
}
ServerHandler.prototype =
{
  // PUBLIC API

  /**
   * Handles a request to this server, responding to the request appropriately
   * and initiating server shutdown if necessary.
   *
   * This method never throws an exception.
   *
   * @param connection : Connection
   *   the connection for this request
   * @param metadata : Request
   *   request metadata as generated from the initial request
   */
  handleResponse: function(connection, metadata)
  {
    var response = new Response();

    var path = metadata.path;
    dumpn("*** path == " + path);

    try
    {
      try
      {
        // explicit paths first, then files based on existing directory mappings,
        // then (if the file doesn't exist) built-in server default paths
        if (path in this._overridePaths)
          this._overridePaths[path](metadata, response);
        else
          this._handleDefault(metadata, response);
      }
      catch (e)
      {
        response.recycle();

        if (!(e instanceof HttpError))
        {
          dumpn("*** unexpected error: e == " + e);
          throw HTTP_500;
        }
        if (e.code != 404)
          throw e;

        dumpn("*** default: " + (path in this._defaultPaths));

        if (path in this._defaultPaths)
          this._defaultPaths[path](metadata, response);
        else
          throw HTTP_404;
      }
    }
    catch (e)
    {
      var errorCode = "internal";

      try
      {
        if (!(e instanceof HttpError))
          throw e;

        errorCode = e.code;
        dumpn("*** errorCode == " + errorCode);

        response.recycle();

        this._handleError(errorCode, metadata, response);
      }
      catch (e2)
      {
        dumpn("*** error handling " + errorCode + " error: " +
              "e2 == " + e2 + ", shutting down server");

        response.destroy();
        connection.close();
        connection.server.stop();
        return;
      }
    }

    this._end(response, connection);
  },

  //
  // see nsIHttpServer.registerFile
  //
  registerFile: function(path, file)
  {
    if (!file)
    {
      dumpn("*** unregistering '" + path + "' mapping");
      delete this._overridePaths[path];
      return;
    }

    dumpn("*** registering '" + path + "' as mapping to " + file.path);
    file = file.clone();

    var self = this;
    this._overridePaths[path] =
      function(metadata, response)
      {
        if (!file.exists())
          throw HTTP_404;

        response.setStatusLine(metadata.httpVersion, 200, "OK");
        self._writeFileResponse(metadata, file, response);
      };
  },

  //
  // see nsIHttpServer.registerPathHandler
  //
  registerPathHandler: function(path, handler)
  {
    // XXX true path validation!
    if (path.charAt(0) != "/")
      throw Cr.NS_ERROR_INVALID_ARG;

    this._handlerToField(handler, this._overridePaths, path);
  },

  //
  // see nsIHttpServer.registerDirectory
  //
  registerDirectory: function(path, directory)
  {
    // strip off leading and trailing '/' so that we can use lastIndexOf when
    // determining exactly how a path maps onto a mapped directory --
    // conditional is required here to deal with "/".substring(1, 0) being
    // converted to "/".substring(0, 1) per the JS specification
    var key = path.length == 1 ? "" : path.substring(1, path.length - 1);

    // the path-to-directory mapping code requires that the first character not
    // be "/", or it will go into an infinite loop
    if (key.charAt(0) == "/")
      throw Cr.NS_ERROR_INVALID_ARG;

    key = toInternalPath(key, false);

    if (directory)
    {
      dumpn("*** mapping '" + path + "' to the location " + directory.path);
      this._pathDirectoryMap.put(key, directory);
    }
    else
    {
      dumpn("*** removing mapping for '" + path + "'");
      this._pathDirectoryMap.put(key, null);
    }
  },

  //
  // see nsIHttpServer.registerErrorHandler
  //
  registerErrorHandler: function(err, handler)
  {
    if (!(err in HTTP_ERROR_CODES))
      dumpn("*** WARNING: registering non-HTTP/1.1 error code " +
            "(" + err + ") handler -- was this intentional?");

    this._handlerToField(handler, this._overrideErrors, err);
  },

  //
  // see nsIHttpServer.setIndexHandler
  //
  setIndexHandler: function(handler)
  {
    if (!handler)
      handler = defaultIndexHandler;
    else if (typeof(handler) != "function")
      handler = createHandlerFunc(handler);

    this._indexHandler = handler;
  },

  //
  // see nsIHttpServer.registerContentType
  //
  registerContentType: function(ext, type)
  {
    if (!type)
      delete this._mimeMappings[ext];
    else
      this._mimeMappings[ext] = headerUtils.normalizeFieldValue(type);
  },

  // NON-XPCOM PUBLIC API

  /**
   * Returns true if this handler is in the middle of handling any current
   * requests; this must be false before the server in which this is used may be
   * safely shut down.
   */
  hasPendingRequests: function()
  {
    return this._pendingRequests > 0;
  },


  // PRIVATE API

  /**
   * Sets or remove (if handler is null) a handler in an object with a key.
   *
   * @param handler
   *   a handler, either function or an nsIHttpRequestHandler
   * @param dict
   *   The object to attach the handler to.
   * @param key
   *   The field name of the handler.
   */
  _handlerToField: function(handler, dict, key)
  {
    // for convenience, handler can be a function if this is run from xpcshell
    if (typeof(handler) == "function")
      dict[key] = handler;
    else if (handler)
      dict[key] = createHandlerFunc(handler);
    else
      delete dict[key];
  },

  /**
   * Handles a request which maps to a file in the local filesystem (if a base
   * path has already been set; otherwise the 404 error is thrown).
   *
   * @param metadata : Request
   *   metadata for the incoming request
   * @param response : Response
   *   an uninitialized Response to the given request, to be initialized by a
   *   request handler
   * @throws HTTP_###
   *   if an HTTP error occurred (usually HTTP_404); note that in this case the
   *   calling code must handle cleanup of the response by calling .destroy()
   *   or .recycle()
   */
  _handleDefault: function(metadata, response)
  {
    response.setStatusLine(metadata.httpVersion, 200, "OK");

    var path = metadata.path;
    NS_ASSERT(path.charAt(0) == "/", "invalid path: <" + path + ">");

    // determine the actual on-disk file; this requires finding the deepest
    // path-to-directory mapping in the requested URL
    var file = this._getFileForPath(path);

    // the "file" might be a directory, in which case we either serve the
    // contained index.html or make the index handler write the response
    if (file.exists() && file.isDirectory())
    {
      file.append("index.html"); // make configurable?
      if (!file.exists() || file.isDirectory())
      {
        metadata._ensurePropertyBag();
        metadata._bag.setPropertyAsInterface("directory", file.parent);
        this._indexHandler(metadata, response);
        return;
      }
    }

    // alternately, the file might not exist
    if (!file.exists())
      throw HTTP_404;

    // finally...
    dumpn("*** handling '" + path + "' as mapping to " + file.path);
    this._writeFileResponse(metadata, file, response);
  },

  /**
   * Writes an HTTP response for the given file, including setting headers for
   * file metadata.
   *
   * @param metadata : Request
   *   the Request for which a response is being generated
   * @param file : nsILocalFile
   *   the file which is to be sent in the response
   * @param response : Response
   *   the response to which the file should be written
   */
  _writeFileResponse: function(metadata, file, response)
  {
    const PR_RDONLY = 0x01;

    var type = this._getTypeFromFile(file);
    if (type == SJS_TYPE)
    {
      try
      {
        var fis = new FileInputStream(file, PR_RDONLY, 0444,
                                      Ci.nsIFileInputStream.CLOSE_ON_EOF);
        var sis = new ScriptableInputStream(fis);
        var s = Cu.Sandbox(gGlobalObject);
        Cu.evalInSandbox(sis.read(file.fileSize), s);
        s.handleRequest(metadata, response);
      }
      catch (e)
      {
        dumpn("*** error running SJS: " + e);
        throw HTTP_500;
      }
    }
    else
    {
      try
      {
        response.setHeader("Last-Modified",
                           toDateString(file.lastModifiedTime),
                           false);
      }
      catch (e) { /* lastModifiedTime threw, ignore */ }

      response.setHeader("Content-Type", type, false);
  
      var fis = new FileInputStream(file, PR_RDONLY, 0444,
                                    Ci.nsIFileInputStream.CLOSE_ON_EOF);
      response.bodyOutputStream.writeFrom(fis, file.fileSize);
      fis.close();
      
      maybeAddHeaders(file, metadata, response);
    }
  },

  /**
   * Gets a content-type for the given file, first by checking for any custom
   * MIME-types registered with this handler for the file's extension, second by
   * asking the global MIME service for a content-type, and finally by failing
   * over to application/octet-stream.
   *
   * @param file : nsIFile
   *   the nsIFile for which to get a file type
   * @returns string
   *   the best content-type which can be determined for the file
   */
  _getTypeFromFile: function(file)
  {
    try
    {
      var name = file.leafName;
      var dot = name.lastIndexOf(".");
      if (dot > 0)
      {
        var ext = name.slice(dot + 1);
        if (ext in this._mimeMappings)
          return this._mimeMappings[ext];
      }
      return Cc["@mozilla.org/uriloader/external-helper-app-service;1"]
               .getService(Ci.nsIMIMEService)
               .getTypeFromFile(file);
    }
    catch (e)
    {
      return "application/octet-stream";
    }
  },

  /**
   * Returns the nsILocalFile which corresponds to the path, as determined using
   * all registered path->directory mappings and any paths which are explicitly
   * overridden.
   *
   * @param path : string
   *   the server path for which a file should be retrieved, e.g. "/foo/bar"
   * @throws HttpError
   *   when the correct action is the corresponding HTTP error (i.e., because no
   *   mapping was found for a directory in path, the referenced file doesn't
   *   exist, etc.)
   * @returns nsILocalFile
   *   the file to be sent as the response to a request for the path
   */
  _getFileForPath: function(path)
  {
    // decode and add underscores as necessary
    try
    {
      path = toInternalPath(path, true);
    }
    catch (e)
    {
      throw HTTP_400; // malformed path
    }

    // next, get the directory which contains this path
    var pathMap = this._pathDirectoryMap;

    // An example progression of tmp for a path "/foo/bar/baz/" might be:
    // "foo/bar/baz/", "foo/bar/baz", "foo/bar", "foo", ""
    var tmp = path.substring(1);
    while (true)
    {
      // do we have a match for current head of the path?
      var file = pathMap.get(tmp);
      if (file)
      {
        // XXX hack; basically disable showing mapping for /foo/bar/ when the
        //     requested path was /foo/bar, because relative links on the page
        //     will all be incorrect -- we really need the ability to easily
        //     redirect here instead
        if (tmp == path.substring(1) &&
            tmp.length != 0 &&
            tmp.charAt(tmp.length - 1) != "/")
          file = null;
        else
          break;
      }

      // if we've finished trying all prefixes, exit
      if (tmp == "")
        break;

      tmp = tmp.substring(0, tmp.lastIndexOf("/"));
    }

    // no mapping applies, so 404
    if (!file)
      throw HTTP_404;


    // last, get the file for the path within the determined directory
    var parentFolder = file.parent;
    var dirIsRoot = (parentFolder == null);

    // Strategy here is to append components individually, making sure we
    // never move above the given directory; this allows paths such as
    // "<file>/foo/../bar" but prevents paths such as "<file>/../base-sibling";
    // this component-wise approach also means the code works even on platforms
    // which don't use "/" as the directory separator, such as Windows
    var leafPath = path.substring(tmp.length + 1);
    var comps = leafPath.split("/");
    for (var i = 0, sz = comps.length; i < sz; i++)
    {
      var comp = comps[i];

      if (comp == "..")
        file = file.parent;
      else if (comp == "." || comp == "")
        continue;
      else
        file.append(comp);

      if (!dirIsRoot && file.equals(parentFolder))
        throw HTTP_403;
    }

    return file;
  },

  /**
   * Writes the error page for the given HTTP error code over the given
   * connection.
   *
   * @param errorCode : uint
   *   the HTTP error code to be used
   * @param connection : Connection
   *   the connection on which the error occurred
   */
  handleError: function(errorCode, connection)
  {
    var response = new Response();

    dumpn("*** error in request: " + errorCode);

    try
    {
      this._handleError(errorCode, new Request(connection.port), response);
      this._end(response, connection);
    }
    catch (e)
    {
      connection.close();
      connection.server.stop();
    }
  }, 

  /**
   * Handles a request which generates the given error code, using the
   * user-defined error handler if one has been set, gracefully falling back to
   * the x00 status code if the code has no handler, and failing to status code
   * 500 if all else fails.
   *
   * @param errorCode : uint
   *   the HTTP error which is to be returned
   * @param metadata : Request
   *   metadata for the request, which will often be incomplete since this is an
   *   error
   * @param response : Response
   *   an uninitialized Response should be initialized when this method
   *   completes with information which represents the desired error code in the
   *   ideal case or a fallback code in abnormal circumstances (i.e., 500 is a
   *   fallback for 505, per HTTP specs)
   */
  _handleError: function(errorCode, metadata, response)
  {
    if (!metadata)
      throw Cr.NS_ERROR_NULL_POINTER;

    var errorX00 = errorCode - (errorCode % 100);

    try
    {
      if (!(errorCode in HTTP_ERROR_CODES))
        dumpn("*** WARNING: requested invalid error: " + errorCode);

      // RFC 2616 says that we should try to handle an error by its class if we
      // can't otherwise handle it -- if that fails, we revert to handling it as
      // a 500 internal server error, and if that fails we throw and shut down
      // the server

      // actually handle the error
      try
      {
        if (errorCode in this._overrideErrors)
          this._overrideErrors[errorCode](metadata, response);
        else if (errorCode in this._defaultErrors)
          this._defaultErrors[errorCode](metadata, response);
      }
      catch (e)
      {
        // don't retry the handler that threw
        if (errorX00 == errorCode)
          throw HTTP_500;

        dumpn("*** error in handling for error code " + errorCode + ", " +
              "falling back to " + errorX00 + "...");
        if (errorX00 in this._overrideErrors)
          this._overrideErrors[errorX00](metadata, response);
        else if (errorX00 in this._defaultErrors)
          this._defaultErrors[errorX00](metadata, response);
        else
          throw HTTP_500;
      }
    }
    catch (e)
    {
      response.recycle();

      // we've tried everything possible for a meaningful error -- now try 500
      dumpn("*** error in handling for error code " + errorX00 + ", falling " +
            "back to 500...");

      try
      {
        if (500 in this._overrideErrors)
          this._overrideErrors[500](metadata, response);
        else
          this._defaultErrors[500](metadata, response);
      }
      catch (e2)
      {
        dumpn("*** multiple errors in default error handlers!");
        dumpn("*** e == " + e + ", e2 == " + e2);
        throw e2;
      }
    }
  },

  /**
   * Called when all processing necessary for the current request has completed
   * and response headers and data have been determined.  This method takes
   * those headers and data, sends them to the HTTP client, and halts further
   * processing.  It will also send a quit message to the server if necessary.
   *
   * This method never throws an exception.
   *
   * @param response : Response
   *   the desired response
   * @param connection : Connection
   *   the connection associated with the given response
   * @note
   *   after completion, response must be considered "dead", and none of its
   *   methods or properties may be accessed
   */
  _end:  function(response, connection)
  {
    // post-processing
    response.setHeader("Connection", "close", false);
    response.setHeader("Server", "httpd.js", false);
    response.setHeader("Date", toDateString(Date.now()), false);

    var bodyStream = response.bodyInputStream;

    // XXX suckage time!
    //
    // If the body of the response has had no data written to it (or has never
    // been accessed -- same deal internally since we'll create one if we have
    // to access bodyInputStream but have neither an input stream nor an
    // output stream), the in-tree implementation of nsIPipe is such that
    // when we try to close the pipe's output stream with no data in it, this
    // is interpreted as an error and closing the output stream also closes
    // the input stream.  .available then throws, so we catch and deal as best
    // as we can.
    //
    // Unfortunately, the easy alternative (substitute a storage stream for a
    // pipe) also doesn't work.  There's no problem writing zero bytes to the
    // output end of the stream, but then attempting to get an input stream to
    // read fails because the seek position must be strictly less than the
    // buffer size.
    //
    // Much as I'd like the zero-byte body to be a mostly-unimportant problem,
    // there are some HTTP responses such as 304 Not Modified which MUST have
    // zero-byte bodies, so this *is* a necessary hack.
    try
    {
      var available = bodyStream.available();
    }
    catch (e)
    {
      available = 0;
    }

    response.setHeader("Content-Length", available.toString(), false);


    // construct and send response

    // request-line
    var preamble = "HTTP/" + response.httpVersion + " " +
                   response.httpCode + " " +
                   response.httpDescription + "\r\n";

    // headers
    var head = response.headers;
    var headEnum = head.enumerator;
    while (headEnum.hasMoreElements())
    {
      var fieldName = headEnum.getNext()
                              .QueryInterface(Ci.nsISupportsString)
                              .data;
      preamble += fieldName + ": " + head.getHeader(fieldName) + "\r\n";
    }

    // end request-line/headers
    preamble += "\r\n";

    var outStream = connection.output;
    try
    {
      outStream.write(preamble, preamble.length);
    }
    catch (e)
    {
      // Connection closed already?  Even if not, failure to write the response
      // means we probably will fail later anyway, so in the interests of
      // avoiding exceptions we'll (possibly) close the connection and return.
      response.destroy();
      connection.close();
      return;
    }

    // In certain situations, it's possible for us to have a race between
    // the copy observer's onStopRequest and the listener for a channel
    // opened to this server.  Since we include a Content-Length header with
    // every response, if the channel snarfs up all the data we promise,
    // calls onStopRequest on the listener (and the server is shut down
    // by that listener, causing the script to finish executing), and then
    // tries to call onStopRequest on the copyObserver, we'll call into a
    // scope with no Components and cause assertions *and* fail to close the
    // connection properly.  To combat this, during server shutdown we delay
    // full shutdown until any pending requests are fully copied using this
    // property on the server handler.  We increment before (possibly)
    // starting the copy observer and decrement when the copy completes,
    // ensuring that all copies complete before the server fully shuts down.
    //
    // We do this for every request primarily to simplify maintenance of this
    // property (and also because it's less fragile when we can remove the
    // zero-sized body hack used above).
    this._pendingRequests++;

    var server = this._server;

    // If we have a body, send it -- if we don't, then don't bother with a
    // heavyweight async copy which doesn't need to happen and just do
    // response post-processing (usually handled by the copy observer)
    // directly
    if (available != 0)
    {
      /**
       * Observer of the copying of data from the body stream generated by a
       * request handler to the output stream for the server socket.  It
       * handles all post-request-writing cleanup details, such as closing
       * open streams and shutting down the server in case of errors.
       */
      var copyObserver =
        {
          onStartRequest: function(request, context) { /* don't care */ },

          /**
           * Called when the async stream copy completes.  This is place where
           * final cleanup should occur, including stream closures and
           * response destruction.  Note that errors which are detected here
           * should still shut down the server, for safety.
           */
          onStopRequest: function(request, cx, statusCode)
          {
            // statusCode can indicate user-triggered failures (e.g. if the user
            // closes the connection during the copy, which would cause a status
            // of NS_ERROR_NET_RESET), so don't treat its value being an error
            // code as catastrophic.  I can create this situation when running
            // Mochitests in a debug build by clicking the Stop button during
            // test execution, but it's not exactly a surefire way to reproduce
            // the problem.
            if (!Components.isSuccessCode(statusCode))
            {
              dumpn("*** WARNING: non-success statusCode in onStopRequest: " +
                    statusCode);
            }

            // we're completely finished with this response
            response.destroy();

            connection.end();
          },

          QueryInterface: function(aIID)
          {
            if (aIID.equals(Ci.nsIRequestObserver) ||
                aIID.equals(Ci.nsISupports))
              return this;

            throw Cr.NS_ERROR_NO_INTERFACE;
          }
        };


      //
      // Now write out the body, async since we might be serving this to
      // ourselves on the same thread, and writing too much data would deadlock.
      //
      var copier = new StreamCopier(bodyStream, outStream,
                                    null,
                                    true, true, 8192);
      copier.asyncCopy(copyObserver, null);
    }
    else
    {
      // finished with the response -- destroy
      response.destroy();
      this._server._endConnection(connection);
    }
  },

  // FIELDS

  /**
   * This object contains the default handlers for the various HTTP error codes.
   */
  _defaultErrors:
  {
    400: function(metadata, response)
    {
      // none of the data in metadata is reliable, so hard-code everything here
      response.setStatusLine("1.1", 400, "Bad Request");
      response.setHeader("Content-Type", "text/plain", false);

      var body = "Bad request\n";
      response.bodyOutputStream.write(body, body.length);
    },
    403: function(metadata, response)
    {
      response.setStatusLine(metadata.httpVersion, 403, "Forbidden");
      response.setHeader("Content-Type", "text/html", false);

      var body = "<html>\
                    <head><title>403 Forbidden</title></head>\
                    <body>\
                      <h1>403 Forbidden</h1>\
                    </body>\
                  </html>";
      response.bodyOutputStream.write(body, body.length);
    },
    404: function(metadata, response)
    {
      response.setStatusLine(metadata.httpVersion, 404, "Not Found");
      response.setHeader("Content-Type", "text/html", false);

      var body = "<html>\
                    <head><title>404 Not Found</title></head>\
                    <body>\
                      <h1>404 Not Found</h1>\
                      <p>\
                        <span style='font-family: monospace;'>" +
                          htmlEscape(metadata.path) +
                       "</span> was not found.\
                      </p>\
                    </body>\
                  </html>";
      response.bodyOutputStream.write(body, body.length);
    },
    500: function(metadata, response)
    {
      response.setStatusLine(metadata.httpVersion,
                             500,
                             "Internal Server Error");
      response.setHeader("Content-Type", "text/html", false);

      var body = "<html>\
                    <head><title>500 Internal Server Error</title></head>\
                    <body>\
                      <h1>500 Internal Server Error</h1>\
                      <p>Something's broken in this server and\
                        needs to be fixed.</p>\
                    </body>\
                  </html>";
      response.bodyOutputStream.write(body, body.length);
    },
    501: function(metadata, response)
    {
      response.setStatusLine(metadata.httpVersion, 501, "Not Implemented");
      response.setHeader("Content-Type", "text/html", false);

      var body = "<html>\
                    <head><title>501 Not Implemented</title></head>\
                    <body>\
                      <h1>501 Not Implemented</h1>\
                      <p>This server is not (yet) Apache.</p>\
                    </body>\
                  </html>";
      response.bodyOutputStream.write(body, body.length);
    },
    505: function(metadata, response)
    {
      response.setStatusLine("1.1", 505, "HTTP Version Not Supported");
      response.setHeader("Content-Type", "text/html", false);

      var body = "<html>\
                    <head><title>505 HTTP Version Not Supported</title></head>\
                    <body>\
                      <h1>505 HTTP Version Not Supported</h1>\
                      <p>This server only supports HTTP/1.0 and HTTP/1.1\
                        connections.</p>\
                    </body>\
                  </html>";
      response.bodyOutputStream.write(body, body.length);
    }
  },

  /**
   * Contains handlers for the default set of URIs contained in this server.
   */
  _defaultPaths:
  {
    "/": function(metadata, response)
    {
      response.setStatusLine(metadata.httpVersion, 200, "OK");
      response.setHeader("Content-Type", "text/html", false);

      var body = "<html>\
                    <head><title>MozJSHTTP</title></head>\
                    <body>\
                      <h1>MozJSHTTP</h1>\
                      <p>If you're seeing this page, MozJSHTTP is up and\
                        serving requests!  Now set a base path and serve some\
                        files!</p>\
                    </body>\
                  </html>";

      response.bodyOutputStream.write(body, body.length);
    },

    "/trace": function(metadata, response)
    {
      response.setStatusLine(metadata.httpVersion, 200, "OK");
      response.setHeader("Content-Type", "text/plain", false);

      var body = "Request (semantically equivalent, slightly reformatted):\n\n";
      body += metadata.method + " " + metadata.path;

      if (metadata.queryString)
        body +=  "?" + metadata.queryString;
        
      body += " HTTP/" + metadata.httpVersion + "\r\n";

      var headEnum = metadata.headers;
      while (headEnum.hasMoreElements())
      {
        var fieldName = headEnum.getNext()
                                .QueryInterface(Ci.nsISupportsString)
                                .data;
        body += fieldName + ": " + metadata.getHeader(fieldName) + "\r\n";
      }

      response.bodyOutputStream.write(body, body.length);
    }
  }
};


/**
 * Maps absolute paths to files on the local file system (as nsILocalFiles).
 */
function FileMap()
{
  /** Hash which will map paths to nsILocalFiles. */
  this._map = {};
}
FileMap.prototype =
{
  // PUBLIC API

  /**
   * Maps key to a clone of the nsILocalFile value if value is non-null;
   * otherwise, removes any extant mapping for key.
   *
   * @param key : string
   *   string to which a clone of value is mapped
   * @param value : nsILocalFile
   *   the file to map to key, or null to remove a mapping
   */
  put: function(key, value)
  {
    if (value)
      this._map[key] = value.clone();
    else
      delete this._map[key];
  },

  /**
   * Returns a clone of the nsILocalFile mapped to key, or null if no such
   * mapping exists.
   *
   * @param key : string
   *   key to which the returned file maps
   * @returns nsILocalFile
   *   a clone of the mapped file, or null if no mapping exists
   */
  get: function(key)
  {
    var val = this._map[key];
    return val ? val.clone() : null;
  }
};


// Response CONSTANTS

// token       = *<any CHAR except CTLs or separators>
// CHAR        = <any US-ASCII character (0-127)>
// CTL         = <any US-ASCII control character (0-31) and DEL (127)>
// separators  = "(" | ")" | "<" | ">" | "@"
//             | "," | ";" | ":" | "\" | <">
//             | "/" | "[" | "]" | "?" | "="
//             | "{" | "}" | SP  | HT
const IS_TOKEN_ARRAY =
  [0, 0, 0, 0, 0, 0, 0, 0, //   0
   0, 0, 0, 0, 0, 0, 0, 0, //   8
   0, 0, 0, 0, 0, 0, 0, 0, //  16
   0, 0, 0, 0, 0, 0, 0, 0, //  24

   0, 1, 0, 1, 1, 1, 1, 1, //  32
   0, 0, 1, 1, 0, 1, 1, 0, //  40
   1, 1, 1, 1, 1, 1, 1, 1, //  48
   1, 1, 0, 0, 0, 0, 0, 0, //  56

   0, 1, 1, 1, 1, 1, 1, 1, //  64
   1, 1, 1, 1, 1, 1, 1, 1, //  72
   1, 1, 1, 1, 1, 1, 1, 1, //  80
   1, 1, 1, 0, 0, 0, 1, 1, //  88

   1, 1, 1, 1, 1, 1, 1, 1, //  96
   1, 1, 1, 1, 1, 1, 1, 1, // 104
   1, 1, 1, 1, 1, 1, 1, 1, // 112
   1, 1, 1, 0, 1, 0, 1];   // 120


/**
 * Determines whether the given character code is a CTL.
 *
 * @param code : uint
 *   the character code
 * @returns boolean
 *   true if code is a CTL, false otherwise
 */
function isCTL(code)
{
  return (code >= 0 && code <= 31) || (code == 127);
}

/**
 * Represents a response to an HTTP request, encapsulating all details of that
 * response.  This includes all headers, the HTTP version, status code and
 * explanation, and the entity itself.
 */
function Response()
{
  // delegate initialization behavior to .recycle(), for code-sharing;
  // see there for field descriptions as well
  this.recycle();
}
Response.prototype =
{
  // PUBLIC CONSTRUCTION API

  //
  // see nsIHttpResponse.bodyOutputStream
  //
  get bodyOutputStream()
  {
    this._ensureAlive();

    if (!this._bodyOutputStream && !this._outputProcessed)
    {
      const PR_UINT32_MAX = Math.pow(2, 32) - 1;
      var pipe = new Pipe(false, false, 0, PR_UINT32_MAX, null);
      this._bodyOutputStream = pipe.outputStream;
      this._bodyInputStream = pipe.inputStream;
    }

    return this._bodyOutputStream;
  },

  //
  // see nsIHttpResponse.write
  //
  write: function(data)
  {
    var dataAsString = String(data);
    this.bodyOutputStream.write(dataAsString, dataAsString.length);
  },

  //
  // see nsIHttpResponse.setStatusLine
  //
  setStatusLine: function(httpVersion, code, description)
  {
    this._ensureAlive();

    if (!(code >= 0 && code < 1000))
      throw Cr.NS_ERROR_INVALID_ARG;

    try
    {
      var httpVer;
      // avoid version construction for the most common cases
      if (!httpVersion || httpVersion == "1.1")
        httpVer = nsHttpVersion.HTTP_1_1;
      else if (httpVersion == "1.0")
        httpVer = nsHttpVersion.HTTP_1_0;
      else
        httpVer = new nsHttpVersion(httpVersion);
    }
    catch (e)
    {
      throw Cr.NS_ERROR_INVALID_ARG;
    }

    // Reason-Phrase = *<TEXT, excluding CR, LF>
    // TEXT          = <any OCTET except CTLs, but including LWS>
    //
    // XXX this ends up disallowing octets which aren't Unicode, I think -- not
    //     much to do if description is IDL'd as string
    if (!description)
      description = "";
    for (var i = 0; i < description.length; i++)
      if (isCTL(description.charCodeAt(i)) && description.charAt(i) != "\t")
        throw Cr.NS_ERROR_INVALID_ARG;

    // set the values only after validation to preserve atomicity
    this._httpDescription = description;
    this._httpCode = code;
    this._httpVersion = httpVer;
  },

  //
  // see nsIHttpResponse.setHeader
  //
  setHeader: function(name, value, merge)
  {
    this._ensureAlive();

    this._headers.setHeader(name, value, merge);
  },

  // POST-CONSTRUCTION API (not exposed externally)

  /**
   * The HTTP version number of this, as a string (e.g. "1.1").
   */
  get httpVersion()
  {
    this._ensureAlive();

    return this._httpVersion.toString();
  },

  /**
   * The HTTP status code of this response, as a string of three characters per
   * RFC 2616.
   */
  get httpCode()
  {
    this._ensureAlive();

    var codeString = (this._httpCode < 10 ? "0" : "") +
                     (this._httpCode < 100 ? "0" : "") +
                     this._httpCode;
    return codeString;
  },

  /**
   * The description of the HTTP status code of this response, or "" if none is
   * set.
   */
  get httpDescription()
  {
    this._ensureAlive();

    return this._httpDescription;
  },

  /**
   * The headers in this response, as an nsHttpHeaders object.
   */
  get headers()
  {
    this._ensureAlive();

    return this._headers;
  },

  //
  // see nsHttpHeaders.getHeader
  //
  getHeader: function(name)
  {
    this._ensureAlive();

    return this._headers.getHeader(name);
  },

  /**
   * A stream containing the data stored in the body of this response, which is
   * the data written to this.bodyOutputStream.  Accessing this property will
   * prevent further writes to bodyOutputStream and will remove that property
   * from this, so the only time this should be accessed should be after this
   * Response has been fully processed by a request handler.
   */
  get bodyInputStream()
  {
    this._ensureAlive();

    if (!this._outputProcessed)
    {
      // if nothing was ever written to bodyOutputStream, we never created a
      // pipe -- do so now by writing the empty string to this.bodyOutputStream
      if (!this._bodyOutputStream)
        this.bodyOutputStream.write("", 0);

      this._outputProcessed = true;
    }
    if (this._bodyOutputStream)
    {
      this._bodyOutputStream.close(); // flushes buffered data
      this._bodyOutputStream = null;  // don't try closing again
    }
    return this._bodyInputStream;
  },

  /**
   * Resets this response to its original state, destroying any currently-held
   * resources in the process.  Use this method to invalidate an existing
   * response and reuse it later, such as when an arbitrary handler has
   * failed and may have altered the visible state of this (such as by
   * setting headers).
   *
   * This method may be called on Responses which have been .destroy()ed.
   */
  recycle: function()
  {
    if (this._bodyOutputStream)
    {
      this._bodyOutputStream.close();
      this._bodyOutputStream = null;
    }
    if (this._bodyInputStream)
    {
      this._bodyInputStream.close();
      this._bodyInputStream = null;
    }

    /**
     * The HTTP version of this response; defaults to 1.1 if not set by the
     * handler.
     */
    this._httpVersion = nsHttpVersion.HTTP_1_1;

    /**
     * The HTTP code of this response; defaults to 200.
     */
    this._httpCode = 200;

    /**
     * The description of the HTTP code in this response; defaults to "OK".
     */
    this._httpDescription = "OK";

    /**
     * An nsIHttpHeaders object in which the headers in this response should be
     * stored.
     */
    this._headers = new nsHttpHeaders();

    /**
     * Set to true when this has its .destroy() method called; further actions on
     * this will then fail.
     */
    this._destroyed = false;

    /**
     * Flipped when this.bodyOutputStream is closed; prevents the body from being
     * reopened after it has data written to it and has been closed.
     */
    this._outputProcessed = false;
  },

  /**
   * Destroys all resources held by this.  After this method is called, no other
   * method or property on this must be accessed (except .recycle, which may be
   * used to reuse this Response).  Although in many situations resources may be
   * automagically cleaned up, it is highly recommended that this method be
   * called whenever a Response is no longer used, both as a precaution and
   * because this implementation may not always do so.
   *
   * This method is idempotent.
   */
  destroy: function()
  {
    if (this._destroyed)
      return;

    if (this._bodyOutputStream)
    {
      this._bodyOutputStream.close();
      this._bodyOutputStream = null;
    }
    if (this._bodyInputStream)
    {
      this._bodyInputStream.close();
      this._bodyInputStream = null;
    }

    this._destroyed = true;
  },

  // PRIVATE IMPLEMENTATION

  /** Ensures that this hasn't had its .destroy() method called. */
  _ensureAlive: function()
  {
    if (this._destroyed)
      throw Cr.NS_ERROR_FAILURE;
  }
};


/**
 * A container for utility functions used with HTTP headers.
 */
const headerUtils =
{
  /**
   * Normalizes fieldName (by converting it to lowercase) and ensures it is a
   * valid header field name (although not necessarily one specified in RFC
   * 2616).
   *
   * @throws NS_ERROR_INVALID_ARG
   *   if fieldName does not match the field-name production in RFC 2616
   * @returns string
   *   fieldName converted to lowercase if it is a valid header, for characters
   *   where case conversion is possible
   */
  normalizeFieldName: function(fieldName)
  {
    if (fieldName == "")
      throw Cr.NS_ERROR_INVALID_ARG;

    for (var i = 0, sz = fieldName.length; i < sz; i++)
    {
      if (!IS_TOKEN_ARRAY[fieldName.charCodeAt(i)])
      {
        dumpn(fieldName + " is not a valid header field name!");
        throw Cr.NS_ERROR_INVALID_ARG;
      }
    }

    return fieldName.toLowerCase();
  },

  /**
   * Ensures that fieldValue is a valid header field value (although not
   * necessarily as specified in RFC 2616 if the corresponding field name is
   * part of the HTTP protocol), normalizes the value if it is, and
   * returns the normalized value.
   *
   * @param fieldValue : string
   *   a value to be normalized as an HTTP header field value
   * @throws NS_ERROR_INVALID_ARG
   *   if fieldValue does not match the field-value production in RFC 2616
   * @returns string
   *   fieldValue as a normalized HTTP header field value
   */
  normalizeFieldValue: function(fieldValue)
  {
    // field-value    = *( field-content | LWS )
    // field-content  = <the OCTETs making up the field-value
    //                  and consisting of either *TEXT or combinations
    //                  of token, separators, and quoted-string>
    // TEXT           = <any OCTET except CTLs,
    //                  but including LWS>
    // LWS            = [CRLF] 1*( SP | HT )
    //
    // quoted-string  = ( <"> *(qdtext | quoted-pair ) <"> )
    // qdtext         = <any TEXT except <">>
    // quoted-pair    = "\" CHAR
    // CHAR           = <any US-ASCII character (octets 0 - 127)>

    // Any LWS that occurs between field-content MAY be replaced with a single
    // SP before interpreting the field value or forwarding the message
    // downstream (section 4.2); we replace 1*LWS with a single SP
    var val = fieldValue.replace(/(?:(?:\r\n)?[ \t]+)+/g, " ");

    // remove leading/trailing LWS (which has been converted to SP)
    val = val.replace(/^ +/, "").replace(/ +$/, "");

    // that should have taken care of all CTLs, so val should contain no CTLs
    for (var i = 0, len = val.length; i < len; i++)
      if (isCTL(val.charCodeAt(i)))
        throw Cr.NS_ERROR_INVALID_ARG;

    // XXX disallows quoted-pair where CHAR is a CTL -- will not invalidly
    //     normalize, however, so this can be construed as a tightening of the
    //     spec and not entirely as a bug
    return val;
  }
};



/**
 * Converts the given string into a string which is safe for use in an HTML
 * context.
 *
 * @param str : string
 *   the string to make HTML-safe
 * @returns string
 *   an HTML-safe version of str
 */
function htmlEscape(str)
{
  // this is naive, but it'll work
  var s = "";
  for (var i = 0; i < str.length; i++)
    s += "&#" + str.charCodeAt(i) + ";";
  return s;
}


/**
 * Constructs an object representing an HTTP version (see section 3.1).
 *
 * @param versionString
 *   a string of the form "#.#", where # is an non-negative decimal integer with
 *   or without leading zeros
 * @throws
 *   if versionString does not specify a valid HTTP version number
 */
function nsHttpVersion(versionString)
{
  var matches = /^(\d+)\.(\d+)$/.exec(versionString);
  if (!matches)
    throw "Not a valid HTTP version!";

  /** The major version number of this, as a number. */
  this.major = parseInt(matches[1], 10);

  /** The minor version number of this, as a number. */
  this.minor = parseInt(matches[2], 10);

  if (isNaN(this.major) || isNaN(this.minor) ||
      this.major < 0    || this.minor < 0)
    throw "Not a valid HTTP version!";
}
nsHttpVersion.prototype =
{
  /**
   * Returns the standard string representation of the HTTP version represented
   * by this (e.g., "1.1").
   */
  toString: function ()
  {
    return this.major + "." + this.minor;
  },

  /**
   * Returns true if this represents the same HTTP version as otherVersion,
   * false otherwise.
   *
   * @param otherVersion : nsHttpVersion
   *   the version to compare against this
   */
  equals: function (otherVersion)
  {
    return this.major == otherVersion.major &&
           this.minor == otherVersion.minor;
  }
};

nsHttpVersion.HTTP_1_0 = new nsHttpVersion("1.0");
nsHttpVersion.HTTP_1_1 = new nsHttpVersion("1.1");


/**
 * An object which stores HTTP headers for a request or response.
 *
 * Note that since headers are case-insensitive, this object converts headers to
 * lowercase before storing them.  This allows the getHeader and hasHeader
 * methods to work correctly for any case of a header, but it means that the
 * values returned by .enumerator may not be equal case-sensitively to the
 * values passed to setHeader when adding headers to this.
 */
function nsHttpHeaders()
{
  /**
   * A hash of headers, with header field names as the keys and header field
   * values as the values.  Header field names are case-insensitive, but upon
   * insertion here they are converted to lowercase.  Header field values are
   * normalized upon insertion to contain no leading or trailing whitespace.
   *
   * Note also that per RFC 2616, section 4.2, two headers with the same name in
   * a message may be treated as one header with the same field name and a field
   * value consisting of the separate field values joined together with a "," in
   * their original order.  This hash stores multiple headers with the same name
   * in this manner.
   */
  this._headers = {};
}
nsHttpHeaders.prototype =
{
  /**
   * Sets the header represented by name and value in this.
   *
   * @param name : string
   *   the header name
   * @param value : string
   *   the header value
   * @throws NS_ERROR_INVALID_ARG
   *   if name or value is not a valid header component
   */
  setHeader: function(fieldName, fieldValue, merge)
  {
    var name = headerUtils.normalizeFieldName(fieldName);
    var value = headerUtils.normalizeFieldValue(fieldValue);

    if (merge && name in this._headers)
      this._headers[name] = this._headers[name] + "," + value;
    else
      this._headers[name] = value;
  },

  /**
   * Returns the value for the header specified by this.
   *
   * @throws NS_ERROR_INVALID_ARG
   *   if fieldName does not constitute a valid header field name
   * @throws NS_ERROR_NOT_AVAILABLE
   *   if the given header does not exist in this
   * @returns string
   *   the field value for the given header, possibly with non-semantic changes
   *   (i.e., leading/trailing whitespace stripped, whitespace runs replaced
   *   with spaces, etc.) at the option of the implementation
   */
  getHeader: function(fieldName)
  {
    var name = headerUtils.normalizeFieldName(fieldName);

    if (name in this._headers)
      return this._headers[name];
    else
      throw Cr.NS_ERROR_NOT_AVAILABLE;
  },

  /**
   * Returns true if a header with the given field name exists in this, false
   * otherwise.
   *
   * @param fieldName : string
   *   the field name whose existence is to be determined in this
   * @throws NS_ERROR_INVALID_ARG
   *   if fieldName does not constitute a valid header field name
   * @returns boolean
   *   true if the header's present, false otherwise
   */
  hasHeader: function(fieldName)
  {
    var name = headerUtils.normalizeFieldName(fieldName);
    return (name in this._headers);
  },

  /**
   * Returns a new enumerator over the field names of the headers in this, as
   * nsISupportsStrings.  The names returned will be in lowercase, regardless of
   * how they were input using setHeader (header names are case-insensitive per
   * RFC 2616).
   */
  get enumerator()
  {
    var headers = [];
    for (var i in this._headers)
    {
      var supports = new SupportsString();
      supports.data = i;
      headers.push(supports);
    }

    return new nsSimpleEnumerator(headers);
  }
};


/**
 * Constructs an nsISimpleEnumerator for the given array of items.
 *
 * @param items : Array
 *   the items, which must all implement nsISupports
 */
function nsSimpleEnumerator(items)
{
  this._items = items;
  this._nextIndex = 0;
}
nsSimpleEnumerator.prototype =
{
  hasMoreElements: function()
  {
    return this._nextIndex < this._items.length;
  },
  getNext: function()
  {
    if (!this.hasMoreElements())
      throw Cr.NS_ERROR_NOT_AVAILABLE;

    return this._items[this._nextIndex++];
  },
  QueryInterface: function(aIID)
  {
    if (Ci.nsISimpleEnumerator.equals(aIID) ||
        Ci.nsISupports.equals(aIID))
      return this;

    throw Cr.NS_ERROR_NO_INTERFACE;
  }
};


/**
 * A representation of the data in an HTTP request.
 *
 * @param port : uint
 *   the port on which the server receiving this request runs
 */
function Request(port)
{
  this._method = "";
  this._path = "";
  this._queryString = "";
  this._host = "";
  this._port = port;
  this._host = "localhost"; // XXX or from environment or server itself?

  /**
   * The headers in this request.
   */
  this._headers = new nsHttpHeaders();

  /**
   * For the addition of ad-hoc properties and new functionality without having
   * to change nsIHttpRequestMetadata every time; currently lazily created,
   * as its only use is in directory listings.
   */
  this._bag = null;
}
Request.prototype =
{
  // SERVER METADATA

  //
  // see nsIHttpRequestMetadata.host
  //
  get host()
  {
    return this._host;
  },

  //
  // see nsIHttpRequestMetadata.port
  //
  get port()
  {
    return this._port;
  },

  // REQUEST LINE

  //
  // see nsIHttpRequestMetadata.method
  //
  get method()
  {
    return this._method;
  },

  //
  // see nsIHttpRequestMetadata.httpVersion
  //
  get httpVersion()
  {
    return this._httpVersion.toString();
  },

  //
  // see nsIHttpRequestMetadata.path
  //
  get path()
  {
    return this._path;
  },

  //
  // see nsIHttpRequestMetadata.queryString
  //
  get queryString()
  {
    return this._queryString;
  },

  // HEADERS

  //
  // see nsIHttpRequestMetadata.getHeader
  //
  getHeader: function(name)
  {
    return this._headers.getHeader(name);
  },

  //
  // see nsIHttpRequestMetadata.hasHeader
  //
  hasHeader: function(name)
  {
    return this._headers.hasHeader(name);
  },

  //
  // see nsIHttpRequestMetadata.headers
  //
  get headers()
  {
    return this._headers.enumerator;
  },

  //
  // see nsIPropertyBag.enumerator
  //
  get enumerator()
  {
    this._ensurePropertyBag();
    return this._bag.enumerator;
  },

  //
  // see nsIPropertyBag.getProperty
  //
  getProperty: function(name) 
  {
    this._ensurePropertyBag();
    return this._bag.getProperty(name);
  },
  
  /** Ensures a property bag has been created for ad-hoc behaviors. */
  _ensurePropertyBag: function()
  {
    if (!this._bag)
      this._bag = new WritablePropertyBag();
  }
};


// XPCOM trappings

/**
 * Creates a factory for instances of an object created using the passed-in
 * constructor.
 */
function makeFactory(ctor)
{
  function ci(outer, iid)
  {
    if (outer != null)
      throw Components.results.NS_ERROR_NO_AGGREGATION;
    return (new ctor()).QueryInterface(iid);
  } 

  return {
           createInstance: ci,
           lockFactory: function(lock) { },
           QueryInterface: function(aIID)
           {
             if (Ci.nsIFactory.equals(aIID) ||
                 Ci.nsISupports.equals(aIID))
               return this;
             throw Cr.NS_ERROR_NO_INTERFACE;
           }
         };
}

/** The XPCOM module containing the HTTP server. */
const module =
{
  // nsISupports
  QueryInterface: function(aIID)
  {
    if (Ci.nsIModule.equals(aIID) ||
        Ci.nsISupports.equals(aIID))
      return this;
    throw Cr.NS_ERROR_NO_INTERFACE;
  },

  // nsIModule
  registerSelf: function(compMgr, fileSpec, location, type)
  {
    compMgr = compMgr.QueryInterface(Ci.nsIComponentRegistrar);
    
    for (var key in this._objects)
    {
      var obj = this._objects[key];
      compMgr.registerFactoryLocation(obj.CID, obj.className, obj.contractID,
                                               fileSpec, location, type);
    }
  },
  unregisterSelf: function (compMgr, location, type)
  {
    compMgr = compMgr.QueryInterface(Ci.nsIComponentRegistrar);

    for (var key in this._objects)
    {
      var obj = this._objects[key];
      compMgr.unregisterFactoryLocation(obj.CID, location);
    }
  },
  getClassObject: function(compMgr, cid, iid)
  {
    if (!iid.equals(Ci.nsIFactory))
      throw Cr.NS_ERROR_NOT_IMPLEMENTED;

    for (var key in this._objects)
    {
      if (cid.equals(this._objects[key].CID))
        return this._objects[key].factory;
    }
    
    throw Cr.NS_ERROR_NO_INTERFACE;
  },
  canUnload: function(compMgr)
  {
    return true;
  },

  // private implementation
  _objects:
  {
    server:
    {
      CID:         Components.ID("{54ef6f81-30af-4b1d-ac55-8ba811293e41}"),
      contractID:  "@mozilla.org/server/jshttp;1",
      className:   "MozJSHTTP server",
      factory:     makeFactory(nsHttpServer)
    }
  }
};


/** NSGetModule, so this code can be used as a JS component. */
function NSGetModule(compMgr, fileSpec)
{
  return module;
}


/**
 * Creates a new HTTP server listening for loopback traffic on the given port,
 * starts it, and runs the server until the server processes a shutdown request,
 * spinning an event loop so that events posted by the server's socket are
 * processed.
 *
 * This method is primarily intended for use in running this script from within
 * xpcshell and running a functional HTTP server without having to deal with
 * non-essential details.
 *
 * Note that running multiple servers using variants of this method probably
 * doesn't work, simply due to how the internal event loop is spun and stopped.
 *
 * @note
 *   This method only works with Mozilla 1.9 (i.e., Firefox 3 or trunk code);
 *   you should use this server as a component in Mozilla 1.8.
 * @param port
 *   the port on which the server will run, or -1 if there exists no preference
 *   for a specific port; note that attempting to use some values for this
 *   parameter (particularly those below 1024) may cause this method to throw or
 *   may result in the server being prematurely shut down
 * @param basePath
 *   a local directory from which requests will be served (i.e., if this is
 *   "/home/jwalden/" then a request to /index.html will load
 *   /home/jwalden/index.html); if this is omitted, only the default URLs in
 *   this server implementation will be functional
 */
function server(port, basePath)
{
  if (basePath)
  {
    var lp = Cc["@mozilla.org/file/local;1"]
               .createInstance(Ci.nsILocalFile);
    lp.initWithPath(basePath);
  }

  // if you're running this, you probably want to see debugging info
  DEBUG = true;

  var srv = new nsHttpServer();
  if (lp)
    srv.registerDirectory("/", lp);
  srv.registerContentType("sjs", SJS_TYPE);
  srv.start(port);

  var thread = gThreadManager.currentThread;
  while (!srv.isStopped())
    thread.processNextEvent(true);

  // get rid of any pending requests
  while (thread.hasPendingEvents())
    thread.processNextEvent(true);

  DEBUG = false;
}
