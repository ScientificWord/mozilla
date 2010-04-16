<html><body><pre>
/* See license.txt for terms of usage */
/* John J. Barton johnjbarton@johnjbarton.com March 2007 */

Extension for Mccoy to sign update.rdf files and add updateHash to them using only the command line.

INSTALL:
Copy the files in the directory "signOnTheLine" to mccoy/extensions/signOnTheLine@johnjbarton.com

USAGE:
1) Use mccoy.exe to create a key, remember the name you use.
2) Don't set a password (or unset it using the GUI)
3) To sign a file-url:
mccoy.exe -sign <file-url> -key <name> 
4) To sign a file-url and add the updateHash for xpi-file to that file-url:
mccoy.exe -sign <file-url> -key name -addOnFileName <xpi-file>

LIMITATIONS:
The updateHash for xpi-file is added to any part of the update.rdf that has an updateLink RDF thingy.  
So this will only work out for you if you have only one xpi-file in the update.rdf

</pre></body></html>