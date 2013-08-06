<?xml version="1.0"?>
<xsl:stylesheet 
      xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
      xmlns:exsl="http://exslt.org/common"
      xmlns:mml="http://www.w3.org/1998/Math/MathML"
       version="1.1">

  <xsl:variable name="output-mode">
    <xsl:text>Portable-LaTeX</xsl:text>
<!--
    <xsl:text>SW-LaTeX</xsl:text>
-->
  </xsl:variable>


<!-- 
  The following variable controls translation of unrecogized text unicodes.
  They can be scripted as black boxes or \U{}'s - For Koh.
-->

  <xsl:variable name="script-unicodes">
    <xsl:text>true</xsl:text>
<!--
    <xsl:text>false</xsl:text>
-->
  </xsl:variable>


  <xsl:template match="mml:math">
       <xsl:variable name="the-math">
        <xsl:choose>
           <xsl:when test="count(*)=1">
               <sw-domath><xsl:copy-of select="@*"/><xsl:copy-of select="*"/></sw-domath>
           </xsl:when>

           <xsl:otherwise>
             <xsl:choose>
               <xsl:when test="@display='block'">
                 <sw-domath display="block"><mml:mrow><xsl:copy-of select="*"/></mml:mrow></sw-domath>
               </xsl:when>
               <xsl:otherwise>
                 <sw-domath><mml:mrow><xsl:copy-of select="@*"/><xsl:copy-of select="*"/></mml:mrow></sw-domath>
               </xsl:otherwise>
             </xsl:choose>
           </xsl:otherwise>
        </xsl:choose>
     </xsl:variable>
     <xsl:apply-templates select="exsl:node-set($the-math)"/>
  </xsl:template>

  <xsl:template match="mml:menclose">
    <xsl:choose>
      <xsl:when test="@notation='box'">
        \fbox{<xsl:apply-templates/>}
      </xsl:when>
      <xsl:otherwise/>
    </xsl:choose>
  </xsl:template>


  <xsl:template match="sw-domath">
      
  <!-- Record some info about the top level structure of the math -->

     <xsl:variable name="math-structure.tr">

      <!-- mml:math is expected to contain 1 child??? -->

	    <xsl:choose>
        <xsl:when test="count(*)=1 and ./*[1][self::mml:mrow]">
	        <xsl:choose>

            <!-- if structure is math/mrow/mstyle, examine objects within the mstyle -->

	          <xsl:when test="./mml:mrow[1]/*[1][self::mml:mstyle] and count(./mml:mrow[1]/*)=1">
  
              <xsl:for-each select="./*[1]">
                <xsl:for-each select="./*[1]">
		              <displaystyle>
                    <xsl:value-of select="@displaystyle"/>
                  </displaystyle>
		              <n-children>
                    <xsl:value-of select="count(*)"/>
                  </n-children>
                  <xsl:if test="./*[1][self::mml:mtable]">
                    <first-is-table>
                      <xsl:text>true</xsl:text>
                    </first-is-table>
                  </xsl:if>
                </xsl:for-each>
              </xsl:for-each>
	          </xsl:when>

	          <xsl:otherwise>
<!-- if math/mrow look at objects within the mrow -->
                <displaystyle>
                <xsl:text>false</xsl:text>
              </displaystyle>
			        <n-children>
                <xsl:value-of select="count(./*[1]/*)"/>
              </n-children>
              <xsl:if test="./mml:mrow[1]/*[1][self::mml:mtable]">
                <first-is-table>
                  <xsl:text>true</xsl:text>
                </first-is-table>
              </xsl:if>
		        </xsl:otherwise>
		      </xsl:choose>
	      </xsl:when>

        <xsl:otherwise>
<!-- if math/1 container object,  look at the object -->
  		      <displaystyle>
                <xsl:text>false</xsl:text>
          </displaystyle>
		      <n-children>
              <xsl:value-of select="count(*)"/>
          </n-children>
      
          <xsl:if test="./*[1][self::mml:mtable]">
              <first-is-table>
                  <xsl:text>true</xsl:text>
              </first-is-table>
          </xsl:if>
		    </xsl:otherwise>
	    </xsl:choose>
    </xsl:variable>
    <xsl:variable name="math-structure" select="exsl:node-set($math-structure.tr)"/>

    <!-- Decide if inline OR display.  The <mml:math> tag should contain
         contain a "display" tag with a value of 'inline' or 'block' -->

    <xsl:variable name="is-display.tr">
    <!-- <xsl:message>Finding is-display for node <xsl:value-of select="name()"/>, display is "{@display}<xsl:value-of select="@display"/></xsl:message> -->
      <xsl:choose>

	      <xsl:when test="@display='inline'">
          <xsl:text>false</xsl:text>
	      </xsl:when>

	      <xsl:when test="@display='block'">
          <xsl:text>true</xsl:text>
          <xsl:message>Setting is-display to true</xsl:message>
	      </xsl:when>
        
        <!-- mode attrib deprecated in MathML 2.0 -->
	      <xsl:when test="@mode='inline'">
          <xsl:text>false</xsl:text>
	      </xsl:when>
	      
	      <xsl:when test="@mode='display'">
          <xsl:text>true</xsl:text>
	      </xsl:when>
	      
	      <xsl:otherwise>
          <xsl:choose>
            <xsl:when test="string-length(math-structure/displaystyle)&gt;0">
              <xsl:value-of select="$math-structure/displaystyle"/>
            </xsl:when>
	          
	          <xsl:otherwise>
              <xsl:text>false</xsl:text>
	          </xsl:otherwise>
          </xsl:choose>
	      </xsl:otherwise>
	    </xsl:choose>
    </xsl:variable>
    <xsl:variable name="is-display" select="$is-display.tr"/>

<!--
<xsl:text>AAA-</xsl:text>
<xsl:value-of select="$math-structure/displaystyle"/>
<xsl:text>,</xsl:text>
<xsl:value-of select="$math-structure/n-children"/>
<xsl:text>,</xsl:text>
<xsl:value-of select="$math-structure/first-is-table"/>
<xsl:text>-BBB</xsl:text>
-->

<stream-with-break-tokens>
    <xsl:choose>
<!-- JCS 
      <xsl:when test="count(*)&gt;1">
        <xsl:text>ERROR? - multiple first level children in math.</xsl:text>
      </xsl:when>
-->

<!-- Handle display MATH -->

      <xsl:when test="$is-display='true'">
        <xsl:choose>

          <xsl:when test="$math-structure/n-children=0">
              <xsl:text xml:space="preserve">\[&#xA; \]</xsl:text>
          </xsl:when>

          <xsl:when test="$math-structure/n-children=1">
  <!--   single object in the display -->

            <xsl:choose>

              <xsl:when test="$math-structure/first-is-table='true'">

<!--     the single object is a displayed table -->

<!-- could be a displayed equation, an eqnarray, an array, a \frame{}, etc. -->

                <xsl:variable name="table-structure.tr">
                  <xsl:for-each select="./*[1][self::mml:mrow]">
<!-- if math/mrow/mstyle look at objects within the mstyle -->
                    <xsl:choose>
	                    <xsl:when test="./*[1][self::mml:mstyle]">
                          <xsl:for-each select="./*[1][self::mml:mstyle]">
                            <xsl:for-each select="./*[1][self::mml:mtable]">

		  	  	  	              <n-mtr>
                                <xsl:value-of select="count(mml:mtr)"/>
                              </n-mtr>

		  	  	  	              <n-mlabeledtr>
                                <xsl:value-of select="count(mml:mlabeledtr)"/>
                              </n-mlabeledtr>

                              <xsl:for-each select="mml:mtr|mml:mlabeledtr">
                                <xsl:sort select="count(mml:mtd)"/>
		  	  	  	                  <n-cells>
                                    <xsl:value-of select="count(mml:mtd)"/>
                                  </n-cells>
                              </xsl:for-each>

                              <xsl:for-each select="mml:mtr|mml:mlabeledtr">
                                <xsl:sort select="count(./mml:mtd/mml:mrow/mml:maligngroup)"/>
    	                            <n-aligns>
                                    <xsl:value-of select="count(./mml:mtd/mml:mrow/mml:maligngroup)"/>
                                  </n-aligns>
                              </xsl:for-each>
                            </xsl:for-each>
                          </xsl:for-each>
	                    </xsl:when>
	                    
	                    <xsl:otherwise>
<!-- if math/mrow look at objects within the mrow -->
                        <xsl:for-each select="./*[1][self::mml:mtable]">

	  	  	  	            <n-mtr>
                            <xsl:value-of select="count(mml:mtr)"/>
                          </n-mtr>

	  	  	  	            <n-mlabeledtr>
                            <xsl:value-of select="count(mml:mlabeledtr)"/>
                          </n-mlabeledtr>

                          <xsl:for-each select="mml:mtr|mml:mlabeledtr">
                            <xsl:sort select="count(mml:mtd)"/>
	  	  	  	              <n-cells>
                              <xsl:value-of select="count(mml:mtd)"/>
                            </n-cells>
                          </xsl:for-each>

                          <xsl:for-each select="mml:mtr|mml:mlabeledtr">
                            <xsl:sort select="count(./mml:mtd/mml:mrow/mml:maligngroup)"/>
							                <n-aligns>
                                <xsl:value-of select="count(./mml:mtd/mml:mrow/mml:maligngroup)"/>
                              </n-aligns>
                          </xsl:for-each>

                        </xsl:for-each>
	                    </xsl:otherwise>
                    </xsl:choose>

                  </xsl:for-each>
                </xsl:variable>


                <xsl:variable name="table-structure" select="exsl:node-set($table-structure.tr)"/>
                <xsl:choose>
<!-- mtables with 1 cell per row -->

                  <xsl:when test="$table-structure/n-cells[position()=last()]='1'">

                    <xsl:choose>
<!-- mtable, 1 column, single row -->
                      <xsl:when test="$table-structure/n-mtr+$table-structure/n-mlabeledtr=1">
                        <xsl:choose>
                          <xsl:when test="$table-structure/n-mlabeledtr=1">
<!--   labeled - \begin{equation} -->
                            <xsl:apply-templates mode="labeled-equation"/>
                          </xsl:when>
                          <xsl:when test="$table-structure/n-mtr=1
                          and             descendant::mml:mtable[1][@frame='solid']">
<!--   framed  - \fbox or \frame -->
                            <xsl:text xml:space="preserve">\[&#xA;</xsl:text>
                            <xsl:apply-templates/>
                            <xsl:text xml:space="preserve">&#xA;\]</xsl:text>
                          </xsl:when>

                          <xsl:otherwise>
<!--   1 cell array?? -->
                            <xsl:text xml:space="preserve">\[&#xA;</xsl:text>
                            <xsl:apply-templates/>
                            <xsl:text xml:space="preserve">&#xA;\]</xsl:text>
                          </xsl:otherwise>
                        </xsl:choose>
					  </xsl:when>
<!-- mtable, 1 column, end single row -->

					  <xsl:otherwise>
<!-- mtable, 1 column, multiple rows -->
                        <xsl:choose>
                          <xsl:when test="descendant::mml:mtable[1][@columnalign='left']
                          or              descendant::mml:mtable[1][@columnalign='right']">
<!--   \begin{array} -->
                            <xsl:text>$</xsl:text>
                            <xsl:apply-templates/>
                            <xsl:text>$</xsl:text>
                          </xsl:when>
						  <xsl:otherwise>
<!--   \begin{eqnarray} -->
<!--   \begin{align} -->
<!--   \begin{gather} -->
                            <xsl:apply-templates mode="eqnarray"/>
						  </xsl:otherwise>
						</xsl:choose>
<!-- end multiple rows -->
					  </xsl:otherwise>

                    </xsl:choose>
                  </xsl:when>

<!-- end 1 column per row -->

                  <xsl:otherwise>

<!-- mtables with multiple cells per row -->
                    <xsl:text xml:space="preserve">\[&#xA;</xsl:text>
                    <xsl:apply-templates/>
                    <xsl:text xml:space="preserve">&#xA;\]</xsl:text>
                  </xsl:otherwise>

                </xsl:choose>

              </xsl:when>

              <xsl:otherwise>
<!-- not an mtable, n-children = 1 -->
                                <xsl:text xml:space="preserve">\[&#xA;</xsl:text>
                <xsl:apply-templates/>
                <xsl:text xml:space="preserve">&#xA;\]</xsl:text>
              </xsl:otherwise>
            </xsl:choose>

          </xsl:when>

          <xsl:otherwise>
<!-- The display math contains more than 1 first level object.
     Might want to use \begin{equation*}..\end{equation*} here. -->
            <xsl:text xml:space="preserve">\[&#xA;</xsl:text>
            <xsl:apply-templates/>
            <xsl:text xml:space="preserve">&#xA;\]</xsl:text>
          </xsl:otherwise>

        </xsl:choose>

<!-- end displays -->

      </xsl:when>

      <xsl:when test="$is-display='false'">
<!-- known to be inline math -->
          <xsl:if test="$math-structure/n-children&gt;0">
        <xsl:text>$</xsl:text>
            <xsl:apply-templates/>
        <xsl:text>$</xsl:text>
        </xsl:if>
      </xsl:when>

      <xsl:otherwise>
<!-- Here <math> doesn't give it's "display " attribute and doesn't 
  contain an <mstyle displaystyle"whatever"> as it's only child
  so we don't know if it's inline or display. -->
          <xsl:text>$</xsl:text>
  
        <xsl:apply-templates/>
        <xsl:text>$</xsl:text>
      </xsl:otherwise>

    </xsl:choose>
</stream-with-break-tokens>

</xsl:template>


  <xsl:include href="attrutil.xsl"/>
  <xsl:include href="mathmap.xsl"/>
  <xsl:include href="textmap.xsl"/>
  <xsl:include href="mtext.xsl"/>
  <xsl:include href="mstyle.xsl"/>




<!-- mspace is a leaf node - not finished -->

  <xsl:template match="mml:mspace">
  
<!--
$ab\ c~d\quad e\qquad f\,g\;h$\/$i\!j{}k$\noindent $l\hspace{1in}mnop$

req space - b\ c - absorbed here
  <mml:mo lspace="0.444444em">&InvisibleTimes;</mml:mo>
non breaking space - c~d
  <mml:mspace width="thickmathspace" linebreak="nobreak" />
em-space - d\quad e - converted to mtext
  <mml:mtext>&emsp;</mml:mtext>
2em-space - e\qquad f - converted to mtext
  <mml:mtext>&emsp;&emsp;</mml:mtext>
thin space - f\,g - absorbed here
  <mml:mo lspace="0.333333em">&InvisibleTimes;</mml:mo>
thick space - g\;h - absorbed here
  <mml:mo lspace="0.444444em">&InvisibleTimes;</mml:mo>
  <mml:mi>h</mml:mi>
italic correction - disregarded completely
  <mml:mi>i</mml:mi>
neg thin space - i\!j - absorbed here??
  <mml:mo lspace="0.000000em">&InvisibleTimes;</mml:mo>
  <mml:mi>j</mml:mi>
zero space - j{}k - absorbed here??
  <mml:mo lspace="0.166667em">&InvisibleTimes;</mml:mo>
  <mml:mi>k</mml:mi>
no indent - disregarded completely
  <mml:mi>l</mml:mi>
  <mml:mspace width="1.000000in" />
  <mml:mi>m</mml:mi>
-->
    <xsl:choose>
      <xsl:when test="@type">
        <xsl:choose>
          <xsl:when test="@type='normalSpace'"> </xsl:when>
          <xsl:when test="@type='requiredSpace'">\ </xsl:when>
          <xsl:when test="@type='nonBreakingSpace'">~</xsl:when>
          <xsl:when test="@type='emSpace'">\quad </xsl:when>
          <xsl:when test="@type='twoEmSpace'">\qquad </xsl:when>
          <xsl:when test="@type='thinSpace'">\thinspace </xsl:when>
          <xsl:when test="@type='thickSpace'">\ </xsl:when>
          <xsl:when test="@type='italicCorrectionSpace'">\/ </xsl:when>
          <xsl:when test="@type='negativeThinSpace'">\negthinspace </xsl:when>
          <xsl:when test="@type='zeroSpace'">{}</xsl:when>
          <xsl:when test="@type='noIndent'">\noindent </xsl:when>
          <xsl:when test="@type='customSpace'">\hspace<xsl:if test="@atEnd='true'">*</xsl:if> {
            <xsl:if test="@class='stretchySpace'">\stretch{<xsl:value-of select="@flex"/>}</xsl:if>
            <xsl:if test="@dim">
              <xsl:value-of select="@dim"/>
            </xsl:if>
             }
            <xsl:if test="@fillWith">
              <xsl:if test="@fillWith='dots'">\dotfill </xsl:if>
              <xsl:if test="@fillWith='line'">\hrulefill </xsl:if>
            </xsl:if>
          </xsl:when>
        </xsl:choose>
      </xsl:when>
      <xsl:otherwise>
        <xsl:choose>
          <xsl:when test="@width='thickmathspace'
          and             @linebreak='nobreak'">
            <xsl:text>~</xsl:text>
          </xsl:when>
          <xsl:when test="@width='0.500000in'">
            <xsl:text>\hspace{0.5in}</xsl:text>
          </xsl:when>
          <xsl:when test="@width='veryverythinmathspace'">
          </xsl:when>
          <xsl:when test="@width='thickmathspace'">
            <xsl:text>\;</xsl:text>
          </xsl:when>
          <xsl:when test="@width='thinmathspace'">
            <xsl:text>\,</xsl:text>
          </xsl:when>
          <xsl:when test="@width='negativethinmathspace'">
            <xsl:text>\!</xsl:text>
          </xsl:when>
          <xsl:when test="@linebreak='goodbreak'">
            <xsl:text xml:space="preserve">\allowbreak </xsl:text>
          </xsl:when>
          <xsl:otherwise>
            <xsl:text>?mspace?</xsl:text>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="mml:mspace" mode="in-text">
      <xsl:apply-templates select="."/>
  </xsl:template>


  <xsl:template match="mml:mrow">
      <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="mml:mrow" mode="in-text">
      <xsl:variable name="LaTeX-contents.tr">
	  <raw-LaTeX>
        <xsl:apply-templates mode="in-text"/>
	  </raw-LaTeX>
    </xsl:variable>
    <xsl:variable name="LaTeX-contents" select="exsl:node-set($LaTeX-contents.tr)"/>

    <xsl:call-template name="remove-dollar-dollar">
      <xsl:with-param name="LaTeX-zstr" select="$LaTeX-contents/raw-LaTeX"/>
    </xsl:call-template>
  </xsl:template>


  <xsl:template match="mml:maligngroup">
      <!-- xsl:text xml:space="preserve"> &amp; </xsl:text -->
  </xsl:template>

  <xsl:template match="mml:maligngroup" mode="in-text">
  <xsl:text xml:space="preserve">maligngroup in text??</xsl:text>
    <!-- xsl:text xml:space="preserve"> &amp; </xsl:text -->
  </xsl:template>

  <xsl:include href="mn.xsl"/>
  <xsl:include href="fencemo.xsl"/>
  <xsl:include href="mo.xsl"/>
  <xsl:include href="mi.xsl"/>
  <xsl:include href="mfrac.xsl"/>



  <xsl:template match="mml:msqrt">
      <xsl:text>\sqrt{</xsl:text>
    <xsl:call-template name="do-implied-mrow"/>
    <xsl:text>}</xsl:text>
  </xsl:template>

  <xsl:template match="mml:msqrt" mode="in-text">
      <xsl:text>$</xsl:text>
    <xsl:apply-templates select="."/>
    <xsl:text>$</xsl:text>
  </xsl:template>


  <xsl:template match="mml:mroot">
      <xsl:text>\sqrt[{</xsl:text>
    <xsl:call-template name="do-positional-arg">
      <xsl:with-param name="arg-num" select="2"/>
    </xsl:call-template>
    <xsl:text>}]{</xsl:text>
    <xsl:call-template name="do-positional-arg">
      <xsl:with-param name="arg-num" select="1"/>
    </xsl:call-template>
    <xsl:text>}</xsl:text>
  </xsl:template>

  <xsl:template match="mml:mroot" mode="in-text">
      <xsl:text>$</xsl:text>
    <xsl:apply-templates select="."/>
    <xsl:text>$</xsl:text>
  </xsl:template>


  <xsl:template match="mml:merror">
      <xsl:text xml:space="preserve">merror </xsl:text>
  </xsl:template>

  <xsl:template match="mml:merror" mode="in-text">
      <xsl:text xml:space="preserve">merror in text</xsl:text>
  </xsl:template>


  <xsl:template match="mml:mpadded">
      <xsl:apply-templates/>
    <xsl:call-template name="do-vspace"/>
  </xsl:template>

  <xsl:template match="mml:mpadded" mode="in-text">
      <xsl:apply-templates mode="in-text"/>
    <xsl:call-template name="do-vspace"/>
  </xsl:template>


  <xsl:template name="do-vspace">
  
<!--
<mml:mpadded depth="0.100000in" height="0.200000in">  \rule[-0.1in]{0in}{0.3in}
-->													  

	<xsl:choose>
      <xsl:when test="string-length(@height)&gt;0
      and             string-length(@depth)&gt;0">
		<xsl:choose>
          <xsl:when test="@height='+8.5pt'
          and             @depth='+3.5pt'
          ">
            <xsl:text xml:space="preserve">\strut </xsl:text>
          </xsl:when>
          <xsl:otherwise>

            <xsl:variable name="ht">
              <xsl:call-template name="get-number-chars">
                <xsl:with-param name="attrib-cdata" select="normalize-space(@height)"/>
              </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="ht-unit">
              <xsl:value-of select="substring-after(@height,$ht)"/>
            </xsl:variable>

            <xsl:variable name="dp">
              <xsl:call-template name="get-number-chars">
                <xsl:with-param name="attrib-cdata" select="normalize-space(@depth)"/>
              </xsl:call-template>
            </xsl:variable>
            <xsl:variable name="dp-unit">
              <xsl:value-of select="substring-after(@depth,$dp)"/>
            </xsl:variable>


            <xsl:text>\rule</xsl:text>
            <xsl:if test="string-length(@depth)&gt;0">
              <xsl:text>[-</xsl:text>
<!--
 There isn't anything to add here - I'm calling "add-dimens" because it formats output.
-->
              <xsl:call-template name="add-dimens">
                <xsl:with-param name="value1" select="$ht"/>
                <xsl:with-param name="unit1"  select="$ht-unit"/>
                <xsl:with-param name="value2" select="0"/>
                <xsl:with-param name="unit2"  select="$ht-unit"/>
              </xsl:call-template>
              <xsl:text>]</xsl:text>
            </xsl:if>
            <xsl:text>{0in}{</xsl:text>
              <xsl:call-template name="add-dimens">
                <xsl:with-param name="value1" select="$ht"/>
                <xsl:with-param name="unit1"  select="$ht-unit"/>
                <xsl:with-param name="value2" select="$dp"/>
                <xsl:with-param name="unit2"  select="$dp-unit"/>
              </xsl:call-template>
            <xsl:text>}</xsl:text>
          </xsl:otherwise>
		</xsl:choose>
      </xsl:when>

      <xsl:when test="string-length(@depth)&gt;0">
		<xsl:choose>
          <xsl:when test="@depth='+0.500000ex'">
            <xsl:text xml:space="preserve">\smallskip </xsl:text>
          </xsl:when>
          <xsl:when test="@depth='+1.000000ex'">
            <xsl:text xml:space="preserve">\medskip </xsl:text>
          </xsl:when>
          <xsl:when test="@depth='+2.000000ex'">
            <xsl:text xml:space="preserve">\bigskip </xsl:text>
          </xsl:when>
          <xsl:when test="@depth='+6.000000pt'">
            <xsl:text>\vspace{6pt}</xsl:text>
          </xsl:when>
          <xsl:otherwise>
<!-- xsl:text xml:space="preserve"> unexpected mpadded </xsl:text -->
            <xsl:text>\vspace{</xsl:text>
			  <xsl:value-of select="@depth"/>
            <xsl:text>}</xsl:text>
          </xsl:otherwise>
		</xsl:choose>
      </xsl:when>

      <xsl:when test="string-length(@height)&gt;0">
        <!-- xsl:text xml:space="preserve"> unexpected mpadded </xsl:text -->
      </xsl:when>

      <xsl:otherwise>
        <!--xsl:text xml:space="preserve"> unexpected mpadded </xsl:text-->
      </xsl:otherwise>
	</xsl:choose>
  </xsl:template>


  <xsl:template match="mml:mphantom">
      <xsl:text>\phantom{</xsl:text>
    <xsl:apply-templates/>
    <xsl:text>}</xsl:text>
  </xsl:template>

  <xsl:template match="mml:mphantom" mode="in-text">
      <xsl:text xml:space="preserve">mphantom in text</xsl:text>
    <xsl:apply-templates mode="in-text"/>
  </xsl:template>


  <xsl:template match="mml:maction">
  <!--
      <mml:maction actiontype="toggle" selection="2">
        <mml:mrow>
          <mml:msup>
            <mml:mi>x</mml:mi>
            <mml:mn>2</mml:mn>
          </mml:msup>
          <mml:mo form="infix">&#x2b;</mml:mo>
          <mml:mn>12</mml:mn>
        </mml:mrow>
        <mml:mrow>
          <mml:msup>
            <mml:mi>x</mml:mi>
            <mml:mn>2</mml:mn>
          </mml:msup>
          <mml:mo form="infix">&#x2b;</mml:mo>
          <mml:mn>12</mml:mn>
        </mml:mrow>
      </mml:maction>
-->
    <xsl:choose>

      <xsl:when test="$output-mode='SW-LaTeX'">
        <xsl:choose>
          <xsl:when test="@actiontype='toggle'">
            <xsl:text>\FORMULA{</xsl:text>
            <xsl:apply-templates select="./*[1]"/>
            <xsl:text>}{</xsl:text>
            <xsl:apply-templates select="./*[2]"/>
            <xsl:text>}{evaluate}</xsl:text>
          </xsl:when>
          <xsl:otherwise>
            <!--xsl:text xml:space="preserve"> unexpected maction </xsl:text -->
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>

      <xsl:when test="output-mode='Portable-LaTeX'">
        <xsl:choose>
          <xsl:when test="@actiontype='toggle'">
            <xsl:choose>
              <xsl:when test="@selection='1'">
                <xsl:apply-templates select="./*[1]"/>
              </xsl:when>
              <xsl:when test="@selection='2'">
                <xsl:apply-templates select="./*[2]"/>
              </xsl:when>
              <xsl:otherwise>
                <!--xsl:text xml:space="preserve"> unexpected maction </xsl:text-->
              </xsl:otherwise>
            </xsl:choose>
          </xsl:when>
          <xsl:otherwise>
            <!--xsl:text xml:space="preserve"> unexpected maction </xsl:text-->
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>

      <xsl:otherwise>
      </xsl:otherwise>
    </xsl:choose>

  </xsl:template>

  <xsl:template match="mml:maction" mode="in-text">
      <xsl:text>$</xsl:text>
    <xsl:apply-templates select="."/>
    <xsl:text>$</xsl:text>
  </xsl:template>


  <xsl:include href="mfenced.xsl"/>
  <xsl:include href="scripts.xsl"/>
  <xsl:include href="munder.xsl"/>
  <xsl:include href="mover.xsl"/>
  <xsl:include href="munderover.xsl"/>
 


  <xsl:template match="mml:mprescripts">
      <xsl:text xml:space="preserve">mprescripts </xsl:text>
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="mml:mprescripts" mode="in-text">
      <xsl:text>$</xsl:text>
    <xsl:apply-templates select="."/>
    <xsl:text>$</xsl:text>
  </xsl:template>


  <xsl:template match="mml:mmultiscripts">
      <xsl:text xml:space="preserve">mmultiscripts </xsl:text>
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="mml:mmultiscripts" mode="in-text">
      <xsl:text>$</xsl:text>
    <xsl:apply-templates select="."/>
    <xsl:text>$</xsl:text>
  </xsl:template>


  <xsl:template match="mml:mtr">
      <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="mml:mtr" mode="in-text">
      <xsl:variable name="LaTeX-contents.tr">
	  <raw-LaTeX>
        <xsl:apply-templates mode="in-text"/>
	  </raw-LaTeX>
    </xsl:variable>
    <xsl:variable name="LaTeX-contents" select="exsl:node-set($LaTeX-contents.tr)"/>

    <xsl:call-template name="remove-dollar-dollar">
      <xsl:with-param name="LaTeX-zstr" select="$LaTeX-contents/raw-LaTeX"/>
    </xsl:call-template>
  </xsl:template>


  <xsl:template match="mml:mtd">
      <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="mml:mtd" mode="in-text">
      <xsl:variable name="LaTeX-contents.tr">
	  <raw-LaTeX>
        <xsl:apply-templates mode="in-text"/>
	  </raw-LaTeX>
    </xsl:variable>
    <xsl:variable name="LaTeX-contents" select="exsl:node-set($LaTeX-contents.tr)"/>

    <xsl:call-template name="remove-dollar-dollar">
      <xsl:with-param name="LaTeX-zstr" select="$LaTeX-contents/raw-LaTeX"/>
    </xsl:call-template>
  </xsl:template>


  <xsl:template name="frame-or-fbox">
    <xsl:param name="LaTeX-nom"/>
  
    <xsl:variable name="LaTeX-contents.tr">
	  <raw-LaTeX>
        <xsl:apply-templates mode="in-text"/>
	  </raw-LaTeX>
    </xsl:variable>
    <xsl:variable name="LaTeX-contents" select="exsl:node-set($LaTeX-contents.tr)"/>

    <xsl:value-of select="$LaTeX-nom"/>
    <xsl:call-template name="remove-dollar-dollar">
      <xsl:with-param name="LaTeX-zstr" select="$LaTeX-contents/raw-LaTeX"/>
    </xsl:call-template>
    <xsl:text>}</xsl:text>
  </xsl:template>


<!-- Tables and Matrices -->





  <xsl:template match="mml:mtable" mode="eqnarray">
  
    <xsl:variable name="table-structure.tr">
	  <n-mtr>
        <xsl:value-of select="count(mml:mtr)"/>
      </n-mtr>
	  <n-mlabeledtr>
        <xsl:value-of select="count(mml:mlabeledtr)"/>
      </n-mlabeledtr>

      <xsl:for-each select="mml:mtr|mml:mlabeledtr">
        <xsl:sort select="count(./mml:mtd/*[1][self::mml:maligngroup])"/>
	    <n-lead-aligns>
          <xsl:value-of select="count(./mml:mtd/*[1][self::mml:maligngroup])"/>
        </n-lead-aligns>
      </xsl:for-each>

      <xsl:for-each select="mml:mtr|mml:mlabeledtr">
        <xsl:sort select="count(./mml:mtd/descendant-or-self::mml:maligngroup)"/>
	    <n-total-aligns>
          <xsl:value-of select="count(./mml:mtd/descendant-or-self::mml:maligngroup)"/>
        </n-total-aligns>
      </xsl:for-each>
    </xsl:variable>

    <xsl:variable name="table-structure" select="exsl:node-set($table-structure.tr)"/>

<!-- mtables with 1 cell per row -->

<!-- Diagnostics
<xsl:text>BSSSSS</xsl:text>
<xsl:value-of select="$table-structure/n-lead-aligns[1]"/>
<xsl:value-of select="$table-structure/n-lead-aligns[last()]"/>
<xsl:text>BSSSSS</xsl:text>
<xsl:value-of select="$table-structure/n-total-aligns[1]"/>
<xsl:value-of select="$table-structure/n-total-aligns[last()]"/>
<xsl:text>BSSSSS</xsl:text>
-->

    <xsl:choose>
<!-- mtables with 1 cell per row, 2 alignments -->
      <xsl:when test="$table-structure/n-lead-aligns[1]=1
      and             $table-structure/n-lead-aligns[last()]=1
      and             $table-structure/n-total-aligns[1]=3
      and             $table-structure/n-total-aligns[last()]=3">
            <xsl:call-template name="eqnarray">
              <xsl:with-param name="n-rows"        select="$table-structure/n-mtr"/>
              <xsl:with-param name="n-labeledrows" select="$table-structure/n-mlabeledtr"/>
              <xsl:with-param name="n-aligns"      select="2"/>
            </xsl:call-template>
      </xsl:when>

<!-- mtables with 1 cell per row, 1 alignment -->
      <xsl:when test="$table-structure/n-lead-aligns[1]=1
      and             $table-structure/n-lead-aligns[last()]=1
      and             $table-structure/n-total-aligns[1]=2
      and             $table-structure/n-total-aligns[last()]=2">
            <xsl:call-template name="eqnarray">
              <xsl:with-param name="n-rows"        select="$table-structure/n-mtr"/>
              <xsl:with-param name="n-labeledrows" select="$table-structure/n-mlabeledtr"/>
              <xsl:with-param name="n-aligns"      select="1"/>
            </xsl:call-template>
      </xsl:when>

<!-- mtables with 1 cell per row, 1 alignment 
      <xsl:when test="$table-structure/n-internal-aligns[last()-1]=0
      and             $table-structure/n-internal-aligns[last()]=1">
            <xsl:call-template name="eqnarray">
              <xsl:with-param name="n-rows"        select="$table-structure/n-mtr"/>
              <xsl:with-param name="n-labeledrows" select="$table-structure/n-mlabeledtr"/>
              <xsl:with-param name="n-aligns"      select="1"/>
            </xsl:call-template>
      </xsl:when>
-->

<!-- mtables with 1 cell per row, 0 alignments -->
      <xsl:when test="$table-structure/n-lead-aligns[1]=1
      and             $table-structure/n-lead-aligns[last()]=1
      and             $table-structure/n-total-aligns[1]=1
      and             $table-structure/n-total-aligns[last()]=1">
            <xsl:call-template name="eqnarray">
              <xsl:with-param name="n-rows"        select="$table-structure/n-mtr"/>
              <xsl:with-param name="n-labeledrows" select="$table-structure/n-mlabeledtr"/>
              <xsl:with-param name="n-aligns"      select="0"/>
            </xsl:call-template>
      </xsl:when>

<!-- mtables with 1 cell per row, 4 alignments -->
      <xsl:when test="$table-structure/n-lead-aligns[1]=1
      and             $table-structure/n-lead-aligns[last()]=1
      and             $table-structure/n-total-aligns[1]=4
      and             $table-structure/n-total-aligns[last()]=4">
            <xsl:call-template name="eqnarray">
              <xsl:with-param name="n-rows"        select="$table-structure/n-mtr"/>
              <xsl:with-param name="n-labeledrows" select="$table-structure/n-mlabeledtr"/>
              <xsl:with-param name="n-aligns"      select="4"/>
            </xsl:call-template>
      </xsl:when>


      <xsl:otherwise>
		<xsl:choose>
          <xsl:when test="@frame!='' or @rowlines!='' or @columnlines!=''">
            <xsl:call-template name="tabular"/>
          </xsl:when>
          <xsl:otherwise>
            <xsl:call-template name="array">
              <xsl:with-param name="LaTeX-env" select="'array'"/>
            </xsl:call-template>
          </xsl:otherwise>
		</xsl:choose>
      </xsl:otherwise>

    </xsl:choose>


  </xsl:template>



<!-- 1 column, 1 labeled row, 0 plain rows -->

  <xsl:template match="mml:mtable" mode="labeled-equation">
      <xsl:text>\begin{equation}&#xA;</xsl:text>
    <xsl:for-each select="mml:mlabeledtr">

      <xsl:for-each select="mml:mtd">
        <xsl:apply-templates/>
<!--
        <xsl:text xml:space="preserve"> \tag{</xsl:text>
        <xsl:apply-templates mode="in-text" select="*[1]"/>
        <xsl:text>}</xsl:text>
-->
		<xsl:if test="./*[1][self::mml:mrow][@id!='']">
          <xsl:text xml:space="preserve"> \label{</xsl:text>
          <xsl:value-of select="child::mml:mrow/@id"/>
          <xsl:text>}</xsl:text>
		</xsl:if>

      </xsl:for-each>
    </xsl:for-each>

    <xsl:text>&#xA;\end{equation}</xsl:text>
  </xsl:template>


  <xsl:template match="mml:mtable" mode="in-text">
  	<xsl:choose>
      <xsl:when test="@frame!='' or @rowlines!='' or @columnlines!=''">
        <xsl:call-template name="tabular"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text>$</xsl:text>
        <xsl:call-template name="array">
          <xsl:with-param name="LaTeX-env" select="'array'"/>
        </xsl:call-template>
        <xsl:text>$</xsl:text>
      </xsl:otherwise>
	</xsl:choose>
  </xsl:template>


 
<!-- Template for mtable variants that translate
  to LaTeX objects that occur within MATH.

  \begin{array}     .. \end{array}  
  \begin{cases}     .. \end{cases}  
  \begin{split}     .. \end{split}  
  \begin{gathered}  .. \end{gathered}  
  \begin{aligned}   .. \end{aligned}
  \fbox{..}
  \frame{..}
  
  \begin{tabular}   .. \end{tabular}  
-->


  <xsl:template match="mml:mtable">
  
    <xsl:variable name="table-structure.tr">
	  <n-mtr>
        <xsl:value-of select="count(mml:mtr)"/>
      </n-mtr>
	  <n-mlabeledtr>
        <xsl:value-of select="count(mml:mlabeledtr)"/>
      </n-mlabeledtr>
      <xsl:for-each select="mml:mtr|mml:mlabeledtr">
        <xsl:sort select="count(mml:mtd)"/>
	    <n-cells>
          <xsl:value-of select="count(mml:mtd)"/>
        </n-cells>
      </xsl:for-each>
      <xsl:for-each select="mml:mtr|mml:mlabeledtr">
        <xsl:sort select="count(./mml:mtd/mml:maligngroup)"/>
	    <n-external-aligns>
          <xsl:value-of select="count(./mml:mtd/mml:maligngroup)"/>
        </n-external-aligns>
      </xsl:for-each>
      <xsl:for-each select="mml:mtr|mml:mlabeledtr">
        <xsl:sort select="count(./mml:mtd/mml:mrow/mml:maligngroup)"/>
	    <n-internal-aligns>
          <xsl:value-of select="count(./mml:mtd/mml:mrow/mml:maligngroup)"/>
        </n-internal-aligns>
      </xsl:for-each>
    </xsl:variable>
    <xsl:variable name="table-structure" select="exsl:node-set($table-structure.tr)"/>


    <xsl:choose>

<!-- mtables with 1 cell per row -->

      <xsl:when test="$table-structure/n-cells[last()]=1">

        <xsl:variable name="env-info.tr">
		<LaTeX-env>
          <xsl:choose>
<!-- gathered -->
            <xsl:when test="$table-structure/n-external-aligns[1]=1
            and             $table-structure/n-external-aligns[last()]=1
            and             $table-structure/n-internal-aligns[1]=0
            and             $table-structure/n-internal-aligns[last()]=0
            ">
			  <xsl:text>gathered</xsl:text>
            </xsl:when>
<!-- aligned -->
            <xsl:when test="$table-structure/n-external-aligns[1]=1
            and             $table-structure/n-external-aligns[last()]=1
            and             $table-structure/n-internal-aligns[1]=1
            and             $table-structure/n-internal-aligns[last()]=1
            ">
			  <xsl:text>aligned</xsl:text>
            </xsl:when>
<!-- split -->
            <xsl:when test="$table-structure/n-external-aligns[1]=1
            and             $table-structure/n-external-aligns[last()]=2
            and             $table-structure/n-internal-aligns[1]=0
            and             $table-structure/n-internal-aligns[last()]=1
            ">
	          <xsl:text>split</xsl:text>
            </xsl:when>
            <xsl:otherwise>
			  <xsl:text>array</xsl:text>
            </xsl:otherwise>
          </xsl:choose>
	    </LaTeX-env>
        </xsl:variable>
        <xsl:variable name="env-info" select="exsl:node-set($env-info.tr)"/>


        <xsl:choose>

          <xsl:when test="$table-structure/n-mtr=1
          and             $table-structure/n-mlabeledtr=0
          and             @frame='solid'">
            <xsl:choose>
              <xsl:when test="@framespacing='0.8em 1.0ex'">
                <xsl:call-template name="frame-or-fbox">
                  <xsl:with-param name="LaTeX-nom" select="'\fbox{'"/>
                </xsl:call-template>
              </xsl:when>
              <xsl:otherwise>
                <xsl:call-template name="frame-or-fbox">
                  <xsl:with-param name="LaTeX-nom" select="'\frame{'"/>
                </xsl:call-template>
              </xsl:otherwise>
            </xsl:choose>
          </xsl:when>

          <xsl:when test="@rowspacing=0
          and @columnspacing=0
		  and (parent::mml:msub or parent::mml:msup or parent::mml:msubsup
		       or parent::mml:munder or parent::mml:mover or parent::mml:munderover)
          and (preceding-sibling::*[1])">
            <xsl:call-template name="substack"/>
          </xsl:when>


          <xsl:otherwise>
		    <xsl:choose>
              <xsl:when test="@frame!='' or @rowlines!='' or @columnlines!=''">
                <xsl:call-template name="tabular"/>
              </xsl:when>
              <xsl:otherwise>
                <xsl:call-template name="array">
                  <xsl:with-param name="LaTeX-env" select="$env-info/LaTeX-env"/>
                </xsl:call-template>
              </xsl:otherwise>
			</xsl:choose>
          </xsl:otherwise>

        </xsl:choose>

      </xsl:when>

<!-- mtables with more than 1 cell per row -->

      <xsl:otherwise>
		<xsl:choose>
          <xsl:when test="@frame!='' or @rowlines!='' or @columnlines!=''">
            <xsl:call-template name="tabular"/>
          </xsl:when>
          <xsl:otherwise>
            <xsl:call-template name="array">
              <xsl:with-param name="LaTeX-env" select="'array'"/>
            </xsl:call-template>
          </xsl:otherwise>
		</xsl:choose>
      </xsl:otherwise>
    </xsl:choose>

  </xsl:template>



  <xsl:include href="MML2arry.xsl"/>
  <xsl:include href="MML2eqna.xsl"/>
  <xsl:include href="MML2tabular.xsl"/>	

<!-- Frequently, the numbered children of a MathML schemata
  translate to an argument of some LaTeX command.

  ie. <msup><1><2></msup>  -> 1^{2}

  The following template generates the contents of the LaTeX arg.
  It looks for special cases like an empty arg, etc.
-->

  <xsl:template name="do-positional-arg">
    <xsl:param name="arg-num"/>
      <xsl:choose>
      <xsl:when test="./*[$arg-num][self::mml:mtext][string()='&#x200B;']">
        <xsl:text>{}</xsl:text>
	  </xsl:when>
      <xsl:otherwise>
        <xsl:apply-templates select="./*[$arg-num]"/>
      </xsl:otherwise>
    </xsl:choose>
    </xsl:template>


<!-- Some MathML schemata contain a single MATH bucket -
  which may be scripted as an implied mrow -
  which translate to an argument of some LaTeX command.

  ie. <msqrt>...</msqrt>  -> \sqrt{...}

  The following template generates the contents of the LaTeX arg.
  It looks for special cases like an empty arg, etc.
-->

  <xsl:template name="do-implied-mrow">
      <xsl:choose>
      <xsl:when test="./*[1][self::mml:mtext][string()='&#x200B;']
      and            count(./*)=1">
        <xsl:text>{}</xsl:text>
	  </xsl:when>
      <xsl:otherwise>
        <xsl:apply-templates/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>



<!-- At certain times - for example when mtable/mtr/mtd is being
  translated into a cell in a LaTeX tabular environment - we are
  generating LaTeX TEXT.  LaTeX Math objects must be nested in $..$
  There is a problem if 2 consecutive translations are nested.
  ie. $\sin $$\theta $

  Translations that generate TEXT are first scripted into an xsl:variable.
  $$'s are stripped from the variable to produce final LaTeX output.

  Sample xsl script:

	.
    <xsl:variable name="LaTeX-contents.tr">
	  <raw-LaTeX>
        <xsl:apply-templates mode="in-text"/>
	  </raw-LaTeX>
    </xsl:variable>
    <xsl:variable name="LaTeX-contents" select="exsl:node-set($LaTeX-contents.tr)"/>

    <xsl:call-template name="remove-dollar-dollar">
      <xsl:with-param name="LaTeX-zstr" select="$LaTeX-contents/raw-LaTeX"/>
    </xsl:call-template>
    .

-->

  <xsl:template name="remove-dollar-dollar">
    <xsl:param name="LaTeX-zstr"/>
  
    <xsl:choose>
      <xsl:when test="contains($LaTeX-zstr,'$$')">
        <xsl:if test="string-length(substring-before($LaTeX-zstr,'$$'))&gt;0">
          <xsl:value-of select="substring-before($LaTeX-zstr,'$$')"/>
        </xsl:if>
        <xsl:if test="string-length(substring-after($LaTeX-zstr,'$$'))&gt;0">
          <xsl:call-template name="remove-dollar-dollar">
            <xsl:with-param name="LaTeX-zstr" select="substring-after($LaTeX-zstr,'$$')"/>
          </xsl:call-template>
        </xsl:if>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="$LaTeX-zstr"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>


<!--
  case  2 : // \ <uID9.1.2>	    required          ems_width +=  5;
  case  3 : // ~<uID9.1.3>	    non-breaking      ems_width +=  5;
  case  4 : // \quad<uID9.1.4>	em				  ems_width +=  18;
  case  5 : // \qquad<uID9.1.5>	2-em              ems_width +=  36;
  case 10 : // {}<uID9.1.10>    empty             zero_space =  TRUE;

  case  6 : // \,<uID9.1.6>	    thin              ems_width +=  3;
  case  7 : // \;<uID9.1.7>	    thick             ems_width +=  5;
  case  8 : // \/<uID9.1.8>	    italic correction ems_width +=  1;

  case  9 : // \!<uID9.1.9>	    negative thin     ems_width -=  3;
-->


  <xsl:template name="operator-lrspace-2LaTeX">
    <xsl:param name="value"/>
    <xsl:param name="unit"/>
  	<xsl:choose>
      <xsl:when test="$unit='em'">
	    <xsl:choose>
          <xsl:when test="$value &lt; 0.1">
            <xsl:text>\/</xsl:text>
          </xsl:when>
          <xsl:when test="$value &lt; 0.2">
            <xsl:text>\,</xsl:text>
          </xsl:when>
          <xsl:when test="$value &lt; 0.6">
            <xsl:text>\;</xsl:text>
          </xsl:when>
          <xsl:when test="$value &lt; 1.5">
            <xsl:text xml:space="preserve">\quad </xsl:text>
          </xsl:when>
          <xsl:when test="$value &lt; 2.5">
            <xsl:text xml:space="preserve">\qquad </xsl:text>
          </xsl:when>
          <xsl:otherwise>
            <!--xsl:text xml:space="preserve">Unexpected l-rspace value.</xsl:text-->
          </xsl:otherwise>
	    </xsl:choose>
      </xsl:when>
      <xsl:otherwise>
	    <!--xsl:text xml:space="preserve">Unexpected l-rspace unit.</xsl:text-->
      </xsl:otherwise>
	</xsl:choose>
  </xsl:template>

  <xsl:include href="HTM2LTeX.xsl"/>


</xsl:stylesheet>
