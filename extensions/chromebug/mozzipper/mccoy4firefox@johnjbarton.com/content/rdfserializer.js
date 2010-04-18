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
 * The Original Code is McCoy.
 *
 * The Initial Developer of the Original Code is
 * the Mozilla Foundation <http://www.mozilla.org/>.
 * Portions created by the Initial Developer are Copyright (C) 2007
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Dave Townsend <dtownsend@oxymoronical.com>
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

/**
 * A serialisation method for RDF data that produces an identical string
 * provided that the RDF assertions match.
 * The serialisation is not complete, only assertions stemming from a given
 * resource are included, multiple references to the same resource are not
 * permitted, and the RDF prolog and epilog are not included.
 * RDF Blob and Date literals are not supported.
 */
function RDFSerializer()
{
  this.cUtils = Cc["@mozilla.org/rdf/container-utils;1"].
                getService(Ci.nsIRDFContainerUtils);
  this.resources = [];
}

RDFSerializer.prototype = {
  INDENT: "  ",      // The indent used for pretty-printing
  resources: null,   // Array of the resources that have been found
  
  /**
   * Escapes characters from a string that should not appear in XML.
   * @param string     The string to be escaped
   * @returns a string with all characters invalid in XML character data
   *          converted to entity references.
   */
  escapeEntities: function(string)
  {
    string = string.replace(/&/g, "&amp;");
    string = string.replace(/</g, "&lt;");
    string = string.replace(/>/g, "&gt;");
    string = string.replace(/"/g, "&quot;");
    return string;
  },
  
  /**
   * Serializes all the elements of an RDF container.
   * @param ds         The datasource holding the data
   * @param container  The RDF container to output the child elements of
   * @param indent     The current level of indent for pretty-printing
   * @returns a string containing the serialized elements.
   */
  serializeContainerItems: function(ds, container, indent)
  {
    var result = "";
    var items = container.GetElements();
    while (items.hasMoreElements()) {
      var item = items.getNext().QueryInterface(Ci.nsIRDFResource);
      result += indent + "<RDF:li>\n"
      result += this.serializeResource(ds, item, indent + this.INDENT);
      result += indent + "</RDF:li>\n"
    }
    return result;
  },
  
  /**
   * Serializes all em:* (see EM_NS) properties of an RDF resource except for
   * the em:signature property. As this serialization is to be compared against
   * the manifest signature it cannot contain the em:signature property itself.
   * @param ds         The datasource holding the data
   * @param resource   The RDF resource to output the properties of
   * @param indent     The current level of indent for pretty-printing
   * @returns a string containing the serialized properties.
   */
  serializeResourceProperties: function(ds, resource, indent)
  {
    var result = "";
    var items = [];
    var arcs = ds.ArcLabelsOut(resource);
    while (arcs.hasMoreElements()) {
      var arc = arcs.getNext().QueryInterface(Ci.nsIRDFResource);
      if (arc.ValueUTF8.substring(0, PREFIX_NS_EM.length) != PREFIX_NS_EM)
        continue;
      var prop = arc.ValueUTF8.substring(PREFIX_NS_EM.length);
      if (prop == "signature")
        continue;
  
      var targets = ds.GetTargets(resource, arc, true);
      while (targets.hasMoreElements()) {
        var target = targets.getNext();
        if (target instanceof Ci.nsIRDFResource) {
          var item = indent + "<em:" + prop + ">\n";
          item += this.serializeResource(ds, target, indent + this.INDENT);
          item += indent + "</em:" + prop + ">\n";
          items.push(item);
        }
        else if (target instanceof Ci.nsIRDFLiteral) {
          items.push(indent + "<em:" + prop + ">" + this.escapeEntities(target.Value) + "</em:" + prop + ">\n");
        }
        else if (target instanceof Ci.nsIRDFInt) {
          items.push(indent + "<em:" + prop + " NC:parseType=\"Integer\">" + target.Value + "</em:" + prop + ">\n");
        }
        else {
          throw new Error("Cannot serialize unknown literal type");
        }
      }
    }
    items.sort();
    result += items.join("");
    return result;
  },
  
  /**
   * Recursively serializes an RDF resource and all resources it links to.
   * This will only output EM_NS properties and will ignore any em:signature
   * property.
   * @param ds         The datasource holding the data
   * @param resource   The RDF resource to serialize
   * @param indent     The current level of indent for pretty-printing.
   *                   Leave undefined for no indent
   * @returns a string containing the serialized resource.
   * @throws if the RDF data contains multiple references to the same resource.
   */
  serializeResource: function(ds, resource, indent)
  {
    if (this.resources.indexOf(resource) != -1 ) {
      // We cannot output multiple references to the same resource.
      throw new Error("Cannot serialize multiple references to "+resource.Value);
    }
    if (indent === undefined)
      indent = "";
    
    this.resources.push(resource);
    var container = null;
    var type = "Description";
    if (this.cUtils.IsSeq(ds, resource)) {
      type = "Seq";
      container = this.cUtils.MakeSeq(ds, resource);
    }
    else if (this.cUtils.IsAlt(ds, resource)) {
      type = "Alt";
      container = this.cUtils.MakeAlt(ds, resource);
    }
    else if (this.cUtils.IsBag(ds, resource)) {
      type = "Bag";
      container = this.cUtils.MakeBag(ds, resource);
    }
  
    var result = indent + "<RDF:" + type;
    if (!gRDF.IsAnonymousResource(resource))
      result += " about=\"" + this.escapeEntities(resource.ValueUTF8) + "\"";
    result += ">\n";
  
    if (container)
      result += this.serializeContainerItems(ds, container, indent + this.INDENT);
      
    result += this.serializeResourceProperties(ds, resource, indent + this.INDENT);
  
    result += indent + "</RDF:" + type + ">\n";
    return result;
  }
}
