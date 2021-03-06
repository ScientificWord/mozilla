<?xml version="1.0"?>
<xsl:stylesheet 
      xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
      xmlns:exsl="http://exslt.org/common"
      xmlns:mml="http://www.w3.org/1998/Math/MathML"
      version="1.1">





  <xsl:template name="check-munderover-decoration">
      <xsl:choose>
      <xsl:when test="./*[2][normalize-space(string())='&#x0332;']">
        <xsl:text>true</xsl:text>
      </xsl:when>
      <xsl:when test="./*[2][normalize-space(string())='&#x2190;']">
        <xsl:text>true</xsl:text>
      </xsl:when>
      <xsl:when test="./*[2][normalize-space(string())='&#x2192;']">
        <xsl:text>true</xsl:text>
      </xsl:when>
      <xsl:when test="./*[2][normalize-space(string())='&#x2194;']">
        <xsl:text>true</xsl:text>
      </xsl:when>
      <xsl:when test="./*[2][normalize-space(string())='&#xF613;']">
        <xsl:text>true</xsl:text>
      </xsl:when>
      <xsl:when test="./*[2][normalize-space(string())='&#x23DF;']">
        <xsl:text>true</xsl:text>
      </xsl:when>
      <xsl:when test="./*[2][normalize-space(string())='&#xFE38;']">
        <xsl:text>true</xsl:text>
      </xsl:when>
     <xsl:when test="./*[2][normalize-space(string())='&#x00AF;']">
        <xsl:text>true</xsl:text>
      </xsl:when>
      <xsl:when test="./*[2][normalize-space(string())='&#x2190;']">
        <xsl:text>true</xsl:text>
      </xsl:when>
      <xsl:when test="./*[2][normalize-space(string())='&#x2192;']">
        <xsl:text>true</xsl:text>
      </xsl:when>
      <xsl:when test="./*[2][normalize-space(string())='&#x2194;']">
        <xsl:text>true</xsl:text>
      </xsl:when>
      <xsl:when test="./*[3][normalize-space(string())='&#xF612;']">
        <xsl:text>true</xsl:text>
      </xsl:when>
      <xsl:when test="./*[3][normalize-space(string())='&#x23DE;']
      or              ./*[3][normalize-space(string())='&#xFE37;']">
        <xsl:text>true</xsl:text>
      </xsl:when>
      <xsl:when test="./*[3][normalize-space(string())='&#x0302;']
      and             ./*[3][@stretchy='true']">
        <xsl:text>true</xsl:text>
      </xsl:when>
      <xsl:when test="./*[3][normalize-space(string())='&#xFE3F;']
      and             ./*[3][@stretchy='true']">
        <xsl:text>true</xsl:text>
      </xsl:when>
      <xsl:when test="./*[3][normalize-space(string())='&#x02DC;']
      and             ./*[3][@stretchy='true']">
        <xsl:text>true</xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text>false</xsl:text>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template >

  
  <xsl:template match="mml:munderover" mode="in-text">
      <xsl:text>$</xsl:text>
    <xsl:apply-templates select="."/>
    <xsl:text>$</xsl:text>
  </xsl:template>

  <xsl:template match="mml:munderover">
  
    <xsl:variable name="munderover-contents.tr">

      
    <is-decoration>
      <xsl:call-template name="check-munderover-decoration"/>
    </is-decoration>

      <big-op-char>
        <xsl:call-template name="is-LaTeX-bigop"/>
      </big-op-char>

<!--
// \xleftarrow<uID5.16.1>!\xleftarrow!OPTPARAM(5.17.1,MATH)REQPARAM(5.17.2,MATH)
// \xrightarrow<uID5.16.2>!\xrightarrow!OPTPARAM(5.17.1,MATH)REQPARAM(5.17.2,MATH)
// Translate the xarrow to MathML - a stretchy operator
//  that we will embed in an <munderover> - embellished operator.
// &larr;<uID3.24.1>infix,25,U02190,stretchy="true"
// &rarr;<uID3.24.2>infix,25,U02192,stretchy="true"
A\xleftarrow{n+\mu-1}B \xrightarrow[T]{n\pm i-1}C
-->
      <xarrow-char>
        <xsl:choose>
          <xsl:when test="*[1][self::mml:mo][@stretchy='true'][
              string()='&#x2190;'
          or  string()='&#x2192;'  ]">
            <xsl:value-of select="./*[1]"/>
          </xsl:when>
		  <xsl:otherwise>
            <xsl:value-of select="'false'"/>
		  </xsl:otherwise>
        </xsl:choose>
      </xarrow-char>

      <movablelimits>
        <xsl:choose>
          <xsl:when test="string-length(*[1][@movablelimits]) &gt; 0">
            <xsl:for-each select="*[1][self::mml:mo]">
              <xsl:value-of select="@movablelimits"/>
            </xsl:for-each>
          </xsl:when>
          <xsl:otherwise>
            <xsl:call-template name="get-mo-attr-val">
              <xsl:with-param name="mo-nom"   select="*[1][string()]"/>
              <xsl:with-param name="attr-nom" select="'movablelimits'"/>
            </xsl:call-template>
          </xsl:otherwise>
        </xsl:choose>
      </movablelimits>

    </xsl:variable>
    <xsl:variable name="munderover-contents" select="exsl:node-set($munderover-contents.tr)"/>


    <xsl:variable name="limits">
      <xsl:choose>
        <xsl:when test="$munderover-contents/movablelimits='false'">
          <xsl:text>\limits </xsl:text>
        </xsl:when>
        <xsl:otherwise>
          <xsl:text>false</xsl:text>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>


    <xsl:choose>
      

<!-- the base element is a big operator -->

      <xsl:when test="$munderover-contents/big-op-char!='false'">

<!--
If the base is an operator with movablelimits=true (or an embellished operator
 whose mo element core has movablelimits=true), and displaystyle=false, 
 then underscript is drawn in a subscript position. In this case, the accentunder
 attribute is ignored. This is often used for limits on symbols such as &sum;.
-->

        <xsl:call-template name="do-embellished-bigop">
          <xsl:with-param name="limits-flag" select="$limits"/>
          <xsl:with-param name="j1"          select="'_{'"/>
          <xsl:with-param name="j2"          select="'}^{'"/>
        </xsl:call-template>
      </xsl:when>

<!-- the base element is a stretchy arrow -->

      <xsl:when test="$munderover-contents/xarrow-char!='false'">
        <xsl:choose>
          <xsl:when test="$munderover-contents/xarrow-char='&#x2190;'">
            <xsl:text>\xleftarrow[</xsl:text>
          </xsl:when>
		  <xsl:otherwise>
            <xsl:text>\xrightarrow[</xsl:text>
		  </xsl:otherwise>
        </xsl:choose>
        <xsl:call-template name="do-positional-arg">
          <xsl:with-param name="arg-num" select="2"/>
        </xsl:call-template>
        <xsl:text>]{</xsl:text>
        <xsl:call-template name="do-positional-arg">
          <xsl:with-param name="arg-num" select="3"/>
        </xsl:call-template>
        <xsl:text>}</xsl:text>
      </xsl:when>

      <!-- The object is decorated -->


    <xsl:when test="$munderover-contents/is-decoration='true'">
        <xsl:choose>
          <xsl:when test="./*[2][normalize-space(string())='&#x0332;']"> 
            <xsl:text>\underline{</xsl:text> 
          </xsl:when>
          <xsl:when test="./*[2][normalize-space(string())='&#x2190;']">
            <xsl:text> \underleftarrow{</xsl:text>             
          </xsl:when>
          <xsl:when test="./*[2][normalize-space(string())='&#x2192;']">
            <xsl:text> \underleftarrow{</xsl:text>
          </xsl:when>
          <xsl:when test="./*[2][normalize-space(string())='&#x2194;']">
            <xsl:text> \underleftrightarrow{</xsl:text>
          </xsl:when>
          <xsl:when test="./*[2][normalize-space(string())='&#x23DF;']
                    or    ./*[2][normalize-space(string())='&#xFE38;']">           
            <xsl:text>\underbrace{</xsl:text>
          </xsl:when>                  
          <xsl:otherwise>
            <!-- Still need to cover \underset case -->
          </xsl:otherwise>         
        </xsl:choose>
        <xsl:choose>
          <xsl:when test="./*[3][normalize-space(string())='&#x00AF;']">
            <xsl:call-template name="math-accent">
              <xsl:with-param name="LaTeX-acc" select="'\overline'"/>
            </xsl:call-template>
            <xsl:text>}</xsl:text>
          </xsl:when>
          <xsl:when test="./*[3][normalize-space(string())='&#x2190;']">
            <xsl:call-template name="math-accent">
              <xsl:with-param name="LaTeX-acc" select="'\overleftarrow'"/>
            </xsl:call-template>
            <xsl:text>}</xsl:text>
          </xsl:when>
          <xsl:when test="./*[3][normalize-space(string())='&#x2192;']">
            <xsl:call-template name="math-accent">
              <xsl:with-param name="LaTeX-acc" select="'\overrightarrow'"/>
            </xsl:call-template>
            <xsl:text>}</xsl:text>
          </xsl:when>
          <xsl:when test="./*[3][normalize-space(string())='&#x2194;']">
            <xsl:call-template name="math-accent">
              <xsl:with-param name="LaTeX-acc" select="'\overleftrightarrow'"/>
            </xsl:call-template>
            <xsl:text>}</xsl:text>
          </xsl:when>
          <xsl:when test="./*[3][normalize-space(string())='&#x23DE;']
                    or    ./*[3][normalize-space(string())='&#xFE37;']">
            <xsl:call-template name="math-accent">
              <xsl:with-param name="LaTeX-acc" select="'\overbrace'"/>
            </xsl:call-template>
            <xsl:text>}</xsl:text>
          </xsl:when>
          <xsl:when test="./*[3][normalize-space(string())='&#x0302;']">
            <xsl:call-template name="math-accent">
              <xsl:with-param name="LaTeX-acc" select="'\widehat'"/>
            </xsl:call-template>
            <xsl:text>}</xsl:text>
          </xsl:when>
          <xsl:when test="./*[3][normalize-space(string())='&#xFE3F;']">
            <xsl:call-template name="math-accent">
              <xsl:with-param name="LaTeX-acc" select="'\widehat'"/>
            </xsl:call-template>
            <xsl:text>}</xsl:text>
          </xsl:when>
          <xsl:when test="./*[3][normalize-space(string())='&#x02DC;']">
            <xsl:call-template name="math-accent">
              <xsl:with-param name="LaTeX-acc" select="'\widetilde'"/>
            </xsl:call-template>                              
            <xsl:text>}</xsl:text>  <xsl:text>}</xsl:text>
          </xsl:when>
          <xsl:when test="./*[3][normalize-space(string())='~']">
             <xsl:call-template name="math-accent">
               <xsl:with-param name="LaTeX-acc" select="'\widetilde'"/>
             </xsl:call-template>
          <xsl:text>}</xsl:text>
          </xsl:when>
                            
          <xsl:otherwise>
          </xsl:otherwise>     

       </xsl:choose>
      </xsl:when>


<!-- possible \limfunc{} -->

	  <xsl:otherwise>
        <xsl:apply-templates select="*[1]"/>
        <xsl:text>_{</xsl:text>
        <xsl:call-template name="do-positional-arg">
          <xsl:with-param name="arg-num" select="2"/>
        </xsl:call-template>
        <xsl:text>}^{</xsl:text>
        <xsl:call-template name="do-positional-arg">
          <xsl:with-param name="arg-num" select="3"/>
        </xsl:call-template>
        <xsl:text>}</xsl:text>
	  </xsl:otherwise>
    </xsl:choose>

   </xsl:template>

</xsl:stylesheet>



<!--
$\overset{T=6}{x+1}+\underset{t<5}{y+1}$

<mml:mover>
  <mml:mrow>
    <mml:mi>x</mml:mi>
    <mml:mo form="infix">+</mml:mo>
    <mml:mn>1</mml:mn>
  </mml:mrow>
  <mml:mrow>
    <mml:mi>T</mml:mi>
    <mml:mo form="infix">=</mml:mo>
    <mml:mn>6</mml:mn>
  </mml:mrow>
</mml:mover>

<mml:mo form="infix">+</mml:mo>

<mml:munder>
  <mml:mrow>
    <mml:mi>y</mml:mi>
    <mml:mo form="infix">+</mml:mo>
    <mml:mn>1</mml:mn>
  </mml:mrow>
  <mml:mrow>
    <mml:mi>t</mml:mi>
    <mml:mo form="infix">&lt;</mml:mo>
    <mml:mn>5</mml:mn>
  </mml:mrow>
</mml:munder>

-->

