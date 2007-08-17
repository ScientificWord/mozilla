// Copyright (c) 2006 MacKichan Software, Inc.  All Rights Reserved.

//NOTE! Either this file or editor.js - but NOT both - needs to be included in any window containing an editor object.
//The separation is to allow msiEditor.js and msiEditorUtilities.js to be included together with editor.js for the
//main document window, while dialogs with editors should actively not include editor.js.


var gComposerWindowControllerID = 0;
var prefAuthorString = "";

const nsIPlaintextEditor = Components.interfaces.nsIPlaintextEditor;
const nsIHTMLEditor = Components.interfaces.nsIHTMLEditor;
const nsITableEditor = Components.interfaces.nsITableEditor;
const nsIEditorStyleSheets = Components.interfaces.nsIEditorStyleSheets;
const nsIEditingSession = Components.interfaces.nsIEditingSession;

const kDisplayModeNormal = 0;
const kDisplayModeAllTags = 1;
const kDisplayModeSource = 2;
const kDisplayModePreview = 3;
const kDisplayModeMenuIDs = ["viewNormalMode", "viewAllTagsMode", "viewSourceMode", "viewPreviewMode"];
const kDisplayModeTabIDS = ["NormalModeButton", "TagModeButton", "SourceModeButton", "PreviewModeButton"];
const kNormalStyleSheet = "chrome://editor/content/EditorContent.css";
const kAllTagsStyleSheet = "chrome://editor/content/EditorAllTags.css";
const kParagraphMarksStyleSheet = "chrome://editor/content/EditorParagraphMarks.css";

const kTextMimeType = "text/plain";
const kHTMLMimeType = "text/html";

const kOutputEncodeBasicEntities = Components.interfaces.nsIDocumentEncoder.OutputEncodeBasicEntities;
const kOutputEncodeHTMLEntities = Components.interfaces.nsIDocumentEncoder.OutputEncodeHTMLEntities;
const kOutputEncodeLatin1Entities = Components.interfaces.nsIDocumentEncoder.OutputEncodeLatin1Entities;
const kOutputEncodeW3CEntities = Components.interfaces.nsIDocumentEncoder.OutputEncodeW3CEntities;
const kOutputFormatted = Components.interfaces.nsIDocumentEncoder.OutputFormatted;
const kOutputLFLineBreak = Components.interfaces.nsIDocumentEncoder.OutputLFLineBreak;
const kOutputSelectionOnly = Components.interfaces.nsIDocumentEncoder.OutputSelectionOnly;
const kOutputWrap = Components.interfaces.nsIDocumentEncoder.OutputWrap;

const msIWebNavigation = Components.interfaces.nsIWebNavigation;
const msIFilePicker = Components.interfaces.nsIFilePicker;
//
//var gPreviousNonSourceDisplayMode = 1;
var gEditorDisplayMode = -1;
//var gDocWasModified = false;  // Check if clean document, if clean then unload when user "Opens"

//var gContentWindow = 0;
//var gSourceContentWindow = 0;
//var gSourceTextEditor = null;
var gContentWindowDeck;
var gTagSelectBar;
var gComputeToolbar;
var gViewFormatToolbar;


var gDefaultTextColor = "";
var gDefaultBackgroundColor = "";

//var gCSSPrefListener;
var gStringBundle;
var gPrefs;
//var gLocalFonts = null;

const msigWin = "Win";
const msigUNIX = "UNIX";
const msigMac = "Mac";

const msiInputBoxCaretOffset = 1;
//var gLastFocusNode = null;
//var gLastFocusNodeWasSelected = false;

//// These must be kept in synch with the XUL <options> lists
//var gFontSizeNames = ["xx-small","x-small","small","medium","large","x-large","xx-large"];
//
////MSI stuff, see msiColorObj::Format() for more comprehensive value.
var gMathStyleSheet = "data:text/css,*|math { color: #FF0000; }";
//
//const nsIFilePicker = Components.interfaces.nsIFilePicker;
//
const kEditorToolbarPrefs = "editor.toolbars.showbutton.";

const XUL_NS = "http://www.mozilla.org/keymaster/gatekeeper/there.is.only.xul";
const mmlns    = "http://www.w3.org/1998/Math/MathML";
const xhtmlns  = "http://www.w3.org/1999/xhtml";

const MSI_EXTENSION = "sci";