PK    �0�               css/PK    |�I@��F�   �   
   css/my.css@import url("resource://app/res/css/baselatex.css");
@import url("resource://app/res/css/latex.css");
@import url("resource://app/res/css/book.css");

PK    �0�            	   graphics/PK    �I@ȾY�p  �p  
   main.xhtml<?xml version="1.0" encoding="UTF-8"?>
<?xml-stylesheet href="css/my.css" type="text/css"?>
<?sw-tagdefs href="resource://app/res/tagdefs/latexdefs.xml" type="text/xml" ?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1 plus MathML 2.0//EN" "http://www.w3.org/Math/DTD/mathml2/xhtml-math11-f.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xmlns:mml="http://www.w3.org/1998/Math/MathML" xmlns:sw="http://www.sciword.com/namespaces/sciword">
<head>

<preamble> 
<documentclass class="book"/>  
<usepackage req="amsfonts"/>

<usepackage req="amsmath"/><texb enc="1" name="setcounter"> <![CDATA[\setcounter {MaxMatrixCols}{10}]]></texb>
<newtheorem name="theorem" style="" label="Theorem"/> <newtheorem name="acknowledgement" style="" counter="theorem" label="Acknowledgement"/>
 <newtheorem name="algorithm" style="" counter="theorem" label="Algorithm"/>
 <newtheorem name="axiom" style="" counter="theorem" label="Axiom"/>
 <newtheorem name="case" style="" counter="theorem" label="Case"/>
 <newtheorem name="claim" style="" counter="theorem" label="Claim"/>
 <newtheorem name="conclusion" style="" counter="theorem" label="Conclusion"/>
 <newtheorem name="condition" style="" counter="theorem" label="Condition"/>
 <newtheorem name="conjecture" style="" counter="theorem" label="Conjecture"/>
 <newtheorem name="corollary" style="" counter="theorem" label="Corollary"/>
 <newtheorem name="criterion" style="" counter="theorem" label="Criterion"/>
 <newtheorem name="definition" style="" counter="theorem" label="Definition"/>
 <newtheorem name="example" style="" counter="theorem" label="Example"/>
 <newtheorem name="exercise" style="" counter="theorem" label="Exercise"/>
 <newtheorem name="lemma" style="" counter="theorem" label="Lemma"/>
 <newtheorem name="notation" style="" counter="theorem" label="Notation"/>
 <newtheorem name="problem" style="" counter="theorem" label="Problem"/>
 <newtheorem name="proposition" style="" counter="theorem" label="Proposition"/>
 <newtheorem name="remark" style="" counter="theorem" label="Remark"/>
 <newtheorem name="solution" style="" counter="theorem" label="Solution"/>
 <newtheorem name="summary" style="" counter="theorem" label="Summary"/>
    <texb enc="1" name="makeatletter"><![CDATA[\makeatletter]]></texb> 

<docformat> <colist/> 
</docformat>  <preambleTeX><![CDATA[
\newenvironment {proof}[1][Proof]{\noindent \textbf {#1.} }{\ \rule {0.5em}{0.5em}}
\usepackage {bm}
\usepackage {tabulary}
\input {tcilatex}]]></preambleTeX> </preamble>
</head> 
<body> 
<bodyText>
<frontmatter/><title><short/><long msiSelectionManagerID="2">Standard </long>L<sup xmlns="http://www.w3.org/1999/xhtml">A</sup>T<sub xmlns="http://www.w3.org/1999/xhtml">E</sub>X <long xmlns="http://www.w3.org/1999/xhtml" msiSelectionManagerID="2">Book</long></title>
<author>The Author</author> 
<date>The Date</date> 
<maketitle/>
<maketoc/>
<makelof/>
<makelot/></bodyText>
<bodyText>
<mainmatter/></bodyText>
<part>
<sectiontitle>The First Part</sectiontitle> 
<chapter>
<sectiontitle>Standard L<sup xmlns="http://www.w3.org/1999/xhtml">A</sup>T<sub xmlns="http://www.w3.org/1999/xhtml">E</sub>X Book</sectiontitle> 
<bodyText>This document illustrates the appearance
of a book created with the shell <bold>Standard LaTeX Book</bold>. The shell
automatically adds blank pages after the title page, the table of contents,
the preface, and where necessary to ensure that new chapters begin on odd-numbered
pages. The shell doesn't contain an abstract. Blank pages carry headers
and page numbers. </bodyText>
<bodyText>The standard <texlogo xmlns="http://www.w3.org/1999/xhtml" name="latex">L<sup>A</sup>T<sub>E</sub>X</texlogo>
shells provide the most general and portable set of document features. You
can achieve almost any typesetting effect by beginning with a standard shell
and adding <texlogo xmlns="http://www.w3.org/1999/xhtml" name="latex">L<sup>A</sup>T<sub>E</sub>X</texlogo>
packages as necessary. </bodyText>
<bodyText>The document class base file
for this shell is <typewriter>book.cls</typewriter>. This typesetting specification
supports a number of class options. To see the available class options,
choose <sansSerif>Typeset, </sansSerif>choose <sansSerif>Options and Packages</sansSerif>,
select the <sansSerif>Class Options</sansSerif> tab, and then click the
<sansSerif>Modify</sansSerif> button. This shell uses the default class
options. </bodyText>
<bodyText>The typesetting specification for this shell
document uses these options and packages with the defaults indicated: </bodyText>
<centeredEnv><bodyText>
<bodyText> 
<table cellspacing="2" cellpadding="2" border="1" opt="ll"><tbody><tr><td><bodyText><bold>Options
and Packages</bold> <br type="_moz"/></bodyText></td><td><bodyText> <bold>Defaults</bold>
<br type="_moz"/></bodyText></td></tr><tr><td><bodyText><hline/>Document
class options <br type="_moz"/></bodyText></td><td><bodyText> Standard <br type="_moz"/></bodyText></td></tr><tr><td><bodyText>Packages:
<br type="_moz"/></bodyText></td><td><bodyText> <br type="_moz"/></bodyText></td></tr><tr><td><bodyText>
<hspace type="emSpace" dim="1em"/>amsfonts
<br type="_moz"/></bodyText></td><td><bodyText> None <br type="_moz"/></bodyText></td></tr><tr><td><bodyText>
<hspace type="emSpace" dim="1em"/>amsmath
<br type="_moz"/></bodyText></td><td><bodyText> Standard <br type="_moz"/></bodyText></td></tr><tr><td><bodyText><hline/><br type="_moz"/></bodyText></td></tr></tbody></table><br type="_moz"/>
 </bodyText>
</bodyText>
</centeredEnv>
 </chapter>
<chapter>
<sectiontitle>Using This Shell</sectiontitle> 
<bodyText>The front matter
of this shell has a number of sample entries that you should replace with
your own. Replace the body of this document with your own text. To start
with a blank document, you may delete the preliminary chapters and the text
in this document. Do not delete the [mainmatter] <texb enc="1" name="TeX"><![CDATA[\TeX {}]]></texb>
field found above in a paragraph by itself or the numbering of different
objects will be wrong. </bodyText>
<bodyText>Changes to the typeset format
of this shell and its associated <texlogo xmlns="http://www.w3.org/1999/xhtml" name="latex">L<sup>A</sup>T<sub>E</sub>X</texlogo>
formatting file (<typewriter>book.cls</typewriter>) are not supported by
MacKichan Software, Inc. If you want to make such changes, please consult
the <texlogo xmlns="http://www.w3.org/1999/xhtml" name="latex">L<sup>A</sup>T<sub>E</sub>X</texlogo>
manuals or a local <texlogo xmlns="http://www.w3.org/1999/xhtml" name="latex">L<sup>A</sup>T<sub>E</sub>X</texlogo>
expert. </bodyText>
<bodyText>If you modify this document and export it
as “Standard LaTeX Book.shl” in the <typewriter>Shells<math xmlns="http://www.w3.org/1998/Math/MathML">
  <mml:mrow>
    <mml:mo form="infix">\</mml:mo>
  </mml:mrow></math>Standard LaTeX</typewriter> directory, it will become
your new Standard LaTeX Book style shell. </bodyText></chapter>
<chapter>
<sectiontitle>Features of This Shell</sectiontitle> 
<section>
<sectiontitle>Section Heading</sectiontitle> 
<bodyText>Use the Section
tag for major sections, and the Subsection tag for subsections. </bodyText>
<subsection>
<sectiontitle>Subsection</sectiontitle> 
<bodyText>This is just some harmless
text under a subsection. </bodyText>
<subsubsection>
<sectiontitle>Subsubsection</sectiontitle> 
<bodyText>This is just some
harmless text under a subsubsection. </bodyText>
<paragraph>
<sectiontitle>Subsubsubsection</sectiontitle> 
<bodyText>This is just some
harmless text under a subsubsubsection. </bodyText>
<subparagraph>
<sectiontitle>Subsubsubsubsection</sectiontitle> 
<bodyText>This is just
some harmless text under a subsubsubsubsection. Here is Table <xref key="TableKey" reftype="obj" req="varioref"/>
 to demonstrate cross referencing a table and so that it will appear in
the list of tables, if used. This floating table was created using the <sansSerif>Table
- (4x3, floating)</sansSerif> fragment. <texb enc="1" name="LaTeX"><![CDATA[\LaTeX {}]]></texb>
will position the table to best take advantage of the flow of text.<texb enc="1" name="B"><![CDATA[\begin {table}[tbp] \centering ]]></texb>
<table cellspacing="2" cellpadding="2" border="1" opt="|l|l|l|"><tbody><tr><td><bodyText><hline/><bold>Head</bold>
<br type="_moz"/></bodyText></td><td><bodyText> <bold>Head</bold> <br type="_moz"/></bodyText></td><td><bodyText>
<bold>Head</bold> <br type="_moz"/></bodyText></td></tr><tr><td><bodyText><hline/>entry
<br type="_moz"/></bodyText></td><td><bodyText> entry <br type="_moz"/></bodyText></td><td><bodyText>
entry <br type="_moz"/></bodyText></td></tr><tr><td><bodyText>entry <br type="_moz"/></bodyText></td><td><bodyText>
entry <br type="_moz"/></bodyText></td><td><bodyText> entry <br type="_moz"/></bodyText></td></tr><tr><td><bodyText>entry
<br type="_moz"/></bodyText></td><td><bodyText> entry <br type="_moz"/></bodyText></td><td><bodyText>
entry <br type="_moz"/></bodyText></td></tr><tr><td><bodyText><hline/><br type="_moz"/></bodyText></td></tr></tbody></table><br type="_moz"/>
<texb enc="1" name="caption"><![CDATA[\caption{Table Caption}]]></texb><marker id="TableKey"/><texb enc="1" name="E"><![CDATA[\end {table}]]></texb>
</bodyText>
</subparagraph>

</paragraph>

</subsubsection>

</subsection>
</section>
<section>
<sectiontitle>Tags</sectiontitle> 
<bodyText>You can apply the logical markup
tag <emphasized>Emphasized</emphasized>. </bodyText>
<bodyText>You can apply
the visual markup tags <bold>Bold</bold>, <italics>Italics</italics>, <roman>Roman</roman>,
<sansSerif>Sans Serif</sansSerif>, <slanted>Slanted</slanted>, <smallCaps>Small
Caps</smallCaps>, and <typewriter>Typewriter</typewriter>. </bodyText>
<bodyText>You
can apply the special mathematics-only tags <math xmlns="http://www.w3.org/1998/Math/MathML">
  <mml:mrow>
    <mml:mi mathvariant="double-struck">B</mml:mi>
    <mml:mo>⁢</mml:mo>
    <mml:mi mathvariant="double-struck">L</mml:mi>
    <mml:mo>⁢</mml:mo>
    <mml:mi mathvariant="double-struck">A</mml:mi>
    <mml:mo>⁢</mml:mo>
    <mml:mi mathvariant="double-struck">C</mml:mi>
    <mml:mo>⁢</mml:mo>
    <mml:mi mathvariant="double-struck">K</mml:mi>
    <mml:mo>⁢</mml:mo>
    <mml:mi mathvariant="double-struck">B</mml:mi>
    <mml:mo>⁢</mml:mo>
    <mml:mi mathvariant="double-struck">O</mml:mi>
    <mml:mo>⁢</mml:mo>
    <mml:mi mathvariant="double-struck">A</mml:mi>
    <mml:mo>⁢</mml:mo>
    <mml:mi mathvariant="double-struck">R</mml:mi>
    <mml:mo>⁢</mml:mo>
    <mml:mi mathvariant="double-struck">D</mml:mi>
  </mml:mrow></math> <math xmlns="http://www.w3.org/1998/Math/MathML">
  <mml:mrow>
    <mml:mi mathvariant="double-struck">B</mml:mi>
    <mml:mo>⁢</mml:mo>
    <mml:mi mathvariant="double-struck">O</mml:mi>
    <mml:mo>⁢</mml:mo>
    <mml:mi mathvariant="double-struck">L</mml:mi>
    <mml:mo>⁢</mml:mo>
    <mml:mi mathvariant="double-struck">D</mml:mi>
  </mml:mrow></math>, <math xmlns="http://www.w3.org/1998/Math/MathML">
  <mml:mrow>
    <mml:mi mathvariant="script">C</mml:mi>
    <mml:mo>⁢</mml:mo>
    <mml:mi mathvariant="script">A</mml:mi>
    <mml:mo>⁢</mml:mo>
    <mml:mi mathvariant="script">L</mml:mi>
    <mml:mo>⁢</mml:mo>
    <mml:mi mathvariant="script">L</mml:mi>
    <mml:mo>⁢</mml:mo>
    <mml:mi mathvariant="script">I</mml:mi>
    <mml:mo>⁢</mml:mo>
    <mml:mi mathvariant="script">G</mml:mi>
    <mml:mo>⁢</mml:mo>
    <mml:mi mathvariant="script">R</mml:mi>
    <mml:mo>⁢</mml:mo>
    <mml:mi mathvariant="script">A</mml:mi>
    <mml:mo>⁢</mml:mo>
    <mml:mi mathvariant="script">P</mml:mi>
    <mml:mo>⁢</mml:mo>
    <mml:mi mathvariant="script">H</mml:mi>
    <mml:mo>⁢</mml:mo>
    <mml:mi mathvariant="script">I</mml:mi>
    <mml:mo>⁢</mml:mo>
    <mml:mi mathvariant="script">C</mml:mi>
  </mml:mrow></math>, and <math xmlns="http://www.w3.org/1998/Math/MathML">
  <mml:mrow>
    <mml:mi mathvariant="fraktur">f</mml:mi>
    <mml:mo>⁢</mml:mo>
    <mml:mi mathvariant="fraktur">r</mml:mi>
    <mml:mo>⁢</mml:mo>
    <mml:mi mathvariant="fraktur">a</mml:mi>
    <mml:mo>⁢</mml:mo>
    <mml:mi mathvariant="fraktur">k</mml:mi>
    <mml:mo>⁢</mml:mo>
    <mml:mi mathvariant="fraktur">t</mml:mi>
    <mml:mo>⁢</mml:mo>
    <mml:mi mathvariant="fraktur">u</mml:mi>
    <mml:mo>⁢</mml:mo>
    <mml:mi mathvariant="fraktur">r</mml:mi>
  </mml:mrow></math>. Note that blackboard bold and calligraphic are correct
only when applied to uppercase letters A through Z. </bodyText>
<bodyText>You
can apply the size tags <tiny>tiny</tiny>, <scriptsize>scriptsize</scriptsize>,
<footnotesize>footnotesize</footnotesize>, <small>small</small>, <normalsize>normalsize</normalsize>,
<large>large</large>, <Large>Large</Large>, <LARGE>LARGE</LARGE>, <huge>huge</huge>
and <Huge>Huge</Huge>. </bodyText>

<bodyText><QTP type="Body Math"> This is a Body Math paragraph. Each time
you press the Enter key, Scientific WorkPlace switches to mathematics mode.
This is convenient for carrying out ``scratchpad'' computations. 
</QTP>
</bodyText>Following is a group of paragraphs marked as Short Quote. This
environment is appropriate for a short quotation or a sequence of short
quotations. 
<shortQuote><bodyText>
<bodyText>The only thing we have to fear is fear
itself. <emphasized>Franklin D. Roosevelt, </emphasized>Mar. 4, 1933 </bodyText>
<bodyText>Ask
not what your country can do for you; ask what you can do for your country.
<emphasized>John F. Kennedy, </emphasized>Jan. 20. 1961 </bodyText>
<bodyText>There
is nothing wrong with America that cannot be cured by what is right with
America. <emphasized>William J. “Bill” Clinton, </emphasized>Jan.
21, 1993 </bodyText>
</bodyText>
</shortQuote>
 
<bodyText>The Long Quotation tag is used for quotations of more than one
paragraph. Following is the beginning of <emphasized>Alice's Adventures
in Wonderland </emphasized>by Lewis Carroll: </bodyText>
<longQuotation><bodyText>
<bodyText>Alice was beginning to get very tired
of sitting by her sister on the bank, and of having nothing to do: once
or twice she had peeped into the book her sister was reading, but it had
no pictures or conversations in it, `and what is the use of a book,' thought
Alice `without pictures or conversation?' </bodyText>
<bodyText>So she was
considering in her own mind (as well as she could, for the hot day made
her feel very sleepy and stupid), whether the pleasure of making a daisy-chain
would be worth the trouble of getting up and picking the daisies, when suddenly
a White Rabbit with pink eyes ran close by her. </bodyText>
<bodyText>There
was nothing so very remarkable in that; nor did Alice think it so very much
out of the way to hear the Rabbit say to itself, `Oh dear! Oh dear! I shall
be late!' (when she thought it over afterwards, it occurred to her that
she ought to have wondered at this, but at the time it all seemed quite
natural); but when the Rabbit actually took a watch out of its waistcoat-pocket,
and looked at it, and then hurried on, Alice started to her feet, for it
flashed across her mind that she had never before seen a rabbit with either
a waistcoat-pocket, or a watch to take out of it, and burning with curiosity,
she ran across the field after it, and fortunately was just in time to see
it pop down a large rabbit-hole under the hedge. </bodyText>
<bodyText>In
another moment down went Alice after it, never once considering how in the
world she was to get out again. </bodyText>
</bodyText>
</longQuotation>
 
<bodyText>Use the Verbatim tag when you want <texlogo xmlns="http://www.w3.org/1999/xhtml" name="latex">L<sup>A</sup>T<sub>E</sub>X</texlogo>
to preserve spacing, perhaps when including a fragment from a program such
as: 
 
<verbatim><bodyText><br/>#include &lt;iostream<rangle/>        // &lt; <rangle/><br/>is used for standard libraries.<br/>void main(void)            // "main" method<br/>always called first.<br/>{<br/> cout &lt;&lt; "Hello World.";  // Send to output<br/>stream.<br/>}<br/>--BG--</bodyText> </verbatim>
 </bodyText></section>
<section>
<sectiontitle>Mathematics and Text</sectiontitle> 
<bodyText>Let <math xmlns="http://www.w3.org/1998/Math/MathML">
  <mml:mrow>
    <mml:mi>H</mml:mi>
  </mml:mrow></math> be a Hilbert space, <math xmlns="http://www.w3.org/1998/Math/MathML">
  <mml:mrow>
    <mml:mi>C</mml:mi>
  </mml:mrow></math> be a closed bounded convex subset of <math xmlns="http://www.w3.org/1998/Math/MathML">
  <mml:mrow>
    <mml:mi>H</mml:mi>
  </mml:mrow></math>, <math xmlns="http://www.w3.org/1998/Math/MathML">
  <mml:mrow>
    <mml:mi>T</mml:mi>
  </mml:mrow></math> a nonexpansive self map of <math xmlns="http://www.w3.org/1998/Math/MathML">
  <mml:mrow>
    <mml:mi>C</mml:mi>
  </mml:mrow></math>. Suppose that as <math xmlns="http://www.w3.org/1998/Math/MathML">
  <mml:mrow>
    <mml:mi>n</mml:mi>
    <mml:mo form="infix">→</mml:mo>
    <mml:mi>∞</mml:mi>
  </mml:mrow></math>, <math xmlns="http://www.w3.org/1998/Math/MathML">
  <mml:mrow>
    <mml:msub>
      <mml:mi>a</mml:mi>
      <mml:mrow>
        <mml:mi>n</mml:mi>
        <mml:mo form="infix">,</mml:mo>
        <mml:mi>k</mml:mi>
      </mml:mrow>
    </mml:msub>
    <mml:mo form="infix">→</mml:mo>
    <mml:mn>0</mml:mn>
  </mml:mrow></math> for each <math xmlns="http://www.w3.org/1998/Math/MathML">
  <mml:mrow>
    <mml:mi>k</mml:mi>
  </mml:mrow></math>, and <math xmlns="http://www.w3.org/1998/Math/MathML">
  <mml:mrow>
    <mml:mrow>
      <mml:msub>
        <mml:mi>γ</mml:mi>
        <mml:mi>n</mml:mi>
      </mml:msub>
      <mml:mo form="infix">=</mml:mo>
      <mml:mrow>
        <mml:munderover>
          <mml:mo form="prefix" movablelimits="true" largeop="true">∑</mml:mo>
          <mml:mrow>
            <mml:mi>k</mml:mi>
            <mml:mo form="infix">=</mml:mo>
            <mml:mn>0</mml:mn>
          </mml:mrow>
          <mml:mi>∞</mml:mi>
        </mml:munderover>
        <mml:msup>
          <mml:mrow>
            <mml:mo form="prefix" fence="true" stretchy="true" symmetric="true">(</mml:mo>
            <mml:mrow>
              <mml:msub>
                <mml:mi>a</mml:mi>
                <mml:mrow>
                  <mml:mi>n</mml:mi>
                  <mml:mo form="infix">,</mml:mo>
                  <mml:mrow>
                    <mml:mi>k</mml:mi>
                    <mml:mo form="infix">+</mml:mo>
                    <mml:mn>1</mml:mn>
                  </mml:mrow>
                </mml:mrow>
              </mml:msub>
              <mml:mo form="infix">−</mml:mo>
              <mml:msub>
                <mml:mi>a</mml:mi>
                <mml:mrow>
                  <mml:mi>n</mml:mi>
                  <mml:mo form="infix">,</mml:mo>
                  <mml:mi>k</mml:mi>
                </mml:mrow>
              </mml:msub>
            </mml:mrow>
            <mml:mo form="postfix" fence="true" stretchy="true" symmetric="true">)</mml:mo>
          </mml:mrow>
          <mml:mo>+</mml:mo>
        </mml:msup>
      </mml:mrow>
    </mml:mrow>
    <mml:mo form="infix">→</mml:mo>
    <mml:mn>0</mml:mn>
  </mml:mrow></math>. Then for each <math xmlns="http://www.w3.org/1998/Math/MathML">
  <mml:mrow>
    <mml:mi>x</mml:mi>
  </mml:mrow></math> in <math xmlns="http://www.w3.org/1998/Math/MathML">
  <mml:mrow>
    <mml:mi>C</mml:mi>
  </mml:mrow></math>, <math xmlns="http://www.w3.org/1998/Math/MathML">
  <mml:mrow>
    <mml:mrow>
      <mml:msub>
        <mml:mi>A</mml:mi>
        <mml:mi>n</mml:mi>
      </mml:msub>
      <mml:mo>⁢</mml:mo>
      <mml:mi>x</mml:mi>
    </mml:mrow>
    <mml:mo form="infix">=</mml:mo>
    <mml:mrow>
      <mml:munderover>
        <mml:mo form="prefix" movablelimits="true" largeop="true">∑</mml:mo>
        <mml:mrow>
          <mml:mi>k</mml:mi>
          <mml:mo form="infix">=</mml:mo>
          <mml:mn>0</mml:mn>
        </mml:mrow>
        <mml:mi>∞</mml:mi>
      </mml:munderover>
      <mml:mrow>
        <mml:msub>
          <mml:mi>a</mml:mi>
          <mml:mrow>
            <mml:mi>n</mml:mi>
            <mml:mo form="infix">,</mml:mo>
            <mml:mi>k</mml:mi>
          </mml:mrow>
        </mml:msub>
        <mml:mo>⁢</mml:mo>
        <mml:msup>
          <mml:mi>T</mml:mi>
          <mml:mi>k</mml:mi>
        </mml:msup>
        <mml:mo>⁢</mml:mo>
        <mml:mi>x</mml:mi>
      </mml:mrow>
    </mml:mrow>
  </mml:mrow></math> converges weakly to a fixed point of <math xmlns="http://www.w3.org/1998/Math/MathML">
  <mml:mrow>
    <mml:mi>T</mml:mi>
  </mml:mrow></math> . </bodyText>
<bodyText>The numbered equation 
<msidisplay><math xmlns="http://www.w3.org/1998/Math/MathML" display="block">
  <mml:mrow key="eqn1">
    <mml:mrow>
      <mml:mrow>
        <mml:msub>
          <mml:mi>u</mml:mi>
          <mml:mrow>
            <mml:mi>t</mml:mi>
            <mml:mo>⁣</mml:mo>
            <mml:mi>t</mml:mi>
          </mml:mrow>
        </mml:msub>
        <mml:mo form="infix">−</mml:mo>
        <mml:mrow>
          <mml:mo form="prefix">Δ</mml:mo>
          <mml:mi>u</mml:mi>
        </mml:mrow>
        <mml:mo form="infix">+</mml:mo>
        <mml:msup>
          <mml:mi>u</mml:mi>
          <mml:mn>5</mml:mn>
        </mml:msup>
        <mml:mo form="infix">+</mml:mo>
        <mml:mrow>
          <mml:mi>u</mml:mi>
          <mml:mo>⁢</mml:mo>
          <mml:msup>
            <mml:mrow>
              <mml:mo form="prefix" fence="true" stretchy="true" symmetric="true">|</mml:mo>
              <mml:mi>u</mml:mi>
              <mml:mo form="postfix" fence="true" stretchy="true" symmetric="true">|</mml:mo>
            </mml:mrow>
            <mml:mrow>
              <mml:mi>p</mml:mi>
              <mml:mo form="infix">−</mml:mo>
              <mml:mn>2</mml:mn>
            </mml:mrow>
          </mml:msup>
        </mml:mrow>
      </mml:mrow>
      <mml:mo form="infix">=</mml:mo>
      <mml:mn>0</mml:mn>
    </mml:mrow>
    <mml:mtext>{<bodyText> in } </bodyText></mml:mtext>
    <mml:mrow>
      <mml:msup>
        <mml:mi mathvariant="bold">R</mml:mi>
        <mml:mn>3</mml:mn>
      </mml:msup>
      <mml:mo form="infix">×</mml:mo>
      <mml:mrow>
        <mml:mo form="prefix" fence="true" stretchy="true" symmetric="true">[</mml:mo>
        <mml:mrow>
          <mml:mn>0</mml:mn>
          <mml:mo form="infix">,</mml:mo>
          <mml:mi>∞</mml:mi>
        </mml:mrow>
        <mml:mo form="postfix" fence="true" stretchy="true" symmetric="true">[</mml:mo>
      </mml:mrow>
    </mml:mrow>
  </mml:mrow></math></msidisplay>is automatically numbered as equation <xref key="eqn1" reftype="obj" req="varioref"/>
. </bodyText></section>
<section>
<sectiontitle>Lists Environments</sectiontitle> 
<bodyText>You can create
numbered, bulleted, and description lists using the Item Tag popup list
on the Tag toolbar. </bodyText>

<numberedlist>  
<numberedListItem><bodyText>List item 1 </bodyText></numberedListItem>
<numberedListItem><bodyText>List
item 2 </bodyText>

<numberedlist>  
<numberedListItem><bodyText>A list item under a list item.
</bodyText>The typeset style for this level is different than the screen
style. The screen shows a lower case alphabetic character followed by a
period while the typeset style uses a lower case alphabetic character surrounded
by parentheses. </numberedListItem>
<numberedListItem><bodyText>Just another
list item under a list item. </bodyText>

<numberedlist>  
<numberedListItem><bodyText>Third level list item under
a list item. </bodyText>

<numberedlist>  
<numberedListItem><bodyText>Fourth and final level of list
items allowed. 

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
<bulletListItem><bodyText>Bullet item 1 </bodyText></bulletListItem>
<bulletListItem><bodyText>Bullet
item 2 </bodyText>

<bulletlist>  
<bulletListItem><bodyText>Second level bullet item. </bodyText>

<bulletlist>  
<bulletListItem><bodyText>Third level bullet item. </bodyText>

<bulletlist>  
<bulletListItem><bodyText>Fourth (and final) level bullet
item. 

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
<descriptionListItem><explicit-item>Description List</explicit-item>
Each description list item has a term followed by the description of that
term. Double click the term box to enter the term, or to change it. </descriptionListItem>
<descriptionListItem><explicit-item>Bunyip</explicit-item>
Mythical beast of Australian Aboriginal legends. 

</descriptionListItem>
</descriptionlist>
 </section>
<section>
<sectiontitle>Theorem-Like Environments</sectiontitle> 
<bodyText>The following
theorem-like environments (in alphabetical order) are available in this
style. </bodyText>

<bodyText>
<acknowledgement><bodyText> This is an acknowledgement 
</bodyText></acknowledgement>
 </bodyText>

<bodyText>
<algorithm><bodyText> This is an algorithm 
</bodyText></algorithm>
 </bodyText>

<bodyText>
<axiom><bodyText> This is an axiom 
</bodyText></axiom>
 </bodyText>

<bodyText>
<case><bodyText> This is a case 
</bodyText></case>
 </bodyText>

<bodyText>
<claim><bodyText> This is a claim 
</bodyText></claim>
 </bodyText>

<bodyText>
<conclusion><bodyText> This is a conclusion 
</bodyText></conclusion>
 </bodyText>

<bodyText>
<condition><bodyText> This is a condition 
</bodyText></condition>
 </bodyText>

<bodyText>
<conjecture><bodyText> This is a conjecture 
</bodyText></conjecture>
 </bodyText>

<bodyText>
<corollary><bodyText> This is a corollary 
</bodyText></corollary>
 </bodyText>

<bodyText>
<criterion><bodyText> This is a criterion 
</bodyText></criterion>
 </bodyText>

<bodyText>
<definition><bodyText> This is a definition 
</bodyText></definition>
 </bodyText>

<bodyText>
<example><bodyText> This is an example 
</bodyText></example>
 </bodyText>

<bodyText>
<exercise><bodyText> This is an exercise 
</bodyText></exercise>
 </bodyText>

<bodyText>
<lemma><bodyText> This is a lemma 
</bodyText></lemma>
 </bodyText>
<proof>
<bodyText>This is the proof of the lemma. 
</bodyText>
</proof>
 

<bodyText>
<notation><bodyText> This is notation 
</bodyText></notation>
 </bodyText>

<bodyText>
<problem><bodyText> This is a problem 
</bodyText></problem>
 </bodyText>

<bodyText>
<proposition><bodyText> This is a proposition 
</bodyText></proposition>
 </bodyText>

<bodyText>
<remark><bodyText> This is a remark 
</bodyText></remark>
 </bodyText>

<bodyText>
<summary><bodyText> This is a summary 
</bodyText></summary>
 </bodyText>

<bodyText>
<theorem><bodyText> This is a theorem 
</bodyText></theorem>
 </bodyText>
<proof>
<bodyText>[Proof of the Main Theorem] This is the proof. 
</bodyText>
</proof>
 
</section>
</chapter>
<chapter>
<sectiontitle><shortTitle>Short Chapter Title</shortTitle>Chapter and Sections
With Short Heading Titles</sectiontitle> 
<section>
<sectiontitle><shortTitle>Short Section Title</shortTitle>This is my section</sectiontitle>

<bodyText>Just some harmless text after the section. </bodyText>
<subsection>
<sectiontitle><shortTitle>Short subtitle</shortTitle>This is my subsection</sectiontitle>

<bodyText>Just some harmless text after the subsection. </bodyText>
<subsubsection>
<sectiontitle><shortTitle>Short subsubtitle</shortTitle>This is my subsubsection</sectiontitle>

<bodyText>Just some harmless text after the subsubsection. </bodyText>
<paragraph>
<sectiontitle><shortTitle>Short partitle</shortTitle>This is my paragraph</sectiontitle>

<bodyText>Just some harmless text after the paragraph </bodyText>
<subparagraph>
<sectiontitle><shortTitle>Short subpartitle</shortTitle>This is my subparagraph</sectiontitle>
</subparagraph>
<subparagraph>
<sectiontitle><shortTitle>Short subpartitle2</shortTitle>This is another
subparagraph</sectiontitle> 
<bodyText>Just some harmless text following.
</bodyText>
<bodyText>
<appendix/></bodyText>
</subparagraph>

</paragraph>

</subsubsection>

</subsection>

</section>
</chapter>
<chapter>
<sectiontitle>The First Appendix</sectiontitle> 
<bodyText>The appendix
fragment is used only once. Subsequent appendices can be created using the
Chapter Section/Body Tag. </bodyText>
<bodyText>
<backmatter/></bodyText></chapter>
<chapter>
<sectiontitle>Afterword</sectiontitle> 
<bodyText>The back matter often
includes one or more of an index, an afterword, acknowledgements, a bibliography,
a colophon, or any other similar item. In the back matter, chapters do not
produce a chapter number, but they are entered in the table of contents.
If you are not using anything in the back matter, you can delete the back
matter <texb enc="1" name="TeX"><![CDATA[\TeX {}]]></texb> field and everything
that follows it. </bodyText>
<bodyText>

</bodyText>

</chapter>

</part>
</body> 
</html>PK    �0�               plots/PK     �0�                            css/PK     |�I@��F�   �   
             "   css/my.cssPK     �0�            	            �   graphics/PK     �I@ȾY�p  �p  
               main.xhtmlPK     �0�                        r  plots/PK        :r    