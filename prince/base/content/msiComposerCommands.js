// Copyright (c) 2006 MacKichan Software, Inc.  All Rights Reserved.


/* Implementations of nsIControllerCommand for composer commands */

//var gComposerJSCommandControllerID = 0;


// constants for file operations

  const PR_RDONLY      = 0x01
  const PR_WRONLY      = 0x02
  const PR_RDWR        = 0x04
  const PR_CREATE_FILE = 0x08
  const PR_APPEND      = 0x10
  const PR_TRUNCATE    = 0x20
  const PR_SYNC        = 0x40
  const PR_EXCL        = 0x80

//-----------------------------------------------------------------------------------
function msiSetupHTMLEditorCommands(editorElement)
{
  var commandTable = msiGetComposerCommandTable(editorElement);
  if (!commandTable)                   
  {
    alert("No command table for editor element in msiSetupHTMLEditorCommands!");
    return;
  }
  
  // Include everthing a text editor does
//  alert("Calling msiSetupTextEditorCommands from msiSetupHTMLEditorCommands");
  msiSetupTextEditorCommands(editorElement);

  //dump("Registering HTML editor commands\n");

  commandTable.registerCommand("cmd_renderedHTMLEnabler", nsDummyHTMLCommand);

  commandTable.registerCommand("cmd_grid",  msiGridCommand);

  commandTable.registerCommand("cmd_listProperties",  msiListPropertiesCommand);
  commandTable.registerCommand("cmd_pageProperties",  msiPagePropertiesCommand);
  commandTable.registerCommand("cmd_colorProperties", msiColorPropertiesCommand);
  commandTable.registerCommand("cmd_advancedProperties", msiAdvancedPropertiesCommand);
  commandTable.registerCommand("cmd_objectProperties",   msiObjectPropertiesCommand);
  commandTable.registerCommand("cmd_removeNamedAnchors", msiRemoveNamedAnchorsCommand);
  commandTable.registerCommand("cmd_editLink",        msiEditLinkCommand);
  
  commandTable.registerCommand("cmd_form",          msiFormCommand);
  commandTable.registerCommand("cmd_inputtag",      msiInputTagCommand);
  commandTable.registerCommand("cmd_inputimage",    msiInputImageCommand);
  commandTable.registerCommand("cmd_textarea",      msiTextAreaCommand);
  commandTable.registerCommand("cmd_select",        msiSelectCommand);
  commandTable.registerCommand("cmd_button",        msiButtonCommand);
  commandTable.registerCommand("cmd_label",         msiLabelCommand);
  commandTable.registerCommand("cmd_fieldset",      msiFieldSetCommand);
  commandTable.registerCommand("cmd_isindex",       msiIsIndexCommand);
  commandTable.registerCommand("cmd_image",         msiImageCommand);
  commandTable.registerCommand("cmd_hline",         msiHLineCommand);
  commandTable.registerCommand("cmd_link",          msiLinkCommand);
  commandTable.registerCommand("cmd_anchor",        msiAnchorCommand);
  commandTable.registerCommand("cmd_insertHTMLWithDialog", msiInsertHTMLWithDialogCommand);
  commandTable.registerCommand("cmd_insertBreak",   msiInsertBreakCommand);
  commandTable.registerCommand("cmd_insertBreakAll",msiInsertBreakAllCommand);
  commandTable.registerCommand("cmd_insertHorizontalSpaces", msiInsertHorizontalSpacesCommand);
  commandTable.registerCommand("cmd_insertVerticalSpaces", msiInsertVerticalSpacesCommand);
  commandTable.registerCommand("cmd_msiInsertRules", msiInsertRulesCommand);
  commandTable.registerCommand("cmd_msiInsertBreaks", msiInsertBreaksCommand);
                                              

  commandTable.registerCommand("cmd_table",              msiInsertOrEditTableCommand);
  commandTable.registerCommand("cmd_editTable",          msiEditTableCommand);
  commandTable.registerCommand("cmd_SelectTable",        msiSelectTableCommand);
  commandTable.registerCommand("cmd_SelectRow",          msiSelectTableRowCommand);
  commandTable.registerCommand("cmd_SelectColumn",       msiSelectTableColumnCommand);
  commandTable.registerCommand("cmd_SelectCell",         msiSelectTableCellCommand);
  commandTable.registerCommand("cmd_SelectAllCells",     msiSelectAllTableCellsCommand);
  commandTable.registerCommand("cmd_InsertTable",        msiInsertTableCommand);
  commandTable.registerCommand("cmd_InsertRowAbove",     msiInsertTableRowAboveCommand);
  commandTable.registerCommand("cmd_InsertRowBelow",     msiInsertTableRowBelowCommand);
  commandTable.registerCommand("cmd_InsertColumnBefore", msiInsertTableColumnBeforeCommand);
  commandTable.registerCommand("cmd_InsertColumnAfter",  msiInsertTableColumnAfterCommand);
  commandTable.registerCommand("cmd_InsertCellBefore",   msiInsertTableCellBeforeCommand);
  commandTable.registerCommand("cmd_InsertCellAfter",    msiInsertTableCellAfterCommand);
  commandTable.registerCommand("cmd_DeleteTable",        msiDeleteTableCommand);
  commandTable.registerCommand("cmd_DeleteRow",          msiDeleteTableRowCommand);
  commandTable.registerCommand("cmd_DeleteColumn",       msiDeleteTableColumnCommand);
  commandTable.registerCommand("cmd_DeleteCell",         msiDeleteTableCellCommand);
  commandTable.registerCommand("cmd_DeleteCellContents", msiDeleteTableCellContentsCommand);
  commandTable.registerCommand("cmd_JoinTableCells",     msiJoinTableCellsCommand);
  commandTable.registerCommand("cmd_SplitTableCell",     msiSplitTableCellCommand);
  commandTable.registerCommand("cmd_TableOrCellColor",   msiTableOrCellColorCommand);
  commandTable.registerCommand("cmd_NormalizeTable",     msiNormalizeTableCommand);
  commandTable.registerCommand("cmd_ConvertToTable",     msiConvertToTable);
  commandTable.registerCommand("cmd_MSIAnimateGifsOn",   msiGIFAnimation);
  commandTable.registerCommand("cmd_MSIAnimateGifsOff",  msiGIFAnimation);
  commandTable.registerCommand("cmd_printDirect",           msiPrintDirectCommand);
  commandTable.registerCommand("cmd_printDvi",           msiPrintCommand);
  commandTable.registerCommand("cmd_printPdf",           msiPrintCommand);
  commandTable.registerCommand("cmd_previewDirect",         msiPreviewDirectCommand);
  commandTable.registerCommand("cmd_previewDvi",         msiPreviewCommand);
  commandTable.registerCommand("cmd_previewPdf",         msiPreviewCommand);
  commandTable.registerCommand("cmd_compileDvi",         msiCompileCommand);
  commandTable.registerCommand("cmd_compilePdf",         msiCompileCommand);
  commandTable.registerCommand("cmd_updateStructToolbar", msiUpdateStructToolbarCommand);
  commandTable.registerCommand("cmd_insertReturnFancy", msiInsertReturnFancyCommand);
  commandTable.registerCommand("cmd_insertSubstructure", msiInsertSubstructureCommand);
  commandTable.registerCommand("cmd_documentInfo",       msiDocumentInfoCommand);
  commandTable.registerCommand("cmd_documentStyle",       msiDocumentStyleCommand);
  commandTable.registerCommand("cmd_macrofragment", msiMacroFragmentCommand);
  commandTable.registerCommand("cmd_viewInvisibles", msiViewInvisiblesCommand);
  commandTable.registerCommand("cmd_msiReviseHyperlink", msiReviseHyperlinkCommand);
  commandTable.registerCommand("cmd_reviseAnchor", msiReviseAnchorCommand);
  commandTable.registerCommand("cmd_reviseImage", msiReviseImageCommand);
//  commandTable.registerCommand("cmd_reviseLine",  msiReviseLineCommand);
  commandTable.registerCommand("cmd_reviseForm",  msiReviseFormCommand);
  commandTable.registerCommand("cmd_reviseTextarea", msiReviseTextareaCommand);
  commandTable.registerCommand("cmd_reviseButton",  msiReviseButtonCommand);
  commandTable.registerCommand("cmd_reviseLabel",  msiReviseLabelCommand);
  commandTable.registerCommand("cmd_reviseFieldset", msiReviseFieldsetCommand);
  commandTable.registerCommand("cmd_reviseChars",  msiReviseCharsCommand);
  commandTable.registerCommand("cmd_reviseHTML",   msiReviseHTMLCommand);
  commandTable.registerCommand("cmd_reviseHorizontalSpaces", msiReviseHorizontalSpacesCommand);
  commandTable.registerCommand("cmd_reviseVerticalSpaces", msiReviseVerticalSpacesCommand);
  commandTable.registerCommand("cmd_msiReviseRules",   msiReviseRulesCommand);
  commandTable.registerCommand("cmd_msiReviseBreaks",  msiReviseBreaksCommand);
  commandTable.registerCommand("cmd_note", msiNoteCommand);
  commandTable.registerCommand("cmd_frame", msiFrameCommand);
  commandTable.registerCommand("cmd_citation", msiCitationCommand);
  commandTable.registerCommand("cmd_showTeXLog", msiShowTeXLogCommand);
  commandTable.registerCommand("cmd_showXSLTLog", msiShowXSLTLogCommand);
  commandTable.registerCommand("cmd_gotoparagraph", msiGoToParagraphCommand);
  commandTable.registerCommand("cmd_countwords", msiWordCountCommand);
}

function msiSetupTextEditorCommands(editorElement)
{
  var commandTable = msiGetComposerCommandTable(editorElement);
  if (!commandTable)
    return;
  
  //dump("Registering plain text editor commands\n");
  
  commandTable.registerCommand("cmd_find",       msiFindCommand);
  commandTable.registerCommand("cmd_findNext",   msiFindAgainCommand);
  commandTable.registerCommand("cmd_findPrev",   msiFindAgainCommand);
  commandTable.registerCommand("cmd_rewrap",     msiRewrapCommand);
  commandTable.registerCommand("cmd_spelling",   msiSpellingCommand);
  commandTable.registerCommand("cmd_validate",   msiValidateCommand);
  commandTable.registerCommand("cmd_checkLinks", msiCheckLinksCommand);
  commandTable.registerCommand("cmd_insertChars", msiInsertCharsCommand);
  commandTable.registerCommand("cmd_oneshotGreek", msiOneShotGreek);
  commandTable.registerCommand("cmd_oneshotSymbol", msiOneShotSymbol);
}

function msiSetupComposerWindowCommands(editorElement)
{
  // Don't need to do this if already done
  var topWin = msiGetTopLevelWindow(editorElement);
  if (topWin.mComposerWindowControllerID)
    return;

  // Create a command controller and register commands
  //   specific to Web Composer window (file-related commands, HTML Source...)
  //   We can't use the composer controller created on the content window else
  //     we can't process commands when in HTMLSource editor
  // IMPORTANT: For each of these commands, the doCommand method 
  //            must first call FinishHTMLSource() 
  //            to go from HTML Source mode to any other edit mode

  var windowControllers = topWin.controllers;

  if (!windowControllers) return;

  var commandTable;
  var composerController;
  var editorController;
  try {
    composerController = Components.classes["@mozilla.org/embedcomp/base-command-controller;1"].createInstance();

    editorController = composerController.QueryInterface(Components.interfaces.nsIControllerContext);
    editorController.init(null); // init it without passing in a command table

    // Get the nsIControllerCommandTable interface we need to register commands
    var interfaceRequestor = composerController.QueryInterface(Components.interfaces.nsIInterfaceRequestor);
    commandTable = interfaceRequestor.getInterface(Components.interfaces.nsIControllerCommandTable);
  }
  catch (e)
  {
    dump("Failed to create composerController\n");
    return;
  }


  if (!commandTable)
  {
    dump("Failed to get interface for nsIControllerCommandManager\n");
    return;
  }

  // File-related commands
  commandTable.registerCommand("cmd_open",           msiOpenCommand);
  commandTable.registerCommand("cmd_new",            msiNewCommand);
  commandTable.registerCommand("cmd_save",           msiSaveCommand);
  commandTable.registerCommand("cmd_softSave",       msiSoftSaveCommand);
  commandTable.registerCommand("cmd_saveAs",         msiSaveAsCommand);
  commandTable.registerCommand("cmd_saveCopyAs",     msiSaveCopyAsCommand);
  commandTable.registerCommand("cmd_exportToText",   msiExportToTextCommand);
  commandTable.registerCommand("cmd_saveAndChangeEncoding",  msiSaveAndChangeEncodingCommand);
  commandTable.registerCommand("cmd_publish",        msiPublishCommand);
  commandTable.registerCommand("cmd_publishAs",      msiPublishAsCommand);
  commandTable.registerCommand("cmd_publishSettings",msiPublishSettingsCommand);
  commandTable.registerCommand("cmd_revert",         msiRevertCommand);
  commandTable.registerCommand("cmd_openRemote",     msiOpenRemoteCommand);
  commandTable.registerCommand("cmd_preview",        msiPreviewCommand);
  commandTable.registerCommand("cmd_editSendPage",   msiSendPageCommand);
  commandTable.registerCommand("cmd_print",          msiDirectPrintCommand);
  commandTable.registerCommand("cmd_printSetup",     msiPrintSetupCommand);
  commandTable.registerCommand("cmd_quit",           nsQuitCommand);
  commandTable.registerCommand("cmd_close",          msiCloseCommand);
  commandTable.registerCommand("cmd_preferences",    nsPreferencesCommand);

  commandTable.registerCommand("cmd_MSIAutoSubDlg",  msiAutoSubDlgCommand);

  // Edit Mode commands
  if (msiWindowHasHTMLEditor(topWin))
//  if (msiGetEditorType(editorElement) == "html")
  {
//    if (msiIsTopLevelEditor(editorElement))  //The "mode" commands are only available for the top-level editor.
//    {
      commandTable.registerCommand("cmd_NormalMode",         msiNormalModeCommand);
      commandTable.registerCommand("cmd_AllTagsMode",        msiAllTagsModeCommand);
      commandTable.registerCommand("cmd_HTMLSourceMode",     msiHTMLSourceModeCommand);
      commandTable.registerCommand("cmd_PreviewMode",        msiPreviewModeCommand);
      commandTable.registerCommand("cmd_FinishHTMLSource",   msiFinishHTMLSourceCmd);
      commandTable.registerCommand("cmd_CancelHTMLSource",   msiCancelHTMLSource);
//    }
  }

  windowControllers.insertControllerAt(0, editorController);

  // Store the controller ID so we can be sure to get the right one later
  topWin.mComposerWindowControllerID = windowControllers.getControllerId(editorController);
}

//-----------------------------------------------------------------------------------
function msiGetComposerCommandTable(editorElement)
{
  var controller;
  if (editorElement.mComposerJSCommandControllerID)
  {
    try { 
      controller = editorElement.contentWindow.controllers.getControllerById(editorElement.mComposerJSCommandControllerID);
    } catch (e) {}
  }
  if (!controller)
  {
    //create it
    controller = Components.classes["@mozilla.org/embedcomp/base-command-controller;1"].createInstance();

    var editorController = controller.QueryInterface(Components.interfaces.nsIControllerContext);
    editorController.init(null);
    editorController.setCommandContext(editorElement);
    editorElement.contentWindow.controllers.insertControllerAt(0, controller);
  
    // Store the controller ID so we can be sure to get the right one later
    editorElement.mComposerJSCommandControllerID = editorElement.contentWindow.controllers.getControllerId(controller);
//    alert("Creating command controller for editorElement [" + editorElement.nodeName + ", id " + editorElement.id + "]");
  }

  if (controller)

  {
    var interfaceRequestor = controller.QueryInterface(Components.interfaces.nsIInterfaceRequestor);
    return interfaceRequestor.getInterface(Components.interfaces.nsIControllerCommandTable);
  }
  return null;
}

//START HERE to reexamine more utility functions.

//-----------------------------------------------------------------------------------
function msiGoUpdateCommandState(command, editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var bIsTop = (editorElement == null || msiIsTopLevelEditor(editorElement));
  try
  {
    var controller = null;
    if (editorElement)
      controller = msiGetControllerForCommand(command, editorElement);
    if (!controller)
      controller = top.document.commandDispatcher.getControllerForCommand(command);
    if (!(controller instanceof Components.interfaces.nsICommandController))
      return;

    var params = newCommandParams();
    if (!params) return;

    controller.getCommandStateWithParams(command, params);

    switch (command)
    {
      case "cmd_bold":
      case "cmd_italic":
      case "cmd_underline":
      case "cmd_var":
      case "cmd_samp":
      case "cmd_code":
      case "cmd_acronym":
      case "cmd_abbr":
      case "cmd_cite":
      case "cmd_strong":
      case "cmd_em":
      case "cmd_superscript":
      case "cmd_subscript":
      case "cmd_strikethrough":
      case "cmd_tt":
      case "cmd_nobreak":
      case "cmd_ul":
      case "cmd_ol":
      case "cmd_MSImathtext":
        msiPokeStyleUI(command, params.getBooleanValue("state_all"));
        break;

      case "cmd_paragraphState":
      case "cmd_align":
      case "cmd_highlight":
      case "cmd_backgroundColor":
      case "cmd_fontColor":
      case "cmd_fontFace":
      case "cmd_fontSize":
      case "cmd_absPos":
        msiPokeMultiStateUI(command, params);
        break;

      case "cmd_texttag":
      case "cmd_paratag":
      case "cmd_structtag":
      case "cmd_othertag":

        msiPokeTagStateUI(command, params);
        break;

      case "cmd_viewInvisibles":
        updateViewMenuFromEditor(editorElement);
      break;

      case "cmd_decreaseZIndex":
      case "cmd_increaseZIndex":
      case "cmd_indent":
      case "cmd_outdent":
      case "cmd_increaseFont":
      case "cmd_decreaseFont":
      case "cmd_removeStyles":
      
      case "cmd_smiley":
        break;

      default: dump("no update for command: " +command+"\n");
    }
  }
  catch (e) { dump("An error occurred updating the "+command+" command: \n"+e+"\n"); }
}

function msiGoUpdateComposerMenuItems(commandset, editorElement)
{
  //dump("Updating commands for " + commandset.id + "\n");

  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  for (var i = 0; i < commandset.childNodes.length; i++)
  {
    var commandNode = commandset.childNodes[i];
    var commandID = commandNode.id;
    if (commandID)
    {
      msiGoUpdateCommand(commandID, editorElement);  // enable or disable
      if (commandNode.hasAttribute("state"))
        msiGoUpdateCommandState(commandID, editorElement);
    }
  }
}

function msiGoUpdateTagSelectors(commandset, editorElement)
{
  //dump("Updating commands for " + commandset.id + "\n");

  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  for (var i = 0; i < commandset.childNodes.length; i++)
  {
    var commandNode = commandset.childNodes[i];
    var commandID = commandNode.id;
    if (commandID)
    {
      msiGoUpdateCommand(commandID, editorElement);  // enable or disable
      if (commandNode.hasAttribute("state"))
        msiGoUpdateCommandState(commandID, editorElement);
    }
  }
}
//
////-----------------------------------------------------------------------------------
function msiGoDoCommandParams(command, params, editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  try
  {
//    var controller = top.document.commandDispatcher.getControllerForCommand(command);
    var controller = msiGetControllerForCommand(command, editorElement);
    if (!controller)
    {
      msiDumpWithID("In msiGoDoCommandParams, controller not found for command [" + command + "] for editor [@].\n", editorElement);
      controller = top.document.commandDispatcher.getControllerForCommand(command);
    }

    if (controller && controller.isCommandEnabled(command))
    {
      if (controller instanceof Components.interfaces.nsICommandController)
      {
        controller.doCommandWithParams(command, params);

        // the following two lines should be removed when we implement observers
        if (params)
          controller.getCommandStateWithParams(command, params);
      }
      else
      {
        controller.doCommand(command);
      }
      msiResetStructToolbar(editorElement);
    }
  }
  catch (e)
  {
    dump("An error occurred executing the "+command+" command\n"+e);
  }
}

function msiPokeStyleUI(uiID, aDesiredState)
{
  try {
    var docList = msiGetUpdatableItemContainers(uiID, msiGetActiveEditorElement());
    for (var i = 0; i < docList.length; ++i)
    {
      var commandNode = docList[i].getElementById(uiID);
      if (commandNode)
      {
        var uiState = ("true" == commandNode.getAttribute("state"));
        if (aDesiredState != uiState)
        {
          var newState;
          if (aDesiredState)
            newState = "true";
          else
            newState = "false";
          commandNode.setAttribute("state", newState);
        }
      }
    }
//    var topWindow = msiGetTopLevelWindow();
//    if ("pokeStyleUI" in topWindow)
//      topWindow.pokeStyleUI(uiID, aDesiredState);
//    if (topWindow != window && "pokeStyleUI" in window)
//      window.pokeStyleUI(uiID, aDesiredState);
////    var commandNode = topWindow.document.getElementById(uiID);
////    if (!commandNode)
////      return;
////
////    var uiState = ("true" == commandNode.getAttribute("state"));
////    if (aDesiredState != uiState)
////    {
////      var newState;
////      if (aDesiredState)
////        newState = "true";
////      else
////        newState = "false";
////      commandNode.setAttribute("state", newState);
////    }
  } catch(e) { dump("poking UI for "+uiID+" failed: "+e+"\n"); }
}

function msiDoStyleUICommand(cmdStr)
{
  try
  {
    var cmdParams = newCommandParams();
    msiGoDoCommandParams(cmdStr, cmdParams);
    if (cmdParams)
      msiPokeStyleUI(cmdStr, cmdParams.getBooleanValue("state_all"));

    msiResetStructToolbar();
  } catch(e) {}
}

function msiPokeMultiStateUI(uiID, cmdParams)
{
  var isMixed = cmdParams.getBooleanValue("state_mixed");
  var desiredAttrib;
  if (isMixed)
    desiredAttrib = "mixed";
  else
    desiredAttrib = cmdParams.getCStringValue("state_attribute");

  try
  {
    var docList = msiGetUpdatableItemContainers(uiID, msiGetActiveEditorElement());
    for (var i = 0; i < docList.length; ++i)
    {
      var commandNode = docList[i].getElementById(uiID);
      if (commandNode != null)
      {
        var uiState = commandNode.getAttribute("state");
        if (desiredAttrib != uiState)
        {
          commandNode.setAttribute("state", desiredAttrib);
        }
      }
    }
  } catch(e) {}
}

function msiPokeTagStateUI(uiID, cmdParams)
{
  var textboxName;
  switch (uiID)
  {
    case "cmd_texttag":
      textboxName = "TextTagSelections";
      break;
    case "cmd_paratag":
      textboxName = "ParaTagSelections";
      break;
    case "cmd_secttag":
      textboxName = "SectTagSelections";
      break;
    case "cmd_othertag":
      textboxName = "OtherTagSelections";
      break;
    default:
      break;
//      return;
  }   
  var desiredAttrib;
  desiredAttrib = cmdParams.getStringValue("state_attribute");

  try
  {
    var docList = msiGetUpdatableItemContainers(uiID, msiGetActiveEditorElement());
    for (var i = 0; i < docList.length; ++i)
    {
      var commandNode = docList[i].getElementById(uiID);
      if (commandNode)
      {
        var uiState = commandNode.getAttribute("state");
        if (desiredAttrib != uiState)
        {
          commandNode.setAttribute("state", desiredAttrib);
    //      commandNode.setAttribute("value", desiredAttrib);
          var textbox = docList[i].getElementById(textboxName);
          if (textbox)
            textbox.textValue = desiredAttrib;
        }
      }
    }
  } catch(e) {}
}
//
//
function msiDoStatefulCommand(commandID, newState, editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var docList = msiGetUpdatableItemContainers(commandID, editorElement);
  for (var i = 0; i < docList.length; ++i)
  {
    var commandNode = docList[i].getElementById(commandID);
    if (commandNode)
      commandNode.setAttribute("state", newState);
  }

  try
  {
    editorElement.contentWindow.focus();   // needed for command dispatch to work

    var cmdParams = newCommandParams();
    if (!cmdParams) return;

    cmdParams.setCStringValue("state_attribute", newState);
    var editor = msiGetEditor(editorElement);
    var ns = new Object;
    if (commandID=="cmd_texttag" && editor && editor.tagListManager && editor.tagListManager.getClearTextTag(ns) == newState)

    {
      msiGoDoCommand('cmd_removeStyles');
    }
    else
      msiGoDoCommandParams(commandID, cmdParams, editorElement);
    // BBM: temporary hack!
    switch (commandID)
    {
      case "cmd_texttag":
      case "cmd_paratag":
      case "cmd_structtag":
      case "cmd_othertag":
        msiPokeTagStateUI(commandID, cmdParams);
        break;
      default:
        msiPokeMultiStateUI(commandID, cmdParams);
    }
    msiResetStructToolbar(editorElement);
  } catch(e) { dump("error thrown in msiDoStatefulCommand: "+e+"\n"); }
}

function getNextTagWindow(commandID)
{
  var idNext;
  switch (commandID)
  {
    case "cmd_texttag":
      idNext = "ParaTagSelections";
      break;
    case "cmd_paratag":
      idNext = "StructTagSelections";
      break;
    case "cmd_structtag":
      idNext = "OtherTagSelections";
      break;
    case "cmd_othertag":
      idNext = "TextTagSelections";
      break;
    default:
      idNext = "TextTagSelections";
  }
  return document.getElementById(idNext);
}
      
function getPrevTagWindow(commandID)
{
  var idPrev;
  switch (commandID)
  {
    case "cmd_texttag":
      idPrev = "OtherTagSelections";
      break;
    case "cmd_paratag":
      idPrev = "TextTagSelections";
      break;
    case "cmd_structtag":
      idPrev = "ParaTagSelections";
      break;
    case "cmd_othertag":
      idPrev = "StructTagSelections";
      break;
    default:
      idNext = "TextTagSelections";
  }
  return document.getElementById(idPrev);
}
      

function doTagKeyCommand(event, commandID, value)
{
//  if (event.keyCode == KeyEvent.DOM_VK_RETURN) msiDoStatefulCommand(commandID, value);
//  else 
  if (event.keyCode == KeyEvent.DOM_VK_ESCAPE) msiGetActiveEditorElement().contentWindow.focus();
  else if (event.keyCode == KeyEvent.DOM_VK_TAB)
  {
    if (event.shiftKey) getPrevTagWindow(commandID).focus();
    else getNextTagWindow(commandID).focus();
  } 
}

function getViewSettingsFromViewMenu()
{
  var viewSettings = new msiViewSettings(1);   //1 is really the default - hide invisibles, show everything else
  var invisChoices = [["viewInvisibles","showInvisibles"], ["viewHelperLines","showHelperLines"], 
                      ["viewInputBoxes","showInputBoxes"], ["viewIndexEntries","showIndexEntries"],
                      ["viewMarkers","showMarkers"],       ["viewFootnotes", "showFootnotes"],
                      ["viewOtherNotes","showOtherNotes"]];
  var theWindow = msiGetTopLevelWindow();
  var theDocument = theWindow ? theWindow.document : null;
  if (theDocument != null)
  {
    for (var ix = 0; ix < invisChoices.length; ++ix)
    {
      var menuItem = document.getElementById(invisChoices[ix][0]);
      if (menuItem)
      {
        viewSettings[invisChoices[ix][1]] = (menuItem.getAttribute("checked") == "true");
      }
    }
  }
  return viewSettings;
}

function updateEditorFromViewMenu(editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var viewSettings = getViewSettingsFromViewMenu();
  dump("Calling msiEditorDoShowInvisibles, with viewSettings = [invis: " + viewSettings.showInvisibles + ", helperLines: " + viewSettings.showHelperLines + ", inputBoxes: " + viewSettings.showInputBoxes + ", indexEntries: " + viewSettings.showIndexEntries + ", markers: " + viewSettings.showMarkers + "]\n");
  msiEditorDoShowInvisibles(editorElement, viewSettings);
}

function updateViewMenuFromEditor(editorElement)
{
  if (!("viewSettings" in editorElement) || (editorElement.viewSettings == null))
    return;  //No settings in the editor - leave menu as is.

  var invisChoices = [["viewInvisibles","showInvisibles"], ["viewHelperLines","showHelperLines"], 
                      ["viewInputBoxes","showInputBoxes"], ["viewIndexEntries","showIndexEntries"],
                      ["viewMarkers","showMarkers"]];
  for (var ix = 0; ix < invisChoices.length; ++ix)
  {
    var menuItem = document.getElementById(invisChoices[ix][0]);
    if (menuItem != null)
    {
      if (editorElement.viewSettings[invisChoices[ix][1]])
        menuItem.setAttribute("checked", "true");
      else
        menuItem.setAttribute("checked", "false");
    }
  }
}

////-----------------------------------------------------------------------------------
//function PrintObject(obj)
//{
//  dump("-----" + obj + "------\n");
//  var names = "";
//  for (var i in obj)
//  {
//    if (i == "value")
//      names += i + ": " + obj.value + "\n";
//    else if (i == "id")
//      names += i + ": " + obj.id + "\n";
//    else
//      names += i + "\n";
//  }
//  
//  dump(names + "-----------\n");
//}
//
////-----------------------------------------------------------------------------------
//function PrintNodeID(id)
//{
//  PrintObject(document.getElementById(id));
//}

//START HERE for a few more commands to look into.

//-----------------------------------------------------------------------------------
var nsDummyHTMLCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return (msiIsDocumentEditable() && msiIsEditingRenderedHTML());
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    // do nothing
    dump("Hey, who's calling the dummy command?\n");
  }

};


//-----------------------------------------------------------------------------------
var msiGIFAnimation = 
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return (msiIsDocumentEditable() && msiIsEditingRenderedHTML());
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon){},
  doCommand: function(aCommand) {
    if (aCommand == "cmd_MSIAnimateGifsOff")
      msiStopAnimation();
    else if (aCommand == "cmd_MSIAnimateGifsOn")
      msiStartAnimation();
    // else some registration screw-up
  }
};


//-----------------------------------------------------------------------------------
var msiOpenCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;    // we can always do this
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var fp = Components.classes["@mozilla.org/filepicker;1"].
               createInstance(Components.interfaces.nsIFilePicker);
    fp.init(window, GetString("OpenAppFile"), Components.interfaces.nsIFilePicker.modeOpen);
    msiSetFilePickerDirectory(fp, MSI_EXTENSION);
    fp.appendFilter(GetString("AppDocs"),"*."+MSI_EXTENSION);
    fp.appendFilter(GetString("XHTMLFiles"),"*.xhtml; *.xht");
    fp.appendFilters(Components.interfaces.nsIFilePicker.filterAll);

    try {
      fp.show();
    }
    catch (ex) {
      dump("filePicker.show() threw an exception: "+ex.toString()+"\n");
    }

    try 
    {
      if ((fp.file) && (fp.file.path.length > 0)) 
      {
        dump("Ready to edit page: " + fp.fileURL.spec +"\n");
        var documentfile = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
        documentfile.initWithPath( fp.file.path );
        var regexp = /\.sci$/i;
        var newdocumentfile;
        if (regexp.test(fp.file.path))
          newdocumentfile = createWorkingDirectory(documentfile);
        else newdocumentfile = documentfile;
        msiEditPage(newdocumentfile.path, window, false);
        msiSaveFilePickerDirectoryEx(fp, documentfile.parent.path, MSI_EXTENSION);
      }
    } 
    catch (e) 
    { 
      dump(" open:doCommand failed: "+e+"\n"); 
    }
  }
};



//-----------------------------------------------------------------------------------
var msiNewCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;    // we can always do this
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var newdocumentfile;
    var dir;
    var data={file: "not yet"};
    window.openDialog("chrome://prince/content/openshell.xul","_blank", "chrome,close,titlebar,modal,resizable=yes", data);
    if (data.filename)
    {
      if (data.filename && data.filename.length > 0) {
        dump("Ready to edit shell: " + data.filename +"\n");
        try {
          var thefile = Components.classes["@mozilla.org/file/local;1"].           
            createInstance(Components.interfaces.nsILocalFile);
          thefile.initWithPath(data.filename);
          newdocumentfile = createWorkingDirectory(thefile);
          msiEditPage("file:///"+newdocumentfile.path, window, false);
        } catch (e) { dump("msiEditPage failed: "+e+"\n"); }

      }
    }
  }
} 


//// STRUCTURE TOOLBAR
////
var msiUpdateStructToolbarCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    msiUpdateStructToolbar(editorElement);
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},
  doCommand: function(aCommand)  {}
}

// ******* File output commands and utilities ******** //
//-----------------------------------------------------------------------------------
var msiSaveCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    // Always allow saving when editing a remote document,
    //  otherwise the document modified state would prevent that
    //  when you first open a remote file.
    var editorElement = msiGetActiveEditorElement();
    if (!msiIsTopLevelEditor(editorElement))
      return false;
    try
    {
      var docUrl = msiGetEditorURL(editorElement);
      return msiIsDocumentEditable(editorElement) &&
        (msiIsDocumentModified(editorElement) || msiIsHTMLSourceChanged(editorElement) ||
         IsUrlAboutBlank(docUrl) || IsUrlUntitled(docUrl) || GetScheme(docUrl) != "file");
    } catch (e) {return false;}
  },
  
  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var result = false;
    var editorElement = msiGetActiveEditorElement();
    if (!msiIsTopLevelEditor(editorElement))
      return result;

    var editor = msiGetEditor(editorElement);
    if (editor)
    {
      msiFinishHTMLSource(editorElement);
      var url = msiGetEditorURL(editorElement);
      result = msiSaveDocument(true, IsUrlAboutBlank(url)||IsUrlUntitled(url), false, editor.contentsMIMEType, editor, editorElement);
      editorElement.contentWindow.focus();
    }
    return result;
  }
}

function doSoftSave(editorElement, editor)
{
  if (editor)
  {
    // we should be doing this only for top level documents, and we should restore the focus
    msiFinishHTMLSource(editorElement);
    var url = msiGetEditorURL(editorElement);
    result = msiSoftSave(editor, editorElement);
  }
  return result;
}

var msiSoftSaveCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return msiSaveCommand.isCommandEnabled(aCommand, dummy);
  },
  
  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var result = false;
    var editorElement = msiGetActiveEditorElement();
    if (!msiIsTopLevelEditor(editorElement))
      return result;

    var editor = msiGetEditor(editorElement);
    return doSoftSave(editorElement, editor);
  }
}

var msiSaveAsCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    if (!msiIsTopLevelEditor(editorElement))
      return false;
    return msiIsDocumentEditable(editorElement);
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement();
    if (!msiIsTopLevelEditor(editorElement))
      return false;
    var editor = msiGetEditor(editorElement);
    if (editor)
    {
      msiFinishHTMLSource(editorElement);
      var result = msiSaveDocument(true, true, false, editor.contentsMIMEType, editor, editorElement);
      editorElement.contentWindow.focus();
      return result;
    }
    return false;                                   
  }
}



var msiSaveCopyAsCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
    var editorElement = msiGetActiveEditorElement();
    if (!msiIsTopLevelEditor(editorElement))
      return false;
    return msiIsDocumentEditable(editorElement);
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement();
    if (!msiIsTopLevelEditor(editorElement))
      return false;
    var editor = msiGetEditor(editorElement);
    if (editor)
    {
      msiFinishHTMLSource(editorElement);
      var result = msiSaveDocument(true, true, true, editor.contentsMIMEType, editor, editorElement);
      editorElement.contentWindow.focus();
      return result;
    }
    return false;
  }
}



var msiExportToTextCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    if (!msiIsTopLevelEditor(editorElement))
      return false;
    return (msiIsDocumentEditable(editorElement));
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement();
    if (!msiIsTopLevelEditor(editorElement))
      return false;
    var editor = msiGetEditor(editorElement);
    if (editor)
    {
      msiFinishHTMLSource(editorElement);
      var result = msiSaveDocument(true, true, true, "text/plain", editor, editorElement);
      editorElement.contentWindow.focus();
      return result;
    }
    return false;
  }
}

var msiSaveAndChangeEncodingCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    if (!msiIsTopLevelEditor(editorElement))
      return false;
    return (msiIsDocumentEditable(editorElement));
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {    
    var editorElement = msiGetActiveEditorElement();
    if (!msiIsTopLevelEditor(editorElement))
      return false;
    var editor = msiGetEditor(editorElement);
    msiFinishHTMLSource(editorElement);
    window.ok = false;
    window.exportToText = false;
    var oldTitle = msiGetDocumentTitle(editorElement);
    window.openDialog("chrome://editor/content/EditorSaveAsCharset.xul","_blank", "chrome,close,titlebar,modal,resizable=yes");

    if (msiGetDocumentTitle(editorElement) != oldTitle)
      UpdateWindowTitle();

    if (window.ok)
    {
      if (window.exportToText)
      {
        window.ok = msiSaveDocument(true, true, true, "text/plain", editorElement);
      }
      else
      {
        var editor = msiGetEditor(editorElement);
        window.ok = msiSaveDocument(true, true, false, (editor ? editor.contentsMIMEType : null), editor, editorElement);
      }
    }

    editorElement.contentWindow.focus();
    return window.ok;
  }
};
//
var msiPrintDirectCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },
  
  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    PrintUtils.print();
  }
}

function getNavToolbox()
{
  return document.getElementById("toolboxes");
}

function onEnterPP()
{
  var toolbox = document.getElementById("toolboxes");
  toolbox.hidden = true;
}

function onExitPP()
{
  var toolbox = document.getElementById("toolboxes");
  toolbox.hidden = false;
}

function getBrowser()
{
  return document.getElementById("preview-frame");
}

var msiPreviewDirectCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },
  
  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    PrintUtils.printPreview(onEnterPP, onExitPP);
  }
}


var msiPublishCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetTopLevelEditorElement();
    if (msiIsDocumentEditable(editorElement))
    {
      // Always allow publishing when editing a local document,
      //  otherwise the document modified state would prevent that
      //  when you first open any local file.
      try {
        var docUrl = msiGetEditorURL(editorElement);
//        return IsDocumentModified() || IsHTMLSourceChanged()
//               || IsUrlAboutBlank(docUrl) || GetScheme(docUrl) == "file";
        return (msiIsDocumentModified(editorElement) || msiIsHTMLSourceChanged(editorElement) ||
                IsUrlAboutBlank(docUrl) || IsUrlUntitled(docUrl) || GetScheme(docUrl) == "file");
      } catch (e) {return false;}
    }
    return false;
  },
  
  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetTopLevelEditorElement();
    if (msiGetEditor(editorElement))
    {
      var docUrl = msiGetEditorURL(editorElement);
      var filename = GetFilename(docUrl);
      var publishData;
      var showPublishDialog = false;

      // First check pref to always show publish dialog
      try {
        var prefs = GetPrefs();
        if (prefs)
          showPublishDialog = prefs.getBoolPref("editor.always_show_publish_dialog");
      } catch(e) {}

      if (!showPublishDialog && filename)
      {
        // Try to get publish data from the document url
        publishData = CreatePublishDataFromUrl(docUrl);

        // If none, use default publishing site? Need a pref for this
        //if (!publishData)
        //  publishData = GetPublishDataFromSiteName(GetDefaultPublishSiteName(), filename);
      }

      if (showPublishDialog || !publishData)
      {
        // Show the publish dialog
        publishData = {};
        window.ok = false;
        var oldTitle = msiGetDocumentTitle(editorElement);
        window.openDialog("chrome://editor/content/EditorPublish.xul","_blank", 
                          "chrome,close,titlebar,modal", "", "", publishData);
        if (msiGetDocumentTitle(editorElement) != oldTitle)
          UpdateWindowTitle();

        window.content.focus();
        if (!window.ok)
          return false;
      }
      if (publishData)
      {
        msiFinishHTMLSource(editorElement);
//        FinishHTMLSource();
        return msiPublish(publishData, editorElement);
      }
    }
    return false;
  }
}

var msiPublishAsCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return (msiIsDocumentEditable(msiGetTopLevelEditorElement()));
//    return (IsDocumentEditable());
  },
  
  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetTopLevelEditorElement();
    if (msiGetEditor(editorElement))
    {
      msiFinishHTMLSource(editorElement);

      window.ok = false;
      var publishData = {};
      var oldTitle = msiGetDocumentTitle(editorElement);
      window.openDialog("chrome://editor/content/EditorPublish.xul","_blank", 
                        "chrome,close,titlebar,modal", "", "", publishData);
      if (msiGetDocumentTitle(editorElement) != oldTitle)
        UpdateWindowTitle();

      window.content.focus();
      if (window.ok)
        return msiPublish(publishData, editorElement);
    }
    return false;
  }
}

// ------- output utilites   ----- //

//START HERE to reexamine output utilities. Some may need to be overwritten.

// returns a fileExtension string
function msiGetExtensionBasedOnMimeType(aMIMEType)
{
  try {
    var mimeService = null;
    mimeService = Components.classes["@mozilla.org/mime;1"].getService();
    mimeService = mimeService.QueryInterface(Components.interfaces.nsIMIMEService);

    var fileExtension = mimeService.getPrimaryExtension(aMIMEType, null);

    // the MIME service likes to give back ".htm" for text/html files,
    // so do a special-case fix here. Also we want to use the sci extension
    if (fileExtension == "htm" || fileExtension == "xml")
      fileExtension = MSI_EXTENSION;

    return fileExtension;
  }
  catch (e) {}
  return "";
}

function msiGetSuggestedFileName(aDocumentURLString, aMIMEType, editorElement)
{
  var filename = GetFilename(aDocumentURLString);
  if (filename.length > 0) return filename;
  
  // I kind of doubt that any of the following code gets used
  dump("If you see this message, look at msiGetSuggestedFileName in msiComposerCommands\n");
  
  var extension = msiGetExtensionBasedOnMimeType(aMIMEType);
  if (extension)
  {
    if (extension == "xhtml") extension = "sci";
    extension = "." + extension;
  }
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();

  // check for existing file name we can use
  if (aDocumentURLString.length >= 0 && !IsUrlAboutBlank(aDocumentURLString))
  {
    var docURI = null;
    try {

      var ioService = msiGetIOService();
      docURI = ioService.newURI(aDocumentURLString, msiGetEditor(editorElement).documentCharacterSet, null);
      docURI = docURI.QueryInterface(Components.interfaces.nsIURL);

      // grab the file name
      var url = docURI.fileBaseName;
      if (url)
        return url+extension;
    } catch(e) {}
  } 

  // check if there is a title we can use
  var title = msiGetDocumentTitle(editorElement);
  // generate a valid filename, if we can't just go with "untitled"
  return title+extension;
}

// returns file picker result
function msiPromptForSaveLocation(aDoSaveAsText, aEditorType, aMIMEType, aDocumentURLString, editorElement)
{
  var dialogResult = {};
  dialogResult.filepickerClick = msIFilePicker.returnCancel;
  dialogResult.resultingURI = "";
  dialogResult.resultingLocalFile = null;

  if (!editorElement)
    editorElement = msiGetActiveEditorElement();

  var fp = null;
  try {
    fp = Components.classes["@mozilla.org/filepicker;1"].createInstance(msIFilePicker);
  } catch (e) {}
  if (!fp) return dialogResult;

  // determine prompt string based on type of saving we'll do
  var promptString;
  if (aDoSaveAsText || aEditorType == "text")
    promptString = GetString("ExportToText");
  else
    promptString = GetString("SaveDocumentAs")

  fp.init(window, promptString, msIFilePicker.modeSave);
  // Set filters according to the type of output
  if (aDoSaveAsText)
    fp.appendFilters(msIFilePicker.filterText);
  else
  {
    fp.appendFilter("SWP Documents","*."+MSI_EXTENSION);
    fp.defaultExtension = MSI_EXTENSION;
  }
  fp.appendFilters(msIFilePicker.filterAll);

  // now let's actually set the filepicker's suggested filename
  var suggestedFileName = msiGetSuggestedFileName(aDocumentURLString, aMIMEType, editorElement);
  if (suggestedFileName)
  {
    var lastDot = suggestedFileName.lastIndexOf(".");
    if (lastDot != -1)
      suggestedFileName = suggestedFileName.slice(0, lastDot);
  
    fp.defaultString = suggestedFileName;
  }
  // set the file picker's current directory
  // assuming we have information needed (like prior saved location)
  try {
    var ioService = msiGetIOService();
    var fileHandler = msiGetFileProtocolHandler();
    
    var isLocalFile = true;
    try {
      var docURI = ioService.newURI(aDocumentURLString, msiGetEditor(editorElement).documentCharacterSet, null);
      isLocalFile = docURI.schemeIs("file");
    }
    catch (e) {}

    var parentLocation = null;
    if (isLocalFile)
    {
      var fileLocation = fileHandler.getFileFromURLSpec(aDocumentURLString); // this asserts if url is not local
      parentLocation = fileLocation.parent;
    }
    if (parentLocation)
    {
      // Save current filepicker's default location
      if ("gFilePickerDirectory" in window)
        gFilePickerDirectory = fp.displayDirectory;

      fp.displayDirectory = parentLocation;
    }
    else
    {
      // Initialize to the last-used directory for the particular type (saved in prefs)
      msiSetFilePickerDirectory(fp, aEditorType);
    }
  }
  catch(e) {}

  dialogResult.filepickerClick = fp.show();
  if (dialogResult.filepickerClick != msIFilePicker.returnCancel)
  {
    // reset urlstring to new save location
    dialogResult.resultingURIString = fileHandler.getURLSpecFromFile(fp.file);
    dialogResult.resultingLocalFile = fp.file;
    msiSaveFilePickerDirectory(fp, aEditorType);
  }
  else if ("gFilePickerDirectory" in window && gFilePickerDirectory)
    fp.displayDirectory = gFilePickerDirectory; 

  return dialogResult;
}

// returns a boolean (whether to continue (true) or not (false) because user canceled)
function msiPromptAndSetTitleIfNone(editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  if (msiGetDocumentTitle(editorElement)) // we have a title; no need to prompt!
    return true;

  var promptService = msiGetPromptService();
  if (!promptService) return false;

  var result = {value:null};
  var captionStr = GetString("DocumentTitle");
  var msgStr = GetString("NeedDocTitle") + '\n' + GetString("DocTitleHelp");
  var confirmed = promptService.prompt(window, captionStr, msgStr, result, null, {value:0});
  if (confirmed)
    msiSetDocumentTitle(editorElement, TrimString(result.value));

  return confirmed;
}

function msiPersistObj(aStatus, aState)
{
  this.result = aStatus;
  this.currentState = aState;
}

// Don't forget to do these things after calling OutputFileWithPersistAPI:
// we need to update the uri before notifying listeners
//    if (doUpdateURI)
//      SetDocumentURI(docURI);
//    UpdateWindowTitle();
//    if (!aSaveCopy)
//      editor.resetModificationCount();
      // this should cause notification to listeners that document has changed

const msiWebPersist = Components.interfaces.nsIWebBrowserPersist;
function msiOutputFileWithPersistAPI(editorDoc, aDestinationLocation, aRelatedFilesParentDir, aMimeType, editorElement)
{
  if (!editorElement)
    editorElement = msiGetPrimaryEditorElementForWindow(window);
  editorElement.mPersistObj = null;
  var editor = msiGetEditor(editorElement);
  try {
    var imeEditor = editor.QueryInterface(Components.interfaces.nsIEditorIMESupport);
    imeEditor.ForceCompositionEnd();
    } catch (e) {}

  var isLocalFile = false;
  try {
    var tmp1 = aDestinationLocation.QueryInterface(Components.interfaces.nsIFile);
    isLocalFile = true;
  } 
  catch (e) {
    try {
      var tmp = aDestinationLocation.QueryInterface(Components.interfaces.nsIURI);
      isLocalFile = tmp.schemeIs("file");
    }
    catch (e) {}
  }

  try {
    // we should supply a parent directory if/when we turn on functionality to save related documents
    var persistObj = Components.classes["@mozilla.org/embedding/browser/nsWebBrowserPersist;1"].createInstance(msiWebPersist);
    persistObj.progressListener = new msigEditorOutputProgressListener(editorElement);
    
    var wrapColumn = msiGetWrapColumn(editorElement);
    var outputFlags = msiGetOutputFlags(aMimeType, wrapColumn, editorElement);

    // for 4.x parity as well as improving readability of file locally on server
    // this will always send crlf for upload (http/ftp)
    if (!isLocalFile) // if we aren't saving locally then send both cr and lf
    {
      outputFlags |= msiWebPersist.ENCODE_FLAGS_CR_LINEBREAKS | msiWebPersist.ENCODE_FLAGS_LF_LINEBREAKS;

      // we want to serialize the output for all remote publishing
      // some servers can handle only one connection at a time
      // some day perhaps we can make this user-configurable per site?
      persistObj.persistFlags = persistObj.persistFlags | msiWebPersist.PERSIST_FLAGS_SERIALIZE_OUTPUT;
    }

    // note: we always want to set the replace existing files flag since we have
    // already given user the chance to not replace an existing file (file picker)
    // or the user picked an option where the file is implicitly being replaced (save)
    persistObj.persistFlags = persistObj.persistFlags 
                            | msiWebPersist.PERSIST_FLAGS_NO_BASE_TAG_MODIFICATIONS
                            | msiWebPersist.PERSIST_FLAGS_REPLACE_EXISTING_FILES
                            | msiWebPersist.PERSIST_FLAGS_DONT_FIXUP_LINKS
                            | msiWebPersist.PERSIST_FLAGS_DONT_CHANGE_FILENAMES
                            | msiWebPersist.PERSIST_FLAGS_FIXUP_ORIGINAL_DOM;
    persistObj.saveDocument(editorDoc, aDestinationLocation, aRelatedFilesParentDir, 
                            aMimeType, outputFlags, wrapColumn);
    editorElement.mPersistObj = persistObj;
  }
  catch(e) { dump("caught an error, bail\n"); return false; }

  return true;
}

// returns output flags based on mimetype, wrapCol and prefs
function msiGetOutputFlags(aMimeType, aWrapColumn, editorElement)
{
  var outputFlags = 0;
  var editor = msiGetEditor(editorElement);
  var outputEntity = (editor && editor.documentCharacterSet == "ISO-8859-1")
    ? msiWebPersist.ENCODE_FLAGS_ENCODE_LATIN1_ENTITIES
    : msiWebPersist.ENCODE_FLAGS_ENCODE_BASIC_ENTITIES;
  if (aMimeType == "text/plain")
  {
    // When saving in "text/plain" format, always do formatting
    outputFlags |= msiWebPersist.ENCODE_FLAGS_FORMATTED;
  }
  else
  {
    try {
      // Should we prettyprint? Check the pref
      var prefs = GetPrefs();
      if (prefs.getBoolPref("editor.prettyprint"))
        outputFlags |= msiWebPersist.ENCODE_FLAGS_FORMATTED;

      // How much entity names should we output? Check the pref
      var encodeEntity = prefs.getCharPref("editor.encode_entity");
      switch (encodeEntity) {
        case "basic"  : outputEntity = msiWebPersist.ENCODE_FLAGS_ENCODE_BASIC_ENTITIES; break;
        case "latin1" : outputEntity = msiWebPersist.ENCODE_FLAGS_ENCODE_LATIN1_ENTITIES; break;
        case "html"   : outputEntity = msiWebPersist.ENCODE_FLAGS_ENCODE_HTML_ENTITIES; break;
        case "none"   : outputEntity = 0; break;
      }
    }
    catch (e) {}
  }
  outputFlags |= outputEntity;

  if (aWrapColumn > 0)
    outputFlags |= msiWebPersist.ENCODE_FLAGS_WRAP;

  return outputFlags;
}
//
//// returns number of column where to wrap
//const nsIWebBrowserPersist = Components.interfaces.nsIWebBrowserPersist;
function msiGetWrapColumn(editorelement)
{
  try {
    return msiGetEditor(editorElement).wrapWidth;
  } catch (e) {}
  return 72;
}

function msiGetPromptService()
{
  var promptService;
  try {
    promptService = Components.classes["@mozilla.org/embedcomp/prompt-service;1"].getService();
    promptService = promptService.QueryInterface(Components.interfaces.nsIPromptService);
  }
  catch (e) {}
  return promptService;
}

//const gShowDebugOutputStateChange = false;
//const gShowDebugOutputProgress = false;
//const gShowDebugOutputStatusChange = false;
//
//const gShowDebugOutputLocationChange = false;
//const gShowDebugOutputSecurityChange = false;
//
//const nsIWebProgressListener = Components.interfaces.nsIWebProgressListener;
//const nsIChannel = Components.interfaces.nsIChannel;
//
//const kErrorBindingAborted = 2152398850;
//const kErrorBindingRedirected = 2152398851;
//const kFileNotFound = 2152857618;
//
function msigEditorOutputProgressListener(editorElement)
{
  this.msiEditorElement = editorElement;

  this.onStateChange = function(aWebProgress, aRequest, aStateFlags, aStatus)
  {
    var editor = msiGetEditor(this.msiEditorElement);

    // Use this to access onStateChange flags
    var requestSpec;
    var bDebugOutputStateChange = ("gShowDebugOutputStateChange" in window) && window.gShowDebugOutputStateChange;
    try {
      var channel = aRequest.QueryInterface(nsIChannel);
      requestSpec = StripUsernamePasswordFromURI(channel.URI);
    } catch (e) {
      if ( bDebugOutputStateChange )
        dump("***** onStateChange; NO REQUEST CHANNEL\n");
    }

    var pubSpec;
    if (this.msiEditorElement.mgPublishData)
      pubSpec = this.msiEditorElement.mgPublishData.publishUrl + this.msiEditorElement.mgPublishData.docDir + this.msiEditorElement.mgPublishData.filename;

    if (bDebugOutputStateChange)
    {
      dump("\n***** onStateChange request: " + requestSpec + "\n");
      dump("      state flags: ");

      if (aStateFlags & nsIWebProgressListener.STATE_START)
        dump(" STATE_START, ");
      if (aStateFlags & nsIWebProgressListener.STATE_STOP)
        dump(" STATE_STOP, ");
      if (aStateFlags & nsIWebProgressListener.STATE_IS_NETWORK)
        dump(" STATE_IS_NETWORK ");

      dump("\n * requestSpec="+requestSpec+", pubSpec="+pubSpec+", aStatus="+aStatus+"\n");

      msiDumpDebugStatus(aStatus);
    }
    // The rest only concerns publishing, so bail out if no dialog
    if (!this.msiEditorElement.mgProgressDialog)
      return;

    // Detect start of file upload of any file:
    // (We ignore any START messages after gPersistObj says publishing is finished
    if ((aStateFlags & nsIWebProgressListener.STATE_START)
         && this.msiEditorElement.mPersistObj && requestSpec
         && (this.msiEditorElement.mPersistObj.currentState != this.msiEditorElement.mPersistObj.PERSIST_STATE_FINISHED))
    {
      try {
        // Add url to progress dialog's list showing each file uploading
        this.msiEditorElement.mgProgressDialog.SetProgressStatus(GetFilename(requestSpec), "busy");
      } catch(e) {}
    }

    // Detect end of file upload of any file:
    if (aStateFlags & nsIWebProgressListener.STATE_STOP)
    {
      // ignore aStatus == kErrorBindingAborted; check http response for possible errors
      try {
        // check http channel for response: 200 range is ok; other ranges are not
        var httpChannel = aRequest.QueryInterface(Components.interfaces.nsIHttpChannel);
        var httpResponse = httpChannel.responseStatus;
        if (httpResponse < 200 || httpResponse >= 300)
          aStatus = httpResponse;   // not a real error but enough to pass check below
        else if (aStatus == kErrorBindingAborted)
          aStatus = 0;

        if (bDebugOutputStateChange)
          dump("http response is: "+httpResponse+"\n");
      } 
      catch(e) 
      {
        if (aStatus == kErrorBindingAborted)
          aStatus = 0;
      }

      // We abort publishing for all errors except if image src file is not found
      var abortPublishing = (aStatus != 0 && aStatus != kFileNotFound);

      // Notify progress dialog when we receive the STOP
      //  notification for a file if there was an error 
      //  or a successful finish
      //  (Check requestSpec to be sure message is for destination url)
      if (aStatus != 0 
           || (requestSpec && requestSpec.indexOf(GetScheme(this.msiEditorElement.mgPublishData.publishUrl)) == 0))
      {
        try {
          this.msiEditorElement.mgProgressDialog.SetProgressFinished(GetFilename(requestSpec), aStatus);
        } catch(e) {}
      }


      if (abortPublishing)
      {
        // Cancel publishing
        this.msiEditorElement.mPersistObj.cancelSave();

        // Don't do any commands after failure
        this.msiEditorElement.mgCommandAfterPublishing = null;

        // Restore original document to undo image src url adjustments
        if (this.msiEditorElement.mgRestoreDocumentSource)
        {
          try {
            editor.rebuildDocumentFromSource(this.msiEditorElement.mgRestoreDocumentSource);

            // Clear transaction cache since we just did a potentially 
            //  very large insert and this will eat up memory
            editor.transactionManager.clear();
          }
          catch (e) {}
        }

        // Notify progress dialog that we're finished
        //  and keep open to show error
        this.msiEditorElement.mgProgressDialog.SetProgressFinished(null, 0);

        // We don't want to change location or reset mod count, etc.
        return;
      }

      //XXX HACK: "file://" protocol is not supported in network code
      //    (bug 151867 filed to add this support, bug 151869 filed
      //     to remove this and other code in nsIWebBrowserPersist)
      //    nsIWebBrowserPersist *does* copy the file(s), but we don't 
      //    get normal onStateChange messages.

      // Case 1: If images are included, we get fairly normal
      //    STATE_START/STATE_STOP & STATE_IS_NETWORK messages associated with the image files,
      //    thus we must finish HTML file progress below

      // Case 2: If just HTML file is uploaded, we get STATE_START and STATE_STOP
      //    notification with a null "requestSpec", and 
      //    the gPersistObj is destroyed before we get here!
      //    So create an new object so we can flow through normal processing below
      if (!requestSpec && GetScheme(this.msiEditorElement.mgPublishData.publishUrl) == "file"
          && (!this.msiEditorElement.mPersistObj || this.msiEditorElement.mPersistObj.currentState == nsIWebBrowserPersist.PERSIST_STATE_FINISHED))
      {
        aStateFlags |= nsIWebProgressListener.STATE_IS_NETWORK;
        if (!this.msiEditorElement.mPersistObj)
        {          
          this.msiEditorElement.mPersistObj = new msiPersistObj(aStatus, nsIWebBrowserPersist.PERSIST_STATE_FINISHED);
        }
      }

      // STATE_IS_NETWORK signals end of publishing, as does the gPersistObj.currentState
      if (aStateFlags & nsIWebProgressListener.STATE_IS_NETWORK
          && this.msiEditorElement.mPersistObj.currentState == nsIWebBrowserPersist.PERSIST_STATE_FINISHED)
      {
        if (GetScheme(this.msiEditorElement.mgPublishData.publishUrl) == "file")
        {
          //XXX "file://" hack: We don't get notified about the HTML file, so end progress for it
          // (This covers both "Case 1 and 2" described above)
          this.msiEditorElement.mgProgressDialog.SetProgressFinished(this.msiEditorElement.mgPublishData.filename, this.msiEditorElement.mPersistObj.result);
        }

        if (this.msiEditorElement.mPersistObj.result == 0)
        {
          // All files are finished and publishing succeeded (some images may have failed)
          try {
            // Make a new docURI from the "browse location" in case "publish location" was FTP
            // We need to set document uri before notifying listeners
            var docUrl = msiGetDocUrlFromPublishData(this.msiEditorElement.mgPublishData, this.msiEditorElement);
            SetDocumentURI(GetIOService().newURI(docUrl, editor.documentCharacterSet, null));

            UpdateWindowTitle();

            // this should cause notification to listeners that doc has changed
            editor.resetModificationCount();

            // Set UI based on whether we're editing a remote or local url
            SetSaveAndPublishUI(urlstring);

          } catch (e) {}

          // Save publishData to prefs
          if (this.msiEditorElement.mgPublishData)
          {
            if (this.msiEditorElement.mgPublishData.savePublishData)
            {
              // We published successfully, so we can safely
              //  save docDir and otherDir to prefs
              this.msiEditorElement.mgPublishData.saveDirs = true;
              SavePublishDataToPrefs(this.msiEditorElement.mgPublishData);
            }
            else
              SavePassword(this.msiEditorElement.mgPublishData);
          }

          // Ask progress dialog to close, but it may not
          // if user checked checkbox to keep it open
          this.msiEditorElement.mgProgressDialog.RequestCloseDialog();
        }
        else
        {
          // We previously aborted publishing because of error:
          //   Calling gPersistObj.cancelSave() resulted in a non-zero gPersistObj.result,
          //   so notify progress dialog we're finished
          this.msiEditorElement.mgProgressDialog.SetProgressFinished(null, 0);
        }
      }
    }
  };

  this.onProgressChange = function(aWebProgress, aRequest, aCurSelfProgress,
                              aMaxSelfProgress, aCurTotalProgress, aMaxTotalProgress)
  {
    if (!this.msiEditorElement.mPersistObj)
      return;

    var bDebugOutputProgress = ("gShowDebugOutputProgress" in window) && window.gShowDebugOutputProgress;
    if (bDebugOutputProgress)
    {
      dump("\n onProgressChange: gPersistObj.result="+this.msiEditorElement.mPersistObj.result+"\n");
      try {
      var channel = aRequest.QueryInterface(nsIChannel);
      dump("***** onProgressChange request: " + channel.URI.spec + "\n");
      }
      catch (e) {}
      dump("*****       self:  "+aCurSelfProgress+" / "+aMaxSelfProgress+"\n");
      dump("*****       total: "+aCurTotalProgress+" / "+aMaxTotalProgress+"\n\n");

      if (this.msiEditorElement.mPersistObj.currentState == this.msiEditorElement.mPersistObj.PERSIST_STATE_READY)
        dump(" Persister is ready to save data\n\n");
      else if (this.msiEditorElement.mPersistObj.currentState == this.msiEditorElement.mPersistObj.PERSIST_STATE_SAVING)
        dump(" Persister is saving data.\n\n");
      else if (this.msiEditorElement.mPersistObj.currentState == this.msiEditorElement.mPersistObj.PERSIST_STATE_FINISHED)
        dump(" PERSISTER HAS FINISHED SAVING DATA\n\n\n");
    }
  };

  this.onLocationChange = function(aWebProgress, aRequest, aLocation)
  {
    var bDebugOutputLocationChange = ("gShowDebugOutputLocationChange" in window) && window.gShowDebugOutputLocationChange;
    if (bDebugOutputLocationChange)

    {
      dump("***** onLocationChange: "+aLocation.spec+"\n");
      try {
        var channel = aRequest.QueryInterface(nsIChannel);
        dump("*****          request: " + channel.URI.spec + "\n");
      }
      catch(e) {}
    }
  };

  this.onStatusChange = function(aWebProgress, aRequest, aStatus, aMessage)
  {
    var bDebugOutputStatusChange = ("gShowDebugOutputStatusChange" in window) && window.gShowDebugOutputStatusChange;
    if (bDebugOutputStatusChange)
    {
      dump("***** onStatusChange: "+aMessage+"\n");
      try {
        var channel = aRequest.QueryInterface(nsIChannel);
        dump("*****        request: " + channel.URI.spec + "\n");
      }
      catch (e) { dump("          couldn't get request\n"); }
      
      msiDumpDebugStatus(aStatus);

      if (this.msiEditorElement.mPersistObj)
      {
        if(this.msiEditorElement.mPersistObj.currentState == this.msiEditorElement.mPersistObj.PERSIST_STATE_READY)
          dump(" Persister is ready to save data\n\n");
        else if(this.msiEditorElement.mPersistObj.currentState == this.msiEditorElement.mPersistObj.PERSIST_STATE_SAVING)
          dump(" Persister is saving data.\n\n");
        else if(this.msiEditorElement.mPersistObj.currentState == this.msiEditorElement.mPersistObj.PERSIST_STATE_FINISHED)
          dump(" PERSISTER HAS FINISHED SAVING DATA\n\n\n");
      }
    }
  };

  this.onSecurityChange = function(aWebProgress, aRequest, state)
  {
    var bDebugOutputSecurityChange = ("gShowDebugOutputSecurityChange" in window) && window.gShowDebugOutputSecurityChange;
    if (bDebugOutputSecurityChange)
    {
      try {
        var channel = aRequest.QueryInterface(nsIChannel);
        dump("***** onSecurityChange request: " + channel.URI.spec + "\n");
      } catch (e) {}
    }
  };

  this.QueryInterface = function(aIID)
  {
    if (aIID.equals(Components.interfaces.nsIWebProgressListener)
    || aIID.equals(Components.interfaces.nsISupports)
    || aIID.equals(Components.interfaces.nsISupportsWeakReference)
    || aIID.equals(Components.interfaces.nsIPrompt)
    || aIID.equals(Components.interfaces.nsIAuthPrompt))
      return this;
    throw Components.results.NS_NOINTERFACE;
  };

// nsIPrompt
  this.alert = function(dlgTitle, text)
  {
    AlertWithTitle(dlgTitle, text, this.msiEditorElement.mgProgressDialog ? this.msiEditorElement.mgProgressDialog : window);
  };

  this.alertCheck = function(dialogTitle, text, checkBoxLabel, checkObj)
  {
    AlertWithTitle(dialogTitle, text);
  };

  this.confirm = function(dlgTitle, text)
  {
    return ConfirmWithTitle(dlgTitle, text, null, null);
  };

  this.confirmCheck = function(dlgTitle, text, checkBoxLabel, checkObj)
  {
    var promptServ = msiGetPromptService();
    if (!promptServ)
      return;

    promptServ.confirmEx(window, dlgTitle, text, nsIPromptService.STD_OK_CANCEL_BUTTONS,
                         "", "", "", checkBoxLabel, checkObj);
  };

  this.confirmEx = function(dlgTitle, text, btnFlags, btn0Title, btn1Title, btn2Title, checkBoxLabel, checkVal)
  {
    var promptServ = msiGetPromptService();
    if (!promptServ)
     return 0;

    return promptServ.confirmEx(window, dlgTitle, text, btnFlags,
                        btn0Title, btn1Title, btn2Title,
                        checkBoxLabel, checkVal);
  };

  this.prompt = function(dlgTitle, text, inoutText, checkBoxLabel, checkObj)
  {
    var promptServ = msiGetPromptService();
    if (!promptServ)
     return false;

    return promptServ.prompt(window, dlgTitle, text, inoutText, checkBoxLabel, checkObj);
  };

  this.promptPassword = function(dlgTitle, text, pwObj, checkBoxLabel, savePWObj)
  {

    var promptServ = msiGetPromptService();
    if (!promptServ)
     return false;

    var ret = false;
    try {
      // Note difference with nsIAuthPrompt::promptPassword, which has 
      // just "in" savePassword param, while nsIPrompt is "inout"
      // Initialize with user's previous preference for this site
      if (this.msiEditorElement.mgPublishData)
        savePWObj.value = this.msiEditorElement.mgPublishData.savePassword;

      ret = promptServ.promptPassword(this.msiEditorElement.mgProgressDialog ? this.msiEditorElement.mgProgressDialog : window,
                                      dlgTitle, text, pwObj, checkBoxLabel, savePWObj);

      if (!ret)
        setTimeout(CancelPublishing, 0);

      if (ret && this.msiEditorElement.mgPublishData)
        msiUpdateUsernamePasswordFromPrompt(this.msiEditorElement.mgPublishData, this.msiEditorElement.mgPublishData.username, pwObj.value, savePWObj.value);
    } catch(e) {}

    return ret;
  };

  this.promptUsernameAndPassword = function(dlgTitle, text, userObj, pwObj, checkBoxLabel, savePWObj)
  {
    var ret = msiPromptUsernameAndPassword(dlgTitle, text, savePWObj.value, userObj, pwObj, this.msiEditorElement);
    if (!ret)
      setTimeout(CancelPublishing, 0);

    return ret;
  };

  this.select = function(dlgTitle, text, count, selectList, outSelection)
  {
    var promptServ = msiGetPromptService();
    if (!promptServ)
      return false;

    return promptServ.select(window, dlgTitle, text, count, selectList, outSelection);
  };

// nsIAuthPrompt
  this.prompt = function(dlgTitle, text, pwrealm, savePW, defaultText, result)
  {
    var promptServ = msiGetPromptService();
    if (!promptServ)
      return false;

    var savePWObj = {value:savePW};
    var ret = promptServ.prompt(this.msiEditorElement.mgProgressDialog ? this.msiEditorElement.mgProgressDialog : window,
                                dlgTitle, text, defaultText, pwrealm, savePWObj);
    if (!ret)
      setTimeout(CancelPublishing, 0);
    return ret;
  };

  this.promptUsernameAndPassword = function(dlgTitle, text, pwrealm, savePW, userObj, pwObj)
  {
    var ret = msiPromptUsernameAndPassword(dlgTitle, text, savePW, userObj, pwObj, this.msiEditorElement);
    if (!ret)
      setTimeout(CancelPublishing, 0);
    return ret;
  };

  this.promptPassword = function(dlgTitle, text, pwrealm, savePW, pwObj)
  {
    var ret = false;
    try {
      var promptServ = msiGetPromptService();
      if (!promptServ)
        return false;

      // Note difference with nsIPrompt::promptPassword, which has 
      // "inout" savePassword param, while nsIAuthPrompt is just "in"
      // Also nsIAuth doesn't supply "checkBoxLabel"
      // Initialize with user's previous preference for this site
      var savePWObj = {value:savePW};
      // Initialize with user's previous preference for this site
      if (this.msiEditorElement.mgPublishData)
        savePWObj.value = this.msiEditorElement.mgPublishData.savePassword;

      ret = promptServ.promptPassword(this.msiEditorElement.mgProgressDialog ? this.msiEditorElement.mgProgressDialog : window,
                                      dlgTitle, text, pwObj, GetString("SavePassword"), savePWObj);

      if (!ret)
        setTimeout(CancelPublishing, 0);

      if (ret && this.msiEditorElement.mgPublishData)
        msiUpdateUsernamePasswordFromPrompt(this.msiEditorElement.mgPublishData, this.msiEditorElement.mgPublishData.username, pwObj.value, savePWObj.value);
    } catch(e) {}

    return ret;
  };
}
//
function msiPromptUsernameAndPassword(dlgTitle, text, savePW, userObj, pwObj, editorElement)
{
  // HTTP prompts us twice even if user Cancels from 1st attempt!
  // So never put up dialog if there's no publish data
  if (!editorElement || !editorElement.mgPublishData)
    return false;

  var ret = false;
  try {
    var promptServ = msiGetPromptService();
    if (!promptServ)
      return false;

    var savePWObj = {value:savePW};

    // Initialize with user's previous preference for this site
    if (editorElement.mgPublishData)
    {
      // HTTP put uses this dialog if either username or password is bad,
      //   so prefill username input field with the previous value for modification
      savePWObj.value = editorElement.mgPublishData.savePassword;
      if (!userObj.value)
        userObj.value = editorElement.mgPublishData.username;
    }

    ret = promptServ.promptUsernameAndPassword(editorElement.mgProgressDialog ? editorElement.mgProgressDialog : window, 
                                               dlgTitle, text, userObj, pwObj, 
                                               GetString("SavePassword"), savePWObj);
    if (ret && editorElement.mgPublishData)
      msiUpdateUsernamePasswordFromPrompt(editorElement.mgPublishData, userObj.value, pwObj.value, savePWObj.value);

  } catch (e) {}

  return ret;
}

function msiDumpDebugStatus(aStatus)
{
  // see nsError.h and netCore.h and ftpCore.h

  if (aStatus == kErrorBindingAborted)
    dump("***** status is NS_BINDING_ABORTED\n");
  else if (aStatus == kErrorBindingRedirected)
    dump("***** status is NS_BINDING_REDIRECTED\n");
  else if (aStatus == 2152398859) // in netCore.h 11
    dump("***** status is ALREADY_CONNECTED\n");
  else if (aStatus == 2152398860) // in netCore.h 12
    dump("***** status is NOT_CONNECTED\n");
  else if (aStatus == 2152398861) //  in nsISocketTransportService.idl 13
    dump("***** status is CONNECTION_REFUSED\n");
  else if (aStatus == 2152398862) // in nsISocketTransportService.idl 14
    dump("***** status is NET_TIMEOUT\n");
  else if (aStatus == 2152398863) // in netCore.h 15
    dump("***** status is IN_PROGRESS\n");
  else if (aStatus == 2152398864) // 0x804b0010 in netCore.h 16
    dump("***** status is OFFLINE\n");
  else if (aStatus == 2152398865) // in netCore.h 17
    dump("***** status is NO_CONTENT\n");
  else if (aStatus == 2152398866) // in netCore.h 18
    dump("***** status is UNKNOWN_PROTOCOL\n");
  else if (aStatus == 2152398867) // in netCore.h 19
    dump("***** status is PORT_ACCESS_NOT_ALLOWED\n");
  else if (aStatus == 2152398868) // in nsISocketTransportService.idl 20
    dump("***** status is NET_RESET\n");
  else if (aStatus == 2152398869) // in ftpCore.h 21
    dump("***** status is FTP_LOGIN\n");
  else if (aStatus == 2152398870) // in ftpCore.h 22
    dump("***** status is FTP_CWD\n");
  else if (aStatus == 2152398871) // in ftpCore.h 23
    dump("***** status is FTP_PASV\n");
  else if (aStatus == 2152398872) // in ftpCore.h 24
    dump("***** status is FTP_PWD\n");
  else if (aStatus == 2152857601)
    dump("***** status is UNRECOGNIZED_PATH\n");
  else if (aStatus == 2152857602)
    dump("***** status is UNRESOLABLE SYMLINK\n");
  else if (aStatus == 2152857604)
    dump("***** status is UNKNOWN_TYPE\n");
  else if (aStatus == 2152857605)
    dump("***** status is DESTINATION_NOT_DIR\n");
  else if (aStatus == 2152857606)
    dump("***** status is TARGET_DOES_NOT_EXIST\n");
  else if (aStatus == 2152857608)
    dump("***** status is ALREADY_EXISTS\n");
  else if (aStatus == 2152857609)
    dump("***** status is INVALID_PATH\n");
  else if (aStatus == 2152857610)
    dump("***** status is DISK_FULL\n");
  else if (aStatus == 2152857612)
    dump("***** status is NOT_DIRECTORY\n");
  else if (aStatus == 2152857613)
    dump("***** status is IS_DIRECTORY\n");
  else if (aStatus == 2152857614)
    dump("***** status is IS_LOCKED\n");
  else if (aStatus == 2152857615)
    dump("***** status is TOO_BIG\n");
  else if (aStatus == 2152857616)
    dump("***** status is NO_DEVICE_SPACE\n");
  else if (aStatus == 2152857617)
    dump("***** status is NAME_TOO_LONG\n");
  else if (aStatus == 2152857618) // 80520012
    dump("***** status is FILE_NOT_FOUND\n");
  else if (aStatus == 2152857619)
    dump("***** status is READ_ONLY\n");
  else if (aStatus == 2152857620)
    dump("***** status is DIR_NOT_EMPTY\n");
  else if (aStatus == 2152857621)
    dump("***** status is ACCESS_DENIED\n");
  else if (aStatus == 2152398878)
    dump("***** status is ? (No connection or time out?)\n");
  else
    dump("***** status is " + aStatus + "\n");
}

// Update any data that the user supplied in a prompt dialog
function msiUpdateUsernamePasswordFromPrompt(publishData, username, password, savePassword)
{
  if (!publishData)
    return;
  
  // Set flag to save publish data after publishing if it changed in dialog 
  //  and the "SavePassword" checkbox was checked
  //  or we already had site data for this site
  // (Thus we don't automatically create a site until user brings up Publish As dialog)
  publishData.savePublishData = (gPublishData.username != username || gPublishData.password != password)
                                && (savePassword || !publishData.notInSiteData);

  publishData.username = username;
  publishData.password = password;
  publishData.savePassword = savePassword;
}

const kSupportedTextMimeTypes =
[
  "text/plain",
  "text/css",
  "text/rdf",
  "text/xsl",
  "text/tex",
  "text/javascript",
  "application/x-javascript",
  "text/xul",
  "application/vnd.mozilla.xul+xml"
];

function msiIsSupportedTextMimeType(aMimeType)
{
  for (var i = 0; i < kSupportedTextMimeTypes.length; i++)
  {
    if (kSupportedTextMimeTypes[i] == aMimeType)
      return true;
  }
  return false;
}


// Now that we save documents in a zip file, the save operation has two steps. We first save
// everything that is in memory to the disk in the working directory. Then we replace the *.sci
// or write a new one, update backup files etc. after the first step is completed. Since the first
// step is what we call a soft save, we pull that out as a single procedure.

function msiSoftSave( editor, editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  if (!msiIsTopLevelEditor(editorElement))
    return false;
  //if (!editor) 
    editor = msiGetEditor(editorElement);
  var aMimeType = editor.contentsMIMEType;
  var editorDoc = editor.document;
  if (!editorDoc)
    throw Components.results.NS_ERROR_NOT_INITIALIZED;

  // if we don't have the right editor type bail (we handle text and html)
//  var editorType = editor.editortype;
//  if (editorType != "text" && editorType != "html" 
//      && editorType != "htmlmail" && editorType != "textmail")
//    throw Components.results.NS_ERROR_NOT_IMPLEMENTED;
//
  var saveAsTextFile = msiIsSupportedTextMimeType(aMimeType);
  // check if the file is to be saved is a format we don't understand; if so, bail
  if (aMimeType != "text/html" && aMimeType != "application/xhtml+xml" && aMimeType != "text/xml" && !saveAsTextFile)
    throw Components.results.NS_ERROR_NOT_IMPLEMENTED;

  if (saveAsTextFile)
    aMimeType = "text/plain";
  var urlstring = msiGetEditorURL(editorElement);
  var currentFile = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
  var currFilePath = GetFilepath(urlstring);
  // for Windows
#ifdef XP_WIN32
      currFilePath = currFilePath.replace("/","\\","g");
#endif
  currentFile.initWithPath( currFilePath );
  var success;
  success = msiOutputFileWithPersistAPI(editorDoc, currentFile, null, aMimeType, editorElement);
  if (success) editor.contentsMIMEType = aMimeType;
  return success;
}


    
function deleteWorkingDirectory(editorElement)
{
  var htmlurlstring = msiGetEditorURL(editorElement); 
  if (!htmlurlstring || htmlurlstring.length == 0) return;
  var workingDir = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
  var htmlpath = GetFilepath(htmlurlstring);
// we know we shouldn't delete the directory unless it really is a working directory; i.e., unless it 
// ends with "_work/main.xhtml"
  var regEx = /_work\/main.xhtml$/i;  // BBM: localize this
  if (regEx.test(htmlpath))
  {
    try
    {
#ifdef XP_WIN32
      htmlpath = htmlpath.replace("/","\\","g");
#endif
      workingDir.initWithPath( htmlpath );  
      workingDir = workingDir.parent;
      if (workingDir.exists())
        workingDir.remove(1);
    } catch(exc) { msiDumpWithID("In deleteWorkingDirectory for editorElement [@], trying to delete directory [" + htmlpath + "]; exception is [" + exc + "].\n", editorElement); }
  }
  else alert("Trying to remove 'work directory': "+htmlpath+"\n");
}


// throws an error or returns true if user attempted save; false if user canceled save
//
// Discussion:
// First we do a soft save. Once that is done, all the necessary data is on the dist, in a directory we call D.
// The original file is A.sci.
// There may be a previously created file A.bak.
//
// If we are doing SaveAs or SaveCopy (SaveAs is alway true when SaveCopy is true), we bring up the 
// PromptForSaveLocation dialog box. Assume the filename returned is B.sci. We also assume that if B.sci
// exists, the user has already given permission to overwrite it. If we are doing a straight save, B=A.
//
// We do the following:
//
// Save the directory D to a zipfile called B.tempsci.
//
// If successful:
//   If A==B (a straight save), rename A.bak to A.tempbak, rename A.sci to A.bak, rename A.tempsci to A.sci.
//   If all is successful, delete A.tempbak 
//   We will now have A.sci, A.bak.
//   Now delete directory D.
//
//   If A!=B (a save-as), delete B.bak and B.sci if they exist. Rename B.tempsci to B.sci.
//   BBM - addendum. I decided not to delete B.bak
//   Delete directory D unless we are returning to editing.
//

function msiSaveDocument(aContinueEditing, aSaveAs, aSaveCopy, aMimeType, editor, editorElement)
{
  var success =  msiSoftSave( editor, editorElement);
  if (!success) 
    throw Components.results.NS_ERROR_UNEXPECTED;

  // The making of A.sci:
  // Say the file being edited is /home/joe/SWPDocs/untitled1_work/main.xhtml
  // or maybe                     C:/SWPDocs/untitled1_work/main.html
  // Then
  // htmlurlstring      = file:///home/joe/SWPDocs/untitled1_work/main.xhtml
  // or                   file://C:/SWPDocs/untitled1_work/main.xhtml
  // sciurlstring       = file:///home/joe/SWPDocs/untitled1.sci
  // or                   file://C:/SWPDocs/untitled1.sci
  // htmlpath           = /home/joe/SWPDocs/untitled1_work/main.xhtml
  // or                   C:/SWPDocs/untitled1_work/main.xhtml
  // currentSciFilePath = /home/joe/SWPDocs/untitled1.sci
  // or                   C:/SWPDocs/untitled1.sci
  var htmlurlstring = msiGetEditorURL(editorElement); // this is the url of the file in the directory D. It was updated by the soft save.
  var sciurlstring = msiFindOriginalDocname(htmlurlstring); // this is the uri of A.sci
  var mustShowFileDialog = (aSaveAs || aSaveCopy || IsUrlUntitled(sciurlstring) || (sciurlstring == ""));

  // If editing a remote URL, force SaveAs dialog
  if (!mustShowFileDialog && GetScheme(sciurlstring) != "file" && GetScheme(sciurlstring) != "resource")
  {
    mustShowFileDialog = true;
  }
  var saveAsTextFile = msiIsSupportedTextMimeType(aMimeType);
  var replacing = !aSaveAs;
  var titleChanged = false;
  var doUpdateURI = false;
  var destLocalFile = null;
  var destURI;
  var currentFile = null;
  var currentSciFile = null;
  var workingDir = null;
  var leafname;
  var isSciFile;
  
  var workingDir = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
  currentSciFile = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
  var htmlpath = GetFilepath(htmlurlstring);
  var currentSciFilePath = GetFilepath(sciurlstring);  // the path for A.sci
  var regEx = /_work\/main.xhtml$/i;  // BBM: localize this
  isSciFile = regEx.test(htmlpath);
// for Windows
#ifdef XP_WIN32
  htmlpath = htmlpath.replace("/","\\","g");
  currentSciFilePath = currentSciFilePath.replace("/","\\","g");
#endif
  currentSciFile.initWithPath( currentSciFilePath );  // now = A.sci
  if (isSciFile) 
  {
    workingDir.initWithPath( htmlpath );  // now = the path of the xhtml file in the working dir D
    workingDir = workingDir.parent;       // now = the directory D
  }

  if (mustShowFileDialog)
  {
    var urlstring;
    try {
      // Prompt for title if we are saving to .sci
      if (!saveAsTextFile && (editor.editortype == "html"))
      {
        var userContinuing = msiPromptAndSetTitleIfNone(editorElement); // not cancel
        if (!userContinuing)
          return false;
      }

      var dialogResult = msiPromptForSaveLocation(saveAsTextFile, editor.editortype=="html"?MSI_EXTENSION:editor.editortype, 
        aMimeType, sciurlstring, editorElement);
      if (dialogResult.filepickerClick == msIFilePicker.returnCancel)
        return false;

      replacing = (dialogResult.filepickerClick == msIFilePicker.returnReplace);
      urlstring = dialogResult.resultingURIString;

      // jcs without .clone() this always set destLocalFile void
      destLocalFile = dialogResult.resultingLocalFile.clone();  // this is B.sci
      // update the new URL for the webshell unless we are saving a copy
      if (!aSaveCopy)
        doUpdateURI = true;
    } catch (e) {
       return false; 
    }
    var ioService;
    try {
      // if somehow we didn't get a local file but we did get a uri, 
      // attempt to create the localfile if it's a "file" url
      var docURI;

      if (!Boolean(destLocalFile) )
      {
        ioService = msiGetIOService();
        docURI = ioService.newURI(urlstring, editor.documentCharacterSet, null);
    
        if (docURI.schemeIs("file"))
        {
          var fileHandler = msiGetFileProtocolHandler();
          destLocalFile = fileHandler.getFileFromURLSpec(urlstring).QueryInterface(Components.interfaces.nsILocalFile);
        }
      }
    }
    catch (e)
    {
      success = false;
    }
  } // mustShowDialog
  if (!success)
  { 
    var saveDocStr = GetString("SaveDocument");
    var failedStr = GetString("SaveFileFailed");
    AlertWithTitle(saveDocStr, failedStr);
    throw Components.results.NS_ERROR_UNEXPECTED;
  }

  // now get the leaf name

  // jcs What is this supposed to do? If "replacing" just means that the
  // jcs save target already exists
  // jcs if (replacing)
  // jcs {
  // jcs   currentSciFile.initWithPath( currentSciFilePath );  // now = A.sci
  // jcs   destLocalFile = currentSciFile;       // clone???
  // jcs }

  leafname = destLocalFile.leafName;
  if (leafname.lastIndexOf(".") > 0)
  {
    leafname = leafname.slice(0, leafname.lastIndexOf("."));
  }
  if (isSciFile)
  {
    var zipfile = destLocalFile.parent.clone();
    zipfile.append(leafname+".tempsci"); 

  // zip D into the zipfile
    try {
      var zw = Components.classes["@mozilla.org/zipwriter;1"]
                            .createInstance(Components.interfaces.nsIZipWriter);
      if (zipfile.exists()) zipfile.remove(0);
      zipfile.create(0,0755);
      zw.open( zipfile, PR_RDWR | PR_CREATE_FILE | PR_TRUNCATE);
      zipDirectory(zw, "", workingDir); 
      zw.close();
    }
    catch(e) {
      throw Components.results.NS_ERROR_UNEXPECTED;
    }
// If successful:
//   If A==B (a straight save), rename A.bak to A.tempbak, rename A.sci to A.bak, rename A.tempsci to A.sci.
//   If all is successful, delete A.tempbak 
//   We will now have A.sci, A.bak.
//   Now delete directory D.
//
//   If A!=B (a save-as), B.sci if it exists. Rename B.tempsci to B.sci.
//   Delete directory D.
//
    var tempfile;
    if (replacing)
    {
      tempfile = zipfile.clone();
        // rename A.tbak to A.tempbak
      tempfile = tempfile.parent;
      tempfile.append(leafname+".bak");
      if (tempfile.exists()) tempfile.moveTo(null, leafname+".tempbak");
        // rename A.sci to A.bak
      tempfile = tempfile.parent;
      tempfile.append(leafname+".sci");
      if (tempfile.exists()) tempfile.moveTo(null, leafname+".bak");
        // rename A.tempsci to A.sci
      zipfile.moveTo(null, leafname+".sci");
        // delete A.tempbak
      tempfile = tempfile.parent;
      tempfile.append(leafname+".tempbak");
      if (tempfile.exists()) tempfile.remove(0);
    }
    else
    {
        // delete B.bak
      tempfile = zipfile.clone();
  //    tempfile = tempfile.parent;
  //    tempfile.append(leafname+".bak");
  //    if (tempfile.exists()) tempfile.remove(0);
        // delete B.sci
      tempfile = tempfile.parent;
      tempfile.append(leafname+".sci");
      if (tempfile.exists()) tempfile.remove(0);
        // rename B.tempsci to B.sci
      zipfile.moveTo(null, leafname+".sci");
    }
    if (!aContinueEditing) workingDir.remove(1);
    else
    {
      // if the editorElement did have a shell file, it doesn't any longer
      editorElement.isShellFile = false;
      if (doUpdateURI)
      {

        var newWorkingDir = workingDir.clone();
        newWorkingDir = newWorkingDir.parent;
        newWorkingDir.append(leafname+"_work");

        if (newWorkingDir.exists())
           newWorkingDir.remove(true); // recursive delete
        

        workingDir.moveTo(null, leafname+"_work");
         
        var newMainfile = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
        newMainfile.initWithPath(destLocalFile.path.replace(".sci","")+"_work");
        newMainfile.append("main.xhtml");
        //Create a new uri from nsILocalFile
        var newURI = msiGetFileProtocolHandler().newFileURI(newMainfile);

        // We need to set new document uri before notifying listeners
        SetDocumentURI(newURI);
        document.getElementById("filename").value = leafname;
      }
    }
  }
  else
  { // we are not saving a .sci file. The file has already been saved (by SoftSave). Copy it to the destination.
    var destDir; // = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
    destDir = destLocalFile.parent.clone();
    currentSciFile.copyTo(destDir, destLocalFile.leafName);
    // this still needs work: In the SaveAs and SaveCopyAs cases, we don't want to change the orginal file. We need to copy before
    // we do the SoftSave.
    

  }

  UpdateWindowTitle();

  if (!aSaveCopy)
    editor.resetModificationCount();
  // this should cause notification to listeners that document has changed

  // Set UI based on whether we're editing a remote or local url
  if (!aSaveCopy)
    msiSetSaveAndPublishUI(sciurlstring, editorElement);
  return true;
}








function SetDocumentURI(uri)
{
  try {
    var editorElement = msiGetActiveEditorElement();
    if (!msiIsTopLevelEditor(editorElement)) return;
//  not sure if we want this:
//  uri = msiFindOriginalDocname(uri);
    editorElement.docShell.setCurrentURI(uri);
  } catch (e) { dump("SetDocumentURI:\n"+e +"\n"); }
}


////-------------------------------  Publishing
//var gPublishData;
//var gProgressDialog;
//var gCommandAfterPublishing = null;
//var gRestoreDocumentSource;
//
function msiPublish(publishData, editorElement)
{
  if (!publishData)
    return false;
  if (!editorElement)
    editorElement = msiGetTopLevelEditorElement();
  if (!editorElement)
    return false;

  // Set data in global for username password requests
  //  and to do "post saving" actions after monitoring nsIWebProgressListener messages
  //  and we are sure file transfer was successful
  editorElement.mgPublishData = publishData;

  editorElement.mgPublishData.docURI = msiCreateURIFromPublishData(publishData, true, editorElement);
  if (!editorElement.mgPublishData.docURI)
  {
    AlertWithTitle(GetString("Publish"), GetString("PublishFailed"));
    return false;
  }

  if (editorElement.mgPublishData.publishOtherFiles)
    editorElement.mgPublishData.otherFilesURI = msiCreateURIFromPublishData(publishData, false, editorElement);
  else
    editorElement.mgPublishData.otherFilesURI = null;

  if (gShowDebugOutputStateChange)
  {
    dump("\n *** publishData: PublishUrl="+publishData.publishUrl+", BrowseUrl="+publishData.browseUrl+
      ", Username="+publishData.username+", Dir="+publishData.docDir+
      ", Filename="+publishData.filename+"\n");
    dump(" * gPublishData.docURI.spec w/o pass="+StripPassword(editorElement.mgPublishData.docURI.spec)+", PublishOtherFiles="+editorElement.mgPublishData.publishOtherFiles+"\n");
  }

  // XXX Missing username will make FTP fail 
  // and it won't call us for prompt dialog (bug 132320)
  // (It does prompt if just password is missing)
  // So we should do the prompt ourselves before trying to publish
  if (GetScheme(publishData.publishUrl) == "ftp" && !publishData.username)
  {
    var message = GetString("PromptFTPUsernamePassword").replace(/%host%/, GetHost(publishData.publishUrl));
    var savePWobj = {value:publishData.savePassword};
    var userObj = {value:publishData.username};
    var pwObj = {value:publishData.password};
    if (!PromptUsernameAndPassword(GetString("Prompt"), message, savePWobj, userObj, pwObj))
      return false; // User canceled out of dialog

    // Reset data in URI objects
    editorElement.mgPublishData.docURI.username = publishData.username;
    editorElement.mgPublishData.docURI.password = publishData.password;

    if (editorElement.mgPublishData.otherFilesURI)
    {
      editorElement.mgPublishData.otherFilesURI.username = publishData.username;
      editorElement.mgPublishData.otherFilesURI.password = publishData.password;
    }
  }

  try {
    // We launch dialog as a dependent 
    // Don't allow editing document!
    msiSetDocumentEditable(false, editorElement);

    // Start progress monitoring
    editorElement.mgProgressDialog =
      window.openDialog("chrome://prince/content/msiEditorPublishProgress.xul", "_blank",
                        "chrome,dependent,titlebar", editorElement.mgPublishData, editorElement.mPersistObj);

  } catch (e) {}

  // Network transfer is often too quick for the progress dialog to be initialized
  //  and we can completely miss messages for quickly-terminated bad URLs,
  //  so we can't call OutputFileWithPersistAPI right away.
  // msiStartPublishing() is called at the end of the dialog's onload method
  return true;
}

function msiStartPublishing(editorElement)
{
  var editor = msiGetEditor(editorElement);
  if (editor && editorElement && editorElement.mgPublishData && editorElement.mgPublishData.docURI && editorElement.mgProgressDialog)
  {
    editorElement.mgRestoreDocumentSource = null;

    // Save backup document since nsIWebBrowserPersist changes image src urls
    // but we only need to do this if publishing images and other related files
    if (editorElement.mgPublishData.otherFilesURI)
    {
      try {
        // (256 = Output encoded entities)
        editorElement.mgRestoreDocumentSource = 
          editor.outputToString(editor.contentsMIMEType, 256);
      } catch (e) {}
    }

    OutputFileWithPersistAPI(editor.document, 
                             editorElement.mgPublishData.docURI, editorElement.mgPublishData.otherFilesURI, 
                             editor.contentsMIMEType);
    return editorElement.mPersistObj;
  }
  return null;
}

function msiCancelPublishing(editorElement)
{
  try {
    editorElement.mgPersistObj.cancelSave();
    editorElement.mgProgressDialog.SetProgressStatusCancel();
  } catch (e) {}

  // If canceling publishing do not do any commands after this    
  editorElement.mgCommandAfterPublishing = null;

  if (editorElement.mgProgressDialog)
  {
    // Close Progress dialog 
    // (this will call FinishPublishing())
    editorElement.mgProgressDialog.CloseDialog();
  }
  else
    msiFinishPublishing(editorElement);
}

function msiFinishPublishing(editorElement)
{
  msiSetDocumentEditable(true, editorElement);
  editorElement.mgProgressDialog = null;
  editorElement.mgPublishData = null;
  editorElement.mgRestoreDocumentSource = null;

  if (editorElement.mgCommandAfterPublishing)
  {
    // Be sure to null out the global now incase of trouble when executing command
    var command = editorElement.mgCommandAfterPublishing;
    editorElement.mgCommandAfterPublishing = null;
    msiGoDoCommand(command, editorElement);
  }
}

// Create a nsIURI object filled in with all required publishing info
function msiCreateURIFromPublishData(publishData, doDocUri, editorElement)
{
  if (!publishData || !publishData.publishUrl)
    return null;

  var URI;
  try {
    var spec = publishData.publishUrl;
    if (doDocUri)
      spec += FormatDirForPublishing(publishData.docDir) + publishData.filename; 
    else
      spec += FormatDirForPublishing(publishData.otherDir);

    var ioService = msiGetIOService();
    URI = ioService.newURI(spec, msiGetEditor(editorElement).documentCharacterSet, null);

    if (publishData.username)
      URI.username = publishData.username;
    if (publishData.password)
      URI.password = publishData.password;
  }
  catch (e) {}

  return URI;
}

// Resolve the correct "http:" document URL when publishing via ftp
function msiGetDocUrlFromPublishData(publishData, editorElement)
{
  if (!publishData || !publishData.filename || !publishData.publishUrl)
    return "";

  // If user was previously editing an "ftp" url, then keep that as the new scheme
  var url;
  var docScheme = GetScheme(msiGetEditorURL(editorElement));

  // Always use the "HTTP" address if available
  // XXX Should we do some more validation here for bad urls???
  // Let's at least check for a scheme!
  if (!GetScheme(publishData.browseUrl))
    url = publishData.publishUrl;
  else
    url = publishData.browseUrl;

  url += FormatDirForPublishing(publishData.docDir) + publishData.filename;

  if (GetScheme(url) == "ftp")
    url = InsertUsernameIntoUrl(url, publishData.username);

  return url;
}

function msiSetSaveAndPublishUI(urlstring, editorElement)
{
  // Be sure enabled state of toolbar buttons are correct
  msiGoUpdateCommand("cmd_save", editorElement);
  msiGoUpdateCommand("cmd_publish", editorElement);
}

function msiSetDocumentEditable(isDocEditable, editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  if (editor && editor.document)
  {
    try {
      var flags = editor.flags;
      editor.flags = isDocEditable ?  
            flags &= ~nsIPlaintextEditor.eEditorReadonlyMask :
            flags | nsIPlaintextEditor.eEditorReadonlyMask;
    } catch(e) {}

    // update all commands
    window.updateCommands("create");
  }  
}

// ****** end of save / publish **********//

//Below are the MSI versions of command controllers for Composer commands. These are to be attached as the controllers
//belonging to editor elements within dialogs (or in other "subordinate" locations).
//-----------------------------------------------------------------------------------

//A number of these commands are really most suitable for use from primary editors. Typically, the MSI controllers will simply
//return "false" from isCommandEnabled and defer to the usual ("ns____" in lieu of "msi_____") controllers otherwise.

//-----------------------------------------------------------------------------------

//This is the "Publishing Site Settings..." item. Presumably NOT needed by any subordinate editor element.
var msiPublishSettingsCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    if (!msiIsTopLevelEditor(editorElement))
      return false;

    if (msiIsDocumentEditable(editorElement))
    {
      // Always allow publishing when editing a local document,
      //  otherwise the document modified state would prevent that
      //  when you first open any local file.
      try {
        var docUrl = msiGetEditorURL(editorElement);
        return msiIsDocumentModified(editorElement) || msiIsHTMLSourceChanged(editorElement)
               || IsUrlAboutBlank(docUrl) || IsUrlUntitled(docUrl) || GetScheme(docUrl) == "file";
      } catch (e) {return false;}
    }
    return false;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetTopLevelEditorElement();
    if (msiGetEditor(editorElement))
    {
      var docUrl = msiGetEditorURL(editorElement);
      var filename = GetFilename(docUrl);
      var publishData;
      var showPublishDialog = false;

      // First check pref to always show publish dialog
      try {
        var prefs = GetPrefs();
        if (prefs)
          showPublishDialog = prefs.getBoolPref("editor.always_show_publish_dialog");
      } catch(e) {}

      if (!showPublishDialog && filename)
      {
        // Try to get publish data from the document url
        publishData = CreatePublishDataFromUrl(docUrl);

        // If none, use default publishing site? Need a pref for this
        //if (!publishData)
        //  publishData = GetPublishDataFromSiteName(GetDefaultPublishSiteName(), filename);
      }

      if (showPublishDialog || !publishData)
      {
        // Show the publish dialog
        publishData = {};
        publishData.msiEditorParent = editorElement;
        window.ok = false;
        var oldTitle = msiGetDocumentTitle(editorElement);
        window.openDialog("chrome://editor/content/EditorPublish.xul","_blank", 
                          "chrome,close,titlebar,modal", "", "", publishData);
        if (msiGetDocumentTitle(editorElement) != oldTitle)
          UpdateWindowTitle();

        editorElement.focus();
        if (!window.ok)
          return false;
      }
      if (publishData)
      {
        msiFinishHTMLSource(editorElement);
        return msiPublish(publishData, editorElement);
      }
    }
    return false;
  }
}

//-----------------------------------------------------------------------------------

// The Revert command replaces the current files with the .bak versions. It should be disabled if there are no .bak files.
var msiRevertCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    var url = msiGetEditorURL(editorElement);
    return (msiIsDocumentEditable(editorElement) && msiIsDocumentModified(editorElement)
              && !IsUrlUntitled(url) && !IsUrlAboutBlank(url));
//    return (IsDocumentEditable() &&
//            IsDocumentModified() &&
//            !IsUrlAboutBlank(GetDocumentUrl()));
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    // Confirm with the user to abandon current changes
    var promptService = msiGetPromptService();
    if (promptService)
    {
      // Put the page title in the message string
      var editorElement = msiGetActiveEditorElement();
      var title = msiGetDocumentTitle(editorElement);
      if (!title)
        title = GetString("untitled");

      var msg = GetString("AbandonChanges").replace(/%title%/,title);

      var result = promptService.confirmEx(window, GetString("RevertCaption"), msg,
  						      (promptService.BUTTON_TITLE_REVERT * promptService.BUTTON_POS_0) +
  						      (promptService.BUTTON_TITLE_CANCEL * promptService.BUTTON_POS_1),
  						      null, null, null, null, {value:0});

      // Reload page if first button (Revert) was pressed
      if(result == 0)
      {
        msiCancelHTMLSource(editorElement);
        var urlstring = msiGetEditorURL(editorElement);
        var documentfile = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
        var currFilePath = GetFilepath(urlstring);
        var scifilepath = msiFindOriginalDocname(currFilePath);
        var scifile = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
        scifile.initWithPath(scifilepath);
// for Windows
#ifdef XP_WIN32
      currFilePath = currFilePath.replace("/","\\","g");
#endif
        documentfile.initWithPath( currFilePath );
        msiRevertFile( true, documentfile, false );
        createWorkingDirectory(scifile);
        msiEditorLoadUrl(editorElement, msiGetEditorURL(editorElement));
      }
    }
  }
};

//-----------------------------------------------------------------------------------
//We probably don't want to allow alternate editors (typically attached to controls in a dialog) to perform this
//command (which is "Close Window"). Otherwise we defer to nsCloseCommand?
var msiCloseCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    if (!editorElement|| !msiIsTopLevelEditor(editorElement))
      return false;
    return true;
  },
  
  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    msiCloseWindow();
  }
};

function msiCloseWindow(theWindow)
{
  if (!theWindow)
    theWindow = window;
  // Check to make sure document is saved. "true" means allow "Don't Save" button,
  //   so user can choose to close without saving
  var editorElement = msiGetPrimaryEditorElementForWindow(theWindow);
  if (msiCheckAndSaveDocument(editorElement, "cmd_close", true))
//  if (CheckAndSaveDocument("cmd_close", true)) 
  {
//    if (window.InsertCharWindow)
//      SwitchInsertCharToAnotherEditorOrClose();
//Ought to do what here? We'll assume that any dialog dependent on this window will be dealt with by our dialog management
//code, and not worry it.  rwa
    ShutdownAnEditor(editorElement);
    try {
      var basewin = theWindow.QueryInterface(Components.interfaces.nsIInterfaceRequestor)
                      .getInterface(Components.interfaces.nsIWebNavigation)
                      .QueryInterface(Components.interfaces.nsIDocShellTreeItem)
                      .treeOwner
                      .QueryInterface(Components.interfaces.nsIInterfaceRequestor)
                      .getInterface(Components.interfaces.nsIBaseWindow);
      basewin.destroy();
    } catch (e) {}
  }
}

//-----------------------------------------------------------------------------------
//While it isn't clear that we'll never want this available in an alternate editor, there's no need for a
//version of it different from nsOpenRemoteCommand. For now, we'll just have
var msiOpenRemoteCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    if (!editorElement || !msiIsTopLevelEditor(editorElement))
      return false;
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    /* The last parameter is the current browser window.
       Use 0 and the default checkbox will be to load into an editor
       and loading into existing browser option is removed
     */
    window.openDialog( "chrome://prince/content/openLocation.xul", "_blank", "chrome,modal,titlebar", 0);
    window.content.focus();
  }
};

////-----------------------------------------------------------------------------------
//This command may be assumed to be useful only from a primary editor. If sufficient reason should arise to change this,
//the version below isn't ready for prime time; a version using the correct editor would be needed.
var msiPreviewCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    if (!editorElement || !msiIsTopLevelEditor(editorElement))
      return false;
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement();
    // Don't continue if user canceled during prompt for saving
    // DocumentHasBeenSaved will test if we have a URL and suppress "Don't Save" button if not
    if (!CheckAndSaveDocument("cmd_preview", DocumentHasBeenSaved()))
      return;

    // Check if we saved again just in case?
    if (DocumentHasBeenSaved())
    {
      var browser;
      try {
        // Find a browser with this URL
        var windowManager = Components.classes["@mozilla.org/appshell/window-mediator;1"].getService();
        var windowManagerInterface = windowManager.QueryInterface(Components.interfaces.nsIWindowMediator);
        var enumerator = windowManagerInterface.getEnumerator("navigator:browser");

        var documentURI = msiGetEditorURL(editorElement);
        while ( enumerator.hasMoreElements() )
        {
          browser = enumerator.getNext().QueryInterface(Components.interfaces.nsIDOMWindowInternal);
          if ( browser && (documentURI == browser.getBrowser().currentURI.spec))
            break;

          browser = null;
        }
      }
      catch (ex) {}

      // If none found, open a new browser
      if (!browser)
      {
        browser = window.openDialog(getBrowserURL(), "_blank", "chrome,all,dialog=no", documentURI);
      }
      else
      {
        try {
          browser.BrowserReloadSkipCache();
          browser.focus();
        } catch (ex) {}
      }
    }
  }
};

//-----------------------------------------------------------------------------------
//Again, presumably not needed by a subordinate editor.

var msiSendPageCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    if (!editorElement || !msiIsTopLevelEditor(editorElement))
      return false;
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement();
    // Don't continue if user canceled during prompt for saving
    // DocumentHasBeenSaved will test if we have a URL and suppress "Don't Save" button if not
    if (!CheckAndSaveDocument("cmd_editSendPage", DocumentHasBeenSaved()))
      return;

    // Check if we saved again just in case?
    if (DocumentHasBeenSaved())
    {
      // Launch Messenger Composer window with current page as contents
      try
      {
        openComposeWindow(msiGetEditorURL(editorElement), msiGetDocumentTitle(editorElement));
      } catch (ex) { dump("Cannot Send Page: " + ex + "\n"); }
    }
  }
};

//-----------------------------------------------------------------------------------
//There may be cases where a subordinate editor would want to print, but for now assume not.
var msiDirectPrintCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {

    var editorElement = msiGetActiveEditorElement();
    if (!editorElement || !msiIsTopLevelEditor(editorElement))
      return false;
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    // In editor.js
    msiFinishHTMLSource();
    try {
      NSPrint();
    } catch (e) {}
  }
};

//-----------------------------------------------------------------------------------
var msiPrintCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon)
  {
    
  },
  doCommandParams: function(aCommand, aParams, aRefCon) 
  {
  },
  doCommand: function(aCommand)
  {
    printTeX(aCommand=='cmd_printPdf',false);  
  }
};

//-----------------------------------------------------------------------------------
var msiPreviewCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon)
  {
    
  },
  doCommandParams: function(aCommand, aParams, aRefCon) 
  {
  },
  doCommand: function(aCommand)
  {
    printTeX(aCommand=='cmd_previewPdf',true);  
  }
};

//-----------------------------------------------------------------------------------
var msiCompileCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;  // BBM todo: doesn't this depend on the save state?   
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon)
  {
    
  },
  doCommandParams: function(aCommand, aParams, aRefCon) 
  {
    
  },
  doCommand: function(aCommand)
  {
     compileTeX(aCommand=='cmd_compilePdf')
  }
};


//-----------------------------------------------------------------------------------
//The status of the PrintSetupCommand is the same as the print command.

var msiPrintSetupCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();

    if (!editorElement || !msiIsTopLevelEditor(editorElement))
      return false;
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    // In editor.js
    msiFinishHTMLSource();
    NSPrintSetup();
  }
};

//-----------------------------------------------------------------------------------
var msiOneShotGreek =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon)
  {
    
  },
  doCommandParams: function(aCommand, aParams, aRefCon) 
  {
  },
  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement();
    var editor = msiGetEditor(editorElement);
//    var htmleditor = editor.QueryInterface(Components.interfaces.nsIHTMLEditor);
//    htmleditor
    editor.setOneShotTranslation("greek");  
  }
};

//-----------------------------------------------------------------------------------
var msiOneShotSymbol =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon)
  {
    
  },
  doCommandParams: function(aCommand, aParams, aRefCon) 
  {
  },
  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement();
    var editor = msiGetEditor(editorElement);
    var htmleditor = editor.QueryInterface(Components.interfaces.nsIHTMLEditor);
    htmleditor.setOneShotTranslation("symbol");  
  }
};



//-----------------------------------------------------------------------------------
//msiQuitCommand not even implemented - the comments in the original nsQuitCommand remain relevant, and it's left alone.
var nsQuitCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;    // we can always do this
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {}

  /* The doCommand is not used, since cmd_quit's oncommand="goQuitApplication()" in platformCommunicatorOverlay.xul
  doCommand: function(aCommand)
  {
    // In editor.js
    FinishHTMLSource();
    goQuitApplication();
  }
  */
};

//-----------------------------------------------------------------------------------
var msiAutoSubDlgCommand = 
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
//    AlertWithTitle("Unimplemented", "AutoSubstitution dialog not yet available.");
    var editorElement = msiGetActiveEditorElement();
    try {
      msiOpenModelessDialog("chrome://prince/content/autoSubstituteDialog.xul", "_blank", "chrome,close,titlebar,dependent",
                                        editorElement, "cmd_MSIAutoSubDlg", this, editorElement);
//      window.openDialog("chrome://editor/content/EdReplace.xul", "_blank",
//                        "chrome,modal,titlebar", editorElement);
    }
    catch(ex) {
      dump("*** Exception: couldn't open AutoSubstitution Dialog: " + ex + "\n");
    }
  }
};

//-----------------------------------------------------------------------------------
//Now begin a series of commands which should certainly be available within other editors. We use the fact that
//the last parameter to each is the "editorElement" which registered the command controller. However, some of these
//are ready to be used as they are.

//-----------------------------------------------------------------------------------
var msiFindCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    if (!editorElement)
      return false;
    try
    {
      return editorElement.getEditor(editorElement.contentWindow) != null;
    } catch(exc) {dump("Error in msiFindCommand.isCommandEnabled: " + exc);}
    return false;
  },

  getCommandStateParams: function(aCommand, aParams, editorElement) {},
  doCommandParams: function(aCommand, aParams, editorElement) {},

  doCommand: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    try {
      msiOpenModelessDialog("chrome://prince/content/msiEdReplace.xul", "_blank", "chrome,close,titlebar,dependent,resizable",
                                        editorElement, "cmd_find", this, editorElement);
//      window.openDialog("chrome://editor/content/EdReplace.xul", "_blank",
//                        "chrome,modal,titlebar", editorElement);
    }
    catch(ex) {
      dump("*** Exception: couldn't open Replace Dialog: " + ex + "\n");
    }
    //window.content.focus();
  }
};

//-----------------------------------------------------------------------------------
//In this case we probably want to revise the command handling to use our own find functionality. For now, leave it alone.
var msiFindAgainCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    // we can only do this if the search pattern is non-empty. Not sure how
    // to get that from here
    var editorElement = msiGetActiveEditorElement();
    if (!editorElement)
      return false;
    try
    {
      return editorElement.getEditor(editorElement.contentWindow) != null;
    }
    catch(exc) {dump("Error in msiFindAgainCommand.isCommandEnabled: " + exc);}
    return false;
  },

  getCommandStateParams: function(aCommand, aParams, editorElement) {},
  doCommandParams: function(aCommand, aParams, editorElement) {},

  doCommand: function(aCommand, dummy)
  {
    try {
      var editorElement = msiGetActiveEditorElement();
      var findPrev = aCommand == "cmd_findPrev";
      var findInst = editorElement.webBrowserFind;
      var findService = Components.classes["@mozilla.org/find/find_service;1"]
                                  .getService(Components.interfaces.nsIFindService);
      findInst.findBackwards = findService.findBackwards ^ findPrev;
      findInst.findNext();
      // reset to what it was in dialog, otherwise dialog setting can get reversed
      findInst.findBackwards = findService.findBackwards; 
    }
    catch(exc) {dump("Error in msiFindAgainCommand.doCommand: " + exc);}
  }
};

//-----------------------------------------------------------------------------------
//This is a mail command - we'll asssume for now NOT wanted in subordinate editors.
var msiRewrapCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    if (!editorElement || !msiIsTopLevelEditor(editorElement))
      return false;
//    return true;
    return (msiIsDocumentEditable(editorElement) && !msiIsInHTMLSourceMode(editorElement) &&
            msiGetEditor(editorElement) instanceof Components.interfaces.nsIEditorMailSupport);
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    msiGetCurrentEditor().QueryInterface(Components.interfaces.nsIEditorMailSupport).rewrap(false);
  }
};

//-----------------------------------------------------------------------------------

var msiSpellingCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    return (msiIsDocumentEditable(editorElement) && 
              !msiIsInHTMLSourceMode(editorElement) && msiIsSpellCheckerInstalled());
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    window.cancelSendMessage = false;
    try {
      window.openDialog("chrome://prince/content/EdSpellCheck.xul", "_blank",
              "chrome,close,titlebar,modal", false, false, true, editorElement);
    }
    catch(ex) {}
    editorElement.focus();
  }
};

//Not sure what to do about this one. For the nonce, we'll assume validation won't take place in subordinate editors.
// Validate using http://validator.w3.org/file-upload.html
var URL2Validate;
var msiValidateCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    if (!editorElement || !msiIsTopLevelEditor(editorElement))
      return false;
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    // If the document hasn't been modified,
    // then just validate the current url.
    var editorElement = msiGetActiveEditorElement();
    if (msiIsDocumentModified(editorElement) || msiIsHTMLSourceChanged(editorElement))
    {
      if (!msiCheckAndSaveDocument(editorElement, "cmd_validate", false))
        return;

      // Check if we saved again just in case?
      if (!msiDocumentHasBeenSaved(editorElement))    // user hit cancel?
        return;
    }

    URL2Validate = msiGetEditorURL(editorElement);
    // See if it's a file:
    var ifile;
    try {
      var fileHandler = msiGetFileProtocolHandler();
      ifile = fileHandler.getFileFromURLSpec(URL2Validate);
      // nsIFile throws an exception if it's not a file url
    } catch (e) { ifile = null; }
    if (ifile)
    {
      URL2Validate = ifile.path;
      var vwin = window.open("http://validator.w3.org/file-upload.html",
                             "EditorValidate");
      // Window loads asynchronously, so pass control to the load listener:
      vwin.addEventListener("load", this.validateFilePageLoaded, false);
    }
    else
    {
      var vwin2 = window.open("http://validator.w3.org/check?uri="
                              + URL2Validate
                              + "&doctype=Inline",
                              "EditorValidate");
      // This does the validation, no need to wait for page loaded.
    }
  },
  validateFilePageLoaded: function(event)
  {
    event.target.forms[0].uploaded_file.value = URL2Validate;
  }
};

//Also leave this disabled for now unless main editor.
var msiCheckLinksCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    if (!editorElement || !msiIsTopLevelEditor(editorElement))
      return false;
    return msiIsDocumentEditable(editorElement);
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    window.openDialog("chrome://editor/content/EdLinkChecker.xul","_blank", "chrome,close,titlebar,modal", editorElement);
    editorElement.focus();
  }
};

//-----------------------------------------------------------------------------------
//Mostly unchanged. However, we NEED to add the editor (or at least the editor window) to the dialog info in some way.
//(Either that or revert to using a variant of GetCurrentEditor() on return.)
var msiFormCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    return (msiIsDocumentEditable(editorElement) && msiIsEditingRenderedHTML(editorElement));
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    var dlgWindow = msiOpenModelessDialog("chrome://editor/content/msiEdFormProps.xul", "_blank", "chrome,close,titlebar,dependent",
                                                                                                     editorElement, "cmd_form", this);
//    window.openDialog("chrome://editor/content/msiEdFormProps.xul", "_blank", "chrome,close,titlebar,modal");
    editorElement.focus();
  },

  msiGetReviseObject: function(editor)
  {
    // Get a single selected form element
    var theFormElement = null;
    const kTagName = "form";
    try {
      theFormElement = editor.getSelectedElement(kTagName);
      if (!theFormElement)
        theFormElement = editor.getElementOrParentByTagName(kTagName, editor.selection.anchorNode);
      if (!theFormElement)
        theFormElement = editor.getElementOrParentByTagName(kTagName, editor.selection.focusNode);
    } catch (e) {}
    return theFormElement;
  }
};

var msiReviseFormCommand =
{
  isCommandEnabled: function(aCommand, dummy)  {return true;},
  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon)
  {
    var editorElement = msiGetActiveEditorElement();
    var formNode = aParams.getISupportsValue("reviseObject");
    if (formNode != null && editorElement != null)
    {
      AlertWithTitle("msiComposerCommands.js", "In msiReviseFormCommand, trying to revise a form, dialog not yet implemented.");
//      var dlgWindow = msiOpenModelessDialog("chrome://editor/content/msiEdFormProps.xul", "_blank", "chrome,close,titlebar,dependent",
//                                                                                                     editorElement, "cmd_reviseForm", this);
    }
    editorElement.focus();
  },

  doCommand: function(aCommand, dummy)  {}
};

//-----------------------------------------------------------------------------------
//We do need this one.
var msiInputTagCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    return (msiIsDocumentEditable(editorElement) && msiIsEditingRenderedHTML(editorElement));
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},


  doCommand: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    var dlgWindow = msiOpenModelessDialog("chrome://editor/content/EdInputProps.xul", "_blank", "chrome,close,titlebar,dependent",
                                                                                                     editorElement, "cmd_inputtag", this);
//    dlgWindow.focus();  is this necessary?
//    window.openDialog("chrome://editor/content/EdInputProps.xul", "_blank", "chrome,close,titlebar,modal");
//    editorElement.focus();
  },

  msiGetReviseObject: function(editor)
  {
    var inputElement = null;
    const kTagName = "input";
    try {
      inputElement = editor.getSelectedElement(kTagName);
    } catch (e) {}
    return inputElement;
  }
};

//-----------------------------------------------------------------------------------
//Again, the major change we need is to add the editor to the dialog code.
var msiInputImageCommand =
{
  isCommandEnabled: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement();
    return (msiIsDocumentEditable(editorElement) && msiIsEditingRenderedHTML(editorElement));
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement();
    var dlgWindow = msiOpenModelessDialog("chrome://editor/content/msiEdInputImage.xul", "_blank", "chrome,close,titlebar,dependent",
                                                                                                     editorElement, "cmd_inputimage", this);
//    window.openDialog("chrome://editor/content/EdInputImage.xul", "_blank", "chrome,close,titlebar,modal", editorElement);
//    editorElement.focus();
  },

  msiGetReviseObject: function(editor)
  {
    var inputElement = null;
    const kTagName = "input";
    try {
      inputElement = editor.getSelectedElement(kTagName);
    } catch (e) {}
    return inputElement;
  }
};

//-----------------------------------------------------------------------------------
var msiTextAreaCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    return (msiIsDocumentEditable(editorElement) && msiIsEditingRenderedHTML(editorElement));
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    var dlgWindow = msiOpenModelessDialog("chrome://editor/content/msiEdTextAreaProps.xul", "_blank", "chrome,close,titlebar,dependent",
                                                                                                     editorElement, "cmd_textarea", this);
//    window.openDialog("chrome://editor/content/EdTextAreaProps.xul", "_blank", "chrome,close,titlebar,modal");
//    editorElement.focus();
  },

  msiGetReviseObject: function(editor)
  {
    var textAreaElement = null;
    const kTagName = "textarea";
    try {
      textareaElement = editor.getSelectedElement(kTagName);
    } catch (e) {}
    return textAreaElement;
  }
};

var msiReviseTextareaCommand =
{
  isCommandEnabled: function(aCommand, dummy)  {return true;},
  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon)
  {
    var editorElement = msiGetActiveEditorElement();
    var textAreaNode = aParams.getISupportsValue("reviseObject");
    if (textAreaNode != null && editorElement != null)
    {
      AlertWithTitle("msiComposerCommands.js", "In msiReviseTextareaCommand, trying to revise a textarea, dialog not yet implemented.");
//      var dlgWindow = msiOpenModelessDialog("chrome://editor/content/msiEdTextAreaProps.xul", "_blank", "chrome,close,titlebar,dependent",
//                                                                                                     editorElement, "cmd_reviseTextarea", this);
    }
    editorElement.focus();
  },

  doCommand: function(aCommand, dummy)  {}
};

//-----------------------------------------------------------------------------------
var msiSelectCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    return (msiIsDocumentEditable(editorElement) && msiIsEditingRenderedHTML(editorElement));
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    var dlgWindow = msiOpenModelessDialog("chrome://editor/content/msiEdSelectProps.xul", "_blank", "chrome,close,titlebar,dependent",
                                                                                                     editorElement, "cmd_select", this);
//    window.openDialog("chrome://editor/content/EdSelectProps.xul", "_blank", "chrome,close,titlebar,modal", editorElement);
//    editorElement.focus();
  },

  msiGetReviseObject: function(editor)
  {
    var selectElement = null;
    const kTagName = "select";
    try {
      selectElement = editor.getSelectedElement(kTagName);
    } catch (e) {}
    return selectElement;
  }
};

//-----------------------------------------------------------------------------------
var msiButtonCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    return (msiIsDocumentEditable(editorElement) && msiIsEditingRenderedHTML(editorElement));
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    var dlgWindow = msiOpenModelessDialog("chrome://editor/content/msiEdButtonProps.xul", "_blank", "chrome,close,titlebar,dependent",
                                                                                                     editorElement, "cmd_button", this);
//    window.openDialog("chrome://editor/content/EdButtonProps.xul", "_blank", "chrome,close,titlebar,modal", editorElement);
//    editorElement.focus();
  },

  msiGetReviseObject: function(editor)
  {
    var buttonElement = null;
    const kTagName = "button";
    try {
      buttonElement = editor.getSelectedElement(kTagName);
    } catch (e) {}
    return buttonElement;
  }
};

var msiReviseButtonCommand =
{
  isCommandEnabled: function(aCommand, dummy)  {return true;},
  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon)
  {
    var editorElement = msiGetActiveEditorElement();
    var buttonNode = aParams.getISupportsValue("reviseObject");
    if (buttonNode != null && editorElement != null)
    {
      AlertWithTitle("msiComposerCommands.js", "In msiReviseButtonCommand, trying to revise a button, dialog not yet implemented.");
//      var dlgWindow = msiOpenModelessDialog("chrome://editor/content/msiEdTextAreaProps.xul", "_blank", "chrome,close,titlebar,dependent",
//                                                                                                     editorElement, "cmd_reviseTextarea", this);
    }
    editorElement.focus();
  },

  doCommand: function(aCommand, dummy)  {}
};

//-----------------------------------------------------------------------------------
var msiLabelCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    return (msiIsDocumentEditable(editorElement) && msiIsEditingRenderedHTML(editorElement));
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
//    var tagName = "label";
    try {
      var editor = msiGetEditor(editorElement);
      var labelElement = this.msiGetReviseObject(editor);
//      // Find selected label or if start/end of selection is in label 
//      var labelElement = editor.getSelectedElement(tagName);
//      if (!labelElement)
//        labelElement = editor.getElementOrParentByTagName(tagName, editor.selection.anchorNode);
//      if (!labelElement)
//        labelElement = editor.getElementOrParentByTagName(tagName, editor.selection.focusNode);
      if (labelElement) {
        // We only open the dialog for an existing label
        var dlgWindow = msiOpenModelessDialog("chrome://editor/content/msiEdLabelProps.xul", "_blank", "chrome,close,titlebar,dependent",
                                                                        editorElement, "cmd_label", this, labelElement);
//        msiOpenModalDialog("chrome://editor/content/msiEdLabelProps.xul", "_blank", "chrome,close,titlebar,dependent",
//                                                                                                     editorElement, "cmd_label", this);
//        window.openDialog("chrome://editor/content/EdLabelProps.xul", "_blank", "chrome,close,titlebar,modal", labelElement);
//        editorElement.focus();
      } else {
        msiEditorSetTextProperty(editorElement, tagName, "", "");
      }
    } catch (e) {}
  },

  msiGetReviseObject: function(editor)
  {
    var labelElement = null;
    const kTagName = "label";
    try {
      // Find selected label or if start/end of selection is in label 
      labelElement = editor.getSelectedElement(kTagName);
      if (!labelElement)
        labelElement = editor.getElementOrParentByTagName(kTagName, editor.selection.anchorNode);
      if (!labelElement)
        labelElement = editor.getElementOrParentByTagName(kTagName, editor.selection.focusNode);
    } catch (e) {}
    return labelElement;
  }
};

var msiReviseLabelCommand =
{
  isCommandEnabled: function(aCommand, dummy)  {return true;},
  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon)
  {
    var editorElement = msiGetActiveEditorElement();
    var labelNode = aParams.getISupportsValue("reviseObject");
    if (labelNode != null && editorElement != null)
    {
      AlertWithTitle("msiComposerCommands.js", "In msiReviseLabelCommand, trying to revise a label, dialog not yet implemented.");
//      var dlgWindow = msiOpenModelessDialog("chrome://editor/content/msiEdTextAreaProps.xul", "_blank", "chrome,close,titlebar,dependent",
//                                                                                                     editorElement, "cmd_reviseTextarea", this);
    }
    editorElement.focus();
  },

  doCommand: function(aCommand, dummy)  {}
};

//-----------------------------------------------------------------------------------
//Again, we need to do something to tie the editor to the dialog used.
var msiFieldSetCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    return (msiIsDocumentEditable(editorElement) && msiIsEditingRenderedHTML(editorElement));
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    var dlgWindow = msiOpenModelessDialog("chrome://editor/content/msiEdFieldSetProps.xul", "_blank", "chrome,close,titlebar,dependent",
                                                                        editorElement, "cmd_fieldset", this);
//    window.openDialog("chrome://editor/content/msiEdFieldSetProps.xul", "_blank", "chrome,close,titlebar,modal");
//    editorElement.focus();
  },

  msiGetReviseObject: function(editor)
  {
    var fieldsetElement = null;
    const kTagName = "fieldset";
    try {
      // Find a selected fieldset, or if one is at start or end of selection.
      fieldsetElement = editor.getSelectedElement(kTagName);
      if (!fieldsetElement)
        fieldsetElement = editor.getElementOrParentByTagName(kTagName, editor.selection.anchorNode);
      if (!fieldsetElement)
        fieldsetElement = editor.getElementOrParentByTagName(kTagName, editor.selection.focusNode);
    } catch (e) {}
    return fieldsetElement;
  }
};

var msiReviseFieldsetCommand =
{
  isCommandEnabled: function(aCommand, dummy)  {return true;},
  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon)
  {
    var editorElement = msiGetActiveEditorElement();
    var fieldSetNode = aParams.getISupportsValue("reviseObject");
    if (fieldSetNode != null && editorElement != null)
    {
      AlertWithTitle("msiComposerCommands.js", "In msiReviseFieldsetCommand, trying to revise a fieldset, dialog not yet implemented.");
//      var dlgWindow = msiOpenModelessDialog("chrome://editor/content/msiEdTextAreaProps.xul", "_blank", "chrome,close,titlebar,dependent",
//                                                                                                     editorElement, "cmd_reviseTextarea", this);
    }
    editorElement.focus();
  },

  doCommand: function(aCommand, dummy)  {}
};

//-----------------------------------------------------------------------------------
var msiIsIndexCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    return (msiIsDocumentEditable(editorElement) && msiIsEditingRenderedHTML(editorElement));
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    try {
      var editor = msiGetEditor(editorElement);
      var isindexElement = editor.createElementWithDefaults("isindex");
      isindexElement.setAttribute("prompt", editor.outputToString("text/plain", 1)); // OutputSelectionOnly
      editor.insertElementAtSelection(isindexElement, true);
    } catch (e) {}
  }
};

//-----------------------------------------------------------------------------------
//Once again, need to tie the editor to the dialog.
var msiImageCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    return (msiIsDocumentEditable(editorElement) && msiIsEditingRenderedHTML(editorElement));
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},


  doCommand: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    var dlgWindow = msiOpenModelessDialog("chrome://prince/content/msiEdImageProps.xul", "_blank", "chrome,close,titlebar,dependent",
                                                                                                     editorElement, "cmd_image", this);
//    window.openDialog("chrome://editor/content/EdImageProps.xul","_blank", "chrome,close,titlebar,modal");
//    editorElement.focus();
  },
  msiGetReviseObject: function(editor)
  {
    var imageElement = null;
    try {
      var tagName = "img";
      imageElement = editor.getSelectedElement("input");
      if (!imageElement || imageElement.getAttribute("type") != "image") {
        // Get a single selected image element
        imageElement = editor.getSelectedElement(tagName);
//        if (imageElement)
//          gAnchorElement = editor.getElementOrParentByTagName("href", imageElement);
      }
    } catch (e) {}
    return imageElement;
  }
};

var msiReviseImageCommand =
{
  isCommandEnabled: function(aCommand, dummy)  {return true;},
  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon)
  {
    var editorElement = msiGetActiveEditorElement();
    var imageNode = aParams.getISupportsValue("reviseObject");
    if (imageNode != null && editorElement != null)
    {
      var dlgWindow = msiOpenModelessDialog("chrome://prince/content/msiEdImageProps.xul", "_blank", "chrome,close,titlebar,dependent",
                                                                                                     editorElement, "cmd_reviseImage", this);
    }
    editorElement.focus();
  },

  doCommand: function(aCommand, dummy)  {}
};

//-----------------------------------------------------------------------------------
//Again need to tie the dialog to the editor. May want to have an alternate GetPrefs() as well.
var msiHLineCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    return (msiIsDocumentEditable(editorElement) && msiIsEditingRenderedHTML(editorElement));
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand, dummy)
  {
    // Inserting an HLine is different in that we don't use properties dialog
    //  unless we are editing an existing line's attributes
    //  We get the last-used attributes from the prefs and insert immediately

    var editorElement = msiGetActiveEditorElement();
    var tagName = "hr";
    var editor = msiGetEditor(editorElement);
      
    var hLine;
    try {
      hLine = editor.getSelectedElement(tagName);
    } catch (e) {return;}

    if (hLine)
    {
      // We only open the dialog for an existing HRule
      window.openDialog("chrome://editor/content/EdHLineProps.xul", "_blank", "chrome,close,titlebar,modal");
      editorElement.focus();
    } 
    else
    {
      try {
        hLine = editor.createElementWithDefaults(tagName);

        // We change the default attributes to those saved in the user prefs
        var prefs = GetPrefs();
        var align = prefs.getIntPref("editor.hrule.align");
        if (align == 0)
          editor.setAttributeOrEquivalent(hLine, "align", "left", true);
        else if (align == 2)
          editor.setAttributeOrEquivalent(hLine, "align", "right", true);

        //Note: Default is center (don't write attribute)
  
        var width = prefs.getIntPref("editor.hrule.width");
        var percent = prefs.getBoolPref("editor.hrule.width_percent");
        if (percent)
          width = width +"%";

        editor.setAttributeOrEquivalent(hLine, "width", width, true);

        var height = prefs.getIntPref("editor.hrule.height");
        editor.setAttributeOrEquivalent(hLine, "size", String(height), true);

        var shading = prefs.getBoolPref("editor.hrule.shading");
        if (shading)
          hLine.removeAttribute("noshade");
        else
          hLine.setAttribute("noshade", "noshade");

        editor.insertElementAtSelection(hLine, true);

      } catch (e) {}
    }
  }
};

//-----------------------------------------------------------------------------------
//Here we need a revised version of GetObjectForProperties() which references the editor. We'd then need to tie the
//dialog to the editor as well.
var msiLinkCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    return (msiIsDocumentEditable(editorElement) && msiIsEditingRenderedHTML(editorElement));
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand, dummy)
  {
    // If selected element is an image, launch that dialog instead 
    // since last tab panel handles 
    var editorElement = msiGetActiveEditorElement();
    var element = msiGetObjectForProperties(editorElement);
    if (element && msiGetBaseNodeName(element).toLowerCase() == "img")
      window.openDialog("chrome://prince/content/EdImageProps.xul","_blank", "chrome,close,titlebar,modal", null, true);
    else
      window.openDialog("chrome://editor/content/EdLinkProps.xul","_blank", "chrome,close,titlebar,modal");
    editorElement.focus();
  }
};

var msiReviseHyperlinkCommand =
{
  isCommandEnabled: function(aCommand, dummy)  {return true;},
  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon)
  {
    var editorElement = msiGetActiveEditorElement();
    var linkNode = aParams.getISupportsValue("reviseObject");
    if (linkNode != null && editorElement != null)
    {
      AlertWithTitle("msiComposerCommands.js", "In msiReviseHyperlinkCommand, trying to revise a hyperlink, dialog not implemented.");
//      window.openDialog("chrome://editor/content/EdNamedAnchorProps.xul", "_blank", "chrome,close,titlebar,modal", "", editorElement);
    }
    editorElement.focus();
  },

  doCommand: function(aCommand, dummy)  {}

};

//-----------------------------------------------------------------------------------
//Need to tie the editor to the dialog.
var msiAnchorCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    return (msiIsDocumentEditable(editorElement) && msiIsEditingRenderedHTML(editorElement));
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    window.openDialog("chrome://editor/content/EdNamedAnchorProps.xul", "_blank", "chrome,close,titlebar,modal", "", editorElement);
    editorElement.focus();
  }
};

var msiReviseAnchorCommand =
{
  isCommandEnabled: function(aCommand, dummy)  {return true;},
  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon)
  {
    var editorElement = msiGetActiveEditorElement();
    var anchorNode = aParams.getISupportsValue("reviseObject");
    if (anchorNode != null && editorElement != null)
    {
      AlertWithTitle("msiComposerCommands.js", "In msiReviseAnchorCommand, trying to revise hyperlink anchor, dialog not implemented.");
//      window.openDialog("chrome://editor/content/EdNamedAnchorProps.xul", "_blank", "chrome,close,titlebar,modal", "", editorElement);
    }
    editorElement.focus();
  },

  doCommand: function(aCommand, dummy)  {}
};

//-----------------------------------------------------------------------------------
//Need to tie the editor to the dialog.
var msiInsertHTMLWithDialogCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    return (msiIsDocumentEditable(editorElement) && msiIsEditingRenderedHTML(editorElement));
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    window.openDialog("chrome://editor/content/EdInsSrc.xul","_blank", "chrome,close,titlebar,modal,resizable", "", editorElement);
    editorElement.focus();
  }
};

var msiReviseHTMLCommand =
{
  isCommandEnabled: function(aCommand, dummy)  {return true;},
  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon)
  {
    var editorElement = msiGetActiveEditorElement();
    var htmlNode = aParams.getISupportsValue("reviseObject");
    if (htmlNode != null && editorElement != null)
    {
      AlertWithTitle("msiComposerCommands.js", "In msiReviseHTMLCommand, trying to revise encapsulated HTML, dialog not implemented.");
//      window.openDialog("chrome://editor/content/EdInsSrc.xul","_blank", "chrome,close,titlebar,modal,resizable", "", editorElement);
    }
    editorElement.focus();
  },

  doCommand: function(aCommand, dummy)  {}
};

//-----------------------------------------------------------------------------------
//Need new version of EditorFindOrCreateInsertCharWindow(), or ??????
var msiInsertCharsCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    return msiIsDocumentEditable(editorElement);
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement();
    var dlgWindow = msiOpenModelessDialog("chrome://editor/content/msiEdInsertChars.xul", "_blank", "chrome,close,titlebar,dependent",
                                                                                                     editorElement, "cmd_insertChars", this);
//    msiEditorFindOrCreateInsertCharWindow(editorElement);
  }
};

var msiReviseCharsCommand =
{
  isCommandEnabled: function(aCommand, dummy)  {return true;},
  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon)
  {
    var editorElement = msiGetActiveEditorElement();
    var charsObject = aParams.getISupportsValue("reviseObject");
    if (charsObject != null && editorElement != null)
    {
      AlertWithTitle("msiComposerCommands.js", "In msiReviseCharsCommand, trying to revise a character, dialog not yet implemented.");
//      var dlgWindow = msiOpenModelessDialog("chrome://editor/content/msiEdTextAreaProps.xul", "_blank", "chrome,close,titlebar,dependent",
//                                                                                                     editorElement, "cmd_reviseTextarea", this);
    }
    editorElement.focus();
  },

  doCommand: function(aCommand, dummy)  {}
};
//-----------------------------------------------------------------------------------
var msiInsertBreakCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    return (msiIsDocumentEditable(editorElement) && msiIsEditingRenderedHTML(editorElement));
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    try {
      msiGetEditor(editorElement).insertHTML("<br/>");
    } catch (e) {}
  }
};

//-----------------------------------------------------------------------------------
var msiInsertBreakAllCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    return (msiIsDocumentEditable(editorElement) && msiIsEditingRenderedHTML(editorElement));
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    try {
      msiGetEditor(editorElement).insertHTML("<br clear='all'/>");
    } catch (e) {}
  }
};

//-----------------------------------------------------------------------------------

var msiInsertHorizontalSpacesCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    return (msiIsDocumentEditable(editorElement) && msiIsEditingRenderedHTML(editorElement));
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    var hSpaceData = new Object();
    hSpaceData.spaceType = "normalSpace";
    try {
      msiOpenModelessDialog("chrome://prince/content/HorizontalSpaces.xul", "_blank", "chrome,close,titlebar,dependent",
                                        editorElement, "cmd_insertHorizontalSpaces", this, hSpaceData);
    }
    catch(ex) {
      dump("*** Exception: couldn't open HorizontalSpaces Dialog: " + ex + "\n");
    }
  }
};

var msiReviseHorizontalSpacesCommand =
{
  isCommandEnabled: function(aCommand, dummy)  {return true;},
  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon)
  {
    var editorElement = msiGetActiveEditorElement();
    var hspaceNode = aParams.getISupportsValue("reviseObject");
    if (hspaceNode != null && editorElement != null)
    {
      AlertWithTitle("msiComposerCommands.js", "In msiReviseHorizontalSpacesCommand, trying to revise a horizontal space, dialog not yet implemented.");
//      var dlgWindow = msiOpenModelessDialog("chrome://editor/content/msiEdTextAreaProps.xul", "_blank", "chrome,close,titlebar,dependent",
//                                                                                                     editorElement, "cmd_reviseTextarea", this);
    }
    editorElement.focus();
  },

  doCommand: function(aCommand, dummy)  {}
};

function msiInsertHorizontalSpace(dialogData, editorElement)
{
  var editor = msiGetEditor(editorElement);
  var parentNode = editor.selection.anchorNode;
  var insertPos = editor.selection.anchorOffset;
  if (dialogData.spaceType == "normalSpace")
  {
    editor.insertHTMLWithContext(" ", "", "", "", null, parentNode, insertPos, false);
    dump("In msiInsertHorizontalSpace, inserting normal space.\n");
    return;
  }
  var dimensionsFromSpaceType = 
  {
//    requiredSpace:
//    nonBreakingSpace:
    emSpace:        "1em",
    twoEmSpace:       "2em",
    thinSpace:      "0.17em",
    thickSpace:     "0.5em",
    italicCorrectionSpace: "0.083en",
    negativeThinSpace:   "0.0em",
    zeroSpace:           "0.0em",
    noIndent:            "0.0em"
  };
  var contentFromSpaceType = 
  {
    requiredSpace:     "&#x205f;",  //MEDIUM MATHEMATICAL SPACE in Unicode?
    nonBreakingSpace:  "&#x00a0;",
    emSpace:           "&#x2003;",
    twoEmSpace:          "&#x2001;",  //EM QUAD
    thinSpace:         "&#x2009;",
    thickSpace:        "&#x2002;",  //"EN SPACE" in Unicode?
    italicCorrectionSpace:  "&#x200a;",  //the "HAIR SPACE" in Unicode?
    zeroSpace:          "&#x200b;"
//    negativeThinSpace:
//    noIndent:
  };
  var specialShowInvisibleChars = 
  {
    noIndent:          "&#x2190;"  //left arrow
  };
  var spaceStr = "<hspace type=\"";
  if (dialogData.spaceType != "customSpace")
  {
    spaceStr += dialogData.spaceType;
    if (dialogData.spaceType in dimensionsFromSpaceType)
      spaceStr += "\" dim=\"" + dimensionsFromSpaceType[dialogData.spaceType];
  }
  else if (dialogData.customSpaceData.customType == "fixed")
  {
    spaceStr += "customSpace\" dim=\"";
    spaceStr += String(dialogData.customSpaceData.fixedData.size) + dialogData.customSpaceData.fixedData.units;
  spaceStr += "\" atEnd=\"" + (dialogData.customSpaceData.typesetChoice=="always"?"true":"false");
  }
  else if (dialogData.customSpaceData.customType == "stretchy")
  {
    spaceStr += "stretchySpace\" class=\"stretchySpace\" flex=\"";
    spaceStr += String(dialogData.customSpaceData.stretchData.factor);
    if (dialogData.customSpaceData.stretchData.fillWith == "fillLine")
      spaceStr += "\" fillWith=\"line";
    else if (dialogData.customSpaceData.stretchData.fillWith == "fillDots")
      spaceStr += "\" fillWith=\"dots";
  }
  if (dialogData.spaceType in contentFromSpaceType)
    spaceStr += "\">" + contentFromSpaceType[dialogData.spaceType] + "</hspace>";
  else
    spaceStr += "\"/>";
  dump("In msiInsertHorizontalSpace, inserting space: [" + spaceStr + "].\n");
//  editor.insertHTMLWithContext(spaceStr, "", "", "", null, parentNode, insertPos, false);
  insertXMLAtCursor(editor, spaceStr, true, false);
}

//-----------------------------------------------------------------------------------
var msiInsertVerticalSpacesCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    return (msiIsDocumentEditable(editorElement) && msiIsEditingRenderedHTML(editorElement));
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    var vSpaceData = new Object();
    vSpaceData.spaceType = "smallSkip";
    try {
      msiOpenModelessDialog("chrome://prince/content/VerticalSpaces.xul", "_blank", "chrome,close,titlebar,dependent",
                                        editorElement, "cmd_insertVerticalSpaces", this, vSpaceData);
    }
    catch(ex) {
      dump("*** Exception: couldn't open VerticalSpaces Dialog: " + ex + "\n");
    }
  }
};

var msiReviseVerticalSpacesCommand =
{
  isCommandEnabled: function(aCommand, dummy)  {return true;},
  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon)
  {
    var editorElement = msiGetActiveEditorElement();
    var vspaceNode = aParams.getISupportsValue("reviseObject");
    if (vspaceNode != null && editorElement != null)
    {
      AlertWithTitle("msiComposerCommands.js", "In msiReviseVerticalSpacesCommand, trying to revise a vertical space, dialog not yet implemented.");
//      var dlgWindow = msiOpenModelessDialog("chrome://editor/content/msiEdTextAreaProps.xul", "_blank", "chrome,close,titlebar,dependent",
//                                                                                                     editorElement, "cmd_reviseTextarea", this);
    }
    editorElement.focus();
  },

  doCommand: function(aCommand, dummy)  {}
};

function msiInsertVerticalSpace(dialogData, editorElement)
{
  var editor = msiGetEditor(editorElement);
//  var parentNode = editor.selection.anchorNode;
//  var insertPos = editor.selection.anchorOffset;
  var dimensionsFromSpaceType = 
  {
//    requiredSpace:
//    nonBreakingSpace:
    smallSkip:        "3pt",
    mediumSkip:       "6pt",
    bigSkip:         "12pt"
  };
  var lineHeightFromSpaceType = 
  {
    strut:     "100%",
    mathStrut: "100%"   //not really right, but for the moment
  };
  var contentFromSpaceType = 
  {
//    zeroSpace:          "&#x200b;"
////    negativeThinSpace:
////    noIndent:
  };
  var specialShowInvisibleChars = 
  {
    strut:          ""  //
  };
  var spaceStr = "<vspace type=\"";
  if (dialogData.spaceType != "customSpace")
  {
    spaceStr += dialogData.spaceType;
    if (dialogData.spaceType in dimensionsFromSpaceType)
      spaceStr += "\" dim=\"" + dimensionsFromSpaceType[dialogData.spaceType];
    else if (dialogData.spaceType in lineHeightFromSpaceType)
      spaceStr += "\" lineHt=\"" + lineHeightFromSpaceType[dialogData.spaceType];
  }
  else
  {
    spaceStr += "customSpace\" dim=\"";
    spaceStr += String(dialogData.customSpaceData.sizeData.size) + dialogData.customSpaceData.sizeData.units;
  spaceStr += "\" atEnd=\"" + (dialogData.customSpaceData.typesetChoice=="always"?"true":"false");
  }
  if (dialogData.spaceType in contentFromSpaceType)
    spaceStr += "\">" + contentFromSpaceType[dialogData.spaceType] + "</vspace>";
  else
    spaceStr += "\"/>";
  dump("In msiInsertVerticalSpace, inserting space: [" + spaceStr + "].\n");
//  editor.insertHTMLWithContext(spaceStr, "", "", "", null, parentNode, insertPos, false);
  insertXMLAtCursor(editor, spaceStr, true, false);
}


//-----------------------------------------------------------------------------------
var msiInsertRulesCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    return (msiIsDocumentEditable(editorElement) && msiIsEditingRenderedHTML(editorElement));
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    var rulesData = new Object();
    try {
      msiOpenModelessDialog("chrome://prince/content/msiRulesDialog.xul", "_blank", "chrome,close,titlebar,dependent",
                                        editorElement, "cmd_msiInsertRules", this, rulesData);
    }
    catch(ex) {
      dump("*** Exception: couldn't open msiRules Dialog: " + ex + "\n");
    }
  }
};

var msiReviseRulesCommand =
{
  isCommandEnabled: function(aCommand, dummy)  {return true;},
  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon)
  {
    var editorElement = msiGetActiveEditorElement();
    var ruleNode = aParams.getISupportsValue("reviseObject");
    if (ruleNode != null && editorElement != null)
    {
      AlertWithTitle("msiComposerCommands.js", "In msiReviseRulesCommand, trying to revise a rule, dialog not yet implemented.");
//      var dlgWindow = msiOpenModelessDialog("chrome://editor/content/msiEdTextAreaProps.xul", "_blank", "chrome,close,titlebar,dependent",
//                                                                                                     editorElement, "cmd_reviseTextarea", this);
    }
    editorElement.focus();
  },

  doCommand: function(aCommand, dummy)  {}
};

function msiInsertRules(dialogData, editorElement)
{
  var editor = msiGetEditor(editorElement);
//  var parentNode = editor.selection.anchorNode;
//  var insertPos = editor.selection.anchorOffset;
  var ruleStr = "<msirule ";
  ruleStr += "lift=\"";
  ruleStr += String(dialogData.lift.size) + dialogData.lift.units;
  ruleStr += "\" width=\"";
  ruleStr += String(dialogData.width.size) + dialogData.width.units;
  ruleStr += "\" height=\"";
  ruleStr += String(dialogData.height.size) + dialogData.height.units;
  ruleStr += "\" color=\"" + dialogData.ruleColor;
  ruleStr += "\"/>";
  dump("In msiInsertRules, inserting rule: [" + ruleStr + "].\n");
//  editor.insertHTMLWithContext(spaceStr, "", "", "", null, parentNode, insertPos, false);
  insertXMLAtCursor(editor, ruleStr, true, false);
}


//-----------------------------------------------------------------------------------
var msiInsertBreaksCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    return (msiIsDocumentEditable(editorElement) && msiIsEditingRenderedHTML(editorElement));
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    var breaksData = new Object();
    try {
      msiOpenModelessDialog("chrome://prince/content/msiBreaksDialog.xul", "_blank", "chrome,close,titlebar,dependent",
                                        editorElement, "cmd_msiInsertBreaks", this, breaksData);
    }
    catch(ex) {
      dump("*** Exception: couldn't open msiBreaksDialog: " + ex + "\n");
    }
  }
};

var msiReviseBreaksCommand =
{
  isCommandEnabled: function(aCommand, dummy)  {return true;},
  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon)
  {
    var editorElement = msiGetActiveEditorElement();
    var breakNode = aParams.getISupportsValue("reviseObject");
    if (breakNode != null && editorElement != null)
    {
      AlertWithTitle("msiComposerCommands.js", "In msiReviseBreaksCommand, trying to revise a break, dialog not yet implemented.");
//      var dlgWindow = msiOpenModelessDialog("chrome://editor/content/msiEdTextAreaProps.xul", "_blank", "chrome,close,titlebar,dependent",
//                                                                                                     editorElement, "cmd_reviseTextarea", this);
    }
    editorElement.focus();
  },

  doCommand: function(aCommand, dummy)  {}
};

function msiInsertBreaks(dialogData, editorElement)
{
  var editor = msiGetEditor(editorElement);
//  var parentNode = editor.selection.anchorNode;
//  var insertPos = editor.selection.anchorOffset;
  var contentFromBreakType =
  {
    allowBreak:             "&#x200b;",  //this is the zero-width space
    discretionaryHyphen:    "&#x00ad;",
    noBreak:                "&#x2060;",
    pageBreak:              "&#x000c;",  //formfeed?
    newPage:                "&#x000c;",  //formfeed?
    lineBreak:              "<br xmlns=\"" + xhtmlns + "\"></br>",
    newLine:                "<br xmlns=\"" + xhtmlns + "\"></br>"
  };
  var alternateContentFromBreakType = 

  {
    allowBreak:             "|",
    discretionaryHyphen:    "-",
    noBreak:                "~",
    pageBreak:              "&#x21b5;",
    newPage:                "&#x21b5;",
    lineBreak:              "&#x21b5;",
    newLine:                "&#x21b5;"
  };
  var breakStr = "<msibreak type=\"";
  if (dialogData.breakType == "customNewLine")
  {
    breakStr += "customNewLine\" dim=\"";
    breakStr += String(dialogData.customBreakData.sizeData.size) + dialogData.customBreakData.sizeData.units;
  }

  else
  {
    breakStr += dialogData.breakType;
  }

  if (dialogData.breakType in alternateContentFromBreakType)
    breakStr += "\" invisDisplay=\"" + alternateContentFromBreakType[dialogData.breakType];
  if (dialogData.breakType in contentFromBreakType)
    breakStr += "\">" + contentFromBreakType[dialogData.breakType] + "</msibreak>";
//    breakStr += "\"/>" + contentFromBreakType[dialogData.breakType];
  else
    breakStr += "\"/>";

  dump("In msiInsertBreaks, inserting break: [" + breakStr + "].\n");
  insertXMLAtCursor(editor, breakStr, true, false);
}


//-----------------------------------------------------------------------------------
var msiInsertReturnFancyCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    return (msiIsDocumentEditable(editorElement) && msiIsEditingRenderedHTML(editorElement));
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    try {
      msiGetEditor(editorElement).insertReturnFancy();
    } catch (e) {}
  }
};
//-----------------------------------------------------------------------------------

var msiInsertSubstructureCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    return (msiIsDocumentEditable(editorElement) && msiIsEditingRenderedHTML(editorElement));
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    try {
      var atom = new Object();
      var editor = msiGetEditor(editorElement);
      var currentStruct = editor.tagListManager.currentValue("structtag", atom);
      var nextStruct = editor.tagListManager.getStringPropertyForTag(currentStruct, atom, "prefsub");
      msiDoStatefulCommand('cmd_structtag',nextStruct);
    } catch (e) 
    {
      alert("Exception: "+e);
    }
  }
};

//-----------------------------------------------------------------------------------

var msiMacroFragmentCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    return (msiIsDocumentEditable(editorElement) && msiIsEditingRenderedHTML(editorElement));
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand, dummy)
  {
    var macrofragmentStatusPanel = document.getElementById('macroEntryPanel');
    if (macrofragmentStatusPanel)
    {
      macrofragmentStatusPanel.setAttribute("hidden", "false");
      document.getElementById('macroAndFragments').focus();
    }
  }
};


//-----------------------------------------------------------------------------------
var msiGridCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    return (msiIsDocumentEditable(editorElement) && msiIsEditingRenderedHTML(editorElement));
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    window.openDialog("chrome://editor/content/EdSnapToGrid.xul","_blank", "chrome,close,titlebar,modal");
    editorElement.focus();
  }
};


//-----------------------------------------------------------------------------------
var msiListPropertiesCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    return (msiIsDocumentEditable(editorElement) && msiIsEditingRenderedHTML(editorElement));

  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    window.openDialog("chrome://editor/content/EdListProps.xul","_blank", "chrome,close,titlebar,modal");
    editorElement.focus();
  }
};


//-----------------------------------------------------------------------------------
var msiPagePropertiesCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    return (msiIsDocumentEditable(editorElement) && msiIsEditingRenderedHTML(editorElement));
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    var oldTitle = msiGetDocumentTitle(editorElement);
    window.openDialog("chrome://editor/content/EdPageProps.xul","_blank", "chrome,close,titlebar,modal", "");

    // Update main window title and 
    // recent menu data in prefs if doc title changed
    if (msiGetDocumentTitle(editorElement) != oldTitle && ("UpdateWindowTitle" in window))
      window.UpdateWindowTitle();

    editorElement.focus();
  }
};


//-----------------------------------------------------------------------------------
var msiDocumentInfoCommand = 
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetTopLevelEditorElement();
    if (msiGetEditorURL(editorElement).length > 0)
      return (msiIsDocumentEditable(editorElement) && msiIsEditingRenderedHTML(editorElement));
    return false;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    // Launch Document Info dialog
    var editorElement = msiGetTopLevelEditorElement();
    var documentInfo = new msiDocumentInfo(editorElement);
    documentInfo.initializeDocInfo();
    var dlgInfo = documentInfo.getDialogInfo();

    try {
      msiOpenModelessDialog("chrome://prince/content/DocumentInfo.xul", "_blank", "chrome,close,titlebar,dependent",
                                        editorElement, "cmd_documentInfo", this, dlgInfo);
    }
    catch(ex) {
      dump("*** Exception: couldn't open DocInfo Dialog: " + ex + "\n");
    }
    editorElement.contentWindow.focus();
  }
};


function msiFinishDocumentInfoDialog(editorElement, dlgInfo)
{
  if (!dlgInfo.cancel)
  {
    var docInfo = dlgInfo.parentData;
    docInfo.resetFromDialogInfo(dlgInfo);
    docInfo.putDocInfoToDocument();
  }
}

//The msiDocumentInfo object contains (under the following sub-objects) children which reflect the document nodes
//  containing arbitrary metadata and proprietary information encoded as XML comments. Metadata of particular interest
//  (i.e., that appearing in the Doc Info dialog) is captured in the appropriate sub-object. Each of these basic children
//  will have:
//    (i) a "name" field (the keyword as it appears in the document, for instance, the "rel" attribute of a <link>
//        or the "name" attribute of a <meta> or the key in a "TCIDATA" key-value comment;
//   (ii) either a "contents" or a "uri" field ("uri" only for <link> or <!-- TCIDATA LINK... --> comment);
//  (iii) a "type" field to be used in putting the revised data back into the document - this should be one of the
//        strings "title" (used only for a <title> element), "meta", "link", "comment-meta", "comment-link",
//        "comment-key-value", "comment-meta-alt", or "comment-link-alt", where the "-alt" versions mean the
//        comment contained the contents or href within single rather than double quotes - this is done so a simple regular
//        expression String.replace() can find and replace the value when rewriting the node;
//   (iv) a "status" field which can contain the strings "changed", "deleted", or "done", again to facilitate replacement
//        in the document.

//The function msiDocumentInfo.getDialogInfo() will return a dialogInfo object, where the document data will be
//  digested into the print and save flags and strings appearing in the dialog. (No digestion of the "metadata" or "comments"
//  sub-objects is necessary.)

//The sub-objects other than "dialogInfo" will be:
//  msiDocumentInfo.generalSettings - an object containing the data appearing in the first page of the DocumentInfo dialog. The
//    data objects reflecting the document nodes appear as children of this object. When getDialogInfo() is called, the actual
//    strings appearing in the dialog will be elements of dialogInfo.general.
//  msiDocumentInfo.comments - an object containing the "comment" and "description" data reflected in the Comments page
//    of the dialog. These are data objects reflecting the document nodes; they are msiDocumentInfo.comments.comment and
//    msiDocumentInfo.comments.description.
//  msiDocumentInfo.metadata - an object containing members indexed by (our internal) metadatum name. Each member is an object
//    containing either a "uri" member or a "content" member, depending on whether it represents a <meta> or a <link>.
//    The "internal metadatum name" is taken from the keys in the file docInfoDialog.properties, with the prefix (before
//    the dot) removed.
//    E.g.:
//      data.metadata.author.contents = "Noam Chomsky"
//      data.metadata.previous.uri = "../siblingDirectory/documentBeforeThisOne.xhtml"
//      data.metadata.chapter.uri = "http://junk.mackichan.com/internal/sillyDocuments/chapterTwo.xhtml"
//    When getDialogInfo() is called, the returned dialogInfo will contain a metadata member which is the same as this.
//  msiDocumentInfo.printSettings - an object containing the data children pertaining to print options. The actual dialog
//    values for flags etc. are returned in dialogInfo.printOptions, which is an msiPrintOptions object (see msiEditorUtilities.js).
//  msiDocumentInfo.saveSettings = an object containing the data children pertaining to save options. The actual dialog
//    values for flags etc. are returned in dialogInfo.saveOptions.

//BEWARE ALSO the dual use of the word "comment" here - the member functions referring to "comment" have to do with inserting
//   or parsing "#comment" nodes (XML comments) in the document. The member items called "comments" refer to the "Comment"
//   and "Description" fields as of old, which are presumably to be encoded as <meta name="Comment" content="Gee this is quaint."/>

function msiDocumentInfo(editorElement)
{
  this.mEditorElement = editorElement;
  this.mEditor = msiGetEditor(editorElement);

  this.findMetadataNodes = function(aNode)
  {
    var theName = aNode.localName;
    if (!theName || !theName.length)
      theName = msiGetBaseNodeName(aNode);
    switch(theName.toLowerCase())
    {
      case "meta":
      case "link":
      case "#comment":
      case "title":
      case "author":
      case "address":
        return NodeFilter.FILTER_ACCEPT;
      break;
      
      default:
        return NodeFilter.FILTER_SKIP;
      break;
    }
    return NodeFilter.FILTER_SKIP;
  };

  this.initializeDocInfo = function()
  {
    var docHead = msiGetDocumentHead(this.mEditor);

    this.generalSettings = new Object();
    this.comments = new Object();
    this.printSettings = new Object();
    this.metadata = new Object();
    this.saveSettings = new Object();

    var treeWalker = this.mEditor.document.createTreeWalker(docHead,
                                                            NodeFilter.SHOW_ELEMENT|NodeFilter.SHOW_COMMENT,
                                                            this.findMetadataNodes,
                                                            true);
    if (treeWalker)
    {
      for (var currNode = treeWalker.nextNode(); currNode != null; currNode = treeWalker.nextNode())
      {
        var theName = currNode.localName;
        if (!theName || !theName.length)
          theName = currNode.nodeName;
        var subObject = null;

        switch(theName.toLowerCase())
        {
          case "meta":
          {
            var metaName = currNode.getAttribute("name").toLowerCase();
            var metaValue = currNode.getAttribute("content");
            if (metaName && metaValue && metaName.length && metaValue.length)
            {
              var metaObj = new Object();
              metaObj.contents = metaValue;
              metaObj.name = currNode.getAttribute("name");  //Name in document, rather than one used to index.
              metaObj.type = "meta";
              switch(metaName)
              {
                case "created":
                case "lastrevised":
                case "language":
                  subObject = this.generalSettings;
                break;
                case "comment":
                case "description":
                  subObject = this.comments;
                break;
                default:
                  subObject = this.metadata;
                break;
              }
              subObject[metaName] = metaObj;
            }
          }
          break;
          case "link":
          {
            var linkName = currNode.getAttribute("rel").toLowerCase();
            var linkTarget = currNode.getAttribute("href");
            if (linkName && linkTarget && linkName.length && linkTarget.length)
            {
              var linkObj = new Object();
              linkObj.name = currNode.getAttribute("rel");  //Name in document, rather than one used to index.
              linkObj.type = "link";
              linkObj.uri = linkTarget;
              if (linkName == "documentshell")
                this.generalSettings.documentshell = linkObj;
              else
                this.metadata[linkName] = linkObj;
            }
          }
          break;
          case "title":
          {
            var titleObj = new Object();
            titleObj.name = "title";
            titleObj.type = "title";
            titleObj.contents = "";
            for (var ix = 0; ix < currNode.childNodes.length; ++ix)
            {
              if (currNode.childNodes[ix].nodeName == "#text")
                titleObj.contents += currNode.childNodes[ix].nodeValue;
            }
            this.generalSettings["title"] = titleObj;
          }
          break;
          case "address":
          break;
          case "#comment":
          {
            var commentData = this.parseComment(currNode);
            if (commentData != null)
            {
              var commentName = commentData.name.toLowerCase();
              switch(commentName)
              {
                case "lastrevised":
                case "documentshell":
                case "language":
                case "outputfilter":
                case "version":
                case "bibliographyscheme":
                  subObject = this.generalSettings;
                break;
                case "graphicssave":
                case "viewsettings":
                case "viewpercent":
                case "noteviewsettings":
                case "noteviewpercent":
                case "saveformode":
                case "relativemetadatalinks":
                  subObject = this.saveSettings;
                break;
                case "printoptions":
                case "printviewpercent":
                  subObject = this.printSettings;
                break;
                case "comment":
                case "description":
                  subObject = this.comments;
                break;

                default:
                break;
              }
            }
            if (subObject != null)
              subObject[commentName] = commentData;
          }
          break;
        }
      }
    }

  };

  this.getDialogInfo = function()
  {
    var dlgInfo = new Object();
    this.setGeneralSettingsFromData(dlgInfo);
    this.setDialogCommentsFromData(dlgInfo);
//    dlgInfo.comments = this.comments;
    dlgInfo.metadata = this.metadata;
    this.setPrintFlagsFromData(dlgInfo);
    this.setSaveFlagsFromData(dlgInfo);
    dlgInfo.parentData = this;  //keep to use when the dialog completes (dialog should be modeless for copy-paste purposes)
    return dlgInfo;
  };

  this.resetFromDialogInfo = function(dlgInfo)
  {
    this.setDataFromGeneralSettings(dlgInfo);
    this.setDataFromDialogComments(dlgInfo);
//    this.comments = dlgInfo.comments;
    this.metadata = dlgInfo.metadata;
    this.setDataFromSaveFlags(dlgInfo);
    this.setDataFromPrintFlags(dlgInfo);
  };

  this.putDocInfoToDocument = function()
  {
    var docHead = msiGetDocumentHead(this.mEditor);
    var treeWalker = this.mEditor.document.createTreeWalker(docHead,
                                                            NodeFilter.SHOW_ELEMENT|NodeFilter.SHOW_COMMENT,
                                                            this.findMetadataNodes,
                                                            true);
    if (treeWalker)
    {
      for (var currNode = treeWalker.nextNode(); currNode != null; )
      {
        var objParent = null; 
        var objName = null;
        var theName = currNode.localName;
        if (!theName || !theName.length)
          theName = currNode.nodeName;
        switch(theName.toLowerCase())
        {
          case "meta":
          {
            var metaName = currNode.getAttribute("name").toLowerCase();
            var metaValue = currNode.getAttribute("content");
            if (metaName && metaValue && metaName.length && metaValue.length)
            {
              objName = metaName;
              switch(metaName)
              {
                case "created":
                case "lastrevised":
                case "language":
                  objParent = this.generalSettings;
                break;
                case "comment":
                case "description":
                  objParent = this.comments;
                break;
                default:
                  objParent = this.metadata;
                break;
              }
            }
          }
          break;
          case "link":
          {
            var linkName = currNode.getAttribute("rel").toLowerCase();
            var linkTarget = currNode.getAttribute("href");
            if (linkName && linkTarget && linkName.length && linkTarget.length)
            {
              objName = linkName;
              if (linkName == "documentshell")
                objParent = this.generalSettings;
              else
                objParent = this.metadata;
            }
          }
          break;
          case "title":
            objName = "title";
            objParent = this.generalSettings;
//            this.mDocumentTitle = "";
//            for (var ix = 0; ix < currNode.childNodes; ++ix)
//            {
//              if (currNode.childNodes[ix].nodeName == "#text")
//                this.mDocumentTitle += currNode.childNodes[ix].nodeValue;
//            }
          break;
          case "author":
          case "address":
          break;
          case "#comment":
          {
            var commentData = this.parseComment(currNode);
            if (commentData != null)
            {
              objName = commentData.name;
              switch(commentData.name)
              {
                case "lastrevised":
                case "documentshell":
                case "language":
                  objParent = this.generalSettings;
                break;
                case "graphicssave":
                case "viewsettings":
                case "viewpercent":
                case "noteviewsettings":
                case "noteviewpercent":
                case "relativemetadatalinks":
                  objParent = this.saveSettings;
                break;
                case "printoptions":
                case "printviewpercent":
                  objParent = this.printSettings;
                break;
                case "comment":
                case "description":
                  objParent = this.comments;
                break;

                case "outputfilter":
                case "version":
                case "saveformode":
                case "bibliographyscheme":
                break;

                default:
                break;
              }
            }
          }
          break;
        }
        var nextNode = treeWalker.nextNode();  //Do this way in case we need to delete "currNode" in the revise step.
        if (objParent && objName.length > 0)
          this.reviseIfChanged(objParent, objName, currNode);
        currNode = nextNode;
      }
    }

    var lastMetaNode = null;
    var lastLinkNode = null;
    var lastCommentNode = null;
    var newTreeWalker = this.mEditor.document.createTreeWalker(docHead,
                                                            NodeFilter.SHOW_ELEMENT|NodeFilter.SHOW_COMMENT,
                                                            this.findMetadataNodes,
                                                            true);
    if (newTreeWalker)
    {
      for (var currNode = newTreeWalker.nextNode(); currNode != null; currNode = newTreeWalker.nextNode())
      {
        var theName = currNode.localName;
        if (!theName || !theName.length)
          theName = currNode.nodeName;
        switch(theName.toLowerCase())
        {
          case "meta":
            lastMetaNode = currNode.nextSibling;
          break;
          case "link":
            lastLinkNode = currNode.nextSibling;
          break;
          case "#comment":
            lastCommentNode = currNode.nextSibling;
          break;
          default:
        }
      }
    }

    function insertNewNode(newNode)
    {
      var linebreakNode = document.createTextNode("\n");
      var theTest = [lastMetaNode, lastLinkNode, lastCommentNode];
      var theName = newNode.localName;
      if (!theName || theName.length == 0)
        theName = newNode.nodeName;
      switch(theName.toLowerCase())
      {
        case "meta":
        break;
        case "link":
          theTest = [lastLinkNode, lastMetaNode, lastCommentNode];
        break;
        case "#comment":
          theTest = [lastCommentNode, lastMetaNode, lastLinkNode];
        break;
        default:
          theTest = [];
        break;
      }
      var bDone = false;
      for (var ii = 0; !bDone && (ii < theTest.length); ++ii)
      {
        if (theTest[ii] != null)
        {
          docHead.insertBefore(linebreakNode, theTest[ii]);
          docHead.insertBefore(newNode, theTest[ii]);
          bDone = true;
        }
      }
      if (!bDone)
      {
        docHead.appendChild(newNode);
        docHead.appendChild(linebreakNode);
      }
    }

    var ourChildren = [this.generalSettings, this.saveSettings, this.printSettings, this.comments, this.metadata];
    for (var ix = 0; ix < ourChildren.length; ++ix)
    {
      for (var childObj in ourChildren[ix])
      {
        if (ourChildren[ix][childObj].status == "changed")
        {
          var newNode = this.createNewNode(ourChildren[ix][childObj], childObj);
          insertNewNode(newNode);
        }
      }
    }
//    for (var genObj in this.generalSettings)
//    {
//      if (this.generalSettings[genObj].status == "changed")
//      {
//        var newNode = this.createNewNode(this.generalSettings[genObj]);
//        insertNewNode(newNode);
//      }
//    }
//    for (var commObj in this.comments)
//    {
//      if (this.comments[commObj].status == "changed")
//      {
//        var newNode = this.createNewNode(this.comments[commObj]);
//        insertNewNode(newNode);
//      }
//    }
//    for (var printObj in this.printSettings)
//    {
//      if (this.printSettings[printObj].status == "changed")
//      {
//        var newNode = this.createNewNode(this.printSettings[printObj]);
//        insertNewNode(newNode);
//      }
//    }
//    for (var saveObj in this.saveSettings)
//    {
//      if (this.saveSettings[saveObj].status == "changed")
//      {
//        var newNode = this.createNewNode(this.saveSettings[saveObj]);
//        insertNewNode(newNode);
//      }
//    }

  };

  this.createNewNode = function(dataObj, ourName)
  {
    var newNode = null;
    switch(dataObj.type)
    {
      case "title":
      {
        var newTextNode = this.mEditor.document.createTextNode(dataObj.contents);
        newNode = this.mEditor.document.createElement("title");
        newNode.appendChild(newTextNode);
      }
      break;
      case "meta":
        newNode = this.mEditor.document.createElement("meta");
//        newNode.setAttribute("name", dataObj.name);
        newNode.setAttribute("name", ourName);
        newNode.setAttribute("content", dataObj.contents);
      break;
      case "link":
        newNode = this.mEditor.document.createElement("link");
//        newNode.setAttribute("rel", dataObj.name);
        newNode.setAttribute("rel", ourName);
        newNode.setAttribute("href", dataObj.uri);
      break;
      case "comment-meta":
      case "comment-meta-alt":
      case "comment-link":
      case "comment-link-alt":
      case "comment-key-value":
        newNode = this.createNewCommentNode(dataObj);
      break;
      default:
      break;
    }
    return newNode;
  };

  this.createNewCommentNode = function(dataObj)
  {
    var newNode = null;
    var nameStr = dataObj.name;
    var uriStr = "";
    var contentStr = "";
    if (("uri" in dataObj) && (dataObj.uri != null))
      uriStr = dataObj.uri;
    else if (("contents" in dataObj) && (dataObj.contents != null))
      contentStr = dataObj.contents;
    var theType = dataObj.type;
    if (contentStr.indexOf("\"") >= 0)
    {
      if (contentStr.indexOf("'") < 0)
      {
        if (theType == "comment-meta")
          theType = "comment-meta=alt";
        else if (theType == "comment-link")
          theType = "comment-link-alt";
      }
      else if (theType != "comment-key-value")
      {
        dump("Content string for comment contains both single and double quotes! Using \\ escape.\n");
        contentStr = contentStr.replace(/\\\"/, "&dblquote;");
        contentStr = contentStr.replace(/\\'/, "&singlequote;");
        contentStr = contentStr.replace(/\"/, "&dblquote;");
        contentStr = contentStr.replace(/'/, "&singlequote;");
        contentStr = contentStr.replace("&dblquote;", "\\\"");
        contentStr = contentStr.replace("&singlequote;", "\\'");
      }
    }  
    var commentStr = "";

    switch(theType)
    {
      case "comment-meta":
        commentStr = "TCIDATA{meta name=\"" + nameStr + "\" content=\"" + contentStr + "\"}";
      break;
      case "comment-meta-alt":
        commentStr = "TCIDATA{meta name='" + nameStr + "' content='" + contentStr + "'}";
      break;
      case "comment-link":
        commentStr = "TCIDATA{link rel=\"" + nameStr + "\" href=\"" + contentStr + "\"}";
      break;
      case "comment-link-alt":
        commentStr = "TCIDATA{link name='" + nameStr + "' href='" + contentStr + "'}";
      break;
      case "comment-key-value":
        commentStr = "TCIDATA{" + nameStr + "=" + contentStr + "}";
      break;
    }

    if (commentStr.length > 0)
      newNode = this.mEditor.document.createComment(commentStr);
    return newNode;
  };

  this.parseComment = function(commentNode)
  {
    //Here we're looking for something like "<!-- TCIDATA{<META NAME="GraphicsSave" CONTENT="32">} -->  ?
    //Preferred: <!-- TCIDATA META NAME="GraphicsSave" CONTENT="32" -->
    //But also may get: <!-- TCIDATA Version=5.50.0.2953 -->
    //Also, we probably don't want to expect the "TCIDATA" string. But some use of comments to store our proprietary
    //  information is probably desirable. Will have to be decided soon.
    var retVal = null;
    var theData = commentNode.data;  //Do we need to query for the Comment interface first?
    theData = theData.toLowerCase();

//    var tcidataRegExp = /tcidata[\s]*\{((?:(?:\\\})|(?:[^\}]))+)\}/i;
//	  var fullNameSyntax = /meta[\s]+.*name=\"((?:(?:\\\")|(?:[^\"]))+)\"/i;
//	  var fullContentsSyntax = /meta[\s]+.*content=\"((?:(?:\\\")|(?:[^\"]))+)\"/i;
//	  var altfullNameSyntax = /meta[\s]+.*name=\'((?:(?:\\\')|(?:[^\']))+)\'/i;
//	  var altfullContentsSyntax = /meta[\s]+.*content=\'((?:(?:\\\')|(?:[^\']))+)\'/i;
//	  var fullLinkSyntax = /link[\s]+.*rel=\"((?:(?:\\\")|(?:[^\"]))+)\"/i;
//	  var fullLinkRefSyntax = /link[\s]+.*href=\"((?:(?:\\\")|(?:[^\"]))+)\"/i;
//	  var altfullLinkSyntax = /link[\s]+.*rel=\'((?:(?:\\\')|(?:[^\']))+)\'/i;
//	  var altfullLinkRefSyntax = /link[\s]+.*href=\'((?:(?:\\\')|(?:[^\']))+)\'/i;
//	  var keyValueSyntax = /([\S]+)=(.*)/;
//	  var keyValueValueSyntax = /?:([\S]+)=(.*)/;

    dump("In parseComment, comment data is [" + theData + "].\n");
    var tciData = theData.match(this.tcidataRegExp);
    //NOTE! In JavaScript String.match(regExp), the first thing returned is the full matching expression; capturing-parentheses
    //  matches are returned in subsequent array members. So we're after array[1] in each case...
    if (tciData && (tciData.length > 1))  
    {
      var metaName = tciData[1].match(this.fullNameSyntax);
      if (metaName.length == 0)
        metaName = tciData.match(this.altfullNameSyntax);
      if (metaName.length > 1)
      {
        var theType = "comment-meta";
        var contents = tciData[1].match(this.fullContentsSyntax);
        if (contents.length == 0)
        {
          theType = "comment-meta-alt";
          contents = tciData[1].match(this.altfullContentsSyntax);
        }
        if (contents.length > 1)
        {
          retVal = new Object();
          retVal.name = metaName[1];
          retVal.contents = contents[1];
          retVal.type = theType;
        }
      }
      else
      {
        var linkName = tciData[1].match(this.fullLinkSyntax);
        var theType = "comment-link";
        if (linkName.length == 0)
          linkName = tciData[1].match(this.altfullLinkSyntax);
        if (linkName.length > 1)
        {
          var ref = tciData[1].match(this.fullLinkRefSyntax);
          if (ref.length == 0)
          {
            theType = "comment-link-alt";
            ref = tciData[1].match(this.altfullLinkRefSyntax);
          }
          if (ref.length > 1)
          {
            retVal = new Object();
            retVal.name = linkName[1];
            retVal.uri = ref[1];
            retVal.type = theType;
          }
        }
        else
        {
          var keyValuePair = tciData[1].match(this.keyValueSyntax);
          if (keyValuePair.length > 2)
          {
            retVal = new Object();
            retVal.name = keyValuePair[1];
            retVal.contents = keyValuePair[2];
            retVal.type = "comment-key-value";
          }
        }
      }
    }
    return retVal;
  };

  this.reviseComment = function(theObj, theNode)
  {
    var theData = theNode.data;  //Do we need to query for the Comment interface first?
    theData = theData.toLowerCase();

    var tcidataReplaceRegExp = /(tcidata[\s]*\{)((?:(?:\\\})|(?:[^\}]))+)\}/i;
//	  var fullNameReplaceSyntax = /meta[\s]+.*name=\"((?:(?:\\\")|(?:[^\"]))+)\"/i;
    var fullContentsReplaceSyntax = /(meta[\s]+.*content=\")((?:(?:\\\")|(?:[^\"]))+)\"/i;
//	  var altfullNameReplaceSyntax = /meta[\s]+.*name=\'((?:(?:\\\')|(?:[^\']))+)\'/i;
    var altfullContentsReplaceSyntax = /(meta[\s]+.*content=\')((?:(?:\\\')|(?:[^\']))+)\'/i;
//	  var fullLinkReplaceSyntax = /link[\s]+.*rel=\"((?:(?:\\\")|(?:[^\"]))+)\"/i;
    var fullLinkRefReplaceSyntax = /(link[\s]+.*href=\")((?:(?:\\\")|(?:[^\"]))+)\"/i;
//	  var altfullLinkReplaceSyntax = /link[\s]+.*rel=\'((?:(?:\\\')|(?:[^\']))+)\'/i;
    var altfullLinkRefReplaceSyntax = /(link[\s]+.*href=\')((?:(?:\\\')|(?:[^\']))+)\'/i;
    var keyValueReplaceSyntax = /((?:[\S]+)=)(.*)/;

    dump("In reviseComment, comment data is [" + theData + "].\n");
    var tciData = theData.match(tcidataReplaceRegExp);

    if (tciData && (tciData.length > 2))
    {
      var newData = "";
      switch(theObj.type)
      {
        case "comment-meta":
          newData = tciData[2].replace(fullContentsReplaceSyntax, "$`$1" + theObj.contents + "\"$'");
        break;
        case "comment-meta-alt":
          newData = tciData[2].replace(altfullContentsReplaceSyntax, "$`$1" + theObj.contents + "'$'");
        break;
        case "comment-link":
          newData = tciData[2].replace(fullLinkRefReplaceSyntax, "$`$1" + theObj.uri + "\"$'");
        break;
        case "comment-link-alt":
          newData = tciData[2].replace(altfullLinkRefReplaceSyntax, "$`$1" + theObj.uri + "'$'");
        break;
        case "comment-key-value":
          newData = tciData[2].replace(keyValueReplaceSyntax, "$`$1" + theObj.contents + "$'");
        break;
      }
      if (newData.length > 0)
        theNode.data = theData.replace(tcidataReplaceRegExp, "$`$1" + newData + "}$'");
    }
  };

  this.setGeneralSettingsFromData = function(dlgInfo)
  {
    if (!("general" in dlgInfo) || (dlgInfo.general == null))
      dlgInfo.general = new Object();
    if (this.generalSettings)
    {
      dlgInfo.general.documentUri = msiGetEditorURL(this.mEditorElement);
      if (("created" in this.generalSettings) && (this.generalSettings.created != null))
        dlgInfo.general.created = this.generalSettings.created.contents;
      if (("lastrevised" in this.generalSettings) && (this.generalSettings.lastrevised != null))
        dlgInfo.general.lastRevised = this.generalSettings.lastrevised.contents;
      if (("language" in this.generalSettings) && (this.generalSettings.language != null))
        dlgInfo.general.language = this.generalSettings.language.contents;
      if (("documentshell" in this.generalSettings) && (this.generalSettings.documentshell != null))
        dlgInfo.general.documentShell = this.generalSettings.documentshell.contents;
      if (("title" in this.generalSettings) && (this.generalSettings.title != null))
        dlgInfo.general.documentTitle = this.generalSettings.title.contents;
    }
  };

  this.setDataFromGeneralSettings = function(dlgInfo)
  {
    if (!this.generalSettings)
    {
      dump("In msiDocumentInfo.setDataFromGeneralSettings, generalSettings object is missing!\n");
      this.generalSettings = new Object();
    }
    var theContents = "";
    if (("created" in dlgInfo.general) && (dlgInfo.general.created != null))
      theContents = dlgInfo.general.created;
    this.setObjectFromData(this.generalSettings, "created", (theContents.length > 0), "Created", theContents, "meta");

    theContents = "";
    if (("lastRevised" in dlgInfo.general) && (dlgInfo.general.lastRevised != null))
      theContents = dlgInfo.general.lastRevised;
    this.setObjectFromData(this.generalSettings, "lastrevised", (theContents.length > 0), "LastRevised", theContents, "meta");

    theContents = "";
    if (("language" in dlgInfo.general) && (dlgInfo.general.language != null))
      theContents = dlgInfo.general.language;
    this.setObjectFromData(this.generalSettings, "language", (theContents.length > 0), "Language", theContents, "meta");

    theContents = "";
    if (("documentShell" in dlgInfo.general) && (dlgInfo.general.documentShell != null))
      theContents = dlgInfo.general.documentShell;
    this.setObjectFromData(this.generalSettings, "documentshell", (theContents.length > 0), "DocumentShell", theContents, "comment-link");

    theContents = "";
    if (("documentTitle" in dlgInfo.general) && (dlgInfo.general.documentTitle != null))
      theContents = dlgInfo.general.documentTitle;
    this.setObjectFromData(this.generalSettings, "title", (theContents.length > 0), "Title", theContents, "title");
  };

  this.setDialogCommentsFromData = function(dlgInfo)
  {
    if (!("comments" in dlgInfo) || (dlgInfo.comments == null))
      dlgInfo.comments = new Object();
    dlgInfo.comments.comment = "";
    if (("comment" in this.comments) && ("contents" in this.comments.comment))
      dlgInfo.comments.comment = this.comments.comment.contents;
    dlgInfo.comments.description = "";
    if (("description" in this.comments) && ("contents" in this.comments.description))
      dlgInfo.comments.description = this.comments.description.contents;
  };

  this.setDataFromDialogComments = function(dlgInfo)
  {
    if (!this.comments)
    {
      dump("In msiDocumentInfo.setDataFromDialogComments, comments object is missing!\n");
      this.comments = new Object();
    }
    var theContents = dlgInfo.comments.comment;
    this.setObjectFromData(this.comments, "comment", (theContents.length > 0), "Comment", theContents, "meta");

    theContents = dlgInfo.comments.description;
    this.setObjectFromData(this.comments, "description", (theContents.length > 0), "Description", theContents, "meta");
  };

  this.setPrintFlagsFromData = function(dlgInfo)
  {
    if (!("printOptions" in dlgInfo) || (dlgInfo.printOptions == null))
      dlgInfo.printOptions = new Object();

    var theFlags = 0;
    if (this.printSettings && this.printSettings.printoptions)
      theFlags = this.printSettings.printoptions.contents.valueOf();
    var bUseDefaultPrintOptions = (theFlags == 0);
    if (bUseDefaultPrintOptions)
      dlgInfo.printOptions.theOptions = msiGetDefaultPrintOptions();  //in msiEditorUtilities.js
    else
      dlgInfo.printOptions.theOptions = new msiPrintOptions(theFlags);  //in msiEditorUtilities.js
    if (dlgInfo.printOptions.theOptions.useCurrViewSettings)
      dlgInfo.printOptions.theOptions.reflectViewSettings(this.mEditorElement);
    dlgInfo.printOptions.theOptions.useDefaultPrintOptions = bUseDefaultPrintOptions;

    dlgInfo.printOptions.zoomPercentage = 100;
    if (dlgInfo.printOptions.theOptions.useCurrViewZoom)
      dlgInfo.printOptions.zoomPercentage = msiGetCurrViewPerCent(this.mEditorElement);  //in msiEditorUtilities.js, though not yet really implemented
    else if (this.printSettings.printviewpercent != null && this.printSettings.printviewpercent.contents != null)
      dlgInfo.printOptions.zoomPercentage = this.printSettings.printviewpercent.contents.valueOf();
  };

  this.setDataFromPrintFlags = function(dlgInfo)
  {
    if (!this.printSettings)
    {
      dump("In msiDocumentInfo.setDataFromPrintFlags, printSettings object is missing!\n");
      this.printSettings = new Object();
    }

    if (!this.printSettings.printoptions)
    {
      this.printSettings.printoptions = new Object();
      this.printSettings.printoptions.name = "PrintOptions";
      this.printSettings.printoptions.type = "comment-meta";
    }

    var printFlags = dlgInfo.printOptions.theOptions.getFlags();
    if (!("contents" in this.printSettings.printoptions) || (this.printSettings.printoptions.contents == null)
                   || (this.printSettings.printoptions.contents.valueOf() != printFlags))
    {
      this.printSettings.printoptions.status = "changed";
      this.printSettings.printoptions.contents = String(printFlags);
    }
    else
      this.printSettings.printoptions.status = "unchanged";

    if (!dlgInfo.printOptions.theOptions.useCurrViewZoom)
    {
      if (!this.printSettings.printviewpercent)
      {
        this.printSettings.printviewpercent = new Object();
        this.printSettings.printviewpercent.name = "PrintViewPercent";
        this.printSettings.printviewpercent.type = "comment-meta";
      }
      var zoomPercent = 100;
      if  (("zoomPercentage" in dlgInfo.printOptions) && (dlgInfo.printOptions.zoomPercentage != null))
        zoomPercent = dlgInfo.printOptions.zoomPercentage;
      if (!("contents" in this.printSettings.printviewpercent) || (this.printSettings.printviewpercent.contents == null)
                  || (this.printSettings.printviewpercent.contents.valueOf() != zoomPercent))
      {
        this.printSettings.printviewpercent.status = "changed";
        this.printSettings.printviewpercent.contents = String(zoomPercent);
      }
      else
        this.printSettings.printviewpercent.status = "unchanged";
    }
    else if (("printviewpercent" in this.printSettings) && (this.printSettings.printviewpercent != null))
      this.printSettings.printviewpercent.status = "deleted";
  };

  this.setSaveFlagsFromData = function(dlgInfo)
  {
    if (!("saveOptions" in dlgInfo) || (dlgInfo.saveOptions == null))
      dlgInfo.saveOptions = new Object();

    if ("graphicssave" in this.saveSettings && this.saveSettings.graphicssave != null)
    {
      var nValue = this.saveSettings.graphicssave.contents.valueOf();
      dlgInfo.saveOptions.useRelativeGraphicsPaths = ((nValue & this.graphicsSaveRelativeFlag) != 0);
    }
    else
      dlgInfo.saveOptions.useRelativeGraphicsPaths = true;

    if (("viewsettings" in this.saveSettings) && this.saveSettings.viewsettings != null)
      dlgInfo.saveOptions.storeViewSettings = true;
    else
      dlgInfo.saveOptions.storeViewSettings = false;

    if (("viewpercent" in this.saveSettings) && this.saveSettings.viewpercent != null)
      dlgInfo.saveOptions.storeViewPercent = true;
    else
      dlgInfo.saveOptions.storeViewPercent = false;

    if (("noteviewsettings" in this.saveSettings) && this.saveSettings.noteviewsettings != null)
      dlgInfo.saveOptions.storeNoteViewSettings = true;
    else
      dlgInfo.saveOptions.storeNoteViewSettings = false;

    if (("noteviewpercent" in this.saveSettings) && this.saveSettings.noteviewpercent != null)
      dlgInfo.saveOptions.storeNoteViewPercent = true;
    else
      dlgInfo.saveOptions.storeNoteViewPercent = false;

    if (("relativemetadatalinks" in this.saveSettings) && this.saveSettings.relativemetadatalinks != null)
    {
      var nValue = this.saveSettings.relativemetadatalinks.contents.valueOf();
      dlgInfo.saveOptions.relativeMetadataLinks = (nValue != 0);
    }
    else
      dlgInfo.saveOptions.relativeMetadataLinks = false;

  };

  this.setDataFromSaveFlags = function(dlgInfo)
  {
    var theContents = 0;
    if (dlgInfo.saveOptions.useRelativeGraphicsPaths)
      theContents = this.graphicsSaveRelativeFlag;
    this.setObjectFromData(this.saveSettings, "graphicssave", dlgInfo.saveOptions.useRelativeGraphicsPaths, "GraphicsSave", String(theContents), "comment-meta");

    theContents = 1;
    if (dlgInfo.saveOptions.storeViewSettings)
      theContents = msiGetCurrViewSettings(this.mEditorElement).getFlags();
    this.setObjectFromData(this.saveSettings, "viewsettings", dlgInfo.saveOptions.storeViewSettings, "ViewSettings", String(theContents), "comment-meta");

    theContents = msiGetCurrViewPerCent(this.mEditorElement);
    this.setObjectFromData(this.saveSettings, "viewpercent", dlgInfo.saveOptions.storeViewPercent, "ViewPercent", String(theContents), "comment-meta");

    theContents = 1;
    if (dlgInfo.saveOptions.storeNoteViewSettings)
      theContents = msiGetCurrNoteViewSettings(this.mEditorElement).getFlags();
    this.setObjectFromData(this.saveSettings, "noteviewsettings", dlgInfo.saveOptions.storeNoteViewSettings, "NoteViewSettings", String(theContents), "comment-meta");

    theContents = GetIntPref("NoteViewPercent");
    if (!theContents)
      theContents = 100;
    this.setObjectFromData(this.saveSettings, "noteviewpercent", dlgInfo.saveOptions.storeNoteViewPercent, "NoteViewPercent", String(theContents), "comment-meta");

    var theContents = 0;
    if (dlgInfo.saveOptions.relativeMetadataLinks)
    {
      theContents = dlgInfo.saveOptions.relativeMetadataLinks;
//      dump("In msiComposerCommands, in msiDocumentInfo.setDataFromSaveFlags, dlgInfo.saveOptions.relativeMetadataLinks was [" + theContents + "].\n");
    }
//    else
//      dump("In msiComposerCommands, in msiDocumentInfo.setDataFromSaveFlags, dlgInfo.saveOptions.relativeMetadataLinks was empty!\n");
    this.setObjectFromData(this.saveSettings, "relativemetadatalinks", dlgInfo.saveOptions.relativeMetadataLinks, "relativeMetadataLinks", String(theContents), "comment-meta");
//    dump("In msiComposerCommands, in msiDocumentInfo.setDataFromSaveFlags after setting relativemetadatalinks, value is [" + this.saveSettings.relativemetadatalinks + "].\n");
  };

  this.setObjectFromData = function(theParent, theObject, bSet, theName, theContents, theType)
  {
    if (!theParent)
    {
      dump("Error in msiDocumentInfo.setObjectFromData = null parent object passed in!\n");
      return;
    }
    if (bSet && (!(theObject in theParent) || (theParent[theObject] == null)) )
    {
      theParent[theObject] = new Object();
      theParent[theObject].name = theName;
      theParent[theObject].type = theType;
      theParent[theObject].status = "changed";
    }
    if ((theObject in theParent) && (theParent[theObject] != null))
    {
      theParent[theObject].status = "changed";
      if (theParent[theObject].type == theType)
      {
        if ( ("uri" in theParent[theObject]) && (theParent[theObject].uri != null) )
        {
          if (theParent[theObject].uri == theContents)
            theParent[theObject].status = "unchanged";
        }
        else if ( ("contents" in theParent[theObject]) && (theParent[theObject].contents != null) )
        {
          if (theParent[theObject].contents == theContents)
            theParent[theObject].status = "unchanged";
        }
      }
      if (bSet)
      {
        if (theType == "link")
          theParent[theObject].uri = theContents;
        else
          theParent[theObject].contents = theContents;
      }
      else
        theParent[theObject].status = "deleted";
    }
  };

  this.reviseIfChanged = function(parent, objName, theNode)
  {
    if ((objName in parent) && (parent[objName] != null))
    {
      if (parent[objName].status == "changed")
      {
        switch(parent[objName].type)
        {
          case "meta":
            theNode.setAttribute("content", parent[objName].contents);
            parent[objName].status = "done";
          break;
          case "link":
            theNode.setAttribute("href", parent[objName].uri);
            parent[objName].status = "done";
          break;
          case "comment-meta":
          case "comnment-meta-alt":
          case "comment-link":
          case "comment-link-alt":
          case "comment-key-value":
            this.reviseComment(parent[objName], theNode);
            parent[objName].status = "done";
          break;
          case "title":
          {
            var textNodes = theNode.childNodes;
            var bDone = false;
            for (var ix = 0; ix < textNodes.length; ++ix)
            {
              if ( (textNodes[ix].nodeName == "#text") && (textNodes[ix].nodeValue.length > 0) )
              {
                if (!bDone)
                {
                  textNodes[ix].nodeValue = parent[objName].contents;
                  bDone = true;
                }
                else
                  textNodes[ix].nodeValue = "";
              }
            }
            if (!bDone)
            {
              var newNode = theNode.ownerDocument.createTextNode(parent[objName].contents);
              theNode.appendChild(newNode);
            }
            parent[objName].status = "done";
          }
          break;
        }
      }
      else if ( (parent[objName].status == "deleted") || (parent[objName].status == "done") )
      {
        var prevSibling = theNode.previousSibling;
        theNode.parentNode.removeChild(theNode);
        if ((prevSibling != null) && (prevSibling.nodeName == "#text"))
        {
          theText = TrimString(prevSibling.nodeValue);
          if (!theText.length)
            theNode.parentNode.removeChild(prevSibling);
        }
        parent[objName].status = "done";
      }
    }
  };

  this.getDocumentViewSettings = function()
  {
    if (("viewsettings" in this.saveSettings) && this.saveSettings.viewsettings != null)
    {
      var viewFlags = this.saveSettings.viewsettings.contents.valueOf();
      return new msiViewSettings(viewFlags);
    }
    return null;
  };

  this.getDocumentNoteViewSettings = function()
  {
    if (("noteviewsettings" in this.saveSettings) && this.saveSettings.noteviewsettings != null)
    {
      var viewFlags = this.saveSettings.noteviewsettings.contents.valueOf();
      return new msiViewSettings(viewFlags);
    }
    return null;
  };

  this.tcidataRegExp = /tcidata[\s]*\{((?:(?:\\\})|(?:[^\}]))+)\}/i;
  this.fullNameSyntax = /meta[\s]+.*name=\"((?:(?:\\\")|(?:[^\"]))+)\"/i;
  this.fullContentsSyntax = /meta[\s]+.*content=\"((?:(?:\\\")|(?:[^\"]))+)\"/i;
  this.altfullNameSyntax = /meta[\s]+.*name=\'((?:(?:\\\')|(?:[^\']))+)\'/i;
  this.altfullContentsSyntax = /meta[\s]+.*content=\'((?:(?:\\\')|(?:[^\']))+)\'/i;
  this.fullLinkSyntax = /link[\s]+.*rel=\"((?:(?:\\\")|(?:[^\"]))+)\"/i;
  this.fullLinkRefSyntax = /link[\s]+.*href=\"((?:(?:\\\")|(?:[^\"]))+)\"/i;
  this.altfullLinkSyntax = /link[\s]+.*rel=\'((?:(?:\\\')|(?:[^\']))+)\'/i;
  this.altfullLinkRefSyntax = /link[\s]+.*href=\'((?:(?:\\\')|(?:[^\']))+)\'/i;
  this.keyValueSyntax = /([\S]+)=(.*)/;
  this.keyValueValueSyntax = /(?:[\S]+)=(.*)/;

}

var msiDocumentInfoBase =
{
  graphicsSaveRelativeFlag:             0x20,
  printShowInvisiblesFlag:                 1,
  printShowMatrixLinesFlag:                2,
  printShowInputBoxesFlag:                 4,
  printShowIndexFieldsFlag:                8,  
  printShowMarkerFieldsFlag:          0x0010,
  printUseViewSettingsFlag:           0x0020,
  printUseViewOverridesFlag:          0x0040,
  printBlackTextFlag:                 0x0080,
  printBlackLinesFlag:                0x0100,
  printTransparentBackgroundFlag:     0x0200,
  printTransparentGrayButtonsFlag:    0x0400,
  printSuppressGrayButtonsFlag:       0x0800,
  printUseViewSettingZoomFlag:        0x2000
};

msiDocumentInfo.prototype = msiDocumentInfoBase;

//-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
var msiDocumentStyleCommand = 
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetTopLevelEditorElement();
    if (msiGetEditorURL(editorElement).length > 0)
      return (msiIsDocumentEditable(editorElement) && msiIsEditingRenderedHTML(editorElement));
    return false;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    // Launch Document Style dialog
    var editorElement = msiGetTopLevelEditorElement();
    var documentStyle = {};
    documentStyle.edElement = editorElement;

    try {
      msiOpenModelessDialog("chrome://prince/content/DocumentStyle.xul", "_blank", 
                                        "chrome,close,titlebar,dependent, resizable",
                                        editorElement, "cmd_documentInfo", this, documentStyle);
    }
    catch(ex) {
      dump("*** Exception: couldn't open DocStyle Dialog: " + ex + "\n");
    }
    editorElement.contentWindow.focus();
  }
};


function msiFinishDocumentStyleDialog(editorElement, documentStyle)
{
  if (!documentStyle.cancel)
  {
// inline this    putDocStyleInDocument(editorElement, documentStyle);
    alert("Hi!");
  }
}

var msiViewInvisiblesCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
//    updateViewMenuFromEditor(editorElement);
    return (msiIsDocumentEditable(editorElement) && msiIsEditingRenderedHTML(editorElement));
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement();
    updateEditorFromViewMenu(editorElement);
  }

};

//-----------------------------------------------------------------------------------

var msiNoteCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();

    return (msiIsDocumentEditable(editorElement) && msiIsEditingRenderedHTML(editorElement));
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement();
    //temporary
    // need to get current note if it exists -- if none, initialize as follows 
    msiNote(null, editorElement);
  }
};

//-----------------------------------------------------------------------------------

var msiCitationCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();

    return (msiIsDocumentEditable(editorElement) && msiIsEditingRenderedHTML(editorElement));
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement();
    //temporary
    // need to get current note if it exists -- if none, initialize as follows 
    doInsertCitation(editorElement, "cmd_citation", this);
  }
};

//-----------------------------------------------------------------------------------

var msiFrameCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();

    return (msiIsDocumentEditable(editorElement) && msiIsEditingRenderedHTML(editorElement));
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement();
    //temporary
    // need to get current note if it exists -- if none, initialize as follows 
    msiFrame(null, editorElement);
  }
};

//-----------------------------------------------------------------------------------
var msiObjectPropertiesCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var isEnabled = false;
    var editorElement = msiGetActiveEditorElement();
    if (msiIsDocumentEditable(editorElement) && msiIsEditingRenderedHTML(editorElement))
    {
      var nodeData = msiGetObjectDataForProperties(editorElement);
      var editor = msiGetEditor(editorElement);
      isEnabled = ( (nodeData != null && nodeData.theNode != null) ||
                    (editor != null && editor.getSelectedElement("href") != null) );
    }
    return isEnabled;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    // Launch Object properties for appropriate selected element 
    var editorElement = msiGetActiveEditorElement();
    var nodeData = msiGetObjectDataForProperties(editorElement);
    var element = nodeData.theNode;
    if (element)
    {
      if (element.nodeType == nsIDOMNode.TEXT_NODE)
      {
        if (nodeData.theOffset != null)
        {
          //Need to bring up the ReviseCharacter dialog. Not yet implemented.
          var theCharacter = element.data.charAt(nodeData.theOffset);
        }
        else
        {
          dump("No offset specified in text node for Properties dialog! Aborting.\n");
          return;
        }
      }
      var cmdParams = newCommandParams();
      if (!cmdParams)
      {
        dump("Trouble in msiObjectPropertiesCommand.doCommand! Can't create new CommandParams - aborting.\n");
        return;
      }

      var name = msiGetBaseNodeName(element).toLowerCase();
      cmdParams.setISupportsValue("reviseObject", element);
      var wrappedChildElement = element;
      while ((name == 'mstyle') || (name == 'mrow') )
      {
        var newChildElement = msiNavigationUtils.getSingleWrappedChild(wrappedChildElement);
        if (newChildElement == null)
          break;
        wrappedChildElement = newChildElement;
        name = msiGetBaseNodeName(wrappedChildElement).toLowerCase();
      }

      switch (name)
      {
        case 'img':
          msiGoDoCommandParams("cmd_reviseImage", cmdParams, editorElement);
        break;
        case 'hr':
          msiGoDoCommandParams("cmd_reviseLine", cmdParams, editorElement);
        break;
        case 'form':
          msiGoDoCommandParams("cmd_reviseForm", cmdParams, editorElement);
        break;
        case 'input':
          var type = element.getAttribute("type");
          if (type && type.toLowerCase() == "image")
            msiGoDoCommand("cmd_inputimage", editorElement);
          else
            msiGoDoCommand("cmd_inputtag", editorElement);
        break;
        case 'textarea':
          msiGoDoCommandParams("cmd_reviseTextarea", cmdParams, editorElement);
        break;
        case 'select':
//          msiGoDoCommand("cmd_select", editorElement);
//  Don't think we want to support this???
        break;
        case 'button':
          msiGoDoCommandParams("cmd_reviseButton", cmdParams, editorElement);
        break;
        case 'label':
          msiGoDoCommandParams("cmd_reviseLabel", cmdParams, editorElement);
        break;
        case 'fieldset':
          msiGoDoCommandParams("cmd_reviseFieldset", cmdParams, editorElement);
        break;

        case 'table':
          msiEditorInsertOrEditTable(false, editorElement, "cmd_objectProperties", this);
        break;
        case 'td':
        case 'th':
          msiEditorTableCellProperties(editorElement);
        break;

        case 'ol':
        case 'ul':
        case 'dl':
        case 'li':
          msiGoDoCommand("cmd_listProperties", editorElement);
        break;

        case 'a':
          if (element.name)
          {
            msiGoDoCommandParams("cmd_reviseAnchor", cmdParams, editorElement);
          }
          else if(element.href)
          {
            msiGoDoCommandParams("cmd_msiReviseHyperlink", cmdParams, editorElement);
          }
        break;

        case 'hspace':
          msiGoDoCommandParams("cmd_reviseHorizontalSpaces", cmdParams, editorElement);
        break;

        case 'vspace':
          msiGoDoCommandParams("cmd_reviseVerticalSpaces", cmdParams, editorElement);
        break;

        case 'msirule':
          msiGoDoCommandParams("cmd_msiReviseRules", cmdParams, editorElement);
        break;

        case 'msibreak':
          msiGoDoCommandParams("cmd_msiReviseBreaks", cmdParams, editorElement);
        break;

        case 'mfrac':
          msiGoDoCommandParams("cmd_MSIreviseFractionCmd", cmdParams, editorElement);
        break;

        case 'mroot':
        case 'msqrt':
          msiGoDoCommandParams("cmd_MSIreviseRadicalCmd", cmdParams, editorElement);
        break;

        case 'msub':
        case 'msup':
        case 'msubsup':
          if (msiNavigationUtils.getEmbellishedOperator(wrappedChildElement) != null)
            msiGoDoCommandParams("cmd_MSIreviseOperatorsCmd", cmdParams, editorElement);
//          msiGoDoCommandParams("cmd_MSIreviseScriptsCmd", cmdParams, editorElement);
// Should be no Properties dialog available for these cases? SWP has none...
        break;

        case 'mmatrix':
          msiGoDoCommandParams("cmd_MSIreviseMatrixCmd", cmdParams, editorElement);
        break;

        case 'mmultiscripts':
          if (msiNavigationUtils.getEmbellishedOperator(wrappedChildElement) != null)
            msiGoDoCommandParams("cmd_MSIreviseOperatorsCmd", cmdParams, editorElement);
          else
            msiGoDoCommandParams("cmd_MSIreviseTensorCmd", cmdParams, editorElement);
        break;


        case 'mi':
          if (msiNavigationUtils.isUnit(wrappedChildElement))
            msiGoDoCommandParams("cmd_MSIreviseUnitsCommand", cmdParams, editorElement);
          else if (msiNavigationUtils.isMathname(wrappedChildElement))
            msiGoDoCommandParams("cmd_MSIreviseMathnameCmd", cmdParams, editorElement);
        break;

//    commandTable.registerCommand("cmd_MSIreviseSymbolCmd",    msiReviseSymbolCmd);

        case 'mstyle':
          if (msiNavigationUtils.isFence(wrappedChildElement))
          {
            if (msiNavigationUtils.isBinomial(wrappedChildElement))
              msiGoDoCommandParams("cmd_MSIreviseBinomialsCmd", cmdParams, editorElement);
            else
              msiGoDoCommandParams("cmd_MSIreviseGenBracketsCmd", cmdParams, editorElement);
          }
        break;

        case 'mrow':
          if (msiNavigationUtils.isFence(wrappedChildElement))
          {
            if (msiNavigationUtils.isBinomial(wrappedChildElement))
              msiGoDoCommandParams("cmd_MSIreviseBinomialsCmd", cmdParams, editorElement);
            else
              msiGoDoCommandParams("cmd_MSIreviseGenBracketsCmd", cmdParams, editorElement);
          }
        break;

        case 'mo':
          if (msiNavigationUtils.isMathname(wrappedChildElement))
            msiGoDoCommandParams("cmd_MSIreviseMathnameCmd", cmdParams, editorElement);
          else
            msiGoDoCommandParams("cmd_MSIreviseOperatorsCmd", cmdParams, editorElement);
        break;

        case 'mover':
        case 'munder':
        case 'munderover':
          if (msiNavigationUtils.getEmbellishedOperator(wrappedChildElement) != null)
            msiGoDoCommandParams("cmd_MSIreviseOperatorsCmd", cmdParams, editorElement);

          else
            msiGoDoCommandParams("cmd_MSIreviseDecorationsCmd", cmdParams, editorElement);
        break;

        default:
          msiDoAdvancedProperties(element, editorElement);
          break;
      }
    } else {
      // We get a partially-selected link if asked for specifically
      try {
        element = msiGetEditor(editorElement).getSelectedElement("href");
      } catch (e) {}
      if (element)
      {
        var cmdParams = newCommandParams();
        if (!cmdParams)
        {
          dump("Trouble in msiObjectPropertiesCommand.doCommand! Can't create new CommandParams - aborting.\n");
          return;
        }

        cmdParams.setISupportsValue("reviseObject", element);
        msiGoDoCommandParams("cmd_msiReviseHyperlink", cmdParams, editorElement);
//        msiGoDoReviseCommand("cmd_link", editorElement);
      }
    }
    editorElement.contentWindow.focus();
  },

  msiGetReviseObject: function(editor)
  {
    return msiEditorGetObjectForProperties(editor);
  }

};


////-----------------------------------------------------------------------------------
var msiSmileyCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    try
    {
      var editorElement = msiGetActiveEditorElement();
      return (msiIsDocumentEditable(editorElement) && msiIsEditingRenderedHTML(editorElement));
    }
    catch(exc) {AlertWithTitle("Error in msiComposerCommands.js", "Error in msiSmileyCommand.isCommandEnabled: " + exc);}
    return false;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon)
  {
    var smileyCode = aParams.getCStringValue("state_attribute");

    var strSml;
    switch(smileyCode)
    {
        case ":-)": strSml="s1";
        break;
        case ":-(": strSml="s2";
        break;
        case ";-)": strSml="s3";
        break;
        case ":-P":
        case ":-p":
        case ":-b": strSml="s4";
        break;
        case ":-D": strSml="s5";
        break;
        case ":-[": strSml="s6";
        break;
        case ":-\\":
        case ":\\": strSml="s7";
        break;
        case "=-O":
        case "=-o": strSml="s8";
        break;
        case ":-*": strSml="s9";
        break;
        case ">:o":
        case ">:-o": strSml="s10";
        break;
        case "8-)": strSml="s11";
        break;
        case ":-$": strSml="s12";
        break;
        case ":-!": strSml="s13";
        break;
        case "O:-)":
        case "o:-)": strSml="s14";
        break;
        case ":'(": strSml="s15";
        break;
        case ":-X":
        case ":-x": strSml="s16";
        break;
        default:	strSml="";
        break;
    }

    try
    {
      var editorElement = msiGetActiveEditorElement();
      var editor = msiGetEditor(editorElement);
//      var editor = GetCurrentEditor();
      var selection = editor.selection;
      var extElement = editor.createElementWithDefaults("span");
      extElement.setAttribute("class", "moz-smiley-" + strSml);

      var intElement = editor.createElementWithDefaults("span");
      if (!intElement)
        return;

      //just for mailnews, because of the way it removes HTML
      var topWindow = msiGetTopLevelWindow();
      var smileButMenu = topWindow.document.getElementById('smileButtonMenu');      
      if (smileButMenu.getAttribute("padwithspace"))
         smileyCode = " " + smileyCode + " ";

      var txtElement =  editor.document.createTextNode(smileyCode);
      if (!txtElement)
        return;

      intElement.appendChild (txtElement);
      extElement.appendChild (intElement);


      editor.insertElementAtSelection(extElement,true);
      editorElement.contentWindow.focus();
//      window.content.focus();		

    } 
    catch (e) 
    {
      dump("Exception occured in smiley InsertElementAtSelection: " + e + ".\n");
    }
  },
  // This is now deprecated in favor of "doCommandParams"
  doCommand: function(aCommand) {}
};

//
function msiDoAdvancedProperties(element, editorElement)
{
  try
  {
    if (!editorElement)
      editorElement = findEditorElementForDocument(element.ownerDocument);
    //use which msiopendialog function here?
    var dlgParentWindow = msiGetWindowContainingEditor(editorElement);
    if (element)
    {
      if (element.role || element.localName == "texb")
      {
        var data = new Object();
        if (element.role == "texbutton" || element.localName == "texb")
        {
        // security restrictions prohibit calling openDialog from within XBL code,
        // but we don't want to hard-wire tag names in this code. Thus, the compromise
        // is to create 'roles' for elements, attach behavior to roles (as we do here)
        // and the XBL code will then assign a role to an element. Thus we have 'texbutton'
        // as a role, and currently that role is played by texb tags, but any other tag
        // could play this role as well. 
          try {
            data.tex = element.firstChild.nodeValue;
            dlgParentWindow.openDialog("chrome://prince/content/texbuttoncontents.xul","_blank","chrome,close,titlebar,resizable=yes,modal", data);
            editorElement.contentWindow.focus();
            if (!data.Cancel)
            {
              element.firstChild.nodeValue = data.tex;
            }
          }
          catch (e)
          { dump(e); }
        }
        else if (element.role == "latexstylebutton")
        {
          if (element.prop == "pagenumber")
          {
            try
            {
              data.numstyle = element.value;
              dlgParentWindow.openDialog("chrome://prince/content/latexpagenumberstyle.xul", "_blank", "chrome,close,titlebar,modal,resizable=yes", data);
              editorElement.contentWindow.focus();
              if (!data.Cancel)
              {
                element.value = data.numstyle;
              }
            }
            catch (e)
            { dump(e); }
          }
          else if (element.prop == "headers")
          {
            try {
              data.lheader = element.value;
              data.rheader = element.value2;
              dlgParentWindow.openDialog("chrome://prince/content/latexheaders.xul", "_blank", "chrome,close,titlebar,modal,resizable=yes", data);
              editorElement.contentWindow.focus();
              if (!data.Cancel)
              {
                element.value = data.lheader;
                element.value2 = data.rheader;
              }
            }
            catch (e)
            { dump(e); }
          }
        }
      }      
      else
      {
        dlgParentWindow.openDialog("chrome://editor/content/EdAdvancedEdit.xul", "_blank", "chrome,close,titlebar,modal,resizable=yes", "", element);
        editorElement.contentWindow.focus();
      }
    }
  }
  catch(exc) {AlertWithTitle("Error in msiComposerCommands.js", "Error in msiDoAdvancedProperties: " + exc);}
}

var msiAdvancedPropertiesCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return (msiIsDocumentEditable() && msiIsEditingRenderedHTML());
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    // Launch AdvancedEdit dialog for the selected element
    try
    {
      var editorElement = msiGetActiveEditorElement();
      var element = msiGetEditor(editorElement).getSelectedElement("");
      msiDoAdvancedProperties(element, editorElement);
    } catch (e) {}
  }
};

//-----------------------------------------------------------------------------------
var msiColorPropertiesCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    return (msiIsDocumentEditable(editorElement) && msiIsEditingRenderedHTML(editorElement));
//    return (IsDocumentEditable() && IsEditingRenderedHTML());
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement();
//    if (editorElement != null)
    try
    {
      var theWindow = msiGetWindowContainingEditor(editorElement);
      theWindow.openDialog("chrome://editor/content/EdColorProps.xul","_blank", "chrome,close,titlebar,modal", ""); 
//      UpdateDefaultColors(); 
      msiUpdateDefaultColors(editorElement);
      editorElement.contentWindow.focus();
    }
    catch(exc) {AlertWithTitle("Error in msiComposerCommands.js", "Error in msiColorPropertiesCommand.doCommand: " + exc);}
  }
};

////-----------------------------------------------------------------------------------
var msiRemoveNamedAnchorsCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    // We could see if there's any link in selection, but it doesn't seem worth the work!
    return (msiIsDocumentEditable() && msiIsEditingRenderedHTML());
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    try
    {
      var editorElement = msiGetActiveEditorElement();
      msiEditorRemoveTextProperty(editorElement, "name", "");
      editorElement.contentWindow.focus();
    }
    catch(exc) {AlertWithTitle("Error in msiComposerCommands.js", "Error in msiRemoveNamedAnchorsCommand.doCommand: " + exc);}
  }
};


////-----------------------------------------------------------------------------------
var msiEditLinkCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    // Not really used -- this command is only in context menu, and we do enabling there
    return (msiIsDocumentEditable() && msiIsEditingRenderedHTML());
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement();
    try
    {
      var element = msiGetEditor(editorElement).getSelectedElement("href");
      if (element)
        msiEditPage(element.href, window, false);
    }
    catch (exc) {AlertWithTitle("Error in msiComposerCommands.js", "Error in msiEditLinkCommand.doCommand: " + exc);}
    editorElement.contentWindow.focus();
  }
};


////-----------------------------------------------------------------------------------
var msiNormalModeCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    return msiIsTopLevelEditor(editorElement) && msiIsHTMLEditor(editorElement) && msiIsDocumentEditable(editorElement);
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetTopLevelEditorElement();
    msiSetEditMode(kDisplayModeNormal, editorElement);
  }
};

var msiAllTagsModeCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    return (msiIsTopLevelEditor(editorElement) && msiIsDocumentEditable(editorElement) && msiIsHTMLEditor(editorElement));
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetTopLevelEditorElement();
    msiSetEditMode(kDisplayModeAllTags, editorElement);
  }
};

var msiHTMLSourceModeCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    return (msiIsTopLevelEditor(editorElement) && msiIsDocumentEditable(editorElement) && msiIsHTMLEditor(editorElement));
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetTopLevelEditorElement();
    msiSetEditMode(kDisplayModeSource, editorElement);
  }
};

var msiPreviewModeCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    return (msiIsTopLevelEditor(editorElement) && msiIsDocumentEditable(editorElement) && msiIsHTMLEditor(editorElement));
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetTopLevelEditorElement();
    msiSetEditMode(kDisplayModePreview, editorElement);
  }
};

////-----------------------------------------------------------------------------------
var msiInsertOrEditTableCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return (msiIsDocumentEditable() && msiIsEditingRenderedHTML());
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement();
    if (msiIsInTableCell(editorElement))
      msiEditorTableCellProperties(editorElement);
//      EditorTableCellProperties();
    else
//      EditorInsertOrEditTable(true);
      msiEditorInsertOrEditTable(true, editorElement, aCommand, this);
  }
};

////-----------------------------------------------------------------------------------
var msiEditTableCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return msiIsInTable();
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement();
    msiEditorInsertOrEditTable(false, editorElement, aCommand, this);
  }
};

////-----------------------------------------------------------------------------------
var msiSelectTableCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return msiIsInTable();
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement();
    try
    {
      msiGetTableEditor(editorElement).selectTable();
    }
    catch(exc) {AlertWithTitle("Error in msiComposerCommands.js", "Error in msiSelectTableCommand.doCommand: " + exc);}
    if (editorElement)
      editorElement.contentWindow.focus();
    else
      window.content.focus();
  }
};

var msiSelectTableRowCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return msiIsInTableCell();
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement();
    try

    {
      msiGetTableEditor(editorElement).selectTableRow();
    }
    catch(exc) {AlertWithTitle("Error in msiComposerCommands.js", "Error in msiSelectTableRowCommand.doCommand: " + exc);}
    if (editorElement)
      editorElement.contentWindow.focus();
    else
      window.content.focus();
  }
};

var msiSelectTableColumnCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return msiIsInTableCell();
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},


  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement();
    try {
      msiGetTableEditor(editorElement).selectTableColumn();
    }
    catch(exc) {AlertWithTitle("Error in msiComposerCommands.js", "Error in msiSelectTableColumnCommand.doCommand: " + exc);}
    if (editorElement)
      editorElement.contentWindow.focus();
    else
      window.content.focus();
  }
};

var msiSelectTableCellCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return msiIsInTableCell();
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement();
    try
    {
      msiGetTableEditor(editorElement).selectTableCell();
    }
    catch(exc) {AlertWithTitle("Error in msiComposerCommands.js", "Error in msiSelectTableCellCommand.doCommand: " + exc);}
    if (editorElement)
      editorElement.contentWindow.focus();
    else
      window.content.focus();
  }
};

var msiSelectAllTableCellsCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return msiIsInTable();
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement();
    try
    {
      msiGetTableEditor(editorElement).selectAllTableCells();
    }
    catch(exc) {AlertWithTitle("Error in msiComposerCommands.js", "Error in msiSelectAllTableCellsCommand.doCommand: " + exc);}
    if (editorElement)
      editorElement.contentWindow.focus();
    else
      window.content.focus();
  }
};

////-----------------------------------------------------------------------------------
var msiInsertTableCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return msiIsDocumentEditable() && msiIsEditingRenderedHTML();
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
//    EditorInsertTable();
    var editorElement = msiGetActiveEditorElement();
    msiEditorInsertTable(editorElement, aCommand, this);
  }
};

var msiInsertTableRowAboveCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return msiIsInTableCell();
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement();
    try
    {
      msiGetTableEditor(editorElement).insertTableRow(1, false);
    }
    catch(exc) {AlertWithTitle("Error in msiComposerCommands.js", "Error in msiInsertTableRowAboveCommand.doCommand: " + exc);}
    if (editorElement)
      editorElement.contentWindow.focus();
    else
      window.content.focus();
  }
};

var msiInsertTableRowBelowCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return msiIsInTableCell();
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement();
    try
    {
      msiGetTableEditor(editorElement).insertTableRow(1, true);
    }
    catch(exc) {AlertWithTitle("Error in msiComposerCommands.js", "Error in msiInsertTableRowBelowCommand.doCommand: " + exc);}
    if (editorElement)
      editorElement.contentWindow.focus();
    else
      window.content.focus();
  }
};

var msiInsertTableColumnBeforeCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return msiIsInTableCell();
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement();
    try
    {
      msiGetTableEditor(editorElement).insertTableColumn(1, false);
    }
    catch(exc) {AlertWithTitle("Error in msiComposerCommands.js", "Error in msiInsertTableColumnBeforeCommand.doCommand: " + exc);}
    if (editorElement)
      editorElement.contentWindow.focus();
    else
      window.content.focus();
  }
};

var msiInsertTableColumnAfterCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return msiIsInTableCell();
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement();
    try
    {
      msiGetTableEditor(editorElement).insertTableColumn(1, true);
    }
    catch(exc) {AlertWithTitle("Error in msiComposerCommands.js", "Error in msiInsertTableColumnAfterCommand.doCommand: " + exc);}
    if (editorElement)
      editorElement.contentWindow.focus();
    else
      window.content.focus();
  }
};

var msiInsertTableCellBeforeCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return msiIsInTableCell();
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement();
    try
    {
      msiGetTableEditor(editorElement).insertTableCell(1, false);
    }
    catch(exc) {AlertWithTitle("Error in msiComposerCommands.js", "Error in msiInsertTableCellBeforeCommand.doCommand: " + exc);}
    if (editorElement)
      editorElement.contentWindow.focus();
    else
      window.content.focus();
  }
};

var msiInsertTableCellAfterCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return msiIsInTableCell();
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement();
    try
    {
      msiGetTableEditor(editorElement).insertTableCell(1, true);
    }
    catch(exc) {AlertWithTitle("Error in msiComposerCommands.js", "Error in msiInsertTableCellAfterCommand.doCommand: " + exc);}
    if (editorElement)
      editorElement.contentWindow.focus();
    else
      window.content.focus();
  }
};

////-----------------------------------------------------------------------------------
var msiDeleteTableCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return msiIsInTable();
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement();
    try
    {
      msiGetTableEditor(editorElement).deleteTable();
    }
    catch(exc) {AlertWithTitle("Error in msiComposerCommands.js", "Error in msiDeleteTableCommand.doCommand: " + exc);}
    if (editorElement)
      editorElement.contentWindow.focus();
    else
      window.content.focus();
  }
};

var msiDeleteTableRowCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return msiIsInTableCell();
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement();
    var rows = msiGetNumberOfContiguousSelectedRows(editorElement);
    // Delete at least one row
    if (rows == 0)
      rows = 1;

    try
    {
      var editor = msiGetTableEditor(editorElement);
      editor.beginTransaction();

      // Loop to delete all blocks of contiguous, selected rows
      while (rows)
      {
        editor.deleteTableRow(rows);
        rows = msiGetNumberOfContiguousSelectedRows(editorElement);
      }
    }
    finally { editor.endTransaction(); }
    if (editorElement)
      editorElement.contentWindow.focus();
    else
      window.content.focus();
  }
};

var msiDeleteTableColumnCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return msiIsInTableCell();
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement();
    var columns = msiGetNumberOfContiguousSelectedColumns(editorElement);
    // Delete at least one column
    if (columns == 0)
      columns = 1;

    try
    {
      var editor = msiGetTableEditor(editorElement);
      editor.beginTransaction();

      // Loop to delete all blocks of contiguous, selected columns
      while (columns)
      {
        editor.deleteTableColumn(columns);
        columns = msiGetNumberOfContiguousSelectedColumns(editorElement);
      }
    }
    finally { editor.endTransaction(); }
    if (editorElement)
      editorElement.contentWindow.focus();
    else
      window.content.focus();
  }
};

var msiDeleteTableCellCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return msiIsInTableCell();
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement();
    try
    {
      msiGetTableEditor(editorElement).deleteTableCell(1);   
    }
    catch(exc) {AlertWithTitle("Error in msiComposerCommands.js", "Error in msiDeleteTableCellCommand.doCommand: " + exc);}
    if (editorElement)
      editorElement.contentWindow.focus();
    else
      window.content.focus();
  }
};

var msiDeleteTableCellContentsCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return msiIsInTableCell();
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)

  {
    var editorElement = msiGetActiveEditorElement();
    try
    {
      msiGetTableEditor(editorElement).deleteTableCellContents();
    }
    catch(exc) {AlertWithTitle("Error in msiComposerCommands.js", "Error in msiDeleteTableCellContentsCommand.doCommand: " + exc);}
    if (editorElement)
      editorElement.contentWindow.focus();
    else
      window.content.focus();
  }
};


////-----------------------------------------------------------------------------------
var msiNormalizeTableCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return msiIsInTable();
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement();
    // Use nsnull to let editor find table enclosing current selection
    try
    {
      msiGetTableEditor(editorElement).normalizeTable(null);   
    }
    catch(exc) {AlertWithTitle("Error in msiComposerCommands.js", "Error in msiNormalizeTableCommand.doCommand: " + exc);}
    if (editorElement)
      editorElement.contentWindow.focus();
    else
      window.content.focus();
  }
};

////-----------------------------------------------------------------------------------
var msiJoinTableCellsCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {

    var editorElement = msiGetActiveEditorElement();
    if (msiIsDocumentEditable(editorElement) && msiIsEditingRenderedHTML(editorElement))
    {
      try {
        var editor = msiGetTableEditor(editorElement);
        var tagNameObj = { value: "" };
        var countObj = { value: 0 };
        var cell = editor.getSelectedOrParentTableElement(tagNameObj, countObj);

        // We need a cell and either > 1 selected cell or a cell to the right
        //  (this cell may originate in a row spanned from above current row)
        // Note that editor returns "td" for "th" also.
        // (this is a pain! Editor and gecko use lowercase tagNames, JS uses uppercase!)
        if( cell && (tagNameObj.value == "td"))
        {
          // Selected cells
          if (countObj.value > 1) return true;

          var colSpan = cell.getAttribute("colspan");

          // getAttribute returns string, we need number
          // no attribute means colspan = 1
          if (!colSpan)
            colSpan = Number(1);
          else
            colSpan = Number(colSpan);

          var rowObj = { value: 0 };
          var colObj = { value: 0 };
          editor.getCellIndexes(cell, rowObj, colObj);

          // Test if cell exists to the right of current cell
          // (cells with 0 span should never have cells to the right
          //  if there is, user can select the 2 cells to join them)
          return (colSpan && editor.getCellAt(null, rowObj.value,
                                              colObj.value + colSpan));
        }
      }
      catch (exc) {AlertWithTitle("Error in msiComposerCommands.js", "Error in msiJoinTableCellsCommand.isCommandEnabled: " + exc);}
    }
    return false;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement();
    // Param: Don't merge non-contiguous cells
    try
    {
      msiGetTableEditor(editorElement).joinTableCells(false);
    }
    catch(exc) {AlertWithTitle("Error in msiComposerCommands.js", "Error in msiJoinTableCellsCommand.doCommand: " + exc);}
    if (editorElement)
      editorElement.contentWindow.focus();
    else
      window.content.focus();
  }
};

////-----------------------------------------------------------------------------------
var msiSplitTableCellCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    if (msiIsDocumentEditable(editorElement) && msiIsEditingRenderedHTML(editorElement))
    {
      var tagNameObj = { value: "" };
      var countObj = { value: 0 };
      var cell;
      try {
        cell = msiGetTableEditor(editorElement).getSelectedOrParentTableElement(tagNameObj, countObj);
      } catch (e) {}

      // We need a cell parent and there's just 1 selected cell 
      // or selection is entirely inside 1 cell
      if ( cell && (tagNameObj.value == "td") && 
           countObj.value <= 1 &&
           msiIsSelectionInOneCell(editorElement) )
      {
        var colSpan = cell.getAttribute("colspan");
        var rowSpan = cell.getAttribute("rowspan");
        if (!colSpan) colSpan = 1;
        if (!rowSpan) rowSpan = 1;
        return (colSpan > 1  || rowSpan > 1 ||
                colSpan == 0 || rowSpan == 0);
      }
    }
    return false;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement();
    try
    {
      msiGetTableEditor(editorElement).splitTableCell();
    }
    catch(exc) {AlertWithTitle("Error in msiComposerCommands.js", "Error in msiSplitTableCellCommand.doCommand: " + exc);}
    if (editorElement)
      editorElement.contentWindow.focus();
    else
      window.content.focus();
  }
};

////-----------------------------------------------------------------------------------
var msiTableOrCellColorCommand =
{
  isCommandEnabled: function(aCommand, editorElement)
  {
    return msiIsInTable(editorElement);
  },


  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand, editorElement)
  {
    msiEditorSelectColor("TableOrCell", null, editorElement);
  }
};

////-----------------------------------------------------------------------------------
var nsPreferencesCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    goPreferences('editor', 'chrome://editor/content/pref-composer.xul','editor');
    window.content.focus();
  }
};
//
//
var msiFinishHTMLSourceCmd =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement();
    // In msiEditor.js
    msiFinishHTMLSource(editorElement);
  }
};

var msiCancelHTMLSource =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement();
    // In msiEditor.js
    msiCancelHTMLSource(editorElement);
  }
};

var msiConvertToTable =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    if (msiIsDocumentEditable(editorElement) && msiIsEditingRenderedHTML(editorElement))
    {
      var selection;
      try {
        selection = msiGetEditor(editorElement).selection;
      } catch (e) {}

      if (selection && !selection.isCollapsed)
      {
        // Don't allow if table or cell is the selection
        var element;
        try {
          element = msiGetEditor(editorElement).getSelectedElement("");
        } catch (e) {}
        if (element)
        {
          var name = element.nodeName.toLowerCase();
          if (name == "td" ||
              name == "th" ||
              name == "caption" ||
              name == "table")
            return false;
        }

        // Selection start and end must be in the same cell
        //   in same cell or both are NOT in a cell
        if ( GetParentTableCell(selection.focusNode) !=
             GetParentTableCell(selection.anchorNode) )
          return false;
      
        return true;
      }
    }
    return false;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var editorElement = msiGetActiveEditorElement();
    if (this.isCommandEnabled())
    {
      try
      {
        var theWindow = msiGetWindowContainingEditor(editorElement);
        window.openDialog("chrome://editor/content/EdConvertToTable.xul","_blank", "chrome,close,titlebar,modal");
      }
      catch(exc) {AlertWithTitle("Error in msiComposerCommands.js", "Error in msiConvertToTable.doCommand: " + exc);}
    }
    if (editorElement)
      editorElement.contentWindow.focus();
    else
      window.content.focus();
  }
};

function msiNote(currNode, editorElement)
{
  var data= new Object();
  data.editorElement = editorElement;
  var currNodeTag = "";
  if (currNode) {
    data.type = currNode.getAttribute("type");
    try
    {
      if (currNode.getAttribute("hide") == "true") data.hide=true;

    }
    catch(e){}
  }
  else
  {
    //defaults
    data.type = "";
  }

  window.openDialog("chrome://prince/content/Note.xul","_blank", "chrome,close,titlebar,resizable=yes,modal", data);
  // data comes back altered
  if (data.Cancel)
    return;

  dump(data.type + "\n");
  if (data.type != 'footnote') msiRequirePackage(editorElement, "ragged2e", "raggedrightboxes"); 
  var editor = msiGetEditor(editorElement);
  if (currNode)  // currnode is a note node
  {
    if (data.type == 'footnote') currNode.parentNode.setAttribute("type","footnote");
    else currNode.parentNode.removeAttribute("type");
    currNode.setAttribute("type",data.type);
    if (data.hide) currNode.setAttribute("hide","true")
    else currNode.removeAttribute("hide");
  }
  else
  {
    var namespace = new Object();
    var paraTag = editor.tagListManager.getDefaultParagraphTag(namespace);
    var xml = "<notewrapper xmlns='http://www.w3.org/1999/xhtml'" + ((data.type=='footnote')?" type='footnote'":"")+
      "><note type='"+data.type+"'"+ ((data.hidenote)?" hide='true'":"")
     +"><"+paraTag+"><br/></"+paraTag+"></note></notewrapper>"; 
    editor.deleteSelection(0);
    insertXMLAtCursor(editor,xml,true,false);
  }
}

function msiFrame(currNode, editorElement)
{
  var data= new Object();
  data.editorElement = editorElement;
  var editor = msiGetEditor(editorElement);
  var currNodeTag = "";
  data.newElement = true; 
  if (currNode) {
    data.newElement = false;
    data.element = currNode;
  }
  else
  {
    data.element = null;
  }
  editor.beginTransaction();
  window.openDialog("chrome://prince/content/Frame.xul","_blank", "chrome,close,titlebar,resizable=yes,modal=yes", data);
  // data comes back altered
  var editor = msiGetEditor(editorElement);
  dump("data.newElement is "+data.newElement+"\n");
  if (data.newElement)
  {
    try
    {
      var namespace = new Object();                      
      var paraTag = editor.tagListManager.getDefaultParagraphTag(namespace);
      msiRequirePackage(editorElement, "wrapfig", null);
      msiRequirePackage(editorElement, "boxedminipage", null);
      var selection = editor.selection;
      selection.getRangeAt(0).insertNode(data.element);
      if (!selection.collapsed) editor.deleteSelection(0);
      try
      {
        var defpara = "para";
        var para = editor.document.createElement(defpara);
        var br = editor.document.createElement("br");
        data.element.appendChild(para);
        para.appendChild(br);
      }
      catch(e) {
      }
    }
    catch(e) {
    }
  } 
  editor.endTransaction();
}


function msiStopAnimation()
{                
  var cmdParams = newCommandParams();
  if (!cmdParams) return;
  cmdParams.setLongValue("imageAnimation", 1);
  msiGoDoCommandParams("cmd_setDocumentOptions", cmdParams);
};

function msiStartAnimation()
{
  var cmdParams = newCommandParams();
  if (!cmdParams) return;
  cmdParams.setLongValue("imageAnimation", 0);
  msiGoDoCommandParams("cmd_setDocumentOptions", cmdParams);
};



//function test(firstNode, secondNode) // the nodes come from a split, so the node types should be the same
//// If the firstNode has a 'nexttag' defined, then we should convert the second node to that type. 

//{
//  var nodeName = firstNode.nodeName
//};


function callFunctionKeyDialog()
{
  var editorElement = msiGetActiveEditorElement();
  var theDialog = msiOpenModelessDialog("chrome://prince/content/tagkeyassignments.xul", "_blank", 
      "chrome,close,titlebar,resizable,dependent",
      editorElement, "cmd_tagkeyassignments", null, null);
}

function callOTFontDialog()
{
  var editorElement = msiGetActiveEditorElement();
  var theDialog = msiOpenModelessDialog("chrome://prince/content/otfont.xul", "_blank", 
      "chrome,close,titlebar,resizable,dependent",
      editorElement, "cmd_otfont", null, null);
}


function callFontSizeDialog()
{
  var editorElement = msiGetActiveEditorElement();
  var theDialog = msiOpenModelessDialog("chrome://prince/content/fontsize.xul", "_blank", 
      "chrome,close,titlebar,resizable,dependent",
      editorElement, "cmd_fontsize", null, null);
}


function callColorDialog()
{
  // TODO: get the current color if we are within a fontcolor tag
  var colorObj = { NoDefault:true, Type:"Font", TextColor:"black", PageColor:0, Cancel:false };

  window.openDialog("chrome://editor/content/EdColorPicker.xul", "_blank", "chrome,close,titlebar,modal", 
  "", colorObj);

  // User canceled the dialog
  if (colorObj.Cancel)
    return;
    
  var editorElement = msiGetParentEditorElementForDialog(window);
  if (!editorElement)
  {
    AlertWithTitle("Error", "No editor in otfont.OnAccept!");
  }
  var theWindow = window.opener;
  if (!theWindow || !("msiEditorSetTextProperty" in theWindow))
    theWindow = msiGetTopLevelWindow();
  theWindow.msiRequirePackage(editorElement, "xcolor", null);
  theWindow.msiEditorSetTextProperty(editorElement, "fontcolor", "color", colorObj.TextColor);
  editorElement.contentWindow.focus();
}

var msiShowTeXLogCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    result = false;
    var editorElement = msiGetActiveEditorElement();
    if (!msiIsTopLevelEditor(editorElement))
      return result;

    var editor = msiGetEditor(editorElement);
    if (editor)
    {
      var url = msiGetEditorURL(editorElement);
      var re = /(.*)\/([^\/\.]*)\.[^\/\.]*$/;
      var match = re.exec(url);
      if (match)
      {
        var resurl = match[1]+"/"+match[2]+"_files/tex/"+match[2]+".log";
        var thefile = Components.classes["@mozilla.org/file/local;1"].
        createInstance(Components.interfaces.nsILocalFile);
        thefile.initWithPath(resurl);
        result = thefile.exists();
      }
    }
    return result;
  },
  
  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    result = true;
    var editorElement = msiGetActiveEditorElement();
    if (!msiIsTopLevelEditor(editorElement))
      return result;

    var editor = msiGetEditor(editorElement);
    if (editor)
    {
      var url = msiGetEditorURL(editorElement);
//      var re = /\/([a-zA-Z0-9_]+)\.[a-zA-Z0-9_]+$/i;
      var re = /(.*)\/([^\/\.]*)\.[^\/\.]*$/;
      var match = re.exec(url);
      if (match)
      {
        var resurl = match[1]+"/tex/"+match[2]+".log";
        openDialog("chrome://global/content/viewSource.xul",
               "_blank",
               "all,dialog=no",
               resurl, null, null);
      }
    }
    return result;
  }
}

var msiShowXSLTLogCommand =
{}


var msiGoToParagraphCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },
  
  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    result = true;
    var editorElement = msiGetActiveEditorElement();
    if (!msiIsTopLevelEditor(editorElement))
      return;

    var editor = msiGetEditor(editorElement);
    if (editor)
    {
      // use the first node of the selection as the context node
      var contextNode = editor.selection.focusNode;
      window.openDialog('chrome://prince/content/gotoparagraph.xul','_blank', 'chrome,resizable,close,modal,titlebar',editor, contextNode, editorElement);
      window.content.focus();
    }
  }
};



var msiWordCountCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },
  
  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    result = true;
    var editorElement = msiGetActiveEditorElement();
    if (!msiIsTopLevelEditor(editorElement))
      return;

    var editor = msiGetEditor(editorElement);
    if (editor)
    {
      // use the first node of the selection as the context node
      var wc = countWords(editor.document);
      window.openDialog('chrome://prince/content/wordcount.xul','_blank', 'chrome,resizable,close,titlebar',wc);
      window.content.focus();
    }
  }
};
