<?xml version="1.0"?>
<xsl:stylesheet
      xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
      xmlns:exsl="http://exslt.org/common"
      xmlns:mml="http://www.w3.org/1998/Math/MathML"
      version="1.1">

<!-- mi, mo, and mn contain unicode character data that we map to LaTeX math.
  A single recursive template, chars-to-LaTeX-Math ( in this file ), contains
  the complete map from unicodes to LaTeX math.

  WARNING: To process a string of length n, this algorithm stacks n
  recursive calls and memory use is Order n^2.

  In some cases, a single unicode maps to several TeX symbols.
  It may be desirable to disambiguate these translations using
  attributes.  A few examples follow:

<!ENTITY subseteq  "&#x2286;"> <!ENTITY subseteqq  "&#x2286;">
<!ENTITY supseteq  "&#x2287;"> <!ENTITY supseteqq  "&#x2287;">
<!ENTITY nsubseteq "&#x2288;"> <!ENTITY nsubseteqq "&#x2288;">
<!ENTITY nsupseteq "&#x2289;"> <!ENTITY nsupseteqq "&#x2289;">
<!ENTITY subsetneq "&#x228A;"> <!ENTITY subsetneqq "&#x228A;">
<!ENTITY supsetneq "&#x228B;"> <!ENTITY supsetneqq "&#x228B;">

-->
  <xsl:template match="textcurrency">
    <xsl:text>\textcurrency </xsl:text>
  </xsl:template>

<!--   <xsl:text xml:space="preserve">\textcurrency </xsl:text>
 -->
  <xsl:template name="chars-to-LaTeX-Math">
      <xsl:param name="unicode-cdata"/>
      <xsl:variable name="first-char"
		    select="substring($unicode-cdata,1,1)"/>


      <xsl:variable name="next-char">
	<xsl:if test="string-length($unicode-cdata)&gt;1">
	  <xsl:value-of select="substring($unicode-cdata,2,1)"/>
	</xsl:if>
      </xsl:variable>
      
      <xsl:variable name="char-info-lookup"
		    select="$char-info/char-table/char[@unicode=$first-char][1]/@latex"/>

      <xsl:choose>
         <xsl:when
	          test="contains('0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ',
		                       $first-char)">
            <xsl:value-of select="$first-char"/>
         </xsl:when>


          <xsl:when test="$char-info-lookup != ''">
           <xsl:call-template name="protect"/>
           <xsl:value-of select="$char-info-lookup"/>
           <xsl:text> </xsl:text>
          </xsl:when>

      <xsl:when test="$first-char = '0'
      or              $first-char = '1'
      or              $first-char = '2'
      or              $first-char = '3'
      or              $first-char = '4'
      or              $first-char = '5'
      or              $first-char = '6'
      or              $first-char = '7'
      or              $first-char = '8'
      or              $first-char = '9'
      or              $first-char = 'A'
      or              $first-char = 'B'
      or              $first-char = 'C'
      or              $first-char = 'D'
      or              $first-char = 'E'
      or              $first-char = 'F'
      or              $first-char = 'G'
      or              $first-char = 'H'
      or              $first-char = 'I'
      or              $first-char = 'J'
      or              $first-char = 'K'
      or              $first-char = 'L'
      or              $first-char = 'M'
      or              $first-char = 'N'
      or              $first-char = 'O'
      or              $first-char = 'P'
      or              $first-char = 'Q'
      or              $first-char = 'R'
      or              $first-char = 'S'
      or              $first-char = 'T'
      or              $first-char = 'U'
      or              $first-char = 'V'
      or              $first-char = 'W'
      or              $first-char = 'X'
      or              $first-char = 'Y'
      or              $first-char = 'Z'
      or              $first-char = 'a'
      or              $first-char = 'b'
      or              $first-char = 'c'
      or              $first-char = 'd'
      or              $first-char = 'e'
      or              $first-char = 'f'
      or              $first-char = 'g'
      or              $first-char = 'h'
      or              $first-char = 'i'
      or              $first-char = 'j'
      or              $first-char = 'k'
      or              $first-char = 'l'
      or              $first-char = 'm'
      or              $first-char = 'n'
      or              $first-char = 'o'
      or              $first-char = 'p'
      or              $first-char = 'q'
      or              $first-char = 'r'
      or              $first-char = 's'
      or              $first-char = 't'
      or              $first-char = 'u'
      or              $first-char = 'v'
      or              $first-char = 'w'
      or              $first-char = 'x'
      or              $first-char = 'y'
      or              $first-char = 'z'
      or              $first-char = '!'
      or              $first-char = '('
      or              $first-char = ')'
      or              $first-char = '+'
      or              $first-char = ','
      or              $first-char = '-'
      or              $first-char = '.'
      or              $first-char = '/'
      or              $first-char = ':'
      or              $first-char = ';'
      or              $first-char = '&#x003C;'
      or              $first-char = '='
      or              $first-char = '&#x003E;'
      or              $first-char = '?'
      or              $first-char = '&#x0040;'
      or              $first-char = '['
      or              $first-char = ']'
      or              $first-char = '|'">
		          <xsl:value-of select="$first-char"/>
      </xsl:when>

      <xsl:when test="$first-char=''">
        <xsl:text></xsl:text>
      </xsl:when>

      <xsl:otherwise>

<!--  Start Basic Latin - chars mapped to something other than themselves
    <xsl:choose>
      <xsl:when test="ms:string-compare($first-char,'&#x80') = -1">
-->

          <xsl:choose>
          <xsl:when test="$first-char='&#x0020;'">
            <xsl:text xml:space="preserve">\ </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x0022;'">
            <xsl:text>""</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x0023;'">
            <xsl:text>\#</xsl:text>
          </xsl:when>
          <xsl:when test="string()='$'">
            <xsl:text>\$</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x0025;'">
            <xsl:text>\%</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x0026;'">
            <xsl:text>\&amp;</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x0026;'">
            <xsl:text xml:space="preserve">\And </xsl:text>
          </xsl:when>
          <xsl:when test='$first-char="&#x0027;"'>
            <xsl:text xml:space="preserve">\prime </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x002A;'">
            <xsl:text xml:space="preserve">\ast </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x005C;'">
            <xsl:text xml:space="preserve">\backslash </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x005E;'">
            <xsl:text xml:space="preserve">\char94 </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x005F;'">
            <xsl:text>\_</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x0060;'">
            <xsl:text>``</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='{'">
            <xsl:text>\{</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='}'">
            <xsl:text xml:space="preserve">\}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='~'">
            <xsl:text xml:space="preserve">\sim  </xsl:text>
          </xsl:when>
<!--  End Basic Latin  -  Start Latin-1
          <xsl:otherwise>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:when test="ms:string-compare($first-char,'&#x100') = -1">
        <xsl:choose>
-->
          <xsl:when test="$first-char='&#x00A0;'">
            <xsl:text>~</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00A1;'">
            <xsl:text>\text{\textexclamdown }</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00A2;'">
            <xsl:text xml:space="preserve">\cents </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00A3;'">
            <xsl:text xml:space="preserve">\pounds </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00A4;'">
            <xsl:text xml:space="preserve">\textcurrency </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00A5;'">
            <xsl:text xml:space="preserve">\yen </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00A6;'">
            <xsl:if test="$output-mode='SW-LaTeX'">
              <xsl:text>\U{a6}</xsl:text>
            </xsl:if>
            <xsl:if test="$output-mode='Portable-LaTeX'">
              <xsl:text>{\vert}</xsl:text>
            </xsl:if>
          </xsl:when>
          <xsl:when test="$first-char='&#x00A7;'">
            <xsl:text xml:space="preserve">\S </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00A8;'">
            <xsl:if test="$output-mode='SW-LaTeX'">
              <xsl:text>\U{a8}</xsl:text>
            </xsl:if>
            <xsl:if test="$output-mode='Portable-LaTeX'">
              <xsl:text>\ddot{}</xsl:text>
            </xsl:if>
          </xsl:when>
          <xsl:when test="$first-char='&#x00A9;'">
            <xsl:text xml:space="preserve">\copyright </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00AA;'">
            <xsl:if test="$output-mode='SW-LaTeX'">
              <xsl:text>\U{aa}</xsl:text>
            </xsl:if>
            <xsl:if test="$output-mode='Portable-LaTeX'">
              <xsl:text>{{}^a}</xsl:text>
            </xsl:if>
          </xsl:when>
          <xsl:when test="$first-char='&#x00AB;'">
            <xsl:text xml:space="preserve">\text{\guillemotleft }</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00AC;'">
            <xsl:text xml:space="preserve">\lnot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00AD;'">
            <xsl:text>-</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00AE;'">
            <xsl:text xml:space="preserve">\registered </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00AF;'">
            <xsl:if test="$output-mode='SW-LaTeX'">
              <xsl:text>\U{af}</xsl:text>
            </xsl:if>
            <xsl:if test="$output-mode='Portable-LaTeX'">
              <xsl:text>\bar{}</xsl:text>
            </xsl:if>
          </xsl:when>
          <xsl:when test="$first-char='&#x00B0;'">
            <xsl:if test="$output-mode='SW-LaTeX'">
              <xsl:text>\U{b0}</xsl:text>
            </xsl:if>
            <xsl:if test="$output-mode='Portable-LaTeX'">
<!--               <xsl:text>{{}^\circ}</xsl:text>  -->
              <xsl:text>\ensuremath{{}^\circ}</xsl:text>
            </xsl:if>
          </xsl:when>
          <xsl:when test="$first-char='&#x00B1;'">
            <xsl:text xml:space="preserve">\pm </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00B2;'">
            <xsl:if test="$output-mode='SW-LaTeX'">
              <xsl:text>\U{b2}</xsl:text>
            </xsl:if>
            <xsl:if test="$output-mode='Portable-LaTeX'">
              <xsl:text>{{}^2}</xsl:text>
            </xsl:if>
          </xsl:when>
          <xsl:when test="$first-char='&#x00B3;'">
            <xsl:if test="$output-mode='SW-LaTeX'">
              <xsl:text>\U{b3}</xsl:text>
            </xsl:if>
            <xsl:if test="$output-mode='Portable-LaTeX'">
              <xsl:text>{{}^3}</xsl:text>
            </xsl:if>
          </xsl:when>
          <xsl:when test="$first-char='&#x00B4;'">
            <xsl:if test="$output-mode='SW-LaTeX'">
              <xsl:text>\U{b4}</xsl:text>
            </xsl:if>
            <xsl:if test="$output-mode='Portable-LaTeX'">
              <xsl:text>{\acute{}}</xsl:text>
            </xsl:if>
          </xsl:when>
          <xsl:when test="$first-char='&#x00B5;'">
            <xsl:if test="$output-mode='SW-LaTeX'">
              <xsl:text>\U{b5}</xsl:text>
            </xsl:if>
            <xsl:if test="$output-mode='Portable-LaTeX'">
              <xsl:text>{\mu}</xsl:text>
            </xsl:if>
          </xsl:when>
          <xsl:when test="$first-char='&#x00B6;'">
            <xsl:text xml:space="preserve">\P </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00B7;'">
            <xsl:text>\centerdot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00B8;'">
            <xsl:if test="$output-mode='SW-LaTeX'">
              <xsl:text>\U{b8}</xsl:text>
            </xsl:if>
            <xsl:if test="$output-mode='Portable-LaTeX'">
            </xsl:if>
          </xsl:when>
          <xsl:when test="$first-char='&#x00B9;'">
            <xsl:if test="$output-mode='SW-LaTeX'">
              <xsl:text>\U{b9}</xsl:text>
            </xsl:if>
            <xsl:if test="$output-mode='Portable-LaTeX'">
              <xsl:text>{{}^1}</xsl:text>
            </xsl:if>
          </xsl:when>
          <xsl:when test="$first-char='&#x00BA;'">
            <xsl:if test="$output-mode='SW-LaTeX'">
              <xsl:text>\U{ba}</xsl:text>
            </xsl:if>
            <xsl:if test="$output-mode='Portable-LaTeX'">
              <xsl:text>{{}^o}</xsl:text>
            </xsl:if>
          </xsl:when>
          <xsl:when test="$first-char='&#x00BB;'">
            <xsl:text>\text{\guillemotright }</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00BC;'">
            <xsl:if test="$output-mode='SW-LaTeX'">
              <xsl:text>\U{bc}</xsl:text>
            </xsl:if>
            <xsl:if test="$output-mode='Portable-LaTeX'">
              <xsl:text>{\frac14}</xsl:text>
            </xsl:if>
          </xsl:when>
          <xsl:when test="$first-char='&#x00BD;'">
            <xsl:if test="$output-mode='SW-LaTeX'">
              <xsl:text>\U{bd}</xsl:text>
            </xsl:if>
            <xsl:if test="$output-mode='Portable-LaTeX'">
              <xsl:text>{\frac12}</xsl:text>
            </xsl:if>
          </xsl:when>
          <xsl:when test="$first-char='&#x00BE;'">
            <xsl:if test="$output-mode='SW-LaTeX'">
              <xsl:text>\U{be}</xsl:text>
            </xsl:if>
            <xsl:if test="$output-mode='Portable-LaTeX'">
              <xsl:text>{\frac34}</xsl:text>
            </xsl:if>
          </xsl:when>
          <xsl:when test="$first-char='&#x00BF;'">
            <xsl:text>\text{\textquestiondown }</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00C0;'">
            <xsl:text>\grave{A}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00C1;'">
            <xsl:text>\acute{A}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00C2;'">
            <xsl:text>\hat{A}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00C3;'">
            <xsl:text>\tilde{A}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00C4;'">
            <xsl:text>\ddot{A}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00C5;'">
            <xsl:text>\mathring{A}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00C6;'">
            <xsl:text>\text{\AE}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00C7;'">
            <xsl:text>\text{\c{C}}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00C8;'">
            <xsl:text>\grave{E}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00C9;'">
            <xsl:text>\acute{E}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00CA;'">
            <xsl:text>\hat{E}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00CB;'">
            <xsl:text>\ddot{E}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00CC;'">
            <xsl:text>\grave{I}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00CD;'">
            <xsl:text>\acute{I}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00CE;'">
            <xsl:text>\hat{I}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00CF;'">
            <xsl:text>\ddot{I}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00D0;'">
            <xsl:text>\text{\DH}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00D1;'">
            <xsl:text>\tilde{N}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00D2;'">
            <xsl:text>\grave{O}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00D3;'">
            <xsl:text>\acute{O}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00D4;'">
            <xsl:text>\hat{O}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00D5;'">
            <xsl:text>\tilde{O}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00D6;'">
            <xsl:text>\ddot{O}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00D7;'">
            <xsl:text xml:space="preserve">\times </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00D8;'">
            <xsl:text>\text{\O }</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00D9;'">
            <xsl:text>\grave{U}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00DA;'">
            <xsl:text>\acute{U}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00DB;'">
            <xsl:text>\hat{U}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00DC;'">
            <xsl:text>\ddot{U}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00DD;'">
            <xsl:text>\acute{Y}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00DE;'">
            <xsl:text>\text{\TH}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00DF;'">
            <xsl:text>\text{\ss }</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00E0;'">
            <xsl:text>\grave{a}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00E1;'">
            <xsl:text>\acute{a}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00E2;'">
            <xsl:text>\hat{a}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00E3;'">
            <xsl:text>\tilde{a}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00E4;'">
            <xsl:text>\ddot{a}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00E5;'">
            <xsl:text>\mathring{a}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00E6;'">
            <xsl:text>\text{\ae }</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00E7;'">
            <xsl:text>\text{\c{c}}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00E8;'">
            <xsl:text>\grave{e}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00E9;'">
            <xsl:text>\acute{e}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00EA;'">
            <xsl:text>\hat{e}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00EB;'">
            <xsl:text>\ddot{e}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00EC;'">
            <xsl:text>\grave{\imath}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00ED;'">
            <xsl:text>\acute{\imath}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00EE;'">
            <xsl:text>\hat{\imath}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00EF;'">
            <xsl:text>\ddot{\imath}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00F0;'">
            <xsl:text xml:space="preserve">\eth </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00F1;'">
            <xsl:text>\tilde{n}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00F2;'">
            <xsl:text>\grave{o}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00F3;'">
            <xsl:text>\acute{o}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00F4;'">
            <xsl:text>\hat{o}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00F5;'">
            <xsl:text>\tilde{o}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00F6;'">
            <xsl:text>\ddot{o}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00F7;'">
            <xsl:text xml:space="preserve">\div </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00F8;'">
            <xsl:text>\text{\o}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00F9;'">
            <xsl:text>\grave{u}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00FA;'">
            <xsl:text>\acute{u}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00FB;'">
            <xsl:text>\hat{u}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00FC;'">
            <xsl:text>\ddot{u}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00FD;'">
            <xsl:text>\acute{y}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00FE;'">
            <xsl:text>\text{\th }</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x00FF;'">
            <xsl:text>\ddot{y}</xsl:text>
          </xsl:when>

<!--  End Latin 1  -  Start Latin Extended-A
          <xsl:otherwise>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:when test="ms:string-compare($first-char,'&#x0180') = -1">
        <xsl:choose>
-->
<!-- JLF - Using mathematical italic small dotless i here, rather than x131 for dotless i -->
          <xsl:when test="$first-char='&#x1D6A4;'">
            <xsl:text xml:space="preserve">\imath </xsl:text>
          </xsl:when>

<!--  End Latin Extended-A  -  Start Latin Extended-B
          <xsl:otherwise>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:when test="ms:string-compare($first-char,'&#x024F') = -1">
        <xsl:choose>
-->

<!--  IPA Extensions
          <xsl:otherwise>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:when test="ms:string-compare($first-char,'&#x02AF') = -1">
        <xsl:choose>
-->

<!--  Spacing Modifiers
          <xsl:otherwise>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:when test="ms:string-compare($first-char,'&#x02FF') = -1">
        <xsl:choose>
-->

<!--  Combining Diacritical Marks
          <xsl:otherwise>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:when test="ms:string-compare($first-char,'&#x036F') = -1">
        <xsl:choose>
-->

<!--  Greek
          <xsl:otherwise>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:when test="ms:string-compare($first-char,'&#x03FF') = -1">
        <xsl:choose>
-->

          <xsl:when test="$first-char='&#x0393;'">
            <xsl:text xml:space="preserve">\Gamma </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x0394;'">
            <xsl:text xml:space="preserve">\Delta </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x0398;'">
            <xsl:text xml:space="preserve">\Theta </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x039B;'">
            <xsl:text xml:space="preserve">\Lambda </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x039E;'">
            <xsl:text xml:space="preserve">\Xi </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x03A0;'">
            <xsl:text xml:space="preserve">\Pi </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x03A3;'">
            <xsl:text xml:space="preserve">\Sigma </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x03A6;'">
            <xsl:text xml:space="preserve">\Phi </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x03A8;'">
            <xsl:text xml:space="preserve">\Psi </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x03A9;'">
            <xsl:text xml:space="preserve">\Omega </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x03B1;'">
            <xsl:text xml:space="preserve">\alpha </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x03B2;'">
            <xsl:text xml:space="preserve">\beta </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x03B3;'">
            <xsl:text xml:space="preserve">\gamma </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x03B4;'">
            <xsl:text xml:space="preserve">\delta </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x03B5;'">
            <xsl:text xml:space="preserve">\varepsilon </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x03B5;'">
            <xsl:text xml:space="preserve">\straightepsilon </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x03B6;'">
            <xsl:text xml:space="preserve">\zeta </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x03B7;'">
            <xsl:text xml:space="preserve">\eta </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x03B8;'">
            <xsl:text xml:space="preserve">\theta </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x03B9;'">
            <xsl:text xml:space="preserve">\iota </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x03BA;'">
            <xsl:text xml:space="preserve">\kappa </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x03BB;'">
            <xsl:text xml:space="preserve">\lambda </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x03BC;'">
            <xsl:text xml:space="preserve">\mu </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x03BD;'">
            <xsl:text xml:space="preserve">\nu </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x03BE;'">
            <xsl:text xml:space="preserve">\xi </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x03BF;'">
            <xsl:text xml:space="preserve">o</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x03C0;'">
            <xsl:text xml:space="preserve">\pi </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x03C1;'">
            <xsl:text xml:space="preserve">\rho </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x03C2;'">
            <xsl:text xml:space="preserve">\varsigma </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x03C3;'">
            <xsl:text xml:space="preserve">\sigma </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x03C4;'">
            <xsl:text xml:space="preserve">\tau </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x03C5;'">
            <xsl:text xml:space="preserve">\upsilon </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x03C6;'">
            <xsl:text xml:space="preserve">\varphi </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x03C7;'">
            <xsl:text xml:space="preserve">\chi </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x03C8;'">
            <xsl:text xml:space="preserve">\psi </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x03C9;'">
            <xsl:text xml:space="preserve">\omega </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x03D1;'">
            <xsl:text xml:space="preserve">\vartheta </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x03D2;'">
            <xsl:text xml:space="preserve">\Upsilon </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x03D5;'">
            <xsl:text xml:space="preserve">\phi </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x03D6;'">
            <xsl:text xml:space="preserve">\varpi </xsl:text>
          </xsl:when>
         <xsl:when test="$first-char='&#x03DB;'">
            <xsl:text xml:space="preserve">\stigma </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x03DC;'">
            <xsl:text xml:space="preserve">\digamma </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x03DC;'">
            <xsl:text xml:space="preserve">\Digamma </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x03F0;'">
            <xsl:text xml:space="preserve">\varkappa </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x03F1;'">
            <xsl:text xml:space="preserve">\varrho </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x03F5;'">
            <xsl:text xml:space="preserve">\epsilon </xsl:text>
          </xsl:when>

<!--  Cyrillic
          <xsl:otherwise>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:when test="ms:string-compare($first-char,'&#x4FA') = -1">
        <xsl:choose>
-->

<!--  Armenian
          <xsl:otherwise>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:when test="ms:string-compare($first-char,'&#x590') = -1">
        <xsl:choose>
-->

<!--  Hebrew
          <xsl:otherwise>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:when test="ms:string-compare($first-char,'&#x5F5') = -1">
        <xsl:choose>
-->

<!--  Arabic
          <xsl:otherwise>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:when test="ms:string-compare($first-char,'&#x6FA') = -1">
        <xsl:choose>
-->

<!--  Greek Extended
          <xsl:otherwise>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:when test="ms:string-compare($first-char,'&#x1FFF') = -1">
        <xsl:choose>
-->

<!--  General Punctuation
          <xsl:otherwise>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:when test="ms:string-compare($first-char,'&#x2070') = -1">
        <xsl:choose>
-->

<!-- ENTITY ensp -->
          <xsl:when test="$first-char='&#x2002;'">
            <xsl:text>\,</xsl:text>
          </xsl:when>
<!-- ENTITY emsp -->
          <xsl:when test="$first-char='&#x2003;'">
            <xsl:text>\,</xsl:text>
          </xsl:when>
<!-- ENTITY emsp13 -->
          <xsl:when test="$first-char='&#x2004;'">
            <xsl:text>\,</xsl:text>
          </xsl:when>
<!-- ENTITY emsp14 -->
          <xsl:when test="$first-char='&#x2005;'">
            <xsl:text>\,</xsl:text>
          </xsl:when>
<!-- ENTITY numsp -->
          <xsl:when test="$first-char='&#x2007;'">
            <xsl:text>\,</xsl:text>
          </xsl:when>
<!-- ENTITY puncsp -->
          <xsl:when test="$first-char='&#x2008;'">
            <xsl:text>\,</xsl:text>
          </xsl:when>
<!-- ENTITY thinsp -->
          <xsl:when test="$first-char='&#x2009;'">
            <xsl:text>\,</xsl:text>
          </xsl:when>
<!-- ENTITY hairsp -->
          <xsl:when test="$first-char='&#x200A;'">
            <xsl:text>\,</xsl:text>
          </xsl:when>
<!-- ENTITY ZeroWidthSpace -->
          <xsl:when test="$first-char='&#x200B;'">
            <xsl:text>{}</xsl:text>
          </xsl:when>
<!-- ENTITY dash -->
          <xsl:when test="$first-char='&#x2010;'">
            <xsl:text>-</xsl:text>
          </xsl:when>
<!-- ENTITY ndash -->
          <xsl:when test="$first-char='&#x2013;'">
            <xsl:text>-</xsl:text>
          </xsl:when>
<!-- ENTITY mdash -->
          <xsl:when test="$first-char='&#x2014;'">
            <xsl:text>--</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2018;'">
            <xsl:text>\lq</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2019;'">
            <xsl:text>\rq</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x201A;'">
            <xsl:text>\quotesinglbase</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x201E;'">
            <xsl:text>\quotedblbase</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2039;'">
            <xsl:text>\guilsinglleft</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x203A;'">
            <xsl:text>\guilsinglright</xsl:text>
          </xsl:when>

          <xsl:when test="$first-char='&#x2016;'">
            <xsl:text xml:space="preserve">\Vert </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2020;'">
            <xsl:text xml:space="preserve">\dag </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2021;'">
            <xsl:text xml:space="preserve">\ddag </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2022;'">    <!-- bullet symbol -->
            <xsl:text xml:space="preserve">\bullet </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2219;'">    <!-- bullet operator -->
            <xsl:text xml:space="preserve">\bullet </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2026;'">
            <xsl:text xml:space="preserve">\ldots </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2032;'">
            <xsl:text xml:space="preserve">\prime </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2033;'">
            <xsl:text xml:space="preserve">\prime\prime </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2034;'">
            <xsl:text xml:space="preserve">\prime\prime\prime </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2035;'">
            <xsl:text xml:space="preserve">\backprime </xsl:text>
          </xsl:when>


          <xsl:when test="$first-char='&#x2061;'">
            <xsl:text></xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2062;'">
            <xsl:text></xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2063;'">
            <xsl:text></xsl:text>
          </xsl:when>

<!--  Superscripts and Subscripts
          <xsl:otherwise>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:when test="ms:string-compare($first-char,'&#x208F') = -1">
        <xsl:choose>
-->

<!--  Currency Symbols
          <xsl:otherwise>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:when test="ms:string-compare($first-char,'&#x20CF') = -1">
        <xsl:choose>
-->
          <xsl:when test="$first-char='&#x20AC;'">
            <xsl:text xml:space="preserve">\texteuro </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x20A3;'">
            <xsl:text xml:space="preserve">\textfranc </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x20A4;'">
            <xsl:text xml:space="preserve">\textlira </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x20A7;'">
            <xsl:text xml:space="preserve">\textpeseta </xsl:text>
          </xsl:when>

<!--  Letterlike Symbols
          <xsl:otherwise>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:when test="ms:string-compare($first-char,'&#x214F') = -1">
        <xsl:choose>
-->

          <xsl:when test="$first-char='&#x2102;'">
            <xsl:text>\mathbb{C}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x210B;'">
            <xsl:text>\mathcal{H}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x210C;'">
            <xsl:text>\mathfrak{H}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x210F;'">
            <xsl:text xml:space="preserve">\hbar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2110;'">
            <xsl:text>\mathcal{I}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2111;'">
            <xsl:text xml:space="preserve">\Im </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2111;'">
            <xsl:text>\mathfrak{I}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2112;'">
            <xsl:if test="$output-mode='SW-LaTeX'">
              <xsl:text xml:space="preserve">\tciLaplace </xsl:text>
            </xsl:if>
            <xsl:if test="$output-mode='Portable-LaTeX'">
              <xsl:text>\mathcal{L}</xsl:text>
            </xsl:if>
          </xsl:when>
          <xsl:when test="$first-char='&#x2113;'">
            <xsl:text xml:space="preserve">\ell </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2115;'">
            <xsl:text>\mathbb{N}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2118;'">
            <xsl:text xml:space="preserve">\wp </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2119;'">
            <xsl:text>\mathbb{P}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x211A;'">
            <xsl:text>\mathbb{Q}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x211C;'">
            <xsl:text xml:space="preserve">\Re </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x211C;'">
            <xsl:text>\mathfrak{R}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x211D;'">
            <xsl:text>\mathbb{R}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2124;'">
            <xsl:text>\mathbb{Z}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2127;'">
            <xsl:text xml:space="preserve">\mho </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x212C;'">
            <xsl:text>\mathcal{B}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x212F;'">
            <xsl:text>\mathcal{e}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2130;'">
            <xsl:text>\mathcal{E}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2131;'">
            <xsl:if test="$output-mode='SW-LaTeX'">
              <xsl:text xml:space="preserve">\tciFourier </xsl:text>
            </xsl:if>
            <xsl:if test="$output-mode='Portable-LaTeX'">
              <xsl:text>\mathcal{F}</xsl:text>
            </xsl:if>
          </xsl:when>
          <xsl:when test="$first-char='&#x2133;'">
            <xsl:text>\mathcal{M}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2134;'">
            <xsl:text>\mathcal{o}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2135;'">
            <xsl:text xml:space="preserve">\aleph </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2136;'">
            <xsl:text xml:space="preserve">\beth </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2137;'">
            <xsl:text xml:space="preserve">\gimel </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2138;'">
            <xsl:text xml:space="preserve">\daleth </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x03F6;'">
            <xsl:text xml:space="preserve">\backepsilon </xsl:text>
          </xsl:when>

          <xsl:when test="$first-char='&#x2146;'">
            <xsl:text>d</xsl:text>
          </xsl:when>

<!-- JLF - add test characters here, put in proper place when done -->
          <xsl:when test="$first-char='&#x2132;'">
            <xsl:text>\Finv </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2141;'">
            <xsl:text>\Game </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x1D55C;'">
            <xsl:text xml:space="preserve">\Bbbk </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x019B;'">
            <xsl:text xml:space="preserve">\lambdabar </xsl:text>
          </xsl:when>


<!--  Number Forms
          <xsl:otherwise>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:when test="ms:string-compare($first-char,'&#x218F') = -1">
        <xsl:choose>
-->

<!--  Arrows
          <xsl:otherwise>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:when test="ms:string-compare($first-char,'&#x21FF') = -1">
        <xsl:choose>
-->
          <xsl:when test="$first-char='&#x2190;'">
            <xsl:text xml:space="preserve">\leftarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2191;'">
            <xsl:text xml:space="preserve">\uparrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2192;'">
            <xsl:text xml:space="preserve">\rightarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2193;'">
            <xsl:text xml:space="preserve">\downarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2194;'">
            <xsl:text xml:space="preserve">\leftrightarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2195;'">
            <xsl:text xml:space="preserve">\updownarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2196;'">
            <xsl:text xml:space="preserve">\nwarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2197;'">
            <xsl:text xml:space="preserve">\nearrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2198;'">
            <xsl:text xml:space="preserve">\searrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2199;'">
            <xsl:text xml:space="preserve">\swarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x219A;'">
            <xsl:text xml:space="preserve">\nleftarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x219B;'">
            <xsl:text xml:space="preserve">\nrightarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x219E;'">
            <xsl:text xml:space="preserve">\twoheadleftarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21A0;'">
            <xsl:text xml:space="preserve">\twoheadrightarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21A2;'">
            <xsl:text xml:space="preserve">\leftarrowtail </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21A3;'">
            <xsl:text xml:space="preserve">\rightarrowtail </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21A6;'">
            <xsl:text xml:space="preserve">\mapsto </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21A9;'">
            <xsl:text xml:space="preserve">\hookleftarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21AA;'">
            <xsl:text xml:space="preserve">\hookrightarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21AB;'">
            <xsl:text xml:space="preserve">\looparrowleft </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21AC;'">
            <xsl:text xml:space="preserve">\looparrowright </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21AD;'">
            <xsl:text xml:space="preserve">\leftrightsquigarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21AE;'">
            <xsl:text xml:space="preserve">\nleftrightarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21B0;'">
            <xsl:text xml:space="preserve">\Lsh </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21B1;'">
            <xsl:text xml:space="preserve">\Rsh </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21B6;'">
            <xsl:text xml:space="preserve">\curvearrowleft </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21B7;'">
            <xsl:text xml:space="preserve">\curvearrowright </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21BA;'">
            <xsl:text xml:space="preserve">\circlearrowleft </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21BB;'">
            <xsl:text xml:space="preserve">\circlearrowright </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21BC;'">
            <xsl:text xml:space="preserve">\leftharpoonup </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21BD;'">
            <xsl:text xml:space="preserve">\leftharpoondown </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21BE;'">
            <xsl:text xml:space="preserve">\upharpoonright </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21BF;'">
            <xsl:text xml:space="preserve">\upharpoonleft </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21C0;'">
            <xsl:text xml:space="preserve">\rightharpoonup </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21C1;'">
            <xsl:text xml:space="preserve">\rightharpoondown </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21C2;'">
            <xsl:text xml:space="preserve">\downharpoonright </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21C3;'">
            <xsl:text xml:space="preserve">\downharpoonleft </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21C4;'">
            <xsl:text xml:space="preserve">\rightleftarrows </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21C6;'">
            <xsl:text xml:space="preserve">\leftrightarrows </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21C7;'">
            <xsl:text xml:space="preserve">\leftleftarrows </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21C8;'">
            <xsl:text xml:space="preserve">\upuparrows </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21C9;'">
            <xsl:text xml:space="preserve">\rightrightarrows </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21CA;'">
            <xsl:text xml:space="preserve">\downdownarrows </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21CB;'">
            <xsl:text xml:space="preserve">\leftrightharpoons </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21CC;'">
            <xsl:text xml:space="preserve">\rightleftharpoons </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21CD;'">
            <xsl:text xml:space="preserve">\nLeftarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21CE;'">
            <xsl:text xml:space="preserve">\nLeftrightarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21CF;'">
            <xsl:text xml:space="preserve">\nRightarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21D0;'">
            <xsl:text xml:space="preserve">\impliedby </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21D0;'">
            <xsl:text xml:space="preserve">\Leftarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21D1;'">
            <xsl:text xml:space="preserve">\Uparrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21D2;'">
            <xsl:text xml:space="preserve">\implies </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21D2;'">
            <xsl:text xml:space="preserve">\Rightarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21D3;'">
            <xsl:text xml:space="preserve">\Downarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21D4;'">
            <xsl:text xml:space="preserve">\iff </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21D4;'">
            <xsl:text xml:space="preserve">\Leftrightarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21D5;'">
            <xsl:text xml:space="preserve">\Updownarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21DA;'">
            <xsl:text xml:space="preserve">\Lleftarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21DB;'">
            <xsl:text xml:space="preserve">\Rrightarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x219D;'">
            <xsl:text xml:space="preserve">\rightsquigarrow </xsl:text>
          </xsl:when>

<!--  Mathematical Operators
          <xsl:otherwise>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:when test="ms:string-compare($first-char,'&#x22FF') = -1">
        <xsl:choose>
-->
<!--
          <xsl:when test="$first-char='&#xxxxx;'">
            <xsl:text xml:space="preserve">\shortmid </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xxxxx;'">
            <xsl:text xml:space="preserve">\shortparallel </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xxxxx;'">
            <xsl:text xml:space="preserve">\thickapprox </xsl:text>
          </xsl:when>
-->


          <xsl:when test="$first-char='&#x2200;'">
            <xsl:text xml:space="preserve">\forall </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2201;'">
            <xsl:text xml:space="preserve">\complement </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2202;'">
            <xsl:text xml:space="preserve">\partial </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2203;'">
            <xsl:text xml:space="preserve">\exists </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2204;'">
            <xsl:text xml:space="preserve">\nexists </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2205;'">
            <xsl:text xml:space="preserve">\varnothing </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2207;'">
            <xsl:text xml:space="preserve">\nabla </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2208;'">
            <xsl:text xml:space="preserve">\in </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2209;'">
            <xsl:text xml:space="preserve">\notin </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x220A;'">
            <xsl:text xml:space="preserve">\in </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x220B;'">
            <xsl:text xml:space="preserve">\ni </xsl:text>
          </xsl:when>
	   <xsl:when test="$first-char='&#x220F;'">
            <xsl:text>\tprod </xsl:text>
          </xsl:when>
	  <xsl:when test="$first-char='&#x2210;'">
            <xsl:text>\tcoprod </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2211;'">
            <xsl:text xml:space="preserve">\sum </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2212;'">
            <xsl:text xml:space="preserve">-</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2213;'">
            <xsl:text xml:space="preserve">\mp </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2214;'">
            <xsl:text xml:space="preserve">\dotplus </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29F5;'">
            <xsl:text xml:space="preserve">\setminus </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2216;'">
            <xsl:text xml:space="preserve">\smallsetminus </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2217;'">
            <xsl:text xml:space="preserve">\ast </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2218;'">
            <xsl:text xml:space="preserve">\circ </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x221A;'">
            <xsl:text xml:space="preserve">\surd </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x221D;'">
            <xsl:text xml:space="preserve">\propto </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x221D;'">
            <xsl:text xml:space="preserve">\varpropto </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x221E;'">
            <xsl:text xml:space="preserve">\infty </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x221F;'">
            <xsl:text xml:space="preserve">\rightangle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2220;'">
            <xsl:text xml:space="preserve">\angle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2221;'">
            <xsl:text xml:space="preserve">\measuredangle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2222;'">
            <xsl:text xml:space="preserve">\sphericalangle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2223;'">
            <xsl:text xml:space="preserve">\mid </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2224;'">
            <xsl:text xml:space="preserve">\nmid </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2224;'">
            <xsl:text xml:space="preserve">\nshortmid </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2225;'">
            <xsl:text xml:space="preserve">\parallel </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2226;'">
            <xsl:text xml:space="preserve">\nparallel </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2226;'">
            <xsl:text xml:space="preserve">\nshortparallel </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2227;'">
            <xsl:text xml:space="preserve">\wedge </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2228;'">
            <xsl:text xml:space="preserve">\vee </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2229;'">
            <xsl:text xml:space="preserve">\cap </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x222A;'">
            <xsl:text xml:space="preserve">\cup </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x222B;'">
            <xsl:text xml:space="preserve">\smallint </xsl:text>
          </xsl:when>
	  <xsl:when test="$first-char='&#x222C;'">
            <xsl:text>\iiint </xsl:text>
          </xsl:when>
	  <xsl:when test="$first-char='&#x222D;'">
            <xsl:text>\iiiint </xsl:text>
          </xsl:when>
	  <xsl:when test="$first-char='&#x222E;'">
            <xsl:text>\toint </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2234;'">
            <xsl:text xml:space="preserve">\therefore </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2235;'">
            <xsl:text xml:space="preserve">\because </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2237;'">
            <xsl:text xml:space="preserve">\Colon </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x223C;'">
            <xsl:text xml:space="preserve">\sim </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x223C;'">
            <xsl:text xml:space="preserve">\thicksim </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x223D;'">
            <xsl:text xml:space="preserve">\backsim </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2240;'">
            <xsl:text xml:space="preserve">\wr </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2241;'">
            <xsl:text xml:space="preserve">\nsim </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2242;'">
            <xsl:text xml:space="preserve">\eqsim </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2243;'">
            <xsl:text xml:space="preserve">\simeq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2245;'">
            <xsl:text xml:space="preserve">\cong </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2247;'">
            <xsl:text xml:space="preserve">\ncong </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2248;'">
            <xsl:text xml:space="preserve">\approx </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x224A;'">
            <xsl:text xml:space="preserve">\approxeq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x224D;'">
            <xsl:text xml:space="preserve">\asymp </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x224E;'">
            <xsl:text xml:space="preserve">\Bumpeq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x224F;'">
            <xsl:text xml:space="preserve">\bumpeq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2250;'">
            <xsl:text xml:space="preserve">\doteq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2251;'">
            <xsl:text xml:space="preserve">\doteqdot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2252;'">
            <xsl:text xml:space="preserve">\fallingdotseq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2253;'">
            <xsl:text xml:space="preserve">\risingdotseq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2256;'">
            <xsl:text xml:space="preserve">\eqcirc </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2257;'">
            <xsl:text xml:space="preserve">\circeq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x225C;'">
            <xsl:text xml:space="preserve">\triangleq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x225F;'">
            <xsl:text xml:space="preserve">\questeq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2260;'">
            <xsl:text xml:space="preserve">\neq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2261;'">
            <xsl:text xml:space="preserve">\equiv </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2262;'">
            <xsl:text xml:space="preserve">\not\equiv </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2264;'">
            <xsl:text xml:space="preserve">\leq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2265;'">
            <xsl:text xml:space="preserve">\geq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2266;'">
	    <xsl:choose>
	      <xsl:when test="$next-char='&#x0338;'">
		<xsl:text>\nleqq </xsl:text>
	      </xsl:when>
	      <xsl:otherwise>
		<xsl:text>\leqq </xsl:text>
	      </xsl:otherwise>
	    </xsl:choose>
          </xsl:when>
          <xsl:when test="$first-char='&#x2267;'">
	    <xsl:choose>
	      <xsl:when test="$next-char='&#x0338;'">
		<xsl:text>\ngeqq </xsl:text>
	      </xsl:when>
	      <xsl:otherwise>
		<xsl:text>\geqq </xsl:text>
	      </xsl:otherwise>
	    </xsl:choose>
          </xsl:when>
          <xsl:when test="$first-char='&#x2268;'">
            <xsl:text xml:space="preserve">\lvertneqq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2269;'">
            <xsl:text xml:space="preserve">\gvertneqq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2269;'">
            <xsl:text xml:space="preserve">\gneqq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2269;'">
            <xsl:text xml:space="preserve">\gvertneqq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x226A;'">
            <xsl:text xml:space="preserve">\ll </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x226B;'">
            <xsl:text xml:space="preserve">\gg </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x226C;'">
            <xsl:text xml:space="preserve">\between </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x226E;'">
            <xsl:text xml:space="preserve">\nless </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x226F;'">
            <xsl:text xml:space="preserve">\ngtr </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2270;'">
            <xsl:text xml:space="preserve">\nleq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2270;'">
            <xsl:text xml:space="preserve">\nleqq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2270;'">
            <xsl:text xml:space="preserve">\nleqslant </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2271;'">
            <xsl:text xml:space="preserve">\ngeq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2271;'">
            <xsl:text xml:space="preserve">\ngeqq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2271;'">
            <xsl:text xml:space="preserve">\ngeqslant </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2272;'">
            <xsl:text xml:space="preserve">\lesssim </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2273;'">
            <xsl:text xml:space="preserve">\gtrsim </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2276;'">
            <xsl:text xml:space="preserve">\lessgtr </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2277;'">
            <xsl:text xml:space="preserve">\gtrless </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x227A;'">
            <xsl:text xml:space="preserve">\prec </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x227B;'">
            <xsl:text xml:space="preserve">\succ </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x227C;'">
            <xsl:text xml:space="preserve">\preccurlyeq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x227D;'">
            <xsl:text xml:space="preserve">\succcurlyeq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x227D;'">
            <xsl:text xml:space="preserve">\succeq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x227E;'">
            <xsl:text xml:space="preserve">\precsim </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x227F;'">
            <xsl:text xml:space="preserve">\succsim </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2280;'">
            <xsl:text xml:space="preserve">\nprec </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2281;'">
            <xsl:text xml:space="preserve">\nsucc </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2282;'">
            <xsl:text xml:space="preserve">\subset </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2283;'">
            <xsl:text xml:space="preserve">\supset </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2284;'">
            <xsl:text xml:space="preserve">\not\subset </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2285;'">
            <xsl:text xml:space="preserve">\not\supset </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2286;'">
            <xsl:text xml:space="preserve">\subseteq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2287;'">
            <xsl:text xml:space="preserve">\supseteq </xsl:text>
          </xsl:when>
<!--
          <xsl:when test="$first-char='&#x2288;'">
            <xsl:text xml:space="preserve">\nsubseteq </xsl:text>
          </xsl:when>
-->
          <xsl:when test="$first-char='&#x2288;'">
            <xsl:text xml:space="preserve">\nsubseteqq </xsl:text>
          </xsl:when>
<!--
          <xsl:when test="$first-char='&#x2289;'">
            <xsl:text xml:space="preserve">\nsupseteq </xsl:text>
          </xsl:when>
-->
          <xsl:when test="$first-char='&#x2289;'">
            <xsl:text xml:space="preserve">\nsupseteqq </xsl:text>
          </xsl:when>
<!--
          <xsl:when test="$first-char='&#x228A;'">
            <xsl:text xml:space="preserve">\subsetneqq </xsl:text>
          </xsl:when>
-->
          <xsl:when test="$first-char='&#x228A;'">
            <xsl:text xml:space="preserve">\subsetneq </xsl:text>
          </xsl:when>
<!--
          <xsl:when test="$first-char='&#x228B;'">
            <xsl:text xml:space="preserve">\supsetneqq </xsl:text>
          </xsl:when>
-->
          <xsl:when test="$first-char='&#x228B;'">
            <xsl:text xml:space="preserve">\supsetneq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x228E;'">
            <xsl:text xml:space="preserve">\uplus </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x228F;'">
            <xsl:text xml:space="preserve">\sqsubset </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2290;'">
            <xsl:text xml:space="preserve">\sqsupset </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2291;'">
            <xsl:text xml:space="preserve">\sqsubseteq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2292;'">
            <xsl:text xml:space="preserve">\sqsupseteq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2293;'">
            <xsl:text xml:space="preserve">\sqcap </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2294;'">
            <xsl:text xml:space="preserve">\sqcup </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2295;'">
            <xsl:text xml:space="preserve">\oplus </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2296;'">
            <xsl:text xml:space="preserve">\ominus </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2297;'">
            <xsl:text xml:space="preserve">\otimes </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2298;'">
            <xsl:text xml:space="preserve">\oslash </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2299;'">
            <xsl:text xml:space="preserve">\odot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x229A;'">
            <xsl:text xml:space="preserve">\circledcirc </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x229B;'">
            <xsl:text xml:space="preserve">\circledast </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x229D;'">
            <xsl:text xml:space="preserve">\circleddash </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x229E;'">
            <xsl:text xml:space="preserve">\boxplus </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x229F;'">
            <xsl:text xml:space="preserve">\boxminus </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22A0;'">
            <xsl:text xml:space="preserve">\boxtimes </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22A1;'">
            <xsl:text xml:space="preserve">\boxdot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22A2;'">
            <xsl:text xml:space="preserve">\vdash </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22A3;'">
            <xsl:text xml:space="preserve">\dashv </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22A4;'">
            <xsl:text xml:space="preserve">\top </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22A5;'">
            <xsl:text xml:space="preserve">\bot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27C2;'">
            <xsl:text xml:space="preserve">\perp </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22A7;'">
            <xsl:text xml:space="preserve">\models </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22A8;'">
            <xsl:text xml:space="preserve">\vDash </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22A9;'">
            <xsl:text xml:space="preserve">\Vdash </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22AA;'">
            <xsl:text xml:space="preserve">\Vvdash </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22AC;'">
            <xsl:text xml:space="preserve">\nvdash </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22AD;'">
            <xsl:text xml:space="preserve">\nvDash </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22AE;'">
            <xsl:text xml:space="preserve">\nVdash </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22AF;'">
            <xsl:text xml:space="preserve">\nVDash </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22B2;'">
            <xsl:text xml:space="preserve">\lhd </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22B2;'">
            <xsl:text xml:space="preserve">\vartriangleleft </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22B3;'">
            <xsl:text xml:space="preserve">\rhd </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22B3;'">
            <xsl:text xml:space="preserve">\vartriangleright </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22B4;'">
            <xsl:text xml:space="preserve">\trianglelefteq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22B4;'">
            <xsl:text xml:space="preserve">\unlhd </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22B5;'">
            <xsl:text xml:space="preserve">\trianglerighteq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22B5;'">
            <xsl:text xml:space="preserve">\unrhd </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22B8;'">
            <xsl:text xml:space="preserve">\multimap </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22BA;'">
            <xsl:text xml:space="preserve">\intercal </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22BB;'">
            <xsl:text xml:space="preserve">\veebar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22BC;'">
            <xsl:text xml:space="preserve">\barwedge </xsl:text>
          </xsl:when>
	  <xsl:when test="$first-char='&#x22C0;'">
            <xsl:text>\tbigwedge </xsl:text>
          </xsl:when>
	  <xsl:when test="$first-char='&#x22C1;'">
            <xsl:text>\tbigvee </xsl:text>
          </xsl:when>
	  <xsl:when test="$first-char='&#x22C2;'">
            <xsl:text>\tbigcap </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25C7;'">
            <xsl:text xml:space="preserve">\Diamond </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22C4;'">
            <xsl:text xml:space="preserve">\diamond </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22C5;'">
            <xsl:text xml:space="preserve">\cdot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22C6;'">
            <xsl:text xml:space="preserve">\star </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22C7;'">
            <xsl:text xml:space="preserve">\divideontimes </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22C8;'">
            <xsl:text xml:space="preserve">\bowtie </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22C9;'">
            <xsl:text xml:space="preserve">\ltimes </xsl:text>
          </xsl:when>
	  
          <xsl:when test="$first-char='&#x22CA;'">
            <xsl:text xml:space="preserve">\rtimes </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22CB;'">
            <xsl:text xml:space="preserve">\leftthreetimes </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22CC;'">
            <xsl:text xml:space="preserve">\rightthreetimes </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22CD;'">
            <xsl:text xml:space="preserve">\backsimeq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22CE;'">
            <xsl:text xml:space="preserve">\curlyvee </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22CF;'">
            <xsl:text xml:space="preserve">\curlywedge </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22D0;'">
            <xsl:text xml:space="preserve">\Subset </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22D1;'">
            <xsl:text xml:space="preserve">\Supset </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22D2;'">
            <xsl:text xml:space="preserve">\Cap </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22D3;'">
            <xsl:text xml:space="preserve">\Cup </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22D4;'">
            <xsl:text xml:space="preserve">\pitchfork </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22D6;'">
            <xsl:text xml:space="preserve">\lessdot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22D7;'">
            <xsl:text xml:space="preserve">\gtrdot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22D8;'">
            <xsl:text xml:space="preserve">\lll </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22D9;'">
            <xsl:text xml:space="preserve">\ggg </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22DA;'">
            <xsl:text xml:space="preserve">\lesseqgtr </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22DB;'">
            <xsl:text xml:space="preserve">\gtreqless </xsl:text>
          </xsl:when>
  	  <xsl:when test="$first-char='&#x2A01;'">
            <xsl:text>\tbigoplus </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A95;'">
            <xsl:text xml:space="preserve">\eqslantless </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A1D;'">
            <xsl:text xml:space="preserve">\Join </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A96;'">
            <xsl:text xml:space="preserve">\eqslantgtr </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22DE;'">
            <xsl:text xml:space="preserve">\curlyeqprec </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22DF;'">
            <xsl:text xml:space="preserve">\curlyeqsucc </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22E6;'">
            <xsl:text xml:space="preserve">\lnsim </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22E7;'">
            <xsl:text xml:space="preserve">\gnsim </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22E8;'">
            <xsl:text xml:space="preserve">\precnsim </xsl:text>
          </xsl:when>
	            <xsl:when test="$first-char='&#x22E9;'">
            <xsl:text xml:space="preserve">\succnsim </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22EA;'">
            <xsl:text xml:space="preserve">\ntriangleleft </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22EB;'">
            <xsl:text xml:space="preserve">\ntriangleright </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22EC;'">
            <xsl:text xml:space="preserve">\ntrianglelefteq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22ED;'">
            <xsl:text xml:space="preserve">\ntrianglerighteq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22EE;'">
            <xsl:text xml:space="preserve">\vdots </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22EF;'">
            <xsl:text xml:space="preserve">\cdots </xsl:text>
          </xsl:when>
	   <xsl:when test="$first-char='&#x22F0;'">
            <xsl:text xml:space="preserve">\ddots </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22F1;'">
            <xsl:text xml:space="preserve">\diagdown </xsl:text>
          </xsl:when>
	  <xsl:when test="$first-char='&#x2AB9;'">
            <xsl:text xml:space="preserve">\precnapprox </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2ABA;'">
            <xsl:text xml:space="preserve">\succnapprox </xsl:text>
          </xsl:when>
	  <xsl:when test="$first-char='&#x2ACB;'">
            <xsl:text xml:space="preserve">\subsetneqq </xsl:text>
          </xsl:when>
	  <xsl:when test="$first-char='&#x2ACC;'">
            <xsl:text xml:space="preserve">\supsetneqq </xsl:text>
          </xsl:when>

<!--  Miscellaneous Technical
          <xsl:otherwise>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:when test="ms:string-compare($first-char,'&#x237F') = -1">
        <xsl:choose>
-->
          <xsl:when test="$first-char='&#x2306;'">
            <xsl:text xml:space="preserve">\doublebarwedge </xsl:text>
          </xsl:when>

          <xsl:when test="$first-char='&#x2308;'">
            <xsl:text xml:space="preserve">\lceil </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2309;'">
            <xsl:text xml:space="preserve">\rceil </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x230A;'">
            <xsl:text xml:space="preserve">\lfloor </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x230B;'">
            <xsl:text xml:space="preserve">\rfloor </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x231C;'">
            <xsl:text xml:space="preserve">\ulcorner </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x231E;'">
            <xsl:text xml:space="preserve">\llcorner </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x231D;'">
            <xsl:text xml:space="preserve">\urcorner </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x231F;'">
            <xsl:text xml:space="preserve">\lrcorner </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2329;'
          or              $first-char='&#x3008;'
          or              $first-char='&#x27E8;'">
            <xsl:text xml:space="preserve">\langle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x232A;'
          or              $first-char='&#x3009;'
          or              $first-char='&#x27E9;'">
            <xsl:text xml:space="preserve">\rangle </xsl:text>
          </xsl:when>

          <xsl:when test="$first-char='&#x2322;'">
            <xsl:text xml:space="preserve">\frown </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2323;'">
            <xsl:text xml:space="preserve">\smile </xsl:text>
          </xsl:when>
<!--
\leftdoublebracket<uID3.15.11>
\rightdoublebracket<uID3.15.12>

\rmoustache<uID3.15.20>
\lmoustache<uID3.15.21>
\rgroup<uID3.15.22>
\lgroup<uID3.15.23>
\arrowvert<uID3.15.24>
\Arrowvert<uID3.15.25>
\bracevert<uID3.15.26>
-->


<!--  Control Pictures
          <xsl:otherwise>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:when test="ms:string-compare($first-char,'&#x243F') = -1">
        <xsl:choose>
-->

<!--  Enclosed Alphanumerics
          <xsl:otherwise>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:when test="ms:string-compare($first-char,'&#x24FF') = -1">
        <xsl:choose>
-->
          <xsl:when test="$first-char='&#x24C8;'">
            <xsl:text xml:space="preserve">\circledS </xsl:text>
          </xsl:when>

<!--  Box Drawing
          <xsl:otherwise>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:when test="ms:string-compare($first-char,'&#x257F') = -1">
        <xsl:choose>
-->

<!--  Block Elements
          <xsl:otherwise>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:when test="ms:string-compare($first-char,'&#x259F') = -1">
        <xsl:choose>
-->

<!--  Geometric Shapes
          <xsl:otherwise>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:when test="ms:string-compare($first-char,'&#x25FF') = -1">
        <xsl:choose>
-->
          <xsl:when test="$first-char='&#x25A1;'">
            <xsl:text xml:space="preserve">\square </xsl:text>
           </xsl:when>
          <xsl:when test="$first-char='&#x25A1;'">
            <xsl:text xml:space="preserve">\Box </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25AA;'">
            <xsl:text xml:space="preserve">\blacksquare </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25B3;'">
            <xsl:text xml:space="preserve">\bigtriangleup </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25B4;'">
            <xsl:text xml:space="preserve">\blacktriangle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25B5;'">
            <xsl:text xml:space="preserve">\triangle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25B8;'">
            <xsl:text xml:space="preserve">\blacktriangleright </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25B9;'">
            <xsl:text xml:space="preserve">\triangleright </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25BD;'">
            <xsl:text xml:space="preserve">\bigtriangledown </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25BE;'">
            <xsl:text xml:space="preserve">\blacktriangledown </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25BF;'">
            <xsl:text xml:space="preserve">\triangledown </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25C2;'">
            <xsl:text xml:space="preserve">\blacktriangleleft </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25C3;'">
            <xsl:text xml:space="preserve">\triangleleft </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25CB;'">
            <xsl:text xml:space="preserve">\bigcirc </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25EF;'">
            <xsl:text xml:space="preserve">\bigcirc </xsl:text>
          </xsl:when>

<!--  Miscellaneous Symbols
          <xsl:otherwise>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:when test="ms:string-compare($first-char,'&#x267F') = -1">
        <xsl:choose>
-->

          <xsl:when test="$first-char='&#x2605;'">
            <xsl:text xml:space="preserve">\bigstar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2660;'">
            <xsl:text xml:space="preserve">\spadesuit </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2663;'">
            <xsl:text xml:space="preserve">\clubsuit </xsl:text>
          </xsl:when>
	  <xsl:when test="$first-char='&#x2665;'">
            <xsl:text xml:space="preserve">\heartsuit </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2666;'">
            <xsl:text xml:space="preserve">\diamondsuit </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x266D;'">
            <xsl:text xml:space="preserve">\flat </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x266E;'">
            <xsl:text xml:space="preserve">\natural </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x266F;'">
            <xsl:text xml:space="preserve">\sharp </xsl:text>
          </xsl:when>

<!--  Dingbats
          <xsl:otherwise>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:when test="ms:string-compare($first-char,'&#x27BF') = -1">
        <xsl:choose>
-->

          <xsl:when test="$first-char='&#x2713;'">
            <xsl:text xml:space="preserve">\checkmark </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2720;'">
            <xsl:text xml:space="preserve">\maltese </xsl:text>
          </xsl:when>


          <xsl:when test="$first-char='&#x27F5;'">
            <xsl:text xml:space="preserve">\longleftarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27F6;'">
            <xsl:text xml:space="preserve">\longrightarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27F7;'">
            <xsl:text xml:space="preserve">\longleftrightarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27F8;'">
            <xsl:text xml:space="preserve">\Longleftarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27F9;'">
            <xsl:text xml:space="preserve">\Longrightarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27FA;'">
            <xsl:text xml:space="preserve">\Longleftrightarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27FC;'">
            <xsl:text xml:space="preserve">\longmapsto </xsl:text>
          </xsl:when>

          <xsl:when test="$first-char='&#x290E;'">
            <xsl:text xml:space="preserve">\dashleftarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x290F;'">
            <xsl:text xml:space="preserve">\dashrightarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2933;'">
            <xsl:text xml:space="preserve">\leadsto </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25CA;'">
            <xsl:text xml:space="preserve">\lozenge </xsl:text>
          </xsl:when>

          <xsl:when test="$first-char='&#x29EB;'">
            <xsl:text xml:space="preserve">\blacklozenge </xsl:text>
          </xsl:when>


          <xsl:when test="$first-char='&#x2A38;'">
            <xsl:text xml:space="preserve">\circleddivide </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A3F;'">
            <xsl:text xml:space="preserve">\amalg </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A7D;'">
	    <xsl:choose>
	      <xsl:when test="$next-char='&#x0338;'">
		<xsl:text>\nleqslant </xsl:text>
	      </xsl:when>
	      <xsl:otherwise>
		<xsl:text>\leqslant </xsl:text>
	      </xsl:otherwise>
	    </xsl:choose>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A7E;'">
	    <xsl:choose>
	      <xsl:when test="$next-char='&#x0338;'">
		<xsl:text>\ngeqslant </xsl:text>
	      </xsl:when>
	      <xsl:otherwise>
		<xsl:text>\geqslant </xsl:text>
	      </xsl:otherwise>
	    </xsl:choose>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A85;'">
            <xsl:text xml:space="preserve">\lessapprox </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A86;'">
            <xsl:text xml:space="preserve">\gtrapprox </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A87;'">
            <xsl:text xml:space="preserve">\lneq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A88;'">
            <xsl:text xml:space="preserve">\gneq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A89;'">
            <xsl:text xml:space="preserve">\lnapprox </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A8A;'">
            <xsl:text xml:space="preserve">\gnapprox </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A8B;'">
            <xsl:text xml:space="preserve">\lesseqqgtr </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A8C;'">
            <xsl:text xml:space="preserve">\gtreqqless </xsl:text>
          </xsl:when>
	  <xsl:when test="$first-char='&#x2AAF;'">
	    <xsl:choose>
	      <xsl:when test="$next-char='&#x0338;'">
		<xsl:text>\npreceq </xsl:text>
	      </xsl:when>
	      <xsl:otherwise>
		<xsl:text>\preceq </xsl:text>
	      </xsl:otherwise>
	    </xsl:choose>
	  </xsl:when>
          
          <xsl:when test="$first-char='&#x2AB0;'">
	    <xsl:choose>
	      <xsl:when test="$next-char='&#x0338;'">
		<xsl:text>\nsucceq </xsl:text>
	      </xsl:when>
	      <xsl:otherwise>
		<xsl:text>\succeq </xsl:text>
	      </xsl:otherwise>
	    </xsl:choose>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AB5;'">
            <xsl:text xml:space="preserve">\precneqq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AB6;'">
            <xsl:text xml:space="preserve">\succneqq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AB7;'">
            <xsl:text xml:space="preserve">\precapprox </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AB8;'">
            <xsl:text xml:space="preserve">\succapprox </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AC5;'">
	    <xsl:choose>
	      <xsl:when test="$next-char='&#x0338;'">
		<xsl:text>\nsubseteqq </xsl:text>
	      </xsl:when>
	      <xsl:otherwise>
		<xsl:text>\subseteqq </xsl:text>
	      </xsl:otherwise>
	    </xsl:choose>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AC6;'">
	    <xsl:choose>
	      <xsl:when test="$next-char='&#x0338;'">
		<xsl:text>\nsupseteqq </xsl:text>
	      </xsl:when>
	      <xsl:otherwise>
		<xsl:text>\supseteqq </xsl:text>
	      </xsl:otherwise>
	    </xsl:choose>
          </xsl:when>

          <xsl:when test="$first-char='&#xE200;'">
            <xsl:text xml:space="preserve">\Longleftarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE201;'">
            <xsl:text xml:space="preserve">\longleftarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE202;'">
            <xsl:text xml:space="preserve">\Longleftrightarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE203;'">
            <xsl:text xml:space="preserve">\longleftrightarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE204;'">
            <xsl:text xml:space="preserve">\Longrightarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE205;'">
            <xsl:text xml:space="preserve">\longrightarrow </xsl:text>
          </xsl:when>

          <xsl:when test="$first-char='&#xE206;'">
            <xsl:text xml:space="preserve">\dashleftarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE207;'">
            <xsl:text xml:space="preserve">\dashrightarrow </xsl:text>
          </xsl:when>
<!--
          <xsl:when test="$first-char='&#xE207;'">
            <xsl:text xml:space="preserve">\longmapsto </xsl:text>
          </xsl:when>
-->
          <xsl:when test="$first-char='&#xE2A1;'">
            <xsl:text xml:space="preserve">\gvertneqq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE2A4;'">
            <xsl:text xml:space="preserve">\lvertneqq </xsl:text>
          </xsl:when>

          <xsl:when test="$first-char='&#xE2A6;'">
            <xsl:text xml:space="preserve">\ngeq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE2A7;'">
            <xsl:text xml:space="preserve">\nleq </xsl:text>
          </xsl:when>

          <xsl:when test="$first-char='&#xE2AA;'">
            <xsl:text xml:space="preserve">\nshortmid </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE2AB;'">
            <xsl:text xml:space="preserve">\nshortparallel </xsl:text>
          </xsl:when>

          <xsl:when test="$first-char='&#xE2B8;'">
            <xsl:text xml:space="preserve">\varsubsetneqq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE2B9;'">
            <xsl:text xml:space="preserve">\varsubsetneq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE2BA;'">
            <xsl:text xml:space="preserve">\varsupsetneq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE2BB;'">
            <xsl:text xml:space="preserve">\varsupsetneqq </xsl:text>
          </xsl:when>

          <xsl:when test="$first-char='&#xE2D3;'">
            <xsl:text xml:space="preserve">\emptyset </xsl:text>
          </xsl:when>
<!--JLF - testing alternate symbol for jmath
          <xsl:when test="$first-char='&#xE2D4;'">
            <xsl:text xml:space="preserve">\jmath </xsl:text>
          </xsl:when>
          -->
                       <xsl:when test="$first-char='&#x1D6A5;'">
            <xsl:text xml:space="preserve">\jmath </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE2D5;'">
            <xsl:text xml:space="preserve">\hslash </xsl:text>
          </xsl:when>

          <xsl:when test="$first-char='&#xE2FE;'">
            <xsl:text xml:space="preserve">\preceq </xsl:text>
          </xsl:when>

          <xsl:when test="$first-char='&#xE301;'">
            <xsl:text xml:space="preserve">\shortmid </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE302;'">
            <xsl:text xml:space="preserve">\shortparallel </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE429;'">
            <xsl:text xml:space="preserve">\thicksim </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE306;'">
            <xsl:text xml:space="preserve">\thickapprox </xsl:text>
          </xsl:when>


          <xsl:when test="$first-char='&#xE365;'">
            <xsl:text xml:space="preserve">\iff </xsl:text>
          </xsl:when>

          <xsl:when test="$first-char='&#xE500;'">
            <xsl:text>\mathbb{A}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE501;'">
            <xsl:text>\mathbb{B}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE503;'">
            <xsl:text>\mathbb{D}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE504;'">
            <xsl:text>\mathbb{E}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE505;'">
            <xsl:text>\mathbb{F}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE506;'">
            <xsl:text>\mathbb{G}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE507;'">
            <xsl:text>\mathbb{H}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE508;'">
            <xsl:text>\mathbb{I}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE509;'">
            <xsl:text>\mathbb{J}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE50A;'">
            <xsl:text>\mathbb{K}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE50B;'">
            <xsl:text>\mathbb{L}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE50C;'">
            <xsl:text>\mathbb{M}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE50E;'">
            <xsl:text>\mathbb{O}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE512;'">
            <xsl:text>\mathbb{S}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE513;'">
            <xsl:text>\mathbb{T}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE514;'">
            <xsl:text>\mathbb{U}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE515;'">
            <xsl:text>\mathbb{V}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE516;'">
            <xsl:text>\mathbb{W}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE517;'">
            <xsl:text>\mathbb{X}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE518;'">
            <xsl:text>\mathbb{Y}</xsl:text>
          </xsl:when>


          <xsl:when test="$first-char='&#xE520;'">
            <xsl:text>\mathcal{A}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE522;'">
            <xsl:text>\mathcal{C}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE523;'">
            <xsl:text>\mathcal{D}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE526;'">
            <xsl:text>\mathcal{G}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE529;'">
            <xsl:text>\mathcal{J}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE52A;'">
            <xsl:text>\mathcal{K}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE52D;'">
            <xsl:text>\mathcal{N}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE52E;'">
            <xsl:text>\mathcal{O}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE52F;'">
            <xsl:text>\mathcal{P}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE530;'">
            <xsl:text>\mathcal{Q}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE531;'">
            <xsl:text>\mathcal{R}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE532;'">
            <xsl:text>\mathcal{S}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE533;'">
            <xsl:text>\mathcal{T}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE534;'">
            <xsl:text>\mathcal{U}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE535;'">
            <xsl:text>\mathcal{V}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE536;'">
            <xsl:text>\mathcal{W}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE537;'">
            <xsl:text>\mathcal{X}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE538;'">
            <xsl:text>\mathcal{Y}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE539;'">
            <xsl:text>\mathcal{Z}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE540;'">
            <xsl:text>\mathcal{a}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE541;'">
            <xsl:text>\mathcal{b}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE542;'">
            <xsl:text>\mathcal{c}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE543;'">
            <xsl:text>\mathcal{d}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE545;'">
            <xsl:text>\mathcal{f}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE546;'">
            <xsl:text>\mathcal{g}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE547;'">
            <xsl:text>\mathcal{h}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE548;'">
            <xsl:text>\mathcal{i}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE549;'">
            <xsl:text>\mathcal{j}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE54A;'">
            <xsl:text>\mathcal{k}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE54B;'">
            <xsl:text>\mathcal{l}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE54C;'">
            <xsl:text>\mathcal{m}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE54D;'">
            <xsl:text>\mathcal{n}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE54F;'">
            <xsl:text>\mathcal{p}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE550;'">
            <xsl:text>\mathcal{q}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE551;'">
            <xsl:text>\mathcal{r}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE552;'">
            <xsl:text>\mathcal{s}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE553;'">
            <xsl:text>\mathcal{t}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE554;'">
            <xsl:text>\mathcal{u}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE555;'">
            <xsl:text>\mathcal{v}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE556;'">
            <xsl:text>\mathcal{w}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE557;'">
            <xsl:text>\mathcal{x}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE558;'">
            <xsl:text>\mathcal{y}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE559;'">
            <xsl:text>\mathcal{z}</xsl:text>
          </xsl:when>


          <xsl:when test="$first-char='&#xE560;'">
            <xsl:text>\mathfrak{A}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE561;'">
            <xsl:text>\mathfrak{B}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE562;'">
            <xsl:text>\mathfrak{C}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE563;'">
            <xsl:text>\mathfrak{D}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE564;'">
            <xsl:text>\mathfrak{E}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE565;'">
            <xsl:text>\mathfrak{F}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE566;'">
            <xsl:text>\mathfrak{G}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE569;'">
            <xsl:text>\mathfrak{J}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE56A;'">
            <xsl:text>\mathfrak{K}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE56B;'">
            <xsl:text>\mathfrak{L}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE56C;'">
            <xsl:text>\mathfrak{M}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE56D;'">
            <xsl:text>\mathfrak{N}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE56E;'">
            <xsl:text>\mathfrak{O}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE56F;'">
            <xsl:text>\mathfrak{P}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE570;'">
            <xsl:text>\mathfrak{Q}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE572;'">
            <xsl:text>\mathfrak{S}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE573;'">
            <xsl:text>\mathfrak{T}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE574;'">
            <xsl:text>\mathfrak{U}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE575;'">
            <xsl:text>\mathfrak{V}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE576;'">
            <xsl:text>\mathfrak{W}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE577;'">
            <xsl:text>\mathfrak{X}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE578;'">
            <xsl:text>\mathfrak{Y}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE579;'">
            <xsl:text>\mathfrak{Z}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE580;'">
            <xsl:text>\mathfrak{a}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE581;'">
            <xsl:text>\mathfrak{b}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE582;'">
            <xsl:text>\mathfrak{c}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE583;'">
            <xsl:text>\mathfrak{d}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE584;'">
            <xsl:text>\mathfrak{e}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE585;'">
            <xsl:text>\mathfrak{f}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE586;'">
            <xsl:text>\mathfrak{g}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE587;'">
            <xsl:text>\mathfrak{h}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE588;'">
            <xsl:text>\mathfrak{i}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE589;'">
            <xsl:text>\mathfrak{j}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE58A;'">
            <xsl:text>\mathfrak{k}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE58B;'">
            <xsl:text>\mathfrak{l}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE58C;'">
            <xsl:text>\mathfrak{m}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE58D;'">
            <xsl:text>\mathfrak{n}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE58E;'">
            <xsl:text>\mathfrak{o}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE58F;'">
            <xsl:text>\mathfrak{p}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE590;'">
            <xsl:text>\mathfrak{q}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE591;'">
            <xsl:text>\mathfrak{r}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE592;'">
            <xsl:text>\mathfrak{s}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE593;'">
            <xsl:text>\mathfrak{t}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE594;'">
            <xsl:text>\mathfrak{u}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE595;'">
            <xsl:text>\mathfrak{v}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE596;'">
            <xsl:text>\mathfrak{w}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE597;'">
            <xsl:text>\mathfrak{x}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE598;'">
            <xsl:text>\mathfrak{y}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE599;'">
            <xsl:text>\mathfrak{z}</xsl:text>
          </xsl:when>


          <xsl:when test="$first-char='&#xE5DC;'">
            <xsl:text xml:space="preserve">\npreceq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#xE5F1;'">
            <xsl:text xml:space="preserve">\nsucceq </xsl:text>
          </xsl:when>

<!-- !ENTITY ic or InvisibleComma "&#xE89C;" -->
          <xsl:when test="$first-char='&#xE89C;'">
            <xsl:text xml:space="preserve"></xsl:text>
          </xsl:when>

<!-- !ENTITY DD or CapitalDifferentialD "&#xF74B;" -->
          <xsl:when test="$first-char='&#xF74B;'">
            <xsl:text xml:space="preserve">D</xsl:text>
          </xsl:when>

<!-- ENTITY ee  ExponentialE "&#xF74D;" -->
          <xsl:when test="$first-char='&#xF74D;'">
            <xsl:text>e</xsl:text>
          </xsl:when>

<!-- ENTITY ii  ImaginaryI "&#xF74E;" -->
          <xsl:when test="$first-char='&#xF74E;'">
            <xsl:text>i</xsl:text>
          </xsl:when>

          <xsl:when test="$first-char='&#xF57D;'">
            <xsl:text xml:space="preserve">\longmapsto </xsl:text>
          </xsl:when>

<!-- !ENTITY underbrace "&#x23DF;" -->
          <xsl:when test="$first-char='&#x23DF;'">
            <!-- <xsl:text xml:space="preserve"></xsl:text> -->
          </xsl:when>
<!-- Some plane1 unicodes
            <mml:mi>&#x1D400;</mml:mi>
            <mml:mi mathvariant='bold'>A</mml:mi>

If we use this, don't forget 'roman'
            <mml:mi>&#x1D434;</mml:mi>
            <mml:mi mathvariant='italic'>A</mml:mi>

            <mml:mi>&#x1D468;</mml:mi>
            <mml:mi mathvariant='bold-italic'>A</mml:mi>

            <mml:mi>&#x1D538;</mml:mi>
            <mml:mi mathvariant='double-struck'>A</mml:mi>
            <mml:mi>&Aopf;</mml:mi>

            <mml:mi>&#x1D49C;</mml:mi>
            <mml:mi mathvariant='script'>A</mml:mi>
            <mml:mi>&Ascr;</mml:mi>

            <mml:mi>&#x1D4D0;</mml:mi>
            <mml:mi mathvariant='bold-script'>A</mml:mi>

            <mml:mi>&#x1D504;</mml:mi>
            <mml:mi mathvariant='fraktur'>A</mml:mi>
            <mml:mi>&Afr;</mml:mi>

            <mml:mi>&#x1D56C;</mml:mi>
            <mml:mi mathvariant='bold-fraktur'>A</mml:mi>

            <mml:mi>&#x1D5A0;</mml:mi>
            <mml:mi mathvariant='sans-serif'>A</mml:mi>

            <mml:mi>&#x1D5D4;</mml:mi>
            <mml:mi mathvariant='bold-sans-serif'>A</mml:mi>

            <mml:mi>&#x1D608;</mml:mi>
            <mml:mi mathvariant='sans-serif-italic'>A</mml:mi>

            <mml:mi>&#x1D63D;</mml:mi>
            <mml:mi mathvariant='sans-serif-bold-italic'>A</mml:mi>

            <mml:mi>&#x1D670;</mml:mi>
            <mml:mi mathvariant='monospace'>A</mml:mi>
-->

          <xsl:otherwise>
<!-- BBM: we want to let unidentified unicodes through, since XeLaTeX can handle them, and for PDFLaTeX we post-process
-->          <!-- jcs - this may need a \mathop{} -->
<!--              <xsl:text>\mathop{</xsl:text> -->
             <xsl:value-of select="$first-char"/>
<!--              <xsl:text>}</xsl:text> -->
          </xsl:otherwise>

        </xsl:choose>

<!--  The End
      </xsl:when>
    </xsl:choose>
-->
      </xsl:otherwise>

    </xsl:choose>

    <xsl:if test="string-length($unicode-cdata)&gt;1">
      <xsl:call-template name="chars-to-LaTeX-Math">
        <xsl:with-param name="unicode-cdata" select="substring($unicode-cdata,2)"/>
      </xsl:call-template>
    </xsl:if>
    </xsl:template>


</xsl:stylesheet>
