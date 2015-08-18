<xsl:stylesheet version="1.1" 
    xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:mml="http://www.w3.org/1998/Math/MathML"
    xmlns:html="http://www.w3.org/1999/xhtml"
    xmlns:sw="http://www.sciword.com/namespaces/sciword"
>  

<xsl:template match="html:french">
  \begin{french}
  <xsl:apply-templates />
  \end{french}
</xsl:template>

<xsl:template match="html:textfrench">
  \textfrench{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:USenglish">
  \begin{USEnglish}
  <xsl:apply-templates />
  \end{USEnglish}
</xsl:template>

<xsl:template match="html:textUSenglish">
  \textUSEnglish{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:UKenglish">
  \begin{UKenglish}
  <xsl:apply-templates />
  \end{UKenglish}
</xsl:template>

<xsl:template match="html:textUKenglish">
  \textUKenglish{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:australian">
  \begin{australian}
  <xsl:apply-templates />
  \end{australian}
</xsl:template>

<xsl:template match="html:textaustralian">
  \textaustralian{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:canadian">
  \begin{canadian}
  <xsl:apply-templates />
  \end{canadian}
</xsl:template>

<xsl:template match="html:textcanadian">
  \textcanadian{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:newzealand">
  \begin{newzealand}
  <xsl:apply-templates />
  \end{newzealand}
</xsl:template>

<xsl:template match="html:textnewzealand">
  \textnewzealand{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:austrian">
  \begin{austrian}
  <xsl:apply-templates />
  \end{austrian}
</xsl:template>

<xsl:template match="html:textaustrian">
  \textaustrian{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:naustrian">
  \begin{naustrian}
  <xsl:apply-templates />
  \end{naustrian}
</xsl:template>

<xsl:template match="html:textnaustrian">
  \textnaustrian{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:german">
  \begin{german}
  <xsl:apply-templates />
  \end{german}
</xsl:template>

<xsl:template match="html:textgerman">
  \textgerman{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:germanb">
  \begin{germanb}
  <xsl:apply-templates />
  \end{germanb}
</xsl:template>

<xsl:template match="html:textgermanb">
  \textgermanb{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:ngerman">
  \begin{ngerman}
  <xsl:apply-templates />
  \end{ngerman}
</xsl:template>

<xsl:template match="html:textngerman">
  \textngerman{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:monogreek">
  \begin{monogreek}
  <xsl:apply-templates />
  \end{monogreek}
</xsl:template>

<xsl:template match="html:textmonogreek">
  \textmonogreek{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:polygreek">
  \begin{polygreek}
  <xsl:apply-templates />
  \end{polygreek}
</xsl:template>

<xsl:template match="html:textpolygreek">
  \textpolygreek{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:polutonikogreek">
  \begin{polutonikogreek}
  <xsl:apply-templates />
  \end{polutonikogreek}
</xsl:template>

<xsl:template match="html:textpolutonikogreek">
  \textpolutonikogreek{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:ancientgreek">
  \begin{ancientgreek}
  <xsl:apply-templates />
  \end{ancientgreek}
</xsl:template>

<xsl:template match="html:textancientgreek">
  \textancientgreek{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:francais">
  \begin{francais}
  <xsl:apply-templates />
  \end{francais}
</xsl:template>

<xsl:template match="html:textfrancais">
  \textfrancais{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:frenchb">
  \begin{frenchb}
  <xsl:apply-templates />
  \end{frenchb}
</xsl:template>

<xsl:template match="html:textfrenchb">
  \textfrenchb{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:french">
  \begin{french}
  <xsl:apply-templates />
  \end{french}
</xsl:template>

<xsl:template match="html:textfrench">
  \textfrench{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:acadian">
  \begin{acadian}
  <xsl:apply-templates />
  \end{acadian}
</xsl:template>

<xsl:template match="html:textacadian">
  \textacadian{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:albanian">
  \begin{albanian}
  <xsl:apply-templates />
  \end{albanian}
</xsl:template>

<xsl:template match="html:textalbanian">
  \textalbanian{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:afrikaans">
  \begin{afrikaans}
  <xsl:apply-templates />
  \end{afrikaans}
</xsl:template>

<xsl:template match="html:textafrikaans">
  \textafrikaans{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:Arabic">
  \begin{Arabic}
  <xsl:apply-templates />
  \end{Arabic}
</xsl:template>

<xsl:template match="html:textarabic">
  \textarabic{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:bahasa">
  \begin{bahasa}
  <xsl:apply-templates />
  \end{bahasa}
</xsl:template>

<xsl:template match="html:textbahasa">
  \textbahasa{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:indonesian">
  \begin{indonesian}
  <xsl:apply-templates />
  \end{indonesian}
</xsl:template>

<xsl:template match="html:textindonesian">
  \textindonesian{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:indon">
  \begin{indon}
  <xsl:apply-templates />
  \end{indon}
</xsl:template>

<xsl:template match="html:textindon">
  \textindon{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:bahasai">
  \begin{bahasai}
  <xsl:apply-templates />
  \end{bahasai}
</xsl:template>

<xsl:template match="html:textbahasai">
  \textbahasai{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:bahasam">
  \begin{bahasam}
  <xsl:apply-templates />
  \end{bahasam}
</xsl:template>

<xsl:template match="html:textbahasam">
  \textbahasam{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:basque">
  \begin{basque}
  <xsl:apply-templates />
  \end{basque}
</xsl:template>

<xsl:template match="html:textbasque">
  \textbasque{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:brazil">
  \begin{brazil}
  <xsl:apply-templates />
  \end{brazil}
</xsl:template>

<xsl:template match="html:textbrazil">
  \textbrazil{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:brazilian">
  \begin{brazilian}
  <xsl:apply-templates />
  \end{brazilian}
</xsl:template>

<xsl:template match="html:textbrazilian">
  \textbrazilian{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:breton">
  \begin{breton}
  <xsl:apply-templates />
  \end{breton}
</xsl:template>

<xsl:template match="html:textbreton">
  \textbreton{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:bulgarian">
  \begin{bulgarian}
  <xsl:apply-templates />
  \end{bulgarian}
</xsl:template>

<xsl:template match="html:textbulgarian">
  \textbulgarian{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:canadien">
  \begin{canadien}
  <xsl:apply-templates />
  \end{canadien}
</xsl:template>

<xsl:template match="html:textcanadien">
  \textcanadien{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:catalan">
  \begin{catalan}
  <xsl:apply-templates />
  \end{catalan}
</xsl:template>

<xsl:template match="html:textcatalan">
  \textcatalan{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:coptic">
  \begin{coptic}
  <xsl:apply-templates />
  \end{coptic}
</xsl:template>

<xsl:template match="html:textcoptic">
  \textcoptic{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:croatian">
  \begin{croatian}
  <xsl:apply-templates />
  \end{croatian}
</xsl:template>

<xsl:template match="html:textcroatian">
  \textcroatian{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:czech">
  \begin{czech}
  <xsl:apply-templates />
  \end{czech}
</xsl:template>

<xsl:template match="html:textczech">
  \textczech{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:danish">
  \begin{danish}
  <xsl:apply-templates />
  \end{danish}
</xsl:template>

<xsl:template match="html:textdanish">
  \textdanish{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:divehi">
  \begin{divehi}
  <xsl:apply-templates />
  \end{divehi}
</xsl:template>

<xsl:template match="html:textdivehi">
  \textdivehi{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:dutch">
  \begin{dutch}
  <xsl:apply-templates />
  \end{dutch}
</xsl:template>

<xsl:template match="html:textdutch">
  \textdutch{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:esperanto">
  \begin{esperanto}
  <xsl:apply-templates />
  \end{esperanto}
</xsl:template>

<xsl:template match="html:textesperanto">
  \textesperanto{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:estonian">
  \begin{estonian}
  <xsl:apply-templates />
  \end{estonian}
</xsl:template>

<xsl:template match="html:textestonian">
  \textestonian{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:farsi">
  \begin{farsi}
  <xsl:apply-templates />
  \end{farsi}
</xsl:template>

<xsl:template match="html:textfarsi">
  \textfarsi{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:finnish">
  \begin{finnish}
  <xsl:apply-templates />
  \end{finnish}
</xsl:template>

<xsl:template match="html:textfinnish">
  \textfinnish{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:galician">
  \begin{galician}
  <xsl:apply-templates />
  \end{galician}
</xsl:template>

<xsl:template match="html:textgalician">
  \textgalician{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:hebrew">
  \begin{hebrew}
  <xsl:apply-templates />
  \end{hebrew}
</xsl:template>

<xsl:template match="html:texthebrew">
  \texthebrew{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:hungarian">
  \begin{hungarian}
  <xsl:apply-templates />
  \end{hungarian}
</xsl:template>

<xsl:template match="html:texthungarian">
  \texthungarian{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:icelandic">
  \begin{icelandic}
  <xsl:apply-templates />
  \end{icelandic}
</xsl:template>

<xsl:template match="html:texticelandic">
  \texticelandic{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:interlingua">
  \begin{interlingua}
  <xsl:apply-templates />
  \end{interlingua}
</xsl:template>

<xsl:template match="html:textinterlingua">
  \textinterlingua{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:irish">
  \begin{irish}
  <xsl:apply-templates />
  \end{irish}
</xsl:template>

<xsl:template match="html:textirish">
  \textirish{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:italian">
  \begin{italian}
  <xsl:apply-templates />
  \end{italian}
</xsl:template>

<xsl:template match="html:textitalian">
  \textitalian{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:latin">
  \begin{latin}
  <xsl:apply-templates />
  \end{latin}
</xsl:template>

<xsl:template match="html:textlatin">
  \textlatin{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:lowersorbian">
  \begin{lowersorbian}
  <xsl:apply-templates />
  \end{lowersorbian}
</xsl:template>

<xsl:template match="html:textlowersorbian">
  \textlowersorbian{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:magyar">
  \begin{magyar}
  <xsl:apply-templates />
  \end{magyar}
</xsl:template>

<xsl:template match="html:textmagyar">
  \textmagyar{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:malay">
  \begin{malay}
  <xsl:apply-templates />
  \end{malay}
</xsl:template>

<xsl:template match="html:textmalay">
  \textmalay{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:meyalu">
  \begin{meyalu}
  <xsl:apply-templates />
  \end{meyalu}
</xsl:template>

<xsl:template match="html:textmeyalu">
  \textmeyalu{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:norsk">
  \begin{norsk}
  <xsl:apply-templates />
  \end{norsk}
</xsl:template>

<xsl:template match="html:textnorsk">
  \textnorsk{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:samin">
  \begin{samin}
  <xsl:apply-templates />
  \end{samin}
</xsl:template>

<xsl:template match="html:textsamin">
  \textsamin{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:nynorsk">
  \begin{nynorsk}
  <xsl:apply-templates />
  \end{nynorsk}
</xsl:template>

<xsl:template match="html:textnynorsk">
  \textnynorsk{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:polish">
  \begin{polish}
  <xsl:apply-templates />
  \end{polish}
</xsl:template>

<xsl:template match="html:textpolish">
  \textpolish{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:portuges">
  \begin{portuges}
  <xsl:apply-templates />
  \end{portuges}
</xsl:template>

<xsl:template match="html:textportuges">
  \textportuges{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:portuguese">
  \begin{portuguese}
  <xsl:apply-templates />
  \end{portuguese}
</xsl:template>

<xsl:template match="html:textportuguese">
  \textportuguese{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:romanian">
  \begin{romanian}
  <xsl:apply-templates />
  \end{romanian}
</xsl:template>

<xsl:template match="html:textromanian">
  \textromanian{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:russian">
  \begin{russian}
  <xsl:apply-templates />
  \end{russian}
</xsl:template>

<xsl:template match="html:textrussian">
  \textrussian{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:scottish">
  \begin{scottish}
  <xsl:apply-templates />
  \end{scottish}
</xsl:template>

<xsl:template match="html:textscottish">
  \textscottish{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:serbian">
  \begin{serbian}
  <xsl:apply-templates />
  \end{serbian}
</xsl:template>

<xsl:template match="html:textserbian">
  \textserbian{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:slovak">
  \begin{slovak}
  <xsl:apply-templates />
  \end{slovak}
</xsl:template>

<xsl:template match="html:textslovak">
  \textslovak{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:slovene">
  \begin{slovene}
  <xsl:apply-templates />
  \end{slovene}
</xsl:template>

<xsl:template match="html:textslovene">
  \textslovene{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:spanish">
  \begin{spanish}
  <xsl:apply-templates />
  \end{spanish}
</xsl:template>

<xsl:template match="html:textspanish">
  \textspanish{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:swedish">
  \begin{swedish}
  <xsl:apply-templates />
  \end{swedish}
</xsl:template>

<xsl:template match="html:textswedish">
  \textswedish{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:syriac">
  \begin{syriac}
  <xsl:apply-templates />
  \end{syriac}
</xsl:template>

<xsl:template match="html:textsyriac">
  \textsyriac{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:turkish">
  \begin{turkish}
  <xsl:apply-templates />
  \end{turkish}
</xsl:template>

<xsl:template match="html:textturkish">
  \textturkish{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:ukrainian">
  \begin{ukrainian}
  <xsl:apply-templates />
  \end{ukrainian}
</xsl:template>

<xsl:template match="html:textukrainian">
  \textukrainian{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:uppersorbian">
  \begin{uppersorbian}
  <xsl:apply-templates />
  \end{uppersorbian}
</xsl:template>

<xsl:template match="html:textuppersorbian">
  \textuppersorbian{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:urdu">
  \begin{urdu}
  <xsl:apply-templates />
  \end{urdu}
</xsl:template>

<xsl:template match="html:texturdu">
  \texturdu{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:welsh">
  \begin{welsh}
  <xsl:apply-templates />
  \end{welsh}
</xsl:template>

<xsl:template match="html:textwelsh">
  \textwelsh{<xsl:apply-templates />}
</xsl:template>

</xsl:stylesheet>
