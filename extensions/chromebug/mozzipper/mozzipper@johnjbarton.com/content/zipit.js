
function ZipFile(name, directory, files)
{
    this.name = name;
    this.files = files;
    this.rootDirectory = directory;
}
const PR_RDONLY      = 0x01;
const PR_WRONLY      = 0x02;
const PR_RDWR        = 0x04;
const PR_CREATE_FILE = 0x08;
const PR_APPEND      = 0x10;
const PR_TRUNCATE    = 0x20;
const PR_SYNC        = 0x40;
const PR_EXCL        = 0x80;

    // Return recursively built up list of files as array of strings.

ZipFile.prototype.buildFileList = function()
{
    var root = new Directory(this.rootDirectory);
    this.files = root.getAllFiles();
}

// Zip file list, return closed zip file.
ZipFile.prototype.zipAll = function()
{
    var zipWriter = Components.Constructor("@mozilla.org/zipwriter;1", "nsIZipWriter");
    var writer = new zipWriter();

    var xpi = Components.classes["@mozilla.org/file/local;1"]
                .createInstance(Components.interfaces.nsILocalFile);
    var path = this.name;
    path = path.replace(/\//g, "\\");  // HACK!
    xpi.initWithPath(path);

    try
    {
        writer.open(xpi, PR_RDWR | PR_CREATE_FILE | PR_TRUNCATE);
    }
    catch (exc)
    {
        if (exc.name == "NS_ERROR_FILE_NOT_FOUND")
        {
            FBTrace.sysout("ZipFile.zipall FAILS for file", xpi);
            throw "The file "+xpi.path+" was not found (or its directory does not exist) and the flags "+(PR_RDWR | PR_CREATE_FILE | PR_TRUNCATE)+" didn't permit creating it."
        }
        else
            throw exc;
    }

    var prefixLength = this.rootDirectory.length + 1;
    var allDirectories = [];
    for (var i = 0; i < this.files.length; i++)
    {
        var file = this.files[i];
        var absPath = file.path;
        var parentPath = file.parent.path;
        if (allDirectories.indexOf(parentPath) == -1)
        {
            var relPath = parentPath.substr(prefixLength).replace(/\\/g,"/");
            if (this.debug) FBTrace.sysout("zipAll "+i+") "+relPath);
            if (relPath.length > 1)
            {
                if (this.debug) FBTrace.sysout(" adding directory \n");
                writer.addEntryFile(relPath, Components.interfaces.nsIZipWriter.COMPRESSION_DEFAULT, file.parent, false);
                allDirectories.push(parentPath);
            }
            else
                if (this.debug) FBTrace.sysout(" skipping directory \n");
        }
        var relPath = absPath.substr(prefixLength).replace(/\\/g,"/");

        if (this.debug) FBTrace.sysout("zipAll "+i+") "+relPath);
        if (relPath.length < 1)
        {
            if (this.debug) FBTrace.sysout(" skipping \n");
            continue;
        }
        else
            if (this.debug) FBTrace.sysout(" adding \n");

        try 
        {
        	writer.addEntryFile(relPath, Components.interfaces.nsIZipWriter.COMPRESSION_DEFAULT, file, false);
        } 
        catch (exc)
        {
        	FBTrace.sysout("zipit FAILS nsIZipWriter addEntryFile relPath:"+relPath+" file:"+file.path, exc);
        	throw exc;
        }
    }
    writer.close();
    this.zipfile = xpi;
    return this.zipfile;
}

function Directory(path)
{
    try
    {
        this.directory = Components.classes["@mozilla.org/file/local;1"]
                 .createInstance(Components.interfaces.nsILocalFile);
                 path = path.replace(/\//g, "\\");  // HACK!
        this.directory.initWithPath(path);
        if (!this.directory.exists() || !this.directory.isDirectory())
            this.directory = this.directory.parent;
    }
    catch (exc)
    {
    	if (path)
    		throw "zipit.Directory("+path+") :"+exc;
    	else
    		throw "zipit.Directory called with no path";
    }
}

Directory.prototype.getAllFiles = function()
{
    var array = [];
    this.appendFiles(array, this.directory.directoryEntries);
    return array;
}

Directory.prototype.appendFiles = function(array, entries)
{
    while(entries.hasMoreElements())
    {
        var entry = entries.getNext();
        if (entry instanceof Ci.nsIFile)
        {
            if (entry.isDirectory())
                this.appendFiles(array, entry.directoryEntries);
            if (entry.isFile())
                array.push(entry);
        }
    }
}

Directory.prototype.openNewFile = function(leaf, nocreate)
{
    try
    {
        var file= Components.classes["@mozilla.org/file/local;1"]
                        .createInstance(Components.interfaces.nsILocalFile);
        file.initWithPath(this.directory.path)
        file.appendRelativePath(leaf);
        if( !nocreate && !file.exists() )    // if it doesn't exist, create
            file.create(Components.interfaces.nsIFile.NORMAL_FILE_TYPE, 664);
    }
    catch (exc)
    {
        throw "zipit.Directory.openNewFilew FAILS for leaf "+leaf+" in directory "+this.directory.path + " because "+exc.message;
    }
    return file;
}

var Zipper  = {
    extensions: [],

// Register extensions here

    registerExtension: function(ext)
    {
        this.extensions.push(ext);
    },


// Overall processing

    zipit: function(rootDirectory, xpiName)
    {
    	if (!rootDirectory)
    		throw "mozzipper zipit: no source directory";
    	if (!xpiName)
    		throw "mozziper zipit: no target file name";
    	
        this.registerExtension(PrintZipperExtension); // the second copy is last, so it gets called after the extensions
        this.dispatch(this.extensions, "debugFlags", ["xpi"]);

        var xpi = new ZipFile(xpiName, rootDirectory);
        
        xpi.buildFileList();
        this.dispatch(this.extensions, "filelist", [xpi]);

        this.dispatch(this.extensions, "xpiFileName", [xpi]);

        xpi.zipAll();

        this.dispatch(this.extensions, "xpi", [xpi]);
        return xpi;
    },

    //   Pass thru extensions.
    dispatch: function(extensions, name, args)
    {
        window.dump("zipper.dispatch "+name+" to "+extensions.length+" extensions\n");
       
            for (var i = 0; i < extensions.length; ++i)
            {
                var extension = extensions[i];
                if ( extension.hasOwnProperty(name) )
                {
                	try 
                	{
                		extension[name].apply(extension, args);
                	}
                	catch (exc)
                    {
                    	FBTrace.sysout(" Exception in mozzipper.dispatch "+ name, exc);
                    	FBTrace.sysout(" Exception in mozzipper.dispatch "+ name+" For extension ", extension);
                        FBTrace.sysout(" Exception in mozzipper.dispatch "+ name, exc);
                        throw "mozziper dispatch "+name+" FAILS: "+exc;
                    }
                } 
            }
    },

    istream: Components.classes["@mozilla.org/network/file-input-stream;1"]
                    .createInstance(Components.interfaces.nsIFileInputStream),

    getLines: function(file)
    {
        this.istream.init(file, 0x01, 0444, 0);
        if (! (this.istream instanceof Ci.nsILineInputStream) )
            throw "Zipper.getLines @mozilla.org/network/file-input-stream;1 FAILS to give nsILineInputStream";

        var buf = {};
        var hasmore;
        var input = [];
        do
        {
            hasmore = this.istream.readLine(buf);
            input.push(buf.value);
        } while(hasmore);

        this.istream.close();
        return input;
    },
    
    writeString: function(file, data)
    {
        // http://developer.mozilla.org/en/docs/Code_snippets:File_I/O
        //file is nsIFile, data is a string
        var foStream = Components.classes["@mozilla.org/network/file-output-stream;1"]
                         .createInstance(Components.interfaces.nsIFileOutputStream);

        // use 0x02 | 0x10 to open file for appending.
        foStream.init(file, 0x02 | 0x08 | 0x20, 0666, 0); // write, create, truncate
        foStream.write(data, data.length);
        foStream.close();
    },
}

var PrintZipperExtension = {

    debugFlags: function(phase)
    {
        this.debugPhase = phase;
    },

    filelist: function(zip)
    {
        if (this.debugPhase != "filelist")
            return;

        window.dump("PrintZipperExtension filelist: "+zip.files.length+" files\n");
        for (var i = 0; i < zip.files.length; i++)
        {
            if (this.debug) window.dump(i+") "+zip.files[i].path+"\n");
        }
    },

    xpiFileName: function(zip)
    {
        if (this.debugPhase != "xpiFileName")
            return;

        FBTrace.sysout("PrintZipperExtension xpiFileName: ",zip.name);
    },

    xpi: function(zip)
    {
       if (this.debugPhase != "xpi")
            return;

        FBTrace.sysout("PrintZipperExtension xpi phase zip: ",zip.zipfile.path);
    },
}

var NoSvnZipperExtension = {
    debug: false,

    filelist: function(zip)
    {
        if (this.debug) window.dump("NoSvnZipperExtension input filelist: "+zip.files.length+" files\n");
        var cleanFiles = [];
        var reSVN = /(\\|\/)\.svn(\\|\/)/;
        for (var i = 0; i < zip.files.length; i++)
        {
            if (!reSVN.test(zip.files[i].path))
            {
                cleanFiles.push(zip.files[i]);
            }
            else
            {
                if (this.debug) window.dump("Removing .svn file:"+i+") "+zip.files[i].path+"\n");
            }
        }
        zip.files = cleanFiles;
        if (this.debug) window.dump("NoSvnZipperExtension output filelist: "+zip.files.length+" files\n");
    },

    xpiFileName: function(zip)
    {
        if (this.debug) window.dump("NoSvnZipperExtension xpiFileName: "+zip.name+"\n");
    },

    xpi: function(zip)
    {
        if (this.debug) window.dump("NoSvnZipperExtension zip: "+zip.name+"\n");
    },
}


Zipper.registerExtension(PrintZipperExtension);