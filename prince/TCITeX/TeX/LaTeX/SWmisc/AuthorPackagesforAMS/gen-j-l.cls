%%% ====================================================================
%%% @LaTeX-file{
%%%    filename  = "gen-j-l.cls",
%%%    version   = "1.2b",
%%%    date      = "1996/10/25",
%%%    time      = "15:36:41 EDT",
%%%    checksum  = "34978 36 137 1328",
%%%    author    = "Michael Downes",
%%%    address   = "American Mathematical Society, PO Box 6248,
%%%                       Providence, RI 02940, USA",
%%%    email     = "tech-support@ams.org (Internet)",
%%%    supported = "yes",
%%%    abstract  = "This is a LaTeX documentclass for a typical journal
%%%                 of the American Mathematical Society.",
%%%    docstring = "The checksum field above contains a CRC-16 checksum
%%%                 as the first value, followed by the equivalent of
%%%                 the standard UNIX wc (word count) utility output
%%%                 of lines, words, and characters.  This is produced
%%%                 by Robert Solovay's checksum utility.",
%%% }
%%% ====================================================================

\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{gen-j-l}
              [1996/10/25 v1.2b GEN-J Author Class]

\DeclareOption*{\PassOptionsToClass{\CurrentOption}{amsart}}
\ProcessOptions

\LoadClass{amsart}[1996/10/24]

\def\publname{Unspecified Journal}

\def\ISSN{????-????}

\endinput
