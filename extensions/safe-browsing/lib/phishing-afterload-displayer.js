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
 * The Original Code is Google Safe Browsing.
 *
 * The Initial Developer of the Original Code is Google Inc.
 * Portions created by the Initial Developer are Copyright (C) 2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Fritz Schneider <fritz@google.com> (original author)
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


// Implementation of the warning message we show users when we
// notice navigation to a phishing page after it has loaded. The
// browser view encapsulates all the hide/show logic, so the displayer
// doesn't need to know when to display itself, only how.
//
// Displayers implement the following interface:
//
// start() -- fired to initialize the displayer (to make it active). When
//            called, this displayer starts listening for and responding to
//            events. At most one displayer per tab should be active at a 
//            time, and start() should be called at most once.
// declineAction() -- fired when the user declines the warning.
// acceptAction() -- fired when the user declines the warning
// explicitShow() -- fired when the user wants to see the warning again
// browserSelected() -- the browser is the top tab
// browserUnselected() -- the browser is no long the top tab
// done() -- clean up. May be called once (even if the displayer wasn't
//           activated).
//
// At the moment, all displayers share access to the same xul in 
// safebrowsing-overlay.xul. Hence the need for at most one displayer
// to be active per tab at a time.

/**
 * Factory that knows how to create a displayer appropriate to the
 * user's platform. We use a clunky canvas-based displayer for all
 * platforms until such time as we can overlay semi-transparent
 * areas over browser content.
 *
 * See the base object for a description of the constructor args
 *
 * @constructor
 */
function PROT_PhishMsgDisplayer(msgDesc, browser, doc, url) {

  // TODO: Change this to return a PhishMsgDisplayerTransp on windows
  // (and maybe other platforms) when Firefox 2.0 hits.

  return new PROT_PhishMsgDisplayerCanvas(msgDesc, browser, doc, url);
}


/**
 * Base class that implements most of the plumbing required to hide
 * and show a phishing warning. Subclasses implement the actual
 * showMessage and hideMessage methods. 
 *
 * This class is not meant to be instantiated directly.
 *
 * @param msgDesc String describing the kind of warning this is
 * @param browser Reference to the browser over which we display the msg
 * @param doc Reference to the document in which browser is found
 * @param url String containing url of the problem document
 * @constructor
 */
function PROT_PhishMsgDisplayerBase(msgDesc, browser, doc, url) {
  this.debugZone = "phisdisplayer";
  this.msgDesc_ = msgDesc;                                // currently unused
  this.browser_ = browser;
  this.doc_ = doc;
  this.url_ = url;

  // We'll need to manipulate the XUL in safebrowsing-overlay.xul
  this.messageId_ = "safebrowsing-palm-message";
  this.messageTailId_ = "safebrowsing-palm-message-tail-container";
  this.messageContentId_ = "safebrowsing-palm-message-content";
  this.extendedMessageId_ = "safebrowsing-palm-extended-message";
  this.showmoreLinkId_ = "safebrowsing-palm-showmore-link";
  this.urlbarIconId_ = "safebrowsing-urlbar-icon";
  this.refElementId_ = this.urlbarIconId_;

  // We use this to report user actions to the server
  this.reporter_ = new PROT_Reporter();

  // The active UI elements in our warning send these commands; bind them
  // to their handlers but don't register the commands until we start
  // (because another displayer might be active)
  this.commandHandlers_ = {
    "safebrowsing-palm-showmore":
      BindToObject(this.showMore_, this),
    "safebrowsing-palm-phishingorg":
      BindToObject(this.showURL_, this, PROT_globalStore.getAntiPhishingURL()),
    "safebrowsing-palm-phishingfaq":
      BindToObject(this.showURL_, this, PROT_globalStore.getPhishingFaqURL()),
    "safebrowsing-palm-fraudpage" :
      BindToObject(this.showURL_, this, PROT_globalStore.getHomePageURL()),
    "safebrowsing-palm-falsepositive":
      BindToObject(this.showURL_, this, PROT_globalStore.getFalsePositiveURL()),
  };

  this.windowWatcher_ = 
    Components.classes["@mozilla.org/embedcomp/window-watcher;1"]
    .getService(Components.interfaces.nsIWindowWatcher);
}

/**
 * @returns The default background color of the browser
 */
PROT_PhishMsgDisplayerBase.prototype.getBackgroundColor_ = function() {
  var pref = Components.classes["@mozilla.org/preferences-service;1"].
             getService(Components.interfaces.nsIPrefBranch);
  return pref.getCharPref("browser.display.background_color");
}

/**
 * Fired when the user declines our warning. Report it!
 */
PROT_PhishMsgDisplayerBase.prototype.declineAction = function() {
  G_Debug(this, "User declined warning.");
  G_Assert(this, this.started_, "Decline on a non-active displayer?");
  this.reporter_.report("phishdecline", this.url_);

  this.messageShouldShow_ = false;
  if (this.messageShowing_)
    this.hideMessage_();
}

/**
 * Fired when the user accepts our warning
 */
PROT_PhishMsgDisplayerBase.prototype.acceptAction = function() {
  G_Assert(this, this.started_, "Accept on an unstarted displayer?");
  G_Assert(this, this.done_, "Accept on a finished displayer?");
  G_Debug(this, "User accepted warning.");
  this.reporter_.report("phishaccept", this.url_);

  var url = PROT_globalStore.getGetMeOutOfHereURL();
  this.browser_.loadURI(url);
}

/**
 * Invoked when the browser is resized
 */
PROT_PhishMsgDisplayerBase.prototype.onBrowserResized_ = function(event) {

  G_Debug(this, "Got resize for " + event.target.nodeName);

  if (event.target == this.doc_) {
    G_Debug(this, "User resized browser.");

    if (this.messageShowing_) {
      this.hideMessage_(); 
      this.showMessage_();
    }
  }
}

/**
 * Invoked by the browser view when our browser is switched to
 */
PROT_PhishMsgDisplayerBase.prototype.browserSelected = function() {
  G_Assert(this, this.started_, "Displayer selected before being started???");

  // If messageshowing hasn't been set, then this is the first time this
  // problematic browser tab has been on top, so do our setup and show
  // the warning.
  if (this.messageShowing_ === undefined) {
    this.messageShouldShow_ = true;
    this.ensureNavBarShowing_();
  }

  this.hideLockIcon_();        // Comes back when we are unselected or unloaded
  this.addWarningInUrlbar_();  // Goes away when we are unselected or unloaded

  // messageShouldShow might be false if the user dismissed the warning, 
  // switched tabs, and then switched back. We're still active, but don't
  // want to show the warning again. The user can cause it to show by
  // clicking our icon in the urlbar.
  if (this.messageShouldShow_)
    this.showMessage_();
}

/**
 * Invoked to display the warning message explicitly, for example if the user
 * clicked the url warning icon.
 */
PROT_PhishMsgDisplayerBase.prototype.explicitShow = function() {
  this.messageShouldShow_ = true;
  if (!this.messageShowing_)
    this.showMessage_();
}

/** 
 * Invoked by the browser view when our browser is switched away from
 */
PROT_PhishMsgDisplayerBase.prototype.browserUnselected = function() {
  this.removeWarningInUrlbar_();
  this.unhideLockIcon_();
  if (this.messageShowing_)
    this.hideMessage_();
}

/**
 * Invoked to make this displayer active. The displayer will now start
 * responding to notifications such as commands and resize events. We
 * can't do this in the constructor because there might be many 
 * displayers instantiated waiting in the problem queue for a particular
 * browser (e.g., a page has multiple framed problem pages), and we
 * don't want them all responding to commands!
 *
 * Invoked zero (the page we're a warning for was nav'd away from
 * before it reaches the head of the problem queue) or one (we're
 * displaying this warning) times by the browser view.
 */
PROT_PhishMsgDisplayerBase.prototype.start = function() {
  G_Assert(this, this.started_ == undefined, "Displayer started twice?");
  this.started_ = true;

  this.commandController_ = new PROT_CommandController(this.commandHandlers_);
  this.doc_.defaultView.controllers.appendController(this.commandController_);

  this.resizeHandler_ = BindToObject(this.onBrowserResized_, this);
  this.doc_.defaultView.addEventListener("resize", 
                                         this.resizeHandler_, 
                                         true);
}

/**
 * @returns Boolean indicating whether this displayer is currently
 *          active
 */
PROT_PhishMsgDisplayerBase.prototype.isActive = function() {
  return !!this.started_;
}

/**
 * Invoked by the browser view to clean up after the user is done 
 * interacting with the message. Should be called once by the browser
 * view. 
 */
PROT_PhishMsgDisplayerBase.prototype.done = function() {
  G_Assert(this, !this.done_, "Called done more than once?");
  this.done_ = true;

  // If the Document we're showing the warning for was nav'd away from
  // before we had a chance to get started, we have nothing to do.
  if (this.started_) {

    // If we were started, we must be the current problem, so these things
    // must be showing
    this.removeWarningInUrlbar_();
    this.unhideLockIcon_();

    // Could be though that they've closed the warning dialog
    if (this.messageShowing_)
      this.hideMessage_();

    if (this.resizeHandler_) {
      this.doc_.defaultView.removeEventListener("resize", 
                                                this.resizeHandler_, 
                                                true);
      this.resizeHandler_ = null;
    }
    
    var win = this.doc_.defaultView;
    win.controllers.removeController(this.commandController_);
    this.commandController_ = null;
  }
}

/**
 * Helper function to remove a substring from inside a string.
 *
 * @param orig String to remove substring from
 * 
 * @param toRemove String to remove (if it is present)
 *
 * @returns String with the substring removed
 */
PROT_PhishMsgDisplayerBase.prototype.removeIfExists_ = function(orig,
                                                                toRemove) {
  var pos = orig.indexOf(toRemove);
  if (pos != -1)
    orig = orig.substring(0, pos) + orig.substring(pos + toRemove.length);

  return orig;
}

/**
 * Our message is displayed relative to an icon in the urlbar. However
 * it could be the case that the urlbar isn't showing, in which case
 * we would show the warning message some place crazy, such as
 * offscreen. So here we put it back if it's not there. This is a
 * good thing anyway: the user should be able to see the URL of the
 * site if we think it is suspicious.
 */
PROT_PhishMsgDisplayerBase.prototype.ensureNavBarShowing_ = function() {
  // Could be that the navbar was unselected in the Toolbars menu; fix it
  var navbar = this.doc_.getElementById("nav-bar");
  navbar.setAttribute("collapsed", false);

  // Now for the more complicated case: a popup window with toolbar=no. This
  // adds toolbar and location (among other things) to the chromhidden 
  // attribute on the window. We need to undo this, and manually fill in the
  // location.
  var win = this.doc_.getElementById("main-window");
  var hidden = win.getAttribute("chromehidden");
  G_Debug(this, "Old chromehidden string: " + hidden);

  var newHidden = this.removeIfExists_(hidden, "toolbar");
  newHidden = this.removeIfExists_(newHidden, "location");

  if (newHidden != hidden) {
    G_Debug(this, "New chromehidden string: " + newHidden);
    win.setAttribute("chromehidden", newHidden);

    // Now manually reflect the location
    var urlbar = this.doc_.getElementById("urlbar");
    urlbar.value = this.doc_.getElementById("content")
                   .contentDocument.location.href;
  }
}

/**
 * We don't want to confuse users if they land on a phishy page that uses
 * SSL, so ensure that the lock icon never shows when we're showing our 
 * warning.
 */
PROT_PhishMsgDisplayerBase.prototype.hideLockIcon_ = function() {
  var lockIcon = this.doc_.getElementById("lock-icon");
  lockIcon.hidden = true;
}

/**
 * Ensure they can see it after our warning is finished.
 */
PROT_PhishMsgDisplayerBase.prototype.unhideLockIcon_ = function() {
  var lockIcon = this.doc_.getElementById("lock-icon");
  lockIcon.hidden = false;
}

/**
 * This method makes our warning icon visible in the location bar. It will
 * be removed only when the problematic document is navigated awy from 
 * (i.e., when done() is called), and not when the warning is dismissed.
 */
PROT_PhishMsgDisplayerBase.prototype.addWarningInUrlbar_ = function() {
  var urlbarIcon = this.doc_.getElementById(this.urlbarIconId_);
  urlbarIcon.style.visibility = "visible";
}

/**
 * Hides our urlbar icon
 */
PROT_PhishMsgDisplayerBase.prototype.removeWarningInUrlbar_ = function() {
  var urlbarIcon = this.doc_.getElementById(this.urlbarIconId_);
  urlbarIcon.style.visibility = "collapse";
}

/**
 * VIRTUAL -- Displays the warning message
 */
PROT_PhishMsgDisplayerBase.prototype.showMessage_ = function() { };

/**
 * VIRTUAL -- Hide the warning message from the user.
 */
PROT_PhishMsgDisplayerBase.prototype.hideMessage_ = function() { };

/**
 * Reposition the message relative to refElement in the parent window
 *
 * @param message Reference to the element to position
 * @param tail Reference to the message tail
 * @param refElement Reference to element relative to which we position
 *                   ourselves
 */
PROT_PhishMsgDisplayerBase.prototype.adjustLocation_ = function(message,
                                                                tail,
                                                                refElement) {

  var refX = refElement.boxObject.x;
  var refY = refElement.boxObject.y;
  var refHeight = refElement.boxObject.height;
  var refWidth = refElement.boxObject.width;
  G_Debug(this, "Ref element is at [window-relative] (" + refX + ", " + 
          refY + ")");

  var pixelsIntoRefY = -2;
  var tailY = refY + refHeight - pixelsIntoRefY;
  var tailPixelsLeftOfRefX = tail.boxObject.width;
  var tailPixelsIntoRefX = Math.round(refWidth / 2);
  var tailX = refX - tailPixelsLeftOfRefX + tailPixelsIntoRefX;

  var messageY = tailY + tail.boxObject.height + 3;
  var messagePixelsLeftOfRefX = 375;
  var messageX = refX - messagePixelsLeftOfRefX;
  G_Debug(this, "Message is at [window-relative] (" + messageX + ", " + 
          messageY + ")");
  G_Debug(this, "Tail is at [window-relative] (" + tailX + ", " + 
          tailY + ")");

  tail.style.top = tailY + "px";
  tail.style.left = tailX + "px";
  message.style.top = messageY + "px";
  message.style.left = messageX + "px";
}

/**
 * Show the extended warning message
 */
PROT_PhishMsgDisplayerBase.prototype.showMore_ = function() {
  this.doc_.getElementById(this.extendedMessageId_).hidden = false;
  this.doc_.getElementById(this.showmoreLinkId_).style.display = "none";
}

/**
 * The user clicked on one of the links in the buble.  Display the
 * corresponding page in a new window with all the chrome enabled.
 *
 * @param url The URL to display in a new window
 */
PROT_PhishMsgDisplayerBase.prototype.showURL_ = function(url) {
  this.windowWatcher_.openWindow(this.windowWatcher_.activeWindow,
                                 url,
                                 "_blank",
                                 null,
                                 null);
}


/**
 * A specific implementation of the dislpayer using a canvas. This
 * class is meant for use on platforms that don't support transparent
 * elements over browser content (currently: all platforms). 
 *
 * The main ugliness is the fact that we're hiding the content area and
 * painting the page to canvas. As a result, we must periodically
 * re-paint the canvas to reflect updates to the page. Otherwise if
 * the page was half-loaded when we showed our warning, it would
 * stay that way even though the page actually finished loading. 
 *
 * See base constructor for full details of constructor args.
 *
 * @constructor
 */
function PROT_PhishMsgDisplayerCanvas(msgDesc, browser, doc, url) {
  PROT_PhishMsgDisplayerBase.call(this, msgDesc, browser, doc, url);

  this.dimAreaId_ = "safebrowsing-dim-area-canvas";
  this.contentStackId_ = "safebrowsing-content-stack";
  this.pageCanvasId_ = "safebrowsing-page-canvas";
  this.xhtmlNS_ = "http://www.w3.org/1999/xhtml";     // we create html:canvas
}

PROT_PhishMsgDisplayerCanvas.inherits(PROT_PhishMsgDisplayerBase);

/**
 * Displays the warning message.
 */
PROT_PhishMsgDisplayerCanvas.prototype.showMessage_ = function() {

  G_Debug(this, "Showing message.");
  this.messageShowing_ = true;

  // Unhide our stack. Order here is significant, but don't ask me why
  // for some of these. You need to:
  // 1. add canvas to the stack in a hidden state
  // 2. unhide the stack 
  // 3. get browser dimensions
  // 4. unhide stack contents
  // 5. display to the canvas
  // 6. unhide the warning message

  // (1)
  // We add the canvas dynamically and remove it when we're done because
  // leaving it hanging around consumes a lot of memory.
  var pageCanvas = this.doc_.createElementNS(this.xhtmlNS_, "html:canvas");
  pageCanvas.id = this.pageCanvasId_;
  pageCanvas.hidden = "true";
  var contentStack = this.doc_.getElementById(this.contentStackId_);
  contentStack.insertBefore(pageCanvas, contentStack.firstChild);

  // (2)
  contentStack.hidden = false;

  // (3)
  var w = this.browser_.boxObject.width;     
  G_Debug(this, "browser w=" + w);
  var h = this.browser_.boxObject.height;
  G_Debug(this, "browser h=" + h);
  
  var win = this.browser_.contentWindow;
  var scrollX = win.scrollX;
  G_Debug(this, "win scrollx=" + scrollX);
  var scrollY = win.scrollY;
  G_Debug(this, "win scrolly=" + scrollY);

  // (4) 
  var dimarea = this.doc_.getElementById(this.dimAreaId_);
  dimarea.hidden = false;

  this.browser_.parentNode.collapsed = true;   // And now hide the browser

  // (5) 
  pageCanvas.setAttribute("width", w);
  pageCanvas.setAttribute("height", h);

  var bgcolor = this.getBackgroundColor_();

  var cx = pageCanvas.getContext("2d");
  cx.drawWindow(win, scrollX, scrollY, w, h, bgcolor);

  // Now repaint the window every so often in case the content hasn't fully 
  // loaded at this point.
  var debZone = this.debugZone;
  function repaint() {
    G_Debug(debZone, "Repainting canvas...");
    cx.drawWindow(win, scrollX, scrollY, w, h, bgcolor);
  };
  this.repainter_ = new PROT_PhishMsgCanvasRepainter(repaint);

  // (6)
  var refElement = this.doc_.getElementById(this.refElementId_);
  var message = this.doc_.getElementById(this.messageId_);
  var tail = this.doc_.getElementById(this.messageTailId_);
  message.hidden = false;
  message.style.display = "block";
  tail.hidden = false;
  tail.style.display = "block";
  this.adjustLocation_(message, tail, refElement);
}

/**
 * Hide the warning message from the user.
 */
PROT_PhishMsgDisplayerCanvas.prototype.hideMessage_ = function() {
  G_Debug(this, "Hiding phishing warning.");
  G_Assert(this, this.messageShowing_, "Hide message called but not showing?");

  this.messageShowing_ = false;
  this.repainter_.cancel();
  this.repainter_ = null;

  var message = this.doc_.getElementById(this.messageId_);
  message.hidden = true;
  message.style.display = "none";

  var tail = this.doc_.getElementById(this.messageTailId_);
  tail.hidden = true;
  tail.style.display = "none";
  this.browser_.parentNode.collapsed = false;

  var contentStack = this.doc_.getElementById(this.contentStackId_);
  contentStack.hidden = true;

  var dimarea = this.doc_.getElementById(this.dimAreaId_);
  dimarea.hidden = true;

  var pageCanvas = this.doc_.getElementById(this.pageCanvasId_);
  contentStack.removeChild(pageCanvas);
}


/**
 * Helper class that periodically repaints the canvas. We repaint
 * frequently at first, and then back off to a less frequent schedule
 * at "steady state," and finally just stop altogether. We have to do
 * this because we're not sure if the page has finished loading when
 * we first paint the canvas, and because we want to reflect any
 * dynamically written content into the canvas as it appears in the
 * page after load.
 *
 * @param repaintFunc Function to call to repaint browser.
 *
 * @constructor
 */
function PROT_PhishMsgCanvasRepainter(repaintFunc) {
  this.count_ = 0;
  this.repaintFunc_ = repaintFunc;
  this.initPeriodMS_ = 500;             // Initially repaint every 500ms
  this.steadyStateAtMS_ = 10 * 1000;    // Go slowly after 10 seconds,
  this.steadyStatePeriodMS_ = 3 * 1000; // repainting every 3 seconds, and
  this.quitAtMS_ = 20 * 1000;           // stop after 20 seconds
  this.startMS_ = (new Date).getTime();
  this.alarm_ = new G_Alarm(BindToObject(this.repaint, this), 
                            this.initPeriodMS_);
}

/**
 * Called periodically to repaint the canvas
 */
PROT_PhishMsgCanvasRepainter.prototype.repaint = function() {
  this.repaintFunc_();

  var nextRepaint;
  // If we're in "steady state", use the slow repaint rate, else fast
  if ((new Date).getTime() - this.startMS_ > this.steadyStateAtMS_)
    nextRepaint = this.steadyStatePeriodMS_;
  else 
    nextRepaint = this.initPeriodMS_;

  if (!((new Date).getTime() - this.startMS_ > this.quitAtMS_))
    this.alarm_ = new G_Alarm(BindToObject(this.repaint, this), nextRepaint);
}

/**
 * Called to stop repainting the canvas
 */
PROT_PhishMsgCanvasRepainter.prototype.cancel = function() {
  if (this.alarm_) {
    this.alarm_.cancel();
    this.alarm_ = null;
  }
  this.repaintFunc_ = null;
}
