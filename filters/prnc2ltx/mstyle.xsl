<?xml version="1.0"?>
<xsl:stylesheet 
      xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
      xmlns:exsl="http://exslt.org/common"
      xmlns:mml="http://www.w3.org/1998/Math/MathML"
      version="1.1">


<!-- There are some complications re the translation of mstyle to LaTeX.

 The mstyle schemata carries attributes that are inherited by
 it's descendants.  LaTeX doesn't have any such mechanism.

 ( In LaTeX, most style-relating rendering is set the .sty file.
   ie. by the \documentclass{} or \documentstyle{} statement.

   A few rendering attributes could be activated and
   de-activated using the newenvironment mechanism. )

 Some mstyle attributes have rough equivalents in LaTeX.

 <mstyle fontsize="20pt">   ->	 {\Large
	.                                .
	.								 .
 </mstyle>					->	 }

 There are problems with the translation above - for example
 LaTeX math elements are not subject to the \Large switch. 


 Other mstyle attributes can be translated into LaTeX
 as each element (mi,mo,mn,mtext) is translated.  
 The larger tagged object is lost, but correct LaTeX can be
 generated.  For example, mstyle might set a thick fraction
 line on it's descendants.  Descendant mfracs could be
 translated to \genfrac with appropriate line thicknesses.
 In xsl, ancestors and their attributes are easily queried.


 This initial implementation uses LaTeX switches for translating
 mathsize (fontsize).  A few other attributes are handled by 
 checking ancestors as elements are translated.  More work is
 needed if this script is to give a general translation of mstyle
 to LaTeX.




Additionally, mstyle can be given the following special attributes
that are implicitly inherited by every MathML element
as part of its rendering environment: 

Name                    values                             default

displaystyle            true | false                       inherited
 
scriptlevel             ['+' | '-'] unsigned-integer       inherited
scriptsizemultiplier    number                             0.71
scriptminsize           number v-unit                      8pt

color                   #rgb | #rrggbb | html-color-name   inherited
background              #rgb | #rrggbb | transparent       transparent
							 | html-color-name
veryverythinmathspace   number h-unit                      0.0555556em
verythinmathspace       number h-unit                      0.111111em
thinmathspace           number h-unit                      0.166667em
mediummathspace         number h-unit                      0.222222em
thickmathspace          number h-unit                      0.277778em
verythickmathspace      number h-unit                      0.333333em
veryverythickmathspace  number h-unit                      0.388889em

-->

  <xsl:template match="mml:mstyle">
      <xsl:variable name="switch-name">
      <xsl:call-template name="get-LaTeX-size-from-attribs"/>
    </xsl:variable>

    <xsl:variable name="displaystyle">
      <xsl:value-of select="@displaystyle"/>
    </xsl:variable>

    <xsl:variable name="scriptlevel">
      <xsl:value-of select="@scriptlevel"/>
    </xsl:variable>

    <xsl:variable name="n-children">
      <xsl:value-of select="count(./*)"/>
    </xsl:variable>

    <xsl:variable name="first-is-mfrac">
      <xsl:if test="./*[1][self::mml:mfrac]">
        <xsl:text>true</xsl:text>
      </xsl:if>
    </xsl:variable>


    <xsl:choose>

<!-- The LaTeX fractions that we script have "built-in" style attributes -->

      <xsl:when test="$first-is-mfrac='true' and $n-children=1">
        <xsl:apply-templates/>
      </xsl:when>


      <xsl:when test="$displaystyle='true' and $scriptlevel=0">
        <xsl:text xml:space="preserve">{\displaystyle </xsl:text>
          <xsl:apply-templates/>
        <xsl:text>}</xsl:text>
      </xsl:when>
      <xsl:when test="$displaystyle='false' and $scriptlevel=0">
        <xsl:text xml:space="preserve">{\textstyle </xsl:text>
          <xsl:apply-templates/>
        <xsl:text>}</xsl:text>
      </xsl:when>
      <xsl:when test="$displaystyle='false' and $scriptlevel=1">
        <xsl:text xml:space="preserve">{\scriptstyle </xsl:text>
          <xsl:apply-templates/>
        <xsl:text>}</xsl:text>
      </xsl:when>
      <xsl:when test="$displaystyle='false' and $scriptlevel=2">
        <xsl:text xml:space="preserve">{\scriptscriptstyle </xsl:text>
          <xsl:apply-templates/>
        <xsl:text>}</xsl:text>
      </xsl:when>

      <xsl:when test="string-length($switch-name)&gt;0">
        <xsl:text>{\</xsl:text>
        <xsl:value-of select="$switch-name"/>
        <xsl:text xml:space="preserve"> </xsl:text>
          <xsl:apply-templates/>
        <xsl:text>}</xsl:text>
      </xsl:when>
	  <xsl:otherwise>
        <xsl:apply-templates/>
	  </xsl:otherwise>
	</xsl:choose>

  </xsl:template>


  <xsl:template match="mml:mstyle" mode="in-text">
      <xsl:text>$</xsl:text>
    <xsl:apply-templates select="."/>
    <xsl:text>$</xsl:text>
  </xsl:template>



  <xsl:template name="get-LaTeX-size-from-attribs">
  
	<xsl:choose>

      <xsl:when test="string-length(@mathsize)&gt;0">
        <xsl:choose>
<!--
          <xsl:when test="@mathsize='small'">
            <xsl:text>tiny</xsl:text>
          </xsl:when>
          <xsl:when test="@mathsize='small'">
            <xsl:text>scriptsize</xsl:text>
          </xsl:when>
          <xsl:when test="@mathsize='small'">
            <xsl:text>footnotesize</xsl:text>
          </xsl:when>
-->
          <xsl:when test="@mathsize='small'">
            <xsl:text>small</xsl:text>
          </xsl:when>
          <xsl:when test="@mathsize='normal'">
            <xsl:text>normalsize</xsl:text>
          </xsl:when>
          <xsl:when test="@mathsize='large'">
            <xsl:text>large</xsl:text>
          </xsl:when>

          <xsl:otherwise>

            <xsl:variable name="value">
              <xsl:call-template name="get-number-chars">
                <xsl:with-param name="attrib-cdata" select="@mathsize"/>
              </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="unit">
              <xsl:value-of select="substring-after(@mathsize,$value)"/>
            </xsl:variable>
	  
            <xsl:call-template name="MML-attr-to-LaTeX-size">
              <xsl:with-param name="value" select="$value"/>
              <xsl:with-param name="unit"  select="$unit"/>
            </xsl:call-template>

          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>


      <xsl:when test="string-length(@scriptlevel)&gt;0">
	    <xsl:choose>
          <xsl:when test="@scriptlevel='1'">
            <xsl:text>scriptsize</xsl:text>
          </xsl:when>
          <xsl:when test="@scriptlevel='2'">
            <xsl:text>tiny</xsl:text>
          </xsl:when>
          <xsl:otherwise>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>


      <xsl:when test="string-length(@fontsize)&gt;0">

<!-- deprecated in MathML 2.0 -->

        <xsl:variable name="value">
          <xsl:call-template name="get-number-chars">
            <xsl:with-param name="attrib-cdata" select="@fontsize"/>
          </xsl:call-template>
        </xsl:variable>
        <xsl:variable name="unit">
          <xsl:value-of select="substring-after(@fontsize,$value)"/>
        </xsl:variable>
	  
        <xsl:call-template name="MML-attr-to-LaTeX-size">
          <xsl:with-param name="value" select="$value"/>
          <xsl:with-param name="unit"  select="$unit"/>
        </xsl:call-template>

      </xsl:when>

      <xsl:otherwise>
      </xsl:otherwise>
    </xsl:choose>


  </xsl:template>



  <xsl:template name="MML-attr-to-LaTeX-size">
    <xsl:param name="value"/>
    <xsl:param name="unit"/>
  
	<xsl:choose>

      <xsl:when test="$unit='ex'">
        <xsl:choose>
          <xsl:when test="$value &lt; 0.95">
            <xsl:text>tiny</xsl:text>
          </xsl:when>
          <xsl:when test="$value &lt; 1.20">
            <xsl:text>scriptsize</xsl:text>
          </xsl:when>
          <xsl:when test="$value &lt; 1.45">
            <xsl:text>footnotesize</xsl:text>
          </xsl:when>
          <xsl:when test="$value &lt; 1.70">
            <xsl:text>small</xsl:text>
          </xsl:when>
          <xsl:when test="$value &lt; 1.95">
            <xsl:text>normalsize</xsl:text>
          </xsl:when>
          <xsl:when test="$value &lt; 2.20">
            <xsl:text>large</xsl:text>
          </xsl:when>
          <xsl:when test="$value &lt; 2.45">
            <xsl:text>Large</xsl:text>
          </xsl:when>
          <xsl:when test="$value &lt; 2.70">
            <xsl:text>LARGE</xsl:text>
          </xsl:when>
          <xsl:when test="$value &lt; 2.95">
            <xsl:text>huge</xsl:text>
          </xsl:when>
          <xsl:otherwise>
            <xsl:text>Huge</xsl:text>
          </xsl:otherwise>
	    </xsl:choose>
      </xsl:when>

      <xsl:when test="$unit='%'">
        <xsl:choose>
          <xsl:when test="$value &lt; 185">
            <xsl:text>Large</xsl:text>
          </xsl:when>
          <xsl:when test="$value &lt; 210">
            <xsl:text>LARGE</xsl:text>
          </xsl:when>
          <xsl:when test="$value &lt; 240">
            <xsl:text>huge</xsl:text>
          </xsl:when>
          <xsl:otherwise>
            <xsl:text>Huge</xsl:text>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>

      <xsl:when test="$unit='pt'">
        <xsl:choose>
          <xsl:when test="$value &lt; 12">
            <xsl:text></xsl:text>
          </xsl:when>
          <xsl:when test="$value &lt; 17">
            <xsl:text>Large</xsl:text>
          </xsl:when>
          <xsl:when test="$value &lt; 25">
            <xsl:text>LARGE</xsl:text>
          </xsl:when>
          <xsl:when test="$value &lt; 37">
            <xsl:text>huge</xsl:text>
          </xsl:when>
          <xsl:otherwise>
            <xsl:text>Huge</xsl:text>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>

      <xsl:otherwise>
		      </xsl:otherwise>
	</xsl:choose>

  </xsl:template>


</xsl:stylesheet>

