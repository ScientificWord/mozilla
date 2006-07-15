/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 * vim: set ts=4 sw=4 et tw=80:
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
 * Portions created by the Initial Developer are Copyright (C) 1999
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Chris Waterson <waterson@netscape.com>
 *   Axel Hecht <axel@pike.org>
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

#include "nsRDFXMLSerializer.h"

#include "nsIAtom.h"
#include "nsIOutputStream.h"
#include "nsIRDFService.h"
#include "nsIRDFContainerUtils.h"
#include "nsIServiceManager.h"
#include "nsString.h"
#include "nsXPIDLString.h"
#include "nsVoidArray.h"
#include "rdf.h"
#include "rdfutil.h"

#include "rdfIDataSource.h"

#include "nsITimelineService.h"

PRInt32 nsRDFXMLSerializer::gRefCnt = 0;
nsIRDFContainerUtils* nsRDFXMLSerializer::gRDFC;
nsIRDFResource* nsRDFXMLSerializer::kRDF_instanceOf;
nsIRDFResource* nsRDFXMLSerializer::kRDF_type;
nsIRDFResource* nsRDFXMLSerializer::kRDF_nextVal;
nsIRDFResource* nsRDFXMLSerializer::kRDF_Bag;
nsIRDFResource* nsRDFXMLSerializer::kRDF_Seq;
nsIRDFResource* nsRDFXMLSerializer::kRDF_Alt;

static const char kRDFDescriptionOpen[]      = "  <RDF:Description";
static const char kIDAttr[]                  = " RDF:ID=\"";
static const char kAboutAttr[]               = " RDF:about=\"";
static const char kRDFDescriptionClose[]     = "  </RDF:Description>\n";
static const char kRDFResource1[] = " RDF:resource=\"";
static const char kRDFResource2[] = "\"/>\n";
static const char kRDFParseTypeInteger[] = " NC:parseType=\"Integer\">";
static const char kRDFParseTypeDate[] = " NC:parseType=\"Date\">";
static const char kRDFUnknown[] = "><!-- unknown node type -->";

NS_IMETHODIMP
nsRDFXMLSerializer::Create(nsISupports* aOuter, REFNSIID aIID, void** aResult)
{
    if (aOuter)
        return NS_ERROR_NO_AGGREGATION;

    nsCOMPtr<nsIRDFXMLSerializer> result = new nsRDFXMLSerializer();
    if (! result)
        return NS_ERROR_OUT_OF_MEMORY;
    // The serializer object is here, addref gRefCnt so that the
    // destructor can safely release it.
    gRefCnt++;

    nsresult rv;
    rv = result->QueryInterface(aIID, aResult);

    if (NS_FAILED(rv)) return rv;

    if (gRefCnt == 1) do {
        nsCOMPtr<nsIRDFService> rdf = do_GetService("@mozilla.org/rdf/rdf-service;1", &rv);
        if (NS_FAILED(rv)) break;

        rv = rdf->GetResource(NS_LITERAL_CSTRING(RDF_NAMESPACE_URI "instanceOf"),
                              &kRDF_instanceOf);
        if (NS_FAILED(rv)) break;

        rv = rdf->GetResource(NS_LITERAL_CSTRING(RDF_NAMESPACE_URI "type"),
                              &kRDF_type);
        if (NS_FAILED(rv)) break;

        rv = rdf->GetResource(NS_LITERAL_CSTRING(RDF_NAMESPACE_URI "nextVal"),
                              &kRDF_nextVal);
        if (NS_FAILED(rv)) break;

        rv = rdf->GetResource(NS_LITERAL_CSTRING(RDF_NAMESPACE_URI "Bag"),
                              &kRDF_Bag);
        if (NS_FAILED(rv)) break;

        rv = rdf->GetResource(NS_LITERAL_CSTRING(RDF_NAMESPACE_URI "Seq"),
                              &kRDF_Seq);
        if (NS_FAILED(rv)) break;

        rv = rdf->GetResource(NS_LITERAL_CSTRING(RDF_NAMESPACE_URI "Alt"),
                              &kRDF_Alt);
        if (NS_FAILED(rv)) break;

        rv = CallGetService("@mozilla.org/rdf/container-utils;1", &gRDFC);
        if (NS_FAILED(rv)) break;
    } while (0);

    return rv;
}

nsRDFXMLSerializer::nsRDFXMLSerializer()
{
    MOZ_COUNT_CTOR(nsRDFXMLSerializer);
}

nsRDFXMLSerializer::~nsRDFXMLSerializer()
{
    MOZ_COUNT_DTOR(nsRDFXMLSerializer);

    if (--gRefCnt == 0) {
        NS_IF_RELEASE(kRDF_Bag);
        NS_IF_RELEASE(kRDF_Seq);
        NS_IF_RELEASE(kRDF_Alt);
        NS_IF_RELEASE(kRDF_instanceOf);
        NS_IF_RELEASE(kRDF_type);
        NS_IF_RELEASE(kRDF_nextVal);
        NS_IF_RELEASE(gRDFC);
    }
}

NS_IMPL_ISUPPORTS2(nsRDFXMLSerializer, nsIRDFXMLSerializer, nsIRDFXMLSource)

NS_IMETHODIMP
nsRDFXMLSerializer::Init(nsIRDFDataSource* aDataSource)
{
    if (! aDataSource)
        return NS_ERROR_NULL_POINTER;

    mDataSource = aDataSource;
    mDataSource->GetURI(getter_Copies(mBaseURLSpec));

    // Add the ``RDF'' prefix, by default.
    nsCOMPtr<nsIAtom> prefix;

    prefix = do_GetAtom("RDF");
    AddNameSpace(prefix, NS_LITERAL_STRING("http://www.w3.org/1999/02/22-rdf-syntax-ns#"));

    prefix = do_GetAtom("NC");
    AddNameSpace(prefix, NS_LITERAL_STRING("http://home.netscape.com/NC-rdf#"));

    mQNames.Init();
    mPrefixID = 0;

    return NS_OK;
}

NS_IMETHODIMP
nsRDFXMLSerializer::AddNameSpace(nsIAtom* aPrefix, const nsAString& aURI)
{
    nsCOMPtr<nsIAtom> prefix = aPrefix;
    if (!prefix) {
        // Make up a prefix, we don't want default namespaces, so
        // that we can use QNames for elements and attributes alike.
        prefix = EnsureNewPrefix();
    }
    mNameSpaces.Put(aURI, prefix);
    return NS_OK;
}

static nsresult
rdf_BlockingWrite(nsIOutputStream* stream, const char* buf, PRUint32 size)
{
    PRUint32 written = 0;
    PRUint32 remaining = size;
    while (remaining > 0) {
        nsresult rv;
        PRUint32 cb;

        if (NS_FAILED(rv = stream->Write(buf + written, remaining, &cb)))
            return rv;

        written += cb;
        remaining -= cb;
    }
    return NS_OK;
}

static nsresult
rdf_BlockingWrite(nsIOutputStream* stream, const nsCSubstring& s)
{
    return rdf_BlockingWrite(stream, s.BeginReading(), s.Length());
}

static nsresult
rdf_BlockingWrite(nsIOutputStream* stream, const nsAString& s)
{
    NS_ConvertUTF16toUTF8 utf8(s);
    return rdf_BlockingWrite(stream, utf8.get(), utf8.Length());
}

already_AddRefed<nsIAtom>
nsRDFXMLSerializer::EnsureNewPrefix()
{
    nsCAutoString qname;
    nsCOMPtr<nsIAtom> prefix;
    PRBool isNewPrefix;
    do {
        isNewPrefix = PR_TRUE;
        qname.AssignLiteral("NS");
        qname.AppendInt(++mPrefixID, 10);
        prefix = do_GetAtom(qname);
        nsNameSpaceMap::const_iterator iter = mNameSpaces.first();
        while (iter != mNameSpaces.last() && isNewPrefix) {
            isNewPrefix = (iter->mPrefix != prefix);
            ++iter;
        } 
    } while (!isNewPrefix);
    nsIAtom* outPrefix = nsnull;
    prefix.swap(outPrefix);
    return outPrefix;
}

// This converts a property resource (like
// "http://www.w3.org/TR/WD-rdf-syntax#Description") into a QName
// ("RDF:Description"), and registers the namespace, if it's made up.

nsresult
nsRDFXMLSerializer::RegisterQName(nsIRDFResource* aResource)
{
    nsCAutoString uri, qname;
    aResource->GetValueUTF8(uri);

    nsNameSpaceMap::const_iterator iter = mNameSpaces.GetNameSpaceOf(uri);
    if (iter != mNameSpaces.last()) {
        NS_ENSURE_TRUE(iter->mPrefix, NS_ERROR_UNEXPECTED);
        iter->mPrefix->ToUTF8String(qname);
        qname.Append(':');
        qname += StringTail(uri, uri.Length() - iter->mURI.Length());
        return mQNames.Put(aResource, qname) ? NS_OK : NS_ERROR_FAILURE;
    }

    // Okay, so we don't have it in our map. Try to make one up. This
    // is very bogus.
    PRInt32 i = uri.RFindChar('#'); // first try a '#'
    if (i == -1) {
        i = uri.RFindChar('/');
        if (i == -1) {
            // Okay, just punt and assume there is _no_ namespace on
            // this thing...
            return mQNames.Put(aResource, uri) ? NS_OK : NS_ERROR_FAILURE;
        }
    }

    // Take whatever is to the right of the '#' or '/' and call it the
    // local name, make up a prefix.
    nsCOMPtr<nsIAtom> prefix = EnsureNewPrefix();
    mNameSpaces.Put(StringHead(uri, i+1), prefix);
    prefix->ToUTF8String(qname);
    qname.Append(':');
    qname += StringTail(uri, uri.Length() - (i + 1));

    return mQNames.Put(aResource, qname) ? NS_OK : NS_ERROR_FAILURE;
}

nsresult
nsRDFXMLSerializer::GetQName(nsIRDFResource* aResource, nsCString& aQName)
{
    return mQNames.Get(aResource, &aQName) ? NS_OK : NS_ERROR_UNEXPECTED;
}

PRBool
nsRDFXMLSerializer::IsContainerProperty(nsIRDFResource* aProperty)
{
    // Return `true' if the property is an internal property related
    // to being a container.
    if (aProperty == kRDF_instanceOf)
        return PR_TRUE;

    if (aProperty == kRDF_nextVal)
        return PR_TRUE;

    PRBool isOrdinal = PR_FALSE;
    gRDFC->IsOrdinalProperty(aProperty, &isOrdinal);
    if (isOrdinal)
        return PR_TRUE;

    return PR_FALSE;
} 


// convert '&', '<', and '>' into "&amp;", "&lt;", and "&gt", respectively.
static const char amp[] = "&amp;";
static const char lt[] = "&lt;";
static const char gt[] = "&gt;";
static const char quot[] = "&quot;";

static void
rdf_EscapeAmpersandsAndAngleBrackets(nsCString& s)
{
    PRUint32 newLength, origLength;
    newLength = origLength = s.Length();

    // Compute the length of the result string.
    const char* start = s.BeginReading();
    const char* end = s.EndReading();
    const char* c = start;
    while (c != end) {
        switch (*c) {
        case '&' :
            newLength += sizeof(amp) - 2;
            break;
        case '<':
        case '>':
            newLength += sizeof(gt) - 2;
            break;
        default:
            break;
        }
        ++c;
    }
    if (newLength == origLength) {
        // nothing to escape
        return;
    }

    // escape the chars from the end back to the front.
    s.SetLength(newLength);

    // Buffer might have changed, get the pointers again
    start = s.BeginReading(); // begin of string
    c = start + origLength - 1; // last char in original string
    char* w = s.EndWriting() - 1; // last char in grown buffer
    while (c >= start) {
        switch (*c) {
        case '&' :
            w -= 4;
            nsCharTraits<char>::copy(w, amp, sizeof(amp) - 1);
            break;
        case '<':
            w -= 3;
            nsCharTraits<char>::copy(w, lt, sizeof(lt) - 1);
            break;
        case '>':
            w -= 3;
            nsCharTraits<char>::copy(w, gt, sizeof(gt) - 1);
            break;
        default:
            *w = *c;
        }
        --w;
        --c;
    }
}

// convert '"' to "&quot;"
static void
rdf_EscapeQuotes(nsCString& s)
{
    PRInt32 i = 0;
    while ((i = s.FindChar('"', i)) != -1) {
        s.Replace(i, 1, quot, sizeof(quot) - 1);
        i += sizeof(quot) - 2;
    }
}

static void
rdf_EscapeAttributeValue(nsCString& s)
{
    rdf_EscapeAmpersandsAndAngleBrackets(s);
    rdf_EscapeQuotes(s);
}


nsresult
nsRDFXMLSerializer::SerializeInlineAssertion(nsIOutputStream* aStream,
                                             nsIRDFResource* aResource,
                                             nsIRDFResource* aProperty,
                                             nsIRDFLiteral* aValue)
{
    nsresult rv;
    nsCString qname;
    rv = GetQName(aProperty, qname);
    NS_ENSURE_SUCCESS(rv, rv);

    rv = rdf_BlockingWrite(aStream,
                           NS_LITERAL_CSTRING("\n                   "));
    if (NS_FAILED(rv)) return rv;

    const PRUnichar* value;
    aValue->GetValueConst(&value);
    NS_ConvertUTF16toUTF8 s(value);

    rdf_EscapeAttributeValue(s);

    rv = rdf_BlockingWrite(aStream, qname);
    if (NS_FAILED(rv)) return rv;
    rv = rdf_BlockingWrite(aStream, "=\"", 2);
    if (NS_FAILED(rv)) return rv;
    s.Append('"');
    return rdf_BlockingWrite(aStream, s);
}

nsresult
nsRDFXMLSerializer::SerializeChildAssertion(nsIOutputStream* aStream,
                                            nsIRDFResource* aResource,
                                            nsIRDFResource* aProperty,
                                            nsIRDFNode* aValue)
{
    nsCString qname;
    nsresult rv = GetQName(aProperty, qname);
    NS_ENSURE_SUCCESS(rv, rv);

    rv = rdf_BlockingWrite(aStream, "    <", 5);
    if (NS_FAILED(rv)) return rv;
    rv = rdf_BlockingWrite(aStream, qname);
    if (NS_FAILED(rv)) return rv;

    nsCOMPtr<nsIRDFResource> resource;
    nsCOMPtr<nsIRDFLiteral> literal;
    nsCOMPtr<nsIRDFInt> number;
    nsCOMPtr<nsIRDFDate> date;

    if ((resource = do_QueryInterface(aValue)) != nsnull) {
        nsCAutoString uri;
        resource->GetValueUTF8(uri);

        rdf_MakeRelativeRef(mBaseURLSpec, uri);
        rdf_EscapeAttributeValue(uri);

        rv = rdf_BlockingWrite(aStream, kRDFResource1,
                               sizeof(kRDFResource1) - 1);
        if (NS_FAILED(rv)) return rv;
        rv = rdf_BlockingWrite(aStream, uri);
        if (NS_FAILED(rv)) return rv;
        rv = rdf_BlockingWrite(aStream, kRDFResource2,
                               sizeof(kRDFResource2) - 1);
        if (NS_FAILED(rv)) return rv;

        goto no_close_tag;
    }
    else if ((literal = do_QueryInterface(aValue)) != nsnull) {
        const PRUnichar *value;
        literal->GetValueConst(&value);
        NS_ConvertUTF16toUTF8 s(value);

        rdf_EscapeAmpersandsAndAngleBrackets(s);

        rv = rdf_BlockingWrite(aStream, ">", 1);
        if (NS_FAILED(rv)) return rv;
        rv = rdf_BlockingWrite(aStream, s);
        if (NS_FAILED(rv)) return rv;
    }
    else if ((number = do_QueryInterface(aValue)) != nsnull) {
        PRInt32 value;
        number->GetValue(&value);

        nsCAutoString n;
        n.AppendInt(value);

        rv = rdf_BlockingWrite(aStream, kRDFParseTypeInteger, 
                               sizeof(kRDFParseTypeInteger) - 1);
        if (NS_FAILED(rv)) return rv;
        rv = rdf_BlockingWrite(aStream, n);
        if (NS_FAILED(rv)) return rv;
    }
    else if ((date = do_QueryInterface(aValue)) != nsnull) {
        PRTime value;
        date->GetValue(&value);

        nsCAutoString s;
        rdf_FormatDate(value, s);

        rv = rdf_BlockingWrite(aStream, kRDFParseTypeDate, 
                               sizeof(kRDFParseTypeDate) - 1);
        if (NS_FAILED(rv)) return rv;
        rv = rdf_BlockingWrite(aStream, s);
        if (NS_FAILED(rv)) return rv;
    }
    else {
        // XXX it doesn't support nsIRDFResource _or_ nsIRDFLiteral???
        // We should serialize nsIRDFInt, nsIRDFDate, etc...
        NS_WARNING("unknown RDF node type");

        rv = rdf_BlockingWrite(aStream, kRDFUnknown, sizeof(kRDFUnknown) - 1);
        if (NS_FAILED(rv)) return rv;
    }

    rv = rdf_BlockingWrite(aStream, "</", 2);
    if (NS_FAILED(rv)) return rv;
    rv = rdf_BlockingWrite(aStream, qname);
    if (NS_FAILED(rv)) return rv;
    return rdf_BlockingWrite(aStream, ">\n", 2);

 no_close_tag:
    return NS_OK;
}

nsresult
nsRDFXMLSerializer::SerializeProperty(nsIOutputStream* aStream,
                                      nsIRDFResource* aResource,
                                      nsIRDFResource* aProperty,
                                      PRBool aInline,
                                      PRInt32* aSkipped)
{
    nsresult rv = NS_OK;

    PRInt32 skipped = 0;

    nsCOMPtr<nsISimpleEnumerator> assertions;
    mDataSource->GetTargets(aResource, aProperty, PR_TRUE, getter_AddRefs(assertions));
    if (! assertions)
        return NS_ERROR_FAILURE;

    // Serializing the assertion inline is ok as long as the property has
    // only one target value, and it is a literal that doesn't include line
    // breaks.
    PRBool needsChild = PR_FALSE;

    while (1) {
        PRBool hasMore = PR_FALSE;
        assertions->HasMoreElements(&hasMore);
        if (! hasMore)
            break;

        nsCOMPtr<nsISupports> isupports;
        assertions->GetNext(getter_AddRefs(isupports));
        nsCOMPtr<nsIRDFLiteral> literal = do_QueryInterface(isupports);
        needsChild |= (!literal);

        if (!needsChild) {
            assertions->HasMoreElements(&needsChild);
            if (!needsChild) {
                const PRUnichar* literalVal = nsnull;
                literal->GetValueConst(&literalVal);
                if (literalVal) {
                    for (; *literalVal; literalVal++) {
                        if (*literalVal == PRUnichar('\n') ||
                            *literalVal == PRUnichar('\r')) {
                            needsChild = PR_TRUE;
                            break;
                        }
                    }
                }
            }
        }

        if (aInline && !needsChild) {
            rv = SerializeInlineAssertion(aStream, aResource, aProperty, literal);
        }
        else if (!aInline && needsChild) {
            nsCOMPtr<nsIRDFNode> value = do_QueryInterface(isupports);
            rv = SerializeChildAssertion(aStream, aResource, aProperty, value);
        }
        else {
            ++skipped;
            rv = NS_OK;
        }

        if (NS_FAILED(rv))
            break;
    }

    *aSkipped += skipped;
    return rv;
}


nsresult
nsRDFXMLSerializer::SerializeDescription(nsIOutputStream* aStream,
                                         nsIRDFResource* aResource)
{
    nsresult rv;

    PRBool isTypedNode = PR_FALSE;
    nsCString typeQName;

    nsCOMPtr<nsIRDFNode> typeNode;
    mDataSource->GetTarget(aResource, kRDF_type, PR_TRUE, getter_AddRefs(typeNode));
    if (typeNode) {
        nsCOMPtr<nsIRDFResource> type = do_QueryInterface(typeNode, &rv);
        if (type) {
            // Try to get a namespace prefix.  If none is available,
            // just treat the description as if it weren't a typed node 
            // after all and emit rdf:type as a normal property.  This 
            // seems preferable to using a bogus (invented) prefix.
            isTypedNode = NS_SUCCEEDED(GetQName(type, typeQName));
        }
    }

    nsCAutoString uri;
    rv = aResource->GetValueUTF8(uri);
    if (NS_FAILED(rv)) return rv;

    rdf_MakeRelativeRef(mBaseURLSpec, uri);
    rdf_EscapeAttributeValue(uri);

    // Emit an open tag and the subject
    if (isTypedNode) {
        rv = rdf_BlockingWrite(aStream, NS_LITERAL_STRING("  <"));
        if (NS_FAILED(rv)) return rv;
        // Watch out for the default namespace!
        rv = rdf_BlockingWrite(aStream, typeQName);
        if (NS_FAILED(rv)) return rv;
    }
    else {
        rv = rdf_BlockingWrite(aStream, kRDFDescriptionOpen,
                               sizeof(kRDFDescriptionOpen) - 1);
        if (NS_FAILED(rv)) return rv;
    }
    if (uri[0] == PRUnichar('#')) {
        uri.Cut(0, 1);
        rv = rdf_BlockingWrite(aStream, kIDAttr, sizeof(kIDAttr) - 1);
    }
    else {
        rv = rdf_BlockingWrite(aStream, kAboutAttr, sizeof(kAboutAttr) - 1);
    }
    if (NS_FAILED(rv)) return rv;

    uri.Append('"');
    rv = rdf_BlockingWrite(aStream, uri);
    if (NS_FAILED(rv)) return rv;

    // Any value that's a literal we can write out as an inline
    // attribute on the RDF:Description
    nsAutoVoidArray visited;
    PRInt32 skipped = 0;

    nsCOMPtr<nsISimpleEnumerator> arcs;
    mDataSource->ArcLabelsOut(aResource, getter_AddRefs(arcs));

    if (arcs) {
        // Don't re-serialize rdf:type later on
        if (isTypedNode)
            visited.AppendElement(kRDF_type);

        while (1) {
            PRBool hasMore = PR_FALSE;
            arcs->HasMoreElements(&hasMore);
            if (! hasMore)
                break;

            nsCOMPtr<nsISupports> isupports;
            arcs->GetNext(getter_AddRefs(isupports));

            nsCOMPtr<nsIRDFResource> property = do_QueryInterface(isupports);
            if (! property)
                continue;

            // Ignore properties that pertain to containers; we may be
            // called from SerializeContainer() if the container resource
            // has been assigned non-container properties.
            if (IsContainerProperty(property))
                continue;

            // Only serialize values for the property once.
            if (visited.IndexOf(property.get()) >= 0)
                continue;

            visited.AppendElement(property.get());

            SerializeProperty(aStream, aResource, property, PR_TRUE, &skipped);
        }
    }

    if (skipped) {
        // Close the RDF:Description tag.
        rv = rdf_BlockingWrite(aStream, NS_LITERAL_CSTRING(">\n"));
        if (NS_FAILED(rv)) return rv;

        // Now write out resources (which might have their own
        // substructure) as children.
        mDataSource->ArcLabelsOut(aResource, getter_AddRefs(arcs));

        if (arcs) {
            // Forget that we've visited anything
            visited.Clear();
            // ... except for rdf:type
            if (isTypedNode)
                visited.AppendElement(kRDF_type);

            while (1) {
                PRBool hasMore = PR_FALSE;
                arcs->HasMoreElements(&hasMore);
                if (! hasMore)
                    break;

                nsCOMPtr<nsISupports> isupports;
                arcs->GetNext(getter_AddRefs(isupports));

                nsCOMPtr<nsIRDFResource> property = do_QueryInterface(isupports);
                if (! property)
                    continue;

                // Ignore properties that pertain to containers; we may be
                // called from SerializeContainer() if the container
                // resource has been assigned non-container properties.
                if (IsContainerProperty(property))
                    continue;

                // have we already seen this property?  If so, don't write it
                // out again; serialize property will write each instance.
                if (visited.IndexOf(property.get()) >= 0)
                    continue;

                visited.AppendElement(property.get());

                SerializeProperty(aStream, aResource, property, PR_FALSE, &skipped);
            }
        }

        // Emit a proper close-tag.
        if (isTypedNode) {
            rv = rdf_BlockingWrite(aStream,  NS_LITERAL_CSTRING("  </"));
            if (NS_FAILED(rv)) return rv;
            // Watch out for the default namespace!
            rdf_BlockingWrite(aStream, typeQName);
            if (NS_FAILED(rv)) return rv;
            rdf_BlockingWrite(aStream, ">\n", 2);
            if (NS_FAILED(rv)) return rv;
        }
        else {
            rv = rdf_BlockingWrite(aStream, kRDFDescriptionClose,
                                   sizeof(kRDFDescriptionClose) - 1);
            if (NS_FAILED(rv)) return rv;
        }
    }
    else {
        // If we saw _no_ child properties, then we can don't need a
        // close-tag.
        rv = rdf_BlockingWrite(aStream, NS_LITERAL_CSTRING(" />\n"));
        if (NS_FAILED(rv)) return rv;
    }

    return NS_OK;
}

nsresult
nsRDFXMLSerializer::SerializeMember(nsIOutputStream* aStream,
                                      nsIRDFResource* aContainer,
                                      nsIRDFNode* aMember)
{
    // If it's a resource, then output a "<RDF:li RDF:resource=... />"
    // tag, because we'll be dumping the resource separately. (We
    // iterate thru all the resources in the datasource,
    // remember?) Otherwise, output the literal value.

    nsCOMPtr<nsIRDFResource> resource;
    nsCOMPtr<nsIRDFLiteral> literal;
    nsCOMPtr<nsIRDFInt> number;
    nsCOMPtr<nsIRDFDate> date;

static const char kRDFLIOpen[] = "    <RDF:li";
    nsresult rv = rdf_BlockingWrite(aStream, kRDFLIOpen,
                                    sizeof(kRDFLIOpen) - 1); 
    if (NS_FAILED(rv)) return rv;

    if ((resource = do_QueryInterface(aMember)) != nsnull) {
        nsCAutoString uri;
        resource->GetValueUTF8(uri);

        rdf_MakeRelativeRef(mBaseURLSpec, uri);
        rdf_EscapeAttributeValue(uri);

        rv = rdf_BlockingWrite(aStream, kRDFResource1,
                               sizeof(kRDFResource1) - 1);
        if (NS_FAILED(rv)) return rv;
        rv = rdf_BlockingWrite(aStream, uri);
        if (NS_FAILED(rv)) return rv;
        rv = rdf_BlockingWrite(aStream, kRDFResource2,
                               sizeof(kRDFResource2) - 1);
        if (NS_FAILED(rv)) return rv;

        goto no_close_tag;
    }
    else if ((literal = do_QueryInterface(aMember)) != nsnull) {
        const PRUnichar *value;
        literal->GetValueConst(&value);
static const char kRDFLIOpenGT[] = ">";
        // close the '<RDF:LI' before adding the literal
        rv = rdf_BlockingWrite(aStream, kRDFLIOpenGT,
                               sizeof(kRDFLIOpenGT) - 1);
        if (NS_FAILED(rv)) return rv;

        NS_ConvertUTF16toUTF8 s(value);
        rdf_EscapeAmpersandsAndAngleBrackets(s);

        rv = rdf_BlockingWrite(aStream, s);
        if (NS_FAILED(rv)) return rv;
    }
    else if ((number = do_QueryInterface(aMember)) != nsnull) {
        PRInt32 value;
        number->GetValue(&value);

        nsCAutoString n;
        n.AppendInt(value);

        rv = rdf_BlockingWrite(aStream, kRDFParseTypeInteger, 
                               sizeof(kRDFParseTypeInteger) - 1);
        if (NS_FAILED(rv)) return rv;
        rv = rdf_BlockingWrite(aStream, n);
        if (NS_FAILED(rv)) return rv;
    }
    else if ((date = do_QueryInterface(aMember)) != nsnull) {
        PRTime value;
        date->GetValue(&value);

        nsCAutoString s;
        rdf_FormatDate(value, s);

        rv = rdf_BlockingWrite(aStream, kRDFParseTypeDate, 
                               sizeof(kRDFParseTypeDate) - 1);
        if (NS_FAILED(rv)) return rv;
        rv = rdf_BlockingWrite(aStream, s);
        if (NS_FAILED(rv)) return rv;
    }
    else {
        // XXX it doesn't support nsIRDFResource _or_ nsIRDFLiteral???
        // We should serialize nsIRDFInt, nsIRDFDate, etc...
        NS_WARNING("unknown RDF node type");

        rv = rdf_BlockingWrite(aStream, kRDFUnknown, sizeof(kRDFUnknown) - 1);
        if (NS_FAILED(rv)) return rv;
    }

    {
static const char kRDFLIClose[] = "</RDF:li>\n";
        rv = rdf_BlockingWrite(aStream, kRDFLIClose, sizeof(kRDFLIClose) - 1);
        if (NS_FAILED(rv)) return rv;
    }

 no_close_tag:
    return NS_OK;
}


nsresult
nsRDFXMLSerializer::SerializeContainer(nsIOutputStream* aStream,
                                         nsIRDFResource* aContainer)
{
    nsresult rv;
    nsCAutoString tag;

    // Decide if it's a sequence, bag, or alternation, and print the
    // appropriate tag-open sequence

    if (IsA(mDataSource, aContainer, kRDF_Bag)) {
        tag.AssignLiteral("RDF:Bag");
    }
    else if (IsA(mDataSource, aContainer, kRDF_Seq)) {
        tag.AssignLiteral("RDF:Seq");
    }
    else if (IsA(mDataSource, aContainer, kRDF_Alt)) {
        tag.AssignLiteral("RDF:Alt");
    }
    else {
        NS_ASSERTION(PR_FALSE, "huh? this is _not_ a container.");
        return NS_ERROR_UNEXPECTED;
    }

    rv = rdf_BlockingWrite(aStream, "  <", 3);
    if (NS_FAILED(rv)) return rv;
    rv = rdf_BlockingWrite(aStream, tag);
    if (NS_FAILED(rv)) return rv;


    // Unfortunately, we always need to print out the identity of the
    // resource, even if was constructed "anonymously". We need to do
    // this because we never really know who else might be referring
    // to it...

    nsCAutoString uri;
    if (NS_SUCCEEDED(aContainer->GetValueUTF8(uri))) {
        rdf_MakeRelativeRef(mBaseURLSpec, uri);

        rdf_EscapeAttributeValue(uri);

        if (uri.First() == '#') {
            // Okay, it's actually identified as an element in the
            // current document, not trying to decorate some absolute
            // URI. We can use the 'ID=' attribute...

            uri.Cut(0, 1); // chop the '#'
            rv = rdf_BlockingWrite(aStream, kIDAttr, sizeof(kIDAttr) - 1);
            if (NS_FAILED(rv)) return rv;
        }
        else {
            // We need to cheat and spit out an illegal 'about=' on
            // the sequence. 
            rv = rdf_BlockingWrite(aStream, kAboutAttr,
                                   sizeof(kAboutAttr) - 1);
            if (NS_FAILED(rv)) return rv;
        }

        rv = rdf_BlockingWrite(aStream, uri);
        if (NS_FAILED(rv)) return rv;
        rv = rdf_BlockingWrite(aStream, "\"", 1);
        if (NS_FAILED(rv)) return rv;
    }

    rv = rdf_BlockingWrite(aStream, ">\n", 2);
    if (NS_FAILED(rv)) return rv;

    // First iterate through each of the ordinal elements (the RDF/XML
    // syntax doesn't allow us to place properties on RDF container
    // elements).
    nsCOMPtr<nsISimpleEnumerator> elements;
    rv = NS_NewContainerEnumerator(mDataSource, aContainer, getter_AddRefs(elements));

    if (NS_SUCCEEDED(rv)) {
        while (1) {
            PRBool hasMore;
            rv = elements->HasMoreElements(&hasMore);
            if (NS_FAILED(rv)) break;

            if (! hasMore)
                break;

            nsCOMPtr<nsISupports> isupports;
            elements->GetNext(getter_AddRefs(isupports));

            nsCOMPtr<nsIRDFNode> element = do_QueryInterface(isupports);
            NS_ASSERTION(element != nsnull, "not an nsIRDFNode");
            if (! element)
                continue;

            SerializeMember(aStream, aContainer, element);
        }
    }

    // close the container tag
    rv = rdf_BlockingWrite(aStream, "  </", 4);
    if (NS_FAILED(rv)) return rv;
    tag.Append(">\n", 2);
    rv = rdf_BlockingWrite(aStream, tag);
    if (NS_FAILED(rv)) return rv;

    // Now, we iterate through _all_ of the arcs, in case someone has
    // applied properties to the bag itself. These'll be placed in a
    // separate RDF:Description element.
    nsCOMPtr<nsISimpleEnumerator> arcs;
    mDataSource->ArcLabelsOut(aContainer, getter_AddRefs(arcs));

    PRBool wroteDescription = PR_FALSE;
    while (! wroteDescription) {
        PRBool hasMore = PR_FALSE;
        rv = arcs->HasMoreElements(&hasMore);
        if (NS_FAILED(rv)) break;

        if (! hasMore)
            break;

        nsIRDFResource* property;
        rv = arcs->GetNext((nsISupports**) &property);
        if (NS_FAILED(rv)) break;

        // If it's a membership property, then output a "LI"
        // tag. Otherwise, output a property.
        if (! IsContainerProperty(property)) {
            rv = SerializeDescription(aStream, aContainer);
            wroteDescription = PR_TRUE;
        }

        NS_RELEASE(property);
        if (NS_FAILED(rv))
            break;
    }

    return NS_OK;
}


nsresult
nsRDFXMLSerializer::SerializePrologue(nsIOutputStream* aStream)
{
static const char kXMLVersion[] = "<?xml version=\"1.0\"?>\n";

    nsresult rv;
    rv = rdf_BlockingWrite(aStream, kXMLVersion, sizeof(kXMLVersion) - 1);
    if (NS_FAILED(rv)) return rv;

    // global name space declarations
    rv = rdf_BlockingWrite(aStream, NS_LITERAL_CSTRING("<RDF:RDF "));
    if (NS_FAILED(rv)) return rv;

    nsNameSpaceMap::const_iterator first = mNameSpaces.first();
    nsNameSpaceMap::const_iterator last = mNameSpaces.last();
    for (nsNameSpaceMap::const_iterator entry = first; entry != last; ++entry) {
        if (entry != first) {
            rv = rdf_BlockingWrite(aStream, NS_LITERAL_CSTRING("\n         "));
            if (NS_FAILED(rv)) return rv;
        }
        rv = rdf_BlockingWrite(aStream, NS_LITERAL_CSTRING("xmlns"));
        if (NS_FAILED(rv)) return rv;

        if (entry->mPrefix) {
            rv = rdf_BlockingWrite(aStream, NS_LITERAL_CSTRING(":"));
            if (NS_FAILED(rv)) return rv;
            nsCAutoString prefix;
            entry->mPrefix->ToUTF8String(prefix);
            rv = rdf_BlockingWrite(aStream, prefix);
            if (NS_FAILED(rv)) return rv;
        }

        rv = rdf_BlockingWrite(aStream, NS_LITERAL_CSTRING("=\""));
        if (NS_FAILED(rv)) return rv;
        nsCAutoString uri(entry->mURI);
        rdf_EscapeAttributeValue(uri);
        rv = rdf_BlockingWrite(aStream, uri);
        if (NS_FAILED(rv)) return rv;
        rv = rdf_BlockingWrite(aStream, NS_LITERAL_CSTRING("\""));
        if (NS_FAILED(rv)) return rv;
    }

    return rdf_BlockingWrite(aStream, NS_LITERAL_CSTRING(">\n"));
}


nsresult
nsRDFXMLSerializer::SerializeEpilogue(nsIOutputStream* aStream)
{
    return rdf_BlockingWrite(aStream, NS_LITERAL_CSTRING("</RDF:RDF>\n"));
}

class QNameCollector : public rdfITripleVisitor {
public:
    NS_DECL_ISUPPORTS
    NS_DECL_RDFITRIPLEVISITOR
    QNameCollector(nsRDFXMLSerializer* aParent)
        : mParent(aParent){}
private:
    nsRDFXMLSerializer* mParent;
};

NS_IMPL_ISUPPORTS1(QNameCollector, rdfITripleVisitor)
nsresult
QNameCollector::Visit(nsIRDFNode* aSubject, nsIRDFResource* aPredicate,
                      nsIRDFNode* aObject, PRBool aTruthValue)
{
    if (aPredicate == mParent->kRDF_type) {
        // try to get a type QName for aObject, should be a resource
        nsCOMPtr<nsIRDFResource> resType = do_QueryInterface(aObject);
        if (!resType) {
            // ignore error
            return NS_OK;
        }
        if (mParent->mQNames.Get(resType, nsnull)) {
            return NS_OK;
        }
        mParent->RegisterQName(resType);
        return NS_OK;
    }

    if (mParent->mQNames.Get(aPredicate, nsnull)) {
        return NS_OK;
    }
    if (aPredicate == mParent->kRDF_instanceOf ||
        aPredicate == mParent->kRDF_nextVal)
        return NS_OK;
    PRBool isOrdinal = PR_FALSE;
    mParent->gRDFC->IsOrdinalProperty(aPredicate, &isOrdinal);
    if (isOrdinal)
        return NS_OK;

    mParent->RegisterQName(aPredicate);

    return NS_OK;
}
    
nsresult
nsRDFXMLSerializer::CollectNamespaces()
{
    // Iterate over all Triples to get namespaces for subject resource types
    // and Predicates and cache all the QNames we want to use.
    nsCOMPtr<rdfITripleVisitor> collector = 
        new QNameCollector(this);
    nsCOMPtr<rdfIDataSource> ds = do_QueryInterface(mDataSource); // XXX API
    NS_ENSURE_TRUE(collector && ds, NS_ERROR_FAILURE);
    return ds->VisitAllTriples(collector);
}

//----------------------------------------------------------------------

NS_IMETHODIMP
nsRDFXMLSerializer::Serialize(nsIOutputStream* aStream)
{
    nsresult rv;
    NS_TIMELINE_START_TIMER("rdf/xml-ser");

    rv = CollectNamespaces();
    if (NS_FAILED(rv)) return rv;

    nsCOMPtr<nsISimpleEnumerator> resources;
    rv = mDataSource->GetAllResources(getter_AddRefs(resources));
    if (NS_FAILED(rv)) return rv;

    rv = SerializePrologue(aStream);
    if (NS_FAILED(rv))
        return rv;

    while (1) {
        PRBool hasMore = PR_FALSE;
        resources->HasMoreElements(&hasMore);
        if (! hasMore)
            break;

        nsCOMPtr<nsISupports> isupports;
        resources->GetNext(getter_AddRefs(isupports));

        nsCOMPtr<nsIRDFResource> resource = do_QueryInterface(isupports);
        if (! resource)
            continue;

        if (IsA(mDataSource, resource, kRDF_Bag) ||
            IsA(mDataSource, resource, kRDF_Seq) ||
            IsA(mDataSource, resource, kRDF_Alt)) {
            rv = SerializeContainer(aStream, resource);
        }
        else {
            rv = SerializeDescription(aStream, resource);
        }

        if (NS_FAILED(rv))
            break;
    }

    rv = SerializeEpilogue(aStream);
    NS_TIMELINE_STOP_TIMER("rdf/xml-ser");
    NS_TIMELINE_MARK("rdf/xml-ser");

    return rv;
}


PRBool
nsRDFXMLSerializer::IsA(nsIRDFDataSource* aDataSource, nsIRDFResource* aResource, nsIRDFResource* aType)
{
    nsresult rv;

    PRBool result;
    rv = aDataSource->HasAssertion(aResource, kRDF_instanceOf, aType, PR_TRUE, &result);
    if (NS_FAILED(rv)) return PR_FALSE;

    return result;
}
