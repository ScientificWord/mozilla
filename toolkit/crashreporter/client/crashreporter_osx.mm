/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * The Original Code is Mozilla Toolkit Crash Reporter
 *
 * The Initial Developer of the Original Code is
 *   Mozilla Corporation
 * Portions created by the Initial Developer are Copyright (C) 2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Dave Camp <dcamp@mozilla.com>
 *   Ted Mielczarek <ted.mielczarek@gmail.com>
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

#import <Cocoa/Cocoa.h>
#import <CoreFoundation/CoreFoundation.h>
#include "crashreporter.h"
#include "crashreporter_osx.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sstream>

using std::string;
using std::vector;
using std::ostringstream;

using namespace CrashReporter;

static NSAutoreleasePool* gMainPool;
static CrashReporterUI* gUI = 0;
static string gDumpFile;
static StringTable gQueryParameters;
static string gURLParameter;
static string gSendURL;
static vector<string> gRestartArgs;
static bool gDidTrySend = false;

#define NSSTR(s) [NSString stringWithUTF8String:(s).c_str()]

static NSString* Str(const char* aName)
{
  string str = gStrings[aName];
  if (str.empty()) str = "?";
  return NSSTR(str);
}

static bool RestartApplication()
{
  char** argv = reinterpret_cast<char**>(
    malloc(sizeof(char*) * (gRestartArgs.size() + 1)));

  if (!argv) return false;

  unsigned int i;
  for (i = 0; i < gRestartArgs.size(); i++) {
    argv[i] = (char*)gRestartArgs[i].c_str();
  }
  argv[i] = 0;

  pid_t pid = fork();
  if (pid == -1)
    return false;
  else if (pid == 0) {
    (void)execv(argv[0], argv);
    _exit(1);
  }

  free(argv);

  return true;
}

@implementation CrashReporterUI

-(void)awakeFromNib
{
  gUI = self;
  [mWindow center];

  [mWindow setTitle:[[NSBundle mainBundle]
                      objectForInfoDictionaryKey:@"CFBundleName"]];
}

-(void)showCrashUI:(const string&)dumpfile
   queryParameters:(const StringTable&)queryParameters
           sendURL:(const string&)sendURL
{
  gDumpFile = dumpfile;
  gQueryParameters = queryParameters;
  gSendURL = sendURL;

  [mWindow setTitle:Str(ST_CRASHREPORTERTITLE)];
  [mHeaderLabel setStringValue:Str(ST_CRASHREPORTERHEADER)];

  [mViewReportButton setTitle:Str(ST_VIEWREPORT)];
  [mViewReportButton sizeToFit];
  [mSubmitReportButton setTitle:Str(ST_CHECKSUBMIT)];
  [mIncludeURLButton setTitle:Str(ST_CHECKURL)];
  [mEmailMeButton setTitle:Str(ST_CHECKEMAIL)];
  [mViewReportOkButton setTitle:Str(ST_OK)];

  [mCommentText setPlaceholder:Str(ST_COMMENTGRAYTEXT)];
  [[mEmailText cell] setPlaceholderString:Str(ST_EMAILGRAYTEXT)];

  if (gQueryParameters.find("URL") != gQueryParameters.end()) {
    // save the URL value in case the checkbox gets unchecked
    gURLParameter = gQueryParameters["URL"];
  }
  else {
    // no URL specified, hide checkbox
    [mIncludeURLButton removeFromSuperview];
    // shrink window to fit
    NSRect frame = [mWindow frame];
    NSRect includeURLFrame = [mIncludeURLButton frame];
    NSRect emailFrame = [mEmailMeButton frame];
    int buttonMask = [mViewReportButton autoresizingMask];
    int checkMask = [mSubmitReportButton autoresizingMask];
    int commentScrollMask = [mCommentScrollView autoresizingMask];

    [mViewReportButton setAutoresizingMask:NSViewMinYMargin];
    [mSubmitReportButton setAutoresizingMask:NSViewMinYMargin];
    [mCommentScrollView setAutoresizingMask:NSViewMinYMargin];

    // remove all the space in between
    frame.size.height -= includeURLFrame.origin.y - emailFrame.origin.y;
    [mWindow setFrame:frame display: true animate:NO];

    [mViewReportButton setAutoresizingMask:buttonMask];
    [mSubmitReportButton setAutoresizingMask:checkMask];
    [mCommentScrollView setAutoresizingMask:commentScrollMask];
  }

  // resize some buttons horizontally and possibly some controls vertically
  [self doInitialResizing];

  [self updateSubmit];
  [self updateURL];
  [self updateEmail];

  [mWindow makeKeyAndOrderFront:nil];
}

-(void)showErrorUI:(const string&)message
{
  [self setView: mErrorView animate: NO];

  [mErrorHeaderLabel setStringValue:Str(ST_CRASHREPORTERHEADER)];
  [self setStringFitVertically:mErrorLabel
                        string:NSSTR(message)
                  resizeWindow:YES];
  [mErrorCloseButton setTitle:Str(ST_OK)];

  [mErrorCloseButton setKeyEquivalent:@"\r"];
  [mWindow makeFirstResponder:mErrorCloseButton];
  [mWindow makeKeyAndOrderFront:nil];
}

-(void)showReportInfo
{
  NSDictionary* boldAttr = [NSDictionary
                            dictionaryWithObject:
                            [NSFont boldSystemFontOfSize:
                             [NSFont smallSystemFontSize]]
                                          forKey:NSFontAttributeName];
  NSDictionary* normalAttr = [NSDictionary
                              dictionaryWithObject:
                              [NSFont systemFontOfSize:
                               [NSFont smallSystemFontSize]]
                                            forKey:NSFontAttributeName];

  [mViewReportTextView setString:@""];
  for (StringTable::iterator iter = gQueryParameters.begin();
       iter != gQueryParameters.end();
       iter++) {
    NSAttributedString* key = [[NSAttributedString alloc]
                               initWithString:NSSTR(iter->first + ": ")
                               attributes:boldAttr];
    NSAttributedString* value = [[NSAttributedString alloc]
                                 initWithString:NSSTR(iter->second + "\n")
                                 attributes:normalAttr];
    [[mViewReportTextView textStorage] appendAttributedString: key];
    [[mViewReportTextView textStorage] appendAttributedString: value];
    [key release];
    [value release];
  }

  NSAttributedString* extra = [[NSAttributedString alloc]
                               initWithString:NSSTR("\n" + gStrings[ST_EXTRAREPORTINFO])
                               attributes:normalAttr];
  [[mViewReportTextView textStorage] appendAttributedString: extra];
  [extra release];
}

-(IBAction)submitReportClicked:(id)sender
{
  [self updateSubmit];
}

-(IBAction)viewReportClicked:(id)sender
{
  [self showReportInfo];
  [NSApp beginSheet:mViewReportWindow modalForWindow:mWindow
   modalDelegate:nil didEndSelector:nil contextInfo:nil];
}

- (IBAction)viewReportOkClicked:(id)sender;
{
  [mViewReportWindow orderOut:nil];
  [NSApp endSheet:mViewReportWindow];
}

-(IBAction)closeClicked:(id)sender
{
  [NSApp terminate: self];
}

-(IBAction)restartClicked:(id)sender
{
  RestartApplication();

  if ([mSubmitReportButton state] == NSOnState) {
    [self setStringFitVertically:mProgressText
                          string:Str(ST_REPORTDURINGSUBMIT)
                    resizeWindow:YES];
    // disable all the controls
    [self enableControls:NO];
    [mSubmitReportButton setEnabled:NO];
    [mRestartButton setEnabled:NO];
    [mCloseButton setEnabled:NO];
    [mProgressIndicator startAnimation:self];
    gDidTrySend = true;
    [self sendReport];
  } else {
    [NSApp terminate:self];
  }
}

- (IBAction)includeURLClicked:(id)sender
{
  [self updateURL];
}

-(IBAction)emailMeClicked:(id)sender
{
  [self updateEmail];
}

-(void)controlTextDidChange:(NSNotification *)note
{
  [self updateEmail];
}

- (void)textDidChange:(NSNotification *)aNotification
{
  // update comment parameter
  if ([[[mCommentText textStorage] mutableString] length] > 0)
    gQueryParameters["Comments"] = [[[mCommentText textStorage] mutableString]
                                    UTF8String];
  else
    gQueryParameters.erase("Comments");
}

// Limit the comment field to 500 bytes in UTF-8
- (BOOL)textView:(NSTextView *)aTextView shouldChangeTextInRange:(NSRange)affectedCharRange replacementString:(NSString *)replacementString
{
  // current string length + replacement text length - replaced range length
  if (([[aTextView string]
        lengthOfBytesUsingEncoding:NSUTF8StringEncoding]
	   + [replacementString lengthOfBytesUsingEncoding:NSUTF8StringEncoding]
	   - [[[aTextView string] substringWithRange:affectedCharRange]
        lengthOfBytesUsingEncoding:NSUTF8StringEncoding])
	    > MAX_COMMENT_LENGTH) {
    return NO;
  }
  return YES;
}

- (void)doInitialResizing
{
  NSRect windowFrame = [mWindow frame];
  NSRect restartFrame = [mRestartButton frame];
  NSRect closeFrame = [mCloseButton frame];
  // resize close button to fit text
  float oldCloseWidth = closeFrame.size.width;
  [mCloseButton setTitle:Str(ST_QUIT)];
  [mCloseButton sizeToFit];
  closeFrame = [mCloseButton frame];
  // move close button left if it grew
  closeFrame.origin.x -= closeFrame.size.width - oldCloseWidth;

  if (gRestartArgs.size() == 0) {
    [mRestartButton removeFromSuperview];
    closeFrame.origin.x = restartFrame.origin.x +
      (restartFrame.size.width - closeFrame.size.width);
    [mCloseButton setFrame: closeFrame];
    [mCloseButton setKeyEquivalent:@"\r"];
  } else {
    [mRestartButton setTitle:Str(ST_RESTART)];
    // resize "restart" button
    float oldRestartWidth = restartFrame.size.width;
    [mRestartButton sizeToFit];
    restartFrame = [mRestartButton frame];
    // move left by the amount that the button grew
    restartFrame.origin.x -= restartFrame.size.width - oldRestartWidth;
    closeFrame.origin.x -= restartFrame.size.width - oldRestartWidth;
    [mRestartButton setFrame: restartFrame];
    [mCloseButton setFrame: closeFrame];
    // possibly resize window if both buttons no longer fit
    // leave 20 px from either side of the window, and 12 px
    // between the buttons
    float neededWidth = closeFrame.size.width + restartFrame.size.width +
                        2*20 + 12;
    
    if (neededWidth > windowFrame.size.width) {
      windowFrame.size.width = neededWidth;
      [mWindow setFrame:windowFrame display: true animate: NO];
    }
    [mRestartButton setKeyEquivalent:@"\r"];
  }

  NSButton *checkboxes[] = {
    mSubmitReportButton,
    mIncludeURLButton,
    mEmailMeButton
  };

  for (int i=0; i<3; i++) {
    [checkboxes[i] sizeToFit];
    // keep existing spacing on left side, + 20 px spare on right
    NSRect frame = [checkboxes[i] frame];
    float neededWidth = frame.origin.x + frame.size.width + 20;
    if (neededWidth > windowFrame.size.width) {
      windowFrame.size.width = neededWidth;
      [mWindow setFrame:windowFrame display: true animate: NO];
    }
  }

  // do this down here because we may have made the window wider
  // up above
  [self setStringFitVertically:mDescriptionLabel
                        string:Str(ST_CRASHREPORTERDESCRIPTION)
                  resizeWindow:YES];

  // now pin all the controls (except quit/submit) in place,
  // if we lengthen the window after this, it's just to lengthen
  // the progress text, so nothing above that text should move.
  NSView* views[] = {
    mSubmitReportButton,
    mViewReportButton,
    mCommentScrollView,
    mIncludeURLButton,
    mEmailMeButton,
    mEmailText,
    mProgressIndicator,
    mProgressText
  };
  for (unsigned int i=0; i<sizeof(views)/sizeof(views[0]); i++) {
    [views[i] setAutoresizingMask:NSViewMinYMargin];
  }
}

-(float)setStringFitVertically:(NSControl*)control
                        string:(NSString*)str
                  resizeWindow:(BOOL)resizeWindow
{
  // hack to make the text field grow vertically
  NSRect frame = [control frame];
  float oldHeight = frame.size.height;

  frame.size.height = 10000;
  NSSize oldCellSize = [[control cell] cellSizeForBounds: frame];
  [control setStringValue: str];
  NSSize newCellSize = [[control cell] cellSizeForBounds: frame];

  float delta = newCellSize.height - oldCellSize.height;
  frame.origin.y -= delta;
  frame.size.height = oldHeight + delta;
  [control setFrame: frame];

  if (resizeWindow) {
    NSRect frame = [mWindow frame];
    frame.origin.y -= delta;
    frame.size.height += delta;
    [mWindow setFrame:frame display: true animate: NO];
  }

  return delta;
}

-(void)setView: (NSView*)v animate: (BOOL)animate
{
  NSRect frame = [mWindow frame];

  NSRect oldViewFrame = [[mWindow contentView] frame];
  NSRect newViewFrame = [v frame];

  frame.origin.y += oldViewFrame.size.height - newViewFrame.size.height;
  frame.size.height -= oldViewFrame.size.height - newViewFrame.size.height;

  frame.origin.x += oldViewFrame.size.width - newViewFrame.size.width;
  frame.size.width -= oldViewFrame.size.width - newViewFrame.size.width;

  [mWindow setContentView:v];
  [mWindow setFrame:frame display:true animate:animate];
}

- (void)enableControls:(BOOL)enabled
{
  [mViewReportButton setEnabled:enabled];
  [mIncludeURLButton setEnabled:enabled];
  [mEmailMeButton setEnabled:enabled];
  [mCommentText setEnabled:enabled];
  [mCommentScrollView setHasVerticalScroller:enabled];
  [self updateEmail];
}

-(void)updateSubmit
{
  if ([mSubmitReportButton state] == NSOnState) {
    [self setStringFitVertically:mProgressText
                          string:Str(ST_REPORTPRESUBMIT)
                    resizeWindow:YES];
    [mProgressText setHidden:NO];
    // enable all the controls
    [self enableControls:YES];
  }
  else {
    // not submitting, disable all the controls under
    // the submit checkbox, and hide the status text
    [mProgressText setHidden:YES];
    [self enableControls:NO];
  }
}

-(void)updateURL
{
  if ([mIncludeURLButton state] == NSOnState && !gURLParameter.empty()) {
    gQueryParameters["URL"] = gURLParameter;
  } else {
    gQueryParameters.erase("URL");
  }
}

-(void)updateEmail
{
  if ([mEmailMeButton state] == NSOnState &&
      [mSubmitReportButton state] == NSOnState) {
    NSString* email = [mEmailText stringValue];
    gQueryParameters["Email"] = [email UTF8String];
    [mEmailText setEnabled:YES];
  } else {
    gQueryParameters.erase("Email");
    [mEmailText setEnabled:NO];
  }
}

-(void)sendReport
{
  if (![self setupPost]) {
    LogMessage("Crash report submission failed: could not set up POST data");
   [self setStringFitVertically:mProgressText
                          string:Str(ST_SUBMITFAILED)
                    resizeWindow:YES];
   // quit after 5 seconds
   [self performSelector:@selector(closeClicked:) withObject:self
    afterDelay:5.0];
  }

  [NSThread detachNewThreadSelector:@selector(uploadThread:)
            toTarget:self
            withObject:mPost];
}

-(bool)setupPost
{
  NSURL* url = [NSURL URLWithString:NSSTR(gSendURL)];
  if (!url) return false;

  mPost = [[HTTPMultipartUpload alloc] initWithURL: url];
  if (!mPost) return false;

  NSMutableDictionary* parameters =
    [[NSMutableDictionary alloc] initWithCapacity: gQueryParameters.size()];
  if (!parameters) return false;

  StringTable::const_iterator end = gQueryParameters.end();
  for (StringTable::const_iterator i = gQueryParameters.begin();
       i != end;
       i++) {
    NSString* key = NSSTR(i->first);
    NSString* value = NSSTR(i->second);
    [parameters setObject: value forKey: key];
  }

  [mPost addFileAtPath: NSSTR(gDumpFile) name: @"upload_file_minidump"];
  [mPost setParameters: parameters];
  [parameters release];

  return true;
}

-(void)uploadComplete:(NSData*)data
{
  NSHTTPURLResponse* response = [mPost response];
  [mPost release];

  bool success;
  string reply;
  if (!data || !response || [response statusCode] != 200) {
    success = false;
    reply = "";

    // if data is nil, we probably logged an error in uploadThread
    if (data != nil && response != nil) {
      ostringstream message;
      message << "Crash report submission failed: server returned status "
              << [response statusCode];
      LogMessage(message.str());
    }
  } else {
    success = true;
    LogMessage("Crash report submitted successfully");

    NSString* encodingName = [response textEncodingName];
    NSStringEncoding encoding;
    if (encodingName) {
      encoding = CFStringConvertEncodingToNSStringEncoding(
        CFStringConvertIANACharSetNameToEncoding((CFStringRef)encodingName));
    } else {
      encoding = NSISOLatin1StringEncoding;
    }
    NSString* r = [[NSString alloc] initWithData: data encoding: encoding];
    reply = [r UTF8String];
    [r release];
  }

  SendCompleted(success, reply);

  [mProgressIndicator stopAnimation:self];
  if (success) {
   [self setStringFitVertically:mProgressText
                          string:Str(ST_REPORTSUBMITSUCCESS)
                    resizeWindow:YES];
  } else {
   [self setStringFitVertically:mProgressText
                          string:Str(ST_SUBMITFAILED)
                    resizeWindow:YES];
  }
  // quit after 5 seconds
  [self performSelector:@selector(closeClicked:) withObject:self
   afterDelay:5.0];
}

-(void)uploadThread:(HTTPMultipartUpload*)post
{
  NSAutoreleasePool* autoreleasepool = [[NSAutoreleasePool alloc] init];
  NSError* error = nil;
  NSData* data = [post send: &error];
  if (error) {
    data = nil;
    NSString* errorDesc = [error localizedDescription];
    string message = [errorDesc UTF8String];
    LogMessage("Crash report submission failed: " + message);
  }

  [self performSelectorOnMainThread: @selector(uploadComplete:)
        withObject: data
        waitUntilDone: YES];

  [autoreleasepool release];
}

// to get auto-quit when we close the window
-(BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)theApplication
{
  return YES;
}

-(void)applicationWillTerminate:(NSNotification *)aNotification
{
  // since we use [NSApp terminate:] we never return to main,
  // so do our cleanup here
  if (!gDidTrySend)
    DeleteDump();
}

@end

@implementation TextViewWithPlaceHolder

- (BOOL)becomeFirstResponder
{
  [self setNeedsDisplay:YES];
  return [super becomeFirstResponder];
}

- (void)drawRect:(NSRect)rect
{
  [super drawRect:rect];
  if (mPlaceHolderString && [[self string] isEqualToString:@""] &&
      self != [[self window] firstResponder])
    [mPlaceHolderString drawAtPoint:NSMakePoint(0,0)];
}

- (BOOL)resignFirstResponder
{
  [self setNeedsDisplay:YES];
  return [super resignFirstResponder];
}

- (void)setPlaceholder:(NSString*)placeholder
{
	NSColor* txtColor = [NSColor disabledControlTextColor];
  NSDictionary* txtDict = [NSDictionary
                           dictionaryWithObjectsAndKeys:txtColor,
                           NSForegroundColorAttributeName, nil];
  mPlaceHolderString = [[NSAttributedString alloc]
                       initWithString:placeholder attributes:txtDict];
}

- (void)insertTab:(id)sender
{
  // don't actually want to insert tabs, just tab to next control
  [[self window] selectNextKeyView:sender];
}

- (void)setEnabled:(BOOL)enabled
{
  [self setSelectable:enabled];
  [self setEditable:enabled];
  if (![[self string] isEqualToString:@""]) {
    NSAttributedString* colorString;
    NSColor* txtColor;
    if (enabled)
      txtColor = [NSColor textColor];
    else
      txtColor = [NSColor disabledControlTextColor];
    NSDictionary *txtDict = [NSDictionary
                             dictionaryWithObjectsAndKeys:txtColor,
                             NSForegroundColorAttributeName, nil];
    colorString = [[NSAttributedString alloc]
                   initWithString:[self string]
                   attributes:txtDict];
    [[self textStorage] setAttributedString: colorString];
    [self setInsertionPointColor:txtColor];
    [colorString release];
  }
}

- (void)dealloc
{
  [mPlaceHolderString release];
  [super dealloc];
}

@end

/* === Crashreporter UI Functions === */

bool UIInit()
{
  gMainPool = [[NSAutoreleasePool alloc] init];
  [NSApplication sharedApplication];
  [NSBundle loadNibNamed:@"MainMenu" owner:NSApp];

  return true;
}

void UIShutdown()
{
  [gMainPool release];
}

void UIShowDefaultUI()
{
  [gUI showErrorUI: gStrings[ST_CRASHREPORTERDEFAULT]];
  [NSApp run];
}

bool UIShowCrashUI(const string& dumpfile,
                   const StringTable& queryParameters,
                   const string& sendURL,
                   const vector<string>& restartArgs)
{
  gRestartArgs = restartArgs;

  [gUI showCrashUI: dumpfile
       queryParameters: queryParameters
       sendURL: sendURL];
  [NSApp run];

  return gDidTrySend;
}

void UIError_impl(const string& message)
{
  if (!gUI) {
    // UI failed to initialize, printing is the best we can do
    printf("Error: %s\n", message.c_str());
    return;
  }

  [gUI showErrorUI: message];
  [NSApp run];
}

bool UIGetIniPath(string& path)
{
  path = gArgv[0];
  path.append(".ini");

  return true;
}

bool UIGetSettingsPath(const string& vendor,
                       const string& product,
                       string& settingsPath)
{
  FSRef foundRef;
  OSErr err = FSFindFolder(kUserDomain, kApplicationSupportFolderType,
                           kCreateFolder, &foundRef);
  if (err != noErr)
    return false;

  unsigned char path[PATH_MAX];
  FSRefMakePath(&foundRef, path, sizeof(path));
  NSString* destPath = [NSString stringWithUTF8String:reinterpret_cast<char*>(path)];

  // Note that MacOS ignores the vendor when creating the profile hierarchy -
  // all application preferences directories live alongside one another in
  // ~/Library/Application Support/
  destPath = [destPath stringByAppendingPathComponent: NSSTR(product)];
  // Thunderbird stores its profile in ~/Library/Thunderbird,
  // but we're going to put stuff in ~/Library/Application Support/Thunderbird
  // anyway, so we have to ensure that path exists.
  string tempPath = [destPath UTF8String];
  if (!UIEnsurePathExists(tempPath))
    return false;

  destPath = [destPath stringByAppendingPathComponent: @"Crash Reports"];

  settingsPath = [destPath UTF8String];

  return true;
}

bool UIEnsurePathExists(const string& path)
{
  int ret = mkdir(path.c_str(), S_IRWXU);
  int e = errno;
  if (ret == -1 && e != EEXIST)
    return false;

  return true;
}

bool UIFileExists(const string& path)
{
  struct stat sb;
  int ret = stat(path.c_str(), &sb);
  if (ret == -1 || !(sb.st_mode & S_IFREG))
    return false;

  return true;
}

bool UIMoveFile(const string& file, const string& newfile)
{
  return (rename(file.c_str(), newfile.c_str()) != -1);
}

bool UIDeleteFile(const string& file)
{
  return (unlink(file.c_str()) != -1);
}

std::ifstream* UIOpenRead(const string& filename)
{
  return new std::ifstream(filename.c_str(), std::ios::in);
}

std::ofstream* UIOpenWrite(const string& filename, bool append) // append=false
{
  return new std::ofstream(filename.c_str(),
                           append ? std::ios::out | std::ios::app
                                  : std::ios::out);
}
