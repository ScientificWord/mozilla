/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
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

/* 
nsIMAPBodyShell and associated classes
*/ 

#ifndef IMAPBODY_H
#define IMAPBODY_H

#include "nsImapCore.h"
#include "nsString.h"

class nsImapProtocol;

typedef enum _nsIMAPBodypartType {
	IMAP_BODY_MESSAGE_RFC822,
	IMAP_BODY_MESSAGE_HEADER,
	IMAP_BODY_LEAF,
	IMAP_BODY_MULTIPART
} nsIMAPBodypartType;

class nsIMAPBodyShell;
class nsIMAPBodypartMessage;
class nsHashtable;

class nsIMAPBodypart
{
public:
	// Construction
	virtual PRBool GetIsValid() { return m_isValid; }
	virtual void	SetIsValid(PRBool valid);
	virtual nsIMAPBodypartType	GetType() = 0;

	// Generation
    // Generates an HTML representation of this part.  Returns content length generated, -1 if failed.
    virtual PRInt32 Generate(nsIMAPBodyShell *aShell, PRBool /*stream*/, PRBool /* prefetch */) { return -1; }
    virtual void AdoptPartDataBuffer(char *buf);    // Adopts storage for part data buffer.  If NULL, sets isValid to PR_FALSE.
    virtual void AdoptHeaderDataBuffer(char *buf);  // Adopts storage for header data buffer.  If NULL, sets isValid to PR_FALSE.
    virtual PRBool ShouldFetchInline(nsIMAPBodyShell *aShell) { return PR_TRUE; }  // returns PR_TRUE if this part should be fetched inline for generation.
    virtual PRBool PreflightCheckAllInline(nsIMAPBodyShell *aShell) { return PR_TRUE; }

	virtual PRBool ShouldExplicitlyFetchInline();
	virtual PRBool ShouldExplicitlyNotFetchInline();
        virtual PRBool IsLastTextPart(const char *partNumberString) {return PR_TRUE;}

protected:
    // If stream is PR_FALSE, simply returns the content length that will be generated
    // the body of the part itself
    virtual PRInt32 GeneratePart(nsIMAPBodyShell *aShell, PRBool stream, PRBool prefetch);
    // the MIME headers of the part
    virtual PRInt32 GenerateMIMEHeader(nsIMAPBodyShell *aShell, PRBool stream, PRBool prefetch); 
    // Generates the MIME boundary wrapper for this part.
    virtual PRInt32 GenerateBoundary(nsIMAPBodyShell *aShell, PRBool stream, PRBool prefetch, PRBool lastBoundary);
    // lastBoundary indicates whether or not this should be the boundary for the
    // final MIME part of the multipart message.
    // Generates (possibly empty) filling for a part that won't be filled in inline.
    virtual PRInt32 GenerateEmptyFilling(nsIMAPBodyShell *aShell, PRBool stream, PRBool prefetch);

	// Part Numbers / Hierarchy
public:
	virtual char	*GetPartNumberString() { return m_partNumberString; }
	virtual nsIMAPBodypart	*FindPartWithNumber(const char *partNum);	// Returns the part object with the given number
	virtual nsIMAPBodypart	*GetParentPart() { return m_parentPart; }	// Returns the parent of this part.
																		// We will define a part of type message/rfc822 to be the
																		// parent of its body and header.
																		// A multipart is a parent of its child parts.
																		// All other leafs do not have children.

	// Other / Helpers
public:
	virtual ~nsIMAPBodypart();
	virtual nsIMAPBodypartMessage	*GetnsIMAPBodypartMessage() { return NULL; }

	const char	*GetBodyType() { return m_bodyType; }
	const char	*GetBodySubType() { return m_bodySubType; }
    void SetBoundaryData(char *boundaryData) { m_boundaryData = boundaryData; }

protected:
    virtual void QueuePrefetchMIMEHeader(nsIMAPBodyShell *aShell);
	//virtual void	PrefetchMIMEHeader();			// Initiates a prefetch for the MIME header of this part.
    nsIMAPBodypart(char *partNumber, nsIMAPBodypart *parentPart);

protected:
	PRBool	m_isValid;				// If this part is valid.
	char	*m_partNumberString;	// string representation of this part's full-hierarchy number.  Define 0 to be the top-level message
	char	*m_partData;			// data for this part.  NULL if not filled in yet.
	char	*m_headerData;			// data for this part's MIME header.  NULL if not filled in yet.
	char	*m_boundaryData;		// MIME boundary for this part
	PRInt32	m_partLength;
	PRInt32	m_contentLength;		// Total content length which will be Generate()'d.  -1 if not filled in yet.
	nsIMAPBodypart	*m_parentPart;	// Parent of this part

	// Fields	- Filled in from parsed BODYSTRUCTURE response (as well as others)
	char	*m_contentType;			// constructed from m_bodyType and m_bodySubType
	char	*m_bodyType;
	char	*m_bodySubType;
	char	*m_bodyID;
	char	*m_bodyDescription;
	char	*m_bodyEncoding;
	// we ignore extension data for now
};



// Message headers
// A special type of nsIMAPBodypart
// These may be headers for the top-level message,
// or any body part of type message/rfc822.
class nsIMAPMessageHeaders : public nsIMAPBodypart
{
public:
    nsIMAPMessageHeaders(char *partNum, nsIMAPBodypart *parentPart);
	virtual nsIMAPBodypartType	GetType();
    // Generates an HTML representation of this part.  Returns content length generated, -1 if failed.
    virtual PRInt32 Generate(nsIMAPBodyShell *aShell, PRBool stream, PRBool prefetch);
    virtual PRBool ShouldFetchInline(nsIMAPBodyShell *aShell);
    virtual void QueuePrefetchMessageHeaders(nsIMAPBodyShell *aShell);
};


class nsIMAPBodypartMultipart : public nsIMAPBodypart
{
public:
    nsIMAPBodypartMultipart(char *partNum, nsIMAPBodypart *parentPart);
	virtual nsIMAPBodypartType	GetType();
	virtual ~nsIMAPBodypartMultipart();
    virtual PRBool ShouldFetchInline(nsIMAPBodyShell *aShell);
    virtual PRBool PreflightCheckAllInline(nsIMAPBodyShell *aShell);
    // Generates an HTML representation of this part.  Returns content length generated, -1 if failed.
    virtual PRInt32 Generate(nsIMAPBodyShell *aShell, PRBool stream, PRBool prefetch);
	virtual nsIMAPBodypart	*FindPartWithNumber(const char *partNum);	// Returns the part object with the given number
        virtual PRBool IsLastTextPart(const char *partNumberString);
    void AppendPart(nsIMAPBodypart *part)  { m_partList->AppendElement(part); }
    void SetBodySubType(char *bodySubType);

protected:
	nsVoidArray			*m_partList;			// An ordered list of top-level body parts for this shell
};


// The name "leaf" is somewhat misleading, since a part of type message/rfc822 is technically
// a leaf, even though it can contain other parts within it.
class nsIMAPBodypartLeaf : public nsIMAPBodypart
{
public:
    nsIMAPBodypartLeaf(char *partNum, nsIMAPBodypart *parentPart,
      char *bodyType, char *bodySubType, char *bodyID, char *bodyDescription, char *bodyEncoding, PRInt32 partLength);
	virtual nsIMAPBodypartType	GetType();
    // Generates an HTML representation of this part.  Returns content length generated, -1 if failed.
    virtual PRInt32 Generate(nsIMAPBodyShell *aShell, PRBool stream, PRBool prefetch);
    // returns PR_TRUE if this part should be fetched inline for generation.
    virtual PRBool ShouldFetchInline(nsIMAPBodyShell *aShell);
    virtual PRBool PreflightCheckAllInline(nsIMAPBodyShell *aShell);
};


class nsIMAPBodypartMessage : public nsIMAPBodypartLeaf
{
public:
    nsIMAPBodypartMessage(char *partNum, nsIMAPBodypart *parentPart, PRBool topLevelMessage,
      char *bodyType, char *bodySubType, char *bodyID, char *bodyDescription, char *bodyEncoding, PRInt32 partLength);
    void SetBody(nsIMAPBodypart *body);
	virtual nsIMAPBodypartType	GetType();
	virtual ~nsIMAPBodypartMessage();
    virtual PRInt32 Generate(nsIMAPBodyShell *aShell, PRBool stream, PRBool prefetch);
    virtual PRBool ShouldFetchInline(nsIMAPBodyShell *aShell);
    virtual PRBool PreflightCheckAllInline(nsIMAPBodyShell *aShell);
	virtual nsIMAPBodypart	*FindPartWithNumber(const char *partNum);	// Returns the part object with the given number
	void	AdoptMessageHeaders(char *headers);			// Fills in buffer (and adopts storage) for header object
														// partNum specifies the message part number to which the
														// headers correspond.  NULL indicates the top-level message
	virtual nsIMAPBodypartMessage	*GetnsIMAPBodypartMessage() { return this; }
	virtual	PRBool		GetIsTopLevelMessage() { return m_topLevelMessage; }

protected:
	nsIMAPMessageHeaders		*m_headers;				// Every body shell should have headers
	nsIMAPBodypart			*m_body;	
	PRBool					m_topLevelMessage;		// Whether or not this is the top-level message

};


class nsIMAPMessagePartIDArray;

// We will refer to a Body "Shell" as a hierarchical object representation of a parsed BODYSTRUCTURE
// response.  A shell contains representations of Shell "Parts."  A Body Shell can undergo essentially
// two operations: Construction and Generation.
// Shell Construction occurs from a parsed a BODYSTRUCTURE response, split into empty parts.
// Shell Generation generates a "MIME Shell" of the message and streams it to libmime for
// display.  The MIME Shell has selected (inline) parts filled in, and leaves all others
// for on-demand retrieval through explicit part fetches.

class nsIMAPBodyShell
{
public:
    nsIMAPBodyShell(nsImapProtocol *protocolConnection, nsIMAPBodypartMessage *message, PRUint32 UID, const char *folderName);
	virtual ~nsIMAPBodyShell();
	void	SetConnection(nsImapProtocol *con) { m_protocolConnection = con; }	// To be used after a shell is uncached
	virtual PRBool GetIsValid() { return m_isValid; }
	virtual void	SetIsValid(PRBool valid);

	// Prefetch
	void	AddPrefetchToQueue(nsIMAPeFetchFields, const char *partNum);	// Adds a message body part to the queue to be prefetched
									// in a single, pipelined command
	void	FlushPrefetchQueue();	// Runs a single pipelined command which fetches all of the
									// elements in the prefetch queue
	void	AdoptMessageHeaders(char *headers, const char *partNum);	// Fills in buffer (and adopts storage) for header object
																		// partNum specifies the message part number to which the
																		// headers correspond.  NULL indicates the top-level message
	void	AdoptMimeHeader(const char *partNum, char *mimeHeader);	// Fills in buffer (and adopts storage) for MIME headers in appropriate object.
																		// If object can't be found, sets isValid to PR_FALSE.

	// Generation
	virtual PRInt32 Generate(char *partNum);	// Streams out an HTML representation of this IMAP message, going along and
									// fetching parts it thinks it needs, and leaving empty shells for the parts
									// it doesn't.
									// Returns number of bytes generated, or -1 if invalid.
									// If partNum is not NULL, then this works to generates a MIME part that hasn't been downloaded yet
									// and leaves out all other parts.  By default, to generate a normal message, partNum should be NULL.

  virtual PRBool GetShowAttachmentsInline();  // Returns TRUE if the user has the pref "Show Attachments Inline" set.
                                              // Returns FALSE if the setting is "Show Attachments as Links"
  PRBool	PreflightCheckAllInline();		  // Returns PR_TRUE if all parts are inline, PR_FALSE otherwise.  Does not generate anything.

  // Helpers
  nsImapProtocol *GetConnection() { return m_protocolConnection; }
  PRBool	GetPseudoInterrupted();
  PRBool DeathSignalReceived();
  nsCString &GetUID() { return m_UID; }
  const char	*GetFolderName() { return m_folderName; }
  char	*GetGeneratingPart() { return m_generatingPart; }
  PRBool	IsBeingGenerated() { return m_isBeingGenerated; }	// Returns PR_TRUE if this is in the process of being
															  // generated, so we don't re-enter
  PRBool IsShellCached() { return m_cached; }
  void	SetIsCached(PRBool isCached) { m_cached = isCached; }
  PRBool	GetGeneratingWholeMessage() { return m_generatingWholeMessage; }
  IMAP_ContentModifiedType	GetContentModified() { return m_contentModified; }
  void	SetContentModified(IMAP_ContentModifiedType modType) { m_contentModified = modType; }

protected:

  nsIMAPBodypartMessage	*m_message;

  nsIMAPMessagePartIDArray        *m_prefetchQueue;	// array of pipelined part prefetches.  Ok, so it's not really a queue.

  PRBool                          m_isValid;
  nsImapProtocol                  *m_protocolConnection;  // Connection, for filling in parts
  nsCString                       m_UID;                  // UID of this message
  char                            *m_folderName;          // folder that contains this message
  char                            *m_generatingPart;      // If a specific part is being generated, this is it.  Otherwise, NULL.
  PRBool                          m_isBeingGenerated;     // PR_TRUE if this body shell is in the process of being generated
  PRBool                          m_gotAttachmentPref;    // Whether or not m_showAttachmentsInline has been initialized 
  PRBool                          m_showAttachmentsInline; // Whether or not we should display attachment inline
  PRBool                          m_cached;                 // Whether or not this shell is cached
  PRBool                          m_generatingWholeMessage; // whether or not we are generating the whole (non-MPOD) message
                                                          // Set to PR_FALSE if we are generating by parts
  IMAP_ContentModifiedType        m_contentModified;	// under what conditions the content has been modified.
                                                        // Either IMAP_CONTENT_MODIFIED_VIEW_INLINE or IMAP_CONTENT_MODIFIED_VIEW_AS_LINKS
};



// This class caches shells, so we don't have to always go and re-fetch them.
// This does not cache any of the filled-in inline parts;  those are cached individually
// in the libnet memory cache.  (ugh, how will we do that?)
// Since we'll only be retrieving shells for messages over a given size, and since the
// shells themselves won't be very large, this cache will not grow very big (relatively)
// and should handle most common usage scenarios.

// A body cache is associated with a given host, spanning folders.
// It should pay attention to UIDVALIDITY.

class nsIMAPBodyShellCache
{

public:
	static nsIMAPBodyShellCache	*Create();
	virtual ~nsIMAPBodyShellCache();

	PRBool			AddShellToCache(nsIMAPBodyShell *shell);		// Adds shell to cache, possibly ejecting
																// another entry based on scheme in EjectEntry().
	nsIMAPBodyShell	*FindShellForUID(nsCString &UID, const char *mailboxName, IMAP_ContentModifiedType modType);	// Looks up a shell in the cache given the message's UID.
	nsIMAPBodyShell	*FindShellForUID(PRUint32 UID, const char *mailboxName, IMAP_ContentModifiedType modType);	// Looks up a shell in the cache given the message's UID.
															// Returns the shell if found, NULL if not found.

protected:
	nsIMAPBodyShellCache();
	PRBool	EjectEntry();	// Chooses an entry to eject;  deletes that entry;  and ejects it from the cache,
							// clearing up a new space.  Returns PR_TRUE if it found an entry to eject, PR_FALSE otherwise.
	PRUint32	GetSize() { return m_shellList->Count(); }
	PRUint32	GetMaxSize() { return 20; }


protected:
	nsVoidArray		*m_shellList;	// For maintenance
	nsHashtable		*m_shellHash;	// For quick lookup based on UID

};




// MessagePartID and MessagePartIDArray are used for pipelining prefetches.

class nsIMAPMessagePartID
{
public:
	nsIMAPMessagePartID(nsIMAPeFetchFields fields, const char *partNumberString);
	nsIMAPeFetchFields		GetFields() { return m_fields; }
	const char		*GetPartNumberString() { return m_partNumberString; }


protected:
	const char *m_partNumberString;
	nsIMAPeFetchFields m_fields;
};


class nsIMAPMessagePartIDArray : public nsVoidArray {
public:
	nsIMAPMessagePartIDArray();
	~nsIMAPMessagePartIDArray();

	void				RemoveAndFreeAll();
	int					GetNumParts() {return Count();}
	nsIMAPMessagePartID	*GetPart(int i) 
	{
		NS_ASSERTION(i >= 0 && i < Count(), "invalid message part #");
		return (nsIMAPMessagePartID *) ElementAt(i);
	}
};


#endif // IMAPBODY_H
