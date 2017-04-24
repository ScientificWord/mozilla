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

<xsl:template match="html:english">
  \begin{english}
  <xsl:apply-templates />
  \end{english}
</xsl:template>

<xsl:template match="html:textenglish">
  \textenglish{<xsl:apply-templates />}
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

<xsl:template match="html:amharic">
  \begin{amharic}
  <xsl:apply-templates />
  \end{amharic}
</xsl:template>

<xsl:template match="html:textamharic">
  \textamharic{<xsl:apply-templates />}
</xsl:template>


<xsl:template match="html:Arabic">
  \begin{Arabic}
  <xsl:apply-templates />
  \end{Arabic}
</xsl:template>

<xsl:template match="html:textarabic">
  \textarabic{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:armenian">
  \begin{armenian}
  <xsl:apply-templates />
  \end{armenian}
</xsl:template>

<xsl:template match="html:textarmenian">
  \textarmenian{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:asturian">
  \begin{asturian}
  <xsl:apply-templates />
  \end{asturian}
</xsl:template>

<xsl:template match="html:textasturian">
  \textasturian{<xsl:apply-templates />}
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

<xsl:template match="html:bengali">
  \begin{bengali}
  <xsl:apply-templates />
  \end{bengali}
</xsl:template>

<xsl:template match="html:textbengali">
  \textbengali{<xsl:apply-templates />}
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


<xsl:template match="html:english">
  \begin{english}
  <xsl:apply-templates />
  \end{english}
</xsl:template>

<xsl:template match="html:textenglish">
  \textenglish{<xsl:apply-templates />}
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

<xsl:template match="html:friulan">
  \begin{friulan}
  <xsl:apply-templates />
  \end{friulan}
</xsl:template>

<xsl:template match="html:textfriulan">
  \textfriulan{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:galician">
  \begin{galician}
  <xsl:apply-templates />
  \end{galician}
</xsl:template>

<xsl:template match="html:textgalician">
  \textgalician{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:greek">
  \begin{greek}
  <xsl:apply-templates />
  \end{greek}
</xsl:template>

<xsl:template match="html:textgreek">
  \textgreek{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:hebrew">
  \begin{hebrew}
  <xsl:apply-templates />
  \end{hebrew}
</xsl:template>

<xsl:template match="html:texthebrew">
  \texthebrew{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:hindi">
  \begin{hindi}
  <xsl:apply-templates />
  \end{hindi}
</xsl:template>

<xsl:template match="html:texthindi">
  \texthindi{<xsl:apply-templates />}
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

<xsl:template match="html:kannada">
  \begin{kannada}
  <xsl:apply-templates />
  \end{kannada}
</xsl:template>

<xsl:template match="html:textkannada">
  \textkannada{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:khmer">
  \begin{khmer}
  <xsl:apply-templates />
  \end{khmer}
</xsl:template>

<xsl:template match="html:textkhmer">
  \textkhmer{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:korean">
  \begin{korean}
  <xsl:apply-templates />
  \end{korean}
</xsl:template>

<xsl:template match="html:textkorean">
  \textkorean{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:lao">
  \begin{lao}
  <xsl:apply-templates />
  \end{lao}
</xsl:template>

<xsl:template match="html:textlao">
  \textlao{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:latin">
  \begin{latin}
  <xsl:apply-templates />
  \end{latin}
</xsl:template>

<xsl:template match="html:textlatin">
  \textlatin{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:latvian">
  \begin{latvian}
  <xsl:apply-templates />
  \end{latvian}
</xsl:template>

<xsl:template match="html:textlatvian">
  \textlatvian{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:lithuanian">
  \begin{lithuanian}
  <xsl:apply-templates />
  \end{lithuanian}
</xsl:template>

<xsl:template match="html:textlithuanian">
  \textlithuanian{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:lsorbian">
  \begin{lsorbian}
  <xsl:apply-templates />
  \end{lsorbian}
</xsl:template>

<xsl:template match="html:textlsorbian">
  \textlsorbian{<xsl:apply-templates />}
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

<xsl:template match="html:malayalam">
  \begin{malayalam}
  <xsl:apply-templates />
  \end{malayalam}
</xsl:template>

<xsl:template match="html:textmalayalam">
  \textmalayalam{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:marathi">
  \begin{marathi}
  <xsl:apply-templates />
  \end{marathi}
</xsl:template>

<xsl:template match="html:textmarathi">
  \textmarathi{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:nko">
  \begin{nko}
  <xsl:apply-templates />
  \end{nko}
</xsl:template>

<xsl:template match="html:textnko">
  \textnko{<xsl:apply-templates />}
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

<xsl:template match="html:occitan">
  \begin{occitan}
  <xsl:apply-templates />
  \end{occitan}
</xsl:template>

<xsl:template match="html:textoccitan">
  \textoccitan{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:piedmontese">
  \begin{piedmontese}
  <xsl:apply-templates />
  \end{piedmontese}
</xsl:template>

<xsl:template match="html:textpiedmontese">
  \textpiedmontese{<xsl:apply-templates />}
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

<xsl:template match="html:romansh">
  \begin{romansh}
  <xsl:apply-templates />
  \end{romansh}
</xsl:template>

<xsl:template match="html:textromansh">
  \textromansh{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:russian">
  \begin{russian}
  <xsl:apply-templates />
  \end{russian}
</xsl:template>

<xsl:template match="html:textrussian">
  \textrussian{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:sanskrit">
  \begin{sanskrit}
  <xsl:apply-templates />
  \end{sanskrit}
</xsl:template>

<xsl:template match="html:textsanskrit">
  \textsanskrit{<xsl:apply-templates />}
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

<xsl:template match="html:slovenian">
  \begin{slovenian}
  <xsl:apply-templates />
  \end{slovenian}
</xsl:template>

<xsl:template match="html:textslovenian">
  \textslovenian{<xsl:apply-templates />}
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

<xsl:template match="html:telugu">
  \begin{telugu}
  <xsl:apply-templates />
  \end{telugu}
</xsl:template>

<xsl:template match="html:texttelugu">
  \texttelugu{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:thai">
  \begin{thai}
  <xsl:apply-templates />
  \end{thai}
</xsl:template>

<xsl:template match="html:textthai">
  \textthai{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:tibetan">
  \begin{tibetan}
  <xsl:apply-templates />
  \end{tibetan}
</xsl:template>

<xsl:template match="html:texttibetan">
  \texttibetan{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:turkish">
  \begin{turkish}
  <xsl:apply-templates />
  \end{turkish}
</xsl:template>

<xsl:template match="html:textturkish">
  \textturkish{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:turkmen">
  \begin{turkmen}
  <xsl:apply-templates />
  \end{turkmen}
</xsl:template>

<xsl:template match="html:textturkmen">
  \textturkmen{<xsl:apply-templates />}
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

<xsl:template match="html:usorbian">
  \begin{usorbian}
  <xsl:apply-templates />
  \end{usorbian}
</xsl:template>

<xsl:template match="html:textusorbian">
  \textusorbian{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:urdu">
  \begin{urdu}
  <xsl:apply-templates />
  \end{urdu}
</xsl:template>

<xsl:template match="html:texturdu">
  \texturdu{<xsl:apply-templates />}
</xsl:template>

<xsl:template match="html:vietnamese">
  \begin{vietnamese}
  <xsl:apply-templates />
  \end{vietnamese}
</xsl:template>

<xsl:template match="html:textvietnamese">
  \textvietnamese{<xsl:apply-templates />}
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
