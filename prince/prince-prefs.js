
/* debugging prefs
pref("browser.dom.window.dump.enabled", true);
pref("javascript.options.showInConsole", true);
pref("javascript.options.strict", true);
pref("nglayout.debug.disable_xul_cache", true);
pref("nglayout.debug.disable_xul_fastload", true); */
#ifdef XP_WIN32
pref("swp.tex.bindir","%programfiles%\\texlive\\2010\\bin\\win32");  /* for windows */
#elifdef XP_MACOSX
pref("swp.tex.bindir","/usr/texbin"); /* Mac system preference for TeX puts the right thing on the path */
#else /*XP_UNIX*/
pref("swp.tex.bindir","/usr/local/texlive/2010/bin/i386-linux");  /* for linux */
#endif

/* default table prefs */
/* for Prince */
pref("browser.cache.disk.enable",           false);
pref("browser.hiddenWindowChromeURL", "chrome://prince/content/hiddenwindow.xul");
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
pref("editor.history.url_maximum", 10);
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
pref("editor.prettyprint", true);
pref("editor.publish.",                      "");
pref("editor.resizing.preserve_ratio",       true);
pref("editor.save_associated_files",         true);
pref("editor.table.default_align",           "");
pref("editor.table.default_cellpadding",     "3px");
pref("editor.table.default_cellspacing",     "0px");
pref("editor.table.default_valign",          "");
pref("editor.table.default_wrapping",        "wrap");
pref("editor.table.maintain_structure", true);
pref("editor.text_color",                   "#000000");
pref("editor.throbber.url","chrome://editor-region/locale/region.properties");
pref("accessibility.typeaheadfind.flashBar", 0);
pref("editor.use_background_image",         false);
pref("editor.use_custom_default_colors", 1);
pref("editor.use_html_editor",              0);
pref("editor.use_image_editor",             0);
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
pref("swp.bibtex.dir", "/swp55/TCITeX/BibTeX/bib");
pref("swp.ctrl.m","math"); // other value is 'toggle'
pref("swp.ctrl.t","text"); // other value is'toggle'
pref("swp.defaultDialogShell", "chrome://prince/content/StdDialogShell.xhtml");
pref("swp.GraphicsSnapshopRes", 300);
pref("swp.debugtools", false);
pref("swp.defaultGraphicsVSize", "2.0");
pref("swp.defaultGraphicsHSize", "3.0");
pref("swp.defaultGraphicsInlineOffset", "0.0");
pref("swp.defaultGraphicsSizeUnits", "in");
pref("swp.defaultGraphicsPlacement", "display");
pref("swp.defaultGraphicsFloatLocation.forceHere", false);
pref("swp.defaultGraphicsFloatLocation.here", true);
pref("swp.defaultGraphicsFloatLocation.pageFloats", false);
pref("swp.defaultGraphicsFloatLocation.topPage", false);
pref("swp.defaultGraphicsFloatLocation.bottomPage", false);
pref("swp.defaultGraphicsFloatPlacement", "I");
pref("swp.graphics.border", "0.03");
pref("swp.graphics.HMargin", "0.3");
pref("swp.graphics.VMargin", "0.15");
pref("swp.graphics.padding", "0.2");
pref("swp.graphics.BGColor", "#FFFFFF");
pref("swp.graphics.borderColor", "#000000");
pref("swp.graphicsUseDefaultWidth", false);
pref("swp.graphicsUseDefaultHeight", false);
pref("swp.defaultShell", "articles/Standard_LaTeX_Article.sci");
pref("swp.fancyreturn",true);
pref("swp.generateTeXonsave", true);
pref("swp.graph.DefaultFileType", "xvc");
pref("swp.graph.Dimension",  "2");
pref("swp.graph.HSize", "3.0");
pref("swp.graph.VSize", "4.5");
pref("swp.graph.defaultUnits", "cm");
pref("swp.graph.placement", "display");
pref("swp.graph.placeLocation", "h");  // other choices from LaTeX: subsets of hHptb
pref("swp.graph.floatPlacement", "I");
pref("swp.graph.XAxisLabel", "x");
pref("swp.graph.XTickCount", "0");
pref("swp.graph.YAxisLabel", "y");
pref("swp.graph.YTickCount", "0");
pref("swp.graph.ZAxisLabel", "z");
pref("swp.graph.ZTickCount", "0");
pref("swp.graph.border", "0.03");
pref("swp.graph.HMargin", "0.3");
pref("swp.graph.VMargin", "0.15");
pref("swp.graph.padding", "0.2");
pref("swp.graph.BGColor", "#FFFFFF");
pref("swp.graph.borderColor", "#000000");
pref("swp.GraphicsSnapshotRes", 600);
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
pref("swp.prefDocumentDir","SWPDocs");
pref("swp.prefPDFPath","default");
pref("swp.saveintervalseconds",10);
pref("swp.sci.compression", 0);
pref("swp.sourceview.indentincrement",       2);
pref("swp.sourceview.maxlinelength",        100);
pref("swp.sourceview.minlinelength",         60);
pref("swp.space.after.space", false); // true means space space -> math
pref("swp.spellchecker.enablerealtimespell", true);
pref("swp.viewPDF","default");
pref("swp.webzip.compression", 9);
pref("swp.zoom_factor",            "1.0");
pref("swp.defaultTableUnits", "in");

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
pref("swp.savetoweb.mathjaxsrc", "https://c328740.ssl.cf1.rackcdn.com/mathjax/latest/MathJax.js?config=TeX-AMS-MML_HTMLorMML");
pref("swp.savetoweb.cssurl", "http://www.mackichan.com/supportfiles/6.0/css/");
pref("swp.openlastfile", true);
pref("swp.lastfilesaved", "");
pref("swp.openlastfile", false);
pref("font.mathfont-family", "MathJax_Main, Asana Math, XITS Math, Symbol, DejaVu Sans, Cambria Math");
