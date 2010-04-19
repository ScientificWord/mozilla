// Detrace: remove Firebug tracing statements.
// If you understand parsing of brace matching languages, stop reading now.

try {
const iosvc = Components.classes["@mozilla.org/network/io-service;1"].getService(Components.interfaces.nsIIOService);
const chromeReg = Components.classes["@mozilla.org/chrome/chrome-registry;1"].getService(Components.interfaces.nsIToolkitChromeRegistry);


var DetraceZipperExtension = {
    reJS: /\.js$/,
    reXUL: /\.xul$/,
    reBranchProperties: /branch\.properties/,

    debug: false,

    filelist: function(zip)
    {
        if (DetraceZipperExtension.debug) window.dump("DetraceZipperExtension filelist: "+zip.files.length+" zip.files\n");

        var tmp = this.getTmpFolder();
        if (DetraceZipperExtension.debug) window.dump("Begin deTrace with tmp.path: "+ tmp.path+"\n");
        var outRoot = new Directory(tmp.path);

        var prefixLength = zip.rootDirectory.length + 1;

        // Transform the zip.files by detrace
        for (var i = 0; i < zip.files.length; i++)
        {
            var file = zip.files[i];

            var path = file.path.substr(prefixLength);
            if (path.length < 1)
                continue;

            if (this.reJS.test(path)) //  && (path.indexOf("lib.js") != -1) )
            {
                var outputFile = outRoot.openNewFile(path);
                if (DetraceZipperExtension.debug) window.dump("detrace from "+file.path+" to "+outputFile.path+"\n");
                zip.files[i] = deTraceFile(file, outputFile);
            }
            else if (this.reXUL.test(path))
            {
                var outputFile = outRoot.openNewFile(path);
                if (DetraceZipperExtension.debug) window.dump("detrace deExplore from "+file.path+" to "+outputFile.path+"\n");
                zip.files[i] = filterLines(file, deExploreLine,  outputFile);
            }
            else if (this.reBranchProperties.test(path))
            {
                var outputFile = outRoot.openNewFile(path);
                if (DetraceZipperExtension.debug) window.dump("detrace deX from "+file.path+" to "+outputFile.path+"\n");
                zip.files[i] = filterLines(file, deXLine, outputFile);
            }
            else
            {
                var outputFile = outRoot.openNewFile(path, true);
                if (outputFile && outputFile.parent)
                {
                    try
                    {
                        file.copyTo(outputFile.parent, "");
                        zip.files[i] = outputFile;
                    }
                    catch (exc)
                    {
                        throw "detrace.filelist FAILS for "+file.path+".copyTo("+outputFile.parent.path+") cause: "+exc.message;
                    }
                }
                else
                    throw "detrace.filelist FAILS to open "+path+" in directory "+outRoot.path+" because the nsIFile or its .parent is null\n";
            }
        }
        zip.rootDirectory = outRoot.directory.path;
    },

    xpiFileName: function(zip)
    {
        if (DetraceZipperExtension.debug) window.dump("DetraceZipperExtension xpiFileName: "+zip.name+"\n");
    },

    xpi: function(zip)
    {
        if (DetraceZipperExtension.debug) window.dump("DetraceZipperExtension xpi.path: "+zip.zipfile.path+"\n");
    },
    //***************************************************

    getTmpFolder: function()
    {
        try 
        {
            var tmp = Components.classes["@mozilla.org/file/directory_service;1"]
                     .getService(Components.interfaces.nsIProperties)
                     .get("TmpD", Components.interfaces.nsIFile);
            tmp.append("deTrace");

            if (tmp.exists())
                tmp.remove(true);

            if( !tmp.exists() )
                tmp.createUnique(Components.interfaces.nsIFile.DIRECTORY_TYPE, 0777);
        
            return tmp;
        }
        catch (e)
        {
            FBTrace.sysout("detrace.getTmpFolder FAILS "+e, tmp);            
        }
    },
}

var reComment = /\/\*[^\*]*[\*]\//; // /*anything*/
var reOpeningBrace = /\s*{/;
var reBrace =  /}|{/g;
var reFBTrace = /\s*if\s*\((\s*FBTrace[^\)]*)\)/; // if (FBTrace.DBGblah) capture expression

function SourceIterator(lines) {
    this.input = lines;
    this.i = 0;
}

SourceIterator.prototype.hasNext = function()
{
    return this.i < this.input.length;
}

SourceIterator.prototype.next = function()
{
    var line = this.input[this.i++];
    if (DetraceZipperExtension.debug) window.dump(this.input.length+"|"+this.i+") "+line+"\n");
    return line;
}

SourceIterator.prototype.prev = function()
{
    return this.input[this.i--];
}

function stripComments(raw)
{
    if (typeof(raw) == "undefined")
        return false;
    var block = raw.replace(reComment, "").replace(/\/\/(.*)/, "");
    return block;
}

function LineSink(outFile)
{
    this.foStream = Components.classes["@mozilla.org/network/file-output-stream;1"]
                     .createInstance(Components.interfaces.nsIFileOutputStream);

    //PR_RDWR    0x04    Open for reading and writing.
    //PR_CREATE_FILE     0x08   If the file does not exist, the file is created. If the file exists, this flag has no effect.
    //PR_TRUNCATE    0x20   If the file exists, its length is truncated to 0.
    if (outFile instanceof Ci.nsILocalFile)
        this.foStream.init(outFile, 0x04 | 0x08 | 0x20, 664, 0);
    else
        throw "deTraceFile FAILS: outFile is not an nsILocalFile";

    if (!outFile.isWritable())
        throw("deTraceFile init stream FAILS output file is not writable"+outFile.path);
}

LineSink.prototype.put = function(line)
{
    this.foStream.write(line+"\n", line.length+1);
}

LineSink.prototype.close = function()
{
    this.foStream.close();
}

// Remove lines marked /*@explore*/
var reExplore = /\/\*@explore\*\//;
function deExploreLine(line)
{
    if (reExplore.test(line))
    {
        window.dump("deExplore dropping "+line+"\n");
        return null;
    }
    return line;
}

// Remove X from strings containing eg a30X
var reX = /([0-9]*)X/;
function deXLine(line)
{
    var m = reX.exec(line);
    if (m)
    {
        window.dump("deXLine replace on "+line+"\n");
        return line.replace(m[0], m[1]);
    }
    return line;
}

function filterLines(inFile, filter, outFile)
{
    var input = Zipper.getLines(inFile);
    var output = new LineSink(outFile);
    var iter = new SourceIterator(input);
    while (iter.hasNext())
    {
         var line = filter(iter.next());
         if (line) // skip null lines
             output.put(line);
    }
    output.close();
    return outFile;
}

function deTraceFile(inFile, outFile)
{
    var input = Zipper.getLines(inFile);
    var output = new LineSink(outFile);

    var iter = new SourceIterator(input);
    while (iter.hasNext())
    {
        var line = iter.next();
        var clean = stripComments(line);
        var m = reFBTrace.exec(clean);
        if (m)
        {
            var point = m[0].length;
            if (DetraceZipperExtension.debug) window.dump("deTraceFile found FBTrace expression: "+m[1]+" point:"+point+"\n");

            var block = clean.substr(point);

            skip(block, iter);

            // check for 'else'
            block = stripComments(iter.next());
            if (block === false) return;
            if (DetraceZipperExtension.debug) window.dump("consider else block:"+block+"\n");
            var whatFollows = false;
            while ( ! (whatFollows = /\S/.exec(block) ) )
            {
                block = stripComments(iter.next());
                if (block === false) return;
                if (DetraceZipperExtension.debug) window.dump("consider else block:"+block+"\n");
            }
            // block has something besides whitespace

            m = /else\s(.*)/.exec(block);
            if (m)
                skip(m[1], iter);
            else
            {
                iter.prev();  // put it back
                if (DetraceZipperExtension.debug) window.dump("not else\n");
            }
        }
        else
        {
            output.put(line);
        }
    }
    output.close();
    return outFile;
}

function skip(block, iter) // Don't say I didn't warn you.
{
    if (DetraceZipperExtension.debug) window.dump("skip starts with: "+block+"\n");

    var nextChar = false;
    while( !( nextChar = /\S/.exec(block)) ) // skip whitespace across multiple lines
    {
        block = stripComments(iter.next());
        if (block === false) return;
        if (DetraceZipperExtension.debug) window.dump("consider nextChar block:"+block+"\n");
    }

    if (DetraceZipperExtension.debug) window.dump("skip considers nextChar:"+nextChar[0]+"\n");

    if (nextChar[0] == '{') // then brace match on first non-white
    {
        var braces = 1;
        if ( (nextChar.index + 1) == block.length)
            block = "";
        else
            block = block.substr(nextChar.index + 1);

        if (DetraceZipperExtension.debug) window.dump("found brace braces="+braces+" block:"+block+"\n");

        while( braces )
        {
            var brace = false;
            while ( ! ( brace = reBrace.exec(block) ) )
            {
                block = stripComments(iter.next());
                if (block === false) return;
                if (DetraceZipperExtension.debug) window.dump("consider braces="+braces+" block:"+block+"\n");
            }
            if (brace[0] == '{')
                braces++;
            if (brace[0] == '}')
                braces--;
        }
        // block contains }, drop it
    }
    else // look for lines that end in ;
    {
        var semi = false;
        while( ! ( semi = /;\s*$/.exec(block) ) )
        {
            block = stripComments(iter.next());
            if (block === false) return;
            if (DetraceZipperExtension.debug) window.dump("consider semi block:"+block+"\n");
        }
        // block contains ;, drop it.
    }
}

} catch (exc) {
    window.dump("\ndetrace script FAILS: "+exc+"\n");
}