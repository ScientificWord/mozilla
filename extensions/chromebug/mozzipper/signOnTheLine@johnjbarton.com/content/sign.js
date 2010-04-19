
var SignZipperExtension = {

    filelist: function(zip)
    {
        window.dump("SignZipperExtension filelist: "+zip.files.length+" zip.files\n");
    },

    scanForMetadata: function(zip)
    {
        window.dump("SignZipperExtension scanForMetadata: "+zip.files.length+" zip.files\n");
        var reUpdateRDF = /update\.rdf$/;
        var reUpdateRDFTemplate = /update\.rdf\.tpl\.xml/;
        var reInstallRDFTemplate = /install\.rdf\.tpl\.xml/;
        var reInstallRDF  = /install\.rdf$/;
        var reBranchProperties = /branch\.properties$/;

        for (var i = 0; i < zip.files.length; i++)
        {
            var filename = zip.files[i].leafName;
            //FBTrace.sysout("SignZipperExtension trying leaf \'"+filename+"\'\n");
            if (reUpdateRDF.test(filename))
            {
                this.updateRDF = zip.files[i];
                window.dump("SignZipperExtension found update.rdf "+i+") "+zip.files[i].path+"\n");
            }
            if (reUpdateRDFTemplate.test(filename))
            {
                this.updateRDFTemplate = zip.files[i];
                window.dump("SignZipperExtension found update.rdf template "+i+") "+zip.files[i].path+"\n");
            }
            if (reInstallRDFTemplate.test(filename))
            {
                this.installRDFTemplate = zip.files[i];
                window.dump("SignZipperExtension found install.rdf template "+i+") "+zip.files[i].path+"\n");
            }
            if (reInstallRDF.test(filename))
            {
                this.installRDF  = zip.files[i];
                window.dump("SignZipperExtension found install.rdf  "+i+") "+zip.files[i].path+"\n");
            }
            if (reBranchProperties.test(filename))
            {FBTrace.sysout("SignZipperExtension this.propertiesLoaded "+this.propertiesLoaded+"\n"); 
                if (!this.propertiesLoaded)
                    this.loadProperties(zip.files[i], zip);
                else
                    window.dump("SignZipperExtension got properities, skipping "+filename+"\n"); 
                this.propertiesLoaded = true;
            }
        }
    },

    loadProperties: function(file, zip)
    {
        var input = Zipper.getLines(file);
        FBTrace.sysout("sign.loadProperties "+input.length+" lines from "+file.path+"\n");
        var reKeyValue = /\s*(\w*)\s*=\s*(\S*)/;
        for (var i = 0; i < input.length; i++)
        {
            var line = input[i];
            var m = reKeyValue.exec(line);
            if (m)
            {
                var key = m[1];
                var value = m[2];
                zip[key] = value;
                FBTrace.sysout("Read "+key+"="+value+" from "+file.path+"\n");
            }
            else
                FBTrace.sysout("sign.loadProperties("+file.path+") not a key value:"+line+"\n");
        }
    },

    xpiFileName: function(zip)
    {
        this.scanForMetadata(zip);

        this.keyName = window.keyName;
        if (this.keyName)
        {
            var leaf = this.keyName;
            var version = zip['VERSION'];
            if (version)
            	leaf += "-" + version;
            var release = zip['RELEASE'];
            if (release)
                leaf += release;
            leaf += ".xpi";
            var targetDirectory = new Directory(zip.name);
            var zipFile = targetDirectory.directory.clone();
            zipFile.append(leaf);
            zip.name = zipFile.path;
            zip['LEAF'] = leaf;
            
            var key = getKey(keyName);
            zip['PUBLICKEY'] = key.exportPublicKey();
        }
        window.dump("SignZipperExtension zip.name: "+zip.name+ " zip.LEAF:"+zip.LEAF+"\n");
        
        if (this.installRDFTemplate)
        {
        	var sourceDirectory = new Directory(this.installRDFTemplate.path);
            var file = sourceDirectory.openNewFile("install.rdf");
            window.dump("SignZipperExtension expanding install.rdf template\n");

            if (this.installRDF)
            {
            	var i = zip.files.indexOf(this.installRDF);
            	if (i != -1)
            		zip.files.splice(i, 1);
            	else 
            		throw "mozzipper.sign found an install.rdf at "+this.installRDF.path+" but later it was not on the files list";
            }
            
            this.installRDF = this.expandTemplate(this.installRDFTemplate, zip, file);
 
            var i_template = zip.files.indexOf(this.installRDFTemplate);
            if (i_template != -1)
            	zip.files[i_template] = this.installRDF;
            else
            	throw "mozzipper.sign found an install.rdf.tpl.xml at "+this.installRDFTemplate.path+" but could not remove it from the files list";
        }
    },

    xpi: function(zip)
    {
        var targetDirectory = new Directory(zip.name);
        if (this.updateRDFTemplate)
        {

            var file = targetDirectory.openNewFile("update.rdf");
            window.dump("SignZipperExtension expanding update.rdf template\n");
            this.updateRDF = this.expandTemplate(this.updateRDFTemplate, zip, file);
        }



        window.dump("SignZipperExtension signing update: "+this.updateRDF.path+" with "+this.keyName+" and hash from "+zip.zipfile.path+"\n");
        if (this.updateRDF && this.keyName)
        {
             var fileHandler = iosvc.getProtocolHandler("file")
                 .QueryInterface(Components.interfaces.nsIFileProtocolHandler);
            var updateRDFURL = fileHandler.getURLSpecFromFile(this.updateRDF);
            var xpiFile = targetDirectory.directory.clone();
            xpiFile.append(zip.LEAF);
            var xpiFileURL = fileHandler.getURLSpecFromFile(xpiFile);
            doSign(updateRDFURL, xpiFileURL, this.keyName);
        }
    },

    expandTemplate: function(templateFile, object, outputFile)
    {
        var input = Zipper.getLines(templateFile);
        var output = [];
        var reAts = /@([^@]*?)@/;
        for (var i = 0; i < input.length; i++)
        {
            var line = input[i];
            FBTrace.sysout("sign.expandTemplate trying "+line+"\n");
            var matches;
            while ((matches = reAts.exec(line)) != null)
            {
                var key = matches[1];
                line = line.replace('@'+key+'@', object[key]);
            }
            output.push(line);
            FBTrace.sysout("sign.expandTemplate result "+line+"\n");
        }
        Zipper.writeString(outputFile, output.join('\n'));
        return outputFile;
    },

}


/*
 * Sign the update.rdf file given by name on the command line
 */
function doSign(filename, addOnFileName, keyName)
{

    var key = getKey(keyName);

    try
    {
        // The update.rdf file as an RDF data structure
        //
        var rdf = gRDF.GetDataSourceBlocking(filename).QueryInterface(Ci.nsIRDFRemoteDataSource);
        if (rdf)
        {
            if (addOnFileName)
            {
                if (insertUpdateHash(rdf, addOnFileName))
                {
                    window.dump("Hashed XPI "+addOnFileName+" and inserted into "+filename+"\n");
                    var beCommandLineOnly = true;
                }
                else
                    return;
            }
            else
                window.dump("signOnTheLine: no addOnFileName given, cannot add updateHash\n");

            if (key)
            {
                signUpdate(rdf, key);
                rdf.Flush();
                gRDF.UnregisterDataSource(rdf);
                window.dump("Signed "+filename+" with "+key+"\n");
                var beCommandLineOnly = true;
            }
            else
                window.dump("signOnTheLine: no keyname given, cannot add digital signature to "+filename+"\n");

            if (beCommandLineOnly)  // then our work is done here
                window.close();
            // else bring up the UI
        }
    }
    catch (exc)
    {
        window.dump("Mccoy sign FAILED for filename:"+filename+" because:"+exc+"\n");
    }
}

var DBG_SOTL = false;
var gRDF = Components.classes["@mozilla.org/rdf/rdf-service;1"].
         getService(Ci.nsIRDFService);

function EM_R(property) {
  return gRDF.GetResource(EM_NS(property));
}

function getLinkedResources(rdf)
{
    var linkedResources = [];
    var resources = rdf.GetAllResources();
    var updateLinkArc = gRDF.GetResource(EM_NS("updateLink")).QueryInterface(Ci.nsIRDFResource);
    while (resources.hasMoreElements())
    {
        var res = resources.getNext().QueryInterface(Ci.nsIRDFResource);
        if (DBG_SOTL) window.dump("dumpResources: "+ res.Value+"\n");

        var arcs = rdf.ArcLabelsOut(res);
        while (arcs.hasMoreElements())
        {
            var arc = arcs.getNext().QueryInterface(Ci.nsIRDFResource);

            if (arc.Value.indexOf("updateLink") != -1)  // HACK: this should be RDFish test of some sort.
            {
                if (DBG_SOTL) window.dump(arc.Value+" vs "+updateLinkArc.Value+"\n");
                linkedResources.push(res);
            }
        }
    }
    return linkedResources;
}

function getUpdates(rdf, extensionRes)
{
    var updatesArc = gRDF.GetResource(EM_NS("updates"));
    var updates = rdf.GetTargets(extensionRes, updatesArc, true);
    return updates;
}

/*
 * Look up the PKI key by name given on the command line
 *
 */
function getKey(keyName)
{
    var incoming = keyName;
    if (!incoming)
        throw "usage: McCoy -sign <file-url> -key <keyname>";

    var name = incoming.toLowerCase();

    var items = gList.children;
    var allKeys = "";
    for (var i = 0; i < items.length; i++)
    {
        var itemName = items[i].key.name.toLowerCase();
        allKeys += itemName + " ";
        if (itemName == name)
        {
            return items[i].key;
        }
    }
    throw "McCoy -sign <file-url> -key <keyname> found no key named \'"+name+"\' in "+allKeys+"\n";
}

/**
 * Inserts an updateHash value into the manifest for each addon
 * @param rdf an nsIRDFDataSource for the update.rdf
 * @param folderName string folder name containing zip files
 */
function insertUpdateHash(rdf, addOnFileName)
{
    var linkedResources = getLinkedResources(rdf);

    // We are putting the same hash on all resources with an updateLink.
    // What we should do is get the files name from the updateLink and hash it.
    //
    var hash = getUpdateHash(addOnFileName);  // TODO get the file name from a local version of the link
    if (!hash)
        return false;

    if (DBG_SOTL) window.dump("signOnTheLine insertUpdateHash gets "+hash+" for "+addOnFileName+"\n");

    for (var i = 0; i < linkedResources.length; i++)
    {
        var resource = linkedResources[i];
        unassertAll(rdf, resource, EM_NS("updateHash"));
        var hashInRDF = gRDF.GetLiteral(hash);
        rdf.Assert(resource, EM_NS("updateHash"), hashInRDF, true);
    }
    return true;
}

/**
 * Compute the updateHash fo a given zip file
 * @param filename zip (XPI) filename
 * @return string hash starting with 'sha256:'
 */
function getUpdateHash(filename)
{
    var stream = getStream(filename);
    if (stream)
        return getSHA256ForStream(stream);
}

function getStream(filename)
{
    // Read the file and run it through cryptohash
    try
    {
        var ios = Components.classes["@mozilla.org/network/io-service;1"]
                .getService(Components.interfaces.nsIIOService);
        var url = ios.newURI(filename, null, null);

        if (!url || !url.schemeIs("file")) throw "Expected a file URL.";

        window.dump("signOnTheLine.getUpdateHash xpi url:"+url.spec+"\n");

        var channel = ios.newChannel(filename, null, null);
        var stream = channel.open();
        return stream;
     }
     catch (exc)
     {
        window.dump("signOnTheLine getSteam("+filename+") FAILS:"+exc+"\n");
        var fileHandler = ios.getProtocolHandler("file")
                 .QueryInterface(Components.interfaces.nsIFileProtocolHandler);
        var file = fileHandler.getFileFromURLSpec(filename);
        window.dump("signOnTheLine could not hash file: "+filename);
        if (file.exists())
            window.dump(" which exists ");
        else {
            window.dump(" which does not exist.\n");
            return false;
        }
        if (file.isReadable())
            window.dump(" and is readable.\n");
        else
            window.dump(" and is not readable.\n");
        return false;
     }
 }

 function getSHA256ForStream(stream)
 {
     try
     {
        var hasher = getHashService();

        // http://developer.mozilla.org/en/docs/nsICryptoHash#Computing_the_Hash_of_a_File

        const PR_UINT32_MAX = 0xffffffff; // this tells updateFromStream to read the entire file

        hasher.updateFromStream(stream, PR_UINT32_MAX);

        var hash = hasher.finish(false);  // pass false here to get binary data back

        // convert the binary hash data to a hex string.
        var s = [toHexString(hash.charCodeAt(i)) for (i in hash)].join("");

        return "sha256:"+s; // s now contains your hash in hex
    }
    catch(exc)
    {
        window.dump("signOnTheLine getUpdateHash FAILS:"+exc+"\n");
    }
}

// return the two-digit hexadecimal code for a byte
function toHexString(charCode)
{
  return ("0" + charCode.toString(16)).slice(-2);
}

function getHashService() {
    try {
        var nsICryptoHash = Components.interfaces["nsICryptoHash"];
        // ask user for permission
        netscape.security.PrivilegeManager.enablePrivilege('UniversalXPConnect');
        var hash_service = Components.classes["@mozilla.org/security/hash;1"];
        var hasher = hash_service.getService(nsICryptoHash);
        hasher.init(nsICryptoHash.SHA256);
        return hasher;
    } catch (e) {
        // user refused permission
        window.dump('Permission connect to '+nsICryptoHash+' was denied: '+e+"\n");
    }
}

/* See license.txt for terms of usage */
/* John J. Barton johnjbarton@johnjbarton.com March 2007 */
