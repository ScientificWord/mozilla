/* debugging prefs
pref("browser.dom.window.dump.enabled", true);
pref("javascript.options.showInConsole", true);
pref("javascript.options.strict", true);
pref("nglayout.debug.disable_xul_cache", true);
pref("nglayout.debug.disable_xul_fastload", true); */
/* InstallBuilder now fills these in
#ifdef XP_WIN32
pref("swp.tex.bindir","%programfiles%\\texlive\\2010\\bin\\win32");
pref("swp.bibtex.dir","%programfiles%\\texlive\\2010\\texmf-dist\\BibTeX\\bib");
#elifdef XP_MACOSX

#else
pref("swp.tex.bindir","/usr/local/texlive/2010/bin/i386-linux");
pref("swp.tex.bindir","/usr/local/texlive/2010/bin/i386-linux");
#endif */


/* default table prefs */
/* for Prince */
// pref("nglayout.debug.disable_xul_fastload", true);
pref("ui.-moz-dialog", "#F3F3F3F3");
pref("browser.cache.disk.enable",           false);
pref("browser.hiddenWindowChromeURL", "chrome://prince/content/hiddenwindow.xul");
pref("browser.chromeURL", "chrome://prince/content/prince.xul");
// extend the default range of zooming; max and min percent should agree with the zoomvalue limits
pref("zoom.minPercent", 35);
pref("zoom.maxPercent", 1600);
pref("toolkit.zoomManager.zoomValues", "0.35,0.5,0.75,1.0,1.25,1.5,2.0,2.8,4.0,5.6,8.0,11.0,16.0");

pref("editor.active_link_color",            "#000088");
pref("editor.always_show_publish_dialog",    false);
pref("editor.author",                      "");
pref("editor.auto_save",                    false);
pref("editor.auto_save_delay",              10);
pref("editor.background_color",             "#FFFFFF");
pref("editor.default_background_image",     "");
pref("editor.followed_link_color",          "#FF0000");
pref("editor.grid.size",                     0);
pref("editor.grid.snap",                     false);
pref("editor.history.url_maximum", 			10);
pref("editor.hrule.align",                  1);
pref("editor.hrule.height",                 2);
pref("editor.hrule.shading",                true);
pref("editor.hrule.width",                  100);
pref("editor.hrule.width_percent",          true);
pref("editor.html_editor",                  "");
pref("editor.image_editor",                 "");
pref("editor.lastFileLocation.html",         "");
pref("editor.lastFileLocation.image",        "");
pref("editor.link_color",                   "#0000FF");
pref("editor.positioning.offset",            0);
pref("editor.prettyprint", 					 true);
pref("editor.publish.",                      "");
pref("editor.resizing.preserve_ratio",       true);
pref("editor.save_associated_files",         true);
pref("editor.text_color",                   "#000000");
pref("editor.throbber.url","chrome://editor-region/locale/region.properties");
pref("accessibility.typeaheadfind.flashBar", 0);
pref("editor.use_background_image",         false);
pref("editor.use_custom_default_colors", 1);
pref("editor.use_html_editor",              0);
pref("editor.use_image_editor",             0);
pref("app.support.baseURL", "www.mackichan.com");
pref("extensions.dss.switchPending",false);
pref("fastcursor.enabled", true);
pref("fastcursor.flashbar", 1);
pref("fastcursor.millisecondsfortaps", 500);
pref("fastcursor.tapsforrepeat", 1);
pref("general.skins.selectedSkin",           "modern");
pref("general.useragent.extra.prince", "SW-SWP-SNB/6.0");
pref("javascript.enabled",                  true);
pref("mousewheel.withcontrolkey.action", 3); //enable ctrl+mousewheel to change font size
pref("security.xpconnect.plugin.unrestricted", true);
pref("spellchecker.enablerealtimespell",      true);
pref("spellchecker.realtimespell.warning_color", "red");
pref("swp.messagelogger","dump");
pref("swp.ctrl.m","math"); // other value is 'toggle'
pref("swp.ctrl.t","text"); // other value is'toggle'
pref("swp.defaultDialogShell", "chrome://prince/content/stdDialogShell.xhtml");
pref("swp.graphics.snapshotres", 300);
pref("swp.debugtools", false);
pref("swp.graphics.vsize", "2.8");
pref("swp.graphics.hsize", "3.5");
pref("swp.graphics.keepaspect", true);
pref("swp.graphics.inlineoffset", "0.0");
pref("swp.graphics.units", "in");
pref("swp.graphics.placement", "center");
pref("swp.graphics.captionplacement", "none");
pref("swp.graphics.floatlocation.forcehere", false);
pref("swp.graphics.floatlocation.here", true);
pref("swp.graphics.floatlocation.pagefloats", false);
pref("swp.graphics.floatlocation.toppage", false);
pref("swp.graphics.floatlocation.bottompage", false);
pref("swp.graphics.floatplacement", "I");
pref("swp.graphics.border", "0.01");
pref("swp.graphics.hmargin", "0.1");
pref("swp.graphics.vmargin", "0.1");
pref("swp.graphics.paddingwidth", "0.1");
pref("swp.graphics.bgcolor", "#ffffff");
pref("swp.graphics.bordercolor", "#ffffff");
pref("swp.graphics.usedefaultwidth", false);
pref("swp.graphics.usedefaultheight", false);

// matrix defaults
pref("swp.matrixdef.colalign", "c");  //l, c, or r -- left, center, right
pref("swp.matrixdef.baseline", ""); //t, blank, or b -- baseline aligns with top, center, or bottom
pref("swp.matrixdef.delim", "p"); //p, b, B, v, V, cases, rcases or blank -- parents, square brackets, braces, vert lines, vert double lines, left cases, right cases, or blank
pref("swp.matrixdef.small", false); 

pref("swp.table.vsize", "2.8");
pref("swp.table.hsize", "3.5");
pref("swp.table.inlineoffset", "0.0");
pref("swp.table.units", "cm");
pref("swp.table.placement", "center");
pref("swp.table.floatlocation.forcehere", false);
pref("swp.table.floatlocation.here", true);
pref("swp.table.floatlocation.pagefloats", false);
pref("swp.table.floatlocation.toppage", false);
pref("swp.table.floatlocation.bottompage", false);
pref("swp.table.floatplacement", "I");
pref("swp.table.border", "0.0");
pref("swp.table.hmargin", "0.0");
pref("swp.table.vmargin", "0.0");
pref("swp.table.paddingwidth", "0.0");
pref("swp.table.bgcolor", "#ffffff");
pref("swp.table.bordercolor", "#ffffff");
pref("swp.table.usedefaultwidth", false);
pref("swp.table.usedefaultheight", false);
pref("swp.table.align",           "");
pref("swp.table.valign",          "");
pref("swp.table.cellpadding",     "3px");
pref("swp.table.cellspacing",     "0px");
pref("swp.table.wrapping",        "wrap");

pref("editor.table.maintain_structure", true);


pref("swp.defaultShell", "-Standard_LaTeX/Standard_LaTeX_Article.sci");
pref("swp.contentOfNewWindow", "useDefault");  // other choices are useLast and useBlank
pref("swp.fancyreturn",true);
pref("swp.generateTeXonsave", true);
pref("swp.graph.filetype", "xvc");
pref("swp.graph.Dimension",  "2");
pref("swp.graph.hsize", "3.5");
pref("swp.graph.vsize", "2.8");
pref("swp.graph.units", "in");
pref("swp.graph.keepaspect", true);

pref("swp.graph.placement", "center");
pref("swp.graph.placeLocation", "h");  // other choices from LaTeX: subsets of hHptb
pref("swp.graph.floatlocation.forcehere", false);
pref("swp.graph.floatlocation.here", true);
pref("swp.graph.floatlocation.pagefloats", false);
pref("swp.graph.floatlocation.toppage", false);
pref("swp.graph.floatlocation.bottompage", false);
pref("swp.graph.floatplacement", "I");
pref("swp.graph.xaxislabel", "x");
pref("swp.graph.xtickcount", "0");
pref("swp.graph.yaxislabel", "y");
pref("swp.graph.ytickcount", "0");
pref("swp.graph.zaxislabel", "z");
pref("swp.graph.ztickcount", "0");
pref("swp.graph.border", "0.00");
pref("swp.graph.hmargin", "0.1");
pref("swp.graph.vmargin", "0.1");
pref("swp.graph.HMargin", "0.1");
pref("swp.graph.VMargin", "0.1");
pref("swp.graph.paddingwidth", "0.0");
pref("swp.graph.paddingcolor", "#FFFFFF");
pref("swp.graph.bgcolor", "#FFFFFF");
pref("swp.graph.bordercolor", "#ffffff");
pref("swp.graph.captionplacement", "none");
pref("swp.graph.snapshotres", 600);
pref("swp.matrix.cols", 3);
pref("swp.matrix.rows", 3);
pref("swp.MuPAD.log_engine_received", true);
pref("swp.MuPAD.log_engine_sent", true);
pref("swp.MuPAD.log_mathml_received", false);
pref("swp.MuPAD.log_mathml_sent", false);
pref("swp.newFileConsideredDirty", false);
pref("swp.plot.LineStyle", "Solid");
pref("swp.plot.PointStyle", "Dot");
pref("swp.plot.LineThickness", "thin");
pref("swp.plot.LineColor", "000000");
pref("swp.plot.DirectionalShading", "XYZ");
pref("swp.plot.BaseColor", "ff0000");
pref("swp.plot.SecondaryColor", "0000ff");
pref("swp.plot.PointSymbol", "FilledCircles");
pref("swp.plot.SurfaceStyle", "colorpatch");
pref("swp.plot.IncludePoints", "false");
pref("swp.plot.SurfaceMesh", "mesh");
pref("swp.plot.FillPattern", "Solid");
pref("swp.plot.AIInfo",         "NoInfo");
pref("swp.plot.AIMethod",       "Middle");
pref("swp.plot.AISubIntervals", 10);
pref("swp.plot.Animate",        "false");
pref("swp.plot.DiscAdjust",     "true");
pref("swp.plot.IncludeLines",   "true");
pref("swp.plot.AxisScaling", "LinLin");
pref("swp.plot.AxisEqualScaling", "false");
pref("swp.plot.TubeRadius",     "<math xmlns=\"http://www.w3.org/1998/Math/MathML\"><mrow><mn>1</mn></mrow></math>");
pref("swp.plot.XMax", "<math xmlns=\"http://www.w3.org/1998/Math/MathML\"><mrow><mn>6</mn></mrow></math>");
pref("swp.plot.XMin", "<math xmlns=\"http://www.w3.org/1998/Math/MathML\"><mrow><mo form=\"prefix\">-</mo><mn>6</mn></mrow></math>");
pref("swp.plot.XPts", 30);
pref("swp.plot.XVar", "<math xmlns=\"http://www.w3.org/1998/Math/MathML\"><mrow><mi>x</mi></mrow></math>");
pref("swp.plot.YMax", "<math xmlns=\"http://www.w3.org/1998/Math/MathML\"><mrow><mn>6</mn></mrow></math>");
pref("swp.plot.YMin", "<math xmlns=\"http://www.w3.org/1998/Math/MathML\"><mrow><mo form=\"prefix\">-</mo><mn>6</mn></mrow></math>");
pref("swp.plot.YPts", 10);
pref("swp.plot.YVar", "<math xmlns=\"http://www.w3.org/1998/Math/MathML\"><mrow><mi>y</mi></mrow></math>");
pref("swp.plot.ZMax", "<math xmlns=\"http://www.w3.org/1998/Math/MathML\"><mrow><mn>6</mn></mrow></math>");
pref("swp.plot.ZMin", "<math xmlns=\"http://www.w3.org/1998/Math/MathML\"><mrow><mo form=\"prefix\">-</mo><mn>6</mn></mrow></math>");
pref("swp.plot.ZPts", 6);
pref("swp.plot.ZVar", "<math xmlns=\"http://www.w3.org/1998/Math/MathML\"><mrow><mi>z</mi></mrow></math>");
pref("swp.plot.AnimMax", "<math xmlns=\"http://www.w3.org/1998/Math/MathML\"><mrow><mn>10</mn></mrow></math>");
pref("swp.plot.AnimMin", "<math xmlns=\"http://www.w3.org/1998/Math/MathML\"><mrow><mn>0</mn></mrow></math>");
pref("swp.plot.AnimVar", "<math xmlns=\"http://www.w3.org/1998/Math/MathML\"><mrow><mi>t</mi></mrow></math>");
pref("swp.plot.TubeRadialPts", 9);
#ifdef XP_MACOSX
pref("swp.prefPDFPath","launch");
#else
pref("swp.prefPDFPath","default");
#endif
pref("swp.saveintervalseconds",30);
pref("swp.sci.compression", 0);
pref("swp.sourceview.indentincrement",       2);
pref("swp.sourceview.maxlinelength",        100);
pref("swp.sourceview.minlinelength",         60);
pref("swp.space.after.space", "nil");
pref("swp.spellchecker.enablerealtimespell", true);
pref("swp.viewPDF","default");
pref("swp.webzip.compression", 9);
pref("swp.zoom_factor",            "1.0");
pref("swp.defaultTableUnits", "in");
pref("swp.bibchoice", "manual");
pref("swp.bibtex.dir", "");
pref("swp.bibtexstyle.dir", "");

pref("toolkit.defaultChromeURI", "chrome://prince/content/prince.xul");
pref("toolkit.scrollbar.clickToScroll.scrollDelay", 100);
pref("toolkit.scrollbar.scrollIncrement", 20);
pref("toolkit.scrollbar.smoothscroll", true);
pref("ui.caretBlinkTime", 500);
pref("ui.caretWidth", 1);
pref("swp.user.barconj", true);
pref("swp.user.degree", 3);
pref("swp.user.derivformat", "derivformat_input");
pref("swp.user.diffD", "diffD_D");
pref("swp.user.diffd", "diffd_d");
pref("swp.user.digitsrendered", 5);
pref("swp.user.dotderivative", false);
pref("swp.user.engReceived", true);
pref("swp.user.engSent", true);
pref("swp.user.e_exp", true);
pref("swp.user.i_imaginary", true);
pref("swp.user.j_imaginary", false);
pref("swp.user.output_imagi", "imagi");
pref("swp.user.loge", true);
pref("swp.user.logReceived", false);
pref("swp.user.logSent", false);
pref("swp.user.lowerthreshold", 4);
pref("swp.user.matrix_delim", "matrix_none");
pref("swp.user.mixednum", false);
pref("swp.user.primederiv", true);
pref("swp.user.primesasn", 4);
pref("swp.user.principal", false);
pref("swp.user.special", false);
pref("swp.user.trigargs", false);
pref("swp.user.upperthreshold", 4);
pref("swp.user.usearc", false);
pref("swp.savetoweb.mathjaxsrc", "https://cdnjs.cloudflare.com/ajax/libs/mathjax/2.7.1/MathJax.js?config=TeX-AMS-MML_HTMLorMML");
pref("swp.savetoweb.cssurl", "www.mackichan.com/supportfiles/6.0/css/");
pref("swp.openlastfile", false);
pref("swp.lastfilesaved", "");
pref("swp.firstrun", true);
pref("font.mathfont-family", "MathJax_Math, XITS Math, Symbol, DejaVu Sans, Cambria Math, Asana Math");

#ifdef USE_MOZILLA_UPDATES

// Whether or not app updates are enabled
pref("app.update.enabled", true);

// This preference turns on app.update.mode and allows automatic download and
// install to take place. We use a separate boolean toggle for this to make
// the UI easier to construct.
pref("app.update.auto", true);

// Defines how the Application Update Service notifies the user about updates:
//
// AUM Set to:        Minor Releases:     Major Releases:
// 0                  download no prompt  download no prompt
// 1                  download no prompt  download no prompt if no incompatibilities
// 2                  download no prompt  prompt
//
// See chart in nsUpdateService.js.in for more details
//
pref("app.update.mode", 2);

// If set to true, the Update Service will present no UI for any event.
pref("app.update.silent", false);

// Update service URL:
// You do not need to use all the %VAR% parameters. Use what you need, %PRODUCT%,%VERSION%,%BUILD_ID%,%CHANNEL% for example
pref("app.update.url", "https://update.mackichan.com/update/3/%PRODUCT%/%VERSION%/%BUILD_ID%/%BUILD_TARGET%/%LOCALE%/%OS_VERSION%/update.xml");

// URL user can browse to manually if for some reason all update installation
// attempts fail.
pref("app.update.url.manual", "update.mackichan.com/%PRODUCT%");

// A default value for the "More information about this update" link
// supplied in the "An update is available" page of the update wizard.
pref("app.update.url.details", "update.mackichan.com/%PRODUCT%");

// User-settable override to app.update.url for testing purposes.
//pref("app.update.url.override", "");

// Interval: Time between checks for a new version (in seconds)
//           default=1 day
pref("app.update.interval", 86400);

// Interval: Time before prompting the user to download a new version that
//           is available (in seconds) default=1 day
pref("app.update.nagTimer.download", 86400);

// Interval: Time before prompting the user to restart to install the latest
//           download (in seconds) default=30 minutes
pref("app.update.nagTimer.restart", 1800);

// Interval: When all registered timers should be checked (in milliseconds)
//           default=5 seconds
pref("app.update.timer", 600000);

// Whether or not we show a dialog box informing the user that the update was
// successfully applied. This is off in Firefox by default since we show a
// upgrade start page instead! Other apps may wish to show this UI, and supply
// a whatsNewURL field in their brand.properties that contains a link to a page
// which tells users what's new in this new update.
pref("app.update.showInstalledUI", true);

// 0 = suppress prompting for incompatibilities if there are updates available
//     to newer versions of installed addons that resolve them.
// 1 = suppress prompting for incompatibilities only if there are VersionInfo
//     updates available to installed addons that resolve them, not newer
//     versions.
pref("app.update.incompatible.mode", 0);

#endif


