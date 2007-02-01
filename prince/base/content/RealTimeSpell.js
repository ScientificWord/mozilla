var RealTimeSpell =
{
  editor : null,
  editorRTS: null,

  Init : function (editor, enable)
  {
    this.editor = editor;
    this.editorRTS = editor.getInlineSpellChecker(true);
    this.editorRTS.enableRealTimeSpell = enable;
  },

  getMispelledWord : function()
  {
    if (!this.editorRTS) return null;

    var selection = this.editor.selection;
    return this.editorRTS.getMispelledWord(selection.anchorNode,selection.anchorOffset);
  },

  updateMenu : function(menuid, separatorid)
  {
    var word = this.getMispelledWord();

    var suggestionsMenu = document.getElementById(menuid);
    var suggestionsMenuSeparator = document.getElementById(separatorid);
    if (word){
      suggestionsMenu.removeAttribute("hidden");
      suggestionsMenuSeparator.removeAttribute("hidden");
    } else {
      suggestionsMenu.setAttribute("hidden","true");
      suggestionsMenuSeparator.setAttribute("hidden","true");
    }
  },

  updateSuggestionsMenu : function (menupopup, addtodictionaryid, addtodictionaryseparatorid, word)
  {
    if (!this.editorRTS) return false;

    var addSeparator = document.getElementById(addtodictionaryseparatorid);
    var addToDictionary = document.getElementById(addtodictionaryid);

    var child = menupopup.firstChild;
    while (child){
      var next = child.nextSibling;
      if ((child != addSeparator) && (child != addToDictionary)){
        menupopup.removeChild(child);
      }
      child = next;
    }

    if (!word){
      word = this.getMispelledWord();
      if (!word) return false;
    }

    var spellChecker = this.editorRTS.spellChecker;
    if (!spellChecker) return false;

    var found = false;
    var isIncorrect = spellChecker.CheckCurrentWord(word.toString());
    if (isIncorrect){
      do {
        var suggestion = spellChecker.GetSuggestedWord();
        if (!suggestion.length) break;

        found = true;

        var item = document.createElement("menuitem");
        item.setAttribute("label",suggestion);
        item.setAttribute("value",suggestion);
        menupopup.appendChild(item);
      } while(1);
    }

    if (addSeparator){
      if (found) addSeparator.removeAttribute("hidden");
      else addSeparator.setAttribute("hidden","true");
    }

    return isIncorrect;
  },

  selectSuggestion : function (newword, node, offset)
  {
    if (!this.editorRTS) return null;

    if (!node){
      var selection = this.editor.selection;
      node = selection.anchorNode;
      offset = selection.anchorOffset;
    }

    this.editorRTS.replaceWord(node,offset,newword);
  },

  addToDictionary : function (node, offset)
  {
    if (!this.editorRTS) return null;

    if (!node){
      var selection = this.editor.selection;
      node = selection.anchorNode;
      offset = selection.anchorOffset;
    }

    var word = this.editorRTS.getMispelledWord(node,offset);
    if (word) this.editorRTS.addWordToDictionary(word);
  }
}
