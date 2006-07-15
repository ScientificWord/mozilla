/* -*- Mode: Java; tab-width: 4; c-basic-offset: 4; -*-
 * 
 * ***** BEGIN LICENSE BLOCK *****
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
 * The Original Code is mozilla.org Code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Ben "Count XULula" Goodger <rgoodger@ihug.co.nz>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
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

/** class WizardManager( string frame_id, string tagURLPrefix, 
 *                       string tagURLPostfix, object wizardMap ) ;
 *  purpose: class for managing the state of a generic wizard
 *  in:  string frame_id content frame id/name 
 *       string tagURLPrefix prefix root of wizard pages
 *       string tagURLPostfix extension suffix of wizard pages (e.g. ".xul")
 *       object wizardMap navigation map object
 *  out: nothing.
 **/
function WizardManager( frame_id, tagURLPrefix, tagURLPostfix, wizardMap )
{
  // current page
  this.currentPageTag       = null;
  // data grid of navigable pages
  this.wizardMap            = ( wizardMap ) ? wizardMap : null;
  // current page
  this.currentPageNumber    = 0;
  // flag to signify no page-change occurred.
  this.firstTime            = true;   // was false, let's see what this does. o_O
  // frame which holds data
  this.content_frame        = document.getElementById( frame_id );
  // wizard state manager 
  this.WSM                  = new WidgetStateManager( frame_id );
  // wizard button handler set
  this.WHANDLER             = new WizardHandlerSet( this.WSM, this );
  
  // url handling
  this.URL_PagePrefix       = ( tagURLPrefix  ) ? tagURLPrefix : null;
  this.URL_PagePostfix      = ( tagURLPostfix ) ? tagURLPrefix : null; 

  // string bundle  
  this.bundle               = srGetStrBundle("chrome://global/locale/wizardManager.properties");

  this.LoadPage             = WM_LoadPage;
  this.GetURLFromTag        = WM_GetURLFromTag;
  this.GetTagFromURL        = WM_GetTagFromURL;
  this.SetHandlers          = WM_SetHandlers;
  this.SetPageData          = WM_SetPageData;
  this.SavePageData         = WM_SavePageData;
  this.ProgressUpdate       = WM_ProgressUpdate;
  this.GetMapLength         = WM_GetMapLength;

  // set up handlers from wizard overlay
  // #include chrome://global/content/wizardOverlay.js
  doSetWizardButtons( this );
}

/** void LoadPage( string page ) ;
 *  purpose: loads a page into the content frame
 *  in:  string page tag referring to the complete file name of the current page
 *       string frame_id optional supply of page frame, if content_frame is not 
 *                       defined
 *  out: boolean success indicator.
 **/
function WM_LoadPage( pageURL, absolute )
{
	if( pageURL != "" )
	{
    if ( this.firstTime && !absolute )
      this.ProgressUpdate( this.currentPageNumber );

    // 1.1: REMOVED to fix no-field-page-JS error bug. reintroduce if needed.
    // if ( !this.firstTime )
    //   this.WSM.SavePageData( this.content_frame );

    // build a url from a tag, or use an absolute url
    if( !absolute ) {
            var src = this.GetURLFromTag( pageURL );
    } else {
      src = pageURL;
    }
    if( this.content_frame )
			this.content_frame.setAttribute("src", src);
    else 
      return false;

  	this.firstTime = false;
    return true;
	}
  return false;
}
/** string GetUrlFromTag( string tag ) ;
 *  - purpose: creates a complete URL based on a tag.
 *  - in:  string tag representing the specific page to be loaded
 *  - out: string url representing the complete location of the page.
 **/               
function WM_GetURLFromTag( tag ) 
{
  return this.URL_PagePrefix + tag + this.URL_PagePostfix;
}
/** string GetTagFromURL( string tag ) ;
 *  - purpose: fetches a tag from a URL
 *  - in:   string url representing the complete location of the page.
 *  - out:  string tag representing the specific page to be loaded
 **/               
function WM_GetTagFromURL( url )
{
  return url.substring(this.URL_PagePrefix.length, this.URL_PagePostfix.length);
}

// SetHandlers pass-through for setting wizard button handlers easily
function WM_SetHandlers( onNext, onBack, onFinish, onCancel, onPageLoad, enablingFunc )
{
  this.WHANDLER.SetHandlers( onNext, onBack, onFinish, onCancel, onPageLoad, enablingFunc );
}
// SetPageData pass-through
function WM_SetPageData()
{
  this.WSM.SetPageData();
}
// SavePageData pass-through
function WM_SavePageData()
{
  this.WSM.SavePageData();
}

/** int GetMapLength()
 *  - purpose: returns the number of pages in the wizardMap
 *  - in:   nothing
 *  - out:  integer number of pages in wizardMap
 **/               
function WM_GetMapLength()
{
  var count = 0;
  for ( i in this.wizardMap )
    count++;
  return count;
}

/** void ProgressUpdate ( int currentPageNumber );
 *  - purpose: updates the "page x of y" display if available
 *  - in:   integer representing current page number. 
 *  - out:  nothing
 **/               
function WM_ProgressUpdate( currentPageNumber )
{
  var statusbar = document.getElementById ( "status" );
  if ( statusbar ) {
      var string;
      try {
          string = this.bundle.formatStringFromName("oflabel",
                                                    [currentPageNumber+1,
                                                    this.GetMapLength()], 2);
      } catch (e) {
          string = "";
      }
    statusbar.setAttribute( "progress", string );
  }
}

