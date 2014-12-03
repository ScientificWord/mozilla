<?xml version="1.0"?>
<xsl:stylesheet version="1.1" 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:mml="http://www.w3.org/1998/Math/MathML"
    xmlns:html="http://www.w3.org/1999/xhtml"
    xmlns:sw="http://www.sciword.com/namespaces/sciword"
    xmlns:msi="http://www.sciword.com/namespaces/sciword"
    xmlns:exsl="http://exslt.org/common">

<xsl:template match="@*|node()">
    <xsl:copy>
      <xsl:apply-templates select="@*|node()"/>
    </xsl:copy>
</xsl:template>

<!-- throw away hlines. these are processed below -->
<xsl:template match="html:hline | mml:hline" />

<!-- throws away empty mrows that are not required -->
<xsl:template match="mml:mrow">
  <xsl:choose>
    <xsl:when test="
                    parent::mml:mfrac or 
                    parent::mml:msub or 
                    parent::mml:msup or
                    parent::mml:msubsup or
                    parent::mml:munder or
                    parent::mml:mover or
                    parent::mml:munderover">
      <mml:mrow><xsl:apply-templates/></mml:mrow>
    </xsl:when>
    <xsl:otherwise>
      <xsl:apply-templates/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>



<xsl:template name="get-spec">
   <xsl:param name="col">
      <xsl:value-of select="1" />
    </xsl:param>
   <xsl:param name="str"/>
   
   <xsl:choose>
     <xsl:when test="$str=''" />

     <!-- check for left-side lines -->
     <xsl:when test="substring($str,1,1)='|'" >
       <xsl:choose>
          <xsl:when test="string-length($str)>1 and substring($str,2,1)='|'">
            <!-- a double line -->
            <xsl:choose>
              <xsl:when test="string-length($str)=2">
                <html:cell-attribute>
                  <xsl:attribute name="type">line</xsl:attribute>
		              <xsl:attribute name="dir">vertical</xsl:attribute>
                  <xsl:attribute name="kind">double</xsl:attribute>
		              <xsl:attribute name="side">right</xsl:attribute>
                  <xsl:attribute name="col">
                     <xsl:value-of select="$col - 1"/>
		              </xsl:attribute>                  
		            </html:cell-attribute>
	            </xsl:when>
	            <xsl:otherwise>
                <html:cell-attribute>
                  <xsl:attribute name="type">line></xsl:attribute> 
                  <xsl:attribute name="dir">vertical</xsl:attribute>
		              <xsl:attribute name="kind">double</xsl:attribute>
		              <xsl:attribute name="side">left</xsl:attribute>
                  <xsl:attribute name="col">
                    <xsl:value-of select="$col"/>
		              </xsl:attribute>                 
		            </html:cell-attribute>
                
		            <xsl:call-template name="get-spec">
		              <xsl:with-param name="str">
                     <xsl:value-of select="substring($str,3)"/>
		              </xsl:with-param>
		              <xsl:with-param name = "col" >
                     <xsl:value-of select="$col" />
		              </xsl:with-param>
		            </xsl:call-template>
	            </xsl:otherwise>
	          </xsl:choose>
          </xsl:when>
          
          <xsl:otherwise>
            <!-- a single line -->
            <xsl:choose>
	             <xsl:when test="string-length($str)=1">
		             <html:cell-attribute>
                    <xsl:attribute name="type">line</xsl:attribute>
                    <xsl:attribute name="dir">
                      <xsl:value-of select="'vertical'"/>
                    </xsl:attribute>
                    <xsl:attribute name="kind">single</xsl:attribute>
		                <xsl:attribute name="col">
                      <xsl:value-of select="$col - 1"/>
		                </xsl:attribute>
		               <xsl:attribute name="side">right</xsl:attribute>
		             </html:cell-attribute>
	            </xsl:when>
	            <xsl:otherwise>
                <html:cell-attribute>
                  <xsl:attribute name="type">line</xsl:attribute>
                  <xsl:attribute name="dir">
                    <xsl:value-of select="'vertical'"/>
                  </xsl:attribute>
                  <xsl:attribute name="kind">single</xsl:attribute>
		              <xsl:attribute name="col">
                    <xsl:value-of select="$col"/>
		              </xsl:attribute>
		              <xsl:attribute name="side">left</xsl:attribute>
		            </html:cell-attribute>		
	            </xsl:otherwise>
	          </xsl:choose>
                  
            <xsl:call-template name="get-spec">
              <xsl:with-param name="str">
                <xsl:value-of select="substring($str,2)"/>
              </xsl:with-param>
              <xsl:with-param name="col">
                <xsl:value-of select="$col" />
              </xsl:with-param>
            </xsl:call-template>
          </xsl:otherwise>
       </xsl:choose>
     </xsl:when>
   
     <!-- check for l,r,c etc -->
     
     <xsl:when test="substring($str,1,1)='l'" >
       <html:cell-attribute>
         <xsl:attribute name="type">left-justify</xsl:attribute>
	       <xsl:attribute name="col">
           <xsl:value-of select="$col"/>
         </xsl:attribute>
       </html:cell-attribute>
       <xsl:call-template name="get-spec">
         <xsl:with-param name="str">
           <xsl:value-of select="substring($str,2)"/>
         </xsl:with-param>
         <xsl:with-param name="col">
           <xsl:value-of select="$col + 1" />
         </xsl:with-param>
       </xsl:call-template>
     </xsl:when>
     
     <xsl:when test="substring($str,1,1)='r'" >
       <html:cell-attribute>
         <xsl:attribute name="type">right-justify</xsl:attribute>
	       <xsl:attribute name="col">
           <xsl:value-of select="$col"/>
         </xsl:attribute>
       </html:cell-attribute>
       <xsl:call-template name="get-spec">
         <xsl:with-param name="str">
           <xsl:value-of select="substring($str,2)"/>
         </xsl:with-param>
         <xsl:with-param name="col">
           <xsl:value-of select="$col + 1" />
         </xsl:with-param>
       </xsl:call-template>
     </xsl:when>
     
     <xsl:when test="substring($str,1,1)='c'" >
       <html:cell-attribute>
         <xsl:attribute name="type">center-justify</xsl:attribute>
	       <xsl:attribute name="col">
           <xsl:value-of select="$col"/>
         </xsl:attribute>
       </html:cell-attribute>
       <xsl:call-template name="get-spec">
         <xsl:with-param name="str">
           <xsl:value-of select="substring($str,2)"/>
         </xsl:with-param>
         <xsl:with-param name="col">
           <xsl:value-of select="$col + 1" />
         </xsl:with-param>
       </xsl:call-template>     </xsl:when>

     <xsl:otherwise>Huh??</xsl:otherwise>
 
   </xsl:choose>   
</xsl:template>

<xsl:template name="get-hlines">
   <xsl:param name="raw-hlines"/>
   <xsl:for-each select="$raw-hlines">
      <xsl:variable name="parent" select=".."/>
      
      <html:cell-attribute>
         <xsl:attribute name="type">line</xsl:attribute>
         <xsl:attribute name="dir">horizontal</xsl:attribute>
         <xsl:choose>
           <xsl:when test="$parent=''">
              <xsl:attribute name="row">
	               <xsl:value-of select="@row - 1"/>
              </xsl:attribute>
              <xsl:attribute name="side">bottom</xsl:attribute>
           </xsl:when>
           <xsl:otherwise>  
              <xsl:attribute name="row">
	              <xsl:value-of select="@row"/>
              </xsl:attribute>
              <xsl:attribute name="side">top</xsl:attribute>
           </xsl:otherwise>
        </xsl:choose>                  
         
      </html:cell-attribute>
   </xsl:for-each>
</xsl:template>



<xsl:template match="html:table">
   
   <xsl:variable name="colspec">
     <xsl:value-of select="./@cols"/>
   </xsl:variable>
  
   <xsl:variable name="theHLines" 
                 select=".//*[local-name()='hline' or local-name()='cline']"/>
   
    <xsl:variable name="the-spec.tr">
     <xsl:call-template name="get-spec">
      <xsl:with-param name="str">
        <xsl:value-of select="$colspec"/>
      </xsl:with-param>
     </xsl:call-template>
     <xsl:call-template name="get-hlines">
       <xsl:with-param name="raw-hlines" select="exsl:node-set($theHLines)"/>
     </xsl:call-template>
   </xsl:variable>

   <xsl:variable name="the-spec" select="exsl:node-set($the-spec.tr)"/>

   <xsl:variable name="theTable" select="."/>
   
   <!-- html:note>
       [thespec: <xsl:copy-of select="$the-spec"/>]
   </html:note -->

   <html:table>
     <xsl:apply-templates select="@*"/>
     <xsl:apply-templates>
       <xsl:with-param name="the-spec">
          <xsl:copy-of select="$the-spec"/>
       </xsl:with-param>
     </xsl:apply-templates>
   </html:table>

</xsl:template>

<xsl:template match="html:tbody">
  <xsl:param name="the-spec"/>
  <xsl:variable name = "nrows" select="count(./html:tr)"/>
  <xsl:copy>
    <xsl:apply-templates select="@*|node()">
       <xsl:with-param name="the-spec" select="$the-spec"/>
    </xsl:apply-templates>
  </xsl:copy>
</xsl:template>

<xsl:template match="html:tr|mml:tr">
  <xsl:param name="the-spec" />
  <xsl:variable name="content">
     <xsl:value-of select="."/>
  </xsl:variable>
  <xsl:choose>
    <xsl:when test="(position()=last()) and ($content='')">
    </xsl:when>
    <xsl:otherwise>
       <xsl:copy>                        
          <xsl:apply-templates select="@*|node()">
            <xsl:with-param name="the-spec" select="$the-spec"/>
          </xsl:apply-templates>
       </xsl:copy>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>



<xsl:template match="html:td">
   <xsl:param name="the-spec"/>
   <xsl:variable name="pos">
     <xsl:value-of select='position()'/>
   </xsl:variable>
   <xsl:variable name="row" select="1+count(../preceding-sibling::*)"/>
   <xsl:variable name="col">
     <xsl:call-template name="sum-colspans">
       <xsl:with-param name="nodes" select="preceding-sibling::*"/>
       <xsl:with-param name="result" select="1" />
     </xsl:call-template>
   </xsl:variable>
   
   <xsl:variable name="my-spec" select="exsl:node-set($the-spec)"/>

   <xsl:variable name="my-attribs.tr">
      <xsl:copy-of select="$my-spec//*[(@col = number($col)) or (@row = $row)]"/>
   </xsl:variable>

   <xsl:variable name="my-attribs" select="exsl:node-set($my-attribs.tr)"/>

   <html:td>
     <xsl:apply-templates select="@*"/>
     <xsl:if test="$my-attribs//*[@side ='left']">
       <xsl:choose>
          <xsl:when test="$my-attribs//*[@side ='left' and @kind='double']">
            <xsl:attribute name="line-left">double</xsl:attribute>
          </xsl:when>
          <xsl:otherwise>
             <xsl:attribute name="line-left">solid</xsl:attribute>
          </xsl:otherwise>
      </xsl:choose>
     </xsl:if>
     <xsl:if test="$my-attribs//*[@side ='right']">
       <xsl:choose>
          <xsl:when test="$my-attribs//*[@side ='right' and @kind='double']">
             <xsl:attribute name="line-right">double</xsl:attribute>
          </xsl:when>
          <xsl:otherwise>
             <xsl:attribute name="line-right">solid</xsl:attribute>
          </xsl:otherwise>
       </xsl:choose>     
     </xsl:if>
     <xsl:if test="$my-attribs//*[@side ='top']">
       <xsl:choose>
          <xsl:when test="$my-attribs//*[@side ='top' and @kind='double']">
            <xsl:attribute name="line-top">double</xsl:attribute>
          </xsl:when>
          <xsl:otherwise>
             <xsl:attribute name="line-top">solid</xsl:attribute>
          </xsl:otherwise>
       </xsl:choose>     
     </xsl:if>
     <xsl:if test="$my-attribs//*[@side ='bottom']">
       <xsl:choose>
          <xsl:when test="$my-attribs//*[@side ='bottom' and @kind='double']">
             <xsl:attribute name="line-bottom">double</xsl:attribute>
          </xsl:when>
          <xsl:otherwise>
             <xsl:attribute name="line-bottom">solid</xsl:attribute>
          </xsl:otherwise>
       </xsl:choose>
     </xsl:if>
     <xsl:if test="$my-attribs//*[@type ='left-justify']">
        <xsl:attribute name="align">left</xsl:attribute>
     </xsl:if>
     <xsl:if test="$my-attribs//*[@type ='right-justify']">
        <xsl:attribute name="align">right</xsl:attribute>
     </xsl:if>
     <xsl:if test="$my-attribs//*[@type ='center-justify']">
        <xsl:attribute name="align">center</xsl:attribute>
     </xsl:if>

     <xsl:apply-templates select="@*|node()"/>
     
   </html:td>

</xsl:template>

<xsl:template name="colspan">
  <xsl:param name="node" />
  <xsl:variable name="cs">
     <xsl:choose>
        <xsl:when test="@colspan">
           <xsl:value-of select="./@colspan" />
        </xsl:when>
        <xsl:otherwise>
          <xsl:text>1</xsl:text>
        </xsl:otherwise>
     </xsl:choose>
  </xsl:variable>
  <xsl:value-of select="$cs" />
</xsl:template>

<xsl:template name="sum-colspans" >
   <xsl:param name="nodes" select="/.."/>
   <xsl:param name="result" select="0"/>
   <xsl:choose>
      <xsl:when test="not($nodes)">
        <xsl:value-of select="$result"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:variable name="cs">
          <xsl:call-template name="colspan">
            <xsl:with-param name="node" select="$nodes[1]" />
          </xsl:call-template>
        </xsl:variable>
        <xsl:call-template name="sum-colspans">
           <xsl:with-param name="nodes" select="$nodes[position() != 1]" />
           <xsl:with-param name="result" select="$result + $cs" />
        </xsl:call-template>
      </xsl:otherwise>
   </xsl:choose>
</xsl:template>



</xsl:stylesheet>
