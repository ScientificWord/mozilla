/* See license.txt for terms of usage */

// Debug lines are marked with  at column 120
// Use variable name "fileName" for href returned by JSD, file:/ not same as DOM
// Use variable name "url" for normalizedURL, file:/// comparable to DOM
// Convert from fileName to URL with normalizeURL
// We probably don't need denormalizeURL since we don't send .fileName back to JSD

// ************************************************************************************************
// Constants

const CLASS_ID = Components.ID("{a380e9c0-cb39-11da-a94d-0800200c9a66}");
const CLASS_NAME = "Firebug Service";
const CONTRACT_ID = "@joehewitt.com/firebug;1";
const Cc = Components.classes;
const Ci = Components.interfaces;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

const PrefService = Cc["@mozilla.org/preferences-service;1"];
const DebuggerService = Cc["@mozilla.org/js/jsd/debugger-service;1"];
const ConsoleService = Cc["@mozilla.org/consoleservice;1"];
const Timer = Cc["@mozilla.org/timer;1"];
const ObserverServiceFactory = Cc["@mozilla.org/observer-service;1"];

const jsdIDebuggerService = Ci.jsdIDebuggerService;
const jsdIScript = Ci.jsdIScript;
const jsdIStackFrame = Ci.jsdIStackFrame;
const jsdICallHook = Ci.jsdICallHook;
const jsdIExecutionHook = Ci.jsdIExecutionHook;
const jsdIErrorHook = Ci.jsdIErrorHook;
const jsdIFilter = Components.interfaces.jsdIFilter;
const nsISupports = Ci.nsISupports;
const nsIPrefBranch = Ci.nsIPrefBranch;
const nsIPrefBranch2 = Ci.nsIPrefBranch2;
const nsIComponentRegistrar = Ci.nsIComponentRegistrar;
const nsIFactory = Ci.nsIFactory;
const nsIConsoleService = Ci.nsIConsoleService;
const nsITimer = Ci.nsITimer;
const nsITimerCallback = Ci.nsITimerCallback;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

const NS_ERROR_NO_INTERFACE = Components.results.NS_ERROR_NO_INTERFACE;
const NS_ERROR_NOT_IMPLEMENTED = Components.results.NS_ERROR_NOT_IMPLEMENTED;
const NS_ERROR_NO_AGGREGATION = Components.results.NS_ERROR_NO_AGGREGATION;

const PCMAP_SOURCETEXT = jsdIScript.PCMAP_SOURCETEXT;
const PCMAP_PRETTYPRINT = jsdIScript.PCMAP_PRETTYPRINT;

const COLLECT_PROFILE_DATA = jsdIDebuggerService.COLLECT_PROFILE_DATA;
const DISABLE_OBJECT_TRACE = jsdIDebuggerService.DISABLE_OBJECT_TRACE;
const HIDE_DISABLED_FRAMES = jsdIDebuggerService.HIDE_DISABLED_FRAMES;
const DEBUG_WHEN_SET = jsdIDebuggerService.DEBUG_WHEN_SET;
const MASK_TOP_FRAME_ONLY = jsdIDebuggerService.MASK_TOP_FRAME_ONLY;

const TYPE_FUNCTION_CALL = jsdICallHook.TYPE_FUNCTION_CALL;
const TYPE_FUNCTION_RETURN = jsdICallHook.TYPE_FUNCTION_RETURN;
const TYPE_TOPLEVEL_START = jsdICallHook.TYPE_TOPLEVEL_START;
const TYPE_TOPLEVEL_END = jsdICallHook.TYPE_TOPLEVEL_END;

const RETURN_CONTINUE = jsdIExecutionHook.RETURN_CONTINUE;
const RETURN_VALUE = jsdIExecutionHook.RETURN_RET_WITH_VAL;
const RETURN_THROW_WITH_VAL = jsdIExecutionHook.RETURN_THROW_WITH_VAL;
const RETURN_CONTINUE_THROW = jsdIExecutionHook.RETURN_CONTINUE_THROW;

const NS_OS_TEMP_DIR = "TmpD"

const STEP_OVER = 1;
const STEP_INTO = 2;
const STEP_OUT = 3;
const STEP_SUSPEND = 4;

const TYPE_ONE_SHOT = nsITimer.TYPE_ONE_SHOT;

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

const BP_NORMAL = 1;
const BP_MONITOR = 2;
const BP_UNTIL = 4;
const BP_ONRELOAD = 8;  // XXXjjb: This is a mark for the UI to test
const BP_ERROR = 16;
const BP_TRACE = 32; // BP used to initiate traceCalls

const LEVEL_TOP = 1;
const LEVEL_EVAL = 2;
const LEVEL_EVENT = 3;

const COMPONENTS_FILTERS = [
    new RegExp("^(file:/.*/)extensions/%7B[\\da-fA-F]{8}-[\\da-fA-F]{4}-[\\da-fA-F]{4}-[\\da-fA-F]{4}-[\\da-fA-F]{12}%7D/components/.*\\.js$"),
    new RegExp("^(file:/.*/)extensions/firebug@software\\.joehewitt\\.com/components/.*\\.js$"),
    new RegExp("^(file:/.*/extensions/)\\w+@mozilla\\.org/components/.*\\.js$"),
    new RegExp("^(file:/.*/components/)ns[A-Z].*\\.js$"),
    new RegExp("^(file:/.*/components/)firebug-[^\\.]*\\.js$"),
    new RegExp("^(file:/.*/Contents/MacOS/extensions/.*/components/).*\\.js$"),
    new RegExp("^(file:/.*/modules/).*\\.jsm$"),
    ];

const reDBG = /DBG_(.*)/;
const reXUL = /\.xul$|\.xml$/;
const reTooMuchRecursion = /too\smuch\srecursion/;

// ************************************************************************************************
// Globals

var jsd, fbs, prefs;
var consoleService;
var observerService;

var contextCount = 0;

var urlFilters = [
    'chrome://',
    'XStringBundle',
    'x-jsd:ppbuffer?type=function', // internal script for pretty printing
    ];


var clients = [];
var debuggers = [];
var netDebuggers = [];
var scriptListeners = [];

var stepMode = 0;
var stepFrame;
var stepFrameLineId;
var stepStayOnDebuggr; // if set, the debuggr we want to stay within
var stepFrameCount;
var hookFrameCount = 0;

var haltDebugger = null;

var breakpoints = {};
var breakpointCount = 0;
var disabledCount = 0;
var monitorCount = 0;
var conditionCount = 0;
var runningUntil = null;

var errorBreakpoints = [];

var profileCount = 0;
var profileStart;

var enabledDebugger = false;
var reportNextError = false;
var errorInfo = null;

var timer = Timer.createInstance(nsITimer);
var waitingForTimer = false;

var FBTrace = null;

// ************************************************************************************************

function FirebugService()
{

    FBTrace = Cc["@joehewitt.com/firebug-trace-service;1"]
                 .getService(Ci.nsISupports).wrappedJSObject.getTracer("extensions.firebug");

    fbs = this;

    this.wrappedJSObject = this;
    this.timeStamp = new Date();  /* explore */
    this.breakpoints = breakpoints; // so chromebug can see it /* explore */
    this.onDebugRequests = 0;  // the number of times we called onError but did not call onDebug
    fbs._lastErrorDebuggr = null;


    this.profiling = false;
    this.pauseDepth = 0;

    prefs = PrefService.getService(nsIPrefBranch2);
    fbs.prefDomain = "extensions.firebug.service."
    prefs.addObserver(fbs.prefDomain, fbs, false);

    observerService = ObserverServiceFactory.getService(Ci.nsIObserverService);
    observerService.addObserver(QuitApplicationGrantedObserver, "quit-application-granted", false);
    observerService.addObserver(QuitApplicationRequestedObserver, "quit-application-requested", false);
    observerService.addObserver(QuitApplicationObserver, "quit-application", false);

    this.scriptsFilter = "all";
    // XXXjj For some reason the command line will not function if we allow chromebug to see it.?
    this.alwayFilterURLsStarting = ["chrome://chromebug", "x-jsd:ppbuffer", "chrome://firebug/content/commandLine.js"];  // TODO allow override
    this.onEvalScriptCreated.kind = "eval";
    this.onTopLevelScriptCreated.kind = "top-level";
    this.onEventScriptCreated.kind = "event";

    this.onXScriptCreatedByTag = {}; // fbs functions by script tag
    this.nestedScriptStack = Components.classes["@mozilla.org/array;1"]
                        .createInstance(Components.interfaces.nsIMutableArray);  // scripts contained in leveledScript that have not been drained
}

FirebugService.prototype =
{
    osOut: function(str)
    {
        if (!this.outChannel)
        {
            try
            {
                var appShellService = Components.classes["@mozilla.org/appshell/appShellService;1"].
                    getService(Components.interfaces.nsIAppShellService);
                this.hiddenWindow = appShellService.hiddenDOMWindow;
                this.outChannel = "hidden";
            }
            catch(exc)
            {
                consoleService = Cc["@mozilla.org/consoleservice;1"].getService(Ci.nsIConsoleService);
                consoleService.logStringMessage("Using consoleService because nsIAppShellService.hiddenDOMWindow not available "+exc);
                this.outChannel = "service";
            }
        }
        if (this.outChannel === "hidden")  // apparently can't call via JS function
            this.hiddenWindow.dump(str);
        else
            consoleService.logStringMessage(str);
    },

    shutdown: function()  // call disableDebugger first
    {
        timer = null;

        if (!jsd)
            return;

        try
        {
            do
            {
                var depth = jsd.exitNestedEventLoop();
            }
            while(depth > 0);
        }
        catch (exc)
        {
            // Seems to be the normal path...FBTrace.sysout("FirebugService, attempt to exitNestedEventLoop fails "+exc);
        }


        try
        {
            prefs.removeObserver(fbs.prefDomain, fbs);
        }
        catch (exc)
        {
            FBTrace.sysout("fbs prefs.removeObserver fails "+exc, exc);
        }

        try
        {
            observerService.removeObserver(QuitApplicationGrantedObserver, "quit-application-granted");
            observerService.removeObserver(QuitApplicationRequestedObserver, "quit-application-requested");
            observerService.removeObserver(QuitApplicationObserver, "quit-application");
        }
        catch (exc)
        {
            FBTrace.sysout("fbs quit-application-observers removeObserver fails "+exc, exc);
        }

        jsd = null;
    },

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
    // nsISupports

    QueryInterface: function(iid)
    {
        if (!iid.equals(nsISupports))
            throw NS_ERROR_NO_INTERFACE;

        return this;
    },

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
    // nsIObserver
    observe: function(subject, topic, data)
    {
        fbs.obeyPrefs();
    },
    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    get lastErrorWindow()
    {
        var win = this._lastErrorWindow;
        this._lastErrorWindow = null; // Release to avoid leaks
        return win;
    },

    registerClient: function(client)  // clients are essentially XUL windows
    {
        clients.push(client);
        return clients.length;
    },

    unregisterClient: function(client)
    {
        for (var i = 0; i < clients.length; ++i)
        {
            if (clients[i] == client)
            {
                clients.splice(i, 1);
                break;
            }
        }
    },

    registerDebugger: function(debuggrWrapper)  // first one in will be last one called. Returns state enabledDebugger
    {
        var debuggr = debuggrWrapper.wrappedJSObject;

        if (debuggr)
        {
            debuggers.push(debuggr);
            if (debuggers.length == 1)
                this.enableDebugger();
        }
        else
            throw "firebug-service debuggers must have wrappedJSObject";

        try {
            if (debuggr.suspendActivity)
                netDebuggers.push(debuggr);
        } catch(exc) {
        }
        try {
            if (debuggr.onScriptCreated)
                scriptListeners.push(debuggr);
        } catch(exc) {
        }
        return  debuggers.length;  // 1.3.1 return to allow Debugger to check progress
    },

    unregisterDebugger: function(debuggrWrapper)
    {
        var debuggr = debuggrWrapper.wrappedJSObject;

        for (var i = 0; i < debuggers.length; ++i)
        {
            if (debuggers[i] == debuggr)
            {
                debuggers.splice(i, 1);
                break;
            }
        }

        for (var i = 0; i < netDebuggers.length; ++i)
        {
            if (netDebuggers[i] == debuggr)
            {
                netDebuggers.splice(i, 1);
                break;
            }
        }
        for (var i = 0; i < scriptListeners.length; ++i)
        {
            if (scriptListeners[i] == debuggr)
            {
                scriptListeners.splice(i, 1);
                break;
            }
        }

        if (debuggers.length == 0)
            this.disableDebugger();

        return debuggers.length;
    },

    lockDebugger: function()
    {
        if (this.locked)
            return;

        this.locked = true;

        dispatch(debuggers, "onLock", [true]);
    },

    unlockDebugger: function()
    {
        if (!this.locked)
            return;

        this.locked = false;

        dispatch(debuggers, "onLock", [false]);
    },

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
    forceGarbageCollection: function()
    {
        jsd.GC(); // Force the engine to perform garbage collection.
    },
    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    enterNestedEventLoop: function(callback)
    {
        dispatch(netDebuggers, "suspendActivity");
        fbs.nestedEventLoopDepth = jsd.enterNestedEventLoop({
            onNest: function()
            {
                dispatch(netDebuggers, "resumeActivity");
                callback.onNest();
            }
        });
        dispatch(netDebuggers, "resumeActivity");
        return fbs.nestedEventLoopDepth;
    },

    exitNestedEventLoop: function()
    {
        dispatch(netDebuggers, "suspendActivity");
        try
        {
            return jsd.exitNestedEventLoop();
        }
        catch (exc)
        {
        }
    },

    halt: function(debuggr)
    {
        haltDebugger = debuggr;
    },

    step: function(mode, startFrame, stayOnDebuggr)
    {
        stepMode = mode;
        stepFrame = startFrame;
        stepFrameCount = countFrames(startFrame);
        stepFrameLineId = stepFrameCount + startFrame.script.fileName + startFrame.line;
        stepStayOnDebuggr = stayOnDebuggr;

    },

    suspend: function(stayOnDebuggr, context)
    {
        stepMode = STEP_SUSPEND;
        stepFrameLineId = null;
        stepStayOnDebuggr = stayOnDebuggr;

        dispatch(debuggers, "onBreakingNext", [stayOnDebuggr, context]);

        this.hookInterrupts();
    },

    runUntil: function(sourceFile, lineNo, startFrame, debuggr)
    {
        runningUntil = this.addBreakpoint(BP_UNTIL, sourceFile, lineNo, null, debuggr);
        stepFrameCount = countFrames(startFrame);
        stepFrameLineId = stepFrameCount + startFrame.script.fileName + startFrame.line;
    },

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    setBreakpoint: function(sourceFile, lineNo, props, debuggr)
    {
        var bp = this.addBreakpoint(BP_NORMAL, sourceFile, lineNo, props, debuggr);
        if (bp)
        {
            dispatch(debuggers, "onToggleBreakpoint", [sourceFile.href, lineNo, true, bp]);
            return true;
        }
        return false;
    },

    clearBreakpoint: function(url, lineNo)
    {
        var bp = this.removeBreakpoint(BP_NORMAL, url, lineNo);
        if (bp)
            dispatch(debuggers, "onToggleBreakpoint", [url, lineNo, false, bp]);
        else
        {
        }
    },

    enableBreakpoint: function(url, lineNo)
    {
        var bp = this.findBreakpoint(url, lineNo);
        if (bp && bp.type & BP_NORMAL)
        {
            bp.disabled &= ~BP_NORMAL;
            dispatch(debuggers, "onToggleBreakpoint", [url, lineNo, true, bp]);
            --disabledCount;
        }
        else {
        }
    },

    disableBreakpoint: function(url, lineNo)
    {
        var bp = this.findBreakpoint(url, lineNo);
        if (bp && bp.type & BP_NORMAL)
        {
            bp.disabled |= BP_NORMAL;
            ++disabledCount;
            dispatch(debuggers, "onToggleBreakpoint", [url, lineNo, true, bp]);
        }
        else
        {
        }

    },

    isBreakpointDisabled: function(url, lineNo)
    {
        var bp = this.findBreakpoint(url, lineNo);
        if (bp && bp.type & BP_NORMAL)
            return bp.disabled & BP_NORMAL;
        else
            return false;
    },

    setBreakpointCondition: function(sourceFile, lineNo, condition, debuggr)
    {
        var bp = this.findBreakpoint(sourceFile.href, lineNo);
        if (!bp)
        {
            bp = this.addBreakpoint(BP_NORMAL, sourceFile, lineNo, null, debuggr);
        }

        if (!bp)
            return;

        if (bp.hitCount <= 0 )
        {
            if (bp.condition && !condition)
            {
                --conditionCount;
            }
            else if (condition && !bp.condition)
            {
                ++conditionCount;
            }
        }
        bp.condition = condition;

        dispatch(debuggers, "onToggleBreakpoint", [sourceFile.href, lineNo, true, bp]);
    },

    getBreakpointCondition: function(url, lineNo)
    {
        var bp = this.findBreakpoint(url, lineNo);
        return bp ? bp.condition : "";
    },

    clearAllBreakpoints: function(sourceFiles)
    {
        for (var i = 0; i < sourceFiles.length; ++i)
        {
            var url = sourceFiles[i].href;
            if (!url)
                continue;

            var urlBreakpoints = breakpoints[url];
            if (!urlBreakpoints)
                continue;

            var removals = urlBreakpoints.length;
            for (var j = 0; j < removals; ++j)
            {
                var bp = urlBreakpoints[0];  // this one will be spliced out each time
                this.clearBreakpoint(url, bp.lineNo);
            }
         }
    },

    enumerateBreakpoints: function(url, cb)  // url is sourceFile.href, not jsd script.fileName
    {
        if (url)
        {
            var urlBreakpoints = breakpoints[url];
            if (urlBreakpoints)
            {
                for (var i = 0; i < urlBreakpoints.length; ++i)
                {
                    var bp = urlBreakpoints[i];
                    if (bp.type & BP_NORMAL)
                    {
                        if (bp.scriptsWithBreakpoint && bp.scriptsWithBreakpoint.length > 0)
                        {
                            for (var j = 0; j < bp.scriptsWithBreakpoint.length; j++)
                            {
                                var rc = cb.call(url, bp.lineNo, bp, bp.scriptsWithBreakpoint[j]);
                                if (rc)
                                    return [bp];
                            }
                        } else {
                            var rc = cb.call(url, bp.lineNo, bp);
                            if (rc)
                                return [bp];
                        }
                    }
                }
            }
        }
        else
        {
            var bps = [];
            for (var url in breakpoints)
                bps.push(this.enumerateBreakpoints(url, cb));
            return bps;
        }
    },

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
    // error breakpoints are a way of selecting breakpoint from the Console
    //
    setErrorBreakpoint: function(sourceFile, lineNo, debuggr)
    {
        var url = sourceFile.href;
        var index = this.findErrorBreakpoint(url, lineNo);
        if (index == -1)
        {
            this.setBreakpoint(sourceFile, lineNo, null, debuggr);
            errorBreakpoints.push({href: url, lineNo: lineNo, type: BP_ERROR });
            dispatch(debuggers, "onToggleErrorBreakpoint", [url, lineNo, true, debuggr]);
        }
    },

    clearErrorBreakpoint: function(sourceFile, lineNo, debuggr)
    {
        var url = sourceFile.href;
        var index = this.findErrorBreakpoint(url, lineNo);
        if (index != -1)
        {
            this.clearBreakpoint(url, lineNo);
            errorBreakpoints.splice(index, 1);

            dispatch(debuggers, "onToggleErrorBreakpoint", [url, lineNo, false, debuggr]);
        }
    },

    hasErrorBreakpoint: function(url, lineNo)
    {
        return this.findErrorBreakpoint(url, lineNo) != -1;
    },

    enumerateErrorBreakpoints: function(url, cb)
    {
        if (url)
        {
            for (var i = 0; i < errorBreakpoints.length; ++i)
            {
                var bp = errorBreakpoints[i];
                if (bp.href == url)
                    cb.call(bp.href, bp.lineNo, bp);
            }
        }
        else
        {
            for (var i = 0; i < errorBreakpoints.length; ++i)
            {
                var bp = errorBreakpoints[i];
                cb.call(bp.href, bp.lineNo, bp);
            }
        }
    },

    findErrorBreakpoint: function(url, lineNo)
    {
        for (var i = 0; i < errorBreakpoints.length; ++i)
        {
            var bp = errorBreakpoints[i];
            if (bp.lineNo == lineNo && bp.href == url)
                return i;
        }

        return -1;
    },

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    traceAll: function(urls, debuggr)
    {
        this.hookCalls(debuggr.onFunctionCall, false);  // call on all passed urls
    },

    untraceAll: function(debuggr)
    {
        jsd.functionHook = null; // undo hookCalls()
    },

    traceCalls: function(sourceFile, lineNo, debuggr)
    {
        var bp = this.monitor(sourceFile, lineNo, debuggr); // set a breakpoint on the starting point
        bp.type |= BP_TRACE;
        // when we hit the bp in onBreakPoint we being tracing.
    },

    untraceCalls: function(sourceFile, lineNo, debuggr)
    {
        var bp = lineNo != -1 ? this.findBreakpoint(url, lineNo) : null;
        if (bp)
        {
            bp.type &= ~BP_TRACE;
            this.unmonitor(sourceFile, lineNo);
        }
    },

    monitor: function(sourceFile, lineNo, debuggr)
    {
        if (lineNo == -1)
            return null;

        var bp = this.addBreakpoint(BP_MONITOR, sourceFile, lineNo, null, debuggr);
        if (bp)
        {
            ++monitorCount;
            dispatch(debuggers, "onToggleMonitor", [sourceFile.href, lineNo, true]);
        }
        return bp;
    },

    unmonitor: function(sourceFile, lineNo)
    {
        if (lineNo != -1 && this.removeBreakpoint(BP_MONITOR, sourceFile.href, lineNo))
        {
            --monitorCount;
            dispatch(debuggers, "onToggleMonitor", [sourceFile.href, lineNo, false]);
        }
    },

    isMonitored: function(url, lineNo)
    {
        var bp = lineNo != -1 ? this.findBreakpoint(url, lineNo) : null;
        return bp && bp.type & BP_MONITOR;
    },

    enumerateMonitors: function(url, cb)
    {
        if (url)
        {
            var urlBreakpoints = breakpoints[url];
            if (urlBreakpoints)
            {
                for (var i = 0; i < urlBreakpoints.length; ++i)
                {
                    var bp = urlBreakpoints[i];
                    if (bp.type & BP_MONITOR)
                        cb.call(url, bp.lineNo, bp);
                }
            }
        }
        else
        {
            for (var url in breakpoints)
                this.enumerateBreakpoints(url, cb);
        }
    },

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    enumerateScripts: function(length)
    {
        var scripts = [];
        jsd.enumerateScripts( {
            enumerateScript: function(script) {
                var fileName = script.fileName;
                if ( !isFilteredURL(fileName) ) {
                    scripts.push(script);
                }
            }
        });
        length.value = scripts.length;
        return scripts;
    },

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    startProfiling: function()
    {
        if (!this.profiling)
        {
            this.profiling = true;
            profileStart = new Date();

            jsd.flags |= COLLECT_PROFILE_DATA;
        }

        ++profileCount;
    },

    stopProfiling: function()
    {
        if (--profileCount == 0)
        {
            jsd.flags &= ~COLLECT_PROFILE_DATA;

            var t = profileStart.getTime();

            this.profiling = false;
            profileStart = null;

            return new Date().getTime() - t;
        }
        else
            return -1;
    },

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    enableDebugger: function()
    {
        if (waitingForTimer)
        {
            timer.cancel();
            waitingForTimer = false;
        }
        if (enabledDebugger)
            return;

        enabledDebugger = true;

        this.obeyPrefs();

        if (!jsd)
        {
            jsd = DebuggerService.getService(jsdIDebuggerService);

        }

        if (!jsd.isOn)
        {
            jsd.on(); // this should be the only call to jsd.on().
            jsd.flags |= DISABLE_OBJECT_TRACE;

            if (jsd.pauseDepth && FBTrace.DBG_FBS_ERRORS)
                FBTrace.sysout("fbs.enableDebugger found non-zero jsd.pauseDepth !! "+jsd.pauseDepth)
        }

        if (!this.filterChrome)
            this.createChromeBlockingFilters();

        var active = fbs.isJSDActive();

        dispatch(clients, "onJSDActivate", [active, "fbs enableDebugger"]);
        this.hookScripts();
    },

    obeyPrefs: function()
    {
        this.showStackTrace = prefs.getBoolPref("extensions.firebug.service.showStackTrace");
        this.breakOnErrors = prefs.getBoolPref("extensions.firebug.service.breakOnErrors");
        this.trackThrowCatch = prefs.getBoolPref("extensions.firebug.service.trackThrowCatch");
        this.scriptsFilter = prefs.getCharPref("extensions.firebug.service.scriptsFilter");
        this.filterSystemURLs = prefs.getBoolPref("extensions.firebug.service.filterSystemURLs");  // may not be exposed to users

        FirebugPrefsObserver.syncFilter();

        try {
            // CREATION and BP generate a huge trace
        }
        catch (exc)
        {
            FBTrace.sysout("firebug-service: constructor getBoolPrefs FAILED with exception=",exc);
        }
    },

    disableDebugger: function()
    {
        if (!enabledDebugger)
            return;

        if (!timer)  // then we probably shutdown
            return;

        enabledDebugger = false;

        if (jsd.isOn)
        {
            jsd.pause();
            fbs.unhookScripts();

            while (jsd.pauseDepth > 0)  // unwind completely
                jsd.unPause();
            fbs.pauseDepth = 0;

            jsd.off();
        }

        var active = fbs.isJSDActive();
        dispatch(clients, "onJSDDeactivate", [active, "fbs disableDebugger"]);

        fbs.onXScriptCreatedByTag = {};  // clear any uncleared top level scripts

    },

    pause: function()  // must support multiple calls
    {
        if (!enabledDebugger)
            return "not enabled";
        var rejection = [];
        dispatch(clients, "onPauseJSDRequested", [rejection]);

        if (rejection.length == 0)  // then everyone wants to pause
        {
            if (fbs.pauseDepth == 0)  // don't pause if we are paused.
            {
                fbs.pauseDepth++;
                jsd.pause();
                fbs.unhookScripts();
            }
            var active = fbs.isJSDActive();
            dispatch(clients, "onJSDDeactivate", [active, "pause depth "+jsd.pauseDepth]);
        }
        else // we don't want to pause
        {
            while (fbs.pauseDepth > 0)  // make sure we are not paused.
                fbs.unPause();
            fbs.pauseDepth = 0;
        }
        return fbs.pauseDepth;
    },

    unPause: function(force)
    {
        if (fbs.pauseDepth > 0 || force)
        {
            fbs.pauseDepth--;
            fbs.hookScripts();

            var depth = jsd.unPause();
            var active = fbs.isJSDActive();


            dispatch(clients, "onJSDActivate", [active, "unpause depth"+jsd.pauseDepth]);

        }
        else  // we were not paused.
        {
        }
        return fbs.pauseDepth;
    },

    isJSDActive: function()
    {
        return (jsd && jsd.isOn && (jsd.pauseDepth == 0) );
    },

    broadcast: function(message, args)  // re-transmit the message (string) with args [objs] to XUL windows.
    {
        dispatch(clients, message, args);
    },

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    normalizeURL: function(url)
    {
        // For some reason, JSD reports file URLs like "file:/" instead of "file:///", so they
        // don't match up with the URLs we get back from the DOM
        return url ? url.replace(/file:\/([^/])/, "file:///$1") : "";
    },

    denormalizeURL: function(url)
    {
        // This should not be called.
        return url ? url.replace(/file:\/\/\//, "file:/") : "";
    },


    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
    // jsd Hooks

    // When (debugger keyword and not halt)||(bp and BP_UNTIL) || (onBreakPoint && no conditions)
    // || interuptHook.  rv is ignored
    onBreak: function(frame, type, rv)
    {
        try
        {
            // avoid step_out from web page to chrome
            if (type==jsdIExecutionHook.TYPE_INTERRUPTED && stepStayOnDebuggr)
            {
                var debuggr = this.reFindDebugger(frame, stepStayOnDebuggr);
                if (!debuggr)
                {
                    // This frame is not for the debugger we want
                    if (stepMode == STEP_OVER || stepMode == STEP_OUT)  // then we are in the debuggr we want and returned in to one we don't
                    {
                        this.stopStepping(); // run, you are free.
                    }

                    return RETURN_CONTINUE;  // This means that we will continue to take interrupts until  when?
                }
                else
                {
                    if (stepMode == STEP_SUSPEND) // then we have interrupted the outerFunction
                    {
                        var scriptTag = frame.script.tag;
                        if (scriptTag in this.onXScriptCreatedByTag) // yes, we have to create the sourceFile
                            this.onBreakpoint(frame, type, rv);  // TODO refactor so we don't get mixed up
                    }
                }
            }
            else
            {
                var debuggr = this.findDebugger(frame);

            }

            if (debuggr)
                return this.breakIntoDebugger(debuggr, frame, type);

        }
        catch(exc)
        {
            ERROR("onBreak failed: "+exc);
        }
        return RETURN_CONTINUE;
    },

    // When engine encounters debugger keyword (only)
    onDebugger: function(frame, type, rv)
    {
        try {
            if (haltDebugger)
            {
                var debuggr = haltDebugger;
                haltDebugger = null;
                return debuggr.onHalt(frame);
            }
            else
                return this.onBreak(frame, type, rv);
         }
         catch(exc)
         {
            ERROR("onDebugger failed: "+exc);
            return RETURN_CONTINUE;
         }
    },

    // when the onError handler returns false
    onDebug: function(frame, type, rv)
    {
        if ( isFilteredURL(frame.script.fileName) )
        {
            reportNextError = false;
            return RETURN_CONTINUE;
        }

        try
        {
            var breakOnNextError = this.needToBreakForError(reportNextError);
            var debuggr = (reportNextError || breakOnNextError) ? this.findDebugger(frame) : null;

            if (reportNextError)
            {
                reportNextError = false;
                if (debuggr)
                {
                    var hookReturn = debuggr.onError(frame, errorInfo);
                    if (hookReturn >=0)
                        return hookReturn;
                    else if (hookReturn==-1)
                        breakOnNextError = true;
                    if (breakOnNextError)
                        debuggr = this.reFindDebugger(frame, debuggr);
                }
            }

            if (breakOnNextError)
            {
                breakOnNextError = false;
                if (debuggr)
                    return this.breakIntoDebugger(debuggr, frame, type);
            }
        } catch (exc) {
            ERROR("onDebug failed: "+exc);
        }
        return RETURN_CONTINUE;
    },

    onBreakpoint: function(frame, type, val)
    {
        var scriptTag = frame.script.tag;
        if (scriptTag in this.onXScriptCreatedByTag)
        {
            var onXScriptCreated = this.onXScriptCreatedByTag[scriptTag];
            delete this.onXScriptCreatedByTag[scriptTag];
            frame.script.clearBreakpoint(0);
            try {
                var sourceFile = onXScriptCreated(frame, type, val);
            } catch (e) {
                FBTrace.sysout("onBreakpoint called onXScriptCreated and it didn't end well:",e);
            }

            if (!sourceFile || !sourceFile.breakOnZero || sourceFile.breakOnZero != scriptTag)
                return RETURN_CONTINUE;
            else  // sourceFile.breakOnZero matches the script we have halted.
            {
            }
        }


        var bp = this.findBreakpointByScript(frame.script, frame.pc);
        if (bp)
        {
            // See issue 1179, should not break if we resumed from a single step and have not advanced.
            if (disabledCount || monitorCount || conditionCount || runningUntil)
            {
                if (bp.type & BP_MONITOR && !(bp.disabled & BP_MONITOR))
                {
                    if (bp.type & BP_TRACE && !(bp.disabled & BP_TRACE) )
                        this.hookCalls(bp.debugger.onFunctionCall, true);
                    else
                        bp.debugger.onMonitorScript(frame);
                }

                if (bp.type & BP_UNTIL)
                {
                    this.stopStepping();
                    if (bp.debugger)
                        return this.breakIntoDebugger(bp.debugger, frame, type);
                }
                else if (!(bp.type & BP_NORMAL) || bp.disabled & BP_NORMAL)
                {
                    return  RETURN_CONTINUE;
                }
                else if (bp.type & BP_NORMAL)
                {
                    var passed = testBreakpoint(frame, bp);
                    if (!passed)
                        return RETURN_CONTINUE;
                }
                // type was normal, but passed test
            }
            else  // not special, just break for sure
                return this.breakIntoDebugger(bp.debugger, frame, type);
        }
        else
        {
        }

        if (runningUntil)
            return RETURN_CONTINUE;
        else
            return this.onBreak(frame, type, val);
    },

    onThrow: function(frame, type, rv)
    {
        if ( isFilteredURL(frame.script.fileName) )
            return RETURN_CONTINUE_THROW;

        if (rv && rv.value && rv.value.isValid)
        {
            var value = rv.value;
            if (value.jsClassName == "Error" && reTooMuchRecursion.test(value.stringValue))
            {
                if (fbs._lastErrorCaller)
                {
                    if (fbs._lastErrorCaller == frame.script.tag) // then are unwinding recursion
                    {
                        fbs._lastErrorCaller = frame.callingFrame ? frame.callingFrame.script.tag : null;
                        return RETURN_CONTINUE_THROW;
                    }
                }
                else
                {
                    fbs._lastErrorCaller = frame.callingFrame.script.tag;
                    frame = fbs.discardRecursionFrames(frame);
                    // go on to process the throw.
                }
            }
            else
            {
                delete fbs._lastErrorCaller; // throw is not recursion
            }
        }
        else
        {
            delete fbs._lastErrorCaller; // throw is not recursion either
        }

        // Remember the error where the last exception is thrown - this will
        // be used later when the console service reports the error, since
        // it doesn't currently report the window where the error occurred

        this._lastErrorWindow =  getFrameGlobal(frame);

        if (this.showStackTrace)  // store these in case the throw is not caught
        {
            var debuggr = this.findDebugger(frame);  // sets debuggr.breakContext
            if (debuggr)
            {
                fbs._lastErrorScript = frame.script;
                fbs._lastErrorLine = frame.line;
                fbs._lastErrorDebuggr = debuggr;
                fbs._lastErrorContext = debuggr.breakContext; // XXXjjb this is bad API
            }
            else
                delete fbs._lastErrorDebuggr;
        }

        if (fbs.trackThrowCatch)
        {
            var debuggr = this.findDebugger(frame);
            if (debuggr)
                return debuggr.onThrow(frame, rv);
        }

        return RETURN_CONTINUE_THROW;
    },

    onError: function(message, fileName, lineNo, pos, flags, errnum, exc)
    {
        errorInfo = { errorMessage: message, sourceName: fileName, lineNumber: lineNo,
                message: message, fileName: fileName, lineNo: lineNo,
                columnNumber: pos, flags: flags, category: "js", errnum: errnum, exc: exc };

        if (message=="out of memory")  // bail
        {
            return true;
        }

        reportNextError = { fileName: fileName, lineNo: lineNo };
        return false; // Drop into onDebug, sometimes only
    },

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    onEventScriptCreated: function(frame, type, val, noNestTest)
    {
        if (fbs.showEvents)
        {
            try
            {
               if (!noNestTest)
                {
                    // In onScriptCreated we saw a script with baseLineNumber = 1. We marked it as event and nested.
                    // Now we know its event, not nested.
                    if (fbs.nestedScriptStack.length > 0)
                    {
                        fbs.nestedScriptStack.removeElementAt(0);
                    }
                    else
                    {
                    }
                }

                var debuggr = fbs.findDebugger(frame);  // sets debuggr.breakContext
                if (debuggr)
                {
                    var sourceFile = debuggr.onEventScriptCreated(frame, frame.script, fbs.nestedScriptStack.enumerate());
                    fbs.resetBreakpoints(sourceFile);
                }
                else
                {
                }
            } catch(exc) {
            }
        }

        fbs.clearNestedScripts();
        return sourceFile;
    },

    onEvalScriptCreated: function(frame, type, val)
    {
        if (fbs.showEvals)
        {
            try
            {
                if (!frame.callingFrame)
                {
                    return fbs.onEventScriptCreated(frame, type, val, true);
                }
                // In onScriptCreated we found a no-name script, set a bp in PC=0, and a flag.
                // onBreakpoint saw the flag, cleared the flag, and sent us here.
                // Start by undoing our damage
                var outerScript = frame.script;

                var debuggr = fbs.findDebugger(frame);  // sets debuggr.breakContext
                if (debuggr)
                {
                    var sourceFile = debuggr.onEvalScriptCreated(frame, outerScript, fbs.nestedScriptStack.enumerate());
                    fbs.resetBreakpoints(sourceFile);
                }
                else
                {
                }
            }
            catch (exc)
            {
                ERROR("onEvalScriptCreated failed: "+exc);
            }
        }

        fbs.clearNestedScripts();
        return sourceFile;
    },

    onTopLevelScriptCreated: function(frame, type, val)
    {
        try
        {
            // In onScriptCreated we may have found a script at baseLineNumber=1
            // Now we know its not an event
            if (fbs.nestedScriptStack.length > 0)
            {
                var firstScript = fbs.nestedScriptStack.queryElementAt(0, jsdIScript);
                if (firstScript.tag in fbs.onXScriptCreatedByTag)
                {
                    delete  fbs.onXScriptCreatedByTag[firstScript.tag];
                    if (firstScript.isValid)
                        firstScript.clearBreakpoint(0);
                }
            }

            // On compilation of a top-level (global-appending) function.
            // After this top-level script executes we lose the jsdIScript so we can't build its line table.
            // Therefore we need to build it here.
            var debuggr = fbs.findDebugger(frame);  // sets debuggr.breakContext
            if (debuggr)
            {
                var sourceFile = debuggr.onTopLevelScriptCreated(frame, frame.script, fbs.nestedScriptStack.enumerate());
                fbs.resetBreakpoints(sourceFile, frame.script.baseLineNumber+frame.script.lineExtent);
            }
            else
            {
                // modules end up here?
            }
        }
        catch (exc)
        {
            FBTrace.sysout("onTopLevelScriptCreated FAILED: ", exc);
            ERROR("onTopLevelScriptCreated Fails: "+exc);
        }

        fbs.clearNestedScripts();
        return sourceFile;
    },

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    clearNestedScripts: function()
    {
        var innerScripts = fbs.nestedScriptStack.enumerate();
        while (innerScripts.hasMoreElements())
        {
            var script = innerScripts.getNext();
            if (script.isValid && script.baseLineNumber == 1)
            {
                script.clearBreakpoint(0);
                if (this.onXScriptCreatedByTag[script.tag])
                    delete this.onXScriptCreatedByTag[script.tag];
            }
        }
        fbs.nestedScriptStack.clear();
    },

    onScriptCreated: function(script)
    {
        if (!fbs)
        {
             return;
        }

        try
        {
            var fileName = script.fileName;
            if (isFilteredURL(fileName))
            {
                try {
                } catch (exc) { /*Bug 426692 */ }
                return;
            }

            // reset tracing flags on first unfiltered filename
            if (!FBTrace.DBG_FF_START && !fbs.firstUnfilteredFilename)
            {
                fbs.firstUnfilteredFilename = true;
                FBTrace.DBG_FBS_BP = fbs.resetBP ? true : false;
                FBTrace.DBG_FBS_CREATION = fbs.resetCreation ? true : false;
            }

            if (fbs.pendingXULFileName && fbs.pendingXULFileName != script.fileName)
                fbs.flushXUL();

            if (!script.functionName) // top or eval-level
            {
                // We need to detect eval() and grab its source.
                var hasCaller = fbs.createdScriptHasCaller();
                if (hasCaller)
                {
                    // components end up here
                    fbs.onXScriptCreatedByTag[script.tag] = this.onEvalScriptCreated;
                }
                else
                    fbs.onXScriptCreatedByTag[script.tag] = this.onTopLevelScriptCreated;

                script.setBreakpoint(0);
            }
            else if (script.baseLineNumber == 1)
            {
                // could be a 1) Browser-generated event handler or 2) a nested script at the top of a file
                // One way to tell is assume both then wait to see which we hit first:
                // 1) bp at pc=0 for this script or 2) for a top-level on at the same filename

                fbs.onXScriptCreatedByTag[script.tag] = this.onEventScriptCreated; // for case 1
                script.setBreakpoint(0);

                fbs.nestedScriptStack.appendElement(script, false);  // for case 2

            }
            else if( reXUL.test(script.fileName) )
            {
                fbs.pendingXULFileName = script.fileName;  // if these were different, we would already have called flushXUL()
                fbs.nestedScriptStack.appendElement(script, false);
            }
            else
            {
                fbs.nestedScriptStack.appendElement(script, false);
                dispatch(scriptListeners,"onScriptCreated",[script, fileName, script.baseLineNumber]);
            }
        }
        catch(exc)
        {
            ERROR("onScriptCreated failed: "+exc);
            FBTrace.sysout("onScriptCreated failed: ", exc);
        }
    },

    flushXUL: function()
    {
        for ( var i = debuggers.length - 1; i >= 0; i--)
        {
            try
            {
                var debuggr = debuggers[i];
                if (debuggr.onXULScriptCreated)
                    debuggr.onXULScriptCreated(fbs.pendingXULFileName, fbs.nestedScriptStack.enumerate());
            }
            catch (exc)
            {
                FBTrace.sysout("firebug-service flushXUL FAILS: ",exc);
            }
        }
        delete fbs.pendingXULFileName;
        fbs.clearNestedScripts();
    },

    createdScriptHasCaller: function()
    {
        var frame = Components.stack; // createdScriptHasCaller

        frame = frame.caller;         // onScriptCreated
        if (!frame) return frame;

        frame = frame.caller;         // hook apply
        if (!frame) return frame;
        frame = frame.caller;         // native interpret?
        if (!frame) return frame;
        frame = frame.caller;         // our creator ... or null if we are top level
        return frame;
    },

    onScriptDestroyed: function(script)
    {
        if (!fbs)
             return;
        if (script.tag in fbs.onXScriptCreatedByTag)
            delete  fbs.onXScriptCreatedByTag[script.tag];

        try
        {
            var fileName = script.fileName;
            if (isFilteredURL(fileName))
                return;
            dispatch(scriptListeners,"onScriptDestroyed",[script]);
        }
        catch(exc)
        {
            ERROR("onScriptDestroyed failed: "+exc);
            FBTrace.sysout("onScriptDestroyed failed: ", exc);
        }
    },

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    createFilter: function(pattern, pass)
    {
        var filter = {
                globalObject: null,
                flags: pass ? (jsdIFilter.FLAG_ENABLED | jsdIFilter.FLAG_PASS) : jsdIFilter.FLAG_ENABLED,
                urlPattern: pattern,
                startLine: 0,
                endLine: 0
            };
        return filter;
    },

    setChromeBlockingFilters: function()
    {
        if (!fbs.isChromeBlocked)
        {
            jsd.appendFilter(this.noFilterHalter);  // must be first
            jsd.appendFilter(this.filterChrome);
            jsd.appendFilter(this.filterComponents);
            jsd.appendFilter(this.filterFirebugComponents);
            jsd.appendFilter(this.filterModules);
            jsd.appendFilter(this.filterStringBundle);
            jsd.appendFilter(this.filterPrettyPrint);
            jsd.appendFilter(this.filterWrapper);

            for (var i = 0; i < this.componentFilters.length; i++)
                jsd.appendFilter(this.componentFilters[i]);

            fbs.isChromeBlocked = true;

        }
    },

    removeChromeBlockingFilters: function()
    {
        if (fbs.isChromeBlocked)
        {
            jsd.removeFilter(this.filterChrome);
            jsd.removeFilter(this.filterComponents);
            jsd.removeFilter(this.filterFirebugComponents);
            jsd.removeFilter(this.filterModules);
            jsd.removeFilter(this.filterStringBundle);
            jsd.removeFilter(this.filterPrettyPrint);
            jsd.removeFilter(this.filterWrapper);
            jsd.removeFilter(this.noFilterHalter);
            for (var i = 0; i < this.componentFilters.length; i++)
                jsd.removeFilter(this.componentFilters[i]);

            fbs.isChromeBlocked = false;
        }
    },

    createChromeBlockingFilters: function() // call after components are loaded.
    {
        try
        {
            this.filterModules = this.createFilter("*/firefox/modules/*");
            this.filterComponents = this.createFilter("*/firefox/components/*");
            this.filterFirebugComponents = this.createFilter("*/components/firebug-*");
            this.filterStringBundle = this.createFilter("XStringBundle");
            this.filterChrome = this.createFilter("chrome://*");
            this.filterPrettyPrint = this.createFilter("x-jsd:ppbuffer*");
            this.filterWrapper = this.createFilter("XPCSafeJSObjectWrapper.cpp");
            this.noFilterHalter = this.createFilter("chrome://firebug/content/debuggerHalter.js", true);

            // jsdIFilter does not allow full regexp matching.
            // So to filter components, we filter their directory names, which we obtain by looking for
            // scripts that match regexps

            var componentsUnfound = [];
            for( var i = 0; i < COMPONENTS_FILTERS.length; ++i )
            {
                componentsUnfound.push(COMPONENTS_FILTERS[i]);
            }

            this.componentFilters = [];

            jsd.enumerateScripts( {
                enumerateScript: function(script) {
                    var fileName = script.fileName;
                for( var i = 0; i < componentsUnfound.length; ++i )
                    {
                        if ( componentsUnfound[i].test(fileName) )
                        {
                            var match = componentsUnfound[i].exec(fileName);
                            fbs.componentFilters.push(fbs.createFilter(match[1]));
                            componentsUnfound.splice(i, 1);
                            return;
                        }
                    }
                }
            });
        } catch (exc) {
            FBTrace.sysout("createChromeblockingFilters fails >>>>>>>>>>>>>>>>> "+exc, exc);
        }

    },

    traceFilters: function(from)
    {
        FBTrace.sysout("fbs.traceFilters from "+from);
        jsd.enumerateFilters({ enumerateFilter: function(filter)
            {
                FBTrace.sysout("jsdIFilter "+filter.urlPattern, filter);
            }});
    },
    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    getJSContexts: function()
    {
        var enumeratedContexts = [];
        jsd.enumerateContexts( {enumerateContext: function(jscontext)
        {
                try
                {
                    if (!jscontext.isValid)
                        return;

                    var wrappedGlobal = jscontext.globalObject;
                    if (!wrappedGlobal)
                        return;

                    var unwrappedGlobal = wrappedGlobal.getWrappedValue();
                    if (!unwrappedGlobal)
                        return;

                    var global = new XPCNativeWrapper(unwrappedGlobal);

                    if (global)
                    {
                        var document = global.document;
                        if (document)
                        {
                        }
                        else
                        {
                            return; // skip these
                        }
                    }
                    else
                    {
                        return; // skip this
                    }

                    enumeratedContexts.push(jscontext);
                }
                catch(e)
                {
                    FBTrace.sysout("jscontext dump FAILED "+e, e);
                }

        }});
        return enumeratedContexts;
    },

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    findDebugger: function(frame)
    {
        if (debuggers.length < 1)
            return;

        var checkFrame = frame;
        while (checkFrame) // We may stop in a component, but want the callers Window
        {
            var frameScopeRoot = getFrameScopeRoot(checkFrame);
            if (frameScopeRoot)
                break;

            checkFrame = checkFrame.callingFrame;
        }

        if (!checkFrame && FBTrace.DBG_FBS_FINDDEBUGGER)
            FBTrace.sysout("fbs.findDebugger fell thru bottom of stack", frame);

        // frameScopeRoot should be the top window for the scope of the frame function
        // or null
        fbs.last_debuggr = fbs.askDebuggersForSupport(frameScopeRoot, frame);
        if (fbs.last_debuggr)
             return fbs.last_debuggr;
        else
            return null;
    },

    isChromebug: function(global)
    {
        // TODO this is a kludge: isFilteredURL stops users from seeing firebug but chromebug has to disable the filter

        var location = fbs.getLocationSafe(global);
        if (location)
        {
            if (location.indexOf("chrome://chromebug/") >= 0 || location.indexOf("chrome://fb4cb/") >= 0)
                return true;
        }
        return false;
    },

    getLocationSafe: function(global)
    {
        try
        {
            if (global && global.location)  // then we have a window, it will be an nsIDOMWindow, right?
                return global.location.toString();
            else if (global && global.tag)
                return "global_tag_"+global.tag;
        }
        catch (exc)
        {
            // FF3 gives (NS_ERROR_INVALID_POINTER) [nsIDOMLocation.toString]
        }
        return null;
    },

    askDebuggersForSupport: function(global, frame)
    {
        if (global && fbs.isChromebug(global))
            return false;

        for ( var i = debuggers.length - 1; i >= 0; i--)
        {
            try
            {
                var debuggr = debuggers[i];
                if (debuggr.supportsGlobal(global, frame))
                {
                    if (!debuggr.breakContext)
                        FBTrace.sysout("Debugger with no breakContext:",debuggr.supportsGlobal);
                    return debuggr;
                }
            }
            catch (exc)
            {
                FBTrace.sysout("firebug-service askDebuggersForSupport FAILS: "+exc,exc);
            }
        }
        return null;
    },

    dumpIValue: function(value)
    {
        var listValue = {value: null}, lengthValue = {value: 0};
        value.getProperties(listValue, lengthValue);
        for (var i = 0; i < lengthValue.value; ++i)
        {
            var prop = listValue.value[i];
            try {
                var name = unwrapIValue(prop.name);
                FBTrace.sysout(i+"]"+name+"="+unwrapIValue(prop.value));
            }
            catch (e)
            {
                FBTrace.sysout(i+"]"+e);
            }
        }
    },

    reFindDebugger: function(frame, debuggr)
    {
        var frameScopeRoot = getFrameScopeRoot(frame);
        if (frameScopeRoot && debuggr.supportsGlobal(frameScopeRoot, frame)) return debuggr;

        return null;
    },

    discardRecursionFrames: function(frame)
    {
        var i = 0;
        var rest = 0;
        var mark = frame;  // a in abcabcabcdef
        var point = frame;
        while (point = point.callingFrame)
        {
            i++;
            if (point.script.tag == mark.script.tag) // then we found a repeating caller abcabcdef
            {
                mark = point;
                rest = i;
            }
        }
        // here point is null and mark is the last repeater, abcdef
        return mark;
    },

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
    // jsd breakpoints are on a PC in a jsdIScript
    // Users breakpoint on a line of source
    // Because test.js can be included multiple times, the URL+line number from the UI is not unique.
    // sourcefile.href != script.fileName, generally script.fileName cannot be used.
    // If the source is compiled, then we have zero, one, or more jsdIScripts on a line.
    //    If zero, cannot break at that line
    //    If one, set a jsd breakpoint
    //    If more than one, set jsd breakpoint on each script
    // Else we know that the source will be compiled before it is run.
    //    Save the sourceFile.href+line and set the jsd breakpoint when we compile
    //    Venkman called these "future" breakpoints
    //    We cannot prevent future breakpoints on lines that have no script.  Break onCreate with error?

    addBreakpoint: function(type, sourceFile, lineNo, props, debuggr)
    {
        var url = sourceFile.href;
        var bp = this.findBreakpoint(url, lineNo);
        if (bp && bp.type & type)
            return null;

        if (bp)
        {
            bp.type |= type;

            if (debuggr)
                bp.debugger = debuggr;
            else
            {
            }
        }
        else
        {
            bp = this.recordBreakpoint(type, url, lineNo, debuggr, props);
            fbs.setJSDBreakpoint(sourceFile, bp);
        }
        return bp;
    },

    recordBreakpoint: function(type, url, lineNo, debuggr, props)
    {
        var urlBreakpoints = breakpoints[url];
        if (!urlBreakpoints)
            breakpoints[url] = urlBreakpoints = [];

        var bp = {type: type, href: url, lineNo: lineNo, disabled: 0,
            debugger: debuggr,
            condition: "", onTrue: true, hitCount: -1, hit: 0};
        if (props)
        {
            bp.condition = props.condition;
            bp.onTrue = props.onTrue;
            bp.hitCount = props.hitCount;
            if (bp.condition || bp.hitCount > 0)
                ++conditionCount;
            if(props.disabled)
            {
                bp.disabled |= BP_NORMAL;
                ++disabledCount;
            }
        }
        urlBreakpoints.push(bp);
        ++breakpointCount;
        return bp;
    },

    removeBreakpoint: function(type, url, lineNo)
    {
        var urlBreakpoints = breakpoints[url];
        if (!urlBreakpoints)
            return false;

        for (var i = 0; i < urlBreakpoints.length; ++i)
        {
            var bp = urlBreakpoints[i];
            if (bp.lineNo == lineNo)
            {
                bp.type &= ~type;
                if (!bp.type)
                {
                    if (bp.scriptsWithBreakpoint)
                    {
                        for (var j = 0; j < bp.scriptsWithBreakpoint.length; j++)
                        {
                            var script = bp.scriptsWithBreakpoint[j];
                            if (script && script.isValid)
                            {
                                try
                                {
                                    script.clearBreakpoint(bp.pc[j]);
                                }
                                catch (exc)
                                {
                                    FBTrace.sysout("Firebug service failed to remove breakpoint in "+script.tag+" at lineNo="+lineNo+" pcmap:"+bp.pcmap);
                                }
                            }
                        }
                    }
                    // else this was a future breakpoint that never hit or a script that was GCed

                    urlBreakpoints.splice(i, 1);
                    --breakpointCount;

                    if (bp.disabled)
                        --disabledCount;

                    if (bp.condition || bp.hitCount > 0)
                    {
                        --conditionCount;
                    }


                    if (!urlBreakpoints.length)
                        delete breakpoints[url];

                }
                return bp;
            }
        }

        return false;
    },

    findBreakpoint: function(url, lineNo)
    {
        var urlBreakpoints = breakpoints[url];
        if (urlBreakpoints)
        {
            for (var i = 0; i < urlBreakpoints.length; ++i)
            {
                var bp = urlBreakpoints[i];
                if (bp.lineNo == lineNo)
                    return bp;
            }
        }
        return null;
    },

    // When we are called, scripts have been compiled so all relevant breakpoints are not "future"
    findBreakpointByScript: function(script, pc)
    {
        for (var url in breakpoints)
        {
            var urlBreakpoints = breakpoints[url];
            if (urlBreakpoints)
            {
                for (var i = 0; i < urlBreakpoints.length; ++i)
                {
                    var bp = urlBreakpoints[i];
                    if (bp.scriptsWithBreakpoint)
                    {
                        for (var j = 0; j < bp.scriptsWithBreakpoint.length; j++)
                        {
                            if ( bp.scriptsWithBreakpoint[j] && (bp.scriptsWithBreakpoint[j].tag == script.tag) && (bp.pc[j] == pc) )
                                return bp;
                        }
                    }
                }
            }
        }

        return null;
    },

    resetBreakpoints: function(sourceFile, lastLineNumber) // the sourcefile has just been created after compile
    {
        // If the new script is replacing an old script with a breakpoint still
        var url = sourceFile.href;
        var urlBreakpoints = breakpoints[url];
        if (urlBreakpoints)
        {
            for (var i = 0; i < urlBreakpoints.length; ++i)
            {
                var bp = urlBreakpoints[i];
                fbs.setJSDBreakpoint(sourceFile, bp);
                if (lastLineNumber && !bp.jsdLine && !(bp.disabled & BP_NORMAL) && (bp.lineNo < lastLineNumber))
                {
                     fbs.disableBreakpoint(url, bp.lineNo);
                }
            }
        }
        else
        {
        }
    },

    setJSDBreakpoint: function(sourceFile, bp)
    {
        var scripts = sourceFile.getScriptsAtLineNumber(bp.lineNo);
        if (!scripts)
        {
             if (!sourceFile.outerScript || !sourceFile.outerScript.isValid)
             {
                return;
             }
             scripts = [sourceFile.outerScript];
        }

        bp.scriptsWithBreakpoint = [];
        bp.pc = [];
        for (var i = 0; i < scripts.length; i++)
        {
            var script = scripts[i];
            if (!script.isValid)
            {
                continue;
            }

            var pcmap = sourceFile.pcmap_type;
            if (!pcmap)
            {
                pcmap = PCMAP_SOURCETEXT;
            }
            // we subtraced this offset when we showed the user so lineNo is a user line number; now we need to talk
            // to jsd its line world
            var jsdLine = bp.lineNo + sourceFile.getBaseLineOffset();
            // test script.isLineExecutable(jsdLineNo, pcmap) ??

            var isExecutable = false;
            try {
                 isExecutable = script.isLineExecutable(jsdLine, pcmap);
            } catch(e) {
                // guess not then...
            }
            if (isExecutable)
            {
                var pc = script.lineToPc(jsdLine, pcmap);
                var pcToLine = script.pcToLine(pc, pcmap);  // avoid calling this unless we have to...

                if (pcToLine == jsdLine)
                {
                    script.setBreakpoint(pc);

                    bp.scriptsWithBreakpoint.push(script);
                    bp.pc.push(pc);
                    bp.pcmap = pcmap;
                    bp.jsdLine = jsdLine;

                    if (pc == 0)  // signal the breakpoint handler to break for user
                        sourceFile.breakOnZero = script.tag;

                }
                else
                {
                }
            }
            else
            {
            }
         }
    },
    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    breakIntoDebugger: function(debuggr, frame, type)
    {
        // Before we break, clear information about previous stepping session
        this.stopStepping();

        // Break into the debugger - execution will stop here until the user resumes
        var returned;
        try
        {
            returned = debuggr.onBreak(frame, type);
        }
        catch (exc)
        {
            ERROR(exc);
            returned = RETURN_CONTINUE;
        }

        // Execution resumes now. Check if the user requested stepping and if so
        // install the necessary hooks
        hookFrameCount = countFrames(frame);
        this.startStepping();
        return returned;
    },

    needToBreakForError: function(reportNextError)
    {
        return this.breakOnErrors || this.findErrorBreakpoint(this.normalizeURL(reportNextError.fileName), reportNextError.lineNo) != -1;
    },

    startStepping: function()
    {
        if (!stepMode && !runningUntil)
            return;

        this.hookFunctions();

        if (stepMode == STEP_OVER || stepMode == STEP_INTO)
            this.hookInterrupts();
    },

    stopStepping: function()
    {
        stepMode = 0;
        stepFrame = null;
        stepFrameCount = 0;
        stepFrameLineId = null;

        if (runningUntil)
        {
            this.removeBreakpoint(BP_UNTIL, runningUntil.href, runningUntil.lineNo);
            runningUntil = null;
        }

        jsd.interruptHook = null;
        jsd.functionHook = null;
    },

    /*
     * Returns a string describing the step mode or null for not stepping.
     */
    getStepMode: function()
    {
        return getStepName(stepMode);
    },

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    hookFunctions: function()
    {
        function functionHook(frame, type)
        {
            switch (type)
            {
                case TYPE_TOPLEVEL_START: // fall through
                case TYPE_FUNCTION_CALL:
                {
                    ++hookFrameCount;

                    if (stepMode == STEP_OVER)
                        jsd.interruptHook = null;

                    if (stepMode == STEP_INTO)  // normally step into will break in the interrupt handler, but not in event handlers.
                    {
                        fbs.stopStepping();
                        stepMode = STEP_SUSPEND; // break on next
                        fbs.hookInterrupts();
                    }

                    break;
                }
                case TYPE_TOPLEVEL_END: // fall through
                case TYPE_FUNCTION_RETURN:
                {
                    --hookFrameCount;

                    if (hookFrameCount == 0) {  // stack empty
                        if ( (stepMode == STEP_INTO) || (stepMode == STEP_OVER) ) {
                            fbs.stopStepping();
                            stepMode = STEP_SUSPEND; // break on next
                            fbs.hookInterrupts();
                        }
                        else
                        {
                            fbs.stopStepping();
                        }
                    }
                    else if (stepMode == STEP_OVER)
                    {
                        if (hookFrameCount <= stepFrameCount)
                            fbs.hookInterrupts();
                    }
                    else if (stepMode == STEP_OUT)
                    {
                        if (hookFrameCount < stepFrameCount)
                            fbs.hookInterrupts();
                    }

                    break;
                }
            }
        }

        jsd.functionHook = { onCall: functionHook };
    },

    hookInterrupts: function()
    {
        function interruptHook(frame, type, rv)
        {
            /*if ( isFilteredURL(frame.script.fileName) )  // it does not seem feasible to use jsdIFilter-ing TODO try again
            {
                return RETURN_CONTINUE;
            }
             */
            // Sometimes the same line will have multiple interrupts, so check
            // a unique id for the line and don't break until it changes
            var frameLineId = hookFrameCount + frame.script.fileName + frame.line;
            if (frameLineId != stepFrameLineId)
                return fbs.onBreak(frame, type, rv);
            else
                return RETURN_CONTINUE;
        }

        jsd.interruptHook = { onExecute: interruptHook };
    },

    hookScripts: function()
    {
        jsd.scriptHook = {
            onScriptCreated: hook(this.onScriptCreated),
            onScriptDestroyed: hook(this.onScriptDestroyed)
        };
        if (fbs.filterSystemURLs)
            fbs.setChromeBlockingFilters();

        jsd.debuggerHook = { onExecute: hook(this.onDebugger, RETURN_CONTINUE) };
        jsd.debugHook = { onExecute: hook(this.onDebug, RETURN_CONTINUE) };
        jsd.breakpointHook = { onExecute: hook(this.onBreakpoint, RETURN_CONTINUE) };
        jsd.throwHook = { onExecute: hook(this.onThrow, RETURN_CONTINUE_THROW) };
        jsd.errorHook = { onError: hook(this.onError, true) };
    },

    unhookScripts: function()
    {
        jsd.scriptHook = null;
        fbs.removeChromeBlockingFilters();

    },

    hookCalls: function(callBack, unhookAtBottom)
    {
        var contextCached = null;

        function callHook(frame, type)
        {
            switch (type)
            {
                case TYPE_FUNCTION_CALL:
                {
                    ++hookFrameCount;

                    contextCached = callBack(contextCached, frame, hookFrameCount, true);

                    break;
                }
                case TYPE_FUNCTION_RETURN:
                {
                    if(hookFrameCount <= 0)  // ignore returns until we have started back in
                        return;

                    --hookFrameCount;
                    if (unhookAtBottom && hookFrameCount == 0) {  // stack empty
                       jsd.functionHook = null;
                    }

                    contextCached = callBack(contextCached, frame, hookFrameCount, false);

                    break;
                }
            }
        }

        if (jsd.functionHook)
        {
            return;
        }

        hookFrameCount = 0;
        jsd.functionHook = { onCall: callHook };
    },

    getJSD: function()
    {
        return jsd; // for debugging fbs
    },

    dumpFileTrack: function(moreFiles)
    {
        if (moreFiles)
            trackFiles.merge(moreFiles);
        trackFiles.dump();
    },

};

function getStepName(mode)
{
    if (mode==STEP_OVER) return "STEP_OVER";
    if (mode==STEP_INTO) return "STEP_INTO";
    if (mode==STEP_OUT) return "STEP_OUT";
    if (mode==STEP_SUSPEND) return "STEP_SUSPEND";
    else return "(not a step mode)";
}

// ************************************************************************************************

var FirebugFactory =
{
    createInstance: function (outer, iid)
    {
        if (outer != null)
            throw NS_ERROR_NO_AGGREGATION;

        FirebugFactory.initializeService();
        return (new FirebugService()).QueryInterface(iid);
    },
    initializeService: function()
    {
        if (!prefs)
           prefs = PrefService.getService(nsIPrefBranch2);

        var filterSystemURLs =  prefs.getBoolPref("extensions.firebug.service.filterSystemURLs");
        if (filterSystemURLs)  // do not turn jsd on unless we want to see chrome
            return;

        try
        {
            var jsd = DebuggerService.getService(jsdIDebuggerService);
            jsd.initAtStartup = false;
        }
        catch (exc)
        {
        }
    }
};

// ************************************************************************************************

var FirebugModule =
{
    registerSelf: function (compMgr, fileSpec, location, type)
    {
        compMgr = compMgr.QueryInterface(nsIComponentRegistrar);
        compMgr.registerFactoryLocation(CLASS_ID, CLASS_NAME, CONTRACT_ID, fileSpec, location, type);
    },

    unregisterSelf: function(compMgr, fileSpec, location)
    {
        compMgr = compMgr.QueryInterface(nsIComponentRegistrar);
        compMgr.unregisterFactoryLocation(CLASS_ID, location);
    },

    getClassObject: function (compMgr, cid, iid)
    {
        if (!iid.equals(nsIFactory))
            throw NS_ERROR_NOT_IMPLEMENTED;

        if (cid.equals(CLASS_ID))
            return FirebugFactory;

        throw NS_ERROR_NO_INTERFACE;
    },

    canUnload: function(compMgr)
    {
        return true;
    }
};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

function NSGetModule(compMgr, fileSpec)
{
    return FirebugModule;
}

// ************************************************************************************************
// Local Helpers

// called by enumerateScripts, onThrow, onDebug, onScriptCreated/Destroyed.
function isFilteredURL(rawJSD_script_filename)
{
    if (!rawJSD_script_filename)
        return true;
    if (fbs.filterConsoleInjections)
        return true;
    if (rawJSD_script_filename[0] == 'h')
        return false;
    if (rawJSD_script_filename == "XPCSafeJSObjectWrapper.cpp")
        return true;
    if (fbs.filterSystemURLs)
        return systemURLStem(rawJSD_script_filename);
    for (var i = 0; i < fbs.alwayFilterURLsStarting.length; i++)
        if (rawJSD_script_filename.indexOf(fbs.alwayFilterURLsStarting[i]) != -1)
            return true;
    return false;
}

function systemURLStem(rawJSD_script_filename)
{
    if (this.url_class)  // attempt to optimize stream of similar urls
    {
        if ( rawJSD_script_filename.substr(0,this.url_class.length) == this.url_class )
            return this.url_class;
    }
    this.url_class = deepSystemURLStem(rawJSD_script_filename);
    return this.url_class;
}

function deepSystemURLStem(rawJSD_script_filename)
{
    for( var i = 0; i < urlFilters.length; ++i )
    {
        var filter = urlFilters[i];
        if ( rawJSD_script_filename.substr(0,filter.length) == filter )
            return filter;
    }
    for( var i = 0; i < COMPONENTS_FILTERS.length; ++i )
    {
        if ( COMPONENTS_FILTERS[i].test(rawJSD_script_filename) )
        {
            var match = COMPONENTS_FILTERS[i].exec(rawJSD_script_filename);
            urlFilters.push(match[1]);  // cache this for future calls
            return match[1];
        }
    }
    return false;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

function dispatch(listeners, name, args)
{
    var totalListeners = listeners.length;
    for (var i = 0; i < totalListeners; ++i)
    {
        var listener = listeners[i];
        if ( listener.hasOwnProperty(name) )
            listener[name].apply(listener, args);
    }
    //if (FBTrace.DBG_FBS_ERRORS)
    //    FBTrace.sysout("fbs.dispatch "+name+" to "+listeners.length+" listeners");
}

function hook(fn, rv)
{
    return function()
    {
        try
        {
            return fn.apply(fbs, arguments);
        }
        catch (exc)
        {
            var msg =  "Error in hook: "+ exc +" fn=\n"+fn+"\n stack=\n";
            for (var frame = Components.stack; frame; frame = frame.caller)
                msg += frame.filename + "@" + frame.line + ";\n";
               ERROR(msg);
            return rv;
        }
    }
}
var lastWindowScope = null;
function getFrameScopeRoot(frame)  // walk script scope chain to bottom, convert to Window if possible
{
    var scope = frame.scope;
    if (scope)
    {
        while(scope.jsParent)
            scope = scope.jsParent;

        // These are just determined by trial and error.
        if (scope.jsClassName == "Window" || scope.jsClassName == "ChromeWindow" || scope.jsClassName == "ModalContentWindow")
        {
            lastWindowScope = new XPCNativeWrapper(scope.getWrappedValue());
            return  lastWindowScope;
        }

        if (scope.jsClassName == "DedicatedWorkerGlobalScope")
        {
            //var workerScope = new XPCNativeWrapper(scope.getWrappedValue());

            //if (FBTrace.DBG_FBS_FINDDEBUGGER)
            //        FBTrace.sysout("fbs.getFrameScopeRoot found WorkerGlobalScope: "+scope.jsClassName, workerScope);
            // https://bugzilla.mozilla.org/show_bug.cgi?id=507930 if (FBTrace.DBG_FBS_FINDDEBUGGER)
            //        FBTrace.sysout("fbs.getFrameScopeRoot found WorkerGlobalScope.location: "+workerScope.location, workerScope.location);
            return null; // https://bugzilla.mozilla.org/show_bug.cgi?id=507783
        }

        if (scope.jsClassName == "Sandbox")
        {
            var proto = scope.jsPrototype;
            if (proto.jsClassName == "XPCNativeWrapper")
                proto = proto.jsParent;
            if (proto.jsClassName == "Window")
                return new XPCNativeWrapper(proto.getWrappedValue());
        }

        try
        {
             return new XPCNativeWrapper(scope.getWrappedValue());
        }
        catch(exc)
        {
        }
    }
    else
        return null;
}

function getFrameGlobal(frame)
{
    var jscontext = frame.executionContext;
    if (!jscontext)
    {
        return getFrameWindow(frame);
    }
    try
    {
        var frameGlobal = new XPCNativeWrapper(jscontext.globalObject.getWrappedValue());
    }
    catch(exc)
    {
    }
    if (frameGlobal)
            return frameGlobal;
    else
    {
        return getFrameWindow(frame);
    }
}

function getFrameWindow(frame)
{
    if (debuggers.length < 1)  // too early, frame.eval will crash FF2
            return;
    try
    {
        var result = {};
        frame.eval("window", "", 1, result);
        var win = unwrapIValue(result.value);
        if (win instanceof Ci.nsIDOMWindow)
            return getRootWindow(win);
        else
            return getFrameScopeRoot(frame);
    }
    catch (exc)
    {
        return null;
    }
}

function getRootWindow(win)
{
    for (; win; win = win.parent)
    {
        if (!win.parent || win == win.parent || !(win.parent instanceof Window) )
            return win;
    }
    return null;
}

function countFrames(frame)
{
    var frameCount = 0;
    try
    {
        for (; frame; frame = frame.callingFrame)
            ++frameCount;
    }
    catch(exc)
    {

    }

    return frameCount;
}

function testBreakpoint(frame, bp)
{
    if ( bp.condition && bp.condition != "" )
    {
        var result = {};
        frame.scope.refresh();
        if (frame.eval(bp.condition, "", 1, result))
        {
            if (bp.onTrue)
            {
                if (!result.value.booleanValue)
                    return false;
            } else
            {
                var value = unwrapIValue(result.value);
                if (typeof bp.lastValue == "undefined")
                {
                    bp.lastValue = value;
                    return false;
                } else
                {
                    if (bp.lastValue == value)
                        return false;
                    bp.lastValue = value;
                }
            }
        }
    }
    ++bp.hit;
    if ( bp.hitCount > 0 )
    {
        if ( bp.hit < bp.hitCount )
            return false;
    }
    return true;
}

function remove(list, item)
{
    var index = list.indexOf(item);
    if (index != -1)
        list.splice(index, 1);
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

var FirebugPrefsObserver =
{
    syncFilter: function()
    {
        var filter = fbs.scriptsFilter;
        fbs.showEvents = (filter == "all" || filter == "events");
        fbs.showEvals = (filter == "all" || filter == "evals");
    }
};

var QuitApplicationGrantedObserver =
{
    observe: function(subject, topic, data)
    {
    }
};
var QuitApplicationRequestedObserver =
{
    observe: function(subject, topic, data)
    {
    }
};
var QuitApplicationObserver =
{
    observe: function(subject, topic, data)
    {
        fbs.disableDebugger();
        fbs.shutdown();
        fbs = null;
    }
};

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
function unwrapIValue(object)
{
    var unwrapped = object.getWrappedValue();
    try
    {
        if (unwrapped)
            return XPCSafeJSObjectWrapper(unwrapped);
    }
    catch (exc)
    {
    }
}

//* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

var consoleService = null;

function ERROR(text)
{
    FBTrace.sysout(text);

    if (!consoleService)
        consoleService = ConsoleService.getService(nsIConsoleService);

    consoleService.logStringMessage(text + "");
}

function getExecutionStopNameFromType(type)
{
    switch (type)
    {
        case jsdIExecutionHook.TYPE_INTERRUPTED: return "interrupted";
        case jsdIExecutionHook.TYPE_BREAKPOINT: return "breakpoint";
        case jsdIExecutionHook.TYPE_DEBUG_REQUESTED: return "debug requested";
        case jsdIExecutionHook.TYPE_DEBUGGER_KEYWORD: return "debugger_keyword";
        case jsdIExecutionHook.TYPE_THROW: return "interrupted";
        default: return "unknown("+type+")";
    }
}
// For special chromebug tracing
function getTmpFile()
{
    var file = Components.classes["@mozilla.org/file/directory_service;1"].
        getService(Components.interfaces.nsIProperties).
        get("TmpD", Components.interfaces.nsIFile);
    file.append("fbs.tmp");
    file.createUnique(Components.interfaces.nsIFile.NORMAL_FILE_TYPE, 0666);
    FBTrace.sysout("FBS opened tmp file "+file.path);
    return file;
}

function getTmpStream(file)
{
    // file is nsIFile, data is a string
    var foStream = Components.classes["@mozilla.org/network/file-output-stream;1"].
                             createInstance(Components.interfaces.nsIFileOutputStream);

    // use 0x02 | 0x10 to open file for appending.
    foStream.init(file, 0x02 | 0x08 | 0x20, 0666, 0);
    // write, create, truncate
    // In a c file operation, we have no need to set file mode with or operation,
    // directly using "r" or "w" usually.

    return foStream;
}

var trackFiles  = {
    allFiles: {},
    add: function(fileName)
    {
        var name = new String(fileName);
        this.allFiles[name] = [];
    },
    drop: function(fileName)
    {
        var name = new String(fileName);
        this.allFiles[name].push("dropped");
    },
    def: function(frame)
    {
        var jscontext = frame.executionContext;
        if (jscontext)
            frameGlobal = new XPCNativeWrapper(jscontext.globalObject.getWrappedValue());

        var scopeName = fbs.getLocationSafe(frameGlobal);
        if (!scopeName)
            scopeName = "noGlobalObjectLocationInJSContext:"+(jscontext?jscontext.tag:"none");

        var name = new String(frame.script.fileName);
        if (! (name in this.allFiles))
            this.allFiles[name]=["not added"];
        this.allFiles[name].push(scopeName);
    },
    merge: function(moreFiles)
    {
        for (var p in moreFiles)
        {
            if (p in this.allFiles)
                this.allFiles[p] = this.allFiles[p].concat(moreFiles[p]);
            else
                this.allFiles[p] = moreFiles[p];
        }
    },
    dump: function()
    {
        var n = 0;
        for (var p in this.allFiles)
        {
            tmpout( (++n) + ") "+p);
            var where = this.allFiles[p];
            if (where.length > 0)
            {
                for (var i = 0; i < where.length; i++)
                {
                    tmpout(", "+where[i]);
                }
                tmpout("\n");
            }
            else
                tmpout("     none\n");

        }
    },
}

function tmpout(text)
{
    if (!fbs.foStream)
        fbs.foStream = getTmpStream(getTmpFile());

    fbs.foStream.write(text, text.length);

}
