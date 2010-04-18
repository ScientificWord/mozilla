/* See license.txt for terms of usage */


FBL.ns(function() { with (FBL) {

var toggleProfiling = $("fbToggleProfiling");

// ************************************************************************************************

Firebug.Profiler = extend(Firebug.Module,
{
    dispatchName: "profiler",
    showContext: function(browser, context)
    {
        this.setEnabled(context);
    },

    onPanelEnable: function(panelName)
    {
        if (panelName == "net" || panelName == "script")
            this.setEnabled();
    },

    onPanelDisable: function(panelName)
    {
       if (panelName == "net" || panelName == "script")
            this.setEnabled();
    },

    setEnabled: function()
    {
        // The profiler is available only if the debugger (script panel) and console are enabled.
        var debuggerEnabled = Firebug.Debugger.isAlwaysEnabled();
        var consoleEnabled = Firebug.Console.isAlwaysEnabled();
        toggleProfiling.disabled = !debuggerEnabled || !consoleEnabled;

        // Update button's tooltip.
        var tooltipText = toggleProfiling.disabled ? $STR("ProfileButton.Disabled.Tooltip")
            : $STR("ProfileButton.Enabled.Tooltip");
        toggleProfiling.setAttribute("tooltiptext", tooltipText);
    },

    toggleProfiling: function(context)
    {
        if (fbs.profiling)
            this.stopProfiling(context);
        else
            this.startProfiling(context);
    },

    startProfiling: function(context, title)
    {
        fbs.startProfiling();

        Firebug.chrome.setGlobalAttribute("cmd_toggleProfiling", "checked", "true");

        var isCustomMessage = !!title;
        if (!isCustomMessage)
            title = $STR("ProfilerStarted");

        context.profileRow = this.logProfileRow(context, title);
        context.profileRow.customMessage = isCustomMessage ;
    },

    isProfiling: function()
    {
        return (Firebug.chrome.getGlobalAttribute("cmd_toggleProfiling", "checked") === "true")
    },

    stopProfiling: function(context, cancelReport)
    {
        var totalTime = fbs.stopProfiling();
        if (totalTime == -1)
            return;

        Firebug.chrome.setGlobalAttribute("cmd_toggleProfiling", "checked", "false");

        if (cancelReport)
            delete context.profileRow;
        else
            this.logProfileReport(context)
    },

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    logProfileRow: function(context, title)
    {
        var row = Firebug.Console.openGroup(title, context, "profile",
            Firebug.Profiler.ProfileCaption, true, null, true);
        setClass(row, "profilerRunning");

        Firebug.Console.closeGroup(context, true);

        return row;
    },

    logProfileReport: function(context)
    {
        var calls = [];
        var totalCalls = 0;
        var totalTime = 0;

        var sourceFileMap = context.sourceFileMap;
        jsd.enumerateScripts({enumerateScript: function(script)
        {
            if (script.callCount)
            {
                if (!Firebug.filterSystemURLs || !isSystemURL(script.fileName))
                {
                    var sourceLink = FBL.getSourceLinkForScript(script, context);
                    if (sourceLink && sourceLink.href in sourceFileMap)
                    {
                        var call = new ProfileCall(script, context, script.callCount, script.totalExecutionTime,
                                script.totalOwnExecutionTime, script.minExecutionTime, script.maxExecutionTime, sourceLink);
                        calls.push(call);

                        totalCalls += script.callCount;
                        totalTime += script.totalOwnExecutionTime;
                    }
                }
                script.clearProfileData();
            }
        }});

        for (var i = 0; i < calls.length; ++i)
            calls[i].percent = Math.round((calls[i].totalOwnTime/totalTime) * 100 * 100) / 100;

        calls.sort(function(a, b)
        {
           return a.totalOwnTime < b.totalOwnTime ? 1 : -1;
        });

        totalTime = Math.round(totalTime * 1000) / 1000;

        var groupRow = context.profileRow && context.profileRow.ownerDocument
            ? context.profileRow
            : this.logProfileRow(context, "");
        delete context.profileRow;

        removeClass(groupRow, "profilerRunning");

        if (totalCalls > 0)
        {
            var captionBox = groupRow.getElementsByClassName("profileCaption").item(0);
            if (!groupRow.customMessage)
                captionBox.textContent = $STR("Profile");
            var timeBox = groupRow.getElementsByClassName("profileTime").item(0);
            timeBox.textContent = $STRP("plural.Profile_Time", [totalTime, totalCalls], 1);

            var groupBody = groupRow.lastChild;
            var sizer = Firebug.Profiler.ProfileTable.tag.replace({}, groupBody);
            var table = sizer.firstChild;
            var tHeader = table.lastChild;  // no rows inserted.

            var tag = Firebug.Profiler.ProfileCall.tag;
            var insert = tag.insertRows;

            for (var i = 0; i < calls.length; ++i) {
                calls[i].index = i;
                context.throttle(insert, tag, [{object: calls[i]}, tHeader]);
            }

            context.throttle(groupRow.scrollIntoView, groupRow);
        }
        else
        {
            var captionBox = groupRow.getElementsByClassName("profileCaption").item(0);
            captionBox.textContent = $STR("NothingToProfile");
        }
    }
});

// ************************************************************************************************

Firebug.Profiler.ProfileTable = domplate(
{
    tag:
      DIV({class: "profileSizer", "tabindex": "-1" },
        TABLE({class: "profileTable", cellspacing: 0, cellpadding: 0, width: "100%", "role": "grid"},
            TBODY({class: "profileTbody", "role": "presentation"},
                TR({class: "headerRow focusRow profileRow subFocusRow", onclick: "$onClick", "role": "row"},
                    TH({class: "headerCell alphaValue a11yFocus", "role": "columnheader"},
                        DIV({class: "headerCellBox"},
                            $STR("Function")
                        )
                    ),
                    TH({class: "headerCell a11yFocus" , "role": "columnheader"},
                        DIV({class: "headerCellBox", title: $STR("CallsHeaderTooltip")},
                            $STR("Calls")
                        )
                    ),
                    TH({class: "headerCell headerSorted a11yFocus", "role": "columnheader", "aria-sort": "descending"},
                        DIV({class: "headerCellBox", title: $STR("PercentTooltip")},
                            $STR("Percent")
                        )
                    ),
                    TH({class: "headerCell a11yFocus", "role": "columnheader"},
                        DIV({class: "headerCellBox", title: $STR("OwnTimeHeaderTooltip")},
                            $STR("OwnTime")
                        )
                    ),
                    TH({class: "headerCell a11yFocus", "role": "columnheader"},
                        DIV({class: "headerCellBox", title: $STR("TimeHeaderTooltip")},
                            $STR("Time")
                        )
                    ),
                    TH({class: "headerCell a11yFocus", "role": "columnheader"},
                        DIV({class: "headerCellBox", title: $STR("AvgHeaderTooltip")},
                            $STR("Avg")
                        )
                    ),
                    TH({class: "headerCell a11yFocus", "role": "columnheader"},
                        DIV({class: "headerCellBox", title: $STR("MinHeaderTooltip")},
                            $STR("Min")
                        )
                    ),
                    TH({class: "headerCell a11yFocus", "role": "columnheader"},
                        DIV({class: "headerCellBox", title: $STR("MaxHeaderTooltip")},
                            $STR("Max")
                        )
                    ),
                    TH({class: "headerCell alphaValue a11yFocus", "role": "columnheader"},
                        DIV({class: "headerCellBox"},
                            $STR("File")
                        )
                    )
                )
            )
          )
        ),

    onClick: function(event)
    {
        var table = getAncestorByClass(event.target, "profileTable");
        var header = getAncestorByClass(event.target, "headerCell");
        if (!header)
            return;

        var numerical = !hasClass(header, "alphaValue");

        var colIndex = 0;
        for (header = header.previousSibling; header; header = header.previousSibling)
            ++colIndex;

        this.sort(table, colIndex, numerical);
    },

    sort: function(table, colIndex, numerical)
    {
        var tbody = getChildByClass(table, "profileTbody");

        var values = [];
        for (var row = tbody.childNodes[1]; row; row = row.nextSibling)
        {
            var cell = row.childNodes[colIndex];
            var value = numerical ? parseFloat(cell.textContent) : cell.textContent;
            values.push({row: row, value: value});
        }

        values.sort(function(a, b) { return a.value < b.value ? -1 : 1; });

        var headerRow = tbody.firstChild;
        var headerSorted = getChildByClass(headerRow, "headerSorted");
        removeClass(headerSorted, "headerSorted");
        if (headerSorted)
            headerSorted.removeAttribute('aria-sort');

        var header = headerRow.childNodes[colIndex];
        setClass(header, "headerSorted");

        if (!header.sorted || header.sorted == 1)
        {
            removeClass(header, "sortedDescending");
            setClass(header, "sortedAscending");
            header.setAttribute("aria-sort", "ascending");

            header.sorted = -1;

            for (var i = 0; i < values.length; ++i) {
                values[i].row.setAttribute("odd", (i % 2));
                tbody.appendChild(values[i].row);
            }
        }
        else
        {
            removeClass(header, "sortedAscending");
            setClass(header, "sortedDescending");
            header.setAttribute("aria-sort", "descending")

            header.sorted = 1;

            for (var i = values.length-1; i >= 0; --i) {
                values[i].row.setAttribute("odd", (Math.abs(i-values.length-1) % 2));
                tbody.appendChild(values[i].row);
            }
        }
    }
});

// ************************************************************************************************

Firebug.Profiler.ProfileCaption = domplate(Firebug.Rep,
{
    tag:
        SPAN({class: "profileTitle", "role": "status"},
            SPAN({class: "profileCaption"}, "$objects"),
            " ",
            SPAN({class: "profileTime"}, "")
        )
});

// ************************************************************************************************

Firebug.Profiler.ProfileCall = domplate(Firebug.Rep,
{
    tag:
        TR({"class": "focusRow profileRow subFocusRow", odd: "$object|isOddRow", "role": "row"},
            TD({"class": "profileCell", "role": "presentation"},
                FirebugReps.OBJECTLINK("$object|getCallName")
            ),
            TD({"class": "a11yFocus profileCell", "role": "gridcell"}, "$object.callCount"),
            TD({"class": "a11yFocus profileCell", "role": "gridcell"}, "$object.percent%"),
            TD({"class": "a11yFocus profileCell", "role": "gridcell"}, "$object.totalOwnTime|roundTime\\ms"),
            TD({"class": "a11yFocus profileCell", "role": "gridcell"}, "$object.totalTime|roundTime\\ms"),
            TD({"class": "a11yFocus profileCell", "role": "gridcell"}, "$object|avgTime|roundTime\\ms"),
            TD({"class": "a11yFocus profileCell", "role": "gridcell"}, "$object.minTime|roundTime\\ms"),
            TD({"class": "a11yFocus profileCell", "role": "gridcell"}, "$object.maxTime|roundTime\\ms"),
            TD({class: "linkCell profileCell", "role": "presentation"},
                TAG(FirebugReps.SourceLink.tag, {object: "$object|getSourceLink"})
            )
        ),

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    isOddRow: function(call)
    {
        return (call.index % 2) ? 1 : 0;
    },

    getCallName: function(call)
    {
        return cropString(getFunctionName(call.script, call.context), 60);
    },

    avgTime: function(call)
    {
        return call.totalTime / call.callCount;
    },

    getSourceLink: function(call)
    {
        return call.sourceLink;
    },

    roundTime: function(ms)
    {
        return Math.round(ms * 1000) / 1000;
    },

    // * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *

    className: "profile",

    supportsObject: function(object)
    {
        return object instanceof ProfileCall;
    },

    inspectObject: function(call, context)
    {
        var sourceLink = this.getSourceLink(call);
        Firebug.chrome.select(sourceLink);
    },

    getTooltip: function(call)
    {
        try
        {
            var fn = unwrapIValue(call.script.functionObject);
            return FirebugReps.Func.getTooltip(fn, call.context);
        }
        catch (exc)
        {
        }
    },

    getContextMenuItems: function(call, target, context)
    {
        var fn = unwrapIValue(call.script.functionObject);
        return FirebugReps.Func.getContextMenuItems(fn, call.script, context);
    }
});

// ************************************************************************************************

function ProfileCall(script, context, callCount, totalTime, totalOwnTime, minTime, maxTime, sourceLink)
{
    this.script = script;
    this.context = context;
    this.callCount = callCount;
    this.totalTime = totalTime;
    this.totalOwnTime = totalOwnTime;
    this.minTime = minTime;
    this.maxTime = maxTime;
    this.sourceLink = sourceLink;
}

// ************************************************************************************************

Firebug.registerModule(Firebug.Profiler);
Firebug.registerRep(Firebug.Profiler.ProfileCall);

// ************************************************************************************************

}});
