<?xml version="1.0"?>
<xsl:stylesheet 
      xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
      xmlns:exsl="http://exslt.org/common"
      xmlns:mml="http://www.w3.org/1998/Math/MathML"
      version="1.1">

<!--
 The contents of an mtext is unicode CDATA.
 Each character must be mapped to LaTeX text ( which is ASCII ).

Attributes

Name           values                                                           default 
mathvariant    normal | bold | italic | bold-italic | double-struck | bold-fraktur | 
               script | bold-script | fraktur | sans-serif | bold-sans-serif | 
               sans-serif-italic | sans-serif-bold-italic | monospace           normal (except on <mi>) 
mathsize       small | normal  | big | number v-unit                            inherited 
mathcolor      #rgb  | #rrggbb | html-color-name                                inherited 
mathbackground #rgb  | #rrggbb | html-color-name                                inherited 

From the MathML 1.0 spec

    3.2.1 Attributes common to token elements
    Several attributes related to text formatting are provided on all token elements
     except <mspace/>, but on no other elements except <mstyle>. These are: 
    
     Name        values                            default  
     fontsize    number v-unit                     inherited 
     fontweight  normal | bold                     inherited 
     fontstyle   normal | italic                   normal (except on <mi>) 
     fontfamily  string | css-fontfamily           inherited 
     color       #rgb | #rrggbb | html-color-name  inherited 


This script handles the following attributes:

mathvariant - treated here as non-inheriting
mathsize (fontsize) - inheriting
fontweight  - inheriting
fontstyle   - inheriting
-->
  <xsl:template match="mml:mtext">
    <xsl:variable name="content.tr">
       <xsl:apply-templates/>
    </xsl:variable>
    <xsl:variable name="content">
      <xsl:value-of select="$content.tr"/>
    </xsl:variable>
    <!-- xsl:call-template name="mtext-a">
     <xsl:with-param name="content" select="$content"/>
    </xsl:call-template -->
    <xsl:value-of select="$content"/>        
  </xsl:template>

  <xsl:template name="mtext-a">
    <xsl:param name="content"/>

    <xsl:variable name="LaTeX-contents">
      <xsl:call-template name="do-chars-in-TEXT">
        <xsl:with-param name="unicode-cdata" select="$content"/>
      </xsl:call-template>
    </xsl:variable>

<!-- MathPlayer treats "mathvariant" as an inheriting attribute - I don't -->

    <xsl:variable name="LaTeX-style-from-mathvariant">
      <xsl:call-template name="get-LaTeX-style-from-mathvariant"/>
    </xsl:variable>

<!-- Note that we don't handle inherited size here - 
     it was handled when ancestral mstyles were encountered.  -->

    <xsl:variable name="LaTeX-size-switch">
      <xsl:call-template name="get-LaTeX-size-from-attribs"/>
    </xsl:variable>

<!-- Inheriting attributes -->

    <xsl:variable name="LaTeX-fontstyle-switch">
	  <xsl:choose>
        <xsl:when test="@fontstyle">
          <xsl:value-of select="@fontstyle"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:call-template name="get-inherited-attr-value">
            <xsl:with-param name="attr-nom" select="'fontstyle'"/>
          </xsl:call-template>
	    </xsl:otherwise>
	  </xsl:choose>
    </xsl:variable>

    <xsl:variable name="LaTeX-fontweight-switch">
	  <xsl:choose>
        <xsl:when test="@fontweight">
          <xsl:value-of select="@fontweight"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:call-template name="get-inherited-attr-value">
            <xsl:with-param name="attr-nom" select="'fontweight'"/>
          </xsl:call-template>
	    </xsl:otherwise>
	  </xsl:choose>
    </xsl:variable>

<!-- Variable to select a LaTeX tagging cmd to match the MathML attributes
     that control the rendering of this mtext. -->

    <xsl:variable name="LaTeX-tagging-cmd">
      <xsl:choose>
        <xsl:when test="string-length($LaTeX-style-from-mathvariant)&gt;0">
          <xsl:value-of select="$LaTeX-style-from-mathvariant"/>
        </xsl:when>
        <xsl:when test="string-length($LaTeX-fontweight-switch)&gt;0">
          <xsl:if test="$LaTeX-fontweight-switch='bold'">
            <xsl:text>\textbf{</xsl:text>
          </xsl:if>
        </xsl:when>
        <xsl:when test="string-length($LaTeX-fontstyle-switch)&gt;0">
          <xsl:if test="$LaTeX-fontstyle-switch='italic'">
            <xsl:text>\textit{</xsl:text>
          </xsl:if>
        </xsl:when>
        <xsl:otherwise>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>


<!-- Generate SWP format   \text{\Large \textbf{..}} -->

    <xsl:text>\text{</xsl:text>

      <xsl:if test="string-length($LaTeX-size-switch)&gt;0">
        <xsl:text>\</xsl:text>
        <xsl:value-of select="$LaTeX-size-switch"/>
        <xsl:text> </xsl:text>
      </xsl:if>

      <xsl:choose>
        <xsl:when test="string-length($LaTeX-tagging-cmd)&gt;0">
          <xsl:value-of select="$LaTeX-tagging-cmd"/>
          <xsl:call-template name="remove-dollar-dollar">
            <xsl:with-param name="LaTeX-zstr" select="$LaTeX-contents"/>
          </xsl:call-template>
          <xsl:text>}</xsl:text>
        </xsl:when>
        <xsl:otherwise>
          <xsl:call-template name="remove-dollar-dollar">
            <xsl:with-param name="LaTeX-zstr" select="$LaTeX-contents"/>
          </xsl:call-template>
        </xsl:otherwise>
      </xsl:choose>

    <xsl:text>}</xsl:text>

  </xsl:template>



  <xsl:template match="mml:mtext" mode="in-text">
  
    <xsl:variable name="LaTeX-contents">
      <xsl:call-template name="do-chars-in-TEXT">
        <xsl:with-param name="unicode-cdata" select="normalize-space(string())"/>
      </xsl:call-template>
    </xsl:variable>

<!-- Note that we don't handle inherited size here - 
     it was handled as set at the higher level.  -->

    <xsl:variable name="LaTeX-size-switch">
      <xsl:call-template name="get-LaTeX-size-from-attribs"/>
    </xsl:variable>

    <xsl:variable name="LaTeX-fontstyle-switch">
	  <xsl:choose>
        <xsl:when test="@fontstyle">
          <xsl:value-of select="@fontstyle"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:call-template name="get-inherited-attr-value">
            <xsl:with-param name="attr-nom" select="'fontstyle'"/>
          </xsl:call-template>
	    </xsl:otherwise>
	  </xsl:choose>
    </xsl:variable>

    <xsl:variable name="LaTeX-fontweight-switch">
	  <xsl:choose>
        <xsl:when test="@fontweight">
          <xsl:value-of select="@fontweight"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:call-template name="get-inherited-attr-value">
            <xsl:with-param name="attr-nom" select="'fontweight'"/>
          </xsl:call-template>
	    </xsl:otherwise>
	  </xsl:choose>
    </xsl:variable>

    <xsl:variable name="LaTeX-style-from-mathvariant">
      <xsl:call-template name="get-LaTeX-style-from-mathvariant"/>
    </xsl:variable>

    <xsl:variable name="LaTeX-tagging-cmd">
      <xsl:choose>
        <xsl:when test="string-length($LaTeX-style-from-mathvariant)&gt;0">
          <xsl:value-of select="$LaTeX-style-from-mathvariant"/>
        </xsl:when>
        <xsl:when test="string-length($LaTeX-fontweight-switch)&gt;0">
          <xsl:if test="$LaTeX-fontweight-switch='bold'">
            <xsl:text>\textbf{</xsl:text>
          </xsl:if>
        </xsl:when>
        <xsl:when test="string-length($LaTeX-fontstyle-switch)&gt;0">
          <xsl:if test="$LaTeX-fontstyle-switch='italic'">
            <xsl:text>\textit{</xsl:text>
          </xsl:if>
        </xsl:when>
        <xsl:otherwise>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

<!-- Generate SWP format   \Large \textbf{..} -->

    <xsl:if test="string-length($LaTeX-size-switch)&gt;0">
      <xsl:text>\</xsl:text>
      <xsl:value-of select="$LaTeX-size-switch"/>
      <xsl:text> </xsl:text>
    </xsl:if>

    <xsl:choose>
      <xsl:when test="string-length($LaTeX-tagging-cmd)">
        <xsl:value-of select="$LaTeX-tagging-cmd"/>
        <xsl:call-template name="remove-dollar-dollar">
          <xsl:with-param name="LaTeX-zstr" select="$LaTeX-contents"/>
        </xsl:call-template>
        <xsl:text>}</xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:call-template name="remove-dollar-dollar">
          <xsl:with-param name="LaTeX-zstr" select="$LaTeX-contents"/>
        </xsl:call-template>
      </xsl:otherwise>
    </xsl:choose>

  </xsl:template>





<!-- LaTeX text can be bold, italic, etc.  -->

  <xsl:template name="get-LaTeX-style-from-mathvariant">
  
    <xsl:choose>

      <xsl:when test="@mathvariant">

        <xsl:variable name="mathvariant-value">
          <xsl:value-of select="@mathvariant"/>
        </xsl:variable>

<!--
\textup<uID5.450.1>!\textup!REQPARAM(5.450.2,TEXT)
\textit<uID5.451.1>!\textit!REQPARAM(5.451.2,TEXT)
\textsl<uID5.452.1>!\textsl!REQPARAM(5.452.2,TEXT)
\textsc<uID5.453.1>!\textsc!REQPARAM(5.453.2,TEXT)

\textmd<uID5.454.1>!\textmd!REQPARAM(5.454.2,TEXT)
\textbf<uID5.455.1>!\textbf!REQPARAM(5.455.2,TEXT)

\textrm<uID5.456.1>!\textrm!REQPARAM(5.456.2,TEXT)
\textsf<uID5.457.1>!\textsf!REQPARAM(5.457.2,TEXT)
\texttt<uID5.458.1>!\texttt!REQPARAM(5.458.2,TEXT)

\emph<uID5.459.1>!\emph!REQPARAM(5.459.2,INHERIT)
\textnormal<uID5.460.1>!\textnormal!REQPARAM(5.460.2,TEXT)

\textcircled<uID5.461.1>!\textcircled!REQPARAM(5.461.2,TEXT)
-->
        <xsl:choose>
          <xsl:when test="$mathvariant-value='normal'">
          </xsl:when>
          <xsl:when test="$mathvariant-value='bold'">
            <xsl:text>\textbf{</xsl:text>
          </xsl:when>
          <xsl:when test="$mathvariant-value='italic'">
            <xsl:text>\textit{</xsl:text>
          </xsl:when>
          <xsl:when test="$mathvariant-value='bold-italic'">
          </xsl:when>
          <xsl:when test="$mathvariant-value='double-struck'">
          </xsl:when>
          <xsl:when test="$mathvariant-value='bold-fraktur'">
          </xsl:when>
          <xsl:when test="$mathvariant-value='script'">
          </xsl:when>
          <xsl:when test="$mathvariant-value='bold-script'">
          </xsl:when>
          <xsl:when test="$mathvariant-value='fraktur'">
          </xsl:when>
          <xsl:when test="$mathvariant-value='sans-serif'">
            <xsl:text>\textsf{</xsl:text>
          </xsl:when>
          <xsl:when test="$mathvariant-value='bold-sans-serif'">
          </xsl:when>
          <xsl:when test="$mathvariant-value='sans-serif-italic'">
          </xsl:when>
          <xsl:when test="$mathvariant-value='sans-serif-bold-italic'">
          </xsl:when>
          <xsl:when test="$mathvariant-value='monospace'">
            <xsl:text>\texttt{</xsl:text>
          </xsl:when>
          <xsl:otherwise>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>

      <xsl:otherwise>
      </xsl:otherwise>

    </xsl:choose>

  </xsl:template>


</xsl:stylesheet>

