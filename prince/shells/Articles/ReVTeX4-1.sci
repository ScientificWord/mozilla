PK    Zy0�               css/PK    -�C��   �   
   css/my.css@import url("resource://app/res/css/baselatex.css");
@import url("resource://app/res/css/latex.css");
@import url("resource://app/res/css/revtex4-1.css");

PK    oCù�Y{  Y{  
   main.xhtml<?xml version="1.0" encoding="UTF-8"?>
<?xml-stylesheet href="css/my.css" type="text/css"?>
<?sw-tagdefs href="resource://app/res/tagdefs/latexdefs.xml" type="text/xml" ?>
<?sw-xslt href="revtex4-1.xsl" type="text/xml" ?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1 plus MathML 2.0//EN" "http://www.w3.org/Math/DTD/mathml2/xhtml-math11-f.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xmlns:mml="http://www.w3.org/1998/Math/MathML" xmlns:sw="http://www.sciword.com/namespaces/sciword">
<head><sw-meta id="sw-meta" product="Scientific WorkPlace" version="2013080100" created="Thu Aug 01 2013 17:01:27 GMT-0600 (Mountain Daylight Time)" lastrevised="Wed Aug 07 2013 01:03:31 GMT-0600 (Mountain Daylight Time)"/>
    <preamble><colist/>
      <documentclass class="revtex4-1" options="showpacs,showkeys,twocolumn"/>
      <preambleTeX><![CDATA[ \usepackage {bm} \usepackage {tabulary}]]>
      </preambleTeX>
    </preamble>
  </head>
<body showexpanders="true" showshort="true">
    <title>
      <bodyText>REVTeX4-1 Article        
      </bodyText>
    </title>
    <author>Bugs Bunny     
    </author>
    <email alt="E-mail me at: ">bugs@looney.com</email><homepage alt="Visit: ">http://looney.com</homepage><altaffiliation alt="Permanent address: ">Warner
Brothers</altaffiliation><affiliation>Looney Tunes Studio</affiliation>
    <author>Roger Rabbit     
    </author>
    <affiliation>Looney Tunes Studio</affiliation>
    <author>Mickey Mouse     
    </author>
    <affiliation>Disney Studios</affiliation><collaboration>Pepé <surname>Le
Pew</surname></collaboration><noaffiliation/>
    <date><texb enc="1" name="today"><![CDATA[\today]]></texb>
    </date>
    <abstract>
      <descriptionlist>
        <descriptionListItem>
          <bodyText><descriptionLabel>Background</descriptionLabel> This part would describe the
context needed to understand what the paper is about.                
          </bodyText>
        </descriptionListItem>
        <descriptionListItem>
          <bodyText><descriptionLabel>Purpose</descriptionLabel> This part would state the purpose
of the present paper.                
          </bodyText>
        </descriptionListItem>
        <descriptionListItem>
          <bodyText><descriptionLabel>Method</descriptionLabel> This part describe the methods used
in the paper.                
          </bodyText>
        </descriptionListItem>
        <descriptionListItem>
          <bodyText><descriptionLabel>Results</descriptionLabel> This part would summarize the
results.                
          </bodyText>
        </descriptionListItem>
        <descriptionListItem>
          <bodyText><descriptionLabel>Conclusions</descriptionLabel> This part would state the
conclusions of the paper.              
          </bodyText>
        </descriptionListItem>
      </descriptionlist>
    </abstract>
    <pacs>23.3.+x, 56.65.Dy</pacs><keywords>sample;
template</keywords><maketitle/><maketoc/><makelof/><makelot/><section id="tsid_3141595">
      <sectiontitle>REVTeX4-1<br/>
      </sectiontitle>
      <bodyText> This document illustrates the appearance of an article created with the shell
REVTeX4-1. The front matter shown was derived from the REVTeX4-1 documentation which includes fields
for multiple authors and affiliations, the PACS codes, and keywords.</bodyText><bodyText>The document class base
file for this shell is <typewriter>revtex4-1.cls</typewriter>. This typesetting specification
supports a number of class options, including document type, paper size, columns, title page format,
and equation numbering. To see the available class options, choose Typeset, choose Options and
Packages, select the Class Options tab, and then click the Modify button. This shell uses class
options with selections for the American Physical Society, preprint formats, and to display the PACS
and keywords.         
        Changes to the typeset format of this shell and its associated <texlogo xmlns="http://www.w3.org/1999/xhtml" name="latex">L<sup>A</sup>T<sub>E</sub>X</texlogo>
formatting file (<typewriter xmlns="http://www.w3.org/1999/xhtml">revtex4-1.cls</typewriter>) are not supported by MacKichan Software, Inc. If you want to make such changes, please consult
the <texlogo xmlns="http://www.w3.org/1999/xhtml" name="latex">L<sup>A</sup>T<sub>E</sub>X</texlogo> manuals or a local <texlogo xmlns="http://www.w3.org/1999/xhtml" name="latex">L<sup>A</sup>T<sub>E</sub>X</texlogo>
expert.Using This Shell</bodyText></section><section msiSelectionManagerID="4" id="tsid_3141596"><sectiontitle>Using this shell<br/></sectiontitle><bodyText><subsection id="tsid_3141606">
          </subsection>
</bodyText><bodyText xmlns="http://www.w3.org/1999/xhtml">The front matter of this shell has a number of sample entries that you should replace with your own. Replace the body of this document with your
own text. To start with a blank document, you may delete all of the text in this document. </bodyText><bodyText>
                   






</bodyText><subsection xmlns="http://www.w3.org/1999/xhtml" id="tsid_3141607">
<sectiontitle>REV<texlogo xmlns="http://www.w3.org/1999/xhtml" name="tex">T<sub>E</sub>X</texlogo> specific symbols</sectiontitle>
<bodyText>REV<texlogo xmlns="http://www.w3.org/1999/xhtml" name="tex">T<sub>E</sub>X</texlogo> defines some new symbols and accents that are not understood
by the program interface. The paragraph below names each symbol or accent and shows an example of its use. </bodyText>
<bodyText><typewriter><math xmlns="http://www.w3.org/1998/Math/MathML">
  <mrow>
    <mo form="infix">\</mo>
  </mrow></math> lambdabar: </typewriter><math xmlns="http://www.w3.org/1998/Math/MathML">
  <mrow>
    <mi>\lambdabar</mi>
    <mtext>​</mtext>
  </mrow></math>, <typewriter><math xmlns="http://www.w3.org/1998/Math/MathML">
  <mrow>
    <mo form="infix">\</mo>
  </mrow></math> openone: </typewriter><math xmlns="http://www.w3.org/1998/Math/MathML">
  <mrow>
    <texb enc="1" name="\openone "><![CDATA[\openone ]]></texb>
  </mrow></math>, <typewriter><math xmlns="http://www.w3.org/1998/Math/MathML">
  <mrow>
    <mo form="infix">\</mo>
  </mrow></math> vereq{a}{b}: </typewriter><math xmlns="http://www.w3.org/1998/Math/MathML">
  <mrow>
    <texb enc="1" name="\vereq "><![CDATA[\vereq \{a\}\{b\}]]></texb>
  </mrow></math>, <typewriter><math xmlns="http://www.w3.org/1998/Math/MathML">
  <mrow>
    <mo form="infix">\</mo>
  </mrow></math> tensor{}: </typewriter><math xmlns="http://www.w3.org/1998/Math/MathML">
  <mrow>
    <texb enc="1" name="\tensor "><![CDATA[\tensor \{x\}]]></texb>
  </mrow></math>, <typewriter><math xmlns="http://www.w3.org/1998/Math/MathML">
  <mrow>
    <mo form="infix">\</mo>
  </mrow></math> overstar{}: </typewriter><math xmlns="http://www.w3.org/1998/Math/MathML"><mrow><texb enc="1" name="\overstar "><![CDATA[\overstar \{x\}]]></texb></mrow></math>, <typewriter><math xmlns="http://www.w3.org/1998/Math/MathML">
  <mrow>
    <mo form="infix">\</mo>
  </mrow></math> loarrow{}: </typewriter><math xmlns="http://www.w3.org/1998/Math/MathML">
  <mrow>
    <texb enc="1" name="\loarrow "><![CDATA[\loarrow \{x\}]]></texb>
  </mrow></math>, <typewriter><math xmlns="http://www.w3.org/1998/Math/MathML">
  <mrow>
    <mo form="infix">\</mo>
  </mrow></math> roarrow{}: </typewriter><math xmlns="http://www.w3.org/1998/Math/MathML">
  <mrow>
    <texb enc="1" name="\roarrow "><![CDATA[\roarrow \{x\}]]></texb>
  </mrow></math>, <typewriter><math xmlns="http://www.w3.org/1998/Math/MathML">
  <mrow>
    <mo form="infix">\</mo>
  </mrow></math> altsuccsim: </typewriter><math xmlns="http://www.w3.org/1998/Math/MathML">
  <mrow>
    <texb enc="1" name="\altsuccsim "><![CDATA[\altsuccsim ]]></texb>
  </mrow></math> (an alternate for <math xmlns="http://www.w3.org/1998/Math/MathML">
  <mrow>
    <mo form="infix">≿</mo>
  </mrow></math>), and <typewriter><math xmlns="http://www.w3.org/1998/Math/MathML">
  <mrow>
    <mo form="infix">\</mo>
  </mrow></math> altprecsim: </typewriter><math xmlns="http://www.w3.org/1998/Math/MathML">
  <mrow>
    <texb enc="1" name="\altprecsim "><![CDATA[\altprecsim ]]></texb>
  </mrow></math> (an alternate for <math xmlns="http://www.w3.org/1998/Math/MathML"><mrow><mo form="infix">≾</mo></mrow></math>). </bodyText></subsection></section>
                <section msiSelectionManagerID="4" id="tsid_3141597"><subsection id="tsid_3141608">
               <sectiontitle>
                 Wide text to span columns</sectiontitle><bodyText>
              
              </bodyText><bodyText xmlns="http://www.w3.org/1999/xhtml">REVTeX4-1 includes the widetext environment that will typeset across two columns, if the typesetting specification option for two columns is selected.<widetext> This is text that will extend across two columns, should this document be typeset using a
two column option. </widetext></bodyText><bodyText>
              Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.</bodyText><bodyText>
            </bodyText>
             </subsection></section><section id="tsid_3141598">
               <sectiontitle>
                 Headings and tags<br/>
               </sectiontitle>
             </section>    
<subsection id="tsid_3141609">
<sectiontitle>Subsection</sectiontitle>
<bodyText>Use the Section tag for major sections, and the Subsection tag for subsections. </bodyText>    
<subsubsection id="tsid_3141613">
<sectiontitle>Subsubsection</sectiontitle>
<bodyText>This is just some harmless text under a subsubsection. </bodyText>    
<paragraph id="tsid_3141592">
<sectiontitle>Paragraph<br/></sectiontitle>
<bodyText>This is just some harmless text under a paragraph. </bodyText>    
<subparagraph id="tsid_3141603">
<sectiontitle>Subparagraph</sectiontitle>
<bodyText msiSelectionManagerID="5">This is just some harmless text under a subparagraph. Here is Table <xref key="TableKey" reftype="obj" req="varioref"/>  to demonstrate
cross referencing a table and so that it will appear in the list of tables, if used. <texlogo xmlns="http://www.w3.org/1999/xhtml" name="latex">L<sup>A</sup>T<sub>E</sub>X </texlogo>will position the table to best take advantage
of the flow of text.<texb enc="1" name="TeXButton"><![CDATA[\TeXButton{B} {\begin {table}[tbp] \centering } ]]></texb> 
   <table cols="|l|l|l|" cellpadding="4" border="1" req="tabulary" id="tsid_3141616"><tbody><tr><td><bodyText><bold>Head</bold> <br type="_moz"/>
</bodyText>
</td><td><bodyText><bold>Head</bold> <br type="_moz"/>
</bodyText>
</td><td><bodyText><bold>Head</bold>  <br type="_moz"/>
</bodyText>
</td></tr><tr><td><bodyText>entry <br type="_moz"/>
</bodyText>
</td><td><bodyText>entry <br type="_moz"/>
</bodyText>
</td><td><bodyText>entry   <br type="_moz"/>
</bodyText>
</td></tr><tr><td><bodyText>entry <br type="_moz"/>
</bodyText>
</td><td><bodyText>entry <br type="_moz"/>
</bodyText>
</td><td><bodyText>entry   <br type="_moz"/>
</bodyText>
</td></tr><tr><td><bodyText>entry <br type="_moz"/>
</bodyText>
</td><td><bodyText>entry <br type="_moz"/>
</bodyText>
</td><td><bodyText>entry  <br type="_moz"/>
</bodyText>
</td></tr></tbody></table><br type="_moz"/> 
 <caption align="bottom"><bodyText> Table Caption</bodyText> </caption><marker id="TableKey"/><texb enc="1" name="TeXButton"><![CDATA[\TeXButton{E} {\end {table}}
]]></texb></bodyText><bodyText>
                  
</bodyText></subparagraph></paragraph></subsubsection></subsection><subsection id="tsid_3141610">
<sectiontitle>Tags</sectiontitle>
<bodyText>You can apply the logical markup tag <emphasized>Emphasized</emphasized>. </bodyText>
<bodyText>You can apply the visual markup tags <bold>Bold</bold>, <italics>Italics</italics>, <roman>Roman</roman>, <sansSerif>Sans Serif</sansSerif>, <slanted>Slanted</slanted>,
<smallCaps>Small Caps</smallCaps>, and <typewriter>Typewriter</typewriter>. </bodyText>
<bodyText>You can apply the special mathematics-only tags <math xmlns="http://www.w3.org/1998/Math/MathML"><mrow><mi mathvariant="double-struck">B</mi><mo>⁢</mo><mi mathvariant="double-struck">L</mi><mo>⁢</mo><mi mathvariant="double-struck">A</mi><mo>⁢</mo><mi mathvariant="double-struck">C</mi><mo>⁢</mo><mi mathvariant="double-struck">K</mi><mo>⁢</mo><mi mathvariant="double-struck">B</mi><mo>⁢</mo><mi mathvariant="double-struck">O</mi><mo>⁢</mo><mi mathvariant="double-struck">A</mi><mo>⁢</mo><mi mathvariant="double-struck">R</mi><mo>⁢</mo><mi mathvariant="double-struck">D</mi></mrow></math> <math xmlns="http://www.w3.org/1998/Math/MathML">
  <mrow>
    <mi mathvariant="double-struck">B</mi>
    <mo>⁢</mo>
    <mi mathvariant="double-struck">O</mi>
    <mo>⁢</mo>
    <mi mathvariant="double-struck">L</mi>
    <mo>⁢</mo>
    <mi mathvariant="double-struck">D</mi>
  </mrow></math>, <math xmlns="http://www.w3.org/1998/Math/MathML">
  <mrow>
    <mi mathvariant="script">C</mi>
    <mo>⁢</mo>
    <mi mathvariant="script">A</mi>
    <mo>⁢</mo>
    <mi mathvariant="script">L</mi>
    <mo>⁢</mo>
    <mi mathvariant="script">L</mi>
    <mo>⁢</mo>
    <mi mathvariant="script">I</mi>
    <mo>⁢</mo>
    <mi mathvariant="script">G</mi>
    <mo>⁢</mo>
    <mi mathvariant="script">R</mi>
    <mo>⁢</mo>
    <mi mathvariant="script">A</mi>
    <mo>⁢</mo>
    <mi mathvariant="script">P</mi>
    <mo>⁢</mo>
    <mi mathvariant="script">H</mi>
    <mo>⁢</mo>
    <mi mathvariant="script">I</mi>
    <mo>⁢</mo>
    <mi mathvariant="script">C</mi>
  </mrow></math>, and <math xmlns="http://www.w3.org/1998/Math/MathML">
  <mrow>
    <mi mathvariant="fraktur">f</mi>
    <mo>⁢</mo>
    <mi mathvariant="fraktur">r</mi>
    <mo>⁢</mo>
    <mi mathvariant="fraktur">a</mi>
    <mo>⁢</mo>
    <mi mathvariant="fraktur">k</mi>
    <mo>⁢</mo>
    <mi mathvariant="fraktur">t</mi>
    <mo>⁢</mo>
    <mi mathvariant="fraktur">u</mi>
    <mo>⁢</mo>
    <mi mathvariant="fraktur">r</mi>
  </mrow></math>. Note that blackboard bold and calligraphic are correct only when applied to uppercase letters A through Z. </bodyText>
<bodyText>You can apply the size tags <tiny>tiny</tiny>, <scriptsize>scriptsize</scriptsize>, <footnotesize>footnotesize</footnotesize>, <small>small</small>,
<normalsize>normalsize</normalsize>, <large>large</large>, <Large>Large</Large>, <LARGE>LARGE</LARGE>, <huge>huge</huge> and <Huge>Huge.</Huge> </bodyText>
<bodyText>This is a Body Math paragraph. Each time you press the Enter key, Scientific WorkPlace switches to mathematics mode. This is convenient for carrying
out “scratchpad”<hspace type="requiredSpace" dim="1em"/>computations. </bodyText>
<bodyText>Following is a group of paragraphs marked as Short Quote. This environment is appropriate for a short quotation or a sequence of short quotations.
</bodyText>
<shortQuote>
<bodyText> The only thing we have to fear is fear itself. <emphasized>Franklin D. Roosevelt, </emphasized>Mar. 4, 1933 </bodyText>
<bodyText>Ask not what your country can do for you; ask what you can do for your country. <emphasized>John F. Kennedy, </emphasized>Jan. 20. 1961 </bodyText>
<bodyText>There is nothing wrong with America that cannot be cured by what is right with America. <emphasized>William J. “Bill”<hspace type="requiredSpace" dim="1em"/>Clinton,
</emphasized>Jan. 21, 1993 
</bodyText>
</shortQuote>
 
<bodyText>The Long Quotation tag is used for quotations of more than one paragraph. Following is the beginning of <emphasized>Alice's Adventures in Wonderland
</emphasized>by Lewis Carroll: </bodyText>
<longQuotation>
<bodyText> Alice was beginning to get very tired of sitting by her sister on the bank, and of having nothing to do: once or twice she had peeped into the
book her sister was reading, but it had no pictures or conversations in it, `and what is the use of a book,' thought Alice `without pictures or conversation?'
</bodyText>
<bodyText>So she was considering in her own mind (as well as she could, for the hot day made her feel very sleepy and stupid), whether the pleasure of making
a daisy-chain would be worth the trouble of getting up and picking the daisies, when suddenly a White Rabbit with pink eyes ran close by her. </bodyText>
<bodyText>There was nothing so very remarkable in that; nor did Alice think it so very much out of the way to hear the Rabbit say to itself, `Oh dear! Oh
dear! I shall be late!' (when she thought it over afterwards, it occurred to her that she ought to have wondered at this, but at the time it all seemed
quite natural); but when the Rabbit actually took a watch out of its waistcoat-pocket, and looked at it, and then hurried on, Alice started to her feet,
for it flashed across her mind that she had never before seen a rabbit with either a waistcoat-pocket, or a watch to take out of it, and burning with curiosity,
she ran across the field after it, and fortunately was just in time to see it pop down a large rabbit-hole under the hedge. </bodyText>
<bodyText>In another moment down went Alice after it, never once considering how in the world she was to get out again. 
</bodyText>
</longQuotation>
 
<bodyText>Use the Verbatim tag when you want <texlogo xmlns="http://www.w3.org/1999/xhtml" name="latex">L<sup>A</sup>T<sub>E</sub>X</texlogo> to preserve
spacing, perhaps when including a fragment from a program such as: 
 
<verbatim><bodyText><br/>#include &lt;iostream&gt;        // &lt; &gt; is used for standard libraries.<br/>void main(void)            // "main" method always called first.<br/>{<br/> cout &lt;&lt; "Hello World.";  // Send to output stream.<br/>}<br/></bodyText> </verbatim>
 </bodyText>    
</subsection><bodyText>

</bodyText><section xmlns="http://www.w3.org/1999/xhtml" id="tsid_3141599">
<sectiontitle>Mathematics and text</sectiontitle>
<bodyText>Let <math xmlns="http://www.w3.org/1998/Math/MathML">
  <mrow>
    <mi>H</mi>
  </mrow></math> be a Hilbert space, <math xmlns="http://www.w3.org/1998/Math/MathML">
  <mrow>
    <mi>C</mi>
  </mrow></math> be a closed bounded convex subset of <math xmlns="http://www.w3.org/1998/Math/MathML">
  <mrow>
    <mi>H</mi>
  </mrow></math>, <math xmlns="http://www.w3.org/1998/Math/MathML">
  <mrow>
    <mi>T</mi>
  </mrow></math> a nonexpansive self map of <math xmlns="http://www.w3.org/1998/Math/MathML">
  <mrow>
    <mi>C</mi>
  </mrow></math>. Suppose that as <math xmlns="http://www.w3.org/1998/Math/MathML">
  <mrow>
    <mi>n</mi>
    <mo form="infix">→</mo>
    <mi>∞</mi>
  </mrow></math>, <math xmlns="http://www.w3.org/1998/Math/MathML">
  <mrow>
    <msub>
      <mi>a</mi>
      <mrow>
        <mi>n</mi>
        <mo form="infix">,</mo>
        <mi>k</mi>
      </mrow>
    </msub>
    <mo form="infix">→</mo>
    <mn>0</mn>
  </mrow></math> for each <math xmlns="http://www.w3.org/1998/Math/MathML">
  <mrow>
    <mi>k</mi>
  </mrow></math>, and <math xmlns="http://www.w3.org/1998/Math/MathML"><mrow><mrow><msub>
        <mi>γ</mi>
        <mi>n</mi>
      </msub><mo form="infix">=</mo><mrow><munderover>
          <mo form="prefix" movablelimits="true" largeop="true">∑</mo>
          <mrow>
            <mi>k</mi>
            <mo form="infix">=</mo>
            <mn>0</mn>
          </mrow>
          <mi>∞</mi>
        </munderover><msup><mrow><mo form="prefix" fence="true" stretchy="true" symmetric="true">(</mo><mrow><msub><mi>a</mi><mrow><mi>n</mi><mo form="infix">,</mo><mrow><mi>k</mi><mo form="infix">+</mo><mn>1</mn></mrow></mrow></msub><mo form="infix">−</mo><msub>
                <mi>a</mi>
                <mrow>
                  <mi>n</mi>
                  <mo form="infix">,</mo>
                  <mi>k</mi>
                </mrow>
              </msub></mrow><mo form="postfix" fence="true" stretchy="true" symmetric="true">)</mo></mrow><mo>+</mo></msup></mrow></mrow><mo form="infix">→</mo><mn>0</mn></mrow></math>. Then for each <math xmlns="http://www.w3.org/1998/Math/MathML">
  <mrow>
    <mi>x</mi>
  </mrow></math> in <math xmlns="http://www.w3.org/1998/Math/MathML">
  <mrow>
    <mi>C</mi>
  </mrow></math>, <math xmlns="http://www.w3.org/1998/Math/MathML">
  <mrow>
    <mrow>
      <msub>
        <mi>A</mi>
        <mi>n</mi>
      </msub>
      <mo>⁢</mo>
      <mi>x</mi>
    </mrow>
    <mo form="infix">=</mo>
    <mrow>
      <munderover>
        <mo form="prefix" movablelimits="true" largeop="true">∑</mo>
        <mrow>
          <mi>k</mi>
          <mo form="infix">=</mo>
          <mn>0</mn>
        </mrow>
        <mi>∞</mi>
      </munderover>
      <mrow>
        <msub>
          <mi>a</mi>
          <mrow>
            <mi>n</mi>
            <mo form="infix">,</mo>
            <mi>k</mi>
          </mrow>
        </msub>
        <mo>⁢</mo>
        <msup>
          <mi>T</mi>
          <mi>k</mi>
        </msup>
        <mo>⁢</mo>
        <mi>x</mi>
      </mrow>
    </mrow>
  </mrow></math> converges weakly to a fixed point of <math xmlns="http://www.w3.org/1998/Math/MathML">
  <mrow>
    <mi>T</mi>
  </mrow></math> . </bodyText>
<bodyText>The numbered equation 
<msidisplay marker="eqn1" id="eqn1"><math xmlns="http://www.w3.org/1998/Math/MathML" display="block"><mrow><mrow>
      <mrow>
        <msub>
          <mi>u</mi>
          <mrow>
            <mi>t</mi>
            <mo>⁣</mo>
            <mi>t</mi>
          </mrow>
        </msub>
        <mo form="infix">−</mo>
        <mrow>
          <mo form="prefix">Δ</mo>
          <mi>u</mi>
        </mrow>
        <mo form="infix">+</mo>
        <msup>
          <mi>u</mi>
          <mn>5</mn>
        </msup>
        <mo form="infix">+</mo>
        <mrow>
          <mi>u</mi>
          <mo>⁢</mo>
          <msup>
            <mrow>
              <mo form="prefix" fence="true" stretchy="true" symmetric="true">|</mo>
              <mi>u</mi>
              <mo form="postfix" fence="true" stretchy="true" symmetric="true">|</mo>
            </mrow>
            <mrow>
              <mi>p</mi>
              <mo form="infix">−</mo>
              <mn>2</mn>
            </mrow>
          </msup>
        </mrow>
      </mrow>
      <mo form="infix">=</mo>
      <mn>0</mn>
    </mrow><mspace width="thickmathspace"/><mtext><bodyText> in  </bodyText></mtext><mspace width="thickmathspace"/><mrow>
      <msup>
        <mi mathvariant="bold">R</mi>
        <mn>3</mn>
      </msup>
      <mo form="infix">×</mo>
      <mrow>
        <mo form="prefix" fence="true" stretchy="true" symmetric="true">[</mo>
        <mrow>
          <mn>0</mn>
          <mo form="infix">,</mo>
          <mi>∞</mi>
        </mrow>
        <mo form="postfix" fence="true" stretchy="true" symmetric="true">[</mo>
      </mrow>
    </mrow></mrow></math></msidisplay>
 is automatically numbered as equation <xref key="eqn1" reftype="obj" req="varioref"/> . </bodyText>    </section><bodyText>
</bodyText><section xmlns="http://www.w3.org/1999/xhtml" id="tsid_3141600">
<sectiontitle>List environments</sectiontitle>
<bodyText>You can create numbered, bulleted, and description lists using the Item Tag popup list on the Tag toolbar. </bodyText>

<numberedlist>    
<numberedListItem><bodyText>List item 1 </bodyText>   </numberedListItem>
<numberedListItem><bodyText>List item 2 </bodyText>

<numberedlist>    
<numberedListItem><bodyText>A list item under a list item. </bodyText>The typeset style for this level is different than the screen style. The screen shows
a lower case alphabetic character followed by a period while the typeset style uses a lower case alphabetic character surrounded by parentheses.    </numberedListItem>
<numberedListItem><bodyText>Just another list item under a list item. </bodyText>

<numberedlist>    
<numberedListItem><bodyText>Third level list item under a list item. </bodyText>

<numberedlist>    
<numberedListItem><bodyText>Fourth and final level of list items allowed. 

</bodyText>

</numberedListItem>
</numberedlist>
 

</numberedListItem>
</numberedlist>
 

</numberedListItem>
</numberedlist>
 

</numberedListItem>
</numberedlist>
 

<bulletlist>    
<bulletListItem><bodyText>Bullet item 1 </bodyText>   </bulletListItem>
<bulletListItem><bodyText>Bullet item 2 </bodyText>

<bulletlist>    
<bulletListItem><bodyText>Second level bullet item. </bodyText>

<bulletlist>    
<bulletListItem><bodyText>Third level bullet item. </bodyText>

<bulletlist>    
<bulletListItem><bodyText>Fourth (and final) level bullet item. 

</bodyText>

</bulletListItem>
</bulletlist>
 

</bulletListItem>
</bulletlist>
 

</bulletListItem>
</bulletlist>
 

</bulletListItem>
</bulletlist>
 

<descriptionlist>    
<descriptionListItem><bodyText><descriptionLabel>Description List</descriptionLabel> Each description list item has a term followed by the description of
that term. Double click the term box to enter the term, or to change it. </bodyText>   </descriptionListItem>
<descriptionListItem><bodyText><descriptionLabel>Bunyip</descriptionLabel> Mythical beast of Australian Aboriginal legends. 

</bodyText>

</descriptionListItem>
</descriptionlist>
     </section><bodyText>

</bodyText><sectiontitle>Theorem-like Environments</sectiontitle><bodyText>
</bodyText><bodyText>The following theorem-like environments (in alphabetical order) are available in this style. </bodyText><bodyText>
<acknowledgement>
<bodyText> This is an acknowledgement 

</bodyText>
</acknowledgement>
 
</bodyText><algorithm>
<bodyText> This is an algorithm 

</bodyText>
</algorithm><bodyText>
 
</bodyText><axiom>
<bodyText> This is an axiom 

</bodyText>
</axiom><bodyText>
 
</bodyText><claim>
<bodyText> This is a claim 

</bodyText>
</claim><bodyText>
 
</bodyText><conclusion>
<bodyText> This is a conclusion 

</bodyText>
</conclusion><bodyText>
 
</bodyText><condition>
<bodyText> This is a condition 

</bodyText>
</condition><bodyText>
 
</bodyText><conjecture>
<bodyText> This is a conjecture 

</bodyText>
</conjecture><bodyText>
 
</bodyText><corollary>
<bodyText> This is a corollary 

</bodyText>
</corollary><bodyText>
 
</bodyText><criterion>
<bodyText> This is a criterion 

</bodyText>
</criterion><bodyText>
 
</bodyText><definition>
<bodyText> This is a definition 

</bodyText>
</definition><bodyText>
 
</bodyText><example>
<bodyText> This is an example 

</bodyText>
</example><bodyText>
 
</bodyText><exercise>
<bodyText> This is an exercise 

</bodyText>
</exercise><bodyText>
 
</bodyText><lemma>
<bodyText> This is a lemma 

</bodyText>
</lemma><bodyText>
 
</bodyText><proof>
<bodyText> This is the proof of the lemma. 
</bodyText>
</proof><bodyText>
 
</bodyText><notation>
<bodyText> This is notation 

</bodyText>
</notation><bodyText>
 
</bodyText><problem>
<bodyText> This is a problem 

</bodyText>
</problem><bodyText>
 
</bodyText><proposition>
<bodyText> This is a proposition 

</bodyText>
</proposition><bodyText>
 
</bodyText><remark>
<bodyText> This is a remark 

</bodyText>
</remark><bodyText>
 
</bodyText><solution>
<bodyText> This is a solution 

</bodyText>
</solution><bodyText>
 
</bodyText><summary>
<bodyText> This is a summary 

</bodyText>
</summary><bodyText>
 
</bodyText><theorem>
<bodyText> This is a theorem 

</bodyText>
</theorem><bodyText>
 
</bodyText><proof>
<bodyText> [Proof of the Main Theorem]This is the proof. 
</bodyText>
</proof><subsection id="tsid_3141611"><subsubsection id="tsid_3141614"><paragraph id="tsid_3141593"><subparagraph id="tsid_3141604"><bodyText>
              
            </bodyText>    
</subparagraph>

</paragraph>

</subsubsection>
</subsection><section msiSelectionManagerID="4" id="tsid_3141601"><bodyText>
              
            </bodyText><bodyText><subsection id="tsid_3141612"><subsubsection id="tsid_3141615"><paragraph id="tsid_3141594"><subparagraph id="tsid_3141605"><br/><acknowledgments>
                  <bodyText>Replace this text with your acknowledgments, or delete this section if acknowledgments are not used.<br type="_moz"/> </bodyText>
                  </acknowledgments>
                <bodyText><appendix/>* 
                </bodyText>
                
                
    <section id="tsid_3141602">
      <sectiontitle msiSelectionManagerID="8">Back matter<br/>
      </sectiontitle>
      <bodyText>This is the appendix.  If you have more than one appendix section, then delete the *
that follows the Start appendices here field in the paragraph before the appendix section heading. </bodyText>
      <bodyText xmlns="http://www.w3.org/1999/xhtml">Following is a short bibliography. It has no
relationship to the previous text, but can be used to show sample citations such as    <citation xmlns="http://www.w3.org/1999/xhtml" citekey="Hirsh64" hasRemark="false"/> and    <citation xmlns="http://www.w3.org/1999/xhtml" citekey="Reid87" hasRemark="false"/> . This typesetting style
places each citation inside
square brackets. If you want multiple citations to appear in a single
set of square brackets you must type all of the citation keys inside a single citation,
separating each with a comma. Here is an example:    <citation xmlns="http://www.w3.org/1999/xhtml" citekey="HB98a,HB98b,Hirsh64" hasRemark="false"/> . You can delete this sample manual bibliography and replace it with a BibTeX bibliography.<br/></bodyText>
                    <bodyText>
      <bibliography xmlns="http://www.w3.org/1999/xhtml">
        <bibitem bibitemkey="AmPetr92" hasLabel="false">
          <bodyText> American Petroleum Institute, Technical Data Book - Petroleum Refining, 5th
edition, 1992 
          </bodyText>
          <bibitem bibitemkey="HB98a" hasLabel="false">
            <bodyText> Harstad, K. and Bellan, J., ``Isolated fluid oxygen drop behavior in fluid
hydrogen at rocket chamber pressures'', <italics>Int. J. Heat Mass
Transfer</italics>, 1998a, <bold>41</bold>, 3537-3550 
            </bodyText>
            <bibitem bibitemkey="HB98b" hasLabel="false">
              <bodyText> Harstad, K. and Bellan, J., ``The Lewis number under supercritical
conditions'', <italics>Int. J. Heat Mass Transfer, </italics>in print 
              </bodyText>
              <bibitem bibitemkey="Hirsh64" hasLabel="false">
                <bodyText> Hirshfelder, J. O., Curtis, C. F. and Bird, R. B., <italics>Molecular
Theory of Gases and Liquids</italics>, John Wiley and Sons, Inc., 1964 
                </bodyText>
                <bibitem bibitemkey="Prausnitz86" hasLabel="false">
                  <bodyText> Prausnitz, J., Lichtenthaler, R. and de Azevedo, E., Molecular
thermodynamics for fluid-phase equilibrium, Prentice -Hall, Inc., 1986 
                  </bodyText>
                  <bibitem bibitemkey="Reid87" hasLabel="false">
                    <bodyText> Reid, R. C., Prausnitz, J. M. and Polling, B. E., The Properties of
Gases and Liquids, 4th Edition, McGraw-Hill Book Company, 1987   
                    </bodyText>
                  </bibitem>
                </bibitem>
              </bibitem>
            </bibitem>
          </bibitem>
        </bibitem>
      </bibliography>
    </bodyText>

                
                </section>                                
              </subparagraph>
            </paragraph>
          </subsubsection>
        </subsection>
      </bodyText>
    </section>
</body>
</html>PK     Zy0�                            css/PK     -�C��   �   
             "   css/my.cssPK     oCù�Y{  Y{  
             �   main.xhtmlPK      �   k|    