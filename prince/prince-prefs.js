pref("toolkit.defaultChromeURI", "chrome://prince/content/prince.xul");
pref("general.useragent.extra.prince", "Prince/0.1");

/* debugging prefs 
pref("browser.dom.window.dump.enabled", true);
pref("javascript.options.showInConsole", true);
pref("javascript.options.strict", true);
pref("nglayout.debug.disable_xul_cache", true);
pref("nglayout.debug.disable_xul_fastload", true); */


pref("javascript.enabled",                  true);   
pref("toolkit.scrollbar.clickToScroll.scrollDelay", 100);
pref("toolkit.scrollbar.scrollIncrement", 20);
pref("toolkit.scrollbar.smoothscroll", true);

pref("swp.defaultShell", "articles/Standard_LaTeX_Article.sci");
pref("swp.defaultDialogShell", "chrome://prince/content/StdDialogShell.xhtml");
pref("swp.saveintervalseconds",10);
pref("swp.generateTeXonsave", true);
pref("swp.prefDocumentDir","SWPDocs");
pref("swp.viewPDF","fixAcrobat.cmd %1");
pref("swp.fancyreturn",false);
#ifdef XP_WIN32
pref("swp.tex.bindir","%programfiles%\\texlive\\2009\\bin\\win32");  /* for windows */
#else
pref("swp.tex.bindir","/usr/local/texlive/2009/bin/i386-linux");  /* for linux */
#endif
pref("swp.sourceview.maxlinelength",        100);
pref("swp.sourceview.minlinelength",         40);
pref("swp.sourceview.indentincrement",       2);

pref("editor.author",                      "");

pref("editor.text_color",                   "#000000");
pref("editor.link_color",                   "#0000FF");
pref("editor.active_link_color",            "#000088");
pref("editor.followed_link_color",          "#FF0000");
pref("editor.background_color",             "#FFFFFF");
pref("editor.use_background_image",         false);
pref("editor.default_background_image",     "");
pref("editor.use_custom_default_colors", 1);

pref("editor.hrule.height",                 2);
pref("editor.hrule.width",                  100);
pref("editor.hrule.width_percent",          true);
pref("editor.hrule.shading",                true);
pref("editor.hrule.align",                  1);

pref("editor.table.maintain_structure", true);

pref("editor.prettyprint", true);                        i

pref("editor.throbber.url","chrome://editor-region/locale/region.properties");

pref("editor.toolbars.showbutton.new", true);
pref("editor.toolbars.showbutton.open", true);
pref("editor.toolbars.showbutton.save", true);
pref("editor.toolbars.showbutton.publish", true);
pref("editor.toolbars.showbutton.preview", true);
pref("editor.toolbars.showbutton.cut", false);
pref("editor.toolbars.showbutton.copy", false);
pref("editor.toolbars.showbutton.paste", false);
pref("editor.toolbars.showbutton.print", true);
pref("editor.toolbars.showbutton.find", false);
pref("editor.toolbars.showbutton.image", true);
pref("editor.toolbars.showbutton.hline", false);
pref("editor.toolbars.showbutton.table", true);
pref("editor.toolbars.showbutton.link", true);
pref("editor.toolbars.showbutton.namedAnchor", false);
pref("editor.toolbars.showbutton.form", true);

pref("editor.toolbars.showbutton.bold", true);
pref("editor.toolbars.showbutton.italic", true);
pref("editor.toolbars.showbutton.underline", true);
pref("editor.toolbars.showbutton.DecreaseFontSize", true);
pref("editor.toolbars.showbutton.IncreaseFontSize", true);
pref("editor.toolbars.showbutton.ul", true);
pref("editor.toolbars.showbutton.ol", true);
pref("editor.toolbars.showbutton.outdent", true);
pref("editor.toolbars.showbutton.indent", true);

pref("editor.auto_save",                    false);
pref("editor.auto_save_delay",              10);
pref("editor.use_html_editor",              0);
pref("editor.html_editor",                  "");
pref("editor.use_image_editor",             0);
pref("editor.image_editor",                 "");

pref("editor.history.url_maximum", 10);

pref("editor.publish.",                      "");
pref("editor.lastFileLocation.image",        "");
pref("editor.lastFileLocation.html",         "");
pref("editor.save_associated_files",         true);
pref("editor.always_show_publish_dialog",    false);

/*
 * What are the entities that you want Mozilla to save using mnemonic
 * names rather than numeric codes? E.g. If set, we'll output &nbsp;
 * otherwise, we may output 0xa0 depending on the charset.
 *
 * "none"   : don't use any entity names; only use numeric codes.
 * "basic"  : use entity names just for &nbsp; &amp; &lt; &gt; &quot; for 
 *            interoperability/exchange with products that don't support more
 *            than that.
 * "latin1" : use entity names for 8bit accented letters and other special
 *            symbols between 128 and 255.
 * "html"   : use entity names for 8bit accented letters, greek letters, and
 *            other special markup symbols as defined in HTML4.
 */
//pref("editor.encode_entity",                 "html");

pref("editor.grid.snap",                     false);
pref("editor.grid.size",                     0);
pref("editor.resizing.preserve_ratio",       true);
pref("editor.positioning.offset",            0);
pref("general.skins.selectedSkin",           "classic");
pref("spellchecker.enablerealtimespell",      true);
pref("spellchecker.realtimespell.warning_color", "red");
/* for Prince */
pref("mousewheel.withcontrolkey.action", 3); //enable ctrl+mousewheel to change font size

pref("fastcursor.enabled", true);
pref("fastcursor.flashbar", 1);
pref("fastcursor.tapsforrepeat", 1);
pref("fastcursor.millisecondsfortaps", 500);

pref("swp.MuPAD.log_mathml_sent", true);
pref("swp.MuPAD.log_mathml_received", true);
pref("swp.MuPAD.log_engine_sent", true);
pref("swp.MuPAD.log_engine_received", true);

pref("swp.zoom_factor",            "1.0");
pref("swp.graph.XTickCount", "0");
pref("swp.graph.YTickCount", "0");
pref("swp.graph.ZTickCount", "0");
pref("swp.graph.XAxisLabel", "");
pref("swp.graph.YAxisLabel", "");
pref("swp.graph.ZAxisLabel", "");
pref("swp.graph.Dimension",  "2");
pref("swp.graph.ZAxisLabel", "");
pref("swp.graph.DefaultFileType", "xvc");
pref("swp.plot.XMax", "<math xmlns=\"http://www.w3.org/1998/Math/MathML\"><mrow><mn>6</mn></mrow></math>"); 
pref("swp.plot.XMin", "<math xmlns=\"http://www.w3.org/1998/Math/MathML\"><mrow><mo form=\"prefix\">-</mo><mn>6</mn></mrow></math>");
pref("swp.plot.YMax", "<math xmlns=\"http://www.w3.org/1998/Math/MathML\"><mrow><mn>6</mn></mrow></math>");
pref("swp.plot.YMin", "<math xmlns=\"http://www.w3.org/1998/Math/MathML\"><mrow><mo form=\"prefix\">-</mo><mn>6</mn></mrow></math>");
pref("swp.plot.ZMax", "<math xmlns=\"http://www.w3.org/1998/Math/MathML\"><mrow><mn>6</mn></mrow></math>");
pref("swp.plot.ZMin", "<math xmlns=\"http://www.w3.org/1998/Math/MathML\"><mrow><mo form=\"prefix\">-</mo><mn>6</mn></mrow></math>");

pref("swp.plot.XVar", "<math xmlns=\"http://www.w3.org/1998/Math/MathML\"><mrow><mi>x</mi></mrow></math>"); 
pref("swp.plot.YVar", "<math xmlns=\"http://www.w3.org/1998/Math/MathML\"><mrow><mi>y</mi></mrow></math>"); 
pref("swp.plot.ZVar", "<math xmlns=\"http://www.w3.org/1998/Math/MathML\"><mrow><mi>t</mi></mrow></math>"); 
pref("swp.plot.XPts", "<math xmlns=\"http://www.w3.org/1998/Math/MathML\"><mrow><mn>6</mn></mrow></math>");
pref("swp.plot.YPts", "<math xmlns=\"http://www.w3.org/1998/Math/MathML\"><mrow><mn>6</mn></mrow></math>");
pref("swp.plot.ZPts", "<math xmlns=\"http://www.w3.org/1998/Math/MathML\"><mrow><mn>6</mn></mrow></math>");

pref("swp.plot.Animate",        "false");
pref("swp.plot.DiscAdjust",     "true");
pref("swp.plot.IncludeLines",   "true");
pref("swp.plot.TubeRadius",     "<math xmlns=\"http://www.w3.org/1998/Math/MathML\"><mrow><mn>1</mn></mrow></math>");
pref("swp.plot.AISubIntervals", "10");
pref("swp.plot.AIMethod",       "Middle");
pref("swp.plot.AIInfo",         "NoInfo");

pref("swp.plot.XPts", "<math xmlns=\"http://www.w3.org/1998/Math/MathML\"><mrow><mn>30</mn></mrow></math>");
pref("swp.plot.YPts", "<math xmlns=\"http://www.w3.org/1998/Math/MathML\"><mrow><mn>20</mn></mrow></math>");
pref("swp.plot.ZPts", "<math xmlns=\"http://www.w3.org/1998/Math/MathML\"><mrow><mn>20</mn></mrow></math>");

pref("swp.bibtex.dir", "/swp55/TCITeX/BibTeX/bib");
pref("security.xpconnect.plugin.unrestricted", true);
pref("swp.spellchecker.enablerealtimespell", true);
pref("ui.caretBlinkTime", 500);
pref("ui.caretWidth", 1);
