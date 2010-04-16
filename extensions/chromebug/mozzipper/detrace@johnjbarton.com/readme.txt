<html><body><pre>
/* See license.txt for terms of usage */
/* John J. Barton johnjbarton@johnjbarton.com March 2007 */

Extension for Zipper extension to Mccoy that removes FBTrace statements from Firebug

INSTALL:
Requires the Zipper extension to Mccoy
Copy the files in the directory "detrace" to mccoy/extensions/detrace@johnjbarton.com

USAGE:
If the extension is installed it will de-trace files sent to be zipped.

LIMITATIONS:
The detrace looks for "if(FBTrace." and tries to remove the statements.
Its not a parser and it can only handle simple cases

</pre></body></html>