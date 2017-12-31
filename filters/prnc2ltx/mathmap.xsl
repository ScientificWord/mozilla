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
<!--      <xsl:when test="$first-char='&#x005C;'">
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
          </xsl:when> -->
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
            <xsl:text> </xsl:text>
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
	      <xsl:when test="$first-char='&#x2145;'">
            <xsl:text xml:space="preserve">\mathbb{D} </xsl:text>
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
            <xsl:text xml:space="preserve">\Leftarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21D1;'">
            <xsl:text xml:space="preserve">\Uparrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21D2;'">
            <xsl:text xml:space="preserve">\Rightarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21D3;'">
            <xsl:text xml:space="preserve">\Downarrow </xsl:text>
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
            <xsl:text xml:space="preserve">\lneqq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2269;'">
            <xsl:text xml:space="preserve">\gneqq </xsl:text>
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
          <xsl:when test="$first-char='&#x2288;'">
            <xsl:text xml:space="preserve">\nsubseteq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2289;'">
            <xsl:text xml:space="preserve">\nsupseteq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x228A;'">
			<xsl:choose>
              <xsl:when test="$next-char='&#xfe00;'">
                <xsl:text xml:space="preserve">\varsubsetneq </xsl:text>
              </xsl:when> 
			  <xsl:otherwise>
                <xsl:text xml:space="preserve">\subsetneq </xsl:text>
			  </xsl:otherwise>
		    </xsl:choose>
          </xsl:when>
          <xsl:when test="$first-char='&#x228B;'">
			<xsl:choose>
              <xsl:when test="$next-char='&#xfe00;'">
                <xsl:text xml:space="preserve">\varsupsetneq </xsl:text>
              </xsl:when> 
			  <xsl:otherwise>
                <xsl:text xml:space="preserve">\supsetneq </xsl:text>
			  </xsl:otherwise>
		    </xsl:choose>
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
<!-- Actually, no LaTeX for this character
	   <xsl:when test="$first-char='&#x22F0;'">
            <xsl:text xml:space="preserve">\relax </xsl:text>
          </xsl:when>
-->
	      <xsl:when test="$first-char='&#x27CB;'">
            <xsl:text xml:space="preserve">\diagup </xsl:text>
          </xsl:when>
	      <xsl:when test="$first-char='&#x27CD;'">
            <xsl:text xml:space="preserve">\diagdown </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22F1;'">
            <xsl:text xml:space="preserve">\ddots </xsl:text>
          </xsl:when>
	      <xsl:when test="$first-char='&#x2AB9;'">
            <xsl:text xml:space="preserve">\precnapprox </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2ABA;'">
            <xsl:text xml:space="preserve">\succnapprox </xsl:text>
          </xsl:when>
	      <xsl:when test="$first-char='&#x2ACB;'">
	        <xsl:choose>
              <xsl:when test="$next-char='&#xfe00;'">
                <xsl:text xml:space="preserve">\varsubsetneqq </xsl:text>
              </xsl:when> 
		    <xsl:otherwise>
              <xsl:text xml:space="preserve">\subsetneqq </xsl:text>
		    </xsl:otherwise>
		    </xsl:choose>
          </xsl:when>
	      <xsl:when test="$first-char='&#x2ACC;'">
	        <xsl:choose>
              <xsl:when test="$next-char='&#xfe00;'">
                <xsl:text xml:space="preserve">\varsupsetneqq </xsl:text>
              </xsl:when> 
		    <xsl:otherwise>
              <xsl:text xml:space="preserve">\supsetneqq </xsl:text>
		    </xsl:otherwise>
		    </xsl:choose>
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
            <xsl:text xml:space="preserve">\odiv </xsl:text>
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
<!-- Covered above, so don't need these tests that are in the private area (gp)
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
-->
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

<!-- Begin additions to support characters available when using the stix package -->
          <xsl:when test="$first-char='&#x1D55;'">
            <xsl:text xml:space="preserve">\mathbb{k}</xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x03DD;'">
            <xsl:text xml:space="preserve">\digamma </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25B6;'">
            <xsl:text xml:space="preserve">\blacktriangleright </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25C0;'">
            <xsl:text xml:space="preserve">\blacktriangleleft </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2661;'">
            <xsl:text xml:space="preserve">\heartsuit </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2662;'">
            <xsl:text xml:space="preserve">\diamondsuit </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A5E;'">
            <xsl:text xml:space="preserve">\doublebarwedge </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21DD;'">
            <xsl:text xml:space="preserve">\rightsquigarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x01B5;'">
            <xsl:text xml:space="preserve">\Zbar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2025;'">
            <xsl:text xml:space="preserve">\enleadertwodots </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2036;'">
            <xsl:text xml:space="preserve">\backdprime </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2037;'">
            <xsl:text xml:space="preserve">\backtrprime </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2038;'">
            <xsl:text xml:space="preserve">\caretinsert </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x203C;'">
            <xsl:text xml:space="preserve">\Exclam </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2043;'">
            <xsl:text xml:space="preserve">\hyphenbullet </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2047;'">
            <xsl:text xml:space="preserve">\Question </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2057;'">
            <xsl:text xml:space="preserve">\qprime </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x20DD;'">
            <xsl:text xml:space="preserve">\enclosecircle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x20DE;'">
            <xsl:text xml:space="preserve">\enclosesquare </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x20DF;'">
            <xsl:text xml:space="preserve">\enclosediamond </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x20E4;'">
            <xsl:text xml:space="preserve">\enclosetriangle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2107;'">
            <xsl:text xml:space="preserve">\Eulerconst </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2129;'">
            <xsl:text xml:space="preserve">\turnediota </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2142;'">
            <xsl:text xml:space="preserve">\sansLturned </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2143;'">
            <xsl:text xml:space="preserve">\sansLmirrored </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2144;'">
            <xsl:text xml:space="preserve">\Yup </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x214A;'">
            <xsl:text xml:space="preserve">\PropertyLine </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21A8;'">
            <xsl:text xml:space="preserve">\updownarrowbar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21B4;'">
            <xsl:text xml:space="preserve">\linefeed </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21B5;'">
            <xsl:text xml:space="preserve">\carriagereturn </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21B8;'">
            <xsl:text xml:space="preserve">\barovernorthwestarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21B9;'">
            <xsl:text xml:space="preserve">\barleftarrowrightarrowbar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21DE;'">
            <xsl:text xml:space="preserve">\nHuparrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21DF;'">
            <xsl:text xml:space="preserve">\nHdownarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21E0;'">
            <xsl:text xml:space="preserve">\leftdasharrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21E1;'">
            <xsl:text xml:space="preserve">\updasharrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21E2;'">
            <xsl:text xml:space="preserve">\rightdasharrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21E3;'">
            <xsl:text xml:space="preserve">\downdasharrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21E6;'">
            <xsl:text xml:space="preserve">\leftwhitearrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21E7;'">
            <xsl:text xml:space="preserve">\upwhitearrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21E8;'">
            <xsl:text xml:space="preserve">\rightwhitearrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21E9;'">
            <xsl:text xml:space="preserve">\downwhitearrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21EA;'">
            <xsl:text xml:space="preserve">\whitearrowupfrombar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2206;'">
            <xsl:text xml:space="preserve">\increment </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x220E;'">
            <xsl:text xml:space="preserve">\QED </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x223F;'">
            <xsl:text xml:space="preserve">\sinewave </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22B9;'">
            <xsl:text xml:space="preserve">\hermitmatrix </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22BE;'">
            <xsl:text xml:space="preserve">\measuredrightangle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22BF;'">
            <xsl:text xml:space="preserve">\varlrtriangle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2300;'">
            <xsl:text xml:space="preserve">\diameter </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2302;'">
            <xsl:text xml:space="preserve">\house </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2310;'">
            <xsl:text xml:space="preserve">\invnot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2311;'">
            <xsl:text xml:space="preserve">\sqlozenge </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2312;'">
            <xsl:text xml:space="preserve">\profline </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2313;'">
            <xsl:text xml:space="preserve">\profsurf </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2317;'">
            <xsl:text xml:space="preserve">\viewdata </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2319;'">
            <xsl:text xml:space="preserve">\turnednot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x232C;'">
            <xsl:text xml:space="preserve">\varhexagonlrbonds </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2332;'">
            <xsl:text xml:space="preserve">\conictaper </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2336;'">
            <xsl:text xml:space="preserve">\topbot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2340;'">
            <xsl:text xml:space="preserve">\APLnotbackslash </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2353;'">
            <xsl:text xml:space="preserve">\APLboxupcaret </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2370;'">
            <xsl:text xml:space="preserve">\APLboxquestion </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x237C;'">
            <xsl:text xml:space="preserve">\rangledownzigzagarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2394;'">
            <xsl:text xml:space="preserve">\hexagon </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x23B6;'">
            <xsl:text xml:space="preserve">\bbrktbrk </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x23CE;'">
            <xsl:text xml:space="preserve">\varcarriagereturn </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x23E0;'">
            <xsl:text xml:space="preserve">\obrbrak </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x23E1;'">
            <xsl:text xml:space="preserve">\ubrbrak </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x23E2;'">
            <xsl:text xml:space="preserve">\trapezium </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x23E3;'">
            <xsl:text xml:space="preserve">\benzenr </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x23E4;'">
            <xsl:text xml:space="preserve">\strns </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x23E5;'">
            <xsl:text xml:space="preserve">\fltns </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x23E6;'">
            <xsl:text xml:space="preserve">\accurrent </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x23E7;'">
            <xsl:text xml:space="preserve">\elinters </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x24C7;'">
            <xsl:text xml:space="preserve">\circledR </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25A0;'">
            <xsl:text xml:space="preserve">\mdlgblksquare </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25A2;'">
            <xsl:text xml:space="preserve">\squoval </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25A3;'">
            <xsl:text xml:space="preserve">\blackinwhitesquare </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25A4;'">
            <xsl:text xml:space="preserve">\squarehfill </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25A5;'">
            <xsl:text xml:space="preserve">\squarevfill </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25A6;'">
            <xsl:text xml:space="preserve">\squarehvfill </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25A7;'">
            <xsl:text xml:space="preserve">\squarenwsefill </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25A8;'">
            <xsl:text xml:space="preserve">\squareneswfill </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25A9;'">
            <xsl:text xml:space="preserve">\squarecrossfill </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25AB;'">
            <xsl:text xml:space="preserve">\smwhtsquare </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25AC;'">
            <xsl:text xml:space="preserve">\hrectangleblack </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25AD;'">
            <xsl:text xml:space="preserve">\hrectangle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25AE;'">
            <xsl:text xml:space="preserve">\vrectangleblack </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25AF;'">
            <xsl:text xml:space="preserve">\vrectangle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25B0;'">
            <xsl:text xml:space="preserve">\parallelogramblack </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25B1;'">
            <xsl:text xml:space="preserve">\parallelogram </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25B2;'">
            <xsl:text xml:space="preserve">\bigblacktriangleup </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25BA;'">
            <xsl:text xml:space="preserve">\blackpointerright </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25BB;'">
            <xsl:text xml:space="preserve">\whitepointerright </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25BC;'">
            <xsl:text xml:space="preserve">\bigblacktriangledown </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25C4;'">
            <xsl:text xml:space="preserve">\blackpointerleft </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25C5;'">
            <xsl:text xml:space="preserve">\whitepointerleft </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25C6;'">
            <xsl:text xml:space="preserve">\mdlgblkdiamond </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25C8;'">
            <xsl:text xml:space="preserve">\blackinwhitediamond </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25C9;'">
            <xsl:text xml:space="preserve">\fisheye </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25CC;'">
            <xsl:text xml:space="preserve">\dottedcircle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25CD;'">
            <xsl:text xml:space="preserve">\circlevertfill </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25CE;'">
            <xsl:text xml:space="preserve">\bullseye </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25CF;'">
            <xsl:text xml:space="preserve">\mdlgblkcircle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25D0;'">
            <xsl:text xml:space="preserve">\circlelefthalfblack </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25D1;'">
            <xsl:text xml:space="preserve">\circlerighthalfblack </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25D2;'">
            <xsl:text xml:space="preserve">\circlebottomhalfblack </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25D3;'">
            <xsl:text xml:space="preserve">\circletophalfblack </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25D4;'">
            <xsl:text xml:space="preserve">\circleurquadblack </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25D5;'">
            <xsl:text xml:space="preserve">\blackcircleulquadwhite </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25D6;'">
            <xsl:text xml:space="preserve">\blacklefthalfcircle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25D7;'">
            <xsl:text xml:space="preserve">\blackrighthalfcircle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25D8;'">
            <xsl:text xml:space="preserve">\inversebullet </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25D9;'">
            <xsl:text xml:space="preserve">\inversewhitecircle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25DA;'">
            <xsl:text xml:space="preserve">\invwhiteupperhalfcircle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25DB;'">
            <xsl:text xml:space="preserve">\invwhitelowerhalfcircle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25DC;'">
            <xsl:text xml:space="preserve">\ularc </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25DD;'">
            <xsl:text xml:space="preserve">\urarc </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25DE;'">
            <xsl:text xml:space="preserve">\lrarc </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25DF;'">
            <xsl:text xml:space="preserve">\llarc </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25E0;'">
            <xsl:text xml:space="preserve">\topsemicircle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25E1;'">
            <xsl:text xml:space="preserve">\botsemicircle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25E2;'">
            <xsl:text xml:space="preserve">\lrblacktriangle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25E3;'">
            <xsl:text xml:space="preserve">\llblacktriangle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25E4;'">
            <xsl:text xml:space="preserve">\ulblacktriangle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25E5;'">
            <xsl:text xml:space="preserve">\urblacktriangle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25E7;'">
            <xsl:text xml:space="preserve">\squareleftblack </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25E8;'">
            <xsl:text xml:space="preserve">\squarerightblack </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25E9;'">
            <xsl:text xml:space="preserve">\squareulblack </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25EA;'">
            <xsl:text xml:space="preserve">\squarelrblack </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25EC;'">
            <xsl:text xml:space="preserve">\trianglecdot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25ED;'">
            <xsl:text xml:space="preserve">\triangleleftblack </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25EE;'">
            <xsl:text xml:space="preserve">\trianglerightblack </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25F0;'">
            <xsl:text xml:space="preserve">\squareulquad </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25F1;'">
            <xsl:text xml:space="preserve">\squarellquad </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25F2;'">
            <xsl:text xml:space="preserve">\squarelrquad </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25F3;'">
            <xsl:text xml:space="preserve">\squareurquad </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25F4;'">
            <xsl:text xml:space="preserve">\circleulquad </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25F5;'">
            <xsl:text xml:space="preserve">\circlellquad </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25F6;'">
            <xsl:text xml:space="preserve">\circlelrquad </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25F7;'">
            <xsl:text xml:space="preserve">\circleurquad </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25F8;'">
            <xsl:text xml:space="preserve">\ultriangle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25F9;'">
            <xsl:text xml:space="preserve">\urtriangle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25FA;'">
            <xsl:text xml:space="preserve">\lltriangle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25FB;'">
            <xsl:text xml:space="preserve">\mdwhtsquare </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25FC;'">
            <xsl:text xml:space="preserve">\mdblksquare </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25FD;'">
            <xsl:text xml:space="preserve">\mdsmwhtsquare </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25FE;'">
            <xsl:text xml:space="preserve">\mdsmblksquare </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25FF;'">
            <xsl:text xml:space="preserve">\lrtriangle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2606;'">
            <xsl:text xml:space="preserve">\bigwhitestar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2609;'">
            <xsl:text xml:space="preserve">\astrosun </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2621;'">
            <xsl:text xml:space="preserve">\danger </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x263B;'">
            <xsl:text xml:space="preserve">\blacksmiley </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x263C;'">
            <xsl:text xml:space="preserve">\sun </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x263D;'">
            <xsl:text xml:space="preserve">\rightmoon </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x263E;'">
            <xsl:text xml:space="preserve">\leftmoon </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2640;'">
            <xsl:text xml:space="preserve">\female </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2642;'">
            <xsl:text xml:space="preserve">\male </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2664;'">
            <xsl:text xml:space="preserve">\varspadesuit </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2667;'">
            <xsl:text xml:space="preserve">\varclubsuit </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2669;'">
            <xsl:text xml:space="preserve">\quarternote </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x266B;'">
            <xsl:text xml:space="preserve">\twonotes </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x267E;'">
            <xsl:text xml:space="preserve">\acidfree </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2680;'">
            <xsl:text xml:space="preserve">\dicei </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2681;'">
            <xsl:text xml:space="preserve">\diceii </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2682;'">
            <xsl:text xml:space="preserve">\diceiii </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2683;'">
            <xsl:text xml:space="preserve">\diceiv </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2684;'">
            <xsl:text xml:space="preserve">\dicev </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2685;'">
            <xsl:text xml:space="preserve">\dicevi </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2686;'">
            <xsl:text xml:space="preserve">\circledrightdot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2687;'">
            <xsl:text xml:space="preserve">\circledtwodots </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2688;'">
            <xsl:text xml:space="preserve">\blackcircledrightdot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2689;'">
            <xsl:text xml:space="preserve">\blackcircledtwodots </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x26A5;'">
            <xsl:text xml:space="preserve">\Hermaphrodite </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x26AA;'">
            <xsl:text xml:space="preserve">\mdwhtcircle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x26AB;'">
            <xsl:text xml:space="preserve">\mdblkcircle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x26AC;'">
            <xsl:text xml:space="preserve">\mdsmwhtcircle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x26B2;'">
            <xsl:text xml:space="preserve">\neuter </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x272A;'">
            <xsl:text xml:space="preserve">\circledstar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2736;'">
            <xsl:text xml:space="preserve">\varstar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x273D;'">
            <xsl:text xml:space="preserve">\dingasterisk </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x279B;'">
            <xsl:text xml:space="preserve">\draftingarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27C0;'">
            <xsl:text xml:space="preserve">\threedangle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27C1;'">
            <xsl:text xml:space="preserve">\whiteinwhitetriangle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27C3;'">
            <xsl:text xml:space="preserve">\subsetcirc </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27C4;'">
            <xsl:text xml:space="preserve">\supsetcirc </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27D0;'">
            <xsl:text xml:space="preserve">\diamondcdot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x292B;'">
            <xsl:text xml:space="preserve">\rdiagovfdiag </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x292C;'">
            <xsl:text xml:space="preserve">\fdiagovrdiag </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x292D;'">
            <xsl:text xml:space="preserve">\seovnearrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x292E;'">
            <xsl:text xml:space="preserve">\neovsearrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x292F;'">
            <xsl:text xml:space="preserve">\fdiagovnearrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2930;'">
            <xsl:text xml:space="preserve">\rdiagovsearrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2931;'">
            <xsl:text xml:space="preserve">\neovnwarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2932;'">
            <xsl:text xml:space="preserve">\nwovnearrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2934;'">
            <xsl:text xml:space="preserve">\uprightcurvearrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2935;'">
            <xsl:text xml:space="preserve">\downrightcurvedarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2981;'">
            <xsl:text xml:space="preserve">\mdsmblkcircle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2999;'">
            <xsl:text xml:space="preserve">\fourvdots </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x299A;'">
            <xsl:text xml:space="preserve">\vzigzag </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x299B;'">
            <xsl:text xml:space="preserve">\measuredangleleft </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x299C;'">
            <xsl:text xml:space="preserve">\rightanglesqr </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x299D;'">
            <xsl:text xml:space="preserve">\rightanglemdot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x299E;'">
            <xsl:text xml:space="preserve">\angles </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x299F;'">
            <xsl:text xml:space="preserve">\angdnr </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29A0;'">
            <xsl:text xml:space="preserve">\gtlpar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29A1;'">
            <xsl:text xml:space="preserve">\sphericalangleup </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29A2;'">
            <xsl:text xml:space="preserve">\turnangle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29A3;'">
            <xsl:text xml:space="preserve">\revangle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29A4;'">
            <xsl:text xml:space="preserve">\angleubar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29A5;'">
            <xsl:text xml:space="preserve">\revangleubar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29A6;'">
            <xsl:text xml:space="preserve">\wideangledown </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29A7;'">
            <xsl:text xml:space="preserve">\wideangleup </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29A8;'">
            <xsl:text xml:space="preserve">\measanglerutone </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29A9;'">
            <xsl:text xml:space="preserve">\measanglelutonw </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29AA;'">
            <xsl:text xml:space="preserve">\measanglerdtose </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29AB;'">
            <xsl:text xml:space="preserve">\measangleldtosw </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29AC;'">
            <xsl:text xml:space="preserve">\measangleurtone </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29AD;'">
            <xsl:text xml:space="preserve">\measangleultonw </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29AE;'">
            <xsl:text xml:space="preserve">\measangledrtose </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29AF;'">
            <xsl:text xml:space="preserve">\measangledltosw </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29B0;'">
            <xsl:text xml:space="preserve">\revemptyset </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29B1;'">
            <xsl:text xml:space="preserve">\emptysetobar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29B2;'">
            <xsl:text xml:space="preserve">\emptysetocirc </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29B3;'">
            <xsl:text xml:space="preserve">\emptysetoarr </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29B4;'">
            <xsl:text xml:space="preserve">\emptysetoarrl </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29BA;'">
            <xsl:text xml:space="preserve">\obot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29BB;'">
            <xsl:text xml:space="preserve">\olcross </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29BC;'">
            <xsl:text xml:space="preserve">\odotslashdot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29BD;'">
            <xsl:text xml:space="preserve">\uparrowoncircle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29BE;'">
            <xsl:text xml:space="preserve">\circledwhitebullet </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29BF;'">
            <xsl:text xml:space="preserve">\circledbullet </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29C2;'">
            <xsl:text xml:space="preserve">\cirscir </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29C3;'">
            <xsl:text xml:space="preserve">\cirE </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29C9;'">
            <xsl:text xml:space="preserve">\boxonbox </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29CA;'">
            <xsl:text xml:space="preserve">\triangleodot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29CB;'">
            <xsl:text xml:space="preserve">\triangleubar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29CC;'">
            <xsl:text xml:space="preserve">\triangles </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29DC;'">
            <xsl:text xml:space="preserve">\iinfin </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29DD;'">
            <xsl:text xml:space="preserve">\tieinfty </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29DE;'">
            <xsl:text xml:space="preserve">\nvinfty </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29E0;'">
            <xsl:text xml:space="preserve">\laplac </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29E7;'">
            <xsl:text xml:space="preserve">\thermod </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29E8;'">
            <xsl:text xml:space="preserve">\downtriangleleftblack </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29E9;'">
            <xsl:text xml:space="preserve">\downtrianglerightblack </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29EA;'">
            <xsl:text xml:space="preserve">\blackdiamonddownarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29EC;'">
            <xsl:text xml:space="preserve">\circledownarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29ED;'">
            <xsl:text xml:space="preserve">\blackcircledownarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29EE;'">
            <xsl:text xml:space="preserve">\errbarsquare </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29EF;'">
            <xsl:text xml:space="preserve">\errbarblacksquare </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29F0;'">
            <xsl:text xml:space="preserve">\errbardiamond </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29F1;'">
            <xsl:text xml:space="preserve">\errbarblackdiamond </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29F2;'">
            <xsl:text xml:space="preserve">\errbarcircle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29F3;'">
            <xsl:text xml:space="preserve">\errbarblackcircle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AE1;'">
            <xsl:text xml:space="preserve">\perps </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AF1;'">
            <xsl:text xml:space="preserve">\topcir </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B12;'">
            <xsl:text xml:space="preserve">\squaretopblack </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B13;'">
            <xsl:text xml:space="preserve">\squarebotblack </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B14;'">
            <xsl:text xml:space="preserve">\squareurblack </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B15;'">
            <xsl:text xml:space="preserve">\squarellblack </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B16;'">
            <xsl:text xml:space="preserve">\diamondleftblack </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B17;'">
            <xsl:text xml:space="preserve">\diamondrightblack </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B18;'">
            <xsl:text xml:space="preserve">\diamondtopblack </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B19;'">
            <xsl:text xml:space="preserve">\diamondbotblack </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B1A;'">
            <xsl:text xml:space="preserve">\dottedsquare </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B1B;'">
            <xsl:text xml:space="preserve">\lgblksquare </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B1C;'">
            <xsl:text xml:space="preserve">\lgwhtsquare </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B1D;'">
            <xsl:text xml:space="preserve">\vysmblksquare </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B1E;'">
            <xsl:text xml:space="preserve">\vysmwhtsquare </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B1F;'">
            <xsl:text xml:space="preserve">\pentagonblack </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B20;'">
            <xsl:text xml:space="preserve">\pentagon </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B21;'">
            <xsl:text xml:space="preserve">\varhexagon </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B22;'">
            <xsl:text xml:space="preserve">\varhexagonblack </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B23;'">
            <xsl:text xml:space="preserve">\hexagonblack </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B24;'">
            <xsl:text xml:space="preserve">\lgblkcircle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B25;'">
            <xsl:text xml:space="preserve">\mdblkdiamond </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B26;'">
            <xsl:text xml:space="preserve">\mdwhtdiamond </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B27;'">
            <xsl:text xml:space="preserve">\mdblklozenge </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B28;'">
            <xsl:text xml:space="preserve">\mdwhtlozenge </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B29;'">
            <xsl:text xml:space="preserve">\smblkdiamond </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B2A;'">
            <xsl:text xml:space="preserve">\smblklozenge </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B2B;'">
            <xsl:text xml:space="preserve">\smwhtlozenge </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B2C;'">
            <xsl:text xml:space="preserve">\blkhorzoval </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B2D;'">
            <xsl:text xml:space="preserve">\whthorzoval </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B2E;'">
            <xsl:text xml:space="preserve">\blkvertoval </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B2F;'">
            <xsl:text xml:space="preserve">\whtvertoval </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B50;'">
            <xsl:text xml:space="preserve">\medwhitestar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B51;'">
            <xsl:text xml:space="preserve">\medblackstar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B52;'">
            <xsl:text xml:space="preserve">\smwhitestar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B53;'">
            <xsl:text xml:space="preserve">\rightpentagonblack </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B54;'">
            <xsl:text xml:space="preserve">\rightpentagon </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x3012;'">
            <xsl:text xml:space="preserve">\postalmark </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x3030;'">
            <xsl:text xml:space="preserve">\hzigzag </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x214B;'">
            <xsl:text xml:space="preserve">\upand </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2238;'">
            <xsl:text xml:space="preserve">\dotminus </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x223E;'">
            <xsl:text xml:space="preserve">\invlazys </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x228C;'">
            <xsl:text xml:space="preserve">\cupleftarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x228D;'">
            <xsl:text xml:space="preserve">\cupdot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x229C;'">
            <xsl:text xml:space="preserve">\circledequal </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22BD;'">
            <xsl:text xml:space="preserve">\barvee </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2305;'">
            <xsl:text xml:space="preserve">\varbarwedge </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x233D;'">
            <xsl:text xml:space="preserve">\obar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x25EB;'">
            <xsl:text xml:space="preserve">\boxbar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27C7;'">
            <xsl:text xml:space="preserve">\veedot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27D1;'">
            <xsl:text xml:space="preserve">\wedgedot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27E0;'">
            <xsl:text xml:space="preserve">\lozengeminus </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27E1;'">
            <xsl:text xml:space="preserve">\concavediamond </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27E2;'">
            <xsl:text xml:space="preserve">\concavediamondtickleft </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27E3;'">
            <xsl:text xml:space="preserve">\concavediamondtickright </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27E4;'">
            <xsl:text xml:space="preserve">\whitesquaretickleft </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27E5;'">
            <xsl:text xml:space="preserve">\whitesquaretickright </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2982;'">
            <xsl:text xml:space="preserve">\typecolon </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29B5;'">
            <xsl:text xml:space="preserve">\circlehbar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29B6;'">
            <xsl:text xml:space="preserve">\circledvert </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29B7;'">
            <xsl:text xml:space="preserve">\circledparallel </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29B8;'">
            <xsl:text xml:space="preserve">\obslash </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29B9;'">
            <xsl:text xml:space="preserve">\operp </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29C0;'">
            <xsl:text xml:space="preserve">\olessthan </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29C1;'">
            <xsl:text xml:space="preserve">\ogreaterthan </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29C4;'">
            <xsl:text xml:space="preserve">\boxdiag </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29C5;'">
            <xsl:text xml:space="preserve">\boxbslash </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29C6;'">
            <xsl:text xml:space="preserve">\boxast </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29C7;'">
            <xsl:text xml:space="preserve">\boxcircle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29C8;'">
            <xsl:text xml:space="preserve">\boxbox </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29CD;'">
            <xsl:text xml:space="preserve">\triangleserifs </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29D6;'">
            <xsl:text xml:space="preserve">\hourglass </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29D7;'">
            <xsl:text xml:space="preserve">\blackhourglass </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29E2;'">
            <xsl:text xml:space="preserve">\shuffle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29F6;'">
            <xsl:text xml:space="preserve">\dsol </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29F7;'">
            <xsl:text xml:space="preserve">\rsolbar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29FA;'">
            <xsl:text xml:space="preserve">\doubleplus </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29FB;'">
            <xsl:text xml:space="preserve">\tripleplus </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29FE;'">
            <xsl:text xml:space="preserve">\tplus </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29FF;'">
            <xsl:text xml:space="preserve">\tminus </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A22;'">
            <xsl:text xml:space="preserve">\ringplus </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A23;'">
            <xsl:text xml:space="preserve">\plushat </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A24;'">
            <xsl:text xml:space="preserve">\simplus </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A25;'">
            <xsl:text xml:space="preserve">\plusdot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A26;'">
            <xsl:text xml:space="preserve">\plussim </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A27;'">
            <xsl:text xml:space="preserve">\plussubtwo </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A28;'">
            <xsl:text xml:space="preserve">\plustrif </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A29;'">
            <xsl:text xml:space="preserve">\commaminus </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A2A;'">
            <xsl:text xml:space="preserve">\minusdot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A2B;'">
            <xsl:text xml:space="preserve">\minusfdots </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A2C;'">
            <xsl:text xml:space="preserve">\minusrdots </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A2D;'">
            <xsl:text xml:space="preserve">\opluslhrim </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A2E;'">
            <xsl:text xml:space="preserve">\oplusrhrim </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A2F;'">
            <xsl:text xml:space="preserve">\vectimes </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A30;'">
            <xsl:text xml:space="preserve">\dottimes </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A31;'">
            <xsl:text xml:space="preserve">\timesbar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A32;'">
            <xsl:text xml:space="preserve">\btimes </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A33;'">
            <xsl:text xml:space="preserve">\smashtimes </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A34;'">
            <xsl:text xml:space="preserve">\otimeslhrim </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A35;'">
            <xsl:text xml:space="preserve">\otimesrhrim </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A36;'">
            <xsl:text xml:space="preserve">\otimeshat </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A37;'">
            <xsl:text xml:space="preserve">\Otimes </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A39;'">
            <xsl:text xml:space="preserve">\triangleplus </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A3A;'">
            <xsl:text xml:space="preserve">\triangleminus </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A3B;'">
            <xsl:text xml:space="preserve">\triangletimes </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A3C;'">
            <xsl:text xml:space="preserve">\intprod </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A3D;'">
            <xsl:text xml:space="preserve">\intprodr </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A3E;'">
            <xsl:text xml:space="preserve">\fcmp </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A40;'">
            <xsl:text xml:space="preserve">\capdot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A41;'">
            <xsl:text xml:space="preserve">\uminus </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A42;'">
            <xsl:text xml:space="preserve">\barcup </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A43;'">
            <xsl:text xml:space="preserve">\barcap </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A44;'">
            <xsl:text xml:space="preserve">\capwedge </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A45;'">
            <xsl:text xml:space="preserve">\cupvee </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A46;'">
            <xsl:text xml:space="preserve">\cupovercap </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A47;'">
            <xsl:text xml:space="preserve">\capovercup </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A48;'">
            <xsl:text xml:space="preserve">\cupbarcap </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A49;'">
            <xsl:text xml:space="preserve">\capbarcup </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A4A;'">
            <xsl:text xml:space="preserve">\twocups </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A4B;'">
            <xsl:text xml:space="preserve">\twocaps </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A4C;'">
            <xsl:text xml:space="preserve">\closedvarcup </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A4D;'">
            <xsl:text xml:space="preserve">\closedvarcap </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A4E;'">
            <xsl:text xml:space="preserve">\Sqcap </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A4F;'">
            <xsl:text xml:space="preserve">\Sqcup </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A50;'">
            <xsl:text xml:space="preserve">\closedvarcupsmashprod </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A51;'">
            <xsl:text xml:space="preserve">\wedgeodot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A52;'">
            <xsl:text xml:space="preserve">\veeodot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A53;'">
            <xsl:text xml:space="preserve">\Wedge </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A54;'">
            <xsl:text xml:space="preserve">\Vee </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A55;'">
            <xsl:text xml:space="preserve">\wedgeonwedge </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A56;'">
            <xsl:text xml:space="preserve">\veeonvee </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A57;'">
            <xsl:text xml:space="preserve">\bigslopedvee </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A58;'">
            <xsl:text xml:space="preserve">\bigslopedwedge </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A5A;'">
            <xsl:text xml:space="preserve">\wedgemidvert </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A5B;'">
            <xsl:text xml:space="preserve">\veemidvert </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A5C;'">
            <xsl:text xml:space="preserve">\midbarwedge </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A5D;'">
            <xsl:text xml:space="preserve">\midbarvee </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A5F;'">
            <xsl:text xml:space="preserve">\wedgebar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A60;'">
            <xsl:text xml:space="preserve">\wedgedoublebar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A61;'">
            <xsl:text xml:space="preserve">\varveebar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A62;'">
            <xsl:text xml:space="preserve">\doublebarvee </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A63;'">
            <xsl:text xml:space="preserve">\veedoublebar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A64;'">
            <xsl:text xml:space="preserve">\dsub </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A65;'">
            <xsl:text xml:space="preserve">\rsub </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A71;'">
            <xsl:text xml:space="preserve">\eqqplus </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A72;'">
            <xsl:text xml:space="preserve">\pluseqq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AF4;'">
            <xsl:text xml:space="preserve">\interleave </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AF5;'">
            <xsl:text xml:space="preserve">\nhVvert </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AF6;'">
            <xsl:text xml:space="preserve">\threedotcolon </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AFB;'">
            <xsl:text xml:space="preserve">\trslash </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AFD;'">
            <xsl:text xml:space="preserve">\sslash </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AFE;'">
            <xsl:text xml:space="preserve">\talloblong </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2050;'">
            <xsl:text xml:space="preserve">\closure </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x20D2;'">
            <xsl:text xml:space="preserve">\vertoverlay </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x219C;'">
            <xsl:text xml:space="preserve">\leftwavearrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x219F;'">
            <xsl:text xml:space="preserve">\twoheaduparrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21A1;'">
            <xsl:text xml:space="preserve">\twoheaddownarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21A4;'">
            <xsl:text xml:space="preserve">\mapsfrom </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21A5;'">
            <xsl:text xml:space="preserve">\mapsup </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21A7;'">
            <xsl:text xml:space="preserve">\mapsdown </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21AF;'">
            <xsl:text xml:space="preserve">\downzigzagarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21B2;'">
            <xsl:text xml:space="preserve">\Ldsh </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21B3;'">
            <xsl:text xml:space="preserve">\Rdsh </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21C5;'">
            <xsl:text xml:space="preserve">\updownarrows </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21D6;'">
            <xsl:text xml:space="preserve">\Nwarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21D7;'">
            <xsl:text xml:space="preserve">\Nearrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21D8;'">
            <xsl:text xml:space="preserve">\Searrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21D9;'">
            <xsl:text xml:space="preserve">\Swarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21DC;'">
            <xsl:text xml:space="preserve">\leftsquigarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21E4;'">
            <xsl:text xml:space="preserve">\barleftarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21E5;'">
            <xsl:text xml:space="preserve">\rightarrowbar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21F4;'">
            <xsl:text xml:space="preserve">\circleonrightarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21F5;'">
            <xsl:text xml:space="preserve">\downuparrows </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21F6;'">
            <xsl:text xml:space="preserve">\rightthreearrows </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21F7;'">
            <xsl:text xml:space="preserve">\nvleftarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21F8;'">
            <xsl:text xml:space="preserve">\nvrightarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21F9;'">
            <xsl:text xml:space="preserve">\nvleftrightarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21FA;'">
            <xsl:text xml:space="preserve">\nVleftarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21FB;'">
            <xsl:text xml:space="preserve">\nVrightarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21FC;'">
            <xsl:text xml:space="preserve">\nVleftrightarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21FD;'">
            <xsl:text xml:space="preserve">\leftarrowtriangle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21FE;'">
            <xsl:text xml:space="preserve">\rightarrowtriangle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x21FF;'">
            <xsl:text xml:space="preserve">\leftrightarrowtriangle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x220C;'">
            <xsl:text xml:space="preserve">\nni </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x220D;'">
            <xsl:text xml:space="preserve">\smallni </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2239;'">
            <xsl:text xml:space="preserve">\dashcolon </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x223A;'">
            <xsl:text xml:space="preserve">\dotsminusdots </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x223B;'">
            <xsl:text xml:space="preserve">\kernelcontraction </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2244;'">
            <xsl:text xml:space="preserve">\nsime </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2246;'">
            <xsl:text xml:space="preserve">\simneqq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2249;'">
            <xsl:text xml:space="preserve">\napprox </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x224B;'">
            <xsl:text xml:space="preserve">\approxident </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x224C;'">
            <xsl:text xml:space="preserve">\backcong </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2254;'">
            <xsl:text xml:space="preserve">\coloneq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2255;'">
            <xsl:text xml:space="preserve">\eqcolon </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2258;'">
            <xsl:text xml:space="preserve">\arceq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2259;'">
            <xsl:text xml:space="preserve">\wedgeq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x225A;'">
            <xsl:text xml:space="preserve">\veeeq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x225B;'">
            <xsl:text xml:space="preserve">\stareq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x225D;'">
            <xsl:text xml:space="preserve">\eqdef </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x225E;'">
            <xsl:text xml:space="preserve">\measeq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2263;'">
            <xsl:text xml:space="preserve">\Equiv </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x226D;'">
            <xsl:text xml:space="preserve">\nasymp </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2274;'">
            <xsl:text xml:space="preserve">\nlesssim </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2275;'">
            <xsl:text xml:space="preserve">\ngtrsim </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2278;'">
            <xsl:text xml:space="preserve">\nlessgtr </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2279;'">
            <xsl:text xml:space="preserve">\ngtrless </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22A6;'">
            <xsl:text xml:space="preserve">\assert </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22AB;'">
            <xsl:text xml:space="preserve">\VDash </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22B0;'">
            <xsl:text xml:space="preserve">\prurel </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22B1;'">
            <xsl:text xml:space="preserve">\scurel </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22B6;'">
            <xsl:text xml:space="preserve">\origof </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22B7;'">
            <xsl:text xml:space="preserve">\imageof </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22D5;'">
            <xsl:text xml:space="preserve">\equalparallel </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22DC;'">
            <xsl:text xml:space="preserve">\eqless </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22DD;'">
            <xsl:text xml:space="preserve">\eqgtr </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22E0;'">
            <xsl:text xml:space="preserve">\npreccurlyeq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22E1;'">
            <xsl:text xml:space="preserve">\nsucccurlyeq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22E2;'">
            <xsl:text xml:space="preserve">\nsqsubseteq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22E3;'">
            <xsl:text xml:space="preserve">\nsqsupseteq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22E4;'">
            <xsl:text xml:space="preserve">\sqsubsetneq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22E5;'">
            <xsl:text xml:space="preserve">\sqsupsetneq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22F0;'">
            <xsl:text xml:space="preserve">\adots </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22F2;'">
            <xsl:text xml:space="preserve">\disin </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22F3;'">
            <xsl:text xml:space="preserve">\varisins </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22F4;'">
            <xsl:text xml:space="preserve">\isins </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22F5;'">
            <xsl:text xml:space="preserve">\isindot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22F6;'">
            <xsl:text xml:space="preserve">\varisinobar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22F7;'">
            <xsl:text xml:space="preserve">\isinobar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22F8;'">
            <xsl:text xml:space="preserve">\isinvb </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22F9;'">
            <xsl:text xml:space="preserve">\isinE </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22FA;'">
            <xsl:text xml:space="preserve">\nisd </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22FB;'">
            <xsl:text xml:space="preserve">\varnis </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22FC;'">
            <xsl:text xml:space="preserve">\nis </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22FD;'">
            <xsl:text xml:space="preserve">\varniobar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22FE;'">
            <xsl:text xml:space="preserve">\niobar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x22FF;'">
            <xsl:text xml:space="preserve">\bagmember </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x233F;'">
            <xsl:text xml:space="preserve">\APLnotslash </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27C8;'">
            <xsl:text xml:space="preserve">\bsolhsub </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27C9;'">
            <xsl:text xml:space="preserve">\suphsol </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27D2;'">
            <xsl:text xml:space="preserve">\upin </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27D3;'">
            <xsl:text xml:space="preserve">\pullback </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27D4;'">
            <xsl:text xml:space="preserve">\pushout </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27DA;'">
            <xsl:text xml:space="preserve">\DashVDash </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27DB;'">
            <xsl:text xml:space="preserve">\dashVdash </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27DC;'">
            <xsl:text xml:space="preserve">\multimapinv </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27DD;'">
            <xsl:text xml:space="preserve">\vlongdash </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27DE;'">
            <xsl:text xml:space="preserve">\longdashv </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27DF;'">
            <xsl:text xml:space="preserve">\cirbot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27F0;'">
            <xsl:text xml:space="preserve">\UUparrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27F1;'">
            <xsl:text xml:space="preserve">\DDownarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27F2;'">
            <xsl:text xml:space="preserve">\acwgapcirclearrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27F3;'">
            <xsl:text xml:space="preserve">\cwgapcirclearrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27F4;'">
            <xsl:text xml:space="preserve">\rightarrowonoplus </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27FB;'">
            <xsl:text xml:space="preserve">\longmapsfrom </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27FD;'">
            <xsl:text xml:space="preserve">\Longmapsfrom </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27FE;'">
            <xsl:text xml:space="preserve">\Longmapsto </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27FF;'">
            <xsl:text xml:space="preserve">\longrightsquigarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2900;'">
            <xsl:text xml:space="preserve">\nvtwoheadrightarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2901;'">
            <xsl:text xml:space="preserve">\nVtwoheadrightarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2902;'">
            <xsl:text xml:space="preserve">\nvLeftarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2903;'">
            <xsl:text xml:space="preserve">\nvRightarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2904;'">
            <xsl:text xml:space="preserve">\nvLeftrightarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2905;'">
            <xsl:text xml:space="preserve">\twoheadmapsto </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2906;'">
            <xsl:text xml:space="preserve">\Mapsfrom </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2907;'">
            <xsl:text xml:space="preserve">\Mapsto </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2908;'">
            <xsl:text xml:space="preserve">\downarrowbarred </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2909;'">
            <xsl:text xml:space="preserve">\uparrowbarred </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x290A;'">
            <xsl:text xml:space="preserve">\Uuparrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x290B;'">
            <xsl:text xml:space="preserve">\Ddownarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x290C;'">
            <xsl:text xml:space="preserve">\leftbkarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x290D;'">
            <xsl:text xml:space="preserve">\rightbkarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2910;'">
            <xsl:text xml:space="preserve">\drbkarow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2911;'">
            <xsl:text xml:space="preserve">\rightdotarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2912;'">
            <xsl:text xml:space="preserve">\baruparrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2913;'">
            <xsl:text xml:space="preserve">\downarrowbar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2914;'">
            <xsl:text xml:space="preserve">\nvrightarrowtail </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2915;'">
            <xsl:text xml:space="preserve">\nVrightarrowtail </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2916;'">
            <xsl:text xml:space="preserve">\twoheadrightarrowtail </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2917;'">
            <xsl:text xml:space="preserve">\nvtwoheadrightarrowtail </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2918;'">
            <xsl:text xml:space="preserve">\nVtwoheadrightarrowtail </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2919;'">
            <xsl:text xml:space="preserve">\lefttail </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x291A;'">
            <xsl:text xml:space="preserve">\righttail </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x291B;'">
            <xsl:text xml:space="preserve">\leftdbltail </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x291C;'">
            <xsl:text xml:space="preserve">\rightdbltail </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x291D;'">
            <xsl:text xml:space="preserve">\diamondleftarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x291E;'">
            <xsl:text xml:space="preserve">\rightarrowdiamond </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x291F;'">
            <xsl:text xml:space="preserve">\diamondleftarrowbar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2920;'">
            <xsl:text xml:space="preserve">\barrightarrowdiamond </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2921;'">
            <xsl:text xml:space="preserve">\nwsearrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2922;'">
            <xsl:text xml:space="preserve">\neswarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2923;'">
            <xsl:text xml:space="preserve">\hknwarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2924;'">
            <xsl:text xml:space="preserve">\hknearrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2925;'">
            <xsl:text xml:space="preserve">\hksearow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2926;'">
            <xsl:text xml:space="preserve">\hkswarow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2927;'">
            <xsl:text xml:space="preserve">\tona </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2928;'">
            <xsl:text xml:space="preserve">\toea </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2929;'">
            <xsl:text xml:space="preserve">\tosa </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x292A;'">
            <xsl:text xml:space="preserve">\towa </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2936;'">
            <xsl:text xml:space="preserve">\leftdowncurvedarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2937;'">
            <xsl:text xml:space="preserve">\rightdowncurvedarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2938;'">
            <xsl:text xml:space="preserve">\cwrightarcarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2939;'">
            <xsl:text xml:space="preserve">\acwleftarcarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x293A;'">
            <xsl:text xml:space="preserve">\acwoverarcarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x293B;'">
            <xsl:text xml:space="preserve">\acwunderarcarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x293C;'">
            <xsl:text xml:space="preserve">\curvearrowrightminus </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x293D;'">
            <xsl:text xml:space="preserve">\curvearrowleftplus </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x293E;'">
            <xsl:text xml:space="preserve">\cwundercurvearrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x293F;'">
            <xsl:text xml:space="preserve">\ccwundercurvearrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2940;'">
            <xsl:text xml:space="preserve">\acwcirclearrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2941;'">
            <xsl:text xml:space="preserve">\cwcirclearrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2942;'">
            <xsl:text xml:space="preserve">\rightarrowshortleftarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2943;'">
            <xsl:text xml:space="preserve">\leftarrowshortrightarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2944;'">
            <xsl:text xml:space="preserve">\shortrightarrowleftarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2945;'">
            <xsl:text xml:space="preserve">\rightarrowplus </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2946;'">
            <xsl:text xml:space="preserve">\leftarrowplus </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2947;'">
            <xsl:text xml:space="preserve">\rightarrowx </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2948;'">
            <xsl:text xml:space="preserve">\leftrightarrowcircle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2949;'">
            <xsl:text xml:space="preserve">\twoheaduparrowcircle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x294A;'">
            <xsl:text xml:space="preserve">\leftrightharpoonupdown </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x294B;'">
            <xsl:text xml:space="preserve">\leftrightharpoondownup </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x294C;'">
            <xsl:text xml:space="preserve">\updownharpoonrightleft </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x294D;'">
            <xsl:text xml:space="preserve">\updownharpoonleftright </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x294E;'">
            <xsl:text xml:space="preserve">\leftrightharpoonupup </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x294F;'">
            <xsl:text xml:space="preserve">\updownharpoonrightright </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2950;'">
            <xsl:text xml:space="preserve">\leftrightharpoondowndown </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2951;'">
            <xsl:text xml:space="preserve">\updownharpoonleftleft </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2952;'">
            <xsl:text xml:space="preserve">\barleftharpoonup </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2953;'">
            <xsl:text xml:space="preserve">\rightharpoonupbar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2954;'">
            <xsl:text xml:space="preserve">\barupharpoonright </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2955;'">
            <xsl:text xml:space="preserve">\downharpoonrightbar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2956;'">
            <xsl:text xml:space="preserve">\barleftharpoondown </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2957;'">
            <xsl:text xml:space="preserve">\rightharpoondownbar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2958;'">
            <xsl:text xml:space="preserve">\barupharpoonleft </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2959;'">
            <xsl:text xml:space="preserve">\downharpoonleftbar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x295A;'">
            <xsl:text xml:space="preserve">\leftharpoonupbar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x295B;'">
            <xsl:text xml:space="preserve">\barrightharpoonup </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x295C;'">
            <xsl:text xml:space="preserve">\upharpoonrightbar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x295D;'">
            <xsl:text xml:space="preserve">\bardownharpoonright </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x295E;'">
            <xsl:text xml:space="preserve">\leftharpoondownbar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x295F;'">
            <xsl:text xml:space="preserve">\barrightharpoondown </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2960;'">
            <xsl:text xml:space="preserve">\upharpoonleftbar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2961;'">
            <xsl:text xml:space="preserve">\bardownharpoonleft </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2962;'">
            <xsl:text xml:space="preserve">\leftharpoonsupdown </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2963;'">
            <xsl:text xml:space="preserve">\upharpoonsleftright </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2964;'">
            <xsl:text xml:space="preserve">\rightharpoonsupdown </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2965;'">
            <xsl:text xml:space="preserve">\downharpoonsleftright </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2966;'">
            <xsl:text xml:space="preserve">\leftrightharpoonsup </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2967;'">
            <xsl:text xml:space="preserve">\leftrightharpoonsdown </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2968;'">
            <xsl:text xml:space="preserve">\rightleftharpoonsup </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2969;'">
            <xsl:text xml:space="preserve">\rightleftharpoonsdown </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x296A;'">
            <xsl:text xml:space="preserve">\leftharpoonupdash </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x296B;'">
            <xsl:text xml:space="preserve">\dashleftharpoondown </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x296C;'">
            <xsl:text xml:space="preserve">\rightharpoonupdash </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x296D;'">
            <xsl:text xml:space="preserve">\dashrightharpoondown </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x296E;'">
            <xsl:text xml:space="preserve">\updownharpoonsleftright </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x296F;'">
            <xsl:text xml:space="preserve">\downupharpoonsleftright </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2970;'">
            <xsl:text xml:space="preserve">\rightimply </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2971;'">
            <xsl:text xml:space="preserve">\equalrightarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2972;'">
            <xsl:text xml:space="preserve">\similarrightarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2973;'">
            <xsl:text xml:space="preserve">\leftarrowsimilar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2974;'">
            <xsl:text xml:space="preserve">\rightarrowsimilar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2975;'">
            <xsl:text xml:space="preserve">\rightarrowapprox </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2976;'">
            <xsl:text xml:space="preserve">\ltlarr </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2977;'">
            <xsl:text xml:space="preserve">\leftarrowless </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2978;'">
            <xsl:text xml:space="preserve">\gtrarr </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2979;'">
            <xsl:text xml:space="preserve">\subrarr </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x297A;'">
            <xsl:text xml:space="preserve">\leftarrowsubset </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x297B;'">
            <xsl:text xml:space="preserve">\suplarr </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x297C;'">
            <xsl:text xml:space="preserve">\leftfishtail </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x297D;'">
            <xsl:text xml:space="preserve">\rightfishtail </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x297E;'">
            <xsl:text xml:space="preserve">\upfishtail </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x297F;'">
            <xsl:text xml:space="preserve">\downfishtail </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29CE;'">
            <xsl:text xml:space="preserve">\rtriltri </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29CF;'">
            <xsl:text xml:space="preserve">\ltrivb </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29D0;'">
            <xsl:text xml:space="preserve">\vbrtri </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29D1;'">
            <xsl:text xml:space="preserve">\lfbowtie </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29D2;'">
            <xsl:text xml:space="preserve">\rfbowtie </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29D3;'">
            <xsl:text xml:space="preserve">\fbowtie </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29D4;'">
            <xsl:text xml:space="preserve">\lftimes </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29D5;'">
            <xsl:text xml:space="preserve">\rftimes </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29DF;'">
            <xsl:text xml:space="preserve">\dualmap </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29E1;'">
            <xsl:text xml:space="preserve">\lrtriangleeq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29E3;'">
            <xsl:text xml:space="preserve">\eparsl </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29E4;'">
            <xsl:text xml:space="preserve">\smeparsl </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29E5;'">
            <xsl:text xml:space="preserve">\eqvparsl </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29E6;'">
            <xsl:text xml:space="preserve">\gleichstark </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29F4;'">
            <xsl:text xml:space="preserve">\ruledelayed </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A59;'">
            <xsl:text xml:space="preserve">\veeonwedge </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A66;'">
            <xsl:text xml:space="preserve">\eqdot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A67;'">
            <xsl:text xml:space="preserve">\dotequiv </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A68;'">
            <xsl:text xml:space="preserve">\equivVert </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A69;'">
            <xsl:text xml:space="preserve">\equivVvert </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A6A;'">
            <xsl:text xml:space="preserve">\dotsim </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A6B;'">
            <xsl:text xml:space="preserve">\simrdots </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A6C;'">
            <xsl:text xml:space="preserve">\simminussim </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A6D;'">
            <xsl:text xml:space="preserve">\congdot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A6E;'">
            <xsl:text xml:space="preserve">\asteq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A6F;'">
            <xsl:text xml:space="preserve">\hatapprox </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A70;'">
            <xsl:text xml:space="preserve">\approxeqq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A73;'">
            <xsl:text xml:space="preserve">\eqqsim </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A74;'">
            <xsl:text xml:space="preserve">\Coloneq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A75;'">
            <xsl:text xml:space="preserve">\eqeq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A76;'">
            <xsl:text xml:space="preserve">\eqeqeq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A77;'">
            <xsl:text xml:space="preserve">\ddotseq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A78;'">
            <xsl:text xml:space="preserve">\equivDD </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A79;'">
            <xsl:text xml:space="preserve">\ltcir </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A7A;'">
            <xsl:text xml:space="preserve">\gtcir </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A7B;'">
            <xsl:text xml:space="preserve">\ltquest </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A7C;'">
            <xsl:text xml:space="preserve">\gtquest </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A7F;'">
            <xsl:text xml:space="preserve">\lesdot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A80;'">
            <xsl:text xml:space="preserve">\gesdot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A81;'">
            <xsl:text xml:space="preserve">\lesdoto </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A82;'">
            <xsl:text xml:space="preserve">\gesdoto </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A83;'">
            <xsl:text xml:space="preserve">\lesdotor </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A84;'">
            <xsl:text xml:space="preserve">\gesdotol </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A8D;'">
            <xsl:text xml:space="preserve">\lsime </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A8E;'">
            <xsl:text xml:space="preserve">\gsime </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A8F;'">
            <xsl:text xml:space="preserve">\lsimg </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A90;'">
            <xsl:text xml:space="preserve">\gsiml </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A91;'">
            <xsl:text xml:space="preserve">\lgE </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A92;'">
            <xsl:text xml:space="preserve">\glE </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A93;'">
            <xsl:text xml:space="preserve">\lesges </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A94;'">
            <xsl:text xml:space="preserve">\gesles </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A97;'">
            <xsl:text xml:space="preserve">\elsdot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A98;'">
            <xsl:text xml:space="preserve">\egsdot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A99;'">
            <xsl:text xml:space="preserve">\eqqless </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A9A;'">
            <xsl:text xml:space="preserve">\eqqgtr </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A9B;'">
            <xsl:text xml:space="preserve">\eqqslantless </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A9C;'">
            <xsl:text xml:space="preserve">\eqqslantgtr </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A9D;'">
            <xsl:text xml:space="preserve">\simless </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A9E;'">
            <xsl:text xml:space="preserve">\simgtr </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A9F;'">
            <xsl:text xml:space="preserve">\simlE </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AA0;'">
            <xsl:text xml:space="preserve">\simgE </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AA1;'">
            <xsl:text xml:space="preserve">\Lt </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AA2;'">
            <xsl:text xml:space="preserve">\Gt </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AA3;'">
            <xsl:text xml:space="preserve">\partialmeetcontraction </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AA4;'">
            <xsl:text xml:space="preserve">\glj </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AA5;'">
            <xsl:text xml:space="preserve">\gla </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AA6;'">
            <xsl:text xml:space="preserve">\ltcc </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AA7;'">
            <xsl:text xml:space="preserve">\gtcc </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AA8;'">
            <xsl:text xml:space="preserve">\lescc </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AA9;'">
            <xsl:text xml:space="preserve">\gescc </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AAA;'">
            <xsl:text xml:space="preserve">\smt </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AAB;'">
            <xsl:text xml:space="preserve">\lat </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AAC;'">
            <xsl:text xml:space="preserve">\smte </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AAD;'">
            <xsl:text xml:space="preserve">\late </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AAE;'">
            <xsl:text xml:space="preserve">\bumpeqq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AB1;'">
            <xsl:text xml:space="preserve">\precneq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AB2;'">
            <xsl:text xml:space="preserve">\succneq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AB3;'">
            <xsl:text xml:space="preserve">\preceqq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AB4;'">
            <xsl:text xml:space="preserve">\succeqq </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2ABB;'">
            <xsl:text xml:space="preserve">\Prec </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2ABC;'">
            <xsl:text xml:space="preserve">\Succ </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2ABD;'">
            <xsl:text xml:space="preserve">\subsetdot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2ABE;'">
            <xsl:text xml:space="preserve">\supsetdot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2ABF;'">
            <xsl:text xml:space="preserve">\subsetplus </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AC0;'">
            <xsl:text xml:space="preserve">\supsetplus </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AC1;'">
            <xsl:text xml:space="preserve">\submult </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AC2;'">
            <xsl:text xml:space="preserve">\supmult </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AC3;'">
            <xsl:text xml:space="preserve">\subedot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AC4;'">
            <xsl:text xml:space="preserve">\supedot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AC7;'">
            <xsl:text xml:space="preserve">\subsim </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AC8;'">
            <xsl:text xml:space="preserve">\supsim </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AC9;'">
            <xsl:text xml:space="preserve">\subsetapprox </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2ACA;'">
            <xsl:text xml:space="preserve">\supsetapprox </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2ACD;'">
            <xsl:text xml:space="preserve">\lsqhook </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2ACE;'">
            <xsl:text xml:space="preserve">\rsqhook </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2ACF;'">
            <xsl:text xml:space="preserve">\csub </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AD0;'">
            <xsl:text xml:space="preserve">\csup </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AD1;'">
            <xsl:text xml:space="preserve">\csube </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AD2;'">
            <xsl:text xml:space="preserve">\csupe </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AD3;'">
            <xsl:text xml:space="preserve">\subsup </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AD4;'">
            <xsl:text xml:space="preserve">\supsub </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AD5;'">
            <xsl:text xml:space="preserve">\subsub </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AD6;'">
            <xsl:text xml:space="preserve">\supsup </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AD7;'">
            <xsl:text xml:space="preserve">\suphsub </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AD8;'">
            <xsl:text xml:space="preserve">\supdsub </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AD9;'">
            <xsl:text xml:space="preserve">\forkv </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2ADA;'">
            <xsl:text xml:space="preserve">\topfork </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2ADB;'">
            <xsl:text xml:space="preserve">\mlcp </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2ADC;'">
            <xsl:text xml:space="preserve">\forks </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2ADD;'">
            <xsl:text xml:space="preserve">\forksnot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2ADE;'">
            <xsl:text xml:space="preserve">\shortlefttack </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2ADF;'">
            <xsl:text xml:space="preserve">\shortdowntack </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AE0;'">
            <xsl:text xml:space="preserve">\shortuptack </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AE2;'">
            <xsl:text xml:space="preserve">\vDdash </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AE3;'">
            <xsl:text xml:space="preserve">\dashV </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AE4;'">
            <xsl:text xml:space="preserve">\Dashv </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AE5;'">
            <xsl:text xml:space="preserve">\DashV </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AE6;'">
            <xsl:text xml:space="preserve">\varVdash </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AE7;'">
            <xsl:text xml:space="preserve">\Barv </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AE8;'">
            <xsl:text xml:space="preserve">\vBar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AE9;'">
            <xsl:text xml:space="preserve">\vBarv </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AEA;'">
            <xsl:text xml:space="preserve">\barV </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AEB;'">
            <xsl:text xml:space="preserve">\Vbar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AEC;'">
            <xsl:text xml:space="preserve">\Not </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AED;'">
            <xsl:text xml:space="preserve">\bNot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AEE;'">
            <xsl:text xml:space="preserve">\revnmid </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AEF;'">
            <xsl:text xml:space="preserve">\cirmid </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AF0;'">
            <xsl:text xml:space="preserve">\midcir </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AF2;'">
            <xsl:text xml:space="preserve">\nhpar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AF3;'">
            <xsl:text xml:space="preserve">\parsim </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AF7;'">
            <xsl:text xml:space="preserve">\lllnest </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AF8;'">
            <xsl:text xml:space="preserve">\gggnest </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AF9;'">
            <xsl:text xml:space="preserve">\leqqslant </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AFA;'">
            <xsl:text xml:space="preserve">\geqqslant </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B30;'">
            <xsl:text xml:space="preserve">\circleonleftarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B31;'">
            <xsl:text xml:space="preserve">\leftthreearrows </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B32;'">
            <xsl:text xml:space="preserve">\leftarrowonoplus </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B33;'">
            <xsl:text xml:space="preserve">\longleftsquigarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B34;'">
            <xsl:text xml:space="preserve">\nvtwoheadleftarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B35;'">
            <xsl:text xml:space="preserve">\nVtwoheadleftarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B36;'">
            <xsl:text xml:space="preserve">\twoheadmapsfrom </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B37;'">
            <xsl:text xml:space="preserve">\twoheadleftdbkarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B38;'">
            <xsl:text xml:space="preserve">\leftdotarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B39;'">
            <xsl:text xml:space="preserve">\nvleftarrowtail </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B3A;'">
            <xsl:text xml:space="preserve">\nVleftarrowtail </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B3B;'">
            <xsl:text xml:space="preserve">\twoheadleftarrowtail </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B3C;'">
            <xsl:text xml:space="preserve">\nvtwoheadleftarrowtail </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B3D;'">
            <xsl:text xml:space="preserve">\nVtwoheadleftarrowtail </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B3E;'">
            <xsl:text xml:space="preserve">\leftarrowx </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B3F;'">
            <xsl:text xml:space="preserve">\leftcurvedarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B40;'">
            <xsl:text xml:space="preserve">\equalleftarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B41;'">
            <xsl:text xml:space="preserve">\bsimilarleftarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B42;'">
            <xsl:text xml:space="preserve">\leftarrowbackapprox </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B43;'">
            <xsl:text xml:space="preserve">\rightarrowgtr </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B44;'">
            <xsl:text xml:space="preserve">\rightarrowsupset </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B45;'">
            <xsl:text xml:space="preserve">\LLeftarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B46;'">
            <xsl:text xml:space="preserve">\RRightarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B47;'">
            <xsl:text xml:space="preserve">\bsimilarrightarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B48;'">
            <xsl:text xml:space="preserve">\rightarrowbackapprox </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B49;'">
            <xsl:text xml:space="preserve">\similarleftarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B4A;'">
            <xsl:text xml:space="preserve">\leftarrowapprox </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B4B;'">
            <xsl:text xml:space="preserve">\leftarrowbsimilar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2B4C;'">
            <xsl:text xml:space="preserve">\rightarrowbsimilar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x222F;'">
            <xsl:text xml:space="preserve">\smalloiint </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2230;'">
            <xsl:text xml:space="preserve">\smalloiiint </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2231;'">
            <xsl:text xml:space="preserve">\smallintclockwise </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2232;'">
            <xsl:text xml:space="preserve">\smallvarointclockwise </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2233;'">
            <xsl:text xml:space="preserve">\smallointctrclockwise </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A0B;'">
            <xsl:text xml:space="preserve">\smallsumint </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A0D;'">
            <xsl:text xml:space="preserve">\smallintbar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A0E;'">
            <xsl:text xml:space="preserve">\smallintBar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A0F;'">
            <xsl:text xml:space="preserve">\smallfint </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A10;'">
            <xsl:text xml:space="preserve">\smallcirfnint </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A11;'">
            <xsl:text xml:space="preserve">\smallawint </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A12;'">
            <xsl:text xml:space="preserve">\smallrppolint </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A13;'">
            <xsl:text xml:space="preserve">\smallscpolint </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A14;'">
            <xsl:text xml:space="preserve">\smallnpolint </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A15;'">
            <xsl:text xml:space="preserve">\smallpointint </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A16;'">
            <xsl:text xml:space="preserve">\smallsqint </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A17;'">
            <xsl:text xml:space="preserve">\smallintlarhk </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A18;'">
            <xsl:text xml:space="preserve">\smallintx </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A19;'">
            <xsl:text xml:space="preserve">\smallintcap </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A1A;'">
            <xsl:text xml:space="preserve">\smallintcup </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A1B;'">
            <xsl:text xml:space="preserve">\smallupint </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A1C;'">
            <xsl:text xml:space="preserve">\smalllowint </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x222F;'">
            <xsl:text xml:space="preserve">\oiint </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2230;'">
            <xsl:text xml:space="preserve">\oiiint </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2231;'">
            <xsl:text xml:space="preserve">\intclockwise </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2232;'">
            <xsl:text xml:space="preserve">\varointclockwise </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2233;'">
            <xsl:text xml:space="preserve">\ointctrclockwise </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A0B;'">
            <xsl:text xml:space="preserve">\sumint </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A0D;'">
            <xsl:text xml:space="preserve">\intbar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A0E;'">
            <xsl:text xml:space="preserve">\intBar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A0F;'">
            <xsl:text xml:space="preserve">\fint </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A10;'">
            <xsl:text xml:space="preserve">\cirfnint </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A11;'">
            <xsl:text xml:space="preserve">\awint </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A12;'">
            <xsl:text xml:space="preserve">\rppolint </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A13;'">
            <xsl:text xml:space="preserve">\scpolint </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A14;'">
            <xsl:text xml:space="preserve">\npolint </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A15;'">
            <xsl:text xml:space="preserve">\pointint </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A16;'">
            <xsl:text xml:space="preserve">\sqint </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A17;'">
            <xsl:text xml:space="preserve">\intlarhk </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A18;'">
            <xsl:text xml:space="preserve">\intx </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A19;'">
            <xsl:text xml:space="preserve">\intcap </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A1A;'">
            <xsl:text xml:space="preserve">\intcup </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A1B;'">
            <xsl:text xml:space="preserve">\upint </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A1C;'">
            <xsl:text xml:space="preserve">\lowint </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2140;'">
            <xsl:text xml:space="preserve">\Bbbsum </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27D5;'">
            <xsl:text xml:space="preserve">\leftouterjoin </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27D6;'">
            <xsl:text xml:space="preserve">\rightouterjoin </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27D7;'">
            <xsl:text xml:space="preserve">\fullouterjoin </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27D8;'">
            <xsl:text xml:space="preserve">\bigbot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27D9;'">
            <xsl:text xml:space="preserve">\bigtop </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29F8;'">
            <xsl:text xml:space="preserve">\xsol </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29F9;'">
            <xsl:text xml:space="preserve">\xbsol </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A03;'">
            <xsl:text xml:space="preserve">\bigcupdot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A05;'">
            <xsl:text xml:space="preserve">\bigsqcap </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A07;'">
            <xsl:text xml:space="preserve">\conjquant </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A08;'">
            <xsl:text xml:space="preserve">\disjquant </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A09;'">
            <xsl:text xml:space="preserve">\bigtimes </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A0A;'">
            <xsl:text xml:space="preserve">\modtwosum </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A1E;'">
            <xsl:text xml:space="preserve">\bigtriangleleft </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A1F;'">
            <xsl:text xml:space="preserve">\zcmp </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A20;'">
            <xsl:text xml:space="preserve">\zpipe </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2A21;'">
            <xsl:text xml:space="preserve">\zproject </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AFC;'">
            <xsl:text xml:space="preserve">\biginterleave </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2AFF;'">
            <xsl:text xml:space="preserve">\bigtalloblong </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x23B0;'">
            <xsl:text xml:space="preserve">\lmoustache </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2772;'">
            <xsl:text xml:space="preserve">\lbrbrak </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27E6;'">
            <xsl:text xml:space="preserve">\lBrack </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27EA;'">
            <xsl:text xml:space="preserve">\lAngle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27EE;'">
            <xsl:text xml:space="preserve">\lgroup </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2983;'">
            <xsl:text xml:space="preserve">\lBrace </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2985;'">
            <xsl:text xml:space="preserve">\lParen </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x23B1;'">
            <xsl:text xml:space="preserve">\rmoustache </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2773;'">
            <xsl:text xml:space="preserve">\rbrbrak </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27E7;'">
            <xsl:text xml:space="preserve">\rBrack </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27EB;'">
            <xsl:text xml:space="preserve">\rAngle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27EF;'">
            <xsl:text xml:space="preserve">\rgroup </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2984;'">
            <xsl:text xml:space="preserve">\rBrace </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2986;'">
            <xsl:text xml:space="preserve">\rParen </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2980;'">
            <xsl:text xml:space="preserve">\Vvert </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x290A;'">
            <xsl:text xml:space="preserve">\Uuparrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x290B;'">
            <xsl:text xml:space="preserve">\Ddownarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27F0;'">
            <xsl:text xml:space="preserve">\UUparrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27F1;'">
            <xsl:text xml:space="preserve">\DDownarrow </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27EC;'">
            <xsl:text xml:space="preserve">\Lbrbrak </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27ED;'">
            <xsl:text xml:space="preserve">\Rbrbrak </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2987;'">
            <xsl:text xml:space="preserve">\llparenthesis </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2988;'">
            <xsl:text xml:space="preserve">\rrparenthesis </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2989;'">
            <xsl:text xml:space="preserve">\llangle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x298A;'">
            <xsl:text xml:space="preserve">\rrangle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x298B;'">
            <xsl:text xml:space="preserve">\lbrackubar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x298C;'">
            <xsl:text xml:space="preserve">\rbrackubar </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x298D;'">
            <xsl:text xml:space="preserve">\lbrackultick </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x298E;'">
            <xsl:text xml:space="preserve">\rbracklrtick </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x298F;'">
            <xsl:text xml:space="preserve">\lbracklltick </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2990;'">
            <xsl:text xml:space="preserve">\rbrackurtick </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2991;'">
            <xsl:text xml:space="preserve">\langledot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2992;'">
            <xsl:text xml:space="preserve">\rangledot </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2993;'">
            <xsl:text xml:space="preserve">\lparenless </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2994;'">
            <xsl:text xml:space="preserve">\rparengtr </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2995;'">
            <xsl:text xml:space="preserve">\Lparengtr </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2996;'">
            <xsl:text xml:space="preserve">\Rparenless </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2997;'">
            <xsl:text xml:space="preserve">\lblkbrbrak </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2998;'">
            <xsl:text xml:space="preserve">\rblkbrbrak </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29D8;'">
            <xsl:text xml:space="preserve">\lvzigzag </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29D9;'">
            <xsl:text xml:space="preserve">\rvzigzag </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29DA;'">
            <xsl:text xml:space="preserve">\Lvzigzag </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29DB;'">
            <xsl:text xml:space="preserve">\Rvzigzag </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29FC;'">
            <xsl:text xml:space="preserve">\lcurvyangle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x29FD;'">
            <xsl:text xml:space="preserve">\rcurvyangle </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2772;'">
            <xsl:text xml:space="preserve">\lbrbrak </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x2773;'">
            <xsl:text xml:space="preserve">\rbrbrak </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27C5;'">
            <xsl:text xml:space="preserve">\lbag </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27C6;'">
            <xsl:text xml:space="preserve">\rbag </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27EC;'">
            <xsl:text xml:space="preserve">\Lbrbrak </xsl:text>
          </xsl:when>
          <xsl:when test="$first-char='&#x27ED;'">
            <xsl:text xml:space="preserve">\Rbrbrak </xsl:text>
          </xsl:when>
<!-- End additions to support characters available when using the stix package -->


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
