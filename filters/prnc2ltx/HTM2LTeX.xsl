<?xml version="1.0"?>
<xsl:stylesheet 
      xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
      xmlns:exsl="http://exslt.org/common"
      xmlns:msi="C:/xml/xsl/lbtest"
      version="1.1">


<!-- *******************Line breaking***************************************

Basic Algorithm

Line breaking is done on paragraphs.  The initial output scripted within
paragraphs is a list of intermediate objects, tagged as <text> and <math>.
Actual line breaking is preformed on the contents of these nodes. There
are 2 reasons for this decomposition.

  1) to reduce the length of strings processed.
  2) to allow for different breaking strategies in MATH and TEXT.

In TEXT, breaking is based on word boundaries, ie. space characters.

In MATH, breaking is based on tokens that are embedded in the LaTeX stream
generated during initial MathML object translation.  Two different tokens
are used.

  1) \LBe - forces a newline in the output
          - used to produce standard LaTeX environment layout
\begin{eqnarray}
...               \\
...
\end{eqnarray}

  2) \LBo - a newline is allowed at this token

The \LBo token can be added to initial LaTeX output anywhere that LaTeX
would allow a line break.  Currently (for reasons of efficiency) it's only
scripted in front of most binary operators, in front of & in arrays, and
after \allowbreak.  More \LBo's can be scripted as needed.


**************************************************************************** -->

  <xsl:template match="comment()">
    <xsl:choose>
      <xsl:when test="contains(string(.),'MSICOMMENTLINE:')">
        <comment-line>
          <xsl:text>%</xsl:text>
          <xsl:value-of select="substring-after(string(.),'MSICOMMENTLINE:')"/>
        </comment-line>
      </xsl:when>
      <xsl:otherwise>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>


  <xsl:template match="msi:p">
    <xsl:variable name="psuedo-LaTeX.tr">
      <xsl:apply-templates/>
    </xsl:variable>
    <xsl:variable name="psuedo-LaTeX" select="$exsl:node-set(psuedo-LaTeX.tr)"/>

<!-- The children of "psuedo-LaTeX" are mixture of generic text nodes, 
     <comment-line> and <stream-with-break-tokens> nodes.
     Here we convert to a list with explicit <text> and <math> nodes. -->

    <xsl:variable name="text-math-list.tr">
      <xsl:for-each select="$psuedo-LaTeX/node()">
        <xsl:choose>
          <xsl:when test="self::stream-with-break-tokens">
            <math>
              <xsl:apply-templates select="."/>
            </math>
          </xsl:when>
          <xsl:when test="self::comment-line">
            <comment-line>
              <xsl:value-of select="string(.)"/>
            </comment-line>
          </xsl:when>
          <xsl:when test="self::text()">
            <text>
              <xsl:variable name="LaTeX-contents">
                <xsl:call-template name="do-chars-in-TEXT">
                  <xsl:with-param name="unicode-cdata" select="string()"/>
                </xsl:call-template>
              </xsl:variable>
              <xsl:call-template name="remove-dollar-dollar">
                <xsl:with-param name="LaTeX-zstr" select="$LaTeX-contents"/>
              </xsl:call-template>
            </text>
          </xsl:when>
          <xsl:otherwise>
            <text>
              <xsl:value-of select="string(.)"/>
            </text>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:for-each>
    </xsl:variable>
    <xsl:variable name="text-math-list" select="exsl:node-set($text-math-list.tr)"/>

<!--
<xsl:if test="self::comment()">
</xsl:if>
<xsl:for-each select="./node()">
  <xsl:text>&lt;</xsl:text>
  <xsl:value-of select="name(.)"/>
  <xsl:text>&gt;</xsl:text>

  <xsl:for-each select="./node()">
    <xsl:text xml:space="preserve">&lt;</xsl:text>
    <xsl:value-of select="name(.)"/>
    <xsl:text>&gt;</xsl:text>

    <xsl:for-each select="./node()">
      <xsl:text xml:space="preserve">&lt;</xsl:text>
      <xsl:value-of select="name(.)"/>
      <xsl:text>&gt;</xsl:text>

      <xsl:for-each select="./node()">
        <xsl:text xml:space="preserve">&lt;</xsl:text>
        <xsl:value-of select="name(.)"/>
        <xsl:text>&gt;</xsl:text>

        <xsl:for-each select="./node()">
          <xsl:text xml:space="preserve">&lt;</xsl:text>
          <xsl:value-of select="name(.)"/>
          <xsl:text>&gt;</xsl:text>

          <xsl:value-of select="string(.)"/>

          <xsl:text>&lt;/</xsl:text>
          <xsl:value-of select="name(.)"/>
          <xsl:text>&gt;</xsl:text>
        </xsl:for-each>

        <xsl:text>&lt;/</xsl:text>
        <xsl:value-of select="name(.)"/>
        <xsl:text>&gt;</xsl:text>
      </xsl:for-each>

      <xsl:text>&lt;/</xsl:text>
      <xsl:value-of select="name(.)"/>
      <xsl:text>&gt;</xsl:text>
    </xsl:for-each>

    <xsl:text>&lt;/</xsl:text>
    <xsl:value-of select="name(.)"/>
    <xsl:text>&gt;</xsl:text>
  </xsl:for-each>

  <xsl:text>&lt;/</xsl:text>
  <xsl:value-of select="name(.)"/>
  <xsl:text>&gt;</xsl:text>
</xsl:for-each>
-->

<!--
<xsl:for-each select="$text-math-list/*">
  <xsl:text>&lt;</xsl:text>
  <xsl:value-of select="name(.)"/>
  <xsl:text>&gt;</xsl:text>
  <xsl:value-of select="string(.)"/>
  <xsl:text>&lt;/</xsl:text>
  <xsl:value-of select="name(.)"/>
  <xsl:text>&gt;</xsl:text>
</xsl:for-each>
-->


	<xsl:choose>
      <xsl:when test="@class='quote'">
        <xsl:text>\begin{quote}&lt;EOLN/&gt;</xsl:text>
      </xsl:when>
      <xsl:when test="@class='quotation'">
        <xsl:text>\begin{quotation}&lt;EOLN/&gt;</xsl:text>
      </xsl:when>
      <xsl:when test="@class='center'">
        <xsl:text>\begin{center}&lt;EOLN/&gt;</xsl:text>
      </xsl:when>
      <xsl:otherwise>
      </xsl:otherwise>
	</xsl:choose>


<!-- Initiate a series of recursive calls to line-break the contents
     of <text> and <math> nodes in our list. -->

    <xsl:choose>
      <xsl:when test="$text-math-list/*[1]">
        <xsl:for-each select="$text-math-list/*[1]">
              <xsl:call-template name="break-next-LaTeX-frag">
	     	    <xsl:with-param name="tail" select="''"/>
              </xsl:call-template>
        </xsl:for-each>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text xml:space="preserve">\ &lt;EOLN/&gt;</xsl:text>
      </xsl:otherwise>
    </xsl:choose>


	<xsl:choose>
      <xsl:when test="@class='quote'">
        <xsl:text>\end{quote}&lt;EOLN/&gt;</xsl:text>
      </xsl:when>
      <xsl:when test="@class='quotation'">
        <xsl:text>\end{quotation}&lt;EOLN/&gt;</xsl:text>
      </xsl:when>
      <xsl:when test="@class='center'">
        <xsl:text>\end{center}&lt;EOLN/&gt;</xsl:text>
      </xsl:when>
      <xsl:otherwise>
      </xsl:otherwise>
	</xsl:choose>

<!-- Script an empty line to end our LaTeX paragraph. -->

    <xsl:text>&lt;EOLN/&gt;</xsl:text>

  </xsl:template>



<!-- Initial translation of xml(MML) -> LaTeX produces a list
    comprised of <text> and <math> nodes.  The following recursive
	template traverses this list, producing lines of LaTeX output. -->


  <xsl:template name="break-next-LaTeX-frag">
    <xsl:param name="remainder-from-last-node"/>
<!-- Get the LaTeX string for the current node in the text-math list -->
    <xsl:variable name="next-LaTeX-frag">
      <xsl:choose>
        <xsl:when test="self::math">
          <xsl:value-of select="normalize-space(string(.))"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="string(.)"/>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:variable name="is-math">
      <xsl:if test="self::math">
        <xsl:text>true</xsl:text>
      </xsl:if>
    </xsl:variable>

<!-- Any partial line that is left over is returned in "$lines/remainder". -->

    <xsl:variable name="lines.tr">
      <xsl:call-template name="make-lines">
        <xsl:with-param name="left-overs" select="$remainder-from-last-node"/>
        <xsl:with-param name="curr-LaTeX-frag" select="$next-LaTeX-frag"/>
        <xsl:with-param name="next-is-math" select="$is-math"/>
      </xsl:call-template>
    </xsl:variable>
    <xsl:variable name="lines" select="exsl:node-set($lines.tr)"/>

<!--
<xsl:text>call-template make-lines:</xsl:text>
  <xsl:value-of select="$remainder-from-last-node"/>

<xsl:text>|</xsl:text><xsl:value-of select="$next-LaTeX-frag"/><xsl:text>|</xsl:text>
  <xsl:value-of select="$is-math"/>
<xsl:text>:</xsl:text>

<xsl:call-template name="make-lines">
  <xsl:with-param name="left-overs" select="$remainder-from-last-node"/>
  <xsl:with-param name="curr-LaTeX-frag" select="$next-LaTeX-frag"/>
  <xsl:with-param name="next-is-math" select="$is-math"/>
</xsl:call-template>
 -->


<!-- Output the lines generated by the call to "make-lines" above.
     Script a line-ending token after each line.  -->

    <xsl:for-each select="$lines/line">
      <xsl:if test="string-length(string(.))&gt;0">
        <xsl:value-of select="."/>
        <xsl:text>%&lt;EOLN/&gt;</xsl:text>
      </xsl:if>
    </xsl:for-each>

    <xsl:choose>

<!-- Recursive call to "break-next-LaTeX-frag" to handle the next
     node in our psuedo LaTeX tree.  -->

      <xsl:when test="following-sibling::*[1]">

        <xsl:for-each select="following-sibling::*[1]">
          <xsl:call-template name="break-next-LaTeX-frag">
		    <xsl:with-param name="remainder-from-last-node" select="$lines/remainder"/>
          </xsl:call-template>
        </xsl:for-each>
      </xsl:when>

<!-- Here we're at the end of the list - output anything left over
     from line-breaking the current node.  -->

      <xsl:otherwise>

        <xsl:if test="string-length($lines/remainder)">
          <xsl:for-each select="$lines/remainder">
            <xsl:value-of select="."/>
            <xsl:text>%&lt;EOLN/&gt;</xsl:text>
          </xsl:for-each>
        </xsl:if>
      </xsl:otherwise>
    </xsl:choose>

  </xsl:template>


<!-- LaTeX runs are decomposed (math and text separately)
    into a list of nodes that contain line-breaking atoms.
    These atoms are catenated to produce final LaTeX output. -->

  <xsl:template name="make-lines">
    <xsl:param name="left-overs"/>
    <xsl:param name="curr-LaTeX-frag"/>
    <xsl:param name="next-is-math"/>
    <xsl:choose>
<!-- Break MATH and form lines -->

      <xsl:when test="$next-is-math='true'">
 
        <xsl:variable name="math-atom-list.tr">
          <xsl:if test="string-length($left-overs)&gt;0">
<math-atom>
            <xsl:value-of select="string($left-overs)"/>
</math-atom>
          </xsl:if>
          <xsl:call-template name="make-math-atoms">
            <xsl:with-param name="LaTeX-run" select="$curr-LaTeX-frag"/>
          </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="math-atom-list" select="exsl:node-set($math-atom-list.tr)"/>

<!-- Initiate a series of recursive calls to catenate
      the contents of the <math-atom> nodes in our list. -->

        <xsl:for-each select="$math-atom-list/*[1]">
          <xsl:call-template name="join-math-atoms">
            <xsl:with-param name="line-so-far" select="''"/>
          </xsl:call-template>
        </xsl:for-each>
      </xsl:when>

      <xsl:when test="self::comment-line">
        <xsl:if test="string-length($left-overs)&gt;0">
<line>
          <xsl:value-of select="string($left-overs)"/>
</line>
        </xsl:if>
<line>
        <xsl:value-of select="string(.)"/>
</line>
      </xsl:when>

      <xsl:otherwise>
<!-- Break TEXT and form lines -->

        <xsl:variable name="first-char">
          <xsl:value-of select="substring($curr-LaTeX-frag,1,1)"/>
        </xsl:variable>

        <xsl:variable name="last-char">
          <xsl:value-of select="substring($curr-LaTeX-frag,string-length($curr-LaTeX-frag),1)"/>
        </xsl:variable>

        <xsl:variable name="words.tr">
          <xsl:if test="string-length($left-overs)&gt;0">
<line-frag>
            <xsl:value-of select="string($left-overs)"/>
</line-frag>
          </xsl:if>

          <xsl:if test="$first-char='&#x0020;'
          or            $first-char='&#x000A;'
          or            $first-char='&#x000D;'">
<word>
</word>
          </xsl:if>

          <xsl:call-template name="make-words">
            <xsl:with-param name="LaTeX-run" select="normalize-space($curr-LaTeX-frag)"/>
          </xsl:call-template>

          <xsl:if test="$last-char='&#x0020;'
          or            $last-char='&#x000A;'
          or            $last-char='&#x000D;'">
<word>
</word>
          </xsl:if>

        </xsl:variable>
        <xsl:variable name="words" select="exsl:node-set($words.tr)"/>

<!-- Initiate a series of recursive calls to catenate the contents
     of <word> nodes in our list. -->

<!--
<xsl:text>LATEX-FRAG</xsl:text>
<xsl:value-of select="$curr-LaTeX-frag"/>
<xsl:text>/LATEX-FRAG</xsl:text>

<xsl:text>FIRST</xsl:text>
<xsl:value-of select="$first-char"/>
<xsl:text>FIRST</xsl:text>

<xsl:text>LAST</xsl:text>
<xsl:value-of select="$last-char"/>
<xsl:text>LAST</xsl:text>

<xsl:for-each select="$words/*">
<xsl:if test="self::word">
<xsl:text>&lt;WORD&gt;</xsl:text>
<xsl:value-of select="."/>
<xsl:text>&lt;/WORD&gt;</xsl:text>
</xsl:if>
<xsl:if test="self::line-frag">
<xsl:text>&lt;LINE-FRAG&gt;</xsl:text>
<xsl:value-of select="."/>
<xsl:text>&lt;/LINE-FRAG&gt;</xsl:text>
</xsl:if>
</xsl:for-each>
-->

        <xsl:for-each select="$words/*[1]">
          <xsl:call-template name="join-words">
            <xsl:with-param name="line-so-far" select="''"/>
          </xsl:call-template>
        </xsl:for-each>
      </xsl:otherwise>

    </xsl:choose>

  </xsl:template>



<!-- Recursive template to traverse a list 
   <line-frag> -> <word> -> <word> .. 
     and join them into lines. -->

  <xsl:template name="join-words">
    <xsl:param name="line-so-far"/>
    <xsl:variable name="join-char">
    <xsl:choose>
      <xsl:when test="string-length($line-so-far)=0
      or              preceding-sibling::*[1][self::line-frag]">
        <xsl:text xml:space="preserve"></xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text xml:space="preserve"> </xsl:text>
      </xsl:otherwise>
    </xsl:choose>
    </xsl:variable>

    <xsl:variable name="line-after-join">
      <xsl:choose>
        <xsl:when test="string-length( concat($line-so-far,$join-char,string(.)) ) &lt; 72">
          <xsl:value-of select="concat($line-so-far,$join-char,string(.))"/>
	    </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="string(.)"/>
	    </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:if test="string-length(concat($line-so-far,$join-char,string(.)) ) &gt;= 72">
<line>
      <xsl:value-of select="concat($line-so-far,$join-char)"/>
</line>
    </xsl:if>

    <xsl:choose>

<!-- Recursive call to "join-words" to handle
      the next node in our <word> list.  -->

      <xsl:when test="following-sibling::*[1]">
        <xsl:for-each select="following-sibling::*[1]">
          <xsl:call-template name="join-words">
            <xsl:with-param name="line-so-far" select="$line-after-join"/>
          </xsl:call-template>
        </xsl:for-each>
      </xsl:when>

      <xsl:otherwise>
<remainder>
        <xsl:value-of select="$line-after-join"/>
</remainder>
      </xsl:otherwise>
    </xsl:choose>

  </xsl:template>



<!-- Template to break a run of LaTeX text into a list of words.
     The param "LaTeX-run" has no lead or tail whitespace. -->

  <xsl:template name="make-words">
    <xsl:param name="LaTeX-run"/>
    <xsl:choose>
      <xsl:when test="contains($LaTeX-run,' ')">
        <xsl:if test="string-length(substring-before($LaTeX-run,' '))&gt;0">
<word>
          <xsl:value-of select="normalize-space(substring-before($LaTeX-run,' '))"/>
</word>
        </xsl:if>
        <xsl:call-template name="make-words">
          <xsl:with-param name="LaTeX-run" select="normalize-space(substring-after($LaTeX-run,' '))"/>
        </xsl:call-template>
      </xsl:when>

      <xsl:otherwise>
        <xsl:if test="string-length($LaTeX-run)&gt;0">
<word>
          <xsl:value-of select="normalize-space($LaTeX-run)"/>
</word>
        </xsl:if>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>




<!-- Recursive template to traverse a list comprised of

   <math-atom> ... <math-atom-ender> ... 

     Lines of final LaTeX output are put in <line> nodes.
-->

  <xsl:template name="join-math-atoms">
    <xsl:param name="line-so-far"/>
    <xsl:variable name="line-after-join">
      <xsl:choose>
        <xsl:when test="self::math-atom-ender">
	    </xsl:when>

        <xsl:when test="string-length($line-so-far) + string-length(.) &lt; 72">
          <xsl:value-of select="concat( $line-so-far,string(.) )"/>
	    </xsl:when>

        <xsl:otherwise>
          <xsl:value-of select="string(.)"/>
	    </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>



    <xsl:if test="string-length($line-so-far) + string-length(.) &gt;= 72">
      <xsl:if test="string-length($line-so-far) &gt; 0">
<line>
        <xsl:value-of select="$line-so-far"/>
</line>
      </xsl:if>
    </xsl:if>

    <xsl:if test="self::math-atom-ender">
<line>
      <xsl:if test="string-length($line-so-far) + string-length(.) &lt; 72">
        <xsl:value-of select="$line-so-far"/>
      </xsl:if>
      <xsl:value-of select="string(.)"/>
</line>
    </xsl:if>


    <xsl:choose>

<!-- Recursive call to "join-math-atoms" to handle
     the next node in our psuedo LaTeX tree.  -->

      <xsl:when test="following-sibling::*[1]">
        <xsl:for-each select="following-sibling::*[1]">
          <xsl:call-template name="join-math-atoms">
            <xsl:with-param name="line-so-far" select="$line-after-join"/>
          </xsl:call-template>
        </xsl:for-each>
      </xsl:when>

      <xsl:otherwise>
<remainder>
        <xsl:value-of select="$line-after-join"/>
</remainder>
      </xsl:otherwise>
    </xsl:choose>

  </xsl:template>



<!-- Tokens used for line-breaking ( \LBe and \LBo ) are inserted into
   the LaTeX MATH output at initial translation time.  This template uses
   these tokens to build a list of atomic math fragments.  The fragments
   in this list are joined to create lines of final LaTeX output. -->


  <xsl:template name="make-math-atoms">
    <xsl:param name="LaTeX-run"/>
    <xsl:choose>
      <xsl:when test="contains($LaTeX-run,'\LB')">

        <xsl:variable name="break-ilk">
          <xsl:value-of select="substring(substring-after($LaTeX-run,'\LB'),1,1)"/>
        </xsl:variable>

        <xsl:choose>
          <xsl:when test="$break-ilk='e'">
<math-atom-ender>
            <xsl:value-of select="substring-before($LaTeX-run,'\LB')"/>
</math-atom-ender>
<!--jcs
            <xsl:if test="string-length(substring-after($LaTeX-run,'\LBe'))&gt;0">
              <xsl:call-template name="make-math-atoms">
                <xsl:with-param name="LaTeX-run" select="substring-after($LaTeX-run,'\LBe')"/>
              </xsl:call-template>
            </xsl:if>
-->
          </xsl:when>

          <xsl:when test="$break-ilk='o'">
<math-atom>
            <xsl:value-of select="substring-before($LaTeX-run,'\LB')"/>
</math-atom>
            <xsl:if test="string-length(substring-after($LaTeX-run,'\LBo'))&gt;0">
              <xsl:call-template name="make-math-atoms">
                <xsl:with-param name="LaTeX-run" select="substring-after($LaTeX-run,'\LBo')"/>
              </xsl:call-template>
            </xsl:if>
          </xsl:when>

          <xsl:otherwise>
          </xsl:otherwise>

        </xsl:choose>

      </xsl:when>

      <xsl:otherwise>
        <xsl:if test="string-length($LaTeX-run)&gt;0">
<math-atom>
          <xsl:value-of select="$LaTeX-run"/>
</math-atom>
        </xsl:if>
      </xsl:otherwise>
    </xsl:choose>

  </xsl:template>


<!-- ENTITY % ul "(li)+" -->

  <xsl:template match="msi:ul">
    <xsl:text xml:space="preserve">\begin{itemize}&lt;EOLN/&gt;</xsl:text>
    <xsl:apply-templates/>
    <xsl:text xml:space="preserve">\end{itemize}&lt;EOLN/&gt;</xsl:text>
<!-- Script an empty line to end our LaTeX paragraph. -->
    <xsl:text>&lt;EOLN/&gt;</xsl:text>
  </xsl:template>


<!-- ENTITY % ol "(li)+" -->
  <xsl:template match="msi:ol">
    <xsl:text xml:space="preserve">\begin{enumerate}&lt;EOLN/&gt;</xsl:text>
    <xsl:apply-templates/>
    <xsl:text xml:space="preserve">\end{enumerate}&lt;EOLN/&gt;</xsl:text>
<!-- Script an empty line to end our LaTeX paragraph. -->
    <xsl:text>&lt;EOLN/&gt;</xsl:text>
  </xsl:template>


<!-- ENTITY % li "(p)+" -->
  <xsl:template match="msi:li">
    <xsl:text xml:space="preserve">\item </xsl:text>
    <xsl:apply-templates/>
  </xsl:template>


<!-- ENTITY % dl "(dt*, dd)+" -->
  <xsl:template match="msi:dl">
    <xsl:text xml:space="preserve">\begin{description}&lt;EOLN/&gt;</xsl:text>
    <xsl:apply-templates/>
    <xsl:text xml:space="preserve">\end{description}&lt;EOLN/&gt;</xsl:text>
<!-- Script an empty line to end our LaTeX paragraph. -->
    <xsl:text>&lt;EOLN/&gt;</xsl:text>
  </xsl:template>


<!-- ENTITY % dt "(#PCDATA)" -->
  <xsl:template match="msi:dt">
    <xsl:text xml:space="preserve">\item[</xsl:text>
    <xsl:apply-templates/>
    <xsl:text xml:space="preserve">] </xsl:text>
  </xsl:template>


<!-- ENTITY % dd "(p)" -->
  <xsl:template match="msi:dd">
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="msi:h1">
    <xsl:call-template name="do-section-cmd">
      <xsl:with-param name="nom" select="@class"/>
    </xsl:call-template>
  </xsl:template>

  <xsl:template match="msi:h2">
    <xsl:call-template name="do-section-cmd">
      <xsl:with-param name="nom" select="@class"/>
    </xsl:call-template>
  </xsl:template>

  <xsl:template match="msi:h3">
    <xsl:call-template name="do-section-cmd">
      <xsl:with-param name="nom" select="@class"/>
    </xsl:call-template>
  </xsl:template>

  <xsl:template match="msi:h4">
    <xsl:call-template name="do-section-cmd">
      <xsl:with-param name="nom" select="@class"/>
    </xsl:call-template>
  </xsl:template>

  <xsl:template match="msi:h5">
    <xsl:call-template name="do-section-cmd">
      <xsl:with-param name="nom" select="@class"/>
    </xsl:call-template>
  </xsl:template>


  <xsl:template name="do-section-cmd">
    <xsl:param name="nom"/>
    <xsl:call-template name="do-text-bucket">
      <xsl:with-param name="LaTeX-run-nom" select="concat('\',$nom,'{')"/>
    </xsl:call-template>

	<xsl:text>&lt;EOLN/&gt;&lt;EOLN/&gt;</xsl:text>
  </xsl:template>


  <xsl:template match="msi:em">
    <xsl:call-template name="do-text-bucket">
      <xsl:with-param name="LaTeX-run-nom" select="'\emph{'"/>
    </xsl:call-template>
  </xsl:template>


  <xsl:template match="msi:strong">
    <xsl:call-template name="do-text-bucket">
      <xsl:with-param name="LaTeX-run-nom" select="'\textbf{'"/>
    </xsl:call-template>
  </xsl:template>



  <xsl:template name="remove-breaking-token">
    <xsl:param name="LaTeX-zstr"/>
    <xsl:param name="token"/>
    <xsl:choose>
      <xsl:when test="contains($LaTeX-zstr,$token)">
        <xsl:if test="string-length(substring-before($LaTeX-zstr,$token))&gt;0">
          <xsl:value-of select="substring-before($LaTeX-zstr,$token)"/>
        </xsl:if>
        <xsl:if test="string-length(substring-after($LaTeX-zstr,$token))&gt;0">
          <xsl:call-template name="remove-breaking-token">
            <xsl:with-param name="LaTeX-zstr" select="substring-after($LaTeX-zstr,$token)"/>
            <xsl:with-param name="token" select="$token"/>
          </xsl:call-template>
        </xsl:if>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="$LaTeX-zstr"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>



  <xsl:template name="do-text-bucket">
    <xsl:param name="LaTeX-run-nom"/>
    <xsl:variable name="psuedo-LaTeX.tr">
      <xsl:apply-templates/>
    </xsl:variable>
    <xsl:variable name="psuedo-LaTeX" select="exsl:node-set($psuedo-LaTeX.tr)"/>

<translated-text>
    <xsl:if test="string-length($LaTeX-run-nom) &gt; 0">
      <xsl:value-of select="$LaTeX-run-nom"/>
	</xsl:if>

      <xsl:for-each select="$psuedo-LaTeX/node()">
        <xsl:choose>
          <xsl:when test="self::text()">
            <xsl:variable name="LaTeX-text">
              <xsl:call-template name="do-chars-in-TEXT">
                <xsl:with-param name="unicode-cdata" select="string()"/>
              </xsl:call-template>
            </xsl:variable>
            <xsl:call-template name="remove-dollar-dollar">
              <xsl:with-param name="LaTeX-zstr" select="$LaTeX-text"/>
            </xsl:call-template>
		  </xsl:when>
		  <xsl:otherwise>

            <xsl:variable name="temp">
              <xsl:call-template name="remove-breaking-token">
                <xsl:with-param name="LaTeX-zstr" select="string()"/>
                <xsl:with-param name="token" select="'\LBe'"/>
              </xsl:call-template>
            </xsl:variable>

            <xsl:call-template name="remove-breaking-token">
              <xsl:with-param name="LaTeX-zstr" select="$temp"/>
              <xsl:with-param name="token" select="'\LBo'"/>
            </xsl:call-template>

		  </xsl:otherwise>
        </xsl:choose>
      </xsl:for-each>

    <xsl:if test="string-length($LaTeX-run-nom) &gt; 0">
      <xsl:text>}</xsl:text>
	</xsl:if>
</translated-text>

  </xsl:template>




<!-- a href="osc02__note.htm#Note_1" -->

  <xsl:template match="msi:a">
  <anchor>
    <xsl:choose>
      <xsl:when test="string-length(@ID) &gt; 0">
        <xsl:text>\label{</xsl:text>
		<xsl:value-of select="@ID"/>
        <xsl:text>}</xsl:text>
      </xsl:when>
      <xsl:when test="string-length(@id) &gt; 0">
        <xsl:text>\label{</xsl:text>
		<xsl:value-of select="@id"/>
        <xsl:text>}</xsl:text>
      </xsl:when>

      <xsl:when test="string-length(@href) &gt; 0">
        <xsl:text>\NOTE{</xsl:text>
        <xsl:call-template name="do-text-bucket">
          <xsl:with-param name="LaTeX-run-nom" select="''"/>
        </xsl:call-template>
        <xsl:text>}</xsl:text>
      </xsl:when>

      <xsl:when test="string-length(@HREF) &gt; 0">
        <xsl:text>\hyperref{</xsl:text>
        <xsl:call-template name="do-text-bucket">
          <xsl:with-param name="LaTeX-run-nom" select="''"/>
        </xsl:call-template>
        <xsl:text>}{}{}{</xsl:text>
          <xsl:value-of select="@HREF"/>
        <xsl:text>}</xsl:text>
      </xsl:when>

      <xsl:otherwise>
      </xsl:otherwise>
    </xsl:choose>
</anchor>
  </xsl:template>


  <xsl:template match="msi:span">
    <xsl:choose>
      <xsl:when test="substring(@class,1,4)='QTO:'">
<tagged-text>
        <xsl:if test="$output-mode='SW-LaTeX'">
          <xsl:value-of select="string(.)"/>
        </xsl:if>
        <xsl:if test="$output-mode='Portable-LaTeX'">
          <xsl:value-of select="string(.)"/>
        </xsl:if>
</tagged-text>
      </xsl:when>
      <xsl:when test="@class='group'">
        <xsl:call-template name="do-text-bucket">
          <xsl:with-param name="LaTeX-run-nom" select="'{'"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="@class='it'">
        <xsl:call-template name="do-text-bucket">
          <xsl:with-param name="LaTeX-run-nom" select="'\textit{'"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="@class='sf'">
        <xsl:call-template name="do-text-bucket">
          <xsl:with-param name="LaTeX-run-nom" select="'\textsf{'"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="@class='sc'">
        <xsl:call-template name="do-text-bucket">
          <xsl:with-param name="LaTeX-run-nom" select="'\textsc{'"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="@class='sl'">
        <xsl:call-template name="do-text-bucket">
          <xsl:with-param name="LaTeX-run-nom" select="'\textsl{'"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="@class='rm'">
        <xsl:call-template name="do-text-bucket">
          <xsl:with-param name="LaTeX-run-nom" select="'\textrm{'"/>
        </xsl:call-template>
      </xsl:when>

<!-- span class="tiny">tiny roman </span -->
      <xsl:when test="@class='tiny'">
        <xsl:call-template name="do-text-bucket">
          <xsl:with-param name="LaTeX-run-nom" select="'{\tiny '"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="@class='scriptsize'">
        <xsl:call-template name="do-text-bucket">
          <xsl:with-param name="LaTeX-run-nom" select="'{\scriptsize '"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="@class='footnotesize'">
        <xsl:call-template name="do-text-bucket">
          <xsl:with-param name="LaTeX-run-nom" select="'{\footnotesize '"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="@class='small'">
        <xsl:call-template name="do-text-bucket">
          <xsl:with-param name="LaTeX-run-nom" select="'{\small '"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="@class='normalsize'">
        <xsl:call-template name="do-text-bucket">
          <xsl:with-param name="LaTeX-run-nom" select="'{\normalsize '"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="@class='large1'">
        <xsl:call-template name="do-text-bucket">
          <xsl:with-param name="LaTeX-run-nom" select="'{\large '"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="@class='large2'">
        <xsl:call-template name="do-text-bucket">
          <xsl:with-param name="LaTeX-run-nom" select="'{\Large '"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="@class='large3'">
        <xsl:call-template name="do-text-bucket">
          <xsl:with-param name="LaTeX-run-nom" select="'{\LARGE '"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="@class='huge1'">
        <xsl:call-template name="do-text-bucket">
          <xsl:with-param name="LaTeX-run-nom" select="'{\huge '"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="@class='huge2'">
        <xsl:call-template name="do-text-bucket">
          <xsl:with-param name="LaTeX-run-nom" select="'{\Huge '"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="@class='huge3'">
        <xsl:call-template name="do-text-bucket">
          <xsl:with-param name="LaTeX-run-nom" select="'{\HUGE '"/>
        </xsl:call-template>
      </xsl:when>


      <xsl:when test="@class='cite'">
        <xsl:call-template name="do-text-bucket">
          <xsl:with-param name="LaTeX-run-nom" select="'\cite{'"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="@class='ref'">
        <xsl:call-template name="do-text-bucket">
          <xsl:with-param name="LaTeX-run-nom" select="'\ref{'"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="@class='pageref'">
        <xsl:call-template name="do-text-bucket">
          <xsl:with-param name="LaTeX-run-nom" select="'\pageref{'"/>
        </xsl:call-template>
      </xsl:when>


      <xsl:when test="@class='KeyName'">
        <xsl:call-template name="do-text-bucket">
          <xsl:with-param name="LaTeX-run-nom" select="'\QTR{KeyName}{'"/>
        </xsl:call-template>
      </xsl:when>
      <xsl:when test="@class='MenuDialog'">
        <xsl:call-template name="do-text-bucket">
          <xsl:with-param name="LaTeX-run-nom" select="'\QTR{MenuDialog}{'"/>
        </xsl:call-template>
      </xsl:when>

      <xsl:when test="@class='vspace'">
        <xsl:choose>
          <xsl:when test="substring-before(@style,':')='height'">
<tagged-text>
            <xsl:text>\vspace{</xsl:text>
			  <xsl:value-of select="substring-after(@style,'height:')"/>
            <xsl:text>}</xsl:text>
</tagged-text>
          </xsl:when>
          <xsl:otherwise>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>

      <xsl:when test="@style='width:1em'">
<tagged-text>
        <xsl:text xml:space="preserve">\quad </xsl:text>
</tagged-text>
      </xsl:when>
      <xsl:when test="@style='width:2em'">
<tagged-text>
        <xsl:text xml:space="preserve">\qquad </xsl:text>
</tagged-text>
      </xsl:when>
      <xsl:otherwise>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>


<!--
\verb|\section*|  ->  <msi:code>\section*</msi:code>
\texttt{text}     ->  <msi:code>text</msi:code>
The \texttt{\TEXTsymbol{\backslash}text}  ->  The <msi:code>\text</msi:code>
-->
  <xsl:template match="msi:code">
    <xsl:choose>
      <xsl:when test="@class='tt'">
<tagged-text>
        <xsl:call-template name="do-text-bucket">
          <xsl:with-param name="LaTeX-run-nom" select="'\texttt{'"/>
        </xsl:call-template>
</tagged-text>
      </xsl:when>
      <xsl:when test="@class='verb'">
<tagged-text>
        <xsl:text>\verb|</xsl:text>
          <xsl:value-of select="string(.)"/>
        <xsl:text>|</xsl:text>
</tagged-text>
      </xsl:when>
      <xsl:otherwise>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>


<!-- ENTITY % img "(#PCDATA)"  ATTLIST  img  src ID #IMPLIED, alt ID #IMPLIED -->
  <xsl:template match="msi:img">
  </xsl:template>


<!-- ENTITY % hr EMPTY  ATTLIST  hr class ID #REQUIRED -->

  <xsl:template match="msi:hr">
<hr>
    <xsl:choose>
      <xsl:when test="@class='hrulefill'">
        <xsl:text xml:space="preserve">\hrulefill </xsl:text>
      </xsl:when>

      <xsl:otherwise>
      </xsl:otherwise>
    </xsl:choose>
</hr>
  </xsl:template>



  <xsl:template match="msi:br">
<br>
    <xsl:text xml:space="preserve">\smallskip </xsl:text>
</br>
  </xsl:template>



  <xsl:template match="msi:pre">
<pre>
    <xsl:text>\begin{verbatim}</xsl:text>
      <xsl:value-of select="string(.)"/>
      <!-- xsl:text>&lt;EOLN/&gt;</xsl:text -->
    <xsl:text>&lt;EOLN/&gt;\end{verbatim}</xsl:text>
</pre>
  </xsl:template>



  <xsl:template match="msi:div">
    <xsl:if test="string-length(@class)&gt;0">
      <xsl:text>\begin{</xsl:text>
        <xsl:value-of select="@class"/>
      <xsl:text>}</xsl:text>
    </xsl:if>

    <xsl:if test="./*[1][self::msi:span][@class='leadin']">
      <xsl:variable name="JBM">
        <xsl:value-of select="./*[1][self::msi:span][@class]"/>
      </xsl:variable>
      <xsl:if test="contains($JBM,'(')
      and           contains(substring-after($JBM,'('),')')">
        <xsl:text>[</xsl:text>
          <xsl:value-of select="substring-before(substring-after($JBM,'('),')')"/>
        <xsl:text>]</xsl:text>
      </xsl:if>
    </xsl:if>

    <xsl:text>&lt;EOLN/&gt;</xsl:text>

<xsl:apply-templates/>

    <xsl:if test="string-length(@class)&gt;0">
      <xsl:text>\end{</xsl:text>
        <xsl:value-of select="@class"/>
      <xsl:text>}&lt;EOLN/&gt;</xsl:text>
    </xsl:if>

  </xsl:template>



  <xsl:template match="msi:table">
<stream-with-break-tokens>

    <xsl:variable name="column-counts.tr">
      <xsl:call-template name="column-counter"/>
    </xsl:variable>
    <xsl:variable name="column-counts" select="exsl:node-set($column-counts.tr)"/>


    <xsl:variable name="rowlines">
      <xsl:text></xsl:text>
    </xsl:variable>

    <xsl:text>\begin{tabular}</xsl:text>
      <xsl:if test="@align='top'">
        <xsl:text xml:space="preserve">[t]</xsl:text>
      </xsl:if>
      <xsl:if test="@align='bottom'">
        <xsl:text xml:space="preserve">[b]</xsl:text>
      </xsl:if>


      <xsl:text xml:space="preserve">{</xsl:text>
      <xsl:if test="@frame='solid'">
        <xsl:text xml:space="preserve">|</xsl:text>
      </xsl:if>

      <xsl:call-template name="do-cols">
        <xsl:with-param name="columns-to-do" select="$column-counts/ncols[position()=last()]"/>
        <!-- xsl:with-param name="columnlines" select="$columnlines"/ -->
        <!-- xsl:with-param name="columnalign" select="$columnalign"/ -->
      </xsl:call-template>


      <xsl:if test="@frame='solid'">
        <xsl:text xml:space="preserve">|</xsl:text>
      </xsl:if>
      <xsl:text xml:space="preserve">}</xsl:text>
      <!-- JCS <xsl:text xml:space="preserve">\LBe </xsl:text> -->


      <xsl:if test="@frame='solid'">
        <!-- JCS  <xsl:text xml:space="preserve">\hline\LBe</xsl:text> -->
        <xsl:text xml:space="preserve">\hline </xsl:text>
      </xsl:if>

<!-- Loop thru first level children - msi:thead | msi:tbody | msi:tfoot -->

      <xsl:for-each select="*[1]">
        <xsl:call-template name="do-table-block">
          <xsl:with-param name="rows-done" select="0"/>
          <xsl:with-param name="nrows-in-table" select="count($column-counts/ncols)"/>
        </xsl:call-template>
      </xsl:for-each>


      <xsl:if test="@frame='solid'">
        <!-- JCS <xsl:text xml:space="preserve">\\ \hline\LBe</xsl:text> -->
        <xsl:text xml:space="preserve">\\ \hline 
</xsl:text>
      </xsl:if>


    <xsl:text>\end{tabular}</xsl:text>

</stream-with-break-tokens>

  </xsl:template>



  <xsl:template name="column-counter">
    <xsl:variable name="column-counts.tr">
      <xsl:for-each select="msi:thead|msi:tbody|msi:tfoot">
        <xsl:for-each select="*">
        <n-columns>
          <xsl:choose>
            <xsl:when test="self::msi:tr">
              <xsl:value-of select="count(*)"/>
            </xsl:when>
            <xsl:otherwise>
              <xsl:value-of select="1"/>
            </xsl:otherwise>
          </xsl:choose>
        </n-columns>
        </xsl:for-each>
      </xsl:for-each>
    </xsl:variable>
    <xsl:variable name="column-counts" select="exsl:node-set($column-counts.tr)"/>

    <xsl:for-each select="$column-counts/*">
      <xsl:sort select="."/>
      <ncols>
        <xsl:value-of select="."/>
   	  </ncols>
    </xsl:for-each>

  </xsl:template>


<!-- Recursive template to loop thru first level children of msi:table -
     msi:thead | msi:tbody | msi:tfoot -->

  <xsl:template name="do-table-block">
    <xsl:param name="rows-done"/>
    <xsl:param name="nrows-in-table"/>
    <xsl:if test="self::msi:thead|self::msi:tbody|self::msi:tfoot">

<!-- Loop thru first level children - msi:tr -->

      <xsl:for-each select="*">

        <xsl:if test="self::msi:tr">
          <xsl:for-each select="*">
            <xsl:choose>
              <xsl:when test="self::msi:td">

				<xsl:if test="@columnspan&gt;1">
                  <xsl:text>\multicolumn{</xsl:text>
                  <xsl:value-of select="@columnspan"/>
                  <xsl:text>}{</xsl:text>
                  <xsl:text>c</xsl:text>
                  <xsl:text>}{</xsl:text>
				</xsl:if>

                <xsl:call-template name="do-text-bucket">
                  <xsl:with-param name="LaTeX-run-nom" select="''"/>
                </xsl:call-template>

				<xsl:if test="@columnspan&gt;1">
                  <xsl:text>}</xsl:text>
				</xsl:if>
              </xsl:when>

              <xsl:otherwise>
                <xsl:call-template name="do-text-bucket">
                  <xsl:with-param name="LaTeX-run-nom" select="''"/>
                </xsl:call-template>
              </xsl:otherwise>
            </xsl:choose>

            <xsl:if test="position() != last()">
              <xsl:text xml:space="preserve"> &amp; </xsl:text>
            </xsl:if>

          </xsl:for-each>
        </xsl:if>

<!-- At this point, the row has been scripted. Append \\ as required -->

        <xsl:call-template name="end-table-row">
          <xsl:with-param name="current-row" select="position()+$rows-done"/>
          <xsl:with-param name="last-row"    select="$nrows-in-table"/>
          <xsl:with-param name="rowlines"    select="''"/>
        </xsl:call-template>

      </xsl:for-each>

    </xsl:if>


    <xsl:if test="following-sibling::*[1]">
	  <xsl:variable name="rows-in-curr-block">
	    <xsl:value-of select="count(./msi:tr)"/>
	  </xsl:variable>

      <xsl:for-each select="following-sibling::*[1]">
        <xsl:call-template name="do-table-block">
          <xsl:with-param name="rows-done" select="$rows-done+$rows-in-curr-block"/>
          <xsl:with-param name="nrows-in-table" select="$nrows-in-table"/>
        </xsl:call-template>
      </xsl:for-each>
    </xsl:if>

  </xsl:template>


</xsl:stylesheet>

