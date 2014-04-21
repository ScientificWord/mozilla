<xsl:stylesheet version="1.1" 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:mml="http://www.w3.org/1998/Math/MathML"
    xmlns:html="http://www.w3.org/1999/xhtml"
    xmlns:sw="http://www.sciword.com/namespaces/sciword"
    xmlns:exsl="http://exslt.org/common"
>

<!-- xsl:variable name="tagsList" select="document('latexdefs.xml')//*[local-name()='tagproperties']/*[local-name()='tagclasses']"/ -->

<xsl:variable name="paraList"><xsl:text>__bodyText__bodyMath__rtlBodyText__sectiontitle__centered__</xsl:text></xsl:variable>

<xsl:variable name="envList"><xsl:text>__bulletlist__numberedlist__descriptionlist__set__book__part__chapter__section__subsection</xsl:text>
  <xsl:text>__subsubsection__paragraph__subparagraph__longQuotation__dedication__centeredEnv__preface__glossary__verbatim__note</xsl:text>
  <xsl:text>__assertion__conjecture__corollary__criterion__lemma__proposition__theorem__algorithm__assumption__axiom__condition</xsl:text>
  <xsl:text>__definition__example__exercise__hypothesis__problem__property__question__case__claim__conclusion__notation</xsl:text>
  <xsl:text>__remark__summary__proof__</xsl:text>
</xsl:variable>

<xsl:template name="buildtable">
  <xsl:variable name="theTable" select="." />

  <xsl:variable name="topCaption">
    <xsl:choose>
      <xsl:when test="html:caption[not(@align)]">1</xsl:when>
      <xsl:when test="html:caption[@align='top']">1</xsl:when>
      <xsl:otherwise>0</xsl:otherwise>
    </xsl:choose>
  </xsl:variable>

  <xsl:variable name="bottomCaption">
    <xsl:choose>
      <xsl:when test="html:caption[@align='bottom']">1</xsl:when>
      <xsl:otherwise>0</xsl:otherwise>
    </xsl:choose>
  </xsl:variable>

  <xsl:variable name="caption" select="html:caption"/>
 

  <xsl:variable name="embedded" 
    select="count(ancestor::html:table)"/>

  <xsl:variable name="tabularType">
    <xsl:choose>
      <xsl:when test="@width &gt; 0">
        <xsl:choose>
          <xsl:when test="@req='tabulary'">tabulary</xsl:when>
          <xsl:otherwise><xsl:text>tabular*</xsl:text></xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:otherwise>tabular</xsl:otherwise>
    </xsl:choose>
  </xsl:variable>

  <xsl:variable name="mmWidth">
    <xsl:choose>
    <xsl:when test="@width &gt; 0">
      <xsl:call-template name="convertSizeSpecsToMM">
        <xsl:with-param name="theSpec" select="concat(@width,@unit)" />
      </xsl:call-template>
    </xsl:when>
    <xsl:otherwise>0</xsl:otherwise>
    </xsl:choose>
  </xsl:variable>

  <xsl:variable name="cellData.tf">
    <xsl:call-template name="collectCellData" />
  </xsl:variable>

  <xsl:variable name="cellData" 
    select="exsl:node-set($cellData.tf)"/>

  <xsl:value-of select="$newline"/>
  <xsl:if test="$embedded!=0">
    <xsl:text>{</xsl:text>
  </xsl:if>
  <xsl:if test="$topCaption='1'">
    <xsl:apply-templates select="$caption"/>
  </xsl:if>
  <xsl:text>\begin{</xsl:text>
  <xsl:value-of select="$tabularType"/>
  <xsl:text>}</xsl:text>
  <xsl:choose>
    <xsl:when test="@width &gt; 0">{<xsl:value-of select="$mmWidth" />mm}</xsl:when>
    <xsl:otherwise></xsl:otherwise>
  </xsl:choose>
  <xsl:choose>
    <xsl:when test="@valign = 'bottom'"><xsl:text>[b]</xsl:text></xsl:when>
    <xsl:when test="@valign = 'top'"><xsl:text>[t]</xsl:text></xsl:when>
    <xsl:otherwise><xsl:text>[c]</xsl:text></xsl:otherwise>
  </xsl:choose>

  <xsl:variable name="lastCol">
    <xsl:for-each select="$cellData//cellData">
      <xsl:sort select="number(@col)" data-type="number" order="descending" />
      <xsl:if test="position() = 1">
        <xsl:number value="number(@col)" />
      </xsl:if>
    </xsl:for-each>
  </xsl:variable>

  <xsl:variable name="preambleData.tf">
    <xsl:call-template name="getTablePreambleData">
      <xsl:with-param name="theCellData" select="$cellData" />
      <xsl:with-param name="whichCol" select="0" />
      <xsl:with-param name="lastCol" select="$lastCol" />
    </xsl:call-template>
  </xsl:variable>

  <xsl:variable name="preambleData" select="exsl:node-set($preambleData.tf)" />

  <xsl:text>{</xsl:text>
  <xsl:for-each select="$preambleData/columnData">
    <xsl:call-template name="outputPreamble">
      <xsl:with-param name="tabularType" select="$tabularType"/>
      <xsl:with-param name="columnData" select="." />
      <xsl:with-param name="whichCol" select="position() - 1" />
    </xsl:call-template>
  </xsl:for-each>
  <xsl:text>}</xsl:text>

  <xsl:call-template name="getHLineString">
    <xsl:with-param name="theRowData" select="$cellData/rowData[1]" />
    <xsl:with-param name="whichLine" select="'top'" />
  </xsl:call-template>

  <xsl:value-of select="$newline"/>

  <xsl:for-each select="$cellData/rowData">
    <xsl:for-each select="./cellData[(@cellID and string-length(normalize-space(@cellID))) or (@continuation='row')]">
      <xsl:call-template name="outputCell">
        <xsl:with-param name="theCell" select="$theTable//*[(local-name()='td') or (local-name()='th')][generate-id(.)=current()/@cellID]" />
        <xsl:with-param name="continuation" select="@continuation" />
        <xsl:with-param name="theCellData" select="." />
        <xsl:with-param name="colData" select="$preambleData" />
        <xsl:with-param name="positionInRow" select="position()" />
        <xsl:with-param name="tabularType" select="$tabularType"/>
      </xsl:call-template>
    </xsl:for-each>
    <xsl:choose>
      <xsl:when test="position() = last()">
        <xsl:variable name="lastHLine">
          <xsl:call-template name="getHLineString">
            <xsl:with-param name="theRowData" select="." />
            <xsl:with-param name="whichLine" select="'bottom'" />
          </xsl:call-template>
        </xsl:variable>
        <xsl:choose>
          <xsl:when test="string-length(normalize-space($lastHLine))">
            <xsl:text> \\</xsl:text>
            <xsl:value-of select="$newline"/>
            <xsl:value-of select="$lastHLine" />
          </xsl:when>
          <xsl:otherwise>
            <xsl:value-of select="$newline"/>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:otherwise>
        <xsl:text> \\</xsl:text>
        <xsl:value-of select="$newline"/>
         <xsl:call-template name="getHLineString">
          <xsl:with-param name="theRowData" select="." />
          <xsl:with-param name="whichLine" select="'bottom'" />
        </xsl:call-template>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:for-each>
  <xsl:text>\end{</xsl:text>
  <xsl:value-of select="$tabularType"/>
  <xsl:text>}</xsl:text>
  <xsl:if test="$bottomCaption='1'">
     <xsl:apply-templates select="$caption"/>
  </xsl:if>
  <xsl:if test="$embedded">
    <xsl:text>}</xsl:text>
  </xsl:if>
</xsl:template>    

<xsl:template match="html:table|mml:table">
  <xsl:choose>
    <xsl:when test="@pos='center'">
      <xsl:choose>
        <xsl:when test="html:caption">
      \begin{table}[h]
        </xsl:when>
        <xsl:otherwise>
          \begin{center}
        </xsl:otherwise>
      </xsl:choose>
      <xsl:call-template name="buildtable"/>
      <xsl:choose>
        <xsl:when test="html:caption">
      \end{table}
        </xsl:when>
        <xsl:otherwise>
          \end{center}
        </xsl:otherwise>
      </xsl:choose>
    </xsl:when>
    <xsl:when test="@pos='float' or ./html:caption"> 
      \begin{wraptable}{
      <xsl:choose>
        <xsl:when test="not(substring(@placement,1,1))">O</xsl:when>
        <xsl:otherwise><xsl:value-of select="substring(@placement,1,1)"/></xsl:otherwise>
      </xsl:choose>}
      {<xsl:choose><xsl:when test="@width"><xsl:value-of select="@width"/></xsl:when><xsl:otherwise>0pt</xsl:otherwise></xsl:choose>}
      <xsl:call-template name="buildtable"/>
      \end{wraptable}
    </xsl:when>
    <xsl:otherwise>
      <xsl:call-template name="buildtable"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>
		  
<xsl:template match="html:td//html:br">
   <xsl:text xml:space="preserve">\par </xsl:text>
</xsl:template> <!-- don't allow \\ in table data-->

<xsl:template match="html:td//*[self::html:br and position()=last()]" />

<xsl:template match="html:td//html:bodyText">
  <xsl:apply-templates/>
  <xsl:if test="following-sibling::*">
    <xsl:text xml:space="preserve">\par </xsl:text>
  </xsl:if>
</xsl:template>

<xsl:template match="html:td//html:bodyMath">
  <xsl:apply-templates/>
  <xsl:if test="following-sibling::*">
    <xsl:text xml:space="preserve">\par </xsl:text>
  </xsl:if>
</xsl:template>

<xsl:variable name="msipar"><xsl:text>\msipar</xsl:text></xsl:variable>
<xsl:variable name="dblBackslash"><xsl:text xml:space="preserve">\\ </xsl:text></xsl:variable>

<xsl:template match="html:td|html:th" mode="parbox">
  <xsl:variable name="normalOutput">
     <xsl:apply-templates/>
  </xsl:variable>
  <xsl:call-template name="doReplaceMacro">
    <xsl:with-param name="targStr" select="$normalOutput"/>
    <xsl:with-param name="findStr" select="$msipar"/>
    <xsl:with-param name="replaceStr" select="$dblBackslash"/>
    <xsl:with-param name="global" select="1"/>
  </xsl:call-template>
</xsl:template>

<xsl:template match="html:td|html:th|mml:td" mode="doOutput">
  <xsl:if test="@ccolor">
    <xsl:text>\cellcolor</xsl:text> 
    <xsl:choose>
      <xsl:when test="substring(./@ccolor,1,1)='#'">
	<xsl:text>[HTML]{</xsl:text>
	<xsl:value-of select="translate(substring(./@ccolor,2,8),'abcdef','ABCDEF')"/>
      </xsl:when>
      <xsl:otherwise> 
	<xsl:text>{</xsl:text>
	<xsl:value-of select="./@ccolor"/>
      </xsl:otherwise>
    </xsl:choose>
    <xsl:text>}</xsl:text>
  </xsl:if>
  <xsl:apply-templates/>
</xsl:template>

<xsl:template name="collectCellData">
  <xsl:variable name="rawCellData.tf">
    <xsl:call-template name="getCellData">
      <xsl:with-param name="rowList" select="./*[local-name()='tr'] | ./*[local-name()='thead' or local-name()='tbody' or local-name()='tfoot']/*[local-name()='tr']" />
      <xsl:with-param name="currRow" select="1" />
      <xsl:with-param name="currCol" select="1" />
    </xsl:call-template>
  </xsl:variable>
  <xsl:value-of select="$rawCellData.tf" />
  <xsl:variable name="rawCellData" select="exsl:node-set($rawCellData.tf)"/>
  <xsl:variable name="lastRow">
    <xsl:for-each select="$rawCellData/cellData[@row]" >
      <xsl:sort select="@row" data-type="number" order="descending" />
      <xsl:if test="position()=1">
        <xsl:number value="@row" />
      </xsl:if>
    </xsl:for-each>
  </xsl:variable>
  <xsl:variable name="lastCol">
    <xsl:for-each select="$rawCellData/cellData[@col]" >
      <xsl:sort select="@col" data-type="number" order="descending" />
      <xsl:if test="position()=1">
        <xsl:number value="@col" />
      </xsl:if>
    </xsl:for-each>
  </xsl:variable>
  <xsl:call-template name="refineCellData">
    <xsl:with-param name="rawCellData" select="$rawCellData" />
    <xsl:with-param name="rowList" select="./*[local-name()='tr'] | ./*[local-name()='thead' or local-name()='tbody' or local-name()='tfoot']/*[local-name()='tr']" />
    <xsl:with-param name="currRow" select="1" />
    <xsl:with-param name="lastRow" select="$lastRow" />
    <xsl:with-param name="lastCol" select="$lastCol" />
    <xsl:with-param name="tableBorderTop">
      <xsl:choose>
        <xsl:when test="@borderTop">
          <xsl:value-of select="@borderTop" />
        </xsl:when>
        <xsl:otherwise>
          <xsl:text>none</xsl:text>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:with-param>
    <xsl:with-param name="tableBorderRight">
      <xsl:choose>
        <xsl:when test="@borderRight">
          <xsl:value-of select="@borderRight" />
        </xsl:when>
        <xsl:otherwise>
          <xsl:text>none</xsl:text>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:with-param>
    <xsl:with-param name="tableBorderBottom">
      <xsl:choose>
        <xsl:when test="@borderBottom">
          <xsl:value-of select="@borderBottom" />
        </xsl:when>
        <xsl:otherwise>
          <xsl:text>none</xsl:text>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:with-param>
    <xsl:with-param name="tableBorderLeft">
      <xsl:choose>
        <xsl:when test="@borderLeft">
          <xsl:value-of select="@borderLeft" />
        </xsl:when>
        <xsl:otherwise>
          <xsl:text>none</xsl:text>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:with-param>
  </xsl:call-template>
</xsl:template>

<!--This returns an element for each column giving the value to be used for the alignment in that column and the value to be used
    for the right-hand vertical line. The first time through ("whichCol=0"), it returns an element giving the initial left-hand vertical line. -->
<xsl:template name="getTablePreambleData">
  <xsl:param name="theCellData" />
  <xsl:param name="whichCol" select="0" />
  <xsl:param name="lastCol" select="1" />
  <xsl:element name="columnData">
    <xsl:attribute name="lineSpec">
      <xsl:choose>
        <xsl:when test="$whichCol = 0">
          <xsl:for-each select="$theCellData//cellData[number(@col) = 1]">
            <xsl:sort select="count($theCellData//cellData[number(@col) = 1][@borderLeft = current()/@borderLeft])" data-type="number" order="descending" />
            <xsl:sort select="translate(@borderLeft, 'dsn', '123')" />
            <xsl:if test="position() = 1">
              <xsl:value-of select="@borderLeft" />
            </xsl:if>
          </xsl:for-each>
        </xsl:when>
        <xsl:otherwise>
          <xsl:for-each select="$theCellData//cellData[number(@col) = number($whichCol)]">
            <xsl:sort select="count($theCellData//cellData[number(@col) = number($whichCol)][@borderRight = current()/@borderRight])" data-type="number" order="descending" />
            <xsl:sort select="translate(@borderRight, 'dsn', '123')" />
            <xsl:if test="position() = 1">
              <xsl:value-of select="@borderRight" />
            </xsl:if>
          </xsl:for-each>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:attribute>

    <xsl:if test="not($whichCol = 0)">
      <xsl:attribute name="alignment">
        <xsl:choose>
          <xsl:when test="$theCellData//cellData[number(@col) = number($whichCol)][string-length(@width) &gt; 0]">
            <xsl:for-each select="$theCellData//cellData[number(@col) = number($whichCol)][string-length(@width) &gt; 0]" >
              <xsl:sort select="count($theCellData//cellData[number(@col) = number($whichCol)][string-length(@width) &gt; 0][@alignment=current()/@alignment])" data-type="number" order="descending" />
              <xsl:sort select="translate(@alignment, 'pmb', '123')" />
              <xsl:if test="position() = 1">
                <xsl:value-of select="@alignment" />
              </xsl:if>
            </xsl:for-each>
          </xsl:when>
          <xsl:otherwise>
            <xsl:for-each select="$theCellData//cellData[number(@col) = number($whichCol)][string-length(@alignment) &gt; 0]" >
              <xsl:sort select="count($theCellData//cellData[number(@col) = number($whichCol)][@alignment = current()/@alignment])" data-type="number" order="descending" />
              <xsl:sort select="translate(@alignment, 'lcrjpmb', '1234567')" />
              <xsl:if test="position() = 1">
                <xsl:value-of select="@alignment" />
              </xsl:if>
            </xsl:for-each>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:attribute>
      <xsl:attribute name="width">
        <xsl:if test="$theCellData//cellData[number(@col) = number($whichCol)][string-length(@width) &gt; 0]">
          <xsl:for-each select="$theCellData//cellData[number(@col) = number($whichCol)][string-length(@width) &gt; 0]" >
            <xsl:sort select="@width" data-type="number" order="descending" />
            <xsl:if test="position() = 1">
              <xsl:value-of select="@width" />
            </xsl:if>
          </xsl:for-each>
        </xsl:if>
      </xsl:attribute>
    </xsl:if>
  </xsl:element>

  <xsl:if test="$whichCol &lt; $lastCol">
    <xsl:call-template name="getTablePreambleData">
      <xsl:with-param name="theCellData" select="$theCellData" />
      <xsl:with-param name="whichCol" select="number($whichCol) + 1" />
      <xsl:with-param name="lastCol" select="$lastCol" />
    </xsl:call-template>
  </xsl:if>
</xsl:template>

<xsl:variable name="tabularAlignmentMap" select="'lcrj'" />
<xsl:variable name="tabularyAlignmentMap" select="'LCRJ'" />

<xsl:template name="outputPreamble">
  <xsl:param name="tabularType" select="tabular"/>
  <xsl:param name="columnData" />
  <xsl:param name="whichCol" select="0" />
  <xsl:if test="$columnData/@alignment">
    <xsl:choose>
      <xsl:when test="$tabularType='tabulary'">
        <xsl:value-of select="translate(substring(normalize-space($columnData/@alignment),1,1),'lcrj','LCRJ')" />
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="translate(substring(normalize-space($columnData/@alignment),1,1),'LCRJ','lcrj')" />
      </xsl:otherwise>
    </xsl:choose>
  </xsl:if>
  <xsl:if test="string-length($columnData/@width)">
    <xsl:text>p{</xsl:text>
    <xsl:value-of select="$columnData/@width" />
    <xsl:text>mm}</xsl:text>
  </xsl:if>
  <xsl:if test="(normalize-space($columnData/@lineSpec) = 'single') or (normalize-space($columnData/@lineSpec) = 'double')">
    <xsl:text>|</xsl:text>
    <xsl:if test="normalize-space($columnData/@lineSpec) = 'double'">
      <xsl:text>|</xsl:text>
    </xsl:if>
  </xsl:if>
</xsl:template>

<xsl:template name="getHLineString">
  <xsl:param name="theRowData" />
  <xsl:param name="whichLine" select="'bottom'" />
  <xsl:choose>
    <xsl:when test="$whichLine = 'top'">
      <xsl:choose>
        <xsl:when test="not($theRowData/cellData[not(@borderTop = 'single') and not(@borderTop = 'double')])" >
          <xsl:text xml:space="preserve">\hline
</xsl:text>
        </xsl:when>
        <xsl:when test="not($theRowData/cellData[(@borderTop = 'single') or (@borderTop = 'double')])" ></xsl:when>
        <xsl:otherwise>
          <xsl:call-template name="outputCLine">
            <xsl:with-param name="theRow" select="$theRowData" />
            <xsl:with-param name="currPos" select="1" />
            <xsl:with-param name="currState" select="0" />
            <xsl:with-param name="requiredForLine" select="'single'" />
            <xsl:with-param name="topOrBottom" select="'top'" />
          </xsl:call-template>
          <xsl:text xml:space="preserve">
</xsl:text>
        </xsl:otherwise>
      </xsl:choose>
      <xsl:choose>
        <xsl:when test="not($theRowData/cellData[not(@borderTop = 'double')])" >
          <xsl:text xml:space="preserve">\hline
</xsl:text>
        </xsl:when>
        <xsl:when test="not($theRowData/cellData[@borderTop = 'double'])" ></xsl:when>
        <xsl:otherwise>
          <xsl:call-template name="outputCLine">
            <xsl:with-param name="theRow" select="$theRowData" />
            <xsl:with-param name="currPos" select="1" />
            <xsl:with-param name="currState" select="0" />
            <xsl:with-param name="requiredForLine" select="'double'" />
            <xsl:with-param name="topOrBottom" select="'top'" />
          </xsl:call-template>
          <xsl:text xml:space="preserve">
</xsl:text>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:when>
    <xsl:otherwise>
      <xsl:choose>
        <xsl:when test="not($theRowData/cellData[not(@borderBottom = 'single') and not(@borderBottom = 'double')])" >
          <xsl:text xml:space="preserve">\hline
</xsl:text>
        </xsl:when>
        <xsl:when test="not($theRowData/cellData[(@borderBottom = 'single') or (@borderBottom = 'double')])" ></xsl:when>
        <xsl:otherwise>
          <xsl:call-template name="outputCLine">
            <xsl:with-param name="theRow" select="$theRowData" />
            <xsl:with-param name="currPos" select="1" />
            <xsl:with-param name="currState" select="0" />
            <xsl:with-param name="requiredForLine" select="'single'" />
            <xsl:with-param name="topOrBottom" select="'bottom'" />
          </xsl:call-template>
          <xsl:text xml:space="preserve">
</xsl:text>
        </xsl:otherwise>
      </xsl:choose>
      <xsl:choose>
        <xsl:when test="not($theRowData/cellData[not(@borderBottom = 'double')])" >
          <xsl:text xml:space="preserve">\hline
</xsl:text>
        </xsl:when>
        <xsl:when test="not($theRowData/cellData[@borderBottom = 'double'])" ></xsl:when>
        <xsl:otherwise>
          <xsl:call-template name="outputCLine">
            <xsl:with-param name="theRow" select="$theRowData" />
            <xsl:with-param name="currPos" select="1" />
            <xsl:with-param name="currState" select="0" />
            <xsl:with-param name="requiredForLine" select="'double'" />
            <xsl:with-param name="topOrBottom" select="'bottom'" />
          </xsl:call-template>
          <xsl:text xml:space="preserve">
</xsl:text>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template name="outputCLine">
  <xsl:param name="theRow" />
  <xsl:param name="currPos" select="1" />
  <xsl:param name="currState" select="0" />
  <xsl:param name="requiredForLine" select="'single'" />
  <xsl:param name="topOrBottom" select="bottom" />
  <xsl:variable name="theSpec">
    <xsl:choose>
      <xsl:when test="$topOrBottom = 'top'"><xsl:value-of select="$theRow/cellData[$currPos]/@borderTop" /></xsl:when>
      <xsl:otherwise><xsl:value-of select="$theRow/cellData[$currPos]/@borderBottom" /></xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <xsl:variable name="newState">
    <xsl:choose>
      <xsl:when test="($requiredForLine = 'single') and not($theSpec = 'none')">1</xsl:when>
      <xsl:when test="($requiredForLine = 'double') and ($theSpec = 'double')">1</xsl:when>
      <xsl:otherwise>0</xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <xsl:if test="not($newState = $currState)">
    <xsl:choose>
      <xsl:when test="$newState = 1">
        <xsl:text>\cline{</xsl:text><xsl:number value="$currPos" /><xsl:text>-</xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:number value="$currPos - 1" /><xsl:text>}</xsl:text>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:if>
  <xsl:choose>
    <xsl:when test="$theRow/cellData[number($currPos) + 1]">
      <xsl:call-template name="outputCLine">
        <xsl:with-param name="theRow" select="$theRow" />
        <xsl:with-param name="currPos" select="number($currPos) + 1" />
        <xsl:with-param name="currState" select="$newState" />
        <xsl:with-param name="requiredForLine" select="$requiredForLine" />
        <xsl:with-param name="topOrBottom" select="$topOrBottom" />
      </xsl:call-template>
    </xsl:when>
    <xsl:when test="$newState = 1">
      <xsl:number value="$currPos" /><xsl:text>}</xsl:text>
    </xsl:when>
    <xsl:otherwise>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template name="outputCell">
  <xsl:param name="theCell" />
  <xsl:param name="continuation" />
  <xsl:param name="theCellData" />
  <xsl:param name="colData" />
  <xsl:param name="positionInRow" select="1" />
  <xsl:param name="tabularType" select="$tabularType"/>

  <xsl:if test="$positionInRow &gt; 1">
    <xsl:text xml:space="preserve"> &amp; </xsl:text>
  </xsl:if>

  <xsl:if test="$theCell">
    <xsl:variable name="columnspan">
      <xsl:choose>
        <xsl:when test="$theCell/@colspan" ><xsl:number value="$theCell/@colspan"/></xsl:when>
        <xsl:otherwise>1</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:variable name="needMultiRow">
      <xsl:choose>
        <xsl:when test="$theCell/@rowspan and (number($theCell/@rowspan) &gt; 1)" >1</xsl:when>
        <xsl:otherwise>0</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:variable name="needMultiCol">
      <xsl:choose>
        <xsl:when test="$columnspan &gt; 1" >1</xsl:when>
        <xsl:otherwise>
          <xsl:variable name="colLeftLine" select="normalize-space($colData/columnData[number($theCellData/@col)]/@lineSpec)" />
          <xsl:variable name="colRightLine" select="normalize-space($colData/columnData[number($theCellData/@col) + 1]/@lineSpec)" />
          <xsl:variable name="colAlign" select="normalize-space($colData/columnData[number($theCellData/@col) + 1]/@alignment)" />
          <xsl:choose>
            <xsl:when test="not($theCellData/@borderLeft = $colLeftLine)">1</xsl:when>
            <xsl:when test="not($theCellData/@borderRight = $colRightLine)">1</xsl:when>
            <xsl:when test="not($theCellData/@alignment = $colAlign)">1</xsl:when>
            <xsl:otherwise>0</xsl:otherwise>
          </xsl:choose>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:variable name="needParbox">
      <xsl:choose>
        <xsl:when test="number($needMultiRow) or number($needMultiCol)">
          <xsl:call-template name="requiresParbox">
            <xsl:with-param name="theCell" select="$theCell" />
          </xsl:call-template>
        </xsl:when>
        <xsl:otherwise>0</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:choose>
      <xsl:when test="number($needMultiCol)">
        <xsl:text>\multicolumn{</xsl:text><xsl:number value="$columnspan" /><xsl:text>}{</xsl:text>
        <xsl:if test="($theCellData/@borderLeft = 'double') or ($theCellData/@borderLeft = 'single')">
            <xsl:text>|</xsl:text>
          <xsl:if test="$theCellData/@borderLeft = 'double'">
            <xsl:text>|</xsl:text>
          </xsl:if>
        </xsl:if>
        <xsl:choose>
          <xsl:when test="$theCellData/@alignment">
            <xsl:value-of select="translate(substring(normalize-space($theCellData/@alignment),1,1),'LCRJ', 'lcrj')" />
          </xsl:when>
          <xsl:otherwise>
            <xsl:text>l</xsl:text>
          </xsl:otherwise>
        </xsl:choose>
        <xsl:if test="string-length($theCellData/@width)">
          <xsl:text>{</xsl:text><xsl:value-of select="$theCellData/@width" /><xsl:text>mm}</xsl:text>
        </xsl:if>

        <xsl:variable name="rightBorder">
          <xsl:choose>
            <xsl:when test="$columnspan &gt; 1">
              <xsl:value-of select="$theCellData/following-sibling::cellData[$columnspan - 1]/@borderRight" />
            </xsl:when>
            <xsl:otherwise><xsl:value-of select="$theCellData/@borderRight" /></xsl:otherwise>
          </xsl:choose>
        </xsl:variable>

        <xsl:if test="($rightBorder = 'double') or ($rightBorder = 'single')">
            <xsl:text>|</xsl:text>
          <xsl:if test="$rightBorder = 'double'">
            <xsl:text>|</xsl:text>
          </xsl:if>
        </xsl:if>
        <xsl:text>}{</xsl:text>
      </xsl:when>
      <xsl:otherwise></xsl:otherwise>
    </xsl:choose>
    <xsl:if test="$needMultiRow != 0">
      <xsl:text>\multirow{</xsl:text>
      <xsl:number value="$theCell/@rowspan" />
      <xsl:text>}{</xsl:text>
      <xsl:choose>
        <xsl:when test="string-length($theCellData/@width)">
          <xsl:value-of select="$theCellData/@width" /><xsl:text>mm}{</xsl:text>
        </xsl:when>
        <xsl:otherwise><xsl:text>*}{</xsl:text></xsl:otherwise>
      </xsl:choose>
    </xsl:if>
    <xsl:choose>
      <xsl:when test="number($needParbox)">
        <xsl:text>\parbox{</xsl:text><xsl:call-template name="forceGetWidth">
          <xsl:with-param name="theCell" select="$theCell"/>
          <xsl:with-param name="colData" select="$colData"/>
          <xsl:with-param name="whichCol" select="number($theCellData/@col)" />
        </xsl:call-template>
        <xsl:text>mm}{</xsl:text>
        <xsl:apply-templates select="$theCell" mode="parbox" />
        <xsl:text>}</xsl:text>
      </xsl:when>
      <xsl:otherwise>
        <xsl:apply-templates select="$theCell" mode="doOutput" />
      </xsl:otherwise>
    </xsl:choose>
    <xsl:if test="$needMultiRow != 0">
      <xsl:text>}</xsl:text>
    </xsl:if>
    <xsl:if test="number($needMultiCol)">
      <xsl:text>}</xsl:text>
    </xsl:if>
  </xsl:if>
</xsl:template>

<xsl:template name="requiresParbox">
  <xsl:param name="theCell" />
  <xsl:choose>
    <xsl:when test="$theCell//*[contains($envList, concat('__',local-name(),'__'))]">1</xsl:when>
    <xsl:when test="$theCell//*[contains($paraList, concat('__',local-name(),'__'))][following-sibling::*]">1</xsl:when>
    <xsl:when test="$theCell//html:br[following-sibling::*]">1</xsl:when>
    <xsl:otherwise>0</xsl:otherwise>
  </xsl:choose>
</xsl:template>

<!-- The following appears unduly complicated; the idea is to try to give a width for a cell that won't mess up other cells
     and will still allow the contents to have room. The algorithm is to use the total table width if given, or a guess
     depending on how many columns there are if not; to deduct the width of columns which have cells with hard-wired widths;
     then to find a way to compromise on the distribution of the rest of the width without computing the layout of the
     whole table. A max width and min width are decided on based on how much leeway there appears to be, and a crude
     guess as to the size needed (counting text length, and assuming images need their whole widths on one line) is
     compared to the max and min, finally resulting in a number. --> 
<xsl:template name="forceGetWidth">
  <xsl:param name="theCell" />
  <xsl:param name="colData" />
  <xsl:param name="whichCol" />
  <xsl:choose>
    <xsl:when test="$theCell/@width"><xsl:value-of select="$theCell/@width"/></xsl:when>
    <xsl:otherwise>
      <xsl:variable name="theTable" select="$theCell/ancestor::*[local-name()='table'][1]" />
      <xsl:variable name="fullTableWidth">
        <xsl:choose>
          <xsl:when test="$theTable/@width">
            <xsl:call-template name="convertSizeSpecsToMM">
              <xsl:with-param name="theSpec" select="concat($theTable/@width,$theTable/@unit)" />
            </xsl:call-template>
          </xsl:when>
          <xsl:otherwise>0</xsl:otherwise>
        </xsl:choose>
      </xsl:variable>
      <xsl:variable name="numCols">
        <xsl:choose>
          <xsl:when test="$theCell/@colspan"><xsl:value-of select="number($theCell/@colspan)"/></xsl:when>
          <xsl:otherwise>1</xsl:otherwise>
        </xsl:choose>
      </xsl:variable>
      <xsl:variable name="fixedColWidth">
        <xsl:choose>
          <xsl:when test="$numCols &lt; (count($colData/columnData) - 1)">
            <xsl:value-of select="sum($colData/columnData[(position() &gt; $whichCol) and (position() &lt; ($whichCol + 1 + $numCols))][number(@width)]/@width)"/>
          </xsl:when>
          <xsl:otherwise><xsl:value-of select="number($fullTableWidth)"/></xsl:otherwise>
        </xsl:choose>
      </xsl:variable>
      <xsl:variable name="numVariableCols">
        <xsl:choose>
          <xsl:when test="$numCols &lt; (count($colData/columnData) - 1)">
            <xsl:value-of select="count($colData/columnData[(position() &gt; $whichCol) and (position() &lt; ($whichCol + 1 + $numCols))][not(number(@width) &gt; 0)])"/>
          </xsl:when>
          <xsl:otherwise>0</xsl:otherwise>
        </xsl:choose>
      </xsl:variable>
      <xsl:variable name="unavailableWidth">
        <xsl:choose>
          <xsl:when test="$colData/columnData[number(@width)]">
            <xsl:value-of select="sum($colData/columnData[number(@width)]/@width)" />
          </xsl:when>
          <xsl:otherwise>0</xsl:otherwise>
        </xsl:choose>
      </xsl:variable>
      <xsl:variable name="numColsUsing">
        <xsl:choose>
          <xsl:when test="count($colData/columnData[not(number(@width) &gt; 0)]) &gt; 1"><xsl:value-of select="count($colData/columnData[not(number(@width) &gt; 0)]) - 1"/></xsl:when>
          <xsl:otherwise>1</xsl:otherwise>
        </xsl:choose>
      </xsl:variable>
      <xsl:variable name="availableWidth">
        <xsl:choose>
          <xsl:when test="number($fullTableWidth)">
            <xsl:value-of select="number($fullTableWidth) - number($unavailableWidth)" />
          </xsl:when>
          <xsl:when test="($unavailableWidth &gt; 70) or ($numColsUsing &gt; 4)">  <!-- 70mm = 200pt -->
            <xsl:value-of select="175 - number($unavailableWidth)" />  <!-- 175mm = 500pt -->
          </xsl:when>
          <xsl:when test="($unavailableWidth &gt; 35) or ($numColsUsing &gt; 3)">  <!-- 35mm = 100pt -->
            <xsl:value-of select="140 - number($unavailableWidth)" />  <!-- 140mm = 400pt -->
          </xsl:when>
          <xsl:otherwise>
            <xsl:value-of select="70 - number($unavailableWidth)" />  <!-- 70mm = 200pt -->
          </xsl:otherwise>
        </xsl:choose>
      </xsl:variable>
      <xsl:variable name="avgWidth" select="$numVariableCols * (number($availableWidth) div $numColsUsing)" />
      <xsl:variable name="calcMinWidth">
        <xsl:choose>
          <xsl:when test="$numColsUsing &gt; 4"><xsl:value-of select="$fixedColWidth + (.5 * $avgWidth)"/></xsl:when>
          <xsl:when test="$numColsUsing &gt; 2"><xsl:value-of select="$fixedColWidth + (.4 * $avgWidth)"/></xsl:when>
          <xsl:otherwise><xsl:value-of select="$fixedColWidth + (.3 * $avgWidth)"/></xsl:otherwise>
        </xsl:choose>
      </xsl:variable>
      <xsl:variable name="minWidth">
        <xsl:choose>
          <xsl:when test="$calcMinWidth &lt; 7">7</xsl:when>
          <xsl:otherwise><xsl:value-of select="$calcMinWidth"/></xsl:otherwise>
        </xsl:choose>
      </xsl:variable>
      <xsl:variable name="calcMaxWidth">
        <xsl:choose>
          <xsl:when test="$numColsUsing &gt; 4"><xsl:value-of select="$fixedColWidth + (2.5 * $avgWidth)"/></xsl:when>
          <xsl:when test="$numColsUsing=4"><xsl:value-of select="$fixedColWidth + (2 * $avgWidth)"/></xsl:when>
          <xsl:when test="$numColsUsing=3"><xsl:value-of select="$fixedColWidth + (1.5 * $avgWidth)"/></xsl:when>
          <xsl:otherwise><xsl:value-of select="$fixedColWidth + (1.33 * $avgWidth)"/></xsl:otherwise>
        </xsl:choose>
      </xsl:variable>
      <xsl:variable name="maxWidth">
        <xsl:choose>
          <xsl:when test="$calcMaxWidth &gt; (number($availableWidth) - 7 * ($numColsUsing - $numVariableCols))">
            <xsl:value-of select="number($availableWidth) - 7 * ($numColsUsing - $numVariableCols)" />
          </xsl:when>
          <xsl:otherwise><xsl:value-of select="$calcMaxWidth"/></xsl:otherwise>
        </xsl:choose>
      </xsl:variable>
      <xsl:variable name="guessedParaWidth">
        <xsl:for-each select="$theCell//*[contains($paraList, concat('__',local-name(),'__'))]">
          <xsl:sort select="string-length(string(.))" data-type="number" order="descending" />
          <xsl:if test="position()=1"><xsl:value-of select="2.8 * string-length(string(.))"/></xsl:if>  <!-- 2.8mm = 8pt -->
        </xsl:for-each>
      </xsl:variable>
      <xsl:variable name="minParaWidth">
        <xsl:choose>
          <xsl:when test="$theCell//*[local-name()='img' or local-name()='object']">
            <xsl:for-each select="$theCell//*[local-name()='img' or local-name()='object']">
              <xsl:sort select="number(@width)" data-type="number" order="descending" />
              <xsl:if test="position()=1">
                <xsl:call-template name="convertSizeSpecsToMM">
                  <xsl:with-param name="theSpec" select="concat(@width,@unit)" />
                </xsl:call-template>
              </xsl:if>
            </xsl:for-each>
          </xsl:when>
          <xsl:otherwise>0</xsl:otherwise>
        </xsl:choose>
      </xsl:variable>
      <!-- xsl:text xml:space="preserve"> - numColsUsing is </xsl:text><xsl:value-of select="$numColsUsing"/>
      <xsl:text xml:space="preserve">, unavailableWidth is </xsl:text><xsl:value-of select="$unavailableWidth"/>
      <xsl:text xml:space="preserve">, avgWidth is </xsl:text><xsl:value-of select="$avgWidth"/>
      <xsl:text xml:space="preserve">, numVariableCols is </xsl:text><xsl:value-of select="$numVariableCols"/>
      <xsl:text xml:space="preserve">, fixedColWidth is </xsl:text><xsl:value-of select="$fixedColWidth"/>
      <xsl:text xml:space="preserve">, fullTableWidth is </xsl:text><xsl:value-of select="$fullTableWidth"/>
      <xsl:text xml:space="preserve">, numColsUsing should be 1 less than </xsl:text><xsl:value-of select="count($colData/columnData[not(number(@width) &gt; 0)])"/>
      <xsl:text xml:space="preserve">guessedParaWidth is </xsl:text><xsl:value-of select="$guessedParaWidth"/>
      <xsl:text xml:space="preserve">, minParaWidth is </xsl:text><xsl:value-of select="$minParaWidth"/>
      <xsl:text xml:space="preserve">
  </xsl:text -->
      <xsl:choose>
        <xsl:when test="($guessedParaWidth + $minParaWidth) &lt; $minWidth"><xsl:value-of select="$minWidth"/></xsl:when>
        <xsl:when test="($guessedParaWidth + $minParaWidth) &lt; $maxWidth"><xsl:value-of select="$guessedParaWidth + $minParaWidth"/></xsl:when>
        <xsl:when test="$minParaWidth &gt; $maxWidth"><xsl:value-of select="$minParaWidth"/></xsl:when>
        <xsl:when test="($guessedParaWidth div $maxWidth) &gt; 4"><xsl:value-of select="$maxWidth"/></xsl:when>
        <xsl:when test="($guessedParaWidth div $maxWidth) &gt; 2"><xsl:value-of select="(.3 * $avgWidth) + (.7 * $maxWidth)"/></xsl:when>
        <xsl:otherwise><xsl:value-of select="(.6 * $avgWidth) + (.4 * $maxWidth)"/></xsl:otherwise>
      </xsl:choose>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:variable name="rowContainersStr" select="'--thead--tfoot--tbody--'" />

<xsl:attribute-set name="cellDataAttrs">
  <xsl:attribute name="row"><xsl:value-of select="@row" /></xsl:attribute>
  <xsl:attribute name="col"><xsl:value-of select="@col" /></xsl:attribute>
  <xsl:attribute name="cellID"><xsl:value-of select="@cellID" /></xsl:attribute>
  <xsl:attribute name="alignment"><xsl:value-of select="@alignment" /></xsl:attribute>
  <xsl:attribute name="continuation"><xsl:value-of select="@continuation" /></xsl:attribute>
  <xsl:attribute name="width"><xsl:value-of select="@width" /></xsl:attribute>
</xsl:attribute-set>

<!--The point of this is to resolve linestyle conflicts at shared edges and to translate line style specifiers from HTML form
    to "LaTeX" form (either 'none', 'single', or 'double'). -->
    
<xsl:template name="refineCellData">
  <xsl:param name="rawCellData" />
  <xsl:param name="rowList" />
  <xsl:param name="currRow" select="1" />
  <xsl:param name="lastRow" select="1" />
  <xsl:param name="lastCol" select="1" />
  <xsl:param name="tableBorderTop" select="'none'" />
  <xsl:param name="tableBorderRight" select="'none'" />
  <xsl:param name="tableBorderBottom" select="'none'" />
  <xsl:param name="tableBorderLeft" select="'none'" />
  <xsl:element name="rowData">
    <xsl:attribute name="rowID"><xsl:value-of select="generate-id($rowList[number($currRow)])" /></xsl:attribute>
    <xsl:for-each select="$rawCellData/*[number(@row)=number($currRow)]">
      <xsl:sort select="@col" data-type="number" />
      <xsl:variable name="currCol" select="number(@col)" />
      <xsl:copy select="." use-attribute-sets="cellDataAttrs" >
        <xsl:attribute name="borderTop">
          <xsl:call-template name="translateBorderStyle">
            <xsl:with-param name="htmlStyle">
              <xsl:call-template name="combineLineSpecs">
                <xsl:with-param name="spec1">
                  <xsl:choose>
                    <xsl:when test="$currRow=1"><xsl:value-of select="$tableBorderTop" /></xsl:when>
                    <xsl:otherwise>
                      <xsl:value-of select="$rawCellData/*[number(@row) = $currRow - 1][@col=current()/@col]/@borderBottom" />
                    </xsl:otherwise>
                  </xsl:choose>
                </xsl:with-param>
                <xsl:with-param name="spec2"><xsl:value-of select="@borderTop" /></xsl:with-param>
              </xsl:call-template>
            </xsl:with-param>
          </xsl:call-template>
        </xsl:attribute>
        <xsl:attribute name="borderRight">
          <xsl:call-template name="translateBorderStyle">
            <xsl:with-param name="htmlStyle">
              <xsl:call-template name="combineLineSpecs">
                <xsl:with-param name="spec1">
                  <xsl:choose>
                    <xsl:when test="$lastCol = $currCol"><xsl:value-of select="$tableBorderRight" /></xsl:when>
                    <xsl:otherwise>
                      <xsl:value-of select="$rawCellData/*[number(@row) = $currRow][number(@col) = $currCol + 1]/@borderLeft" />
                    </xsl:otherwise>
                  </xsl:choose>
                </xsl:with-param>
                <xsl:with-param name="spec2"><xsl:value-of select="@borderRight" /></xsl:with-param>
              </xsl:call-template>
            </xsl:with-param>
          </xsl:call-template>
        </xsl:attribute>
        <xsl:attribute name="borderBottom">
          <xsl:call-template name="translateBorderStyle">
            <xsl:with-param name="htmlStyle">
              <xsl:call-template name="combineLineSpecs">
                <xsl:with-param name="spec1">
                  <xsl:choose>
                    <xsl:when test="$lastRow = $currRow"><xsl:value-of select="$tableBorderBottom" /></xsl:when>
                    <xsl:otherwise>
                      <xsl:value-of select="$rawCellData/*[number(@row) = $currRow + 1][number(@col) = $currCol]/@borderTop" />
                    </xsl:otherwise>
                  </xsl:choose>
                </xsl:with-param>
                <xsl:with-param name="spec2"><xsl:value-of select="@borderBottom" /></xsl:with-param>
              </xsl:call-template>
            </xsl:with-param>
          </xsl:call-template>
        </xsl:attribute>
        <xsl:attribute name="borderLeft">
          <xsl:call-template name="translateBorderStyle">
            <xsl:with-param name="htmlStyle">
              <xsl:call-template name="combineLineSpecs">
                <xsl:with-param name="spec1">
                  <xsl:choose>
                    <xsl:when test="$currCol = 1"><xsl:value-of select="$tableBorderLeft" /></xsl:when>
                    <xsl:otherwise>
                      <xsl:value-of select="$rawCellData/*[number(@row) = $currRow][number(@col) = $currCol - 1]/@borderRight" />
                    </xsl:otherwise>
                  </xsl:choose>
                </xsl:with-param>
                <xsl:with-param name="spec2"><xsl:value-of select="@borderLeft" /></xsl:with-param>
              </xsl:call-template>
            </xsl:with-param>
          </xsl:call-template>
        </xsl:attribute>
      </xsl:copy>
      <xsl:if test="(position()=last()) and (position() &lt; $lastCol)">
        <xsl:call-template name="generateEmptyCells">
          <xsl:with-param name="numToGenerate" select="$lastCol - position()" />
          <xsl:with-param name="currRow" select="$currRow" />
          <xsl:with-param name="currCol" select="$currCol + 1" />
        </xsl:call-template>
      </xsl:if>
    </xsl:for-each>
  </xsl:element>
  <xsl:if test="$currRow &lt; $lastRow">
    <xsl:call-template name="refineCellData">
      <xsl:with-param name="rawCellData" select="$rawCellData" />
      <xsl:with-param name="rowList" select="$rowList" />
      <xsl:with-param name="currRow" select="number($currRow)+1" />
      <xsl:with-param name="lastRow" select="$lastRow" />
      <xsl:with-param name="lastCol" select="$lastCol" />
      <xsl:with-param name="tableBorderTop" select="$tableBorderTop" />
      <xsl:with-param name="tableBorderRight" select="$tableBorderRight" />
      <xsl:with-param name="tableBorderBottom"  select="$tableBorderBottom" />
      <xsl:with-param name="tableBorderLeft"  select="$tableBorderLeft" />
    </xsl:call-template>
  </xsl:if>
</xsl:template>

<!-- The reason for and logic behind the following functions is essentially:
     i) No information can be set up or used regarding border lines or alignment without knowing what row and column a
        particular cell starts (and ends) in.
    ii) With the possibility of multi-row cells, the only way to determine the starting column for a cell is to have already
        traversed the preceding rows to determine what rows and columns they occupy. That is, the information being determined
        for each cell depends on knowing at that point the information for preceding cells.
   iii) In XSLT, this accumulating of information as you go can't be done in a for-each loop, since any variable assigned
        to collect it can't then be modified (and I can't see any viable way to concatenate it in general in order to construct
        "with-param" values). The only usable approach seems to be recursively calling a function from itself.
    iv) Results returned as nodes or node-sets wouldn't be able to be queried until the variables holding them were fully defined
        (for instance, if you're still constructing an element named <cellsData>, you wouldn't be able to select 
        "cellsData/tr/td[@rowspan &gt; 1]" while constructing the data for a later cell). This seems to force constructing the
        information first as strings, which can be passed and queried. The semicolon-separated strings will take the form of 
        "(2,3)(#ID)(L)(single,none,double,none)" for a normal cell or "(2,4)()()(single,none,single,single)" if the cell is a continuation cell; 
        the first entry gives the (row,column), the second if not empty is the generate-id() of the relevant <td> or <th>, 
        the third gives the alignment specification, and the fourth gives the (top, right, bottom, left) line specifiers.
     -->

<xsl:template name="getCellData">
  <xsl:param name="prevList" select="''" />
  <xsl:param name="rowList" />
  <xsl:param name="currRow" select="1" />
  <xsl:param name="currCol" select="1" />
  <xsl:variable name="cellDataString">
    <xsl:for-each select="$rowList">
      <xsl:if test="position()=1">
        <xsl:call-template name="getCellDataStringForRow">
          <xsl:with-param name="theRowList" select="$rowList" />
          <xsl:with-param name="prevList" select="$prevList" />
          <xsl:with-param name="currRow" select="$currRow" />
          <xsl:with-param name="currCol" select="$currCol" />
        </xsl:call-template>
      </xsl:if>
    </xsl:for-each>
  </xsl:variable>
  <xsl:call-template name="constructCellDataFromString">
    <xsl:with-param name="dataString" select="$cellDataString" />
  </xsl:call-template>
</xsl:template>

<xsl:template name="getCellDataStringForRow">
  <xsl:param name="theRowList" />
  <xsl:param name="prevList" />
  <xsl:param name="currRow" />
  <xsl:param name="currCol" />
  <xsl:variable name="newList">
    <xsl:for-each select="$theRowList">
      <xsl:if test="position()=1">
        <xsl:call-template name="getDataStringForCells">
          <xsl:with-param name="theCellList" select="./*[local-name()='td' or local-name()='th']" />
          <xsl:with-param name="prevList" select="$prevList" />
          <xsl:with-param name="currRow" select="$currRow" />
          <xsl:with-param name="currCol" select="$currCol" />
        </xsl:call-template>
      </xsl:if>
    </xsl:for-each>
  </xsl:variable>
  <xsl:choose>
    <xsl:when test="count($theRowList) &gt; 1">
      <xsl:call-template name="getCellDataStringForRow">
        <xsl:with-param name="theRowList" select="$theRowList[position() &gt; 1]" />
        <xsl:with-param name="prevList" select="$newList" />
        <xsl:with-param name="currRow" select="number($currRow) + 1" />
        <xsl:with-param name="currCol" select="1" />
      </xsl:call-template>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="$newList" />
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template name="getDataStringForCells">
  <xsl:param name="theCellList"/>
  <xsl:param name="prevList"/>
  <xsl:param name="currRow"/>
  <xsl:param name="currCol" select="1"/>
  <xsl:variable name="thisCell" select="$theCellList[1]" />
  <xsl:variable name="rowSpan">
    <xsl:choose>
      <xsl:when test="$thisCell/@rowspan">
        <xsl:value-of select="$thisCell/@rowspan" />
      </xsl:when>
      <xsl:otherwise>1</xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <xsl:variable name="colSpan">
    <xsl:choose>
      <xsl:when test="$thisCell/@colspan">
        <xsl:value-of select="$thisCell/@colspan" />
      </xsl:when>
      <xsl:otherwise>1</xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <xsl:variable name="theCol">
    <xsl:call-template name="findCellNotInString">
      <xsl:with-param name="theString" select="$prevList" />
      <xsl:with-param name="theRow" select="$currRow" />
      <xsl:with-param name="startCol" select="$currCol" />
    </xsl:call-template>
  </xsl:variable>
  <xsl:variable name="allLines">
    <xsl:choose>
      <xsl:when test="$thisCell/@lines">
        <xsl:value-of select="$thisCell/@lines" />
      </xsl:when>
      <xsl:otherwise><xsl:text>none</xsl:text></xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <xsl:variable name="newList">
    <xsl:call-template name="outputCellData">
      <xsl:with-param name="borderTop">
        <xsl:choose>
          <xsl:when test="$thisCell/@line-top">
            <xsl:value-of select="$thisCell/@line-top" />
          </xsl:when>
          <xsl:otherwise><xsl:value-of select="$allLines" /></xsl:otherwise>
        </xsl:choose>
      </xsl:with-param>
      <xsl:with-param name="borderRight">
        <xsl:choose>
          <xsl:when test="$thisCell/@line-right">
            <xsl:value-of select="$thisCell/@line-right" />
          </xsl:when>
          <xsl:otherwise><xsl:value-of select="$allLines" /></xsl:otherwise>
        </xsl:choose>
      </xsl:with-param>
      <xsl:with-param name="borderBottom">
        <xsl:choose>
          <xsl:when test="$thisCell/@line-bottom">
            <xsl:value-of select="$thisCell/@line-bottom" />
          </xsl:when>
          <xsl:otherwise><xsl:value-of select="$allLines" /></xsl:otherwise>
        </xsl:choose>
      </xsl:with-param>
      <xsl:with-param name="borderLeft">
        <xsl:choose>
          <xsl:when test="$thisCell/@line-left">
            <xsl:value-of select="$thisCell/@line-left" />
          </xsl:when>
          <xsl:otherwise><xsl:value-of select="$allLines" /></xsl:otherwise>
        </xsl:choose>
      </xsl:with-param>
      <xsl:with-param name="startRow" select="$currRow" />
      <xsl:with-param name="startCol" select="$theCol" />
      <xsl:with-param name="numRows" select="$rowSpan" />
      <xsl:with-param name="numCols" select="$colSpan" />
      <xsl:with-param name="cellId" select="generate-id($thisCell)" />
      <xsl:with-param name="alignment">
        <xsl:choose>
          <xsl:when test="string-length($thisCell/@cellwidth)">
            <xsl:choose>
              <xsl:when test="$thisCell/@valign='center'">middle</xsl:when>
              <xsl:when test="$thisCell/@valign='bottom'">bottom</xsl:when>
              <xsl:otherwise>para</xsl:otherwise>
            </xsl:choose>
          </xsl:when>
          <xsl:when test="$thisCell/@align"><xsl:value-of select="$thisCell/@align" /></xsl:when>
          <xsl:otherwise><xsl:text>left</xsl:text></xsl:otherwise>
        </xsl:choose>
      </xsl:with-param>
      <xsl:with-param name="widthSpec">
        <xsl:if test="($colSpan = 1) and string-length($thisCell/@cellwidth)">
          <xsl:call-template name="convertSizeSpecsToMM">
            <xsl:with-param name="theSpec" select="$thisCell/@cellwidth" />
          </xsl:call-template>
        </xsl:if>
      </xsl:with-param>
    </xsl:call-template>
  </xsl:variable>
  <xsl:choose>
    <xsl:when test="count($theCellList) &gt; 1">
      <xsl:call-template name="getDataStringForCells">
        <xsl:with-param name="theCellList" select="$theCellList[position() &gt; 1]" />
        <xsl:with-param name="prevList" select="concat($prevList, $newList)" />
        <xsl:with-param name="currRow" select="$currRow" />
        <xsl:with-param name="currCol" select="number($theCol) + number($colSpan)" />
      </xsl:call-template>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="$prevList" /><xsl:value-of select="$newList" />
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:variable name="doubleBorderStyleString"><xsl:text>--double--groove--ridge--</xsl:text></xsl:variable>
<xsl:variable name="singleBorderStyleString"><xsl:text>--dotted--dashed--solid--inset--outset--</xsl:text></xsl:variable>

<xsl:template name="translateBorderStyle">
  <xsl:param name="htmlStyle" select="none" />
  <xsl:choose>
    <xsl:when test="contains($doubleBorderStyleString, concat('--',$htmlStyle,'--'))">
      <xsl:text>double</xsl:text>
    </xsl:when>
    <xsl:when test="contains($singleBorderStyleString, concat('--',$htmlStyle,'--'))">
      <xsl:text>single</xsl:text>
    </xsl:when>
    <xsl:otherwise>
      <xsl:text>none</xsl:text>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template name="combineLineSpecs">
  <xsl:param name="spec1" select="'none'" />
  <xsl:param name="spec2" select="'none'" />
  <xsl:choose>
    <xsl:when test="$spec1='hidden' or $spec2='hidden'">
      <xsl:text>hidden</xsl:text>
    </xsl:when>
    <xsl:when test="contains($doubleBorderStyleString, concat('--',$spec1,'--'))">
      <xsl:value-of select="$spec1" />
    </xsl:when>
    <xsl:when test="contains($doubleBorderStyleString, concat('--',$spec2,'--'))">
      <xsl:value-of select="$spec2" />
    </xsl:when>
    <xsl:when test="contains($singleBorderStyleString, concat('--',$spec1,'--'))">
      <xsl:value-of select="$spec1" />
    </xsl:when>
    <xsl:when test="contains($singleBorderStyleString, concat('--',$spec2,'--'))">
      <xsl:value-of select="$spec2" />
    </xsl:when>
    <xsl:otherwise>
      <xsl:text>none</xsl:text>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template name="findCellNotInString">
  <xsl:param name="theString" />
  <xsl:param name="startCol" select="1" />
  <xsl:param name="theRow" />
  <xsl:choose>
    <xsl:when test="not(contains($theString, concat('(', $theRow, ',', $startCol, ')')))">
      <xsl:value-of select="$startCol" />
    </xsl:when>
    <xsl:otherwise>
      <xsl:call-template name="findCellNotInString">
        <xsl:with-param name="theString" select="$theString" />
        <xsl:with-param name="startCol" select="number($startCol) + 1" />
        <xsl:with-param name="theRow" select="$theRow" />
      </xsl:call-template>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template name="convertSizeSpecsToMM">
  <xsl:param name="theSpec" />
  <xsl:variable name="unitStr" select="translate($theSpec,'0123456789.','')" />
  <xsl:variable name="numberStr">
    <xsl:choose>
      <xsl:when test="string-length($unitStr)"><xsl:value-of select="substring-before($theSpec, $unitStr)" /></xsl:when>
      <xsl:otherwise><xsl:value-of select="$theSpec" /></xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <xsl:choose>
    <xsl:when test="$unitStr = 'mm'"><xsl:value-of select="$numberStr" /></xsl:when>
    <xsl:when test="$unitStr = 'cm'"><xsl:value-of select="$numberStr * 10.0" /></xsl:when>
    <xsl:when test="$unitStr = 'in'"><xsl:value-of select="$numberStr * 25.4" /></xsl:when>
    <xsl:when test="$unitStr = 'pt'"><xsl:value-of select="$numberStr * 0.3514598" /></xsl:when>
    <xsl:when test="$unitStr = 'bp'"><xsl:value-of select="$numberStr * 0.3514598" /></xsl:when>
    <xsl:when test="$unitStr = 'pc'"><xsl:value-of select="$numberStr * 2.54" /></xsl:when>
    <xsl:when test="$unitStr = 'px'"><xsl:value-of select="$numberStr * 0.2645833" /></xsl:when>
    <xsl:otherwise><xsl:value-of select="$numberStr * 0.2645833" /></xsl:otherwise>  <!-- assume pixels -->
  </xsl:choose>
</xsl:template>

<xsl:template name="outputCellData">
  <xsl:param name="borderTop" select="'none'" />
  <xsl:param name="borderRight" select="'none'" />
  <xsl:param name="borderBottom" select="'none'" />
  <xsl:param name="borderLeft" select="'none'" />
  <xsl:param name="startRow" />
  <xsl:param name="startCol" />
  <xsl:param name="numRows" select="1" />
  <xsl:param name="numCols" select="1" />
  <xsl:param name="cellId" />
  <xsl:param name="alignment" />
  <xsl:param name="continuation" />
  <xsl:param name="widthSpec" select="''" />
  <xsl:text>(</xsl:text>
  <xsl:value-of select="$startRow" />
  <xsl:text>,</xsl:text>
  <xsl:value-of select="$startCol" />
  <xsl:text>)(</xsl:text>
  <xsl:choose>
    <xsl:when test="$cellId and string-length($cellId)">
      <xsl:text>#</xsl:text>
      <xsl:value-of select="$cellId" />
    </xsl:when>
    <xsl:when test="$continuation and string-length($continuation)">
      <xsl:value-of select="$continuation" /><xsl:text>cont</xsl:text>
    </xsl:when>
    <xsl:otherwise></xsl:otherwise>
  </xsl:choose>
  <xsl:text>)(</xsl:text>
  <xsl:if test="$alignment">
    <xsl:value-of select="$alignment" />
  </xsl:if>
  <xsl:text>,</xsl:text>
  <xsl:if test="string-length($widthSpec)"><xsl:value-of select="$widthSpec" /></xsl:if>
  <xsl:text>)(</xsl:text>
  <!-- Now put the border attributes -->
  <xsl:value-of select="$borderTop" />
  <xsl:text>,</xsl:text>
  <xsl:choose>
    <xsl:when test="number($numCols) &gt; 1">
      <xsl:text>none</xsl:text>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="$borderRight" />
    </xsl:otherwise>
  </xsl:choose>
  <xsl:text>,</xsl:text>
  <xsl:choose>
    <xsl:when test="number($numRows) &gt; 1">
      <xsl:text>none</xsl:text>
    </xsl:when>
    <xsl:otherwise>
      <xsl:value-of select="$borderBottom" />
    </xsl:otherwise>
  </xsl:choose>
  <xsl:text>,</xsl:text>
  <xsl:value-of select="$borderLeft" />
  <xsl:text>);</xsl:text>
  <xsl:if test="number($numRows) &gt; 1">
    <xsl:call-template name="outputCellData">
      <xsl:with-param name="borderTop" select="'none'" />
      <xsl:with-param name="borderRight" select="$borderRight" />
      <xsl:with-param name="borderBottom" select="$borderBottom" />
      <xsl:with-param name="borderLeft" select="$borderLeft" />
      <xsl:with-param name="startRow" select="number($startRow)+1" />
      <xsl:with-param name="startCol" select="$startCol" />
      <xsl:with-param name="numRows" select="number($numRows) - 1" />
      <xsl:with-param name="numCols" select="1" />
      <xsl:with-param name="continuation" select="'row'" />
      <xsl:with-param name="widthSpec" select="$widthSpec" />
    </xsl:call-template>
  </xsl:if>
  <xsl:if test="number($numCols) &gt; 1">
    <xsl:call-template name="outputCellData">
      <xsl:with-param name="borderTop" select="$borderTop" />
      <xsl:with-param name="borderRight" select="$borderRight" />
      <xsl:with-param name="borderBottom" select="$borderBottom" />
      <xsl:with-param name="borderLeft" select="'none'" />
      <xsl:with-param name="startRow" select="$startRow" />
      <xsl:with-param name="startCol" select="number($startCol) + 1" />
      <xsl:with-param name="numRows" select="$numRows" />
      <xsl:with-param name="numCols" select="number($numCols) - 1" />
      <xsl:with-param name="continuation" select="'col'" />
    </xsl:call-template>
  </xsl:if>
</xsl:template>

<xsl:template name="constructCellDataFromString">
  <xsl:param name="dataString" />
  <xsl:param name="totalString" />
  <xsl:if test="string-length(normalize-space($dataString))">
    <xsl:variable name="rowStr" select="substring-after($dataString, '(')" />
    <xsl:variable name="colStr" select="substring-after($rowStr, ',')" />

    <xsl:variable name="idStr" select="substring-after($colStr, '(')" />
    <xsl:variable name="alignStr" select="substring-after($idStr, '(')" />
    <xsl:variable name="widthStr" select="substring-after($alignStr, ',')" />
    <xsl:variable name="borderTopStr" select="substring-after($widthStr, '(')" />
    <xsl:variable name="borderRightStr" select="substring-after($borderTopStr, ',')" />
    <xsl:variable name="borderBottomStr" select="substring-after($borderRightStr, ',')" />
    <xsl:variable name="borderLeftStr" select="substring-after($borderBottomStr, ',')" />
    <xsl:variable name="remainingStr" select="substring-after($borderLeftStr, ';')" />
    <xsl:element name="cellData">
      <xsl:attribute name="row"><xsl:value-of select="substring-before($rowStr, ',')" /></xsl:attribute>
      <xsl:attribute name="col"><xsl:value-of select="substring-before($colStr, ')')" /></xsl:attribute>
      <xsl:choose>
        <xsl:when test="starts-with($idStr, '#')">
          <xsl:attribute name="cellID"><xsl:value-of select="substring-after(substring-before($idStr, ')'), '#')" /></xsl:attribute>
        </xsl:when>
        <xsl:when test="starts-with($idStr, 'rowcont)')">
          <xsl:attribute name="continuation"><xsl:text>row</xsl:text></xsl:attribute>
        </xsl:when>
        <xsl:when test="starts-with($idStr, 'colcont)')">
          <xsl:attribute name="continuation"><xsl:text>col</xsl:text></xsl:attribute>
        </xsl:when>
      </xsl:choose>
      <xsl:if test="not(starts-with($alignStr, ','))">
        <xsl:attribute name="alignment"><xsl:value-of select="substring-before($alignStr, ',')" /></xsl:attribute>
      </xsl:if>
      <xsl:if test="not(starts-with($widthStr, ')'))">
        <xsl:attribute name="width"><xsl:value-of select="substring-before($widthStr, ')')" /></xsl:attribute>
      </xsl:if>
      <xsl:attribute name="borderTop"><xsl:value-of select="substring-before($borderTopStr, ',')" /></xsl:attribute>
      <xsl:attribute name="borderRight"><xsl:value-of select="substring-before($borderRightStr, ',')" /></xsl:attribute>
      <xsl:attribute name="borderBottom"><xsl:value-of select="substring-before($borderBottomStr, ',')" /></xsl:attribute>
      <xsl:attribute name="borderLeft"><xsl:value-of select="substring-before($borderLeftStr, ')')" /></xsl:attribute>
    </xsl:element>
    <xsl:if test="$remainingStr and string-length($remainingStr)">
      <xsl:call-template name="constructCellDataFromString">
        <xsl:with-param name="dataString" select="$remainingStr" />
      </xsl:call-template>
    </xsl:if>
  </xsl:if>
</xsl:template>

<xsl:template name="generateEmptyCells">
  <xsl:param name="numToGenerate" select="1" />
  <xsl:param name="currRow" select="1" />
  <xsl:param name="currCol" select="1" />
  <xsl:element name="cellData">
    <xsl:attribute name="row"><xsl:value-of select="$currRow" /></xsl:attribute>
    <xsl:attribute name="col"><xsl:value-of select="$currCol" /></xsl:attribute>
    <xsl:attribute name="borderTop"><xsl:value-of select="'none'" /></xsl:attribute>
    <xsl:attribute name="borderRight"><xsl:value-of select="'none'" /></xsl:attribute>
    <xsl:attribute name="borderBottom"><xsl:value-of select="'none'" /></xsl:attribute>
    <xsl:attribute name="borderLeft"><xsl:value-of select="'none'" /></xsl:attribute>
  </xsl:element>
  <xsl:if test="$numToGenerate &gt; 1">
    <xsl:call-template name="generateEmptyCells">
      <xsl:with-param name="numToGenerate" select="number($numToGenerate) - 1" />
      <xsl:with-param name="currRow" select="$currRow" />
      <xsl:with-param name="currCol" select="$currCol + 1" />
    </xsl:call-template>
  </xsl:if>
</xsl:template>

<xsl:template name="doReplaceMacro">
  <xsl:param name="targStr" select="''"/>
  <xsl:param name="findStr" select="''"/>
  <xsl:param name="replaceStr" select="''"/>
  <xsl:param name="global" select="0"/>
  <xsl:choose>
    <xsl:when test="contains($targStr, $findStr)">
      <xsl:choose>
        <xsl:when test="not(contains('abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ', substring(substring-after($targStr,$findStr),1,1)))">
          <xsl:value-of select="substring-before($targStr,$findStr)"/><xsl:value-of select="$replaceStr"/>
          <xsl:choose>
            <xsl:when test="number($global)">
              <xsl:call-template name="doReplaceMacro">
                <xsl:with-param name="targStr" select="substring-after($targStr,$findStr)"/>
                <xsl:with-param name="findStr" select="$findStr"/>
                <xsl:with-param name="replaceStr" select="$replaceStr"/>
                <xsl:with-param name="global" select="1"/>
              </xsl:call-template>
            </xsl:when>
            <xsl:otherwise><xsl:value-of select="substring-after($targStr,$findStr)"/></xsl:otherwise>
          </xsl:choose>
        </xsl:when>
        <xsl:otherwise><xsl:value-of select="$targStr"/></xsl:otherwise>
      </xsl:choose>
    </xsl:when>
    <xsl:otherwise><xsl:value-of select="$targStr"/></xsl:otherwise>
  </xsl:choose>
</xsl:template>

</xsl:stylesheet>
