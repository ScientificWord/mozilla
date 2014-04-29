<?xml version="1.0"?>
<xsl:stylesheet 
      xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
      xmlns:exsl="http://exslt.org/common"
      xmlns:mml="http://www.w3.org/1998/Math/MathML"
      version="1.1">

<!-- LaTeX bigops with limits are scripted as embellished operators
  in MathML - the host schemata are msub, msup, msubsup, munder,
  mover, and munderover.  When bigops are found in these constructs
  the following template is called.
-->

  <xsl:template name="do-embellished-bigop">
    <xsl:param name="limits-flag"/>
    <xsl:param name="j1"/>
    <xsl:param name="j2"/>
  <!-- \sum \limits OR \nolimits 
     _{
     x=10
     }^{
     \substack{ m=2 \\ n=0}
     }
-->
    <xsl:variable name="LaTeX-BigOp">
      <xsl:apply-templates select="./*[1]"/>
    </xsl:variable>

    <xsl:variable name="size">
      <xsl:choose>
        <xsl:when test="parent::mml:mstyle[@displaystyle='true'][count(./*)=1]">
          <xsl:value-of select="'d'"/>
		</xsl:when>
        <xsl:when test="parent::mml:mstyle[@displaystyle='false'][count(./*)=1]">
          <xsl:value-of select="'t'"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="'a'"/>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:choose>
      <xsl:when test="$size='d'">
        <xsl:choose>
          <xsl:when test="$output-mode='SW-LaTeX'">
            <xsl:text>\d</xsl:text>
            <xsl:value-of select="substring($LaTeX-BigOp,2)"/>
          </xsl:when>
          <xsl:otherwise>
            <xsl:text>{\displaystyle</xsl:text>
            <xsl:value-of select="$LaTeX-BigOp"/>
            <xsl:text>}</xsl:text>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:when test="$size='t'">
        <xsl:choose>
          <xsl:when test="$output-mode='SW-LaTeX'">
            <xsl:text>\t</xsl:text>
            <xsl:value-of select="substring($LaTeX-BigOp,2)"/>
          </xsl:when>
          <xsl:otherwise>
            <xsl:text>{\textstyle</xsl:text>
            <xsl:value-of select="$LaTeX-BigOp"/>
            <xsl:text>}</xsl:text>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="$LaTeX-BigOp"/>
      </xsl:otherwise>
    </xsl:choose>

    <xsl:if test="$limits-flag!='false'">
      <xsl:value-of select="$limits-flag"/>
    </xsl:if>

    <xsl:value-of select="$j1"/>

    <xsl:call-template name="do-positional-arg">
      <xsl:with-param name="arg-num" select="2"/>
    </xsl:call-template>

    <xsl:value-of select="$j2"/>

    <xsl:call-template name="do-positional-arg">
      <xsl:with-param name="arg-num" select="3"/>
    </xsl:call-template>

    <xsl:text>}</xsl:text>
  </xsl:template>


  
  <xsl:template name="math-accent">
    <xsl:param name="LaTeX-acc"/>
      <xsl:value-of select="$LaTeX-acc"/>
    <xsl:text>{</xsl:text>
    <xsl:call-template name="do-positional-arg">
      <xsl:with-param name="arg-num" select="1"/>
    </xsl:call-template>
    <xsl:text>}</xsl:text>
  </xsl:template>

  
  <xsl:template match="mml:mover" mode="in-text">
      <xsl:text>$</xsl:text>
    <xsl:apply-templates select="."/>
    <xsl:text>$</xsl:text>
  </xsl:template>


  <xsl:template name="check-decoration">
      <xsl:choose>
<!--
      <xsl:when test="*[2][@stretchy='true']">
        <xsl:text>true</xsl:text>
      </xsl:when>
-->
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
      <xsl:when test="./*[2][normalize-space(string())='&#xF612;']">
        <xsl:text>true</xsl:text>
      </xsl:when>
      <xsl:when test="./*[2][normalize-space(string())='&#x23DE;']
      or              ./*[2][normalize-space(string())='&#xFE37;']">
        <xsl:text>true</xsl:text>
      </xsl:when>
      <xsl:when test="./*[2][normalize-space(string())='&#x0302;']
      and             ./*[2][@stretchy='true']">
        <xsl:text>true</xsl:text>
      </xsl:when>
      <xsl:when test="./*[2][normalize-space(string())='&#x02DC;']
      and             ./*[2][@stretchy='true']">
        <xsl:text>true</xsl:text>
      </xsl:when>

	  <xsl:otherwise>
        <xsl:text>false</xsl:text>
	  </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template name="check-accent">
      <xsl:choose>
<!--
      <xsl:when test="*[2][@accent='true']">
        <xsl:text>true</xsl:text>
      </xsl:when>
-->
      <xsl:when test="./*[2][normalize-space(string())='&#x005E;']">
        <xsl:text>true</xsl:text>
      </xsl:when>
      <xsl:when test="./*[2][normalize-space(string())='&#x0060;']">
        <xsl:text>true</xsl:text>
      </xsl:when>
      <xsl:when test="./*[2][normalize-space(string())='&#x00A8;']">
        <xsl:text>true</xsl:text>
      </xsl:when>
      <xsl:when test="./*[2][normalize-space(string())='&#x00AF;']">
        <xsl:text>true</xsl:text>
      </xsl:when>
      <xsl:when test="./*[2][normalize-space(string())='&#x00B4;']">
        <xsl:text>true</xsl:text>
      </xsl:when>
      <xsl:when test="./*[2][normalize-space(string())='&#x02C7;']">
        <xsl:text>true</xsl:text>
      </xsl:when>
      <xsl:when test="./*[2][normalize-space(string())='&#x02DA;']">
        <xsl:text>true</xsl:text>
      </xsl:when>
      <xsl:when test="./*[2][normalize-space(string())='&#x02DC;']">
        <xsl:text>true</xsl:text>
      </xsl:when>
      <xsl:when test="./*[2][normalize-space(string())='~']">
        <xsl:text>true</xsl:text>
      </xsl:when>
      <xsl:when test="./*[2][normalize-space(string())='&#x02D9;']">
        <xsl:text>true</xsl:text>
      </xsl:when>
      <xsl:when test="./*[2][normalize-space(string())='&#x02D8;']">
        <xsl:text>true</xsl:text>
      </xsl:when>
      <xsl:when test="./*[2][normalize-space(string())='&#x02DD;']">
        <xsl:text>true</xsl:text>
      </xsl:when>
      <xsl:when test="./*[2][normalize-space(string())='&#x0302;']">
        <xsl:text>true</xsl:text>
      </xsl:when>
      
      <xsl:when test="./*[2][normalize-space(string())='&#x20D7;']">
        <xsl:text>true</xsl:text>
      </xsl:when>
      <xsl:when test="./*[2][normalize-space(string())='&#x20DB;']">
        <xsl:text>true</xsl:text>
      </xsl:when>
      <xsl:when test="./*[2][normalize-space(string())='&#x20DC;']">
        <xsl:text>true</xsl:text>
      </xsl:when>
      <xsl:when test="./*[2][normalize-space(string())='&#x2192;']">
        <xsl:text>true</xsl:text>
      </xsl:when>

	  <xsl:otherwise>
        <xsl:text>false</xsl:text>
	  </xsl:otherwise>
    </xsl:choose>
  </xsl:template>



  <xsl:template match="mml:mover">
  
    <xsl:variable name="mover-structure.tr">
      <is-accent>
        <xsl:call-template name="check-accent"/>
      </is-accent>
      <is-decoration>
        <xsl:call-template name="check-decoration"/>
      </is-decoration>
      <big-op-char>
        <xsl:call-template name="is-LaTeX-bigop"/>
      </big-op-char>

      <movablelimits>
        <xsl:if test="*[1][self::mml:mo]">
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
		</xsl:if>
      </movablelimits>

    </xsl:variable>
    <xsl:variable name="mover-structure" select="exsl:node-set($mover-structure.tr)"/>

    <xsl:variable name="limits">
      <xsl:if test="*[1][self::mml:mo]">
      <xsl:choose>
        <xsl:when test="$mover-structure/movablelimits='false'">
          <xsl:text>\limits </xsl:text>
        </xsl:when>
        <xsl:otherwise>
          <xsl:text>false</xsl:text>
        </xsl:otherwise>
      </xsl:choose>
      </xsl:if>
    </xsl:variable>

    <xsl:choose>

      <!-- the top element is an accent operator -->

      <xsl:when test="$mover-structure/is-accent='true'">
        <xsl:choose>
          <xsl:when test="./*[2][normalize-space(string())='&#x005E;']">
            <xsl:call-template name="math-accent">
              <xsl:with-param name="LaTeX-acc" select="'\hat'"/>
            </xsl:call-template>
          </xsl:when>
          <xsl:when test="./*[2][normalize-space(string())='&#x0302;']">
            <xsl:call-template name="math-accent">
              <xsl:with-param name="LaTeX-acc" select="'\hat'"/>
            </xsl:call-template>
          </xsl:when>
          <xsl:when test="./*[2][normalize-space(string())='&#x02C7;']">
            <xsl:call-template name="math-accent">
              <xsl:with-param name="LaTeX-acc" select="'\check'"/>
            </xsl:call-template>
          </xsl:when>
          <xsl:when test="./*[2][normalize-space(string())='&#x02DC;']">
            <xsl:call-template name="math-accent">
              <xsl:with-param name="LaTeX-acc" select="'\tilde'"/>
            </xsl:call-template>
          </xsl:when>
          <xsl:when test="./*[2][normalize-space(string())='~']">
            <xsl:call-template name="math-accent">
              <xsl:with-param name="LaTeX-acc" select="'\tilde'"/>
            </xsl:call-template>
          </xsl:when>
          <xsl:when test="./*[2][normalize-space(string())='&#x00B4;']">
            <xsl:call-template name="math-accent">
              <xsl:with-param name="LaTeX-acc" select="'\acute'"/>
            </xsl:call-template>
          </xsl:when>
          <xsl:when test="./*[2][normalize-space(string())='&#x0060;']">
            <xsl:call-template name="math-accent">
              <xsl:with-param name="LaTeX-acc" select="'\grave'"/>
            </xsl:call-template>
          </xsl:when>
          <xsl:when test="./*[2][normalize-space(string())='&#x02D9;']">
            <xsl:call-template name="math-accent">
              <xsl:with-param name="LaTeX-acc" select="'\dot'"/>
            </xsl:call-template>
          </xsl:when>
          <xsl:when test="./*[2][normalize-space(string())='&#x00A8;']">
            <xsl:call-template name="math-accent">
              <xsl:with-param name="LaTeX-acc" select="'\ddot'"/>
            </xsl:call-template>
          </xsl:when>
          <xsl:when test="./*[2][normalize-space(string())='&#x02D8;']">
            <xsl:call-template name="math-accent">
              <xsl:with-param name="LaTeX-acc" select="'\breve'"/>
            </xsl:call-template>
          </xsl:when>
          
          <xsl:when test="./*[2][normalize-space(string())='&#x00AF;']">
            <xsl:call-template name="math-accent">
              <xsl:with-param name="LaTeX-acc" select="'\bar'"/>
            </xsl:call-template>
          </xsl:when>
          <xsl:when test="./*[2][normalize-space(string())='&#x2192;']
          or              ./*[2][normalize-space(string())='&#x20D7;']">
            <xsl:call-template name="math-accent">
              <xsl:with-param name="LaTeX-acc" select="'\vec'"/>
            </xsl:call-template>
          </xsl:when>
          <xsl:when test="./*[2][normalize-space(string())='&#x20DB;']">
            <xsl:call-template name="math-accent">
              <xsl:with-param name="LaTeX-acc" select="'\dddot'"/>
            </xsl:call-template>
          </xsl:when>
          <xsl:when test="./*[2][normalize-space(string())='&#x02DA;']">
            <xsl:call-template name="math-accent">
              <xsl:with-param name="LaTeX-acc" select="'\mathring'"/>
            </xsl:call-template>
          </xsl:when>
          <xsl:when test="./*[2][normalize-space(string())='&#x20DC;']">
            <xsl:call-template name="math-accent">
              <xsl:with-param name="LaTeX-acc" select="'\ddddot'"/>
            </xsl:call-template>
          </xsl:when>
          <xsl:when test="./*[2][normalize-space(string())='&#x23DE;']
          or              ./*[2][normalize-space(string())='&#xFE37;']">
            <xsl:call-template name="math-accent">
              <xsl:with-param name="LaTeX-acc" select="'\overbrace'"/>
            </xsl:call-template>
          </xsl:when>

		  <xsl:otherwise>
		  </xsl:otherwise>
        </xsl:choose>
	  </xsl:when>


<!-- the base element is a big operator -->

      <xsl:when test="$mover-structure/big-op-char!='false'">
        <xsl:call-template name="do-embellished-bigop">
          <xsl:with-param name="limits-flag" select="$limits"/>
          <xsl:with-param name="j1"          select="'^{'"/>
          <xsl:with-param name="j2"          select="''"/>
        </xsl:call-template>
      </xsl:when>


<!-- the top element is a stretchy operator - decoration -->

      <xsl:when test="$mover-structure/is-decoration='true'">
        <xsl:choose>
          <xsl:when test="./*[2][normalize-space(string())='&#x00AF;']">
            <xsl:call-template name="math-accent">
              <xsl:with-param name="LaTeX-acc" select="'\overline'"/>
            </xsl:call-template>
          </xsl:when>
          <xsl:when test="./*[2][normalize-space(string())='&#x2190;']">
            <xsl:call-template name="math-accent">
              <xsl:with-param name="LaTeX-acc" select="'\overleftarrow'"/>
            </xsl:call-template>
          </xsl:when>
          <xsl:when test="./*[2][normalize-space(string())='&#x2192;']">
            <xsl:call-template name="math-accent">
              <xsl:with-param name="LaTeX-acc" select="'\overrightarrow'"/>
            </xsl:call-template>
          </xsl:when>
          <xsl:when test="./*[2][normalize-space(string())='&#x2194;']">
            <xsl:call-template name="math-accent">
              <xsl:with-param name="LaTeX-acc" select="'\overleftrightarrow'"/>
            </xsl:call-template>
          </xsl:when>
          <xsl:when test="./*[2][normalize-space(string())='&#x23DE;']
          or              ./*[2][normalize-space(string())='&#xFE37;']">
            <xsl:call-template name="math-accent">
              <xsl:with-param name="LaTeX-acc" select="'\overbrace'"/>
            </xsl:call-template>
          </xsl:when>
          <xsl:when test="./*[2][normalize-space(string())='&#x0302;']">
            <xsl:call-template name="math-accent">
              <xsl:with-param name="LaTeX-acc" select="'\widehat'"/>
            </xsl:call-template>
          </xsl:when>
          <xsl:when test="./*[2][normalize-space(string())='&#x02DC;']">
            <xsl:call-template name="math-accent">
              <xsl:with-param name="LaTeX-acc" select="'\widetilde'"/>
            </xsl:call-template>
          </xsl:when>
          <xsl:when test="./*[2][normalize-space(string())='~']">
             <xsl:call-template name="math-accent">
               <xsl:with-param name="LaTeX-acc" select="'\widetilde'"/>
             </xsl:call-template>
          </xsl:when>

		  <xsl:otherwise>
		  </xsl:otherwise>
        </xsl:choose>
	  </xsl:when>

	  <xsl:otherwise>
        <xsl:text xml:space="preserve">\overset{</xsl:text>
        <xsl:call-template name="do-positional-arg">
          <xsl:with-param name="arg-num" select="2"/>
        </xsl:call-template>
        <xsl:text>}{</xsl:text>
        <xsl:call-template name="do-positional-arg">
          <xsl:with-param name="arg-num" select="1"/>
        </xsl:call-template>
        <xsl:text>}</xsl:text>
	  </xsl:otherwise>
    </xsl:choose>
   </xsl:template>

</xsl:stylesheet>

