;;; leim-list.el -- list of LEIM (Library of Emacs Input Method) -*-coding: iso-2022-7bit;-*-
;;
;; This file contains a list of LEIM (Library of Emacs Input Method)
;; methods in the same directory as this file.  Loading this file
;; registers all the input methods in Emacs.
;;
;; Each entry has the form:
;;   (register-input-method
;;    INPUT-METHOD LANGUAGE-NAME ACTIVATE-FUNC
;;    TITLE DESCRIPTION
;;    ARG ...)
;; See the function `register-input-method' for the meanings of the arguments.
;;
;; If this directory is included in load-path, Emacs automatically
;; loads this file at startup time.

(register-input-method
 "chinese-zozy" "Chinese-BIG5" 'quail-use-package
 "$(0I\0D(B" "$(0&d'GTT&,!JI\@c0D5x!K(B"
 "quail/ZOZY")
(register-input-method
 "chinese-ziranma" "Chinese-CNS" 'quail-use-package
 "$AWTH;(B" "$A::WVJdHk!K!>WTH;!?!K(B"
 "quail/ZIRANMA")
(register-input-method
 "welsh" "Welsh" 'quail-use-package
 "$,1!4(B" "Welsh postfix input method, using Unicode"
 "quail/welsh")
(register-input-method
 "vietnamese-telex" "Vietnamese" 'quail-use-package
 "VT" "Vietnamese telex input method"
 "quail/vntelex")
(register-input-method
 "vietnamese-viqr" "Vietnamese" 'quail-use-package
 "VQ" "Vietnamese input method with VIQR mnemonic system"
 "quail/viqr")
(register-input-method
 "chinese-cns-tsangchi" "Chinese-CNS" 'quail-use-package
 "$(GT?(BC" "$(GDcEFrSD+!JT?on!K(BCNS"
 "quail/tsang-cns")
(register-input-method
 "chinese-b5-tsangchi" "Chinese-BIG5" 'quail-use-package
 "$(06A(BB" "$(0&d'GTT&,!J6AQo!K(BBIG5"
 "quail/tsang-b5")
(register-input-method
 "chinese-tonepy" "Chinese-GB" 'quail-use-package
 "$A5wF4(B" "$A::WVJdHk!K4x5wF4Rt!K(B"
 "quail/TONEPY")
(register-input-method
 "tibetan-wylie" "Tibetan" 'quail-use-package
 "TIBw" "Tibetan character input by Extended Wylie key assignment."
 "quail/tibetan")
(register-input-method
 "tibetan-tibkey" "Tibetan" 'quail-use-package
 "TIBt" "Tibetan character input by TibKey key assignment."
 "quail/tibetan")
(register-input-method
 "thai-kesmanee" "Thai" 'quail-use-package
 ",T!!(B>" "Thai Kesmanee input method with TIS620 keyboard layout"
 "quail/thai")
(register-input-method
 "thai-pattachote" "Thai" 'quail-use-package
 ",T!;(B>" "Thai Pattachote input method with TIS620 keyboard layout"
 "quail/thai")
(register-input-method
 "korean-symbol" "Korean" 'quail-use-package
 "$(C=I9z(B" "$(CGQ1[=I9z@T7BG%(B:"
 "quail/symbol-ksc")
(register-input-method
 "chinese-sw" "Chinese-GB" 'quail-use-package
 "$AJWN2(B" "$A::WVJdHk!KJWN2!K(B"
 "quail/SW")
(register-input-method
 "slovak" "Slovak" 'quail-use-package
 "SK" "Standard Slovak keyboard."
 "quail/slovak")
(register-input-method
 "slovak-prog-1" "Slovak" 'quail-use-package
 "SK" "Slovak (non-standard) keyboard for programmers #1."
 "quail/slovak")
(register-input-method
 "slovak-prog-2" "Slovak" 'quail-use-package
 "SK" "Slovak (non-standard) keyboard for programmers #2."
 "quail/slovak")
(register-input-method
 "slovak-prog-3" "Slovak" 'quail-use-package
 "SK" "Slovak (non-standard) keyboard for programmers #3."
 "quail/slovak")
(register-input-method
 "chinese-sisheng" "Chinese" 'quail-use-package
 "$,1":(B" "S,Al(Bsh$,1 3(Bng input method for p$,1 K(Bny$,1 K(Bn transliteration of Chinese."
 "quail/sisheng")
(register-input-method
 "sgml" "UTF-8" 'quail-use-package
 "&" "Unicode characters input method using SGML entities."
 "quail/sgml-input")
(register-input-method
 "rfc1345" "UTF-8" 'quail-use-package
 "m" "Unicode characters input method using RFC1345 mnemonics (non-ASCII only)."
 "quail/rfc1345")
(register-input-method
 "chinese-cns-quick" "Chinese-CNS" 'quail-use-package
 "$(Gv|(BC" "$(GDcEFrSD+!Jv|Mx!K(BCNS"
 "quail/quick-cns")
(register-input-method
 "chinese-b5-quick" "Chinese-BIG5" 'quail-use-package
 "$(0X|(BB" "$(0&d'GTT&,!JX|/y!K(BBIG5"
 "quail/quick-b5")
(register-input-method
 "chinese-qj" "Chinese-GB" 'quail-use-package
 "$AH+(BG" "$A::WVJdHk!KH+=G!K(B"
 "quail/QJ")
(register-input-method
 "chinese-qj-b5" "Chinese-BIG5" 'quail-use-package
 "$(0)A(BB" "$(0KH)tTT&,(B::$(0)A-F(B::"
 "quail/QJ-b5")
(register-input-method
 "chinese-py-punct-b5" "Chinese-BIG5" 'quail-use-package
 "$(03<>K(B" "$(0&d'GTT&,!J3<5x!K(B and `v' for $(0O:X5>KHATT&,(B"
 "quail/pypunct-b5")
(register-input-method
 "chinese-py" "Chinese-CNS" 'quail-use-package
 "$AF4(BG" "$A::WVJdHk!KF4Rt!K(B"
 "quail/PY")
(register-input-method
 "chinese-py-punct" "Chinese-GB" 'quail-use-package
 "$AF47{(B" "$A::WVJdHk(B $AF4Rt7=08(B and `v' for $A1j5c7{:EJdHk(B"
 "quail/py-punct")
(register-input-method
 "chinese-tonepy-punct" "Chinese-GB" 'quail-use-package
 "$AF47{(B" "$A::WVJdHk(B $A4x5wF4Rt7=08(B and `v' for $A1j5c7{:EJdHk(B"
 "quail/py-punct")
(register-input-method
 "chinese-py-b5" "Chinese-BIG5" 'quail-use-package
 "$(03<(BB" "$(0KH)tTT&,(B::$(03<5x(B::"
 "quail/PY-b5")
(register-input-method
 "chinese-punct" "Chinese-GB" 'quail-use-package
 "$A1j(BG" "$A::WVJdHk!K1j5c7{:E!K(B"
 "quail/Punct")
(register-input-method
 "chinese-punct-b5" "Chinese-BIG5" 'quail-use-package
 "$(0O:(BB" "$(0&d'GTT&,!JO:X5>KHA!K(B"
 "quail/Punct-b5")
(register-input-method
 "lao-lrt" "Lao" 'quail-use-package
 "(1E(BR" "Lao input method using LRT (Lao Roman Transcription)."
 "quail/lrt")
(register-input-method
 "latin-1-prefix" "Latin-1" 'quail-use-package
 "1>" "Latin-1 characters input method with prefix modifiers"
 "quail/latin-pre")
(register-input-method
 "catalan-prefix" "Latin-1" 'quail-use-package
 "CA>" "Catalan and Spanish input method with prefix modifiers"
 "quail/latin-pre")
(register-input-method
 "esperanto-prefix" "Latin-3" 'quail-use-package
 "EO>" "Esperanto input method with prefix modifiers"
 "quail/latin-pre")
(register-input-method
 "french-prefix" "French" 'quail-use-package
 "FR>" "French (Fran,Ag(Bais) input method with prefix modifiers"
 "quail/latin-pre")
(register-input-method
 "romanian-prefix" "Romanian" 'quail-use-package
 "RO>" "Romanian (rom,Bb(Bne,B:(Bte) input method with prefix modifiers"
 "quail/latin-pre")
(register-input-method
 "romanian-alt-prefix" "Romanian" 'quail-use-package
 "RO>" "Alternative Romanian (rom,Bb(Bne,B:(Bte) input method with prefix modifiers"
 "quail/latin-pre")
(register-input-method
 "german-prefix" "German" 'quail-use-package
 "DE>" "German (Deutsch) input method with prefix modifiers"
 "quail/latin-pre")
(register-input-method
 "irish-prefix" "Latin-1" 'quail-use-package
 "GA>" "Irish input method with prefix modifiers"
 "quail/latin-pre")
(register-input-method
 "portuguese-prefix" "Latin-1" 'quail-use-package
 "PT>" "Portuguese input method with prefix modifiers"
 "quail/latin-pre")
(register-input-method
 "spanish-prefix" "Spanish" 'quail-use-package
 "ES>" "Spanish (Espa,Aq(Bol) input method with prefix modifiers"
 "quail/latin-pre")
(register-input-method
 "latin-2-prefix" "Latin-2" 'quail-use-package
 "2>" "Latin-2 characters input method with prefix modifiers"
 "quail/latin-pre")
(register-input-method
 "latin-3-prefix" "Latin-3" 'quail-use-package
 "3>" "Latin-3 characters input method with prefix modifiers"
 "quail/latin-pre")
(register-input-method
 "polish-slash" "Polish" 'quail-use-package
 "PL>" "Polish diacritics and slash character are input as `/[acelnosxzACELNOSXZ/]'."
 "quail/latin-pre")
(register-input-method
 "latin-9-prefix" "Latin-9" 'quail-use-package
 "0>" "Latin-9 characters input method with prefix modifiers"
 "quail/latin-pre")
(register-input-method
 "latin-8-prefix" "Latin-8" 'quail-use-package
 "8>" "Latin-8 characters input method with prefix modifiers"
 "quail/latin-pre")
(register-input-method
 "latin-prefix" "Latin" 'quail-use-package
 "L>" "Latin characters input method with prefix modifiers."
 "quail/latin-pre")
(register-input-method
 "latin-1-postfix" "Latin-1" 'quail-use-package
 "1<" "Latin-1 character input method with postfix modifiers"
 "quail/latin-post")
(register-input-method
 "latin-2-postfix" "Latin-2" 'quail-use-package
 "2<" "Latin-2 character input method with postfix modifiers"
 "quail/latin-post")
(register-input-method
 "latin-3-postfix" "Latin-3" 'quail-use-package
 "3<" "Latin-3 character input method with postfix modifiers"
 "quail/latin-post")
(register-input-method
 "latin-4-postfix" "Latin-4" 'quail-use-package
 "4<" "Latin-4 characters input method with postfix modifiers"
 "quail/latin-post")
(register-input-method
 "latin-5-postfix" "Latin-5" 'quail-use-package
 "5<" "Latin-5 characters input method with postfix modifiers"
 "quail/latin-post")
(register-input-method
 "danish-postfix" "Latin-1" 'quail-use-package
 "DA<" "Danish input method (rule: AE -> ,AF(B, OE -> ,AX(B, AA -> ,AE(B, E' -> ,AI(B)"
 "quail/latin-post")
(register-input-method
 "esperanto-postfix" "Latin-3" 'quail-use-package
 "EO<" "Esperanto input method with postfix modifiers"
 "quail/latin-post")
(register-input-method
 "finnish-postfix" "Latin-1" 'quail-use-package
 "FI<" "Finnish (Suomi) input method"
 "quail/latin-post")
(register-input-method
 "french-postfix" "French" 'quail-use-package
 "FR<" "French (Fran,Ag(Bais) input method with postfix modifiers"
 "quail/latin-post")
(register-input-method
 "german-postfix" "German" 'quail-use-package
 "DE<" "German (Deutsch) input method"
 "quail/latin-post")
(register-input-method
 "icelandic-postfix" "Latin-1" 'quail-use-package
 "IS<" "Icelandic (,AM(Bslenska) input method with postfix modifiers"
 "quail/latin-post")
(register-input-method
 "italian-postfix" "Latin-1" 'quail-use-package
 "IT<" "Italian (Italiano) input method with postfix modifiers"
 "quail/latin-post")
(register-input-method
 "norwegian-postfix" "Latin-1" 'quail-use-package
 "NO<" "Norwegian (Norsk) input method (rule: AE->,AF(B   OE->,AX(B   AA->,AE(B   E'->,AI(B)"
 "quail/latin-post")
(register-input-method
 "scandinavian-postfix" "Latin-1" 'quail-use-package
 "SC<" "Scandinavian input method with postfix modifiers"
 "quail/latin-post")
(register-input-method
 "spanish-postfix" "Spanish" 'quail-use-package
 "ES<" "Spanish (Espa,Aq(Bol) input method with postfix modifiers"
 "quail/latin-post")
(register-input-method
 "swedish-postfix" "Latin-1" 'quail-use-package
 "SV<" "Swedish (Svenska) input method (rule: AA -> ,AE(B   AE -> ,AD(B   OE -> ,AV(B   E' -> ,AI(B)"
 "quail/latin-post")
(register-input-method
 "turkish-latin-3-postfix" "Turkish" 'quail-use-package
 "TR3<" "Turkish (T,C|(Brk,Cg(Be) input method with postfix modifiers."
 "quail/latin-post")
(register-input-method
 "turkish-postfix" "Turkish" 'quail-use-package
 "TR<" "Turkish (T,M|(Brk,Mg(Be) input method with postfix modifiers."
 "quail/latin-post")
(register-input-method
 "british" "Latin-1" 'quail-use-package
 ",A#(B@" "British English input method with Latin-1 character ,A#(B (# -> ,A#(B)"
 "quail/latin-post")
(register-input-method
 "french-keyboard" "French" 'quail-use-package
 "FR@" "French (Fran,Ag(Bais) input method simulating some French keyboard"
 "quail/latin-post")
(register-input-method
 "french-azerty" "French" 'quail-use-package
 "AZ@" "French (Fran,Ag(Bais) input method simulating Azerty keyboard"
 "quail/latin-post")
(register-input-method
 "icelandic-keyboard" "Latin-1" 'quail-use-package
 "IS@" "Icelandic (,AM(Bslenska) input method simulating some Icelandic keyboard"
 "quail/latin-post")
(register-input-method
 "danish-keyboard" "Latin-1" 'quail-use-package
 "DA@" "Danish input method simulating SUN Danish keyboard"
 "quail/latin-post")
(register-input-method
 "norwegian-keyboard" "Latin-1" 'quail-use-package
 "NO@" "Norwegian (Norsk) input method simulating SUN Norwegian keyboard"
 "quail/latin-post")
(register-input-method
 "swedish-keyboard" "Latin-1" 'quail-use-package
 "SV@" "Swedish (Svenska) input method simulating SUN Swedish/Finnish keyboard"
 "quail/latin-post")
(register-input-method
 "finnish-keyboard" "Latin-1" 'quail-use-package
 "FI@" "Finnish input method simulating SUN Finnish/Swedish keyboard"
 "quail/latin-post")
(register-input-method
 "german" "German" 'quail-use-package
 "DE@" "German (Deutsch) input method simulating SUN German keyboard"
 "quail/latin-post")
(register-input-method
 "italian-keyboard" "Latin-1" 'quail-use-package
 "IT@" "Italian (Italiano) input method simulating SUN Italian keyboard"
 "quail/latin-post")
(register-input-method
 "spanish-keyboard" "Spanish" 'quail-use-package
 "ES@" "Spanish (Espa,Aq(Bol) input method simulating SUN Spanish keyboard"
 "quail/latin-post")
(register-input-method
 "english-dvorak" "English" 'quail-use-package
 "DV@" "English (ASCII) input method simulating Dvorak keyboard"
 "quail/latin-post")
(register-input-method
 "latin-postfix" "Latin" 'quail-use-package
 "L<" "Latin character input method with postfix modifiers."
 "quail/latin-post")
(register-input-method
 "slovenian" "Slovenian" 'quail-use-package
 "Sl" "Slovenian postfix input."
 "quail/latin-post")
(register-input-method
 "TeX" "UTF-8" 'quail-use-package
 "\\" "LaTeX-like input method for many characters."
 "quail/latin-ltx")
(register-input-method
 "latin-1-alt-postfix" "Latin-1" 'quail-use-package
 "1<" "Latin-1 character input method with postfix modifiers"
 "quail/latin-alt")
(register-input-method
 "latin-2-alt-postfix" "Latin-2" 'quail-use-package
 "2<" "Latin-2 character input method with postfix modifiers"
 "quail/latin-alt")
(register-input-method
 "latin-3-alt-postfix" "Latin-3" 'quail-use-package
 "3<" "Latin-3 character input method with postfix modifiers"
 "quail/latin-alt")
(register-input-method
 "latin-4-alt-postfix" "Latin-4" 'quail-use-package
 "4<" "Latin-4 characters input method with postfix modifiers"
 "quail/latin-alt")
(register-input-method
 "latin-5-alt-postfix" "Latin-5" 'quail-use-package
 "5<" "Latin-5 characters input method with postfix modifiers"
 "quail/latin-alt")
(register-input-method
 "french-alt-postfix" "French" 'quail-use-package
 "FR<" "French (Fran,Ag(Bais) input method with postfix modifiers"
 "quail/latin-alt")
(register-input-method
 "italian-alt-postfix" "Latin-1" 'quail-use-package
 "IT<" "Italian (Italiano) input method with postfix modifiers"
 "quail/latin-alt")
(register-input-method
 "turkish-latin-3-alt-postfix" "Turkish" 'quail-use-package
 "TR3<<" "Turkish (T,A|(Brk,Ag(Be) input method with postfix modifiers."
 "quail/latin-alt")
(register-input-method
 "turkish-alt-postfix" "Turkish" 'quail-use-package
 "TR,A+(B" "Turkish (T,A|(Brk,Ag(Be) input method with postfix modifiers."
 "quail/latin-alt")
(register-input-method
 "dutch" "Dutch" 'quail-use-package
 "NL" "Dutch character mixfix input method."
 "quail/latin-alt")
(register-input-method
 "lithuanian-numeric" "Lithuanian" 'quail-use-package
 "LtN" "Lithuanian numeric input method."
 "quail/latin-alt")
(register-input-method
 "lithuanian-keyboard" "Lithuanian" 'quail-use-package
 "Lt" "Lithuanian standard keyboard input method."
 "quail/latin-alt")
(register-input-method
 "latvian-keyboard" "Latvian" 'quail-use-package
 "Lv" "Latvian standard keyboard input method."
 "quail/latin-alt")
(register-input-method
 "latin-alt-postfix" "Latin" 'quail-use-package
 "L<" "Latin character input method with postfix modifiers."
 "quail/latin-alt")
(register-input-method
 "lao" "Lao" 'quail-use-package
 "(1E(B" "Lao input method simulating Lao keyboard layout based on Thai TIS620"
 "quail/lao")
(register-input-method
 "japanese" "Japanese" 'quail-use-package
 "A$B$"(B" "Japanese input method by Roman transliteration and Kana-Kanji conversion."
 "quail/japanese")
(register-input-method
 "japanese-ascii" "Japanese" 'quail-use-package
 "Aa" "Temporary ASCII input mode used within the input method `japanese'."
 "quail/japanese")
(register-input-method
 "japanese-zenkaku" "Japanese" 'quail-use-package
 "$B#A(B" "Japanese zenkaku alpha numeric character input method."
 "quail/japanese")
(register-input-method
 "japanese-hankaku-kana" "Japanese" 'quail-use-package
 "(I1(B" "Japanese hankaku katakana input method by Roman transliteration."
 "quail/japanese")
(register-input-method
 "japanese-hiragana" "Japanese" 'quail-use-package
 "$B$"(B" "Japanese hiragana input method by Roman transliteration."
 "quail/japanese")
(register-input-method
 "japanese-katakana" "Japanese" 'quail-use-package
 "$B%"(B" "Japanese katakana input method by Roman transliteration."
 "quail/japanese")
(register-input-method
 "ipa" "IPA" 'quail-use-package
 "IPA" "International Phonetic Alphabet for English, French, German and Italian"
 "quail/ipa")
(register-input-method
 "devanagari-itrans" "Devanagari" 'quail-use-package
 "DevIT" "Devanagari ITRANS"
 "quail/indian")
(register-input-method
 "devanagari-kyoto-harvard" "Devanagari" 'quail-use-package
 "DevKH" "Devanagari Kyoto-Harvard"
 "quail/indian")
(register-input-method
 "devanagari-aiba" "Devanagari" 'quail-use-package
 "DevAB" "Devanagari Aiba"
 "quail/indian")
(register-input-method
 "punjabi-itrans" "Punjabi" 'quail-use-package
 "PnjIT" "Punjabi ITRANS"
 "quail/indian")
(register-input-method
 "gujarati-itrans" "Gujarati" 'quail-use-package
 "GjrIT" "Gujarati ITRANS"
 "quail/indian")
(register-input-method
 "oriya-itrans" "Oriya" 'quail-use-package
 "OriIT" "Oriya ITRANS"
 "quail/indian")
(register-input-method
 "bengali-itrans" "Bengali" 'quail-use-package
 "BngIT" "Bengali ITRANS"
 "quail/indian")
(register-input-method
 "assamese-itrans" "Assamese" 'quail-use-package
 "AsmIT" "Assamese ITRANS"
 "quail/indian")
(register-input-method
 "telugu-itrans" "Telugu" 'quail-use-package
 "TlgIT" "Telugu ITRANS"
 "quail/indian")
(register-input-method
 "kannada-itrans" "Kannada" 'quail-use-package
 "KndIT" "Kannada ITRANS"
 "quail/indian")
(register-input-method
 "malayalam-itrans" "Malayalam" 'quail-use-package
 "MlmIT" "Malayalam ITRANS"
 "quail/indian")
(register-input-method
 "tamil-itrans" "Tamil" 'quail-use-package
 "TmlIT" "Tamil ITRANS"
 "quail/indian")
(register-input-method
 "devanagari-inscript" "Devanagari" 'quail-use-package
 "DevIS" "Devanagari keyboard Inscript"
 "quail/indian")
(register-input-method
 "punjabi-inscript" "Punjabi" 'quail-use-package
 "PnjIS" "Punjabi keyboard Inscript"
 "quail/indian")
(register-input-method
 "gujarati-inscript" "Gujarati" 'quail-use-package
 "GjrIS" "Gujarati keyboard Inscript"
 "quail/indian")
(register-input-method
 "oriya-inscript" "Oriya" 'quail-use-package
 "OriIS" "Oriya keyboard Inscript"
 "quail/indian")
(register-input-method
 "bengali-inscript" "Bengali" 'quail-use-package
 "BngIS" "Bengali keyboard Inscript"
 "quail/indian")
(register-input-method
 "assamese-inscript" "Assamese" 'quail-use-package
 "AsmIS" "Assamese keyboard Inscript"
 "quail/indian")
(register-input-method
 "telugu-inscript" "Telugu" 'quail-use-package
 "TlgIS" "Telugu keyboard Inscript"
 "quail/indian")
(register-input-method
 "kannada-inscript" "Kannada" 'quail-use-package
 "KndIS" "Kannada keyboard Inscript"
 "quail/indian")
(register-input-method
 "malayalam-inscript" "Malayalam" 'quail-use-package
 "MlmIS" "Malayalam keyboard Inscript"
 "quail/indian")
(register-input-method
 "tamil-inscript" "Tamil" 'quail-use-package
 "TmlIS" "Tamil keyboard Inscript"
 "quail/indian")
(register-input-method
 "hebrew" "Hebrew" 'quail-use-package
 ",Hr(B" "Hebrew (ISO 8859-8) input method."
 "quail/hebrew")
(register-input-method
 "korean-hanja3" "Korean" 'quail-use-package
 "$(CyS(B3" "3$(C9z=D(BKSC$(CySm.(B: $(Cz1SWGO4B(B $(CySm.@G(B $(Cj$@;(B $(CGQ1[(B3$(C9zcR@87N(B $(C{<usGO?)(B $(C`TwI(B"
 "quail/hanja3")
(register-input-method
 "korean-hanja" "Korean" 'quail-use-package
 "$(CyS(B2" "2$(C9z=D(BKSC$(CySm.(B: $(Cz1SWGO4B(B $(CySm.@G(B $(Cj$@;(B $(CGQ1[(B2$(C9zcR@87N(B $(C{<usGO?)(B $(C`TwI(B"
 "quail/hanja")
(register-input-method
 "korean-hanja-jis" "Korean" 'quail-use-package
 "$B4A(B2" "2$(C9z=D(BJIS$B4A;z(B: $B3:aD$(CGO4B(B $B4A;z$(C@G(B $B1$$(C@;(B $(CGQ1[(B2$(C9z$B<0$(C@87N(B $B8F=P$(CGO?)(B $BA*Z$(B"
 "quail/hanja-jis")
(register-input-method
 "korean-hangul3" "Korean" 'quail-use-package
 "$(CGQ(B3" "$(CGQ1[(B 3$(C9z=D(B: Hangul input method"
 "quail/hangul3")
(register-input-method
 "korean-hangul" "Korean" 'quail-use-package
 "$(CGQ(B2" "$(CGQ1[(B 2$(C9z=D(B: Hangul input method with Hangul keyboard layout (KSC5601)"
 "quail/hangul")
(register-input-method
 "greek-jis" "Greek" 'quail-use-package
 "$B&8(B" "$B&%&K&K&G&M&I&J&A(B: Greek keyboard layout (JIS X0208.1983)"
 "quail/greek")
(register-input-method
 "greek-mizuochi" "Greek" 'quail-use-package
 "CG" "The Mizuochi input method for Classical Greek using mule-unicode-0100-24ff."
 "quail/greek")
(register-input-method
 "greek-babel" "Greek" 'quail-use-package
 "BG" "The TeX Babel input method for Classical Greek using mule-unicode-0100-24ff."
 "quail/greek")
(register-input-method
 "greek-ibycus4" "Greek" 'quail-use-package
 "IB" "The Ibycus4 input method for Classical Greek using mule-unicode-0100-24ff."
 "quail/greek")
(register-input-method
 "greek" "Greek" 'quail-use-package
 ",FY(B" ",FEkkgmij\(B: Greek keyboard layout (ISO 8859-7)"
 "quail/greek")
(register-input-method
 "greek-postfix" "GreekPost" 'quail-use-package
 ",FX(B" ",FEkkgmij\(B: Greek keyboard layout with postfix accents (ISO 8859-7)"
 "quail/greek")
(register-input-method
 "georgian" "Georgian" 'quail-use-package
 "$,1J2(B" "A common Georgian transliteration (using Unicode)"
 "quail/georgian")
(register-input-method
 "chinese-etzy" "Chinese-BIG5" 'quail-use-package
 "$(06/0D(B" "$(0&d'GTT&,!J6/'30D5x!K(B"
 "quail/ETZY")
(register-input-method
 "ethiopic" "Ethiopic" 'quail-use-package
 (quote ("$(3$Q#U!.(B " (ethio-prefer-ascii-space "_" "$(3$h(B") (ethio-prefer-ascii-punctuation "." "$(3$i(B"))) "  Quail package for Ethiopic (Tigrigna and Amharic)"
 "quail/ethiopic")
(register-input-method
 "chinese-ecdict" "Chinese-BIG5" 'quail-use-package
 "$(05CKH(B" "$(0&d'GTT&,!J5CKH[0.)!K(B"
 "quail/ECDICT")
(register-input-method
 "czech" "Czech" 'quail-use-package
 "CZ" "\"Standard\" Czech keyboard in the Windoze NT 105 keys version."
 "quail/czech")
(register-input-method
 "czech-qwerty" "Czech" 'quail-use-package
 "CZ" "\"Standard\" Czech keyboard in the Windoze NT 105 keys version, QWERTY layout."
 "quail/czech")
(register-input-method
 "czech-prog-1" "Czech" 'quail-use-package
 "CZ" "Czech (non-standard) keyboard for programmers #1."
 "quail/czech")
(register-input-method
 "czech-prog-2" "Czech" 'quail-use-package
 "CZ" "Czech (non-standard) keyboard for programmers #2."
 "quail/czech")
(register-input-method
 "czech-prog-3" "Czech" 'quail-use-package
 "CZ" "Czech (non-standard) keyboard for programmers compatible with the default"
 "quail/czech")
(register-input-method
 "russian-typewriter" "Russian" 'quail-use-package
 ",L69(B" ",L9FC:5=(B Russian typewriter layout (ISO 8859-5 encoding)."
 "quail/cyrillic")
(register-input-method
 "cyrillic-jcuken" "Russian" 'quail-use-package
 ",L69(B" ",L9FC:5=(B Russian typewriter layout (ISO 8859-5 encoding)."
 "quail/cyrillic")
(register-input-method
 "russian-computer" "Russian" 'quail-use-package
 "RU" ",L9FC:5=(B Russian computer layout"
 "quail/cyrillic")
(register-input-method
 "cyrillic-macedonian" "Cyrillic" 'quail-use-package
 ",L6(BM" ",L)*5@B7(B-,L#,(B keyboard layout based on JUS.I.K1.004 (ISO 8859-5 encoding)"
 "quail/cyrillic")
(register-input-method
 "cyrillic-serbian" "Cyrillic" 'quail-use-package
 ",L6(BS" ",L)*5@B7(B-,L"+(B keyboard layout based on JUS.I.K1.005 (ISO 8859-5 encoding)"
 "quail/cyrillic")
(register-input-method
 "cyrillic-ukrainian" "Ukrainian" 'quail-use-package
 ",L6(BU" ",L$'5@B7(B-,L&.(B UKRAINIAN (ISO 8859-5 encoding)"
 "quail/cyrillic")
(register-input-method
 "ukrainian-computer" "Ukrainian" 'quail-use-package
 "UK" "$,1(9(F(C(:(5(=(B Ukrainian (Unicode-based for use with KOI8-U encoding)."
 "quail/cyrillic")
(register-input-method
 "cyrillic-yawerty" "Cyrillic" 'quail-use-package
 ",L6O(B" ",LO25@BK(B Roman transcription (ISO 8859-5 encoding)"
 "quail/cyrillic")
(register-input-method
 "cyrillic-translit" "Cyrillic" 'quail-use-package
 ",L6(Bt" "Intuitively transliterated keyboard layout."
 "quail/cyrillic")
(register-input-method
 "belarusian" "Belarusian" 'quail-use-package
 "BE" "$,1(9(F(C(:(5(=(B keyboard layout registered as STB955-94 Belarusian standard."
 "quail/cyrillic")
(register-input-method
 "bulgarian-phonetic" "Bulgarian" 'quail-use-package
 "$,1(6(1(D(B" "Bulgarian Phonetic keyboard layout, producing Unicode."
 "quail/cyrillic")
(register-input-method
 "bulgarian-bds" "Bulgarian" 'quail-use-package
 "$,1(1(4(A(B" "Bulgarian standard keyboard layout (BDS)"
 "quail/cyrillic")
(register-input-method
 "cyrillic-jis-russian" "Cyrillic" 'quail-use-package
 "$B'('+(B" "$B'+'8'5','&'/(B keyboard layout same as JCUKEN (JIS X0208.1983 encoding)"
 "quail/cyril-jis")
(register-input-method
 "chinese-ctlau" "Chinese-CNS" 'quail-use-package
 "$AAuTA(B" "$A::WVJdHk!KAuN}OiJ=TARt!K(B"
 "quail/CTLau")
(register-input-method
 "chinese-ctlaub" "Chinese-BIG5" 'quail-use-package
 "$(0N,Gn(B" "$(0KH)tTT&,!(N,Tg>A*#Gn5x!((B"
 "quail/CTLau-b5")
(register-input-method
 "croatian" "Croatian" 'quail-use-package
 "HR" "\"Standard\" Croatian keyboard."
 "quail/croatian")
(register-input-method
 "croatian-qwerty" "Croatian" 'quail-use-package
 "HR" "Croatian keyboard without the y/z swap."
 "quail/croatian")
(register-input-method
 "croatian-prefix" "Croatian" 'quail-use-package
 "HR" "Croatian input method, postfix."
 "quail/croatian")
(register-input-method
 "croatian-postfix" "Croatian" 'quail-use-package
 "HR" "Croatian input method, postfix."
 "quail/croatian")
(register-input-method
 "croatian-xy" "Croatian" 'quail-use-package
 "HR" "An alternative Croatian input method."
 "quail/croatian")
(register-input-method
 "croatian-cc" "Croatian" 'quail-use-package
 "HR" "Another alternative Croatian input method."
 "quail/croatian")
(register-input-method
 "chinese-ccdospy" "Chinese-GB" 'quail-use-package
 "$AKuF4(B" "$A::WVJdHk!KKuP4F4Rt!K(B"
 "quail/CCDOSPY")
(register-input-method
 "chinese-array30" "Chinese-BIG5" 'quail-use-package
 "$(0#R#O(B" "$(0&d'G!J*h)E#R#O!K(B"
 "quail/ARRAY30")
(register-input-method
 "chinese-4corner" "Chinese-BIG5" 'quail-use-package
 "$(0(?-F(B" "$(0(?-FHAP#(B::"
 "quail/4Corner")
(eval-after-load "quail/PY-b5"
  '(quail-defrule "ling2" ?$(0!r(B nil t))
(eval-after-load "quail/Punct"
  '(quail-defrule " " ?$A!!(B nil t))
(eval-after-load "quail/Punct-b5"
  '(quail-defrule " " ?$(0!!(B nil t))
(autoload 'ucs-input-activate "quail/uni-input"
  "Activate UCS input method.
With arg, activate UCS input method if and only if arg is positive.
While this input method is active, the variable
`input-method-function' is bound to the function `ucs-input-method'.")
(register-input-method "ucs" "UTF-8" 'ucs-input-activate "U+"
		       "Unicode input as hex in the form Uxxxx.")
