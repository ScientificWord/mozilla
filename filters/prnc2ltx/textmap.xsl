<?xml version="1.0"?>
<xsl:stylesheet 
      xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
      xmlns:exsl="http://exslt.org/common"
      xmlns:mml="http://www.w3.org/1998/Math/MathML"
      version="1.1">


<!-- The following template can only be called to script chars
  that go into LaTeX TEXT. The output is scripted to an xsl:variable
  from which $$ is removed to produce final output. 
  
  WARNING: To process a string of length n, this algorithm stacks n
  recursive calls and memory use is Order n^2.

-->

  <xsl:template name="do-chars-in-TEXT">
    <xsl:param name="unicode-cdata"/>
      <xsl:variable name="first-char" select="substring($unicode-cdata,1,1)"/>

    <xsl:choose>
      <xsl:when test="$first-char = ' '">
        <xsl:text xml:space="preserve">&#x20;</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char = '&#x0A;'
      or              $first-char = '&#x0D;'">
        <xsl:text xml:space="preserve">&#x20;</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char = '!'
                   or $first-char = '&#x22;'
                   or $first-char = '('
                   or $first-char = ')'
                   or $first-char = '*'
                   or $first-char = '+'
                   or $first-char = ','
                   or $first-char = '-'
                   or $first-char = '.'
                   or $first-char = '/'
                   or $first-char = '0'
                   or $first-char = '1'
                   or $first-char = '2'
                   or $first-char = '3'
                   or $first-char = '4'
                   or $first-char = '5'
                   or $first-char = '6'
                   or $first-char = '7'
                   or $first-char = '8'
                   or $first-char = '9'
                   or $first-char = ':'
                   or $first-char = ';'
                   or $first-char = '&lt;'
                   or $first-char = '='
                   or $first-char = '&gt;'
                   or $first-char = '?'
                   or $first-char = '@'">
        <xsl:value-of select="$first-char"/>
      </xsl:when>

      <xsl:when test="$first-char = 'A'
                   or $first-char = 'B'
                   or $first-char = 'C'
                   or $first-char = 'D'
                   or $first-char = 'E'
                   or $first-char = 'F'
                   or $first-char = 'G'
                   or $first-char = 'H'
                   or $first-char = 'I'
                   or $first-char = 'J'
                   or $first-char = 'K'
                   or $first-char = 'L'
                   or $first-char = 'M'
                   or $first-char = 'N'
                   or $first-char = 'O'
                   or $first-char = 'P'
                   or $first-char = 'Q'
                   or $first-char = 'R'
                   or $first-char = 'S'
                   or $first-char = 'T'
                   or $first-char = 'U'
                   or $first-char = 'V'
                   or $first-char = 'W'
                   or $first-char = 'X'
                   or $first-char = 'Y'
                   or $first-char = 'Z'
                   or $first-char = '['
                   or $first-char = ']'
                   or $first-char = '`'">
        <xsl:value-of select="$first-char"/>
      </xsl:when>

      <xsl:when test="$first-char = 'a'
                   or $first-char = 'b'
                   or $first-char = 'c'
                   or $first-char = 'd'
                   or $first-char = 'e'
                   or $first-char = 'f'
                   or $first-char = 'g'
                   or $first-char = 'h'
                   or $first-char = 'i'
                   or $first-char = 'j'
                   or $first-char = 'k'
                   or $first-char = 'l'
                   or $first-char = 'm'
                   or $first-char = 'n'
                   or $first-char = 'o'
                   or $first-char = 'p'
                   or $first-char = 'q'
                   or $first-char = 'r'
                   or $first-char = 's'
                   or $first-char = 't'
                   or $first-char = 'u'
                   or $first-char = 'v'
                   or $first-char = 'w'
                   or $first-char = 'x'
                   or $first-char = 'y'
                   or $first-char = 'z'">
        <xsl:value-of select="$first-char"/>
      </xsl:when>

      <xsl:when test="$first-char='#'">
        <xsl:text>\#</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x24;'">
        <xsl:text>\$</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&amp;'">
        <xsl:text>\&amp;</xsl:text>
      </xsl:when>
      <xsl:when test='$first-char="&#x27;"'>
        <xsl:text>'</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='%'">
        <xsl:text>\%</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char = '\'">
        <xsl:text>\TEXTsymbol{\backslash}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='^'">
        <xsl:text>\symbol{94}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='_'">
        <xsl:text>\_</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char = '{'">
        <xsl:text>\{</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char = '|'">
        <xsl:text>\TEXTsymbol{\vert}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char = '}'">
        <xsl:text>\}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char = '~'">
        <xsl:text>\symbol{126}</xsl:text>
      </xsl:when>

<!-- xsl:when test="starts-with($unicode-cdata,'&#xe2;')" -->

<!-- Latin 1 -->

      <xsl:when test="$first-char='&#xa0;'">
        <xsl:text xml:space="preserve">~</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xa1;'">
        <xsl:text xml:space="preserve">\textexclamdown </xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xa2;'">
        <xsl:if test="$output-mode='SW-LaTeX'">
          <xsl:text xml:space="preserve">\cents </xsl:text>
        </xsl:if>
        <xsl:if test="$output-mode='Portable-LaTeX'">
          <xsl:text>\mbox{\rm\rlap/c}</xsl:text>
        </xsl:if>
      </xsl:when>
      <xsl:when test="$first-char='&#xa3;'">
        <xsl:text xml:space="preserve">\pounds </xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xa4;'">
        <xsl:text xml:space="preserve">\textcurrency </xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xa5;'">
        <xsl:text xml:space="preserve">\yen </xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xa6;'">
        <xsl:text xml:space="preserve">$\vert$</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xa7;'">
        <xsl:text xml:space="preserve">\S </xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xa8;'">
        <xsl:if test="$output-mode='SW-LaTeX'">
          <xsl:text>\U{a8}</xsl:text>
        </xsl:if>
        <xsl:if test="$output-mode='Portable-LaTeX'">
          <xsl:text>\"{}</xsl:text>
        </xsl:if>
      </xsl:when>
      <xsl:when test="$first-char='&#xa9;'">
        <xsl:text xml:space="preserve">\copyright </xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xaa;'">
        <xsl:if test="$output-mode='SW-LaTeX'">
          <xsl:text>\U{aa}</xsl:text>
        </xsl:if>
        <xsl:if test="$output-mode='Portable-LaTeX'">
          <xsl:text>${{}^a}$</xsl:text>
        </xsl:if>
      </xsl:when>
      <xsl:when test="$first-char='&#xab;'">
        <xsl:text xml:space="preserve">\guillemotleft </xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xac;'">
        <xsl:text>$\lnot$</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xad;'">
        <xsl:text xml:space="preserve">-</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xae;'">
        <xsl:text xml:space="preserve">\textregistered </xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xaf;'">
        <xsl:if test="$output-mode='SW-LaTeX'">
          <xsl:text>\U{af}</xsl:text>
        </xsl:if>
        <xsl:if test="$output-mode='Portable-LaTeX'">
          <xsl:text>\={}</xsl:text>
        </xsl:if>
      </xsl:when>



      <xsl:when test="$first-char='&#xb0;'">
        <xsl:if test="$output-mode='SW-LaTeX'">
          <xsl:text>\U{b0}</xsl:text>
        </xsl:if>
        <xsl:if test="$output-mode='Portable-LaTeX'">
          <xsl:text>${{}^\circ}$</xsl:text>
        </xsl:if>
      </xsl:when>

      <xsl:when test="$first-char='&#xb1;'">
        <xsl:text>$\pm$</xsl:text>
      </xsl:when>

      <xsl:when test="$first-char='&#xb2;'">
        <xsl:if test="$output-mode='SW-LaTeX'">
          <xsl:text>\U{b2}</xsl:text>
        </xsl:if>
        <xsl:if test="$output-mode='Portable-LaTeX'">
          <xsl:text>${{}^2}$</xsl:text>
        </xsl:if>
      </xsl:when>
      <xsl:when test="$first-char='&#xb3;'">
        <xsl:if test="$output-mode='SW-LaTeX'">
          <xsl:text>\U{b3}</xsl:text>
        </xsl:if>
        <xsl:if test="$output-mode='Portable-LaTeX'">
          <xsl:text>${{}^3}$</xsl:text>
        </xsl:if>
      </xsl:when>
     <xsl:when test="$first-char='&#xb4;'">
    	<xsl:if test="$output-mode='SW-LaTeX'">
          <xsl:text>\U{b4}</xsl:text>
        </xsl:if>
        <xsl:if test="$output-mode='Portable-LaTeX'">
          <xsl:text>\'{}</xsl:text>
        </xsl:if>
      </xsl:when>
      <xsl:when test="$first-char='&#xb5;'">
        <xsl:if test="$output-mode='SW-LaTeX'">
          <xsl:text>\U{b5}</xsl:text>
        </xsl:if>
        <xsl:if test="$output-mode='Portable-LaTeX'">
          <xsl:text>$\mu$</xsl:text>
        </xsl:if>
      </xsl:when>
      <xsl:when test="$first-char='&#xb6;'">
        <xsl:text xml:space="preserve">\P </xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xb7;'">
        <xsl:text xml:space="preserve">\textperiodcentered </xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xb8;'">
        <xsl:if test="$output-mode='SW-LaTeX'">
          <xsl:text>\U{b8}</xsl:text>
        </xsl:if>
        <xsl:if test="$output-mode='Portable-LaTeX'">
          <xsl:text>\c{}</xsl:text>
        </xsl:if>
      </xsl:when>
      <xsl:when test="$first-char='&#xb9;'">
        <xsl:if test="$output-mode='SW-LaTeX'">
          <xsl:text>\U{b9}</xsl:text>
        </xsl:if>
        <xsl:if test="$output-mode='Portable-LaTeX'">
          <xsl:text>${{}^1}$</xsl:text>
        </xsl:if>
      </xsl:when>
      <xsl:when test="$first-char='&#xba;'">
        <xsl:if test="$output-mode='SW-LaTeX'">
          <xsl:text>\U{ba}</xsl:text>
        </xsl:if>
        <xsl:if test="$output-mode='Portable-LaTeX'">
          <xsl:text>${{}^o}$</xsl:text>
        </xsl:if>
      </xsl:when>
      <xsl:when test="$first-char='&#xbb;'">
        <xsl:text xml:space="preserve">\guillemotright </xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xbc;'">
        <xsl:if test="$output-mode='SW-LaTeX'">
          <xsl:text>\U{bc}</xsl:text>
        </xsl:if>
        <xsl:if test="$output-mode='Portable-LaTeX'">
          <xsl:text>$\frac14$</xsl:text>
        </xsl:if>
      </xsl:when>
      <xsl:when test="$first-char='&#xbd;'">
        <xsl:if test="$output-mode='SW-LaTeX'">
          <xsl:text>\U{bd}</xsl:text>
        </xsl:if>
        <xsl:if test="$output-mode='Portable-LaTeX'">
          <xsl:text>$\frac12$</xsl:text>
        </xsl:if>
      </xsl:when>
      <xsl:when test="$first-char='&#xbe;'">
        <xsl:if test="$output-mode='SW-LaTeX'">
          <xsl:text>\U{be}</xsl:text>
        </xsl:if>
        <xsl:if test="$output-mode='Portable-LaTeX'">
          <xsl:text>$\frac34$</xsl:text>
        </xsl:if>
      </xsl:when>
      <xsl:when test="$first-char='&#xbf;'">
        <xsl:text xml:space="preserve">\textquestiondown </xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xc0;'">
        <xsl:text>\`{A}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xc1;'">
        <xsl:text>\'{A}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xc2;'">
        <xsl:text>\^{A}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xc3;'">
        <xsl:text>\~{A}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xc4;'">
        <xsl:text>\"{A}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xc5;'">
        <xsl:text xml:space="preserve">\AA </xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xc6;'">
        <xsl:text xml:space="preserve">\AE </xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xc7;'">
        <xsl:text>\c{C}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xc8;'">
        <xsl:text>\`{E}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xc9;'">
        <xsl:text>\'{E}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xca;'">
        <xsl:text>\^{E}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xcb;'">
        <xsl:text>\"{E}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xcc;'">
        <xsl:text>\`{I}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xcd;'">
        <xsl:text>\'{I}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xce;'">
        <xsl:text>\^{I}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xcf;'">
        <xsl:text>\"{I}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xd0;'">
        <xsl:text xml:space="preserve">\DH </xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xd1;'">
        <xsl:text>\~{N}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xd2;'">
        <xsl:text xml:space="preserve">\`{O}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xd3;'">
        <xsl:text>\'{O}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xd4;'">
        <xsl:text>\^{O}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xd5;'">
        <xsl:text>\~{O}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xd6;'">
        <xsl:text>\"{O}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xd7;'">
        <xsl:text>$\times$</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xd8;'">
        <xsl:text xml:space="preserve">\O </xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xd9;'">
        <xsl:text>\`{U}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xda;'">
        <xsl:text>\'{U}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xdb;'">
        <xsl:text>\^{U}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xdc;'">
        <xsl:text>\"{U}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xdd;'">
        <xsl:text>\'{Y}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xde;'">
        <xsl:text xml:space="preserve">\TH </xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xdf;'">
        <xsl:text xml:space="preserve">\ss </xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xe0;'">
        <xsl:text>\`{a}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xe1;'">
        <xsl:text>\'{a}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xe2;'">
        <xsl:text>\^{a}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xe3;'">
        <xsl:text>\~{a}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xe4;'">
        <xsl:text>\"{a}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xe5;'">
        <xsl:text xml:space="preserve">\aa </xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xe6;'">
        <xsl:text xml:space="preserve">\ae </xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xe7;'">
        <xsl:text>\c{c}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xe8;'">
        <xsl:text>\`{e}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xe9;'">
        <xsl:text>\'{e}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xea;'">
        <xsl:text>\^{e}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xeb;'">
        <xsl:text>\"{e}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xec;'">
        <xsl:text>\`{\i}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xed;'">
        <xsl:text>\'{\i}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xee;'">
        <xsl:text>\^{\i}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xef;'">
        <xsl:text>\"{\i}</xsl:text>
      </xsl:when>

      <xsl:when test="$first-char='&#xf0;'">
        <xsl:text xml:space="preserve">\dh </xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xf1;'">
        <xsl:text>\~{n}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xf2;'">
        <xsl:text>\`{o}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xf3;'">
        <xsl:text>\'{o}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xf4;'">
        <xsl:text>\^{o}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xf5;'">
        <xsl:text>\~{o}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xf6;'">
        <xsl:text>\"{o}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xf7;'">
        <xsl:text>$\div$</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xf8;'">
        <xsl:text xml:space="preserve">\o </xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xf9;'">
        <xsl:text>\`{u}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xfa;'">
        <xsl:text>\'{u}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xfb;'">
        <xsl:text>\^{u}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xfc;'">
        <xsl:text>\"{u}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xfd;'">
        <xsl:text>\'{y}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xfe;'">
        <xsl:text xml:space="preserve">\th  </xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#xff;'">
        <xsl:text>\"{y}</xsl:text>
      </xsl:when>

<!-- Latin Extended A -->

      <xsl:when test="$first-char='&#x100;'">
        <xsl:text>\={A}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x101;'">
        <xsl:text>\={a}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x102;'">
        <xsl:text>\u{A}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x103;'">
        <xsl:text>\u{a}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x104;'">
        <xsl:text>\k{A}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x105;'">
        <xsl:text>\k{a}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x106;'">
        <xsl:text>\'{C}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x107;'">
        <xsl:text>\'{c}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x108;'">
        <xsl:text>\^{C}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x109;'">
        <xsl:text>\^{c}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x10a;'">
        <xsl:text>\.{C}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x10b;'">
        <xsl:text>\.{c}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x10c;'">
        <xsl:text>\v{C}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x10d;'">
        <xsl:text>\v{c}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x10e;'">
        <xsl:text>\v{D}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x10f;'">
        <xsl:text>\v{d}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x110;'">
        <xsl:text xml:space="preserve">\DJ </xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x111;'">
        <xsl:text xml:space="preserve">\dj </xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x112;'">
        <xsl:text>\={E}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x113;'">
        <xsl:text>\={e}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x114;'">
        <xsl:text>\u{E}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x115;'">
        <xsl:text>\u{e}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x116;'">
        <xsl:text>\.{E}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x117;'">
        <xsl:text>\.{e}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x118;'">
        <xsl:text>\k{E}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x119;'">
        <xsl:text>\k{e}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x11a;'">
        <xsl:text>\v{E}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x11b;'">
        <xsl:text>\v{e}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x11c;'">
        <xsl:text>\^{G}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x11d;'">
        <xsl:text>\^{g}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x11e;'">
        <xsl:text>\u{G}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x11f;'">
        <xsl:text>\u{g}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x120;'">
        <xsl:text>\.{G}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x121;'">
        <xsl:text>\.{g}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x122;'">
        <xsl:text>\c{G}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x123;'">
        <xsl:text>\c{g}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x124;'">
        <xsl:text>\^{H}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x125;'">
        <xsl:text>\^{h}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x126;'">
        <xsl:if test="$output-mode='SW-LaTeX'">
          <xsl:text>\U{126}</xsl:text>
        </xsl:if>
        <xsl:if test="$output-mode='Portable-LaTeX'">
          <xsl:text>H\llap{\protect\rule[1.1ex]{.735em}{.1ex}}</xsl:text>
        </xsl:if>
      </xsl:when>
      <xsl:when test="$first-char='&#x127;'">
        <xsl:if test="$output-mode='SW-LaTeX'">
          <xsl:text>\U{127}</xsl:text>
        </xsl:if>
        <xsl:if test="$output-mode='Portable-LaTeX'">
          <xsl:text>h{\hskip-.2em}\llap{\protect\rule[1.1ex]{.325em}{.1ex}}{\hskip.2em}</xsl:text>
        </xsl:if>
      </xsl:when>
      <xsl:when test="$first-char='&#x128;'">
        <xsl:text>\~{I}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x129;'">
        <xsl:text>\~{\i}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x12a;'">
        <xsl:text>\={I}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x12b;'">
        <xsl:text>\={\i}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x12c;'">
        <xsl:text>\u{I}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x12d;'">
        <xsl:text>\u{\i}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x12e;'">
        <xsl:text>\k{I}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x12f;'">
        <xsl:text>\k{i}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x130;'">
        <xsl:text>\.{I}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x131;'">
        <xsl:text xml:space="preserve">\i </xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x132;'">
        <xsl:if test="$output-mode='SW-LaTeX'">
          <xsl:text>\U{132}</xsl:text>
        </xsl:if>
        <xsl:if test="$output-mode='Portable-LaTeX'">
          <xsl:text>\symbol{156}</xsl:text>
        </xsl:if>
      </xsl:when>
      <xsl:when test="$first-char='&#x133;'">
        <xsl:if test="$output-mode='SW-LaTeX'">
          <xsl:text>\U{133}</xsl:text>
        </xsl:if>
        <xsl:if test="$output-mode='Portable-LaTeX'">
          <xsl:text>\symbol{188}</xsl:text>
        </xsl:if>
      </xsl:when>
      <xsl:when test="$first-char='&#x134;'">
        <xsl:text>\^{J}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x135;'">
        <xsl:text>\^{\j}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x136;'">
        <xsl:text>\c{K}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x137;'">
        <xsl:text>\c{k}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x138;'">
        <xsl:text>\U{138}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x139;'">
        <xsl:text>\'{L}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x13a;'">
        <xsl:text>\'{l}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x13b;'">
        <xsl:text>\c{L}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x13c;'">
        <xsl:text>\c{l}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x13d;'">
        <xsl:text>\v{L}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x13e;'">
        <xsl:text>\v{l}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x13f;'">
        <xsl:if test="$output-mode='SW-LaTeX'">
          <xsl:text>\U{13f}</xsl:text>
        </xsl:if>
        <xsl:if test="$output-mode='Portable-LaTeX'">
          <xsl:text>L{\hskip-.05em}\llap{\raise.5ex\hbox{$\cdot$}}{\hskip.05em}</xsl:text>
        </xsl:if>
      </xsl:when>
      <xsl:when test="$first-char='&#x140;'">
        <xsl:if test="$output-mode='SW-LaTeX'">
          <xsl:text>\U{140}</xsl:text>
        </xsl:if>
        <xsl:if test="$output-mode='Portable-LaTeX'">
          <xsl:text>l{\hskip.15em}\llap{\raise.25ex\hbox{$\cdot$}}</xsl:text>
        </xsl:if>
      </xsl:when>
      <xsl:when test="$first-char='&#x141;'">
        <xsl:text xml:space="preserve">\L </xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x142;'">
        <xsl:text xml:space="preserve">\l </xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x143;'">
        <xsl:text>\'{N}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x144;'">
        <xsl:text>\'{n}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x145;'">
        <xsl:text>\c{N}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x146;'">
        <xsl:text>\c{n}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x147;'">
        <xsl:text>\v{N}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x148;'">
        <xsl:text>\v{n}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x149;'">
        <xsl:if test="$output-mode='SW-LaTeX'">
          <xsl:text>\U{149}</xsl:text>
        </xsl:if>
        <xsl:if test="$output-mode='Portable-LaTeX'">
          <xsl:text>n{\hskip-.3em}\llap{\raise1.5ex\hbox{,}}{\hskip.3em}</xsl:text>
        </xsl:if>
      </xsl:when>
      <xsl:when test="$first-char='&#x14a;'">
        <xsl:text xml:space="preserve">\NG </xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x14b;'">
        <xsl:text xml:space="preserve">\ng </xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x14c;'">
        <xsl:text>\={O}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x14d;'">
        <xsl:text>\={o}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x14e;'">
        <xsl:text>\u{O}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x14f;'">
        <xsl:text>\u{o}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x150;'">
        <xsl:text>\H{O}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x151;'">
        <xsl:text>\H{o}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x152;'">
        <xsl:text xml:space="preserve">\OE </xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x153;'">
        <xsl:text xml:space="preserve">\oe </xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x154;'">
        <xsl:text>\'{R}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x155;'">
        <xsl:text>\'{r}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x156;'">
        <xsl:text>\c{R}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x157;'">
        <xsl:text>\c{r}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x158;'">
        <xsl:text>\v{R}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x159;'">
        <xsl:text>\v{r}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x15a;'">
        <xsl:text>\'{S}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x15b;'">
        <xsl:text>\'{s}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x15c;'">
        <xsl:text>\^{S}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x15d;'">
        <xsl:text>\^{s}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x15e;'">
        <xsl:text>\c{S}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x15f;'">
        <xsl:text>\c{s}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x160;'">
        <xsl:text>\v{S}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x161;'">
        <xsl:text>\v{s}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x162;'">
        <xsl:text>\c{T}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x163;'">
        <xsl:text>\c{t}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x164;'">
        <xsl:text>\v{T}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x165;'">
        <xsl:text>\v{t}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x166;'">
        <xsl:if test="$output-mode='SW-LaTeX'">
          <xsl:text>\U{166}</xsl:text>
        </xsl:if>
        <xsl:if test="$output-mode='Portable-LaTeX'">
          <xsl:text>T{\hskip-.05em}\llap{\protect\rule[.7ex]{.6em}{.1ex}}{\hskip.05em}</xsl:text>
        </xsl:if>
      </xsl:when>
      <xsl:when test="$first-char='&#x167;'">
        <xsl:if test="$output-mode='SW-LaTeX'">
          <xsl:text>\U{167}</xsl:text>
        </xsl:if>
        <xsl:if test="$output-mode='Portable-LaTeX'">
          <xsl:text>t{\hskip-.05em}\llap{\protect\rule[.7ex]{.36em}{.1ex}}{\hskip.05em}</xsl:text>
        </xsl:if>
      </xsl:when>
      <xsl:when test="$first-char='&#x168;'">
        <xsl:text>\~{U}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x169;'">
        <xsl:text>\~{u}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x16a;'">
        <xsl:text>\={U}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x16b;'">
        <xsl:text>\={u}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x16c;'">
        <xsl:text>\u{U}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x16d;'">
        <xsl:text>\u{u}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x16e;'">
        <xsl:text>\r{U}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x16f;'">
        <xsl:text>\r{u}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x170;'">
        <xsl:text>\H{U}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x171;'">
        <xsl:text>\H{u}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x172;'">
        <xsl:text>\k{U}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x173;'">
        <xsl:text>\k{u}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x174;'">
        <xsl:text>\^{W}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x175;'">
        <xsl:text>\^{w}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x176;'">
        <xsl:text>\^{Y}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x177;'">
        <xsl:text>\^{y}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x178;'">
        <xsl:text>\"{Y}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x179;'">
        <xsl:text>\'{Z}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x17a;'">
        <xsl:text>\'{z}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x17b;'">
        <xsl:text>\.{Z}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x17c;'">
        <xsl:text>\.{z}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x17d;'">
        <xsl:text>\v{Z}</xsl:text>
      </xsl:when>
      <xsl:when test="$first-char='&#x17e;'">
        <xsl:text>\v{z}</xsl:text>
      </xsl:when>


<!-- !ENTITY mgr "&#x03BC;"> <!ENTITY mu "&#x03BC;" -->
      <xsl:when test="$first-char='&#x03BC;'">
        <xsl:text xml:space="preserve">$\mu$</xsl:text>
      </xsl:when>
<!-- !ENTITY OHgr "&#x03A9;"> <!ENTITY Omega "&#x03A9;" -->
      <xsl:when test="$first-char='&#x03A9;'">
        <xsl:text xml:space="preserve">$\Omega$</xsl:text>
      </xsl:when>


<!-- !ENTITY ensp "&#x2002;" -->
      <xsl:when test="$first-char='&#x2002;'">
        <xsl:text xml:space="preserve">\ </xsl:text>
      </xsl:when>
<!-- !ENTITY emsp "&#x2003;" -->
      <xsl:when test="$first-char='&#x2003;'">
        <xsl:text xml:space="preserve">\quad </xsl:text>
      </xsl:when>
<!-- !ENTITY emsp13 "&#x2004;" -->
      <xsl:when test="$first-char='&#x2004;'">
        <xsl:text>\ </xsl:text>
      </xsl:when>

<!-- !ENTITY emsp14 "&#x2005;" -->
<!-- !ENTITY numsp "&#x2007;" -->
<!-- !ENTITY puncsp "&#x2008;" -->
<!-- !ENTITY thinsp "&#x2009;"> <!ENTITY ThinSpace "&#x2009;" -->
      <xsl:when test="$first-char='&#x2009;'">
        <xsl:text xml:space="preserve">\thinspace </xsl:text>
      </xsl:when>
<!-- !ENTITY hairsp "&#x200A;"> <!ENTITY VeryThinSpace "&#x200A;" -->
<!-- !ENTITY ZeroWidthSpace "&#x200B;" -->
      <xsl:when test="$first-char='&#x200B;'">
        <xsl:text>{}</xsl:text>
      </xsl:when>



<!-- ENTITY dagger "&#x2020;" -->
      <xsl:when test="$first-char='&#x2020;'">
        <xsl:text xml:space="preserve">\dag </xsl:text>
      </xsl:when>
<!-- ENTITY Dagger ddagger "&#x2021;" -->
      <xsl:when test="$first-char='&#x2021;'">
        <xsl:text xml:space="preserve">\ddag </xsl:text>
      </xsl:when>




<!-- !ENTITY prime "&#x2032;" -->
      <xsl:when test="$first-char='&#x2032;'">
        <xsl:text>${}^{\prime}$</xsl:text>
      </xsl:when>
<!-- !ENTITY Prime "&#x2033;" -->
      <xsl:when test="$first-char='&#x2033;'">
        <xsl:text>${}^{\prime\prime}$</xsl:text>
      </xsl:when>

<!-- !ENTITY euro "&#x20A0;" -->
      <xsl:when test="$first-char='&#x20A0;'">
        <xsl:if test="$output-mode='SW-LaTeX'">
          <xsl:text xml:space="preserve">\texteuro </xsl:text>
        </xsl:if>
        <xsl:if test="$output-mode='Portable-LaTeX'">
          <xsl:text>?</xsl:text>
        </xsl:if>
      </xsl:when>

      <xsl:when test="$first-char='&#x20A3;'">
        <xsl:if test="$output-mode='SW-LaTeX'">
          <xsl:text xml:space="preserve">\textfranc </xsl:text>
        </xsl:if>
        <xsl:if test="$output-mode='Portable-LaTeX'">
          <xsl:text>?</xsl:text>
        </xsl:if>
      </xsl:when>

      <xsl:when test="$first-char='&#x20A4;'">
        <xsl:if test="$output-mode='SW-LaTeX'">
          <xsl:text xml:space="preserve">\textlira </xsl:text>
        </xsl:if>
        <xsl:if test="$output-mode='Portable-LaTeX'">
          <xsl:text>?</xsl:text>
        </xsl:if>
      </xsl:when>

      <xsl:when test="$first-char='&#x20A7;'">
        <xsl:if test="$output-mode='SW-LaTeX'">
          <xsl:text>\textpeseta </xsl:text>
        </xsl:if>
        <xsl:if test="$output-mode='Portable-LaTeX'">
          <xsl:text>?</xsl:text>
        </xsl:if>
      </xsl:when>


      <xsl:when test="$first-char='&#x20AC;'">
        <xsl:if test="$output-mode='SW-LaTeX'">
          <xsl:text>\U{20ac}</xsl:text>
        </xsl:if>
        <xsl:if test="$output-mode='Portable-LaTeX'">
          <xsl:text>?</xsl:text>
        </xsl:if>
      </xsl:when>


      <xsl:when test="$first-char='&#x2103;'">
        <xsl:if test="$output-mode='SW-LaTeX'">
          <xsl:text>\U{b0}C</xsl:text>
        </xsl:if>
        <xsl:if test="$output-mode='Portable-LaTeX'">
          <xsl:text>${{}^\circ}$C</xsl:text>
        </xsl:if>
      </xsl:when>

      <xsl:when test="$first-char='&#x2109;'">
        <xsl:if test="$output-mode='SW-LaTeX'">
          <xsl:text>\U{b0}F</xsl:text>
        </xsl:if>
        <xsl:if test="$output-mode='Portable-LaTeX'">
          <xsl:text>${{}^\circ}$F</xsl:text>
        </xsl:if>
      </xsl:when>




<!-- !ENTITY texttrademark "&#x2122;" -->
      <xsl:when test="$first-char='&#x2122;'">
        <xsl:text xml:space="preserve">\texttrademark </xsl:text>
      </xsl:when>


<!-- !ENTITY angst "&#x212B;" -->
      <xsl:when test="$first-char='&#x212B;'">
        <xsl:text xml:space="preserve">\AA </xsl:text>
      </xsl:when>


<!-- !ENTITY vellip "&#x22EE;" -->
      <xsl:when test="$first-char='&#x22EE;'">
        <xsl:text>$\vdots$</xsl:text>
      </xsl:when>

<!-- !ENTITY dtdot "&#x22F1;" -->
      <xsl:when test="$first-char='&#x22F1;'">
        <xsl:text>$\ddots$</xsl:text>
      </xsl:when>


<!-- ENTITY jmath "&#xE2D4;" -->
      <xsl:when test="$first-char='&#xE2D4;'">
        <xsl:text xml:space="preserve">\j </xsl:text>
      </xsl:when>

<!-- ENTITY ThickSpace "&#xE897;" -->
      <xsl:when test="$first-char='&#xE897;'">
        <xsl:text xml:space="preserve">\ </xsl:text>
      </xsl:when>



      <xsl:otherwise>
        <xsl:choose>
          <xsl:when test="$script-unicodes='true'">
            <xsl:text>\U{</xsl:text>
            <xsl:value-of select="$first-char"/>
            <xsl:text>}</xsl:text>
          </xsl:when>
          <xsl:otherwise>
            <xsl:text>\protect\rule{0.1in}{0.1in}</xsl:text>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:otherwise>
    </xsl:choose>

    <xsl:if test="string-length($unicode-cdata)&gt;1">
      <xsl:call-template name="do-chars-in-TEXT">
        <xsl:with-param name="unicode-cdata" select="substring($unicode-cdata,2)"/>
      </xsl:call-template>
    </xsl:if>
  </xsl:template>

</xsl:stylesheet>
