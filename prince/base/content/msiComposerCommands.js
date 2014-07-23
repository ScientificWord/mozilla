// Copyright (c) 2006 MacKichan Software, Inc.  All Rights Reserved.
Components.utils.import("resource://app/modules/unitHandler.jsm");
Components.utils.import("resource://app/modules/os.jsm");
#include productname.inc


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

  var parser = new DOMParser;

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
//  commandTable.registerCommand("cmd_editLink",        msiEditLinkCommand);
  commandTable.registerCommand("cmd_followLink",        msiFollowLinkCommand);

  commandTable.registerCommand("cmd_form",          msiFormCommand);
  commandTable.registerCommand("cmd_inputtag",      msiInputTagCommand);
  commandTable.registerCommand("cmd_inputimage",    msiInputImageCommand);
  commandTable.registerCommand("cmd_textarea",      msiTextAreaCommand);
  commandTable.registerCommand("cmd_select",        msiSelectCommand);
  commandTable.registerCommand("cmd_button",        msiButtonCommand);
//  commandTable.registerCommand("cmd_label",         msiLabelCommand);
  commandTable.registerCommand("cmd_fieldset",      msiFieldSetCommand);
  commandTable.registerCommand("cmd_isindex",       msiIsIndexCommand);
  commandTable.registerCommand("cmd_image",         msiImageCommand);
  commandTable.registerCommand("cmd_video",         msiVideoCommand);
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
  commandTable.registerCommand("cmd_insertHTMLField", msiInsertHTMLFieldCommand);
  commandTable.registerCommand("cmd_marker",             msiMarkerCommand);
  commandTable.registerCommand("cmd_table",              msiInsertOrEditTableCommand);
  commandTable.registerCommand("cmd_editTable",          msiEditTableCommand);
  commandTable.registerCommand("cmd_editTableCell",      msiEditTableCommand);
  commandTable.registerCommand("cmd_editTableCellGroup", msiEditTableCommand);
  commandTable.registerCommand("cmd_editTableRows",      msiEditTableCommand);
  commandTable.registerCommand("cmd_editTableCols",      msiEditTableCommand);
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
  commandTable.registerCommand("cmd_SelectMatrix",						msiSelectTableCommand);
  commandTable.registerCommand("cmd_SelectMatrixRow",					msiSelectTableRowCommand);
  commandTable.registerCommand("cmd_SelectMatrixColumn",			msiSelectTableColumnCommand);
  commandTable.registerCommand("cmd_SelectMatrixCell",				msiSelectTableCellCommand);
  commandTable.registerCommand("cmd_SelectAllMatrixCells",		msiSelectAllTableCellsCommand);
  commandTable.registerCommand("cmd_InsertMatrix",						msiInsertMatrix);
  commandTable.registerCommand("cmd_InsertMatrixRowAbove",		msiInsertTableRowAboveCommand);
  commandTable.registerCommand("cmd_InsertMatrixRowBelow",		msiInsertTableRowBelowCommand);
  commandTable.registerCommand("cmd_InsertMatrixColumnBefore",msiInsertTableColumnBeforeCommand);
  commandTable.registerCommand("cmd_InsertMatrixColumnAfter",	msiInsertTableColumnAfterCommand);
  commandTable.registerCommand("cmd_DeleteMatrix",						msiDeleteTableCommand);
  commandTable.registerCommand("cmd_DeleteMatrixRow",					msiDeleteTableRowCommand);
  commandTable.registerCommand("cmd_DeleteMatrixColumn",			msiDeleteTableColumnCommand);
  commandTable.registerCommand("cmd_DeleteMatrixCellContents",msiDeleteTableCellContentsCommand);

  commandTable.registerCommand("cmd_ConvertToTable",     msiConvertToTable);
  commandTable.registerCommand("cmd_MSIAnimateGifsOn",   msiGIFAnimation);
  commandTable.registerCommand("cmd_MSIAnimateGifsOff",  msiGIFAnimation);
  commandTable.registerCommand("cmd_printDirect",           msiPrintDirectCommand);
  commandTable.registerCommand("cmd_printPdf",           msiPrintCommand);
  commandTable.registerCommand("cmd_previewDirect",         msiPreviewDirectCommand);
  commandTable.registerCommand("cmd_previewPdf",         msiPreviewCommand);
  commandTable.registerCommand("cmd_compilePdf",         msiCompileCommand);
//  commandTable.registerCommand("cmd_updateStructToolbar", msiUpdateStructToolbarCommand);
  commandTable.registerCommand("cmd_insertReturnFancy", msiInsertReturnFancyCommand);
  commandTable.registerCommand("cmd_insertSubstructure", msiInsertSubstructureCommand);
  commandTable.registerCommand("cmd_documentInfo",       msiDocumentInfoCommand);
  commandTable.registerCommand("cmd_documentStyle",       msiDocumentStyleCommand);
  commandTable.registerCommand("cmd_macrofragment", msiMacroFragmentCommand);
  commandTable.registerCommand("cmd_viewInvisibles", msiViewInvisiblesCommand);

  commandTable.registerCommand("cmd_msiReviseHyperlink", msiReviseHyperlinkCommand);
  commandTable.registerCommand("cmd_reviseAnchor", msiReviseAnchorCommand);
  commandTable.registerCommand("cmd_reviseImage", msiReviseImageCommand);
  commandTable.registerCommand("cmd_reviseVideo", msiReviseVideoCommand);
//  commandTable.registerCommand("cmd_reviseLine",  msiReviseLineCommand);
  commandTable.registerCommand("cmd_reviseForm",  msiReviseFormCommand);
  commandTable.registerCommand("cmd_reviseTextarea", msiReviseTextareaCommand);
  commandTable.registerCommand("cmd_reviseButton",  msiReviseButtonCommand);
//  commandTable.registerCommand("cmd_reviseLabel",  msiReviseLabelCommand);
  commandTable.registerCommand("cmd_reviseFieldset", msiReviseFieldsetCommand);
  commandTable.registerCommand("cmd_reviseChars",  msiReviseCharsCommand);
  commandTable.registerCommand("cmd_reviseHTML",   msiReviseHTMLCommand);
  commandTable.registerCommand("cmd_reviseHorizontalSpaces", msiReviseHorizontalSpacesCommand);
  commandTable.registerCommand("cmd_reviseVerticalSpaces", msiReviseVerticalSpacesCommand);
  commandTable.registerCommand("cmd_msiReviseRules",   msiReviseRulesCommand);
  commandTable.registerCommand("cmd_msiReviseBreaks",  msiReviseBreaksCommand);
  commandTable.registerCommand("cmd_MSIsetAlignmentCommand",  msiSetAlignmentCommand);

  commandTable.registerCommand("cmd_note", msiNoteCommand);
  commandTable.registerCommand("cmd_footnote", msiFootnoteCommand);
  commandTable.registerCommand("cmd_frame", msiFrameCommand);
  commandTable.registerCommand("cmd_citation", msiCitationCommand);
  commandTable.registerCommand("cmd_reviseCitation", msiReviseCitationCommand);
  commandTable.registerCommand("cmd_showTeXLog", msiShowTeXLogCommand);
  commandTable.registerCommand("cmd_showBibTeXLog", msiShowBibTeXLogCommand);
  commandTable.registerCommand("cmd_showTeXFile", msiShowTeXFileCommand);
  commandTable.registerCommand("cmd_showXSLTLog", msiShowXSLTLogCommand);
  commandTable.registerCommand("cmd_gotoparagraph", msiGoToParagraphCommand);
  commandTable.registerCommand("cmd_gotoMarker", msiGoToMarkerCommand);
  commandTable.registerCommand("cmd_countwords", msiWordCountCommand);
  commandTable.registerCommand("cmd_reviseCrossRef", msiReviseCrossRefCommand);
  commandTable.registerCommand("cmd_copypicture", msiCopyPictureCommand);
  commandTable.registerCommand("cmd_savepicture", msiSavePictureCommand);
  commandTable.registerCommand("cmd_zoomin", msiZoomInCommand);
  commandTable.registerCommand("cmd_zoomout", msiZoomOutCommand);
  commandTable.registerCommand("cmd_zoomreset", msiZoomResetCommand);
  commandTable.registerCommand("cmd_showhelp", msiShowHelpCommand);
  commandTable.registerCommand("cmd_maketitle", msiMakeTitleCommand);
  commandTable.registerCommand("cmd_maketoc", msiMakeTOCCommand);
  commandTable.registerCommand("cmd_makelot", msiMakeLOTCommand);
  commandTable.registerCommand("cmd_makelof", msiMakeLOFCommand);
  commandTable.registerCommand("cmd_appendix", msiAppendixCommand);
  commandTable.registerCommand("cmd_mainmatter", msiMainMatterCommand);
  commandTable.registerCommand("cmd_backmatter", msiBackMatterCommand);
  commandTable.registerCommand("cmd_frontmatter", msiFrontMatterCommand);
  commandTable.registerCommand("cmd_printindex", msiPrintIndexCommand);
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
  commandTable.registerCommand("cmd_fontcolor", msiFontColor);
  commandTable.registerCommand("cmd_copytex", msiCopyTeX);
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
  commandTable.registerCommand("cmd_saveAsDir",      msiSaveAsCommand);
  commandTable.registerCommand("cmd_saveCopyAs",     msiSaveCopyAsCommand);
  commandTable.registerCommand("cmd_saveCopyAsDir",  msiSaveCopyAsCommand);
  commandTable.registerCommand("cmd_exportToText",   msiExportToTextCommand);
  commandTable.registerCommand("cmd_saveAndChangeEncoding",  msiSaveAndChangeEncodingCommand);
  commandTable.registerCommand("cmd_publish",        msiPublishCommand);
  commandTable.registerCommand("cmd_publishAs",      msiPublishAsCommand);
  commandTable.registerCommand("cmd_publishSettings",msiPublishSettingsCommand);
  commandTable.registerCommand("cmd_revert",         msiRevertCommand);
  commandTable.registerCommand("cmd_openRemote",     msiOpenRemoteCommand);
  commandTable.registerCommand("cmd_preview",        msiPreviewCommand);
  commandTable.registerCommand("cmd_editSendPage",   msiSendPageCommand);
  commandTable.registerCommand("cmd_print",          msiPrintDirectCommand);
  commandTable.registerCommand("cmd_printSetup",     msiPrintSetupCommand);
  commandTable.registerCommand("cmd_quit",           nsQuitCommand);
  commandTable.registerCommand("cmd_close",          msiCloseCommand);
  commandTable.registerCommand("cmd_cleanup",        msiCleanupCommand);
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
    try
    {
        controller = Components.classes["@mozilla.org/embedcomp/base-command-controller;1"].createInstance();

        var editorController = controller.QueryInterface(Components.interfaces.nsIControllerContext);
        editorController.init(null);
        editorController.setCommandContext(editorElement);
        editorElement.contentWindow.controllers.insertControllerAt(0, controller);

        // Store the controller ID so we can be sure to get the right one later
        editorElement.mComposerJSCommandControllerID = editorElement.contentWindow.controllers.getControllerId(controller);
    }
    catch(e)
    {
      msidump(e.message+"\n");
    }
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
      case "cmd_viewInvisibles":
        updateViewMenuFromEditor(editorElement);
      break;

      case "cmd_MSImathtext":
        updateMathText(editorElement);
      break;

      case "cmd_texttag":
      case "cmd_paratag":
      case "cmd_listag":
      case "cmd_structtag":
      case "cmd_envtag":
      case "cmd_frontmtag":
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

function isInMath(editorElement)
{
  var editor = msiGetEditor(editorElement);
  var selNode = editor.getSelectionContainer();
  return msiNavigationUtils.nodeIsInMath(selNode);
//  return editor.tagListManager.selectionContainedInTag("math",null)
}


function updateMathText(editorElement, ismath)
{
  try {
    // var docList = msiGetUpdatableItemContainers(command, editorElement);
    if (ismath == null) {
      ismath = isInMath(editorElement);
    }
    setMathTextToggle(editorElement, ismath);
  }
  catch(e) {
    msidump(e.message);
  }
}

function msiGoUpdateComposerMenuItems(commandset, editorElement)
{
  //dump("Updating commands for " + commandset.id + "\n");

  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  msiUpdateStructToolbar(editorElement);
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
function msiDoAPropertiesDialogFromMenu(command, menuItem)
{
  var theElement = null;
  var editorElement = null;
  var propsData = null;
  if (menuItem)
  {
    if (menuItem.propertiesData)
    {
      propsData = menuItem.propertiesData;
      theElement = menuItem.propertiesData.getPropertiesDialogNode();
      editorElement = menuItem.propertiesData.mEditorElement;
    }
    if (!theElement && menuItem.refElement)
      theElement = menuItem.refElement;
    if (!editorElement && menuItem.refEditor)
      editorElement = menuItem.refEditor;
  }

  if (!theElement || !command || !command.length)
  {
    dump("msiDoAPropertiesDialog called without node or command! Get from context?\n");
    return;
  }
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();

  var cmdParams = newCommandParams();
  if (!cmdParams)
  {
    dump("Trouble in msiDoAPropertiesDialog! Can't create new CommandParams - aborting.\n");
    return;
  }

  cmdParams.setISupportsValue("reviseObject", theElement);
  if (propsData)
    msiSetCommandParamWeakRefValue(cmdParams, "propertiesData", propsData);
//    cmdParams.setISupportsValue("propertiesData", propsData);
  msiGoDoCommandParams(command, cmdParams, editorElement);
}


function msiGoDoCommandParams(command, params, editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  // editorElement && editorElement.focus();
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
  try
  {

     var isMixed = cmdParams.getBooleanValue("state_mixed");
     var desiredAttrib;
     if (isMixed)
       desiredAttrib = "mixed";
     else
          desiredAttrib = cmdParams.getStringValue("state_attribute");


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

//function msiPokeTagStateUI(uiID, cmdParams)
//{
//  return;
//  var textboxName;
//  switch (uiID)
//  {
//    case "cmd_texttag":
//      textboxName = "TextTagSelections";
//      break;
//    case "cmd_paratag":
//    case "cmd_listtag":
//      textboxName = "ParaTagSelections";
//      break;
//    case "cmd_structtag":
//    case "cmd_envtag":
//      textboxName = "StructTagSelections";
//      break;
//    case "cmd_frontmtag":
//      textboxName = "FrontMTagSelections";
//      break;
//    default:
//      break;
////      return;
//  }
//  var desiredAttrib;
//  desiredAttrib = cmdParams.getStringValue("state_attribute");
//
//  try
//  {
//    var docList = msiGetUpdatableItemContainers(uiID, msiGetActiveEditorElement());
//    for (var i = 0; i < docList.length; ++i)
//    {
//      var commandNode = docList[i].getElementById(uiID);
//      if (commandNode)
//      {
//        var uiState = commandNode.getAttribute("state");
//        if (desiredAttrib != uiState)
//        {
//          commandNode.setAttribute("state", desiredAttrib);
//    //      commandNode.setAttribute("value", desiredAttrib);
//          var textbox = docList[i].getElementById(textboxName);
//          if (textbox)
//            textbox.textValue = desiredAttrib;
//        }
//      }
//    }
//  } 
//  catch(e) {}
//}
//
//
function msiDoStatefulCommand(commandID, newState, editorElement)
{
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  var tagmanager = editor.tagListManager;
	var tagclass;
// BBM. I rather late in the game changed this to get the commandID from newState in the case of tag commands
	if (commandID == 'cmd_paratag' || commandID == 'cmd_texttag' || commandID == 'cmd_listtag' ||
    commandID == 'cmd_structtag' || commandID == 'cmd_envtag' || commandID == 'cmd_frontmtag')
	{
		if (tagmanager) tagclass = tagmanager.getClassOfTag(newState, null);
		switch (tagclass)
		{
			case 'paratag' : commandID = 'cmd_paratag'; break;
			case 'listtag' : commandID = 'cmd_listtag'; break;
			case 'structtag' : commandID = 'cmd_structtag'; break;
			case 'envtag' : commandID = 'cmd_envtag'; break;
			case 'texttag' : commandID = 'cmd_texttag'; break;
      case 'frontmtag' : commandID = 'cmd_frontmtag'; break;
		}
	}
  // if (msiDeferStatefulCommand(commandID, newState, editorElement))
  //   return;

  var docList = msiGetUpdatableItemContainers(commandID, editorElement);
  for (var i = 0; i < docList.length; ++i)
  {
    var commandNode = docList[i].getElementById(commandID);
    if (commandNode)
      commandNode.setAttribute("state", newState);
  }

  try
  {
    // BBM: we need the next line of code in a number of places.
    editorElement.contentWindow.focus();   // needed for command dispatch to work

    var cmdParams = newCommandParams();
    if (!cmdParams) return;

    cmdParams.setStringValue("state_attribute", newState);
    var editor = msiGetEditor(editorElement);
    var ns = new Object;
    if (commandID=="cmd_texttag" && editor && editor.tagListManager && editor.tagListManager.getClearTextTag(ns) == newState)
    {
      msiGoDoCommand('cmd_removeStyles');
    }
    else if (commandID=="cmd_structtag" && editor && editor.tagListManager && editor.tagListManager.getClearStructTag(ns) == newState)
    {
      msiGoDoCommand('cmd_removestruct');
    }
    else if (commandID=="cmd_envtag" && editor && editor.tagListManager && editor.tagListManager.getClearEnvTag(ns) == newState)
    {
      msiGoDoCommand('cmd_removeenv');
    }
    else if (commandID=="cmd_frontmtag" && (newState === 'maketitle' || newState === 'maketoc' || newState === 'makelof' || newState === 'makelot'))
    {
      // var thenode = editor.document.createElement(newState);
      // editor.insertElementAtSelection(thenode, true);
      focusOnEditor();
      var dataString="&lt;"+newState+"/&gt;";
      var contextString = "";
      var infoString="(0,0)";
      editor.insertHTMLWithContext(dataString,
                                   contextString, infoString, "text/html",
                                   null,null,0,true);
    }
    else msiGoDoCommandParams(commandID, cmdParams, editorElement);
    // BBM: temporary hack!
    switch (commandID)
    {
      case "cmd_texttag":
      case "cmd_paratag":
      case "cmd_listtag":
      case "cmd_structtag":
      case "cmd_envtag":
      case "cmd_frontmtag":
        break;
      default:
        msiPokeMultiStateUI(commandID, cmdParams);
    }
    msiResetStructToolbar(editorElement);
    if (newState === 'bibitem') 
    {
      msiGoDoCommand("cmd_reviseManualBibItemCmd");
    }
  } catch(e) {
    dump("error thrown in msiDoStatefulCommand: "+e+"\n");
  }
}

//This repeats much of the functionality from msiDoStatefulCommand above, but is called from the dialog which has to run first.
function msiDoTagBibItem(dlgData, paraContainer, editorElement)
{
  var editor = msiGetEditor(editorElement);
  editor.beginTransaction();
  try
  {
    editorElement.contentWindow.focus();   // needed for command dispatch to work
    // if (paraContainer)
    //   editor.selection.collapse(paraContainer, 0);
    var cmdParams = newCommandParams();
    if (!cmdParams) return;

    cmdParams.setStringValue("state_attribute", "bibitem");
    msiGoDoCommandParams("cmd_listtag", cmdParams, editorElement);
  } catch(e) { dump("error thrown in msiDoTagBibItem: "+e+"\n"); }

  var bibitemNode = msiNavigationUtils.getParentOfType(editor.selection.focusNode, "bibitem");
  if (bibitemNode)
    doReviseManualBibItem(editorElement, bibitemNode, dlgData);
  editor.endTransaction();

  var docList = msiGetUpdatableItemContainers("cmd_listtag", editorElement);
  for (var i = 0; i < docList.length; ++i)
  {
    var commandNode = docList[i].getElementById("cmd_listtag");
    if (commandNode)
      commandNode.setAttribute("state", "bibitem");
  }
  try
  {
//    msiPokeTagStateUI("cmd_listtag", cmdParams);
    msiResetStructToolbar(editorElement);
  } catch(exc) { dump("Error after applying tag in msidoTagBibItem: " + exc + "\n"); }
}

//In the case of tagging a paragraph as a "bibitem", this function launches the ManualBibItem dialog
// function msiDeferStatefulCommand(commandID, newState, editorElement)
// {
//   var retVal = false;
//   var editor = msiGetEditor(editorElement);
//   switch(newState)
//   {
//     case "bibitem":
//     {
//       var paraNode = msiNavigationUtils.getTopParagraphParent(editor.selection.focusNode, editor);
//       var bibData = {key : "", bibLabel : "", paragraphNode : paraNode};
//       var dlgWindow = msiOpenModelessDialog("chrome://prince/content/typesetBibitemDlg.xul", "_blank", "chrome,close,titlebar,dependent,resizable",
//                                                            editorElement, commandID, this, bibData);
//       retVal = true;
//     }
//     break;
//   }
//   return retVal;
// }

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
      idNext = "FrontMTagSelections";
      break;
    case "cmd_frontmtag":
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
      idPrev = "FrontMTagSelections";
      break;
    case "cmd_paratag":
      idPrev = "TextTagSelections";
      break;
    case "cmd_structtag":
      idPrev = "ParaTagSelections";
      break;
    case "cmd_frontmtag":
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
  var invisChoices = [["viewInvisibles","showInvisibles"],
                      ["viewSectionExpanders","showSectionExpanders"],
                      ["viewShortTitles","showShortTitles"],
                      // ["viewFMButtons","showFMButtons"],
                      ["viewHelperLines","showHelperLines"],
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
//  dump("Calling msiEditorDoShowInvisibles, with viewSettings = [invis: " + viewSettings.showInvisibles + ", helperLines: " + viewSettings.showHelperLines + ", inputBoxes: " + viewSettings.showInputBoxes + ", indexEntries: " + viewSettings.showIndexEntries + ", markers: " + viewSettings.showMarkers + "]\n");
  msiEditorDoShowInvisibles(editorElement, viewSettings);
}

function updateViewMenuFromEditor(editorElement)
{
  if (!("viewSettings" in editorElement) || (editorElement.viewSettings == null))
    return;  //No settings in the editor - leave menu as is.

  var invisChoices = [["viewInvisibles","showInvisibles"],
                      ["viewSectionExpanders","showSectionExpanders"],
                      ["viewShortTitles","showShortTitles"],
                      // ["viewFMButtons","showFMButtons"],
                      ["viewHelperLines","showHelperLines"],
                      ["viewInputBoxes","showInputBoxes"], ["viewIndexEntries","showIndexEntries"],
                      ["viewMarkers","showMarkers"]];

  // I don't know what Ron intended here, but this can't work because command is not set.                      

  // var docList = msiGetUpdatableItemContainers(command, editorElement);
  // var menuItem;
  // for (var i = 0; i < docList.length; ++i)
  // {
  //   for (var ix = 0; ix < invisChoices.length; ++ix)
  //   {
  //     menuItem = docList[i].getElementById(invisChoices[ix][0]);
  //     if (menuItem != null)
  //     {
  //       if (editorElement.viewSettings[invisChoices[ix][1]])
  //         menuItem.setAttribute("checked", "true");
  //       else
  //         menuItem.setAttribute("checked", "false");
  //     }
  //   }
  // }

  var document = editorElement.ownerDocument;
  var menuItem;
  for (var ix = 0; ix < invisChoices.length; ++ix)
  {
    menuItem = document.getElementById(invisChoices[ix][0]);
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
	  try {
      openDocument();
    }
    catch (e) {
      finalThrow(cmdFailString("open"), e.message);
    }
	}
};

function openDocument()
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
      var newdocumentfile;
      newdocumentfile = createWorkingDirectory(fp.file);
      
      msiEditPage(msiFileURLFromFile(newdocumentfile), window, false, false);
      msiSaveFilePickerDirectoryEx(fp, fp.file.parent.path, MSI_EXTENSION);
    }
  }
  catch (e)
  {
    finalThrow(cmdFailString('Open'), e.message);
  }
}


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
	  try {
      openNewDocument();
    }
    catch (e) {
      finalThrow(cmdFailString('new'), e.message);
    }
	}
}

function openNewDocument()
{

  var newdocumentfile;
  var dir;
  var data={file: "not yet"};
  // jlf - should openshell be modal or dependent
  window.openDialog("chrome://prince/content/openshell.xul","openshell", "chrome,close,titlebar,modal,resizable=yes", data);
  if (data.filename)
  {
    if (data.filename && data.filename.length > 0) {
      dump("Ready to edit shell: " + data.filename +"\n");
      try {
        var thefile = Components.classes["@mozilla.org/file/local;1"].
          createInstance(Components.interfaces.nsILocalFile);
        thefile.initWithPath(data.filename);
        newdocumentfile = createWorkingDirectory(thefile);
        var url = msiFileURLFromAbsolutePath( newdocumentfile.path );
        msiEditPage( url, window, false, true);
      } catch (e) { dump("msiEditPage failed: "+e.toString()+"\n"); }

    }
  }
}

//// STRUCTURE TOOLBAR
////
// var msiUpdateStructToolbarCommand =
// {
//   isCommandEnabled: function(aCommand, dummy)
//   {
//     var editorElement = msiGetActiveEditorElement();
//     try {
//       msiUpdateStructToolbar(editorElement);
//     }
//     catch (e) {
// // silent fail
//       return false;
//     }
//     return true;
//   },
//   getCommandStateParams: function(aCommand, aParams, aRefCon) {},
//   doCommandParams: function(aCommand, aParams, aRefCon) {},
//   doCommand: function(aCommand)  {}
// }

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
    }
    catch (e) {
      finalThrow(cmdFailString('save'), e.message);
    }
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    try {
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
    }
    catch (e) {
      finalThrow(cmdFailString('save'), e.message);
    }
    return result;
  }
}

function doSoftSave(editorElement, editor, noTeX)
{
  var result;
  if (editor)
  {
    // we should be doing this only for top level documents, and we should restore the focus
    msiFinishHTMLSource(editorElement);
    var url = msiGetEditorURL(editorElement);
    result = msiSoftSave(editor, editorElement, noTeX);
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
    try {
      var result = false;
      var editorElement = msiGetActiveEditorElement();
      if (!msiIsTopLevelEditor(editorElement))
        return result;

      var editor = msiGetEditor(editorElement);
      return doSoftSave(editorElement, editor, false);
    }
    catch (e) {
      finalThrow(cmdFailString('softsave'), e.message);
    }
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
    try {
      var editorElement = msiGetActiveEditorElement();
      if (!msiIsTopLevelEditor(editorElement))
        return false;
      var editor = msiGetEditor(editorElement);
      if (editor)
      {
        msiFinishHTMLSource(editorElement);
        var result = msiSaveDocument(true, true, false, editor.contentsMIMEType, editor, editorElement, aCommand==="cmd_saveAsDir");
        editorElement.contentWindow.focus();
        return result;
      }
    }
    catch (e) {
      finalThrow(cmdFailString('saveas'), e.message);
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
    try {
      var editorElement = msiGetActiveEditorElement();
      if (!msiIsTopLevelEditor(editorElement))
        return false;
      var editor = msiGetEditor(editorElement);
      if (editor)
      {
        msiFinishHTMLSource(editorElement);
        var result = msiSaveDocument(true, true, true, editor.contentsMIMEType, editor, editorElement, aCommand==="cmd_saveCopyAsDir");
        editorElement.contentWindow.focus();
        return result;
      }
    }
    catch (e) {
      finalThrow(cmdFailString('savecopyas'), e.message);
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
    try {
      var editorElement = msiGetActiveEditorElement();
      if (!msiIsTopLevelEditor(editorElement))
        return false;
      var editor = msiGetEditor(editorElement);
      if (editor)
      {
        msiFinishHTMLSource(editorElement);
        var result = msiSaveDocument(true, true, true, "text/plain", editor, editorElement, falsle);
        editorElement.contentWindow.focus();
        return result;
      }
    }
    catch (e) {
      finalThrow(cmdFailString('exporttotext'), e.message);
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
    // jlf - should EditorSaveAsCharset be modal or dependent
    window.openDialog("chrome://editor/content/EditorSaveAsCharset.xul","saveascharset", "chrome,close,titlebar,modal,resizable=yes");

    if (msiGetDocumentTitle(editorElement) != oldTitle)
      UpdateWindowTitle();

    if (window.ok)
    {
      if (window.exportToText)
      {
        window.ok = msiSaveDocument(true, true, true, "text/plain", editorElement, false);
      }
      else
      {
        var editor = msiGetEditor(editorElement);
        window.ok = msiSaveDocument(true, true, false, (editor ? editor.contentsMIMEType : null), editor, editorElement, false);
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
    try {
      var editorElement = msiGetActiveEditorElement();
      var doc = editorElement.contentDocument;
#ifndef PROD_SW
      rebuildSnapshots(doc);
#endif
      PrintUtils.print();
    }
    catch (e) {
      finalThrow(cmdFailString('directprint'), e.message);
    }
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
    try {
      var editorElement = msiGetActiveEditorElement();
      var doc = editorElement.contentDocument;
#ifndef PROD_SW
      rebuildSnapshots(doc);
#endif
      PrintUtils.printPreview(onEnterPP, onExitPP);
    }
    catch (e) {
      finalThrow(cmdFailString('directprintpreview'), e.message);
    }
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
        // jlf - should EditorPublish be modal or dependent
        window.openDialog("chrome://editor/content/EditorPublish.xul","publish",
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
      window.openDialog("chrome://editor/content/EditorPublish.xul","publish",
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
  var filename = decodeURI(GetFilename(aDocumentURLString));
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
function msiPromptForSaveLocation(aDoSaveAsText, aEditorType, aMIMEType, aDocumentURLString, editorElement, aUseDirectory)
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

  fp.init(window, promptString, aUseDirectory?msIFilePicker.modeGetFolder:msIFilePicker.modeSave);
  // Set filters according to the type of output
  if (!aUseDirectory) {
    if (aDoSaveAsText)
      fp.appendFilters(msIFilePicker.filterText);
    else
    {
      fp.appendFilter("SWP Documents","*."+MSI_EXTENSION);
      fp.defaultExtension = MSI_EXTENSION;
    }
  }
  else {
    fp.appendFilter("SWP Doc Directories","*."+MSI_EXTENSION);
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
    imeEditor.forceCompositionEnd();
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
    persistObj.progressListener = new msiEditorOutputProgressListener(editorElement);

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
    onsaveMetaData(editorDoc);
    persistObj.saveDocument(editorDoc, aDestinationLocation, aRelatedFilesParentDir,
                            aMimeType, outputFlags, wrapColumn);
    editorElement.mPersistObj = persistObj;
  }
  catch(e) {
    dump("caught an error, bail\n");
    return false;
  }
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
function msiGetWrapColumn(editorElement)
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
function msiEditorOutputProgressListener(editorElement)
{
  this.msiEditorElement = editorElement;

  this.onStateChange = function(aWebProgress, aRequest, aStateFlags, aStatus)
  {
    var editor = msiGetEditor(this.msiEditorElement);
		if (aRequest == null) return;

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
            editor.pdfModCount = -1;

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

function msiSoftSave( editor, editorElement, noTeX)
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
  var saveSelection = editor.selection;

  // if we don't have the right editor type bail (we handle text and html)
//  var editorType = editor.editortype;
//  if (editorType != "text" && editorType != "html"
//      && editorType != "htmlmail" && editorType != "textmail")
//    throw Components.results.NS_ERROR_NOT_IMPLEMENTED;
//

  //Check the my.css file to see if changes need to be written to it.
  saveCSSFileIfChanged(editorDoc);

  // Get the current definitions from compute engine and place in preamble.
  var defnListString = "";
  try {
    defnListString = GetCurrentEngine().getDefinitions();
  }
  catch(e)
  {
    dump("Unable to get definitions ("+e.message+")\n");
  }
  var preamble = editorDoc.getElementsByTagName("preamble")[0];
  if (preamble)
  {
    var oldDefnList = preamble.getElementsByTagName("definitionlist")[0];
    if (oldDefnList)
       oldDefnList.parentNode.removeChild(oldDefnList);
    if (defnListString && defnListString.length > 0)
    {
      var range = editor.document.createRange();
      var s = editor.selection;
      range.setStart(s.anchorNode, s.anchorOffset);
      range.setEnd(s.focusNode, s.focusOffset);
      defnListString = defnListString.replace(/<p>/,"<bodyText>", "g");
      defnListString = defnListString.replace(/<\/p>/,"</bodyText>", "g")
      defnListString = "<doc>" + defnListString + "</doc>";
      var defnList = editorDoc.createElement("definitionlist");
      insertXML(editor, defnListString, defnList, 0, true);
      preamble.appendChild(defnList);
      if(s.rangeCount > 0) s.removeAllRanges();
      s.addRange(range);
    }
  }
  checkPackageDependenciesForEditor(editor);

  var saveAsTextFile = msiIsSupportedTextMimeType(aMimeType);
  // check if the file is to be saved is a format we don't understand; if so, bail
  if (aMimeType != "text/html" && aMimeType != "application/xhtml+xml" && aMimeType != "text/xml" && !saveAsTextFile)
    throw Components.results.NS_ERROR_NOT_IMPLEMENTED;

  var urlstring = msiGetEditorURL(editorElement);
  var url = msiURIFromString(urlstring);
  var currentFile = msiFileFromFileURL(url);
  var compileInfo = new Object();
  if (saveAsTextFile)
    aMimeType = "text/plain";
  else if (GetBoolPref("swp.generateTeXonsave") && !noTeX)
  {
//    var file = currentFile.parent;
//    file.append("tex");
//    file.append("main.tex");
    documentAsTeXFile(editorDoc, null, compileInfo);
  }
  var success;
  success = msiOutputFileWithPersistAPI(editorDoc, currentFile, null, aMimeType, editorElement);
  if (success) editor.contentsMIMEType = aMimeType;
//  editor.selection = saveSelection;
  return success;
}

function getWorkingDirectory(editorElement)
{
  var htmlurlstring = msiGetEditorURL(editorElement);
  if (!htmlurlstring || htmlurlstring.length == 0) return;
  var htmlurl = msiURIFromString(htmlurlstring);
  var workingDir = msiFileFromFileURL(htmlurl);
  workingDir = workingDir.parent;
  return workingDir;
}

function deleteWorkingDirectory(editorElement)
{
  var workingDir = getWorkingDirectory(editorElement);
  if (!workingDir) return;
// we know we shouldn't delete the directory unless it really is a working directory; i.e., unless it
// ends with "_work/main.xhtml"
  var regEx = /_work$/i;
  if (regEx.test(workingDir.path))
  {
    try
    {
      if (workingDir.exists())
        workingDir.remove(true);
    } catch(exc) { msiDumpWithID("In deleteWorkingDirectory for editorElement [@], trying to delete directory [" + htmlpath + "]; exception is [" + exc + "].\n", editorElement); }
  }
  else alert("Trying to remove 'work directory': "+workingDir.path+"\n"); // eventually get rid of this
}


// throws an error or returns true if user attempted save; false if user canceled save
//
// Discussion:
// First we do a soft save. Once that is done, all the necessary data is on the dist, in a directory we call D.
// The original file is A.sci (a zipfile or a directory).
// There may be a previously created file A.bak.
//
// If we are doing SaveAs or SaveCopy (SaveAs is alway true when SaveCopy is true), we bring up the
// PromptForSaveLocation dialog box. Assume the filename returned is B.sci (this can be a directory).
// We also assume that if B.sci exists, the user has already given permission to overwrite it. If we
// are doing a straight save, B=A.
//
// We do the following:
//
// Save the directory D to a zipfile or or copy it to a directory called B.tempsci.
//
// If successful:
//   If A==B (a straight save), rename A.bak to A.tempbak, rename A.sci to A.bak, rename A.tempsci (which in this
//   case is also B.tempsci, which is a copy or a zip of D) to A.sci.
//   If all is successful, delete A.tempbak
//   We will now have A.sci, A.bak.
//
//   If A!=B (a save-as), delete B.bak and rename B.sci to B.bak if they exist. Rename B.tempsci to B.sci.
//
// If successful:
//   Delete directory D unless we are returning to editing.
//

function msiSaveDocument(aContinueEditing, aSaveAs, aSaveCopy, aMimeType, editor, editorElement, fUseDirectory)
{
  var tempdir;
  var success =  msiSoftSave( editor, editorElement, true);
  if (!success) {
    var saveDocStr = GetString("SaveDocument");
    var failedStr = GetString("SaveFileFailed");
    AlertWithTitle(saveDocStr, failedStr);
    throw Components.results.NS_ERROR_UNEXPECTED;
  }

  // The making of B.tempsci:
  // Say the file being edited is /somepath/DocName_work/main.xhtml
  // Then
  // htmlurlstring      = file:///somepath/DocName_work/main.xhtml
  // sciurlstring       = file:///somepath/DocName.sci  (can be a directory)
  // htmlpath           = /somepath/DocName_work/main.xhtml
  // currentSciFilePath = /somepath/DocName.sci (can be a directory)

  var htmlurlstring = msiGetEditorURL(editorElement); // this is the url of the file in the directory D. It was updated by the soft save.
  var htmlurl = msiURIFromString(htmlurlstring);
  var sciurlstring = msiFindOriginalDocname(htmlurlstring); // this is the uri of A.sci
  var mustShowFileDialog = (aSaveAs || aSaveCopy || editorElement.isShellFile || (sciurlstring === ""));

  // If editing a remote URL, force SaveAs dialog
  if (!mustShowFileDialog && GetScheme(sciurlstring) !== "file" && GetScheme(sciurlstring) !== "resource")
  {
    mustShowFileDialog = true;
  }
  var saveAsTextFile = msiIsSupportedTextMimeType(aMimeType);
  var replacing = !aSaveAs;  // hence A=B in the above discussion
  var titleChanged = false;
  var doUpdateURI = false;
  var destLocalFile = null;
  var destURI;
  var currentFile = null;
  var currentSciFile = null;
  var workingDir = null;
  var leafname;
  var isSciFile;
  var fileurl = msiURIFromString(sciurlstring);
  currentSciFile = msiFileFromFileURL(fileurl);
  currentSciFilePath = msiPathFromFileURL(fileurl);

  var regEx = /_work\/main.[a-z]?html?$/i;  // BBM: localize this
  isSciFile = regEx.test(htmlurlstring);
  if (isSciFile)
  {
    workingDir = msiFileFromFileURL(htmlurl);  // now = the path of the xhtml file in the working dir D
    workingDir = workingDir.parent;       // now = the directory D
    tempdir = workingDir.clone();
    var leaf = tempdir.leafName.replace(/_work$/,"");
    tempdir = tempdir.parent;
    tempdir.append(leaf+".sci");
    var url = msiFileURLFromFile(tempdir);
    sciurlstring = url.spec;
  }

  if (mustShowFileDialog)
  {
    var urlstring;
    try {
      // Prompt for title if we are saving to .sci
      if (!saveAsTextFile && (editor.editortype === "html"))
      {
        var userContinuing = msiPromptAndSetTitleIfNone(editorElement); // not cancel
        if (!userContinuing){
          return false;
        }
      }

      var dialogResult = msiPromptForSaveLocation(saveAsTextFile, editor.editortype==="html"?MSI_EXTENSION:editor.editortype,
        aMimeType, sciurlstring, editorElement, fUseDirectory);
      if (dialogResult.filepickerClick === msIFilePicker.returnCancel) {
        return false;
      }
      replacing = replacing || (dialogResult.filepickerClick === msIFilePicker.returnReplace);
      urlstring = dialogResult.resultingURIString;

      // jcs without .clone() this always set destLocalFile void
      destLocalFile = dialogResult.resultingLocalFile.clone();  // this is B.sci
      // update the new URL for the webshell unless we are saving a copy
      if (!Boolean(aSaveCopy)) {
        doUpdateURI = true;
      }
    } catch (e) {
       return false;
    }
    var ioService;
    try {
      // if somehow we didn't get a local file but we did get a uri,
      // attempt to create the localfile if it's a "file" url. This may be a directory.
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
      leafname = destLocalFile.leafName;
    }
    catch (ex)
    {
      var saveDocStr = GetString("SaveDocument");
      var failedStr = GetString("SaveFileFailed");
      AlertWithTitle(saveDocStr, failedStr);
      throw Components.results.NS_ERROR_UNEXPECTED;
    }
  }  // mustShowDialog
  else { // if we didn't show the File Save dialog, we need destLocalFile to be A.sci
//   currentSciFile.initWithPath( currentSciFilePath );  // now = A.sci
    leafname = tempdir.leafName;
    destLocalFile = tempdir.clone();

  }
  if (/\.sci$/i.test(leafname))
  {
    leafname = leafname.slice(0, leafname.lastIndexOf(".")); // trim off extension
  }

  var tempfile;
  if (isSciFile)
  {
    if (fUseDirectory)
    {
      // copy D to a new directory B.tempsci
      var destDir = destLocalFile.parent.clone();
      destDir.append(leafname+".tempsci");
      if (destDir.exists()) {destDir.remove(true);}
      copyDirectory(destDir, workingDir);
//   If successful, i.e., if we got this far:
//   If A==B (a straight save), rename A.bak to A.tempbak, rename A.sci to A.bak, rename A.tempsci to A.sci.
      if (!aSaveAs)
      {
        tempfile = destDir.clone();
        tempfile = tempfile.parent;
        tempfile.append(leafname+".bak");
        if (tempfile.exists()) {tempfile.moveTo(null, leafname+".tempbak");}
        tempfile = tempfile.parent;
        tempfile.append(leafname+".sci");
        if (tempfile.exists()) {tempfile.moveTo(null, leafname+".bak");}
          // rename A.tempsci to A.sci
        destDir.moveTo(null, leafname+".sci");
//   If all is successful, delete A.tempbak
//   We will now have A.sci, A.bak.
        tempfile = tempfile.parent;
        tempfile.append(leafname+".tempbak");
        if (tempfile.exists()) {tempfile.remove(true);}
      } else
      {
        // delete B.bak
        tempfile = destDir.clone();
        tempfile = tempfile.parent;
        tempfile.append(leafname+".bak");
        if (tempfile.exists()) {tempfile.remove(true);}
          // rename B.sci to B.bak
        tempfile = tempfile.parent;
        tempfile.append(leafname+".sci");
        if (tempfile.exists()) {tempfile.moveTo(null, leafname+".bak");}
          // rename B.tempsci to B.sci
        destDir.moveTo(null, leafname+".sci");
      }

//
//   If A!=B (a save-as), delete B.bak if it exists, rename B.sci to B.bak if it exists.
//   Rename B.tempsci to B.sci.
    } else
    {
      var zipfile = destLocalFile.parent.clone();

      zipfile.append(leafname+".tempsci");
      var compression;
      var prefs = Components.classes["@mozilla.org/preferences-service;1"].getService(Components.interfaces.nsIPrefBranch);
      try
      {
        compression = prefs.getIntPref("swp.sci.compression");
      }
      catch(ex2) {compression = 0;}

      // zip D into the zipfile
      try {
        var zw = Components.classes["@mozilla.org/zipwriter;1"]
                              .createInstance(Components.interfaces.nsIZipWriter);
        if (zipfile.exists()) {zipfile.remove(false);}
        zipfile.create(0, 0755);
        zw.open( zipfile, PR_RDWR | PR_CREATE_FILE | PR_TRUNCATE);
        zipDirectory(zw, "", workingDir, compression);
        zw.close();
      }
      catch(ex3) {
        throw ex3.message;
      }
//   If successful, i.e., if we got this far:
//   If A==B (a straight save), rename A.bak to A.tempbak, rename A.sci to A.bak, rename A.tempsci to A.sci.
//   If all is successful, delete A.tempbak
//   We will now have A.sci, A.bak.
//   Now delete directory D.
//
//   If A!=B (a save-as), delete B.bak if it exists, rename B.sci to B.bak if it exists.
//   Rename B.tempsci to B.sci.
//   Delete directory D if not going back to edit.
//
      if (!aSaveAs)
      {
        tempfile = zipfile.clone();
          // rename A.bak to A.tempbak
        tempfile = tempfile.parent;
        tempfile.append(leafname+".bak");
        if (tempfile.exists()) {tempfile.moveTo(null, leafname+".tempbak");}
          // rename A.sci to A.bak
        tempfile = tempfile.parent;
        tempfile.append(leafname+".sci");
        if (tempfile.exists()) tempfile.moveTo(null, leafname+".bak");
          // rename A.tempsci to A.sci
        zipfile.moveTo(null, leafname+".sci");
          // delete A.tempbak
        tempfile = tempfile.parent;
        tempfile.append(leafname+".tempbak");
        if (tempfile.exists()) {tempfile.remove(0);}
      }
      else
      {
          // delete B.bak
        tempfile = zipfile.clone();
        tempfile = tempfile.parent;
        tempfile.append(leafname+".bak");
        if (tempfile.exists()) tempfile.remove(0);
          // rename B.sci to B.bak
        tempfile = tempfile.parent;
        tempfile.append(leafname+".sci");
        if (tempfile.exists()) tempfile.moveTo(null, leafname+".bak");
          // rename B.tempsci to B.sci
        zipfile.moveTo(null, leafname+".sci");
      }
    }
    if (!aContinueEditing)
    {
      var re = /_work$/i;
      var count = 0;
      if (re.test(workingDir.leafName))
      {
        while (count < 2 && workingDir.exists()) {
          try
          {
            workingDir.remove(1); 
          }
          catch(e)
          {
            AlertWithTitle("Unable to remove working directory", "Cannot remove working directory. Does another program have one of the directory's files open?", window);
          }
          count++;
        }
      }
    }
    else
    {
      // if the editorElement did have a shell file, it doesn't any longer
      editorElement.isShellFile = false;
      editorElement.fileLeafName = destLocalFile.leafName;
      var newURI;
      var newWorkingDir;
      var newMainfile;
      if (doUpdateURI)
      {
        // the name has changed, but we want to continue to work in the working
        // directory, so change its name if its location hasn't changed, or move it
        // to its new location otherwise.
        if (destLocalFile.parent.path === workingDir.parent.path) // location not changing
        {
          newWorkingDir = workingDir.parent.clone();
          newWorkingDir.append(leafname+"_work");
          if (newWorkingDir.path !== workingDir.path)
          {
            if (newWorkingDir.exists())
            {
              try {
               newWorkingDir.remove(true); // recursive delete
              }
              catch (e) {
                AlertWithTitle("Unable to remove old working directory", "Cannot remove old working directory. Does another program have one of the directory's files (pdf, tex) open?", window);
              }
            }
            workingDir.moveTo(null, leafname+"_work");
          }
          newMainfile = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
          newMainfile.initWithPath(destLocalFile.path.replace(".sci","")+"_work");
        }
        else
        {
          workingDir.moveTo(destLocalFile.parent, leafname + "_work");
          newWorkingDir = destLocalFile.parent.clone();
          newWorkingDir.append(leafname + "_work");
          newMainfile = newWorkingDir.clone();
        }
        newMainfile.append("main.xhtml");
        //Create a new uri from nsILocalFile
        newURI = msiGetFileProtocolHandler().newFileURI(newMainfile);
        // We need to set new document uri before notifying listeners
        SetDocumentURI(newURI);
        document.getElementById("filename").value = leafname;
        msiUpdateWindowTitle();
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

  var prefs = GetPrefs();
  var path = destLocalFile.path;
  if (!(/\.sci$/.test(path))) path += ".sci";
  prefs.setCharPref("swp.lastfilesaved", path);
  if (!aSaveCopy)
  {
    pdfModCount = -1;
    editor.resetModificationCount();
  }
  // this should cause notification to listeners that document has changed

  // Set UI based on whether we're editing a remote or local url
  if (!aSaveCopy)
    msiSetSaveAndPublishUI(sciurlstring, editorElement);
  SaveRecentFilesPrefs();
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
      window.openDialog("chrome://prince/content/msiEditorPublishProgress.xul", "publishprogress",
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
        window.openDialog("chrome://editor/content/EditorPublish.xul","publish",
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
    try {
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
          var url = msiGetURIFromString(urlstring);
          var documentfile = msiFileFromFileURL(url);
          var currFilePath = GetFilepath(urlstring);
          var scifileUrlString = msiFindOriginalDocname(currFilePath);
          var scifileurl = msiURIFromString(scifileUrlString);
          var scifile = msiFileFromFileURL(scifileurl);;
          msiRevertFile( true, documentfile, false );
          createWorkingDirectory(scifile);
          msiEditorLoadUrl(editorElement, msiGetEditorURL(editorElement));
        }
      }
    }
    catch (e) {
      finalThrow(cmdFailString('revert'), e.message);
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
    try {
      msiCloseWindow();
    }
    catch (e) {
      finalThrow(cmdFailString('close'), e.message);
    }
  }
};

var msiCleanupCommand =
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
    try {
      var editorElement = msiGetActiveEditorElement();
      var editor = msiGetEditor(editorElement);
      var editorDoc = editor.document;
      var param =new Object();
      param.cleanupOptions=[];
      window.openDialog( "chrome://prince/content/cleanup.xul", "cleanup", "chrome,resizable=yes, modal,titlebar", param);
      cleanupWorkDirectory(editorDoc, getWorkingDirectory(editorElement), param.cleanupOptions );
    }
    catch (e) {
      finalThrow(cmdFailString('cleanup'), e.message);
    }
  }
};


function cleanupWorkDirectory( document, directory, cleanupOptions )
/* CleanupOptions is an array consisting of 0 or more of these strings:
auxfiles
logfiles
pdffiles
texfiles
orphanimagefiles
orphanplotfiles
cachedconversions
backupfiles */
{
  var texOptions = ["auxfiles","logfiles","pdffiles","texfiles"];
  if (texOptions.every(function(val){return cleanupOptions.indexOf(val)>=0;} ))
  {
    // getting rid of all the TeX stuff. Just delete the directory
    var texDir = directory.clone();
    texDir.append("tex");
    try
    {
      texDir.remove(true);
    }
    catch(e)
    {
      dump(e.message+"\n");
    }
    texOptions.forEach(function(val){cleanupOptions.splice(cleanupOptions.indexOf(val),1);});
//    alert(cleanupOptions.join());
  }
  else
  {
    cleanupOptions.forEach(function(val){cleanup(directory, document, val);})
  }
}

function cleanup(directory, document, option)
{
  var texDir = directory.clone();
  var cachedir;
  var texgrdir;
  texDir.append("tex");
  switch (option)
  {
    case "auxfiles": deleteFilesByPattern(texDir, /(\.log$|\.pdf$|\.tex$)/, true);
      break;
    case "logfiles": deleteFilesByPattern(texDir, /\.log$/, false);
      break;
    case "pdffiles": deleteFilesByPattern(texDir, /\.pdf$/, false);
      break;
    case "texfiles": deleteFilesByPattern(texDir, /\.tex$/, false);
      break;
    /* We can't safely do this until we are sure all subocs are in memory -- otherwise the subdocs
       written to the disk will lose their graphics and plots!
    case "orphanimagefiles": deleteOrphanedGraphics(directory, document);
      break;
    case "orphanplotfiles": deleteOrphanedPlots(directory, document);
      break; */
    case "cachedconversions": cachedir = directory.clone();
      cachedir.append("cgraphics");
      if (cachedir.exists() && cachedir.isDirectory())
      {
        deleteFilesByPattern(cachedir, /.\*$/, false);
      }
      break;
    case "backupfiles": deleteFilesByPattern(directory, /\.bak$/, false);
      break;
    default: break;
  }
}

function deleteFilesByPattern(directory, regexp, complement)
{
  if (!(directory.exists()) && directory.isDirectory()) return;
  var file;
  try
  {
    var fileenum = directory.directoryEntries;
    while (fileenum.hasMoreElements())
    {
      file = fileenum.getNext();
      file.QueryInterface(Components.interfaces.nsIFile);
      if (regexp.test(file.leafName))
      {
        if (!complement) file.remove(false);
      }
      else if (complement) file.remove(false);
    }
  }
  catch(e)
  {
    dump("Exception in deleteFilesByPattern: "+e.message+"\n");
  }
}

function deleteOrphanedGraphics( basedirectory, document)
{
  var objlist;
  var root = document.documentElement;
  objlist = document.getElementsByTagName("object");
  var length = objlist.length;
  var stringlist = new Object();
  var i;
  var node;
  var url;
  var regexp1 = /(t|c)?graphics\//;
  var regexp2 = /\.[a-zA-Z0-9]+$/;
  var str;
  var arr;
  for (i = 0; i < length; i++)
  {
    node = objlist.item(i);
    if (!(node.getAttribute("msigraph")=="true"))
    {
      url = node.getAttribute("data");
      if (!url) url = node.getAttribute("src");
      if (!url) url = node.getAttribute("href");
      if (regexp1.test(url))
      {
        // it is a local url in the graphics, cgraphics, or tgraphics directory
        arr = url.split("/");
        if (arr && arr.length > 0)
        {
          str = arr[arr.length-1];
          str = str.replace(/\.[a-zA-Z0-9]*$/,'')
          stringlist[str]=1;
        }
      }
    }
  }
  var dirlist = ["tgraphics","cgraphics","graphics"];
  var dir;
  for (i = 0; i < 3; i++)
  {
    dir = basedirectory.clone();
    dir.append(dirlist[i]);
    if (dir.exists() && dir.isDirectory())
    {
      var fileenum = dir.directoryEntries;
      var file;
      while (fileenum && fileenum.hasMoreElements())
      {
        file = fileenum.getNext();
        file.QueryInterface(Components.interfaces.nsIFile);
        str = file.leafName;
        str = str.replace(regexp2,'');
        if (!stringlist[str])  // the name wasn't found in the document
          if (file.exists()) file.remove(false);
      }
    }
  }
}

function deleteOrphanedPlots( basedirectory, document)
{
  var objlist;
  var root = document.documentElement;
  objlist = document.getElementsByTagName("object");
  var length = objlist.length;
  var stringlist = new Object();
  var i;
  var node;
  var url;
  var regexp1 = /plots\//;
  var str;
  var arr;
  for (i = 0; i < length; i++)
  {
    node = objlist.item(i);
    if (node.getAttribute("msigraph")=="true")
    {
      url = node.getAttribute("data");
      if (!url) url = node.getAttribute("src");
      if (!url) url = node.getAttribute("href");
      if (regexp1.test(url))
      {
        // it is a local url in the plots directory
        arr = url.split("/");
        if (arr && arr.length > 0)
        {
          str = arr[arr.length-1];
          str = str.replace(/\.[a-zA-Z0-9]*$/,'')
          stringlist[str]=1;
        }
      }
    }
  }
  var plotsdir = basedirectory.clone();
  plotsdir.append("plots");
  var fileenum = plotsdir.directoryEntries;
  var file;
  while (fileenum.hasMoreElements())
  {
    file = fileenum.getNext();
    file.QueryInterface(Components.interfaces.nsIFile);
    str = file.leafName;
    str.replace(/.[xvcz]*$/,'');
    if (!stringlist[str])  // the name wasn't found in the document
      if (file.exists()) file.remove(false);
  }
}

function msiCloseWindow(theWindow)
{
  if (!theWindow)
    theWindow = window;
  // Check to make sure document is saved. "true" means allow "Don't Save" button,
  //   so user can choose to close without saving
  var editorElement = msiGetPrimaryEditorElementForWindow(theWindow);
  if (msiCheckAndSaveDocument(editorElement, "cmd_close", true))
  {
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
    window.openDialog( "chrome://prince/content/openLocation.xul", "openlocation", "chrome,modal,titlebar", 0);
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
    if (!msiCheckAndSaveDocument("cmd_preview", DocumentHasBeenSaved()))
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
    if (!msiCheckAndSaveDocument("cmd_editSendPage", DocumentHasBeenSaved()))
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
    try
    {
      var editorElement = msiGetActiveEditorElement();
      var doc = editorElement.contentDocument;
#ifndef PROD_SW
      rebuildSnapshots(doc);
#endif
      msiFinishHTMLSource();
      PrintUtils.print();
    }
    catch (e) {
      finalThrow(cmdFailString('directprint'), e.message);
    }
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
    try {
      printTeX(aCommand=='cmd_printPdf',false);
    }
    catch (e) {
      finalThrow(cmdFailString('printPdf'), e.message);
    }
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
    try {
      printTeX(aCommand=='cmd_previewPdf',true);
    }
    catch (e) {
      finalThrow(cmdFailString('previewPDF'), e.message);
    }
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
     try {
       compileTeX(aCommand=='cmd_compilePdf')
     }
     catch (e) {
       finalThrow(cmdFailString('compilePDF'), e.message);
     }
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
    try {
      msiFinishHTMLSource();
      PrintUtils.showPageSetup();
    }
    catch (e) {
      finalThrow(cmdFailString('printsetup'), e.message);
    }
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
    try {
      var editorElement = msiGetActiveEditorElement();
      var editor = msiGetEditor(editorElement);
      var htmleditor = editor.QueryInterface(Components.interfaces.nsIHTMLEditor);
      htmleditor.setOneShotTranslation("greek");
    }
    catch (e) {
      finalThrow(cmdFailString('oneshotgreek'), e.message);
    }
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
    try {
      var editorElement = msiGetActiveEditorElement();
      var editor = msiGetEditor(editorElement);
      var htmleditor = editor.QueryInterface(Components.interfaces.nsIHTMLEditor);
      htmleditor.setOneShotTranslation("symbol");
    }
    catch (e) {
      finalThrow(cmdFailString('oneshotsymbol'), e.message);
    }
  }
};



//-----------------------------------------------------------------------------------

var msiCopyTeX =
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
		try {
  	  var editorElement = msiGetActiveEditorElement();
  	  var editor = msiGetEditor(editorElement);
  	  if (!editor) {
  			throw("No editor in msiCopyTeX");
  		}
  	  var selection = editor.selection;
  	  if (!selection)
  	  {
  	    throw("No selection in msiCopyTeX!");
  	  }
  	  var intermediateText;
  	  intermediateText = editor.outputToString("text/xml", kOutputFormatted | kOutputSelectionOnly);
  	  var output = xmlFragToTeX(intermediateText);
  		const gClipboardHelper = Components.classes["@mozilla.org/widget/clipboardhelper;1"].
  		getService(Components.interfaces.nsIClipboardHelper);
  		gClipboardHelper.copyString(output);
		}
    catch (e) {
      finalThrow(cmdFailString('copyTeX'), e.message);
    }
  }
};
//--------
var msiFontColor =
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
		try {
      var editorElement = msiGetActiveEditorElement();
      var colorObj = { NoDefault:true, Type:"Font", TextColor:"black", PageColor:0, Cancel:false };

      window.openDialog("chrome://prince/content/color.xul", "colorpicker", "resizable=yes, chrome,close,titlebar,modal",
      "", colorObj, null);

      // User canceled the dialog
      if (colorObj.Cancel)
        return;
  	  else {
  			msiGetEditor(editorElement).incrementModificationCount(1);
  		}

      msiEditorSetTextProperty(editorElement, "fontcolor", "color", colorObj.TextColor);
      var theWindow = msiGetTopLevelWindow();
      theWindow.msiRequirePackage(editorElement, "xcolor", null);
		}
    catch (e) {
      finalThrow(cmdFailString('fontcolor'), e.message);
    }
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
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    finalThrow(cmdFailString('quit'), "This command should not be called directly");
  }
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
    var editorElement = msiGetActiveEditorElement();
    try {
      msiOpenModelessDialog("chrome://prince/content/autoSubstituteDialog.xul", "_blank", "chrome,resizable=yes,close,titlebar,dependent",
                                        editorElement, "cmd_MSIAutoSubDlg", this, editorElement);
    }
    catch (e) {
      finalThrow(cmdFailString('autosubdialog'), e.message);
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
    try {
      var editorElement = msiGetActiveEditorElement();
      var editor = editorElement.getEditor(editorElement.contentWindow);
      if (window.gEditorDisplayMode == kDisplayModeSource)
      {
        //openFastCursorBar(false, true);  Didn't work
      }
      else
      {
        msiOpenModelessDialog("chrome://prince/content/msiEdReplace.xul", "replace", "chrome,close,titlebar,dependent,resizable",
                                          editorElement, "cmd_find", this, editorElement);
      }
    }
    catch (e) {
      finalThrow(cmdFailString('find'), e.message);
    }
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
    catch (e) {
      finalThrow(cmdFailString('findagain'), e.message);
    }
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
		try {
      var editorElement = msiGetActiveEditorElement();
      window.cancelSendMessage = false;
      window.openDialog("chrome://prince/content/EdSpellCheck.xul", "spellcheck",
              "chrome,close,titlebar,modal,resizable", false, false, true, editorElement);
  		msiGetEditor(editorElement).incrementModificationCount(1);
      editorElement.focus();
		}
    catch (e) {
      finalThrow(cmdFailString('spellcheck'), e.message);
    }
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
    event.path.forms[0].uploaded_file.value = URL2Validate;
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
    window.openDialog("chrome://editor/content/EdLinkChecker.xul","linkchecker", "chrome,close,titlebar,modal,resizable", editorElement);
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
    try {
      var editorElement = msiGetActiveEditorElement();
      var dlgWindow = msiOpenModelessDialog("chrome://editor/content/msiEdFormProps.xul", "_blank", "chrome,close,titlebar,dependent,resizable",
                                                                                                       editorElement, "cmd_form", this);
  //    window.openDialog("chrome://editor/content/msiEdFormProps.xul", "formprops", "chrome,close,titlebar,modal");
      editorElement.focus();
    }
    catch (e) {
      finalThrow(cmdFailString('previewPDF'), e.message);
    }
  },

  msiGetReviseObject: function(editorElement)
  {
    // Get a single selected form element
    try {
      var editor = msiGetEditor(editorElement);
      var theFormElement = null;
      const kTagName = "form";
      theFormElement = editor.getSelectedElement(kTagName);
      if (!theFormElement)
        theFormElement = editor.getElementOrParentByTagName(kTagName, editor.selection.anchorNode);
      if (!theFormElement)
        theFormElement = editor.getElementOrParentByTagName(kTagName, editor.selection.focusNode);
    }
    catch (e) {
      finalThrow(cmdFailString('getreviseobject'), e.message);
    }
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
    var formNode = msiGetReviseObjectFromCommandParams(aParams);
    if (formNode != null && editorElement != null)
    {
      AlertWithTitle("msiComposerCommands.js", "In msiReviseFormCommand, trying to revise a form, dialog not yet implemented.");
//      var dlgWindow = msiDoModelessPropertiesDialog("chrome://editor/content/msiEdFormProps.xul", "_blank", "chrome,close,titlebar,dependent",
//                                                     editorElement, "cmd_reviseForm", formNode);
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
    var dlgWindow = msiOpenModelessDialog("chrome://editor/content/EdInputProps.xul", "_blank", "chrome,close,titlebar,dependent,resizable",
                                                                                                     editorElement, "cmd_inputtag", this);
//    dlgWindow.focus();  is this necessary?
//    window.openDialog("chrome://editor/content/EdInputProps.xul", "inputprops", "chrome,close,titlebar,modal");
//    editorElement.focus();
  },

  msiGetReviseObject: function(editorElement)
  {
    var inputElement = null;
    const kTagName = "input";
    var editor = msiGetEditor(editorElement);
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
    try {
      var editorElement = msiGetActiveEditorElement();
      var dlgWindow = msiOpenModelessDialog("chrome://editor/content/msiEdInputImage.xul", "inputimage", "chrome,close,titlebar,dependent,resizable",
        editorElement, "cmd_inputimage", this);
    }
    catch (e) {
      finalThrow(cmdFailString('inputimage'), e.message);
    }
  },

  msiGetReviseObject: function(editorElement)
  {
    var inputElement = null;
    const kTagName = "input";
    var editor = msiGetEditor(editorElement);
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
    var dlgWindow = msiOpenModelessDialog("chrome://editor/content/msiEdTextAreaProps.xul", "_blank", "chrome,close,titlebar,dependent,resizable",
                                                                                                     editorElement, "cmd_textarea", this);
//    window.openDialog("chrome://editor/content/EdTextAreaProps.xul", "_blank", "chrome,close,titlebar,modal");
//    editorElement.focus();
  },

  msiGetReviseObject: function(editorElement)
  {
    var textAreaElement = null;
    const kTagName = "textarea";
    var editor = msiGetEditor(editorElement);
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
    var textAreaNode = msiGetReviseObjectFromCommandParams(aParams);
    if (textAreaNode != null && editorElement != null)
    {
      AlertWithTitle("msiComposerCommands.js", "In msiReviseTextareaCommand, trying to revise a textarea, dialog not yet implemented.");
//      var dlgWindow = msiDoModelessPropertiesDialog("chrome://editor/content/msiEdTextAreaProps.xul", "_blank", "chrome,close,titlebar,dependent",
//                                                     editorElement, "cmd_reviseTextarea", textAreaNode);
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
    var dlgWindow = msiOpenModelessDialog("chrome://editor/content/msiEdSelectProps.xul", "selectprops", "chrome,close,titlebar,dependent,resizable",
                                                                                                     editorElement, "cmd_select", this);
//    window.openDialog("chrome://editor/content/EdSelectProps.xul", "selectprops", "chrome,close,titlebar,modal", editorElement);
//    editorElement.focus();
  },

  msiGetReviseObject: function(editorElement)
  {
    var selectElement = null;
    const kTagName = "select";
    var editor = msiGetEditor(editorElement);
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
    var dlgWindow = msiOpenModelessDialog("chrome://editor/content/msiEdButtonProps.xul", "buttonprops", "chrome,close,titlebar,dependent,resizable",
                                                                                                     editorElement, "cmd_button", this);
//    window.openDialog("chrome://editor/content/EdButtonProps.xul", "buttonprops", "chrome,close,titlebar,modal", editorElement);
//    editorElement.focus();
  },

  msiGetReviseObject: function(editorElement)
  {
    var buttonElement = null;
    const kTagName = "button";
    var editor = msiGetEditor(editorElement);
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
    var buttonNode = msiGetReviseObjectFromCommandParams(aParams);
    if (buttonNode != null && editorElement != null)
    {
      AlertWithTitle("msiComposerCommands.js", "In msiReviseButtonCommand, trying to revise a button, dialog not yet implemented.");

    }
    editorElement.focus();
  },

  doCommand: function(aCommand, dummy)  {}
};

//-----------------------------------------------------------------------------------
//var msiLabelCommand =
//{
//  isCommandEnabled: function(aCommand, dummy)
//  {
//    var editorElement = msiGetActiveEditorElement();
//    return (msiIsDocumentEditable(editorElement) && msiIsEditingRenderedHTML(editorElement));
//  },
//
//  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
//  doCommandParams: function(aCommand, aParams, aRefCon) {},
//
//  doCommand: function(aCommand, dummy)
//  {
//    var editorElement = msiGetActiveEditorElement();
////    var tagName = "label";
//    try {
//      var editor = msiGetEditor(editorElement);
//      var labelElement = this.msiGetReviseObject(editorElement);
////      // Find selected label or if start/end of selection is in label
//      if (labelElement) {
//        // We only open the dialog for an existing label
//        var dlgWindow = msiOpenModelessDialog("chrome://editor/content/msiEdLabelProps.xul", "_blank", "chrome,close,titlebar,dependent",
//                                                                        editorElement, "cmd_label", this, labelElement);
////        msiOpenModalDialog("chrome://editor/content/msiEdLabelProps.xul", "labelprops", "chrome,close,titlebar,dependent",
////                                                                                                     editorElement, "cmd_label", this);
////        window.openDialog("chrome://editor/content/EdLabelProps.xul", "labelprops", "chrome,close,titlebar,modal", labelElement);
////        editorElement.focus();
//      } else {
//        msiEditorSetTextProperty(editorElement, tagName, "", "");
//      }
//    } catch (e) {}
//  },
//
//  msiGetReviseObject: function(editorElement)
//  {
//    var labelElement = null;
//    const kTagName = "label";
//    var editor = msiGetEditor(editorElement);
//    try {
//      // Find selected label or if start/end of selection is in label
//      labelElement = editor.getSelectedElement(kTagName);
//      if (!labelElement)
//        labelElement = editor.getElementOrParentByTagName(kTagName, editor.selection.anchorNode);
//      if (!labelElement)
//        labelElement = editor.getElementOrParentByTagName(kTagName, editor.selection.focusNode);
//    } catch (e) {}
//    return labelElement;
//  }
//};
//
//var msiReviseLabelCommand =
//{
//  isCommandEnabled: function(aCommand, dummy)  {return true;},
//  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
//  doCommandParams: function(aCommand, aParams, aRefCon)
//  {
//    var editorElement = msiGetActiveEditorElement();
//    var labelNode = msiGetReviseObjectFromCommandParams(aParams);
//    if (labelNode != null && editorElement != null)
//    {
//      AlertWithTitle("msiComposerCommands.js", "In msiReviseLabelCommand, trying to revise a label, dialog not yet implemented.");
////      var dlgWindow = msiDoModelessPropertiesDialog("chrome://editor/content/what??.xul", "_blank", "chrome,close,titlebar,dependent",
////                                                     editorElement, "cmd_reviseLabel", labelNode);
//    }
//    editorElement.focus();
//  },
//
//  doCommand: function(aCommand, dummy)  {}
//};

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
    var dlgWindow = msiOpenModelessDialog("chrome://editor/content/msiEdFieldSetProps.xul", "fieldsetprops", "chrome,close,titlebar,dependent,resizable",
                                                                        editorElement, "cmd_fieldset", this);
//    window.openDialog("chrome://editor/content/msiEdFieldSetProps.xul", "fieldsetprops", "chrome,close,titlebar,modal");
//    editorElement.focus();
  },

  msiGetReviseObject: function(editorElement)
  {
    var fieldsetElement = null;
    const kTagName = "fieldset";
    var editor = msiGetEditor(editorElement);
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
    var fieldSetNode = msiGetReviseObjectFromCommandParams(aParams);
    if (fieldSetNode != null && editorElement != null)
    {
      AlertWithTitle("msiComposerCommands.js", "In msiReviseFieldsetCommand, trying to revise a fieldset, dialog not yet implemented.");
// TODO: Ron
//      var dlgWindow = msiDoModelessPropertiesDialog("chrome://editor/content/what??.xul", "_blank", "chrome,close,titlebar,dependent",
//                                                     editorElement, "cmd_reviseFieldset", fieldSetNode);
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
    var imageData = {isVideo : false, mNode : null};
    var dlgWindow = msiOpenModelessDialog("chrome://prince/content/msiEdImageProps.xul", "imageprops", "chrome, resizable, close,titlebar,dependent,resizable",
                                                                                                     editorElement, "cmd_image", this, imageData);
  },
  msiGetReviseObject: function(editorElement)
  {
    var imageElement = null;
    var editor = msiGetEditor(editorElement);
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
    var imageNode = msiGetReviseObjectFromCommandParams(aParams);
    if (imageNode != null && editorElement != null)
    {
      var imageData = {isVideo : false, mNode : imageNode};
      var dlgWindow = msiDoModelessPropertiesDialog("chrome://prince/content/msiEdImageProps.xul", "_blank", "chrome,close,titlebar,resizable, dependent",
                                                     editorElement, "cmd_reviseImage", imageNode, imageData);
    }
    editorElement.focus();
  },

  doCommand: function(aCommand, dummy)  {}
};

//-----------------------------------------------------------------------------------
//Once again, need to tie the editor to the dialog.
var msiVideoCommand =
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
    var imageData = {isVideo : true, mNode : null};
    var dlgWindow = msiOpenModelessDialog("chrome://prince/content/msiEdImageProps.xul", "", "chrome, resizable, close,titlebar,dependent,resizable",
                                                                                                     editorElement, "cmd_video", this, imageData);
//    window.openDialog("chrome://editor/content/EdImageProps.xul","imageprops", "chrome,close,titlebar,modal");
//    editorElement.focus();
  },
  msiGetReviseObject: function(editorElement)
  {
    var videoElement = null;
    var editor = msiGetEditor(editorElement);
    try {
      // Get a single selected video element
      var videoTags = ["embed", "object"];
      for (var ix = 0; (!videoElement) && (ix < 2); ++ix)
      {
        videoElement = editor.getSelectedElement(videoTags[ix]);
        if (videoElement && !(videoElement.getAttribute("isVideo") == "true"))
          videoElement = null;
      }
    } catch (e) {}
    return videoElement;
  }
};

var msiReviseVideoCommand =
{
  isCommandEnabled: function(aCommand, dummy)  {return true;},
  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon)
  {
    var editorElement = msiGetActiveEditorElement();
    var videoNode = msiGetReviseObjectFromCommandParams(aParams);
    if (videoNode != null && editorElement != null)
    {
      var imageData = {isVideo : true, mNode : videoNode};
      var dlgWindow = msiDoModelessPropertiesDialog("chrome://prince/content/msiEdImageProps.xul", "imageprops", "chrome,close,titlebar,resizable, dependent",
                                                     editorElement, "cmd_reviseVideo", videoNode, imageData);
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
      window.openDialog("chrome://editor/content/EdHLineProps.xul", "hlineprops", "chrome,close,titlebar,modal,resizable");
			msiGetEditor(editorElement).incrementModificationCount(1);
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
    var element = msiGetObjectDataForProperties(editorElement);
    if (element && msiGetBaseNodeName(element) == "img")
    {
      var imageData = {isVideo : false, mNode : element};
      msiDoModelessPropertiesDialog("chrome://prince/content/msiEdImageProps.xul", "imageprops", "chrome,close,titlebar,resizable, dependent",
                                                     editorElement, "cmd_reviseImage", element, imageData);
//      window.openDialog("chrome://prince/content/msiEdImageProps.xul","imageprops", "resizable,chrome,close,titlebar,dependent", imageData);
    }
    else
      window.openDialog("chrome://prince/content/EdLinkProps.xul","linkprops", "resizable,chrome,close,titlebar,dependent");
		msiGetEditor(editorElement).incrementModificationCount(1);
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
    var linkNode = msiGetReviseObjectFromCommandParams(aParams);
    if (linkNode != null && editorElement != null)
    {
      window.openDialog("chrome://prince/content/EdLinkProps.xul","linkprops", "resizable,chrome,close,titlebar,dependent");
    }
		msiGetEditor(editorElement).incrementModificationCount(1);
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
    window.openDialog("chrome://editor/content/EdNamedAnchorProps.xul", "namedanchorprops", "chrome,close,titlebar,modal,resizable", "", editorElement);
		msiGetEditor(editorElement).incrementModificationCount(1);
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
    var anchorNode = msiGetReviseObjectFromCommandParams(aParams);
    if (anchorNode != null && editorElement != null)
    {
//      AlertWithTitle("msiComposerCommands.js", "In msiReviseAnchorCommand, trying to revise hyperlink anchor, dialog not implemented.");
      var dlgWindow = msiDoModelessPropertiesDialog("chrome://prince/content/marker.xul", "_blank", "chrome, resizable, close, titlebar, dependent", editorElement, "cmd_reviseAnchor", this, anchorNode);
    }
		msiGetEditor(editorElement).incrementModificationCount(1);
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
    window.openDialog("chrome://editor/content/EdInsSrc.xul","insertsource", "chrome,close,titlebar,modal,resizable", "", editorElement);
		msiGetEditor(editorElement).incrementModificationCount(1);
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
    var htmlNode = msiGetReviseObjectFromCommandParams(aParams);
    if (htmlNode != null && editorElement != null)
    {
      AlertWithTitle("msiComposerCommands.js", "In msiReviseHTMLCommand, trying to revise encapsulated HTML, dialog not implemented.");
//      window.openDialog("chrome://editor/content/EdInsSrc.xul","insertsource", "chrome,close,titlebar,modal,resizable", "", editorElement);
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
    var dlgWindow = msiOpenModelessDialog("chrome://editor/content/msiEdReviseChars.xul", "_blank", "chrome, resizable, close, titlebar, dependent", editorElement, "cmd_insertChars", this);
//    msiEditorFindOrCreateInsertCharWindow(editorElement);
  }
};

var msiReviseCharsCommand =
{
  isCommandEnabled: function(aCommand, dummy)  {return true;},
  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon)
  {
    try {
      var editorElement = msiGetActiveEditorElement();
      var charReviseData = msiGetPropertiesDataFromCommandParams(aParams);
      var charData = new Object();
      charData.reviseData = charReviseData;
      if (charReviseData != null && editorElement != null)
      {
        var dlgWindow = msiDoModelessPropertiesDialog("chrome://prince/content/msiEdReviseChars.xul", "_blank", "chrome, resizable, close, titlebar, dependent", editorElement, "cmd_reviseChars", this, charData);
      }
      editorElement.focus();
    }
    catch (e) {
      finalThrow(cmdFailString('revisechars'), e.message);
    }
  },

  doCommand: function(aCommand, dummy)  {}
};

function getStandAloneForm(aUniChar)
{
  var retVal = null;
  switch(aUniChar)
  {
    case "\u0302":
      retVal = "^";
    break;
    case "\u030c":
      retVal =  "\u02c7";
    break;
    case "\u0303":
      retVal = "~";
    break;
    case "\u0301":
      retVal = "\u00b4";
    break;
    case "\u0300":
      retVal = "\u0060";
    break;
    case "\u0306":
      retVal = "\u02d8";
    break;
    case "\u0305":
      retVal = "\u00af";
    break;
    case "\u030b":
      retVal = "\u02dd";
    break;
    case "\u030a":
      retVal = "\u02da";
    break;
    case "\u0307":
      retVal = "\u02d9";
    break;
    case "\u0308":
      retVal = "\u00a8";
    break;
      retVal = "\u20db";
    break;
    case "\u0327":
      retVal = "\u00b8";
    break;
    case "\u0328":
      retVal = "\u02db";
    break;
    case "\u0323":
      retVal = "\u02d3";
    break;
    case "\u0332":
      retVal = "\u005f";
    break;
    default:
      retVal = aUniChar;  //Just return what was passed in
    break;
  }
  return retVal;
}

function msiRevCharQuick(accent)
{
  var editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  var sel = editor.selection; // this operation applies only to the last character in the selection, if it is a single-character mi or mo (in math)
  var isMath = isInMath(editorElement);
  var lastNode = sel.focusNode; 
  var offset = sel.focusOffset;
  var firstNode = sel.anchorNode;
  var firstOffset = sel.anchorOffset;
  var wasCollapsed = sel.isCollapsed;
  var newNode;
  var mathNode;
  var minode;
  var baseminode;
  if (1 === msiNavigationUtils.comparePositions(sel.anchorNode, sel.anchorOffset, sel.focusNode, sel.focusOffset)) {
    lastNode = sel.anchorNode;
    offset = sel.anchorOffset;
    firstNode = sel.focusNode;
    firstOffset = sel.focusOffset;
  }
  var character;
  var combined = {};
  var inoutParent = {};
  var inoutOffset = {};
  var normalizer = Components.classes["@mozilla.org/intl/unicodenormalizer;1"]
                               .createInstance(Components.interfaces.nsIUnicodeNormalizer);
  if (!isMath) {
    var forceMath = (accent[0] == '\u20d7');
    //  assert lastNode is a text node
    while (lastNode && lastNode.nodeType !== Node.TEXT_NODE) lastNode = lastNode.lastChild;
    if ( !lastNode || lastNode.nodeType !== Node.TEXT_NODE) return;
    if (offset === 0) return; // TODO: it is possible that there is a contiguous text node, and that we want to decorate the
      // last character in that node;
    editor.beginTransaction();
    character = lastNode.nodeValue.slice(offset-1,offset);
    editor.selection.collapse(lastNode, offset -1);
    editor.selection.extend(lastNode, offset);
    if (forceMath) {
      accent[0]= getStandAloneForm(accent[0]);
      var htmlstr = "<math xmlns='http://www.w3.org/1998/Math/MathML'><mover><mi>#1</mi><mi>#2</mi></mover></math>";
      htmlstr = htmlstr.replace('#1', character);
      htmlstr = htmlstr.replace('#2', accent);
      editor.insertHTML(htmlstr);
      // if the cursor (which is collapsed) is in the <mover>, move it to just after.
      var mover = editor.selection.anchorNode;
      while (mover && mover.tagName !== 'mover' && mover.tagName !== 'body') mover = mover.parentNode;
      if (mover && mover.tagName === 'mover') {
        editor.setCaretAfterElement(mover);
      }
    }
    else {
      normalizer.NormalizeUnicodeNFC(character+accent, combined);
      editor.insertText(combined.value);
//      editor.selection.collapse(firstNode, firstOffset); 
//      editor.selection.extend(lastNode, offset -1 + combined.value.length);
    }
    editor.endTransaction();
  }
  else
  {
    //we're in math. We require that lastNode be in an mi or an mo, and that there is only one text character there.
    var node;
    if (lastNode.nodeType === Node.TEXT_NODE) node = lastNode.parentNode;
    else node = lastNode;
    while (node && node.tagName !== "mo" && node.tagName !== "mi") {
      node = node.lastChild;
    }
    if ((node == null) || (node.tagName !== "mo" && node.tagName !== "mi"))  return;
    editor.beginTransaction();
    editor.selectElement(node);
    var newNode = editor.document.createElementNS(mmlns, "mover");
    editor.insertNode(newNode, sel.anchorNode, sel.anchorOffset);
    editor.insertNode(node, newNode, 0);
    var minode = editor.document.createElementNS(mmlns, "mi");
    editor.insertNode(minode, newNode, 1);
    var newTextNode = editor.document.createTextNode(getStandAloneForm(accent));
    editor.insertNode(newTextNode, minode, 0);
    editor.endTransaction();
    editor.setCaretAfterElement(newNode);
    if (!wasCollapsed) {
        editor.selection.extend(firstNode, firstOffset);
    }
  }
}



function msiReviseChars(reviseData, dialogData, editorElement)
{
  var editor = msiGetEditor(editorElement);
  editor.beginTransaction();

  var refNode = reviseData.getReferenceNode();
  var theParentNode = refNode.parentNode;
//  var leftNodeObj = new Object();
//  var midNodeObj = new Object();
  var bForceMath = (dialogData.mUpperAccent && msiNavigationUtils.upperAccentForcesMath(dialogData.mUpperAccent));
  bForceMath = bForceMath || (dialogData.mLowerAccent && msiNavigationUtils.lowerAccentForcesMath(dialogData.mLowerAccent));
  var bIsText = msiNavigationUtils.isTextNode(refNode);

  var theUpperAccent = dialogData.mUpperAccent;
  var theLowerAccent = dialogData.mLowerAccent;
  var newNode = null;
  var currNodeName = msiGetBaseNodeName(refNode)
  var baseNodeName = "mi";

  function getBaseNode(aTopNode)
  {
    switch(msiGetBaseNodeName(aTopNode))
    {
      case "mover":
      case "munder":
      case "munderover":
        return getBaseNode(msiNavigationUtils.getIndexedSignificantChild(refNode, 0));
      break;
      default:
        return aTopNode;
      break;
    }
  }

  var theBaseNode = getBaseNode(refNode);
  if (msiNavigationUtils.isMathMLLeafNode(theBaseNode))
    baseNodeName = msiGetBaseNodeName(theBaseNode);

  var newNodeName = "";
  var startOffset, endOffset;
  if (bIsText)
  {
    startOffset = reviseData.getTextOffset();
    endOffset = startOffset + reviseData.getTextLength();
  }
  if (!bIsText || bForceMath)  //this is the case where we're creating a (complex) non-text node
  {
//    if (msiNavigationUtils.upperAccentCombinesWithCharInMath(theUpperAccent))
//      theUpperAccent = null;
//    else
      theUpperAccent = dialogData.mUpperAccentStandAlone;
//    if (msiNavigationUtils.lowerAccentCombinesWithCharInMath(theLowerAccent))
//      theLowerAccent = null;
//    else
      theLowerAccent = dialogData.mLowerAccentStandAlone;
    if (theUpperAccent)
    {
      if (theLowerAccent)
        newNodeName = "munderover";
      else
        newNodeName = "mover";
    }
    else if (theLowerAccent)
      newNodeName = "munder";
//    else if (bForceMath && bIsText)
    else
//      newNodeName = "mi";
      newNodeName = baseNodeName;
    if (newNodeName.length && (currNodeName != newNodeName))
      newNode = editor.document.createElementNS(mmlns, newNodeName);
  }

  var aLogStr = "In msiComposerCommands.js, msiReviseChars(); bIsText is [";
  aLogStr += bIsText ? "true" : "false";
  aLogStr += "] and bForceMath is [";
  aLogStr += bForceMath ? "true" : "false";
  aLogStr += "], while theUpperAccent is [" + theUpperAccent + "] and theLowerAccent is [" + theLowerAccent + "]; refNode";
  msiKludgeLogNodeContentsAndAllAttributes(refNode, ["reviseChars"], aLogStr, true);
  if (newNode)
    msiKludgeLogNodeContentsAndAllAttributes(newNode, ["reviseChars"], "  while newNode", true);

  if (newNode)  //get the node inserted
  {
    if (!bIsText)
    {
      if (msiNavigationUtils.isMathTemplate(newNode) && msiNavigationUtils.isMathTemplate(refNode))
        msiEditorMoveChildren(newNode, refNode, editor)
      msiCopyElementAttributes(newNode, refNode, editor);
//    var oldParts = msiNavigationUtils.treatMathNodeAsAccentedCharacter(refNode);
      editor.replaceNode(newNode, refNode, theParentNode);
      msiKludgeLogNodeContentsAndAllAttributes(refNode, ["reviseChars"], "After msiEditorMoveChildren and editor.replaceNode calls, refNode", true);
      if (msiNavigationUtils.isMathMLLeafNode(refNode))
      {
        switch(newNodeName)
        {
          case "mover":
          case "munder":
          case "munderover":
            editor.insertNode(refNode, newNode, 0);
          break;
        }
      }
      refNode = newNode;
    }
    else
    {
      msiEditorReplaceTextWithNode2(editor, refNode, startOffset, endOffset, newNode);
      refNode = newNode;
    }
  }

  function checkChild(parent, index, mathNodeName, newText)
  {
    var ourLogStr = "  Inside checkChild(), with index [" + index + "], parent";
    msiKludgeLogNodeContents(parent, ["reviseChars"], ourLogStr, false);
    if ((parent.childNodes.length <= index) || (parent.childNodes[index] == null))
    {
      var newChild = editor.document.createElementNS(mmlns, mathNodeName);
      editor.insertNode(newChild, parent, index);
      msiKludgeLogNodeContentsAndAllAttributes(parent, ["reviseChars"], "    In checkChild, inside the insertNode clause after inserting, parent", false);
    }
    if (parent.childNodes[index].textContent != newText)
    {
      var newTextNode = editor.document.createTextNode(newText);
      msiKludgeLogNodeContentsAndAllAttributes(parent.childNodes[index], ["reviseChars"], "    In checkChild, before replacing or inserting text, child", false);
      if (parent.childNodes[index].childNodes.length)
        editor.replaceNode(newTextNode, parent.childNodes[index].childNodes[0], parent.childNodes[index]);
      else
        editor.insertNode(newTextNode, parent.childNodes[index], 0);
    }
    msiKludgeLogNodeContentsAndAllAttributes(parent.childNodes[index], ["reviseChars"], "    In checkChild, after inserting and before end, child node", true);
  }

  function checkLeaf(leafNode, newText)
  {
    if (leafNode.textContent != newText)
    {
      var newTextNode = editor.document.createTextNode(newText);
      msiKludgeLogNodeContentsAndAllAttributes(leafNode, ["reviseChars"], "    In checkLeaf, before replacing or inserting text, child", false);
      if (leafNode.childNodes.length)
        editor.replaceNode(newTextNode, leafNode.childNodes[0], leafNode);
      else
        editor.insertNode(newTextNode, leafNode, 0);
    }
  }

  function checkContents(mathNode, baseText, lowerAccent, upperAccent)
  {
    if (msiNavigationUtils.isMathMLLeafNode(mathNode))
      return checkLeaf(mathNode, baseText);

//    checkChild(mathNode, 0, "mi", baseText);
    checkChild(mathNode, 0, baseNodeName, baseText);
    switch(msiGetBaseNodeName(mathNode))
    {
      case "mi":
      case "mo":
      case "mn":
      break;
      case "mover":
        checkChild(mathNode, 1, "mi", upperAccent);
      break;
      case "munder":
        checkChild(mathNode, 1, "mi", lowerAccent);
      break;
      case "munderover":
        checkChild(mathNode, 1, "mi", lowerAccent);
        checkChild(mathNode, 2, "mi", upperAccent);
      break;
    }
  }

  msiKludgeLogString("In msiComposerCommands.js, msiReviseChars, before checkContents or msiEditorReplaceTextWithText call.\n", ["reviseChars"]);
  if (bIsText && !bForceMath) {
    msiEditorReplaceTextWithText(editor, refNode, startOffset, endOffset, dialogData.mCompiledText);
    var sel = editor.selection;
    sel.collapse(refNode, endOffset);
    //top.document.commandDispatcher.focusedWindow.focus();
  }
  else
    checkContents(refNode, dialogData.mCompiledBaseChar, theLowerAccent, theUpperAccent);

  editor.endTransaction();
}

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
    try {
      var editorElement = msiGetActiveEditorElement();
      msiGetEditor(editorElement).insertHTML("<msibr/>");
    }
    catch (e) {
      finalThrow(cmdFailString('insertbreak'), e.message);
    }
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
    try {
      var editorElement = msiGetActiveEditorElement();
      msiGetEditor(editorElement).insertHTML("<br clear='all'/>");
    }
    catch (e) {
      finalThrow(cmdFailString('getreviseobject'), e.message);
    }
  }
};


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
    try {
      var editorElement = msiGetActiveEditorElement();
      var hSpaceData = new Object();
      hSpaceData.spaceType = "normalSpace";
      msiOpenModelessDialog("chrome://prince/content/HorizontalSpaces.xul", "_blank", "chrome,close,titlebar,dependent,resizable",
                                          editorElement, "cmd_insertHorizontalSpaces", this, hSpaceData);
    }
    catch (e) {
      finalThrow(cmdFailString('inserthorizspace'), e.message);
    }
  }
};

var msiReviseHorizontalSpacesCommand =
{
  isCommandEnabled: function(aCommand, dummy)  {return true;},
  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon)
  {
    try {
      var editorElement = msiGetActiveEditorElement();
      var hSpaceReviseData = msiGetPropertiesDataFromCommandParams(aParams);
      var hSpaceData = new Object();
      hSpaceData.reviseData = hSpaceReviseData;
      if (hSpaceReviseData != null && editorElement != null)
      {
        var dlgWindow = msiDoModelessPropertiesDialog("chrome://prince/content/HorizontalSpaces.xul", "_blank", "chrome,close,titlebar,dependent,resizable",
                                                       editorElement, "cmd_reviseHorizontalSpaces", this, hSpaceData);
      }
      editorElement.focus();
    }
    catch (e) {
      finalThrow(cmdFailString('revisehorizspace'), e.message);
    }
  },

  doCommand: function(aCommand, dummy)  {}
};

function msiInsertStockSpace(spacename)
{
	var editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  if (spacename === "normalSpace")
	{
		editor.insertText(" ");
		return;
  }
  var node;
  try {
    node = editor.document.createElement('hspace');
  }
  catch (e) {
    dump("Unable to create node in msiInsertHorizontalSpace: "+e.message+"\n");
  }
  node.setAttribute('type',spacename);
  var dimsStr = msiSpaceUtils.getHSpaceDims(spacename);
  if (dimsStr)
    node.setAttribute('dim',dimsStr);
  contentStr = msiSpaceUtils.getHSpaceDisplayableContent(spacename);
  if (contentStr)
    node.textContent=contentStr;
  editor.insertElementAtSelection(node,true);
}

function msiInsertHorizontalSpace(dialogData, editorElement)
{
  var editor = msiGetEditor(editorElement);
  var dimsStr, contentStr;
  if (dialogData.spaceType == "normalSpace") editor.insertText(" ");
//  var dimensionsFromSpaceType =
//  {
////    requiredSpace:
////    nonBreakingSpace:
//    emSpace:        "1em",
//    twoEmSpace:       "2em",
//    thinSpace:      "0.17em",
//    thickSpace:     "0.5em",
//    italicCorrectionSpace: "0.083en",
//    negativeThinSpace:   "0.0em",
//    zeroSpace:           "0.0em",
//    noIndent:            "0.0em"
//  };
//  var contentFromSpaceType =
//  {
//    requiredSpace:     "&#x205f;",  //MEDIUM MATHEMATICAL SPACE in Unicode?
//    nonBreakingSpace:  "&#x00a0;",
//    emSpace:           "&#x2003;",
//    twoEmSpace:          "&#x2001;",  //EM QUAD
//    thinSpace:         "&#x2009;",
//    thickSpace:        "&#x2002;",  //"EN SPACE" in Unicode?
//    italicCorrectionSpace:  "&#x200a;",  //the "HAIR SPACE" in Unicode?
//    zeroSpace:          "&#x200b;"
////    negativeThinSpace:
////    noIndent:
//  };
//  var specialShowInvisibleChars =
//  {
//    noIndent:          "&#x2190;"  //left arrow
//  };

 // editor.deleteSelection(1);
  var parent = editor.selection.focusNode;
  var offset = editor.selection.focusOffset;
  var node = null;

  try {
    if (isInMath(editorElement)) {
       node = editor.document.createElementNS(mmlns, 'mspace');
       node.setAttribute('width', "thickmathspace");
    } else {
       node = editor.document.createElement('hspace');
    }
  }
  catch (e) {
    dump("Unable to create node in msiInsertHorizontalSpace: "+e.message+"\n");
  }

  var invisContent = null;
  if (dialogData.spaceType != "customSpace")
  {
    node.setAttribute('type',dialogData.spaceType);
    dimsStr = msiSpaceUtils.getHSpaceDims(dialogData.spaceType);
    if (dimsStr)
      node.setAttribute('dim',dimsStr);
  }
  else if (dialogData.customSpaceData.customType == "fixed")
  {
    node.setAttribute('type','customSpace');
    dimsStr = String(dialogData.customSpaceData.fixedData.size) + dialogData.customSpaceData.fixedData.units;
    node.setAttribute('dim',dimsStr);
    node.setAttribute('atEnd',(dialogData.customSpaceData.typesetChoice=='always'?'true':'false'));
    node.setAttribute('style','min-width: ' + dimsStr);
  }
  else if (dialogData.customSpaceData.customType == "stretchy")
  {
    node.setAttribute('type','customSpace');
    node.setAttribute('class','stretchySpace');
    node.setAttribute('flex', String(dialogData.customSpaceData.stretchData.factor));
    if (dialogData.customSpaceData.stretchData.fillWith == "fillLine")
      node.setAttribute('fillWith','line');
    else if (dialogData.customSpaceData.stretchData.fillWith == "fillDots")
      node.setAttribute('fillWith','dots');
    node.setAttribute('atEnd',(dialogData.customSpaceData.typesetChoice=='always'?'true':'false'));
  }
  contentStr = msiSpaceUtils.getHSpaceDisplayableContent(dialogData.spaceType);
  if (contentStr)
    node.textContent=contentStr;
  editor.insertElementAtSelection(node,true);
}

function msiReviseHorizontalSpace(reviseData, dialogData, editorElement)
{
  var editor = msiGetEditor(editorElement);
  editor.beginTransaction();
  var spaceInfo = reviseData.getSpaceInfo();
  var bWasText = reviseData.isTextReviseData();
  var bIsText = (dialogData.spaceType == "normalSpace");
  var contentStr = msiSpaceUtils.getHSpaceDisplayableContent(dialogData.spaceType);
//  var invisContent = msiSpaceUtils.getHSpaceShowInvis(dialogData.spaceType);
  var cntNodeList = null;
  var logStr = "";
  if (contentStr && contentStr.length)
  {
    parser = new DOMParser();
    contentStr = "<body>" + contentStr + "</body>";
    msiKludgeLogString("In msiComposerCommands.js, in msiReviseHorizontalSpace, retrieving content string and got [" + contentStr + "].\n", ["spaces"]);
    var cntDoc = parser.parseFromString(contentStr,"application/xhtml+xml");
    cntNodeList = cntDoc.documentElement.childNodes;
  }
  logStr = "In msiComposerCommands.js, in msiReviseHorizontalSpace; bIsText is [";
  if (bIsText)
    logStr += "true], and bWasText is [";
  else
    logStr += "false], and bWasText is [";
  if (bWasText)
    logStr += "true].\n";
  else
    logStr += "false].\n";
  msiKludgeLogString(logStr, ["spaces"]);

  var currentNode = reviseData.getReferenceNode();
  var aParentNode = currentNode.parentNode;;
  if (bIsText)
  {
    //If the type has changed, need to get the proper text and insert it - just a space?
    if (dialogData.spaceType != spaceInfo.theSpace)
    {
      var insertPos = msiNavigationUtils.offsetInParent(currentNode);
      if (!cntNodeList)
      {
        var spaceNode = currentNode.ownerDocument.createTextNode(" ");
        cntNodeList = [spaceNode];
      }
      logStr = "In msiComposerCommands.js, in msiReviseHorizontalSpace() inside bIsText case; currentNode is [" + currentNode.nodeName + "] and parent contains [" + aParentNode.childNodes.length + "] children.\n  Inserting following nodes: ";
      for (jx = 0; jx < cntNodeList.length; ++jx)
      {
        editor.insertNode(cntNodeList[jx], currentNode.parentNode, insertPos + jx);
        if (jx > 0)
          logStr += ", [" + cntNodeList[jx].nodeName + "]";
        else
          logStr += "[" + cntNodeList[jx].nodeName + "]";
      }
      editor.deleteNode(currentNode);
      logStr += "and deleting currentNode. Parent now has [" + aParentNode.childNodes.length + "] children and text content of [" + aParentNode.textContent + "].\n";
      msiKludgeLogString(logStr, ["spaces"]);
    }
    editor.endTransaction();
    return;
  }

  //If we get here, bIsText is false, so we want an <hspace> object. Do we already have one? Otherwise create one?
  var ourNode = null;
  var bNeedInsert = false;
  var dimsStr = msiSpaceUtils.getHSpaceDims(dialogData.spaceType);

  if (bWasText)
  {
    ourNode = editor.document.createElementNS(xhtmlns, "hspace");
    bNeedInsert = true;
  }
  else
    ourNode = currentNode;

  if ( (dialogData.spaceType != spaceInfo.theSpace) || (dialogData.spaceType == "customSpace") )
  {
    msiKludgeLogString("Inside the spaceType different clause.\n", ["spaces"]);
    if (dialogData.spaceType != spaceInfo.theSpace)
      editor.setAttribute(ourNode, "type", dialogData.spaceType);
    if (dimsStr && dimsStr.length)
      editor.setAttribute(ourNode, "dim", dimsStr);
    else if (dialogData.spaceType != "customSpace")
//    if (dialogData.spaceType != "customSpace")
      editor.removeAttribute(ourNode, "dim");
    if (contentStr != msiSpaceUtils.getHSpaceDisplayableContent(spaceInfo.theSpace))
    {
      msiKludgeLogString("Inside the contentStr different clause.\n", ["spaces"]);
      for (var ix = ourNode.childNodes.length-1; ix >= 0; --ix)  //Note that the anonymous generated (XBL) content shouldn't be affected by this
        editor.deleteNode(ourNode.childNodes[ix]);
      if (cntNodeList)
      {
//        parser = new DOMParser();
//        var cntDoc = parser.parseFromString(contentStr,"application/xhtml+xml");
//        var cntNodeList = cntDoc.documentElement.childNodes;
        for (ix = 0; ix < cntNodeList.length; ++ix)
          editor.insertNode(cntNodeList[ix], ourNode, ix);
      }
    }
    if (dialogData.spaceType == "customSpace")  //more to do
    {
      var minWidthExpr = /min-width:\s*[^;]+;?/;
      var atEndAttr = (dialogData.customSpaceData.typesetChoice=="always") ? "true" : "false";
      if (!("atEnd" in spaceInfo) || (spaceInfo.atEnd != atEndAttr))
        editor.setAttribute(ourNode, "atEnd", atEndAttr);
      msiKludgeLogString("Inside the custom spaceType clause; customType is [" + dialogData.customSpaceData.customType + "].\n", ["spaces"]);

      if (dialogData.customSpaceData.customType == "fixed")
      {
        dimsStr = String(dialogData.customSpaceData.fixedData.size) + dialogData.customSpaceData.fixedData.units;
        msiKludgeLogString("Inside the customType fixed clause, dimsStr is [" + dimsStr + "].\n", ["spaces"]);
        if (ourNode.getAttribute("dim") != dimsStr)
        {
          var styleStr = ourNode.getAttribute("style");
          if (!styleStr)
            styleStr = "";
          if (styleStr.match(minWidthExpr))
            styleStr = styleStr.replace(minWidthExpr, "min-width: " + dimsStr + ";");
          else
            styleStr += "min-width: " + dimsStr + ";";
          editor.setAttribute(ourNode, "dim", dimsStr);
          editor.setAttribute(ourNode, "style", styleStr);
        }
        if (ourNode.getAttribute("class") == "stretchySpace")
          editor.removeAttribute(ourNode, "class");
        if ( ("customType" in spaceInfo) && (spaceInfo.customType != "fixed") )
        {
          if (ourNode.getAttribute("flex").length)
            editor.removeAttribute(ourNode, "flex");
          if (ourNode.getAttribute("fillWith").length)
            editor.removeAttribute(ourNode, "fillWith");
        }
      }
      else // if (dialogData.customSpaceData.customType == "stretchy")
      {
        if (ourNode.getAttribute("class") != "stretchySpace")
          editor.setAttribute(ourNode, "class", "stretchySpace");
        editor.removeAttribute(ourNode, "dim");
        if ( !("stretchFactor" in spaceInfo) || (Number(spaceInfo.stretchFactor) != dialogData.customSpaceData.stretchData.factor) )
          editor.setAttribute(ourNode, "flex", String(dialogData.customSpaceData.stretchData.factor));
        var fillAttr = "";
        if (dialogData.customSpaceData.stretchData.fillWith && dialogData.customSpaceData.stretchData.fillWith.length)
        {
          switch(dialogData.customSpaceData.stretchData.fillWith)
          {
            case "fillDots":             fillAttr = "dots";              break;
            case "fillLine":             fillAttr = "line";              break;
            default:                                                     break;
          }
        }
        if (fillAttr.length)
        {
          if ( !("fillWith" in spaceInfo) || (spaceInfo.fillWith != fillAttr) )
            editor.setAttribute(ourNode, "fillWith", fillAttr);
        }
        else if (ourNode.getAttribute("fillWith").length)
          editor.removeAttribute(ourNode, "fillWith");
      }

    }
  }

  logStr = "After the spaceType different clause; bNeedInsert is [";
  if (bNeedInsert)
    logStr += "true].\n";
  else
    logStr += "false].\n";
  msiKludgeLogString(logStr, ["spaces"]);

  if (bNeedInsert) //only if bWasText is true
  {
    aParentNode = reviseData.getReferenceNode();
    msiEditorReplaceTextWithNode(editor, aParentNode, reviseData.mOffset, reviseData.mOffset + reviseData.getTextLength(), ourNode);
  }

  editor.endTransaction();
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
    try {
      var editorElement = msiGetActiveEditorElement();
      var vSpaceData = new Object();
      vSpaceData.spaceType = "smallSkip";
      msiOpenModelessDialog("chrome://prince/content/VerticalSpaces.xul", "_blank", "chrome,close,titlebar,dependent,resizable",
                                          editorElement, "cmd_insertVerticalSpaces", this, vSpaceData);
    }
    catch (e) {
      finalThrow(cmdFailString('insertvertspace'), e.message);
    }
  }
};

var msiReviseVerticalSpacesCommand =
{
  isCommandEnabled: function(aCommand, dummy)  {return true;},
  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon)
  {
    try
    {
      var editorElement = msiGetActiveEditorElement();
      var vSpaceReviseData = msiGetPropertiesDataFromCommandParams(aParams);
      var vSpaceData = new Object();
      vSpaceData.reviseData = vSpaceReviseData;
      if (vSpaceReviseData != null && editorElement != null)
      {
  //      AlertWithTitle("msiComposerCommands.js", "In msiReviseVerticalSpacesCommand, trying to revise a vertical space, dialog not yet implemented.");
        var dlgWindow = msiDoModelessPropertiesDialog("chrome://prince/content/VerticalSpaces.xul", "_blank", "chrome,close,titlebar,dependent,resizable",
                                                       editorElement, "cmd_reviseVerticalSpaces", this, vSpaceData);
      }
      editorElement.focus();
    }
    catch (e) {
      finalThrow(cmdFailString('insertvertspace'), e.message);
    }
  },

  doCommand: function(aCommand, dummy)  {}
};

function msiInsertVerticalSpace(dialogData, editorElement)
{
  var editor = msiGetEditor(editorElement);
  var styleStr;
  var dimStr;
  var vAlignStr;
  var node = editor.document.createElement('vspace',true);
  if (dialogData.spaceType != "customSpace")
  {
    node.setAttribute('type',dialogData.spaceType);
    dimStr = msiSpaceUtils.getVSpaceDims(dialogData.spaceType);
    var lineHtStr = null;
    if (dimStr)
    {
      node.setAttribute('dim',dimStr);
    }
    else
    {
      lineHtStr = msiSpaceUtils.getVSpaceLineHeight(dialogData.spaceType);
      if (lineHtStr)
      {
        node.setAttribute('lineHt',lineHtStr);
      }
    }
  }
  else
  {
    node.setAttribute('type','customSpace');
    dimStr = String(dialogData.customSpaceData.sizeData.size) + dialogData.customSpaceData.sizeData.units;
    node.setAttribute('dim', dimStr);
    vAlignStr = String(-(dialogData.customSpaceData.sizeData.size)) + dialogData.customSpaceData.sizeData.units;
    node.setAttribute('atEnd',(dialogData.customSpaceData.typesetChoice=='always' ? 'true': 'false'));
    node.setAttribute('style','height: ' + dimStr + '; vertical-align: ' + vAlignStr + ';');
  }
  var contentStr = msiSpaceUtils.getVSpaceDisplayableContent(dialogData.spaceType);
  if (contentStr)
    node.textContent =contentStr;
  editor.insertElementAtSelection(node,true);
}

function msiReviseVerticalSpace(reviseData, dialogData, editorElement)
{
  var editor = msiGetEditor(editorElement);
  editor.beginTransaction();
  var spaceInfo = reviseData.getSpaceInfo();
  var contentStr = msiSpaceUtils.getVSpaceDisplayableContent(dialogData.spaceType);
//  var invisContent = msiSpaceUtils.getHSpaceShowInvis(dialogData.spaceType);
  var cntNodeList = null;
  var logStr = "";
  if (contentStr && contentStr.length)
  {
    parser = new DOMParser();
    contentStr = "<body>" + contentStr + "</body>";
    msiKludgeLogString("In msiComposerCommands.js, in msiReviseVerticalSpace, retrieving content string and got [" + contentStr + "].\n", ["spaces"]);
    var cntDoc = parser.parseFromString(contentStr,"application/xhtml+xml");
    cntNodeList = cntDoc.documentElement.childNodes;
  }

  var currentNode = reviseData.getReferenceNode();
  var aParentNode = currentNode.parentNode;;
  var ourNode = currentNode;
  var dimsStr = msiSpaceUtils.getVSpaceDims(dialogData.spaceType);
  var lineHtStr = msiSpaceUtils.getVSpaceLineHeight(dialogData.spaceType);

  if ( (dialogData.spaceType != spaceInfo.theSpace) || (dialogData.spaceType == "customSpace") )
  {
    msiKludgeLogString("Inside the vertical spaceType different clause.\n", ["spaces"]);
    if (dialogData.spaceType != spaceInfo.theSpace)
      editor.setAttribute(ourNode, "type", dialogData.spaceType);
    if (dimsStr && dimsStr.length)
      editor.setAttribute(ourNode, "dim", dimsStr);
    else if (dialogData.spaceType != "customSpace")
//    if (dialogData.spaceType != "customSpace")
      editor.removeAttribute(ourNode, "dim");

    var styleStr = ourNode.getAttribute("style");
    if (!styleStr)
      styleStr = "";
    var heightExpr = /height:\s*[^;]+;?/;
    var vertAlignExpr = /vertical-align:\s*[^;]+;?/;

    if (contentStr != msiSpaceUtils.getVSpaceDisplayableContent(spaceInfo.theSpace))
    {
      msiKludgeLogString("Inside the contentStr different clause.\n", ["spaces"]);
      for (var ix = ourNode.childNodes.length-1; ix >= 0; --ix)  //Note that the anonymous generated (XBL) content shouldn't be affected by this
        editor.deleteNode(ourNode.childNodes[ix]);
      if (cntNodeList)
      {
//        parser = new DOMParser();
//        var cntDoc = parser.parseFromString(contentStr,"application/xhtml+xml");
//        var cntNodeList = cntDoc.documentElement.childNodes;
        for (ix = 0; ix < cntNodeList.length; ++ix)
          editor.insertNode(cntNodeList[ix], ourNode, ix);
      }
    }
    if (dialogData.spaceType == "customSpace")  //more to do
    {
      var atEndAttr = (dialogData.customSpaceData.typesetChoice=="always") ? "true" : "false";
      if (!("atEnd" in spaceInfo) || (spaceInfo.atEnd != atEndAttr))
        editor.setAttribute(ourNode, "atEnd", atEndAttr);
      msiKludgeLogString("Inside the custom vertical spaceType clause.\n", ["spaces"]);

      if ( (dialogData.customSpaceData.customType != null) && (dialogData.customSpaceData.customType != "fixed") )
        dump("Problem in msiComposerCommands.js, msiReviseVerticalSpace; dialog reported a stretchy vertical space?\n");
      //Then go ahead and pretend it's a fixed custom space.
      dimsStr = String(dialogData.customSpaceData.sizeData.size) + dialogData.customSpaceData.sizeData.units;
      msiKludgeLogString("Inside the customType fixed clause, dimsStr is [" + dimsStr + "].\n", ["spaces"]);
      if (ourNode.getAttribute("dim") != dimsStr)
      {
        if (styleStr.match(heightExpr))
          styleStr = styleStr.replace(heightExpr, "height: " + dimsStr + ";");
        else
          styleStr += "height: " + dimsStr + ";";
        if (dimsStr[0] == "-")
          dimsStr = dimsStr.substr(1);
        else
          dimsStr = "-" + dimsStr;
        if (styleStr.match(vertAlignExpr))
          styleStr = styleStr.replace(vertAlignExpr, "vertical-align: " + dimsStr + ";");
        else
          styleStr += "vertical-align: " + dimsStr + ";";
        editor.setAttribute(ourNode, "dim", dimsStr);
        editor.setAttribute(ourNode, "style", styleStr);
      }
      if (ourNode.getAttribute("class") == "stretchySpace")
        editor.removeAttribute(ourNode, "class");
    }
    else if (spaceInfo.theSpace == "customSpace" && styleStr.length)
    {
      if (styleStr.match(heightExpr))
        styleStr = styleStr.replace(heightExpr, "");
      if (styleStr.match(vertAlignExpr))
        styleStr = styleStr.replace(vertAlignExpr, "");
      styleStr = TrimString(styleStr);
      if (styleStr.length == 0)
        editor.removeAttribute(ourNode, "style");
      else
        editor.setAttribute(ourNode, "style", styleStr);
    }
  }

  editor.endTransaction();
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
    try {
      var editorElement = msiGetActiveEditorElement();
      var rulesData = new Object();
      msiOpenModelessDialog("chrome://prince/content/msiRulesDialog.xul", "_blank", "chrome,close,titlebar,dependent,resizable",
        editorElement, "cmd_msiInsertRules", this, rulesData);
    }
    catch (e) {
      finalThrow(cmdFailString('insertrules'), e.message);
    }
  }
};

var msiReviseRulesCommand =
{
  isCommandEnabled: function(aCommand, dummy)  {return true;},
  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon)
  {
    try {
      var editorElement = msiGetActiveEditorElement();
      var ruleReviseData = msiGetPropertiesDataFromCommandParams(aParams);
      var ruleData = new Object();
      ruleData.reviseData = ruleReviseData;
      if (ruleReviseData != null && editorElement != null)
      {
  //      AlertWithTitle("msiComposerCommands.js", "In msiReviseRulesCommand, trying to revise a rule, dialog not yet implemented.");
        var dlgWindow = msiDoModelessPropertiesDialog("chrome://prince/content/msiRulesDialog.xul", "_blank", "chrome,close,titlebar,dependent,resizable",
                                                       editorElement, "cmd_msiReviseRules", this, ruleData);
      }
      editorElement.focus();
    }
    catch (e) {
      finalThrow(cmdFailString('reviserules'), e.message);
    }
  },

  doCommand: function(aCommand, dummy)  {}
};

function msiInsertRules(dialogData, editorElement)
{
	var unitHandler = new UnitHandler();
	unitHandler.initCurrentUnit(dialogData.height.units);
  var editor = msiGetEditor(editorElement);
  var node = editor.document.createElement('msirule');
  var styleStr = "";
  var colorStr;
  var liftStr = String(dialogData.lift.size) + dialogData.lift.units;
  var widthStr = String(dialogData.width.size) + dialogData.width.units;
  var heightStr = String(dialogData.height.size) + dialogData.height.units;
  node.setAttribute('lift', liftStr);
  node.setAttribute("req", "xcolor");
  styleStr += "vertical-align: " + unitHandler.getValueAs(dialogData.lift.size,"px")+"px";
  node.setAttribute("width", widthStr);
  styleStr += "; width: " + Math.max(1,unitHandler.getValueAs(dialogData.width.size,"px"))+"px";
  node.setAttribute("height", heightStr);
  styleStr += "; height: " + Math.max(1,unitHandler.getValueAs(dialogData.height.size,"px"))+"px";
  colorStr = dialogData.ruleColor;
  if (colorStr.indexOf("rgb") == 0) colorStr = ConvertRGBColorIntoHEXColor(color);
  else if (colorStr.indexOf("#") == -1) colorStr = textColorToHex(colorStr);
  node.setAttribute("color",colorStr);
  styleStr += "; background-color: " + colorStr + ";";
  node.setAttribute('style',styleStr);
  editor.insertElementAtSelection(node,true);
}

function msiReviseRules(reviseData, dialogData, editorElement)
{
  var editor = msiGetEditor(editorElement);
  try {
    editor.beginTransaction();
//  var parentNode = editor.selection.anchorNode;
//  var insertPos = editor.selection.anchorOffset;

    var currentNode = reviseData.getReferenceNode();
    var aParentNode = currentNode.parentNode;;
    var ourNode = currentNode;

    var vAlignExpr = /vertical-align:\s*[^;]+;?/;
    var htExpr = /height:\s*[^;]+;?/;
    var wdthExpr = /width:\s*[^;]+;?/;
    var bkExpr = /background\-color:\s*[^;]+;?/;
  //      msiKludgeLogString("Inside the custom newline breakType clause; customType is [" + dialogData.customBreakData.customType + "].\n", ["spaces"]);

    var styleStr = ourNode.getAttribute("style");
    var liftStr = String(dialogData.lift.size) + dialogData.lift.units;
    var widthStr = String(dialogData.width.size) + dialogData.width.units;
    var heightStr = String(dialogData.height.size) + dialogData.height.units;
    var bSetStyle = false;
    var colorStr;
  //      msiKludgeLogString("Inside the custom NewLine clause, dimsStr is [" + dimsStr + "].\n", ["spaces"]);
  editor.setAttribute(ourNode, "req", "xcolor");
  if (ourNode.getAttribute("lift") != liftStr)
    {
      editor.setAttribute(ourNode, "lift", liftStr);
      if (!styleStr)
        styleStr = "";
      if (styleStr.match(vAlignExpr))
        styleStr = styleStr.replace(vAlignExpr, "vertical-align: " + liftStr + ";");
      else
        styleStr += "vertical-align: " + liftStr + ";";
      bSetStyle = true;
    }
    if (ourNode.getAttribute("width") != widthStr)
    {
      editor.setAttribute(ourNode, "width", widthStr);
      if (!styleStr)
        styleStr = "";
      if (styleStr.match(wdthExpr))
        styleStr = styleStr.replace(wdthExpr, "width: " + widthStr + ";");
      else
        styleStr += "width: " + widthStr + ";";
      bSetStyle = true;
    }
    if (ourNode.getAttribute("height") != heightStr)
    {
      editor.setAttribute(ourNode, "height", heightStr);
      if (styleStr.match(htExpr))
        styleStr = styleStr.replace(htExpr, "height: " + heightStr + ";");
      else
        styleStr += "height: " + heightStr + ";";
      bSetStyle = true;
    }
    colorStr=dialogData.ruleColor;
    if (colorStr.indexOf("rgb") == 0) colorStr = ConvertRGBColorIntoHEXColor(color);
    else if (colorStr.indexOf("#") == -1) colorStr = textColorToHex(colorStr);
    if (ourNode.getAttribute("color") != colorStr)
    {
      editor.setAttribute(ourNode, "color", colorStr);
      if (styleStr.match(bkExpr))
        styleStr = styleStr.replace(bkExpr, "background-color: " + colorStr + ";");
      else
        styleStr += "background-color: " + colorStr + ";";
      bSetStyle = true;
    }
    if (bSetStyle)
      editor.setAttribute(ourNode, "style", styleStr);

    editor.endTransaction();
  }
  catch(e)
  {
    msidump(e.message);
  }
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
    try {
      var editorElement = msiGetActiveEditorElement();
      var breaksData = new Object();
      msiOpenModelessDialog("chrome://prince/content/msiBreaksDialog.xul", "_blank", "chrome,close,titlebar,resizable,dependent",
                                          editorElement, "cmd_msiInsertBreaks", this, breaksData);
    }
    catch (e) {
      finalThrow(cmdFailString('insertbreaks'), e.message);
    }
  }
};

var msiReviseBreaksCommand =
{
  isCommandEnabled: function(aCommand, dummy)  {return true;},
  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon)
  {
    try
    {
      var editorElement = msiGetActiveEditorElement();
      var breakReviseData = msiGetPropertiesDataFromCommandParams(aParams);
      var breakData = new Object();
      breakData.reviseData = breakReviseData;
      if (breakReviseData != null && editorElement != null)
      {
        var dlgWindow = msiDoModelessPropertiesDialog("chrome://prince/content/msiBreaksDialog.xul", "_blank", "chrome,close,titlebar,dependent,resizable",
                                                       editorElement, "cmd_msiReviseBreaks", this, breakData);
  //      AlertWithTitle("msiComposerCommands.js", "In msiReviseBreaksCommand, trying to revise a break, dialog not yet implemented.");
      }
      editorElement.focus();
    }
    catch (e) {
      finalThrow(cmdFailString('revisebreaks'), e.message);
    }
  },

  doCommand: function(aCommand, dummy)  {}
};

function msiInsertBreaks(dialogData, editorElement)
{
  var editor = msiGetEditor(editorElement);
//  var parentNode = editor.selection.anchorNode;
//  var insertPos = editor.selection.anchorOffset;
//  var contentFromBreakType =
//  {
//    allowBreak:             "&#x200b;",  //this is the zero-width space
//    discretionaryHyphen:    "&#x00ad;",
//    noBreak:                "&#x2060;",
//    pageBreak:              "&#x000c;",  //formfeed?
//    newPage:                "&#x000c;",  //formfeed?
//    lineBreak:              "<br xmlns=\"" + xhtmlns + "\"></br>",
//    newLine:                "<br xmlns=\"" + xhtmlns + "\"></br>"
//  };
//  var alternateContentFromBreakType =
//
//  {
//    allowBreak:             "|",
//    discretionaryHyphen:    "-",
//    noBreak:                "~",
//    pageBreak:              "&#x21b5;",
//    newPage:                "&#x21b5;",
//    lineBreak:              "&#x21b5;",
//    newLine:                "&#x21b5;"
//  };


  var contentStr;
	editor.beginTransaction();
  var node = editor.document.createElement('msibr',true);
  editor.insertElementAtSelection(node,true);
  switch(dialogData.breakType) {
    case "lineBreak":
    case "newLine":
      editor.createNode('br',node,0);
      break;
    case "newPage":
    case "pageBreak":
      editor.createNode('newPageRule',node,0);
      break;
    default:
      contentStr = msiSpaceUtils.getBreakCharContent(dialogData.breakType);
      break;
  }
  //  var breakStr = "<xhtml:msibreak xmlns:xhtml=\"" + xhtmlns + "\" type=\"";
  if (dialogData.breakType == "customNewLine")
  {
    node.setAttribute('type','customNewLine');
    var dimStr=String(dialogData.customBreakData.sizeData.size) + dialogData.customBreakData.sizeData.units;
    node.setAttribute('dim', dimStr);
    var vAlignStr = String(-(dialogData.customBreakData.sizeData.size)) + dialogData.customBreakData.sizeData.units;
    node.setAttribute('style','vertical-align: '+vAlignStr+'; height: '+ dimStr);
    if (!contentStr)
      contentStr = "";
    node.textContent = contentStr;
  }
  else
  {
    node.setAttribute('type',dialogData.breakType);
  }
  var invisStr = msiSpaceUtils.getBreakShowInvis(dialogData.breakType);
  if (invisStr)
    node.setAttribute('invisDisplay',invisStr);
  if (contentStr && contentStr.length > 0) {

    // This code is a horrendous hack caused by the fact that assigning "&#x200b;" to the text content results in "&amp;#x200b"
    parser = new DOMParser();
    contentStr = "<body>" + contentStr + "</body>";
    msiKludgeLogString("In msiComposerCommands.js, in msiReviseBreaks, retrieving content string and got [" + contentStr + "].\n", ["spaces"]);
    var cntDoc = parser.parseFromString(contentStr,"application/xhtml+xml");
    cntNodeList = cntDoc.documentElement.childNodes;
    node.textContent = cntNodeList[0].textContent

    //    node.textContent = contentStr;  //This was the original code. and
    //var newContent = document.createTextNode(contentStr);
    //node.appendChild(newContent);
  }
  editor.endTransaction();
}

function msiReviseBreaks(reviseData, dialogData, editorElement)
{
  var editor = msiGetEditor(editorElement);
  editor.beginTransaction();
  var breakInfo = reviseData.getSpaceInfo();

  var contentStr = msiSpaceUtils.getBreakDisplayableContent(dialogData.breakType);
//  var invisContent = msiSpaceUtils.getHSpaceShowInvis(dialogData.spaceType);
  var cntNodeList = null;
  var logStr = "";
  if (contentStr && contentStr.length)
  {
    parser = new DOMParser();
    contentStr = "<body>" + contentStr + "</body>";
    msiKludgeLogString("In msiComposerCommands.js, in msiReviseBreaks, retrieving content string and got [" + contentStr + "].\n", ["spaces"]);
    var cntDoc = parser.parseFromString(contentStr,"application/xhtml+xml");
    cntNodeList = cntDoc.documentElement.childNodes;
  }

  var currentNode = reviseData.getReferenceNode();
  var aParentNode = currentNode.parentNode;;
  var ourNode = currentNode;
//  var lineHtStr = msiSpaceUtils.getVSpaceLineHeight(dialogData.spaceType);

  if ( (dialogData.breakType != breakInfo.theSpace) || (dialogData.breakType == "customNewLine") )
  {
    msiKludgeLogString("Inside the breakType different clause.\n", ["spaces"]);
    if (dialogData.breakType != breakInfo.theSpace)
      editor.setAttribute(ourNode, "type", dialogData.breakType);
//    if (dimsStr && dimsStr.length)
//      editor.setAttribute(ourNode, "dim", dimsStr);
//    else if (dialogData.spaceType != "customSpace")
////    if (dialogData.spaceType != "customSpace")
//      editor.removeAttribute(ourNode, "dim");
    if (contentStr != msiSpaceUtils.getBreakDisplayableContent(breakInfo.theSpace))
    {
      msiKludgeLogString("Inside the contentStr different clause.\n", ["spaces"]);
      for (var ix = ourNode.childNodes.length-1; ix >= 0; --ix)  //Note that the anonymous generated (XBL) content shouldn't be affected by this
        editor.deleteNode(ourNode.childNodes[ix]);
      if (cntNodeList)
      {
//        parser = new DOMParser();
//        var cntDoc = parser.parseFromString(contentStr,"application/xhtml+xml");
//        var cntNodeList = cntDoc.documentElement.childNodes;
        for (ix = 0; ix < cntNodeList.length; ++ix)
          editor.insertNode(cntNodeList[ix], ourNode, ix);
      }
    }
    if (dialogData.breakType == "customNewLine")  //more to do
    {
      var interiorNode = null;
      var intNodes = ourNode.getElementsByTagNameNS(xhtmlns, "custNL");
      if (intNodes && intNodes.length)
        interiorNode = intNodes[0];
      else
      {
        interiorNode = editor.document.createElementNS(xhtmlns, "custNL");
        editor.insertNode(interiorNode, ourNode, 0);
      }
      var vertAlignExpr = /vertical-align:\s*[^;]+;?/;
      var htExpr = /height:\s*[^;]+;?/;
//      msiKludgeLogString("Inside the custom newline breakType clause; customType is [" + dialogData.customBreakData.customType + "].\n", ["spaces"]);

      //Then go ahead and pretend it's a fixed custom space.
      var dimsStr = String(dialogData.customBreakData.sizeData.size) + dialogData.customBreakData.sizeData.units;
      var vAlignStr = String(-(dialogData.customBreakData.sizeData.size)) + dialogData.customBreakData.sizeData.units;
      msiKludgeLogString("Inside the custom NewLine clause, dimsStr is [" + dimsStr + "].\n", ["spaces"]);
      if (ourNode.getAttribute("dim") != dimsStr)
      {
        editor.setAttribute(ourNode, "dim", dimsStr);
        var styleStr = interiorNode.getAttribute("style");
        if (!styleStr)
          styleStr = "";
        if (styleStr.match(vertAlignExpr))
          styleStr = styleStr.replace(vertAlignExpr, "vertical-align: " + vAlignStr + ";");
        else
          styleStr += "vertical-align: " + vAlignStr + ";";
        if (styleStr.match(htExpr))
          styleStr = styleStr.replace(htExpr, "height: " + dimsStr + ";");
        else
          styleStr += "height: " + dimsStr + ";";
        editor.setAttribute(interiorNode, "style", styleStr);
      }
    }
  }

  editor.endTransaction();
}

function setAlignmentOK(editorElement) {
  var editor = msiGetEditor(editorElement); 
  var selection = editor.selection;
  var isMath;
  var selNode;
  if (selection.isCollapsed) {
    selNode = selection.anchorNode;
    ismath = msiNavigationUtils.isMathNode(selNode);
    if (!ismath) selNode = selNode.parentNode;
    ismath = msiNavigationUtils.isMathNode(selNode);
    while (selNode && ismath && selNode.tagName != 'mtd') {
      if (selNode.tagName != 'mrow' &&
        selNode.tagName != 'mi' &&
        selNode.tagName != 'mo' &&
        selNode.tagName != 'mstyle' &&
        selNode.tagName != 'mphantom') {
        return false;
      }
      selNode = selNode.parentNode;
      ismath = msiNavigationUtils.isMathNode(selNode);
    }
    if (selNode && selNode.tagName == 'mtd') return true;
  }
  return false;
}

function positionInParent(aNode)
{
  if (aNode.parentNode !== null)
  {
    for (var ix = 0; ix < aNode.parentNode.childNodes.length; ++ix)
    {
      if (aNode.parentNode.childNodes[ix] === aNode)
        return ix;
    }
  }
  return -1;
}

function insertAlignment(alignmentNode, editor) {
  var node = editor.selection.anchorNode;
  var offset = editor.selection.anchorOffset;
  var tempOffset;
  var nodeObj = {value: null};
  var offsetObj = {value: null};
  var currentAlignNodes;
  var currentAlignNode;
  var posInParent;
  var i;
  // find existing alignment marks. Remove them for 5.5 behavior
  var mtdNode = editor.getElementOrParentByTagName("mtd", node);
  if (mtdNode ) {
    currentAlignNodes = mtdNode.getElementsByTagName('maligngroup');
    if (currentAlignNodes.length > 0) {
      currentAlignNode = currentAlignNodes[0];
    }
  }
  while (node.nodeType == Node.TEXT_NODE || node.tagName == 'mi' || node.tagName == 'mo') {
    // these can't accept a node, so we go up to the left or the right.
    tempOffset = positionInParent(node);
    node = node.parentNode;
    if (offset > 0) {
      offset = tempOffset+ 1; // point past the node
    } else {
      offset = tempOffset;
    }
  } 
  editor.beginTransaction();
  editor.insertNode(alignmentNode, node, offset);
  if (currentAlignNode) {
    editor.deleteNode(currentAlignNode);
  }
  editor.selection.collapse(node, offset+1);
  editor.endTransaction();
}

var msiSetAlignmentCommand =
{
    isCommandEnabled: function(aCommand, dummy)
    {
      var editorElement = msiGetActiveEditorElement();
      return setAlignmentOK(editorElement);
    },

    getCommandStateParams: function(aCommand, aParams, aRefCon) {},
    doCommandParams: function(aCommand, aParams, aRefCon) {},

    doCommand: function(aCommand, dummy)
    {
      try{
        var editorElement = msiGetActiveEditorElement();
        var editor = msiGetEditor(editorElement);
        var alignmentNode;
        if (setAlignmentOK(editorElement)) {
          alignmentNode = editor.document.createElementNS(mmlns, "maligngroup");
          insertAlignment(alignmentNode, editor);  
        }
      }
      catch (e) {
        finalThrow(cmdFailString('set alignment'), e.message);
      }
    }
};

//----------------------------------------------------
var msiMarkerCommand =
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
		try{
      var editorElement = msiGetActiveEditorElement();
      // more goes here
      window.openDialog("chrome://prince/content/marker.xul", "Insert marker", "resizable=yes,dependent=yes,chrome,close,titlebar");
  		msiGetEditor(editorElement).incrementModificationCount(1);
		}
    catch (e) {
      finalThrow(cmdFailString('reviserules'), e.message);
    }
  }
};
//----------------------------------------------------
var msiInsertHTMLFieldCommand =
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
      // more goes here
      window.openDialog("chrome://prince/content/htmlfield.xul", "HTML field", "resizable=yes,chrome,close,titlebar,dependent");
			msiGetEditor(editorElement).incrementModificationCount(1);
    } catch (e) {}
  }
};
//----------------------------------------------------
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
    try {
      var editorElement = msiGetActiveEditorElement();
      msiGetEditor(editorElement).insertReturnFancy();
    }
    catch (e) {
      finalThrow(cmdFailString('insertreturnfancy'), e.message);
    }
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
    }
    catch (e) {
      finalThrow(cmdFailString('insertsubstructure'), e.message);
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
    try {
      var macrofragmentStatusPanel = document.getElementById('macroEntryPanel');
      if (macrofragmentStatusPanel)
      {
        macrofragmentStatusPanel.setAttribute("hidden", "false");
        document.getElementById('macroAndFragments').focus();
      }
    }
    catch (e) {
      finalThrow(cmdFailString('macrofragment'), e.message);
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
    window.openDialog("chrome://editor/content/EdSnapToGrid.xul","snaptogrid", "chrome,close,titlebar,modal");
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
  doCommandParams: function(aCommand, aParams, aRefCon)
  {
    var editorElement = msiGetActiveEditorElement();
    var listNode = msiGetReviseObjectFromCommandParams(aParams);
    if (listNode != null && editorElement != null)
    {
      AlertWithTitle("msiComposerCommands.js", "In msiListPropertiesCommand, trying to revise list items, dialog not yet implemented.");
//      var dlgWindow = msiDoModelessPropertiesDialog("chrome://editor/content/EdListProps.xul", "_blank", "chrome,close,titlebar,dependent",
//                                                     editorElement, "cmd_listProperties", listNode);
    }
    editorElement.focus();
  },

  doCommand: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();
    window.openDialog("chrome://editor/content/EdListProps.xul","listprops", "chrome,close,titlebar,modal,resizable");
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
    window.openDialog("chrome://editor/content/EdPageProps.xul","pageprops", "chrome,close,titlebar,modal", "");

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
    try
    {
      // Launch Document Info dialog
      var editorElement = msiGetTopLevelEditorElement();
      var documentInfo = new msiDocumentInfo(editorElement);
      documentInfo.initializeDocInfo();
      var dlgInfo = documentInfo.getDialogInfo();
      msiOpenModelessDialog("chrome://prince/content/DocumentInfo.xul", "_blank", "chrome,close,titlebar,dependent,resizable",
                                          editorElement, "cmd_documentInfo", this, dlgInfo);
      editorElement.contentWindow.focus();
    }
    catch (e) {
      finalThrow(cmdFailString('documentinfo'), e.message);
    }
  }
};


function msiFinishDocumentInfoDialog(editorElement, dlgInfo)
{
  if (!dlgInfo.cancel)
  {
    var docInfo = dlgInfo.parentData;
    docInfo.resetFromDialogInfo(dlgInfo);
    docInfo.putDocInfoToDocument();
    msiUpdateWindowTitle();
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
    switch(theName)
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
    this.generalSettings = new Object();
    this.comments = new Object();
    this.printSettings = new Object();
    this.metadata = new Object();
    this.saveSettings = new Object();

    var docHead = msiGetPreamble(this.mEditor);
    if (!docHead)
      return;

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
            titleObj.contents = currNode.textContent;
//            for (var ix = 0; ix < currNode.childNodes.length; ++ix)
//            {
//              if (currNode.childNodes[ix].nodeName == "#text")
//                titleObj.contents += currNode.childNodes[ix].nodeValue;
//            }
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
    var docHead = msiGetPreamble(this.mEditor);
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
              switch(commentData.name.toLowerCase())
              {
                case "lastrevised":
                case "documentshell":
                case "language":
                case "bibliographyscheme":
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
        newNode.setAttribute("req", "hyperref");
      }
      break;
      case "meta":
        newNode = this.mEditor.document.createElement("meta");
//        newNode.setAttribute("name", dataObj.name);
        newNode.setAttribute("name", ourName);
        newNode.setAttribute("content", dataObj.contents);
        newNode.setAttribute("req", "hyperref");
      break;
      case "link":
        newNode = this.mEditor.document.createElement("link");
//        newNode.setAttribute("rel", dataObj.name);
        newNode.setAttribute("rel", ourName);
        newNode.setAttribute("href", dataObj.uri);
        newNode.setAttribute("req", "hyperref");
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
//    theData = theData.toLowerCase();

//    var tcidataRegExp = /tcidata[\s]*\{((?:(?:\\\})|(?:[^\}]))+)\}/i;
//    var fullNameSyntax = /meta[\s]+.*name=\"((?:(?:\\\")|(?:[^\"]))+)\"/i;
//    var fullContentsSyntax = /meta[\s]+.*content=\"((?:(?:\\\")|(?:[^\"]))+)\"/i;
//    var altfullNameSyntax = /meta[\s]+.*name=\'((?:(?:\\\')|(?:[^\']))+)\'/i;
//    var altfullContentsSyntax = /meta[\s]+.*content=\'((?:(?:\\\')|(?:[^\']))+)\'/i;
//    var fullLinkSyntax = /link[\s]+.*rel=\"((?:(?:\\\")|(?:[^\"]))+)\"/i;
//    var fullLinkRefSyntax = /link[\s]+.*href=\"((?:(?:\\\")|(?:[^\"]))+)\"/i;
//    var altfullLinkSyntax = /link[\s]+.*rel=\'((?:(?:\\\')|(?:[^\']))+)\'/i;
//    var altfullLinkRefSyntax = /link[\s]+.*href=\'((?:(?:\\\')|(?:[^\']))+)\'/i;
//    var keyValueSyntax = /([\S]+)=(.*)/;
//    var keyValueValueSyntax = /?:([\S]+)=(.*)/;

    var tciData = theData.match(this.tcidataRegExp);
    //NOTE! In JavaScript String.match(regExp), the first thing returned is the full matching expression; capturing-parentheses
    //  matches are returned in subsequent array members. So we're after array[1] in each case...
    if (tciData && (tciData.length > 1))
    {
      var metaName = tciData[1].match(this.fullNameSyntax);
      if (!metaName || (metaName.length == 0))
        metaName = tciData[1].match(this.altfullNameSyntax);
      if (metaName && (metaName.length > 1))
      {
        var theType = "comment-meta";
        var contents = tciData[1].match(this.fullContentsSyntax);
        if (!contents || (contents.length == 0))
        {
          theType = "comment-meta-alt";
          contents = tciData[1].match(this.altfullContentsSyntax);
        }
        if (contents && (contents.length > 1))
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
        if (!linkName || (linkName.length == 0))
          linkName = tciData[1].match(this.altfullLinkSyntax);
        if (linkName && (linkName.length > 1))
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
//    var fullNameReplaceSyntax = /meta[\s]+.*name=\"((?:(?:\\\")|(?:[^\"]))+)\"/i;
    var fullContentsReplaceSyntax = /(meta[\s]+.*content=\")((?:(?:\\\")|(?:[^\"]))+)\"/i;
//    var altfullNameReplaceSyntax = /meta[\s]+.*name=\'((?:(?:\\\')|(?:[^\']))+)\'/i;
    var altfullContentsReplaceSyntax = /(meta[\s]+.*content=\')((?:(?:\\\')|(?:[^\']))+)\'/i;
//    var fullLinkReplaceSyntax = /link[\s]+.*rel=\"((?:(?:\\\")|(?:[^\"]))+)\"/i;
    var fullLinkRefReplaceSyntax = /(link[\s]+.*href=\")((?:(?:\\\")|(?:[^\"]))+)\"/i;
//    var altfullLinkReplaceSyntax = /link[\s]+.*rel=\'((?:(?:\\\')|(?:[^\']))+)\'/i;
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
    {
      theContents = dlgInfo.general.documentTitle;
    }
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
      dlgInfo.printOptions.zoomPercentage = msiGetCurrViewPercent(this.mEditorElement);  //in msiEditorUtilities.js, though not yet really implemented
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

    theContents = msiGetCurrViewPercent(this.mEditorElement);
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
    try
    {
      // Launch Document Style dialog
      var editorElement = msiGetTopLevelEditorElement();
      var documentStyle = {};
      documentStyle.edElement = editorElement;
      msiOpenModelessDialog("chrome://prince/content/DocumentStyle.xul", "_blank",
                                          "chrome,close,titlebar,dependent, resizable",
                                          editorElement, "cmd_documentInfo", this, documentStyle);
      editorElement.contentWindow.focus();
    }
    catch (e) {
      finalThrow(cmdFailString('documentstyle'), e.message);
    }
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
    try
    {
      var editorElement = msiGetActiveEditorElement();
      updateEditorFromViewMenu(editorElement);
    }
    catch (e) {
      finalThrow(cmdFailString('viewinvis'), e.message);
    }
  }
};

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
    try
    {
      var editorElement = msiGetActiveEditorElement();
      //temporary
      // need to get current note if it exists -- if none, initialize as follows
      msiNote(null, editorElement);
    }
    catch (e) {
      finalThrow(cmdFailString('note'), e.message);
    }
  }
};

var msiFootnoteCommand =
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
    try
    {
      var editorElement = msiGetActiveEditorElement();
      //temporary
      // need to get current note if it exists -- if none, initialize as follows
      msiNote(null, editorElement, 'footnote');
    }
    catch (e) {
      finalThrow(cmdFailString('footnote'), e.message);
    }
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
    try
    {
        var editorElement = msiGetActiveEditorElement();
        var bibChoice = getBibliographyScheme(editorElement);
        if (bibChoice == "bibtex")  //a kludge - must get hooked up to editor to really work
        {
          var bibCiteData = {databaseFile : "", key : "", remark : "", bBibEntryOnly : false};
          var dlgWindow = msiOpenModelessDialog("chrome://prince/content/typesetBibTeXCitation.xul", "_blank", "resizable=yes, chrome,close,titlebar,dependent",
                                                           editorElement, aCommand, this, bibCiteData);
        }
        else
        {
          var manualCiteData = {key : "", remark : ""};
          manualCiteData.keyList = new Array();
          var editor = msiGetEditor(editorElement);
      //    if (editor)
      //      manualCiteData.keyList = manualCiteData.keyList.concat(getEditorBibItemList(editor));

          var dlgWindow = msiOpenModelessDialog("chrome://prince/content/typesetManualCitation.xul", "_blank", "chrome,close,titlebar,dependent,resizable",
                                                                 editorElement, aCommand, this, manualCiteData);
        }
    }
    catch (e) {
      finalThrow(cmdFailString('citation'), e.message);
    }
  }
};

var msiReviseCitationCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();

    return (msiIsDocumentEditable(editorElement) && msiIsEditingRenderedHTML(editorElement));
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon)
  {
    try
    {
      var editorElement = msiGetActiveEditorElement();
      var citeReviseData = msiGetPropertiesDataFromCommandParams(aParams);
      var citeData = {key : "", remark : "", reviseData : citeReviseData};
      var citeNode = citeReviseData.getReferenceNode();
      if (citeNode.hasAttribute("type") && (citeNode.getAttribute("type") == "bibtex"))
      {
        var dlgWindow = msiOpenModelessDialog("chrome://prince/content/typesetBibTeXCitation.xul", "_blank", "chrome,close,titlebar,dependent,resizable",
                                                               editorElement, "cmd_reviseCitationCmd", this, citeData);
      }
      else
      {
        var dlgWindow = msiOpenModelessDialog("chrome://prince/content/typesetManualCitation.xul", "_blank", "chrome,close,titlebar,dependent,resizable",
                                                               editorElement, "cmd_reviseCitationCmd", this, citeData);
      }
      editorElement.focus();
    }
    catch (e) {
      finalThrow(cmdFailString('revisecitation'), e.message);
    }
  },

  doCommand: function(aCommand) {}
};

//-----------------------------------------------------------------------------------

var msiReviseCrossRefCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var editorElement = msiGetActiveEditorElement();

    return (msiIsDocumentEditable(editorElement) && msiIsEditingRenderedHTML(editorElement));
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon)
  {
    try
    {
      var editorElement = msiGetActiveEditorElement();
      var xrefReviseData = msiGetPropertiesDataFromCommandParams(aParams);
      var xrefData = {key : "", refType : "page", reviseData : xrefReviseData};
      var xrefNode = xrefReviseData.getReferenceNode();
      var dlgWindow = msiOpenModelessDialog("chrome://prince/content/xref.xul", "_blank", "chrome,close,titlebar,dependent,resizable",
                                                             editorElement, "cmd_reviseCrossRefCmd", this, xrefData);
      editorElement.focus();
    }
    catch (e) {
      finalThrow(cmdFailString('revisecrossref'), e.message);
    }
  },

  doCommand: function(aCommand) {}
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
    try
    {
      var editorElement = msiGetActiveEditorElement();
      //temporary
      // need to get current note if it exists -- if none, initialize as follows
      msiFrame(editorElement, null, null);
    }
    catch (e) {
      finalThrow(cmdFailString('frame'), e.message);
    }
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
      isEnabled = ( (nodeData != null) && nodeData.hasReviseData() );
//      var editor = msiGetEditor(editorElement);
//      isEnabled = ( (nodeData != null && nodeData.theNode != null) ||
//                    (editor != null && editor.getSelectedElement("href") != null) );
    }
    return isEnabled;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    try
  {
      // Launch Object properties for appropriate selected element
      var editorElement = msiGetActiveEditorElement();
      var nodeData = msiGetObjectDataForProperties(editorElement);
      if (!nodeData) return;
      var element = nodeData.theNode;
      var cmdString = nodeData.getCommandString(0);

      if (element)
      {
        var cmdParams = newCommandParams();
        if (!cmdParams)
        {
          dump("Trouble in msiObjectPropertiesCommand.doCommand! Can't create new CommandParams - aborting.\n");
          return;
        }
        if (element.nodeType == nsIDOMNode.TEXT_NODE)
        {
          if (nodeData.theOffset != null)
          {
            //Need to bring up the ReviseCharacter dialog. Not yet implemented.
  //          var theCharacter = element.data.charAt(nodeData.theOffset);
            cmdString = "cmd_reviseChars";
          }
          else
          {
            dump("No offset specified in text node for Properties dialog! Aborting.\n");
            return;
          }
        }

  //      var name = msiGetBaseNodeName(element).toLowerCase();
        cmdParams.setISupportsValue("reviseObject", element);
        msiSetCommandParamWeakRefValue(cmdParams, "propertiesData", nodeData);
  //      cmdParams.setISupportsValue("propertiesData", nodeData);
        if (cmdString)
          msiGoDoCommandParams(cmdString, cmdParams, editorElement);
        else
        {
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
            case 'object':
            case 'embed':
              if (element.getAttribute("isVideo") == "true")
                msiGoDoCommandParams("cmd_reviseVideo", cmdParams, editorElement);
              else
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
  //          case 'label':
  //            msiGoDoCommandParams("cmd_reviseLabel", cmdParams, editorElement);
  //          break;
            case 'fieldset':
              msiGoDoCommandParams("cmd_reviseFieldset", cmdParams, editorElement);
            break;

            case 'table':
              msiGoDoCommandParams("cmd_editTable", cmdParams, editorElement);
  //            msiEditorInsertOrEditTable(false, editorElement, "cmd_objectProperties", this, nodeData);
            break;
            case 'td':
            case 'th':
              msiGoDoCommandParams("cmd_editTableCell", cmdParams, editorElement);
  //            msiEditorTableCellProperties(editorElement);
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

            case 'msibr':
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

            case 'mtable':
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
        }
      } else {
        // We get a partially-selected link if asked for specifically
        try {
          element = msiGetEditor(editorElement).getSelectedElement("href");
        } catch (e) {}
        if (element)
        {
          var linkCmdParams = newCommandParams();
          if (!linkCmdParams)
          {
            dump("Trouble in msiObjectPropertiesCommand.doCommand! Can't create new CommandParams - aborting.\n");
            return;
          }

          linkCmdParams.setISupportsValue("reviseObject", element);
          var linkData = msiCreatePropertiesObjectDataFromNode(element, editorElement);
          msiSetCommandParamWeakRefValue(linkCmdParams, "propertiesData", linkData);
  //        linkCmdParams.setISupportsValue("propertiesData", linkData);
          msiGoDoCommandParams("cmd_msiReviseHyperlink", cmdParams, editorElement);
  //        msiGoDoReviseCommand("cmd_link", editorElement);
        }
      }
      editorElement.contentWindow.focus();
    }
    catch (e) {
      finalThrow(cmdFailString('objectproperties'), e.message);
    }
  },

  msiGetReviseObject: function(editorElement)
  {
    return msiEditorGetObjectForProperties(editorElement);
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
    var smileyCode = aParams.getStringValue("state_attribute");

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
        default:  strSml="";
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
          dlgParentWindow.openDialog("chrome://prince/content/texbuttoncontents.xul","texbutton","chrome,close,titlebar,resizable=yes,dependent");
					msiGetEditor(editorElement).incrementModificationCount(1);
          editorElement.contentWindow.focus();
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
            dlgParentWindow.openDialog("chrome://prince/content/latexpagenumberstyle.xul", "pagenumberstyle", "chrome,close,titlebar,modal,resizable=yes", data);
            editorElement.contentWindow.focus();
            if (!data.Cancel)
            {
              element.value = data.numstyle;
							msiGetEditor(editorElement).incrementModificationCount(1);
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
            dlgParentWindow.openDialog("chrome://prince/content/latexheaders.xul", "latexheaders", "chrome,close,titlebar,modal,resizable=yes", data);
            editorElement.contentWindow.focus();
            if (!data.Cancel)
            {
              element.value = data.lheader;
              element.value2 = data.rheader;
							msiGetEditor(editorElement).incrementModificationCount(1);
            }
          }
          catch (e)
          { dump(e); }
        }
      }
    }
    else
    {
      dlgParentWindow.openDialog("chrome://editor/content/EdAdvancedEdit.xul", "advedit", "chrome,close,titlebar,modal,resizable=yes", "", element);
      editorElement.contentWindow.focus();
    }
  }
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
    }
    catch (e) {
      finalThrow(cmdFailString('advancedprops'), e.message);
    }
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
      theWindow.openDialog("chrome://editor/content/EdColorProps.xul","colorprops", "chrome,close,resizable,titlebar,modal", "");
//      UpdateDefaultColors();
			msiGetEditor(editorElement).incrementModificationCount(1);
      msiUpdateDefaultColors(editorElement);
      editorElement.contentWindow.focus();
    }
    catch (e) {
      finalThrow(cmdFailString('colorprops'), e.message);
    }
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
    catch (e) {
      finalThrow(cmdFailString('removenamedanchor'), e.message);
    }
  }
};


////-----------------------------------------------------------------------------------
// var msiEditLinkCommand =
// {
//   isCommandEnabled: function(aCommand, dummy)
//   {
//     // Not really used -- this command is only in context menu, and we do enabling there
//     return (msiIsDocumentEditable() && msiIsEditingRenderedHTML());
//   },

//   getCommandStateParams: function(aCommand, aParams, aRefCon) {},
//   doCommandParams: function(aCommand, aParams, aRefCon) {},

//   doCommand: function(aCommand)
//   {
//     var editorElement = msiGetActiveEditorElement();
//     try
//     {
//       var element = msiGetEditor(editorElement).getSelectedElement("href");
//       if (element)
//         msiEditPage(msiURIFromString(element.href), window, false, false);
//     }
//     catch (e) {
//       finalThrow(cmdFailString('editlink'), e.message);
//     }
//     editorElement.contentWindow.focus();
//   }
// };

////-----------------------------------------------------------------------------------
var msiFollowLinkCommand =
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
        msiFollowLink(element);
    }
    catch (e) {
      finalThrow(cmdFailString('followLink'), e.message);
    }
    editorElement.contentWindow.focus();
  }
};

function msiFollowLink( element ) {
  var href = element.getAttribute("href");
  var theProcess = Components.classes["@mozilla.org/process/util;1"].createInstance(Components.interfaces.nsIProcess);
  var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].createInstance(Components.interfaces.nsIProperties);
  var extension;
  var exefile;
  var arg;
  var arr = new Array();
  if (href) {
    if (href.indexOf(".sci") > 0) {
      // do something
    }
    else {
      var os = getOS(window);
      if (os == "win")
      {
        extension = "cmd";
        arr = ['start', '/max', href];
      }
      else 
      {
        extension = "bash";
        arr = [href];
      }
      exefile = dsprops.get("resource:app", Components.interfaces.nsILocalFile);
      exefile.append("shell."+ extension);
      theProcess.init(exefile);
      theProcess.run(false, arr, arr.length);
    }
  }
}


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
    try
    {
      var editorElement = msiGetTopLevelEditorElement();
      msiSetEditMode(kDisplayModeNormal, editorElement);
    }
    catch (e) {
      finalThrow(cmdFailString('normalmode'), e.message);
    }
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
    try
    {
      var editorElement = msiGetTopLevelEditorElement();
      var previousMode = msiGetEditorDisplayMode(editorElement);
      if (previousMode == kDisplayModeSource)
        msiSetEditMode(kDisplayModeNormal, editorElement);
      msiSetEditMode(kDisplayModeAllTags, editorElement);
    }
    catch (e) {
      finalThrow(cmdFailString('alltagsmode'), e.message);
    }
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
    try
    {
      var editorElement = msiGetTopLevelEditorElement();
      msiSetEditMode(kDisplayModeSource, editorElement);
    }
    catch (e) {
      finalThrow(cmdFailString('sourcemode'), e.message);
    }
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
    try
    {
      var editorElement = msiGetTopLevelEditorElement();
      msiSetEditMode(kDisplayModePreview, editorElement);
    }
    catch (e) {
      finalThrow(cmdFailString('pdfpreviewmode'), e.message);
    }
  }
};

////-----Matrix commands
var msiSelectMatrixCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return (msiIsDocumentEditable() && msiIsEditingRenderedHTML());
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    try
    {
      var editorElement = msiGetActiveEditorElement();
        msiGetTableEditor(editorElement).selectTable();
        if (editorElement)
        editorElement.contentWindow.focus();
      else
        window.content.focus();
    }
    catch (e) {
      finalThrow(cmdFailString('selectmatrix'), e.message);
    }
  }
};

var msiSelectMatrixRow =
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
    if (msiIsInMatrixCell(editorElement))
      alert("Implement me!");
  }
};
var msiSelectMatrixColumn =
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
    if (msiIsInMatrixCell(editorElement))
      alert("Implement me!");
  }
};
var msiSelectMatrixCell =
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
    if (msiIsInMatrixCell(editorElement))
      alert("Implement me!");
  }
};
var msiSelectAllMatrixCells =
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
    if (msiIsInMatrixCell(editorElement))
      alert("Implement me!");
  }
};
var msiInsertMatrix =
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
    if (msiIsInMatrixCell(editorElement))
      alert("Implement me!");
  }
};
var msiInsertMatrixRowAbove =
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
    if (msiIsInMatrixCell(editorElement))
      alert("Implement me!");
  }
};
var msiInsertMatrixRowBelow =
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
    if (msiIsInMatrixCell(editorElement))
      alert("Implement me!");
  }
};
var msiInsertMatrixColumnBefore =
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
    if (msiIsInMatrixCell(editorElement))
      alert("Implement me!");
  }
};
var msiInsertMatrixColumnAfter =
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
    if (msiIsInMatrixCell(editorElement))
      alert("Implement me!");
  }
};
var msiDeleteMatrix =
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
    if (msiIsInMatrixCell(editorElement))
      alert("Implement me!");
  }
};
var msiDeleteMatrixRow =
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
    if (msiIsInMatrixCell(editorElement))
      alert("Implement me!");
  }
};
var msiDeleteMatrixColumn =
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
    if (msiIsInMatrixCell(editorElement))
      alert("Implement me!");
  }
};
var msiDeleteMatrixCellContents =
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
    if (msiIsInMatrixCell(editorElement))
      alert("Implement me!");
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
    try
    {
      var editorElement = msiGetActiveEditorElement();
      if (msiIsInTableCell(editorElement))
        msiEditorTableCellProperties(editorElement);
  //      EditorTableCellProperties();
      else
  //      EditorInsertOrEditTable(true);
        msiEditorInsertOrEditTable(true, editorElement, aCommand, this);
    }
    catch (e) {
      finalThrow(cmdFailString('insertedittable'), e.message);
    }
  }
};

////-----------------------------------------------------------------------------------

//This handler deals with the "cmd_editTable", "cmd_editTableCell", "cmd_editTableRows",  and "cmd_editTableCols" commands.
//  Code should pay attention to "aCommand", or at least to "tableNodeData", to know which to focus on.
var msiEditTableCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    if (msiIsInTable())
      return true;
    var propObj;
    var editorElement = msiGetActiveEditorElement();
    var editor = msiGetEditor(editorElement);
    if (editor && editor.selection.isCollapsed)
    {
      var aNode = editor.selection.focusNode;
      var anOffset = editor.selection.focusOffset;
      if ((aNode.nodeType != nsIDOMNode.TEXT_NODE) || (anOffset == 0))
      {
        aNode = msiFindRevisableNodeToLeft(aNode, anOffset, editor);
        if (editor.getElementOrParentByTagName("table", aNode))
          return true;
      }
    }
    return false;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon)
  {
    try
    {
      var editorElement = msiGetActiveEditorElement();
      var tableNodeData = msiGetPropertiesDataFromCommandParams(aParams);

      if (tableNodeData != null && editorElement != null)
      {
        msiEditorInsertOrEditTable(false, editorElement, aCommand, this, tableNodeData);
  //      var dlgWindow = msiDoModelessPropertiesDialog("chrome://prince/content/msiEdTableProps.xul", "_blank", "chrome,close,titlebar,dependent",
  //                                                     editorElement, "cmd_editTable", tableNode);
      }
      editorElement.focus();
    }
    catch (e) {
      finalThrow(cmdFailString('edittable'), e.message);
    }
  },

  doCommand: function(aCommand)
  {
    try
    {
      var editorElement = msiGetActiveEditorElement();
      msiEditorInsertOrEditTable(false, editorElement, aCommand, this);
    }
    catch (e) {
      finalThrow(cmdFailString('edittable'), e.message);
    }
  }
};


////-----------------------------------------------------------------------------------
var msiSelectTableCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    if (aCommand == "cmd_selectTable")
      return msiIsInTable();
    if (aCommand == "cmd_SelectMatrix")
      return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    try
    {
      var editorElement = msiGetActiveEditorElement();
      var editor = msiGetEditor(editorElement);
//      msiGetTableEditor(editorElement).selectAllTableCells();
      var element = editor.selection.anchorNode;
      var tableNode;
      if (element)
      {
        tableNode = GetParentTable(element);
        if (tableNode) {
          editor.selectElement(tableNode);
          editor.markNodeDirty(tableNode);
        }
      }
    
      if (editorElement) {
        editorElement.contentWindow.focus();
      }
      else {
        window.content.focus();
      }
    }
    catch (e) {
      finalThrow(cmdFailString('selecttable'), e.message);
    }
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
    try
    {
      var editorElement = msiGetActiveEditorElement();
      msiGetTableEditor(editorElement).selectTableRow();
      if (editorElement)
        editorElement.contentWindow.focus();
      else
        window.content.focus();
    }
    catch (e) {
      finalThrow(cmdFailString('selecttablerow'), e.message);
    }
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
    try
    {
      var editorElement = msiGetActiveEditorElement();
      msiGetTableEditor(editorElement).selectTableColumn();
      if (editorElement)
        editorElement.contentWindow.focus();
      else
        window.content.focus();
    }
    catch (e) {
      finalThrow(cmdFailString('selecttablecolumn'), e.message);
    }
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
    try
    {
      var editorElement = msiGetActiveEditorElement();
      msiGetTableEditor(editorElement).selectTableCell();
      if (editorElement)
        editorElement.contentWindow.focus();
      else
        window.content.focus();
    }
    catch (e) {
      finalThrow(cmdFailString('selecttablecell'), e.message);
    }
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
    try
    {
      var editorElement = msiGetActiveEditorElement();
      msiGetTableEditor(editorElement).selectAllTableCells();
      if (editorElement)
        editorElement.contentWindow.focus();
      else
        window.content.focus();
    }
    catch (e) {
      finalThrow(cmdFailString('selectalltablecells'), e.message);
    }
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
    try
    {
      var editorElement = msiGetActiveEditorElement();
      msiEditorInsertTable(editorElement, aCommand, this);
    }
    catch (e) {
      finalThrow(cmdFailString('inserttable'), e.message);
    }
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
    try
    {
      var editorElement = msiGetActiveEditorElement();
      msiGetTableEditor(editorElement).insertTableRow(1, false);
      if (editorElement)
        editorElement.contentWindow.focus();
      else
        window.content.focus();
    }
    catch (e) {
      finalThrow(cmdFailString('insertrowabove'), e.message);
    }
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
    try
    {
      var editorElement = msiGetActiveEditorElement();
      msiGetTableEditor(editorElement).insertTableRow(1, true);
      if (editorElement)
        editorElement.contentWindow.focus();
      else
        window.content.focus();
    }
    catch (e) {
      finalThrow(cmdFailString('insertrowbelow'), e.message);
    }
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
    try
    {
      var editorElement = msiGetActiveEditorElement();
      msiGetTableEditor(editorElement).insertTableColumn(1, false);
      if (editorElement)
        editorElement.contentWindow.focus();
      else
        window.content.focus();
    }
    catch (e) {
      finalThrow(cmdFailString('insertcolumnbefore'), e.message);
    }
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
    try
    {
      var editorElement = msiGetActiveEditorElement();
      msiGetTableEditor(editorElement).insertTableColumn(1, true);
      if (editorElement)
        editorElement.contentWindow.focus();
      else
        window.content.focus();
    }
    catch (e) {
      finalThrow(cmdFailString('insertcolumnafter'), e.message);
    }
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
    try
    {
      var editorElement = msiGetActiveEditorElement();
      msiGetTableEditor(editorElement).insertTableCell(1, false);
      if (editorElement)
        editorElement.contentWindow.focus();
      else
        window.content.focus();
    }
    catch (e) {
      finalThrow(cmdFailString('insertcellbefore'), e.message);
    }
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
    try
    {
      var editorElement = msiGetActiveEditorElement();
      msiGetTableEditor(editorElement).insertTableCell(1, true);
      if (editorElement)
        editorElement.contentWindow.focus();
      else
        window.content.focus();
    }
    catch (e) {
      finalThrow(cmdFailString('insertcellafter'), e.message);
    }
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
    try
    {
      var editorElement = msiGetActiveEditorElement();
      msiGetTableEditor(editorElement).deleteTable();
      if (editorElement)
        editorElement.contentWindow.focus();
      else
        window.content.focus();
    }
    catch (e) {
      finalThrow(cmdFailString('deletetable'), e.message);
    }
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
    if (rows == 0){
      rows = 1;
    }
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
    catch (e) {
      finalThrow(cmdFailString('deletetablerow'), e.message);
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
    catch (e) {
      finalThrow(cmdFailString('deletetablecolumn'), e.message);
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
    try
    {
      var editorElement = msiGetActiveEditorElement();
      msiGetTableEditor(editorElement).deleteTableCell(1);
      if (editorElement)
        editorElement.contentWindow.focus();
      else
        window.content.focus();
    }
    catch (e) {
      finalThrow(cmdFailString('deletetablecell'), e.message);
    }
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
    try
    {
      var editorElement = msiGetActiveEditorElement();
      msiGetTableEditor(editorElement).deleteTableCellContents();
      if (editorElement)
        editorElement.contentWindow.focus();
      else
        window.content.focus();
    }
    catch (e) {
     finalThrow(cmdFailString('deletetablecellcontents'), e.message);
    }
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
    try
    {
      var editorElement = msiGetActiveEditorElement();
      // Use nsnull to let editor find table enclosing current selection
      msiGetTableEditor(editorElement).normalizeTable(null);
      if (editorElement)
        editorElement.contentWindow.focus();
      else
        window.content.focus();
    }
    catch (e) {
      finalThrow(cmdFailString('normalizeTable'), e.message);
    }
  }
};

////-----------------------------------------------------------------------------------
var msiJoinTableCellsCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    try
    {
      var editorElement = msiGetActiveEditorElement();
      if (msiIsDocumentEditable(editorElement) && msiIsEditingRenderedHTML(editorElement))
      {
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
      return false;
    }
    catch (e) {
      throw(msiException('Error in msiJoinTableCellsCommand', e.message));
    }
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    try
    {
      var editorElement = msiGetActiveEditorElement();
      // Param: Don't merge non-contiguous cells
      var editor = msiGetTableEditor(editorElement);
      editor.joinTableCells(false);
      var tableNode;
      var element = editor.selection.anchorNode;
      if (element)
      {
        tableNode = GetParentTable(element);
        if (tableNode)
          checkForMultiRowInTable(tableNode, editor);
      }
      if (editorElement)
        editorElement.contentWindow.focus();
      else
        window.content.focus();
    }
    catch (e) {
      finalThrow(cmdFailString('jointablecells'), e.message);
    }
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
    try
    {
      var editorElement = msiGetActiveEditorElement();
      var editor = msiGetTableEditor(editorElement);
      editor.splitTableCell();
      var tableNode;
      var element = editor.selection.anchorNode;
      if (element)
      {
        tableNode = GetParentTable(element);
        if (tableNode)
          checkForMultiRowInTable(tableNode, editor);
      }
      if (editorElement)
        editorElement.contentWindow.focus();
      else
        window.content.focus();
    }
    catch (e) {
      finalThrow(cmdFailString('splittablecell'), e.message);
    }
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
    try
    {
      msiEditorSelectColor("TableOrCell", null, editorElement);
    }
    catch (e) {
      finalThrow(cmdFailString('tablecolor'), e.message);
    }
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
    try
    {
      goPreferences('editor', 'chrome://editor/content/pref-composer.xul','editor');
      window.content.focus();
    }
    catch (e) {
      finalThrow(cmdFailString('preferences'), e.message);
    }
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
    try
    {
      var editorElement = msiGetActiveEditorElement();
      msiFinishHTMLSource(editorElement);
    }
    catch (e) {
      finalThrow(cmdFailString('finishsource'), e.message);
    }
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
    try
    {
      var editorElement = msiGetActiveEditorElement();
      // In msiEditor.js
      msiCancelHTMLSource(editorElement);
    }
    catch (e) {
      finalThrow(cmdFailString('cancelsource'), e.message);
    }
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
		try
    {
      var editorElement = msiGetActiveEditorElement();
      if (this.isCommandEnabled())
      {
        var theWindow = msiGetWindowContainingEditor(editorElement);
        window.openDialog("chrome://editor/content/EdConvertToTable.xul","converttotable", "chrome,close,resizable,titlebar,modal");
  			msiGetEditor(editorElement).incrementModificationCount(1);
      }
      if (editorElement)
        editorElement.contentWindow.focus();
      else
        window.content.focus();
		}
    catch (e) {
      finalThrow(cmdFailString('c'), e.message);
    }
  }
};

///// "cmd_copypicture" /////////
var msiCopyPictureCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
  	var editorElement = msiGetActiveEditorElement();
  	var editor = msiGetEditor(editorElement);
  	if (!editor || !editor.selection || editor.selection.isCollapsed)
      return false;
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    try
    {
      var editorElement = msiGetActiveEditorElement();
      var editor = msiGetEditor(editorElement);
      if (editor && editor.selection && !editor.selection.isCollapsed)
        msiCopyAsPicture(editorElement);
    }
    catch (e) {
      finalThrow(cmdFailString('copypicture'), e.message);
    }
  }
};

///// "cmd_savepicture" /////////
var msiSavePictureCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
  	var editorElement = msiGetActiveEditorElement();
  	var editor = msiGetEditor(editorElement);
  	if (!editor || !editor.selection || editor.selection.isCollapsed)
      return false;
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    try
    {
      var editorElement = msiGetActiveEditorElement();
      var editor = msiGetEditor(editorElement);
      if (editor && editor.selection && !editor.selection.isCollapsed)
        msiSaveAsPicture(editorElement);
    }
    catch (e) {
      finalThrow(cmdFailString('savepicture'), e.message);
    }
  }
};


function msiNote(currNode, editorElement, type, hidden)
{
  var data= new Object();
  if (!editorElement)
    editorElement = msiGetActiveEditorElement();
  data.editorElement = editorElement;
  var wrapperNode;
  var markOrText, noteNumber;
  if (currNode) {
    data.noteNode = currNode;
    data.type = currNode.getAttribute("type");
    try
    {
      if (currNode.getAttribute("hide") == "true") data.hide=true;
      if (data.type == 'footnote')
      {
        wrapperNode = msiNavigationUtils.getParentOfType(currNode, "notewrapper");
        if (wrapperNode)
        {
          markOrText = wrapperNode.getAttribute("markOrText");
          if (markOrText && markOrText.length)
            data.markOrText = markOrText;
          noteNumber = wrapperNode.getAttribute("footnoteNumber");
          if (noteNumber && noteNumber.length)
            data.footnoteNumber = Number(noteNumber);
        }
      }
    }
    catch(e){}
  }
  else
  {
    //defaults
    data.type = "";
  }
  if (type)
  {
    if (!hidden) hidden=false;
    data.hidenote = hidden;
    data.type = type;
  }

  if (!type) {
    window.openDialog("chrome://prince/content/Note.xul","note", "chrome,close,titlebar,resizable=yes,dependent", data);
    // data comes back altered
		if (!data.Cancel) msiGetEditor(editorElement).incrementModificationCount(1);

//      return;
  }

  if (type)
    msiInsertOrReviseNote(currNode, editorElement, data);
//  dump(data.type + "\n");
//  var editor = msiGetEditor(editorElement);
//  editor.beginTransaction();
//  if (currNode)  // currnode is a note node
//  {
//    if (data.type == 'footnote')
//      currNode.parentNode.setAttribute("type","footnote");
//    else
//      currNode.parentNode.removeAttribute("type");
//    currNode.setAttribute("type",data.type);
//    currNode.setAttribute("hide",data.hide?"true":"false");
//    if (data.type != 'footnote')
//    {
//      currNode.setAttribute("req","ragged2e");
//      currNode.setAttribute("opt","raggedrightboxes");
//    }
//    else
//    {
//      if ("footnoteNumber" in data)
//        currNode.setAttribute("footnoteNumber", String(data.footnoteNumber));
//      else
//        currNode.removeAttribute("footnoteNumber");
//      if (data.markOrText != "markAndText")
//        currNode.setAttribute("markOrText", data.markOrText);
//      else
//        currNode.removeAttribute("markOrText");
//    }
//  }
//  else
//  {
//  try
//  {
//    var namespace = new Object();
//    var paraTag = editor.tagListManager.getDefaultParagraphTag(namespace);
//    var wrapperNode = editor.document.createElement('notewrapper');
//    if (data.type == 'footnote') wrapperNode.setAttribute('type','footnote');
//    var node = editor.document.createElement('note');
//    node.setAttribute('type',data.type);
//    node.setAttribute('hide','false');
//    if (data.type != 'footnote')
//    {
//      node.setAttribute("req","ragged2e");
//      node.setAttribute("opt","raggedrightboxes");
//    }
//    else
//    {
//      if ("footnoteNumber" in data)
//        node.setAttribute("footnoteNumber", String(data.footnoteNumber));
//      if (data.markOrText != "markAndText")
//        node.setAttribute("markOrText", data.markOrText);
//    }
//    var paraNode = editor.document.createElement(paraTag);
//    var brNode=editor.document.createElement('br');
//    brNode.setAttribute("type","_moz");
//    if (node)
//      wrapperNode.insertBefore(node, null);
//    if (paraNode)
//      node.insertBefore(paraNode, null);
//    if (brNode)
//      paraNode.insertBefore(brNode, null);
//    editor.insertElementAtSelection(wrapperNode, true);
//    editor.selection.collapse(paraNode, 0);
//  }
//  catch(e)
//  {
//    dump("msiNote: exception = '"+e.message+"'\n");
//  }
//  }
//  editor.endTransaction();
}

function msiInsertOrReviseNote(currNode, editorElement, data)
{
  if (data.Cancel)
    return;

  dump("In msiReviseNote, data.type is " + data.type + "\n");
  var editor = msiGetEditor(editorElement);
  var node;
  editor.beginTransaction();
  if (currNode)  // currnode is a note node
  {
    node = currNode;
    if (data.type == 'footnote')
      currNode.parentNode.setAttribute("type","footnote");
    else
      currNode.parentNode.removeAttribute("type");
    currNode.setAttribute("type",data.type);
    currNode.setAttribute("hide",data.hide?"true":"false");
    if (data.type != 'footnote')
    {
      currNode.setAttribute("req","ragged2e");
      currNode.setAttribute("opt","raggedrightboxes");
    }
    else
    {
      if ("footnoteNumber" in data)
        currNode.parentNode.setAttribute("footnoteNumber", String(data.footnoteNumber));
      else
        currNode.parentNode.removeAttribute("footnoteNumber");
      if ((data.markOrText != null) && (data.markOrText != "markAndText"))
        currNode.parentNode.setAttribute("markOrText", data.markOrText);
      else
        currNode.parentNode.removeAttribute("markOrText");
    }
  }
  else
  {
    try
    {
      var namespace = new Object();
      var paraTag = editor.tagListManager.getDefaultParagraphTag(namespace);
      var wrapperNode = editor.document.createElement('notewrapper');
      if (data.type == 'footnote') wrapperNode.setAttribute('type','footnote');
      node = editor.tagListManager.getNewInstanceOfNode("note", null, editor.document);
      node.setAttribute('type',data.type);
      node.setAttribute('hide','false');
      if (data.type != 'footnote')
      {
        node.setAttribute("req","ragged2e");
        node.setAttribute("opt","raggedrightboxes");
      }
      else
      {
        if ("footnoteNumber" in data)
          wrapperNode.setAttribute("footnoteNumber", String(data.footnoteNumber));
        if (data.markOrText != "markAndText")
          wrapperNode.setAttribute("markOrText", data.markOrText);
      }
      if (node)
      {
        wrapperNode.insertBefore(node, null);
        editor.insertElementAtSelection(wrapperNode, true);
//        editor.setCursorInNewHTML(node);
        editorElement.contentWindow.focus();
      }
    }
    catch(e)
    {
      dump("msiNote: exception = '"+e.message+"'\n");
    }
  }
  var cursors = msiNavigationUtils.getChildrenByTagName(node, "cursor");
  if (cursors) editor.setCursorInNewHTML(node);
  editor.endTransaction();
}

function msiTable(element,editorElement)
{
  return msiEditorInsertOrEditTable(false, editorElement, "", null, null);
}


function msiFrame(editorElement, editor, node)
{
  if (editor==null) editor = msiGetEditor(editorElement);
  editor.beginTransaction();
  window.openDialog("chrome://prince/content/Minipage.xul","frame", "chrome,close,titlebar,dependent, resizable=yes", node);
	msiGetEditor(editorElement).incrementModificationCount(1);
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

  window.openDialog("chrome://editor/content/EdColorPicker.xul", "colorpicker", "chrome,close,titlebar,modal,resizable",
  "", colorObj);

  // User canceled the dialog
  if (colorObj.Cancel)
    return;

  var cmdParams = newCommandParams();
  if (!cmdParams) return;

  var editorElement = msiGetActiveEditorElement();
  dump("EditorElement has name "+editorElement.id+"\n");
  cmdParams.setStringValue("color", colorObj.TextColor);
  editorElement.contentWindow.focus();
  msiGoDoCommandParams("cmd_fontcolor", cmdParams, editorElement);
//  theWindow.msiRequirePackage(editorElement, "xcolor", null);
}

var msiShowBibTeXLogCommand = 
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var result = false;
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
        var resurl = match[1]+"/tex/main.blg";
        var thefile = msiFileFromFileURL(msiURIFromString(resurl));
        result = thefile && 
          thefile.exists();
      }
    }
    return result;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    try
    {
      var result = true;
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
          var resurl = match[1]+"/tex/main.blg";
          openDialog("chrome://global/content/viewSource.xul",
                 "_blank",
                 "all,dialog=no",
                 resurl, null, null);
        }
      }
    }
    catch (e) {
      finalThrow(cmdFailString('showbibtexlog'), e.message);
    }
    return result;
  }
}

var msiShowTeXLogCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var result = false;
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
        var resurl = match[1]+"/tex/main.log";
        var thefile = msiFileFromFileURL(msiURIFromString(resurl));
        result = thefile && 
        thefile.exists();
      }
    }
    return result;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var result;
    try
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
          var resurl = match[1]+"/tex/main.log";
          openDialog("chrome://global/content/viewSource.xul",
                 "_blank",
                 "all,dialog=no",
                 resurl, null, null);
        }
      }
    }
    catch (e) {
      finalThrow(cmdFailString('showtexlog'), e.message);
    }
    return result;
  }
}


var msiShowTeXFileCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    var result = false;
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
        var resurl = match[1]+"/tex/main.tex";
        var thefile = msiFileFromFileURL(msiURIFromString(resurl));
        result = thefile && 
        thefile.exists();
      }
    }
    return result;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    var result;
    try
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
          var resurl = match[1]+"/tex/main.tex";
          openDialog("chrome://global/content/viewSource.xul",
                 "_blank",
                 "all,dialog=no",
                 resurl, null, null);
        }
      }
    }
    catch (e) {
      finalThrow(cmdFailString('showtexfile'), e.message);
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
    try
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
        window.openDialog('chrome://prince/content/gotoparagraph.xul','gotoparagraph', 'chrome,resizable,close,modal,titlebar',editor, contextNode, editorElement);
        window.content.focus();
      }
    }
    catch (e) {
      finalThrow(cmdFailString('gotoparagraph'), e.message);
    }
  }
};


var msiGoToMarkerCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    try
    {
      result = true;
      var editorElement = msiGetActiveEditorElement();
      if (!msiIsTopLevelEditor(editorElement))
        return;

      var editor = msiGetEditor(editorElement);
      if (editor)
      {
        window.openDialog('chrome://prince/content/gotomarker.xul','gotomarker', 'chrome,resizable,close,modal,titlebar',editorElement, this);
        window.content.focus();
      }
    }
    catch (e) {
      finalThrow(cmdFailString('gotomarker'), e.message);
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
    try
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
        window.openDialog('chrome://prince/content/wordcount.xul','wordcount', 'chrome,resizable,close,titlebar, dependent',wc);
        window.content.focus();

      }
    }
    catch (e) {
      finalThrow(cmdFailString('wordcount'), e.message);
    }
  }
};

function doReviseStructureNode(editor, origData, reviseData)
{
  if (!origData.secTitleNode)
  {
    dump("In msiComposerCommands.js, doReviseStructureNode() called without a title node!\n");
    return;
  }
  if (!origData.structNode)
  {
    dump("In msiComposerCommands.js, doReviseStructureNode() called without a structure node!\n");
    return;
  }

  msiEditorEnsureElementAttribute(origData.structNode, "subdoc", reviseData.subDocName, editor);
  msiEditorEnsureElementAttribute(origData.structNode, "nonum", reviseData.noNumAttr, editor);
  var shortTitleNode = origData.shortFormNode;
  if (reviseData.newShortForm && reviseData.newShortForm.length)
  {
    if (!shortTitleNode)
    {
      shortTitleNode = editor.document.createElementNS(xhtmlns, "shortTitle");
      editor.insertNode(shortTitleNode, origData.secTitleNode, 0);  //Short form always goes at the start
    }
    if (!origData.shortFormStr || (reviseData.newShortForm != origData.shortFormStr))
    {
      for (var ix = shortTitleNode.childNodes.length; ix > 0 ; --ix)
        editor.deleteNode(shortTitleNode.childNodes[ix-1]);
      editor.insertHTMLWithContext(reviseData.newShortForm, "", "", "", null, shortTitleNode, 0, false);
    }
  }
  else
  {
    if (shortTitleNode)
      editor.deleteNode(shortTitleNode);
  }
}

function doReviseEnvironmentNode(editor, origData, reviseData)
{
  if (!origData.envNode)
  {
    dump("In msiComposerCommands.js, doReviseEnvironmentNode() called without a environment node!\n");
    return;
  }

  var leadInTypeStr = (reviseData.leadInType == "auto") ? "" : reviseData.leadInType;  //Prevent adding leadInType="auto" as an attribute
  msiEditorEnsureElementAttribute(origData.envNode, "leadInType", leadInTypeStr, editor);
  var numberingStr = "";
  var reqStr = "";
  if (reviseData.bUnnumbered)
  {
    numberingStr = "none";
    reqStr = "amsthm";
  }
  msiEditorEnsureElementAttribute(origData.envNode, "numbering", numberingStr, editor);
  msiEditorEnsureElementAttribute(origData.envNode, "req", reqStr, editor);
  var customLeadInNode = origData.customLeadInNode;
  if (reviseData.leadInType == "custom")
  {
    if (!customLeadInNode)
    {
      customLeadInNode = editor.document.createElementNS(xhtmlns, "envLeadIn");
      editor.insertNode(customLeadInNode, origData.envNode, 0);  //Lead-in always goes at the start
    }
    if (!reviseData.customLeadInStr || !reviseData.customLeadInStr.length)
      reviseData.customLeadInStr = "?";
    if (!origData.customLeadInStr || (reviseData.customLeadInStr != origData.customLeadInStr))
    {
      for (var ix = customLeadInNode.childNodes.length; ix > 0 ; --ix)
        editor.deleteNode(customLeadInNode.childNodes[ix-1]);
      editor.insertHTMLWithContext(reviseData.customLeadInStr, "", "", "", null, customLeadInNode, 0, false);
    }
  }
  else
  {
    if (customLeadInNode)
      editor.deleteNode(customLeadInNode);
  }
}

function doReviseTheoremNode(editor, origData, reviseData)
{
  if (origData.envNode == null)
  {
    dump("In msiComposerCommands.js, doReviseTheoremNode() called without an environment node!\n");
    return;
  }

  var tagName = msiGetBaseNodeName(origData.envNode);
  var leadInTypeStr = (reviseData.leadInType == "auto") ? "" : reviseData.leadInType;  //Prevent adding leadInType="auto" as an attribute
  msiEditorEnsureElementAttribute(origData.envNode, "leadInType", leadInTypeStr, editor);

  var thmList;
  if ((origData.defaultNumbering != reviseData.defaultNumbering) || (origData.theoremstyle != reviseData.theoremstyle))
  {
    thmList = new msiTheoremEnvListForDocument(editor.document);
    thmList.changeDefaultForTag(tagName, origData.defaultNumbering, reviseData.defaultNumbering, origData.theoremstyle, reviseData.theoremstyle);
    thmList.detach();
  }
  var numberingStr = "";
  var reqStr = "";
  if (reviseData.defaultNumbering != reviseData.numbering)
  {
    numberingStr = reviseData.numbering;
    if (numberingStr == "none")
      reqStr = "amsthm";
  }
  msiEditorEnsureElementAttribute(origData.envNode, "numbering", numberingStr, editor);
  msiEditorEnsureElementAttribute(origData.envNode, "req", reqStr, editor);

  var customLeadInNode = origData.customLeadInNode;
  if (reviseData.leadInType == "custom")
  {
    if (!customLeadInNode)
    {
      customLeadInNode = editor.document.createElementNS(xhtmlns, "envLeadIn");
      editor.insertNode(customLeadInNode, origData.envNode, 0);  //Lead-in always goes at the start
    }
    if (!reviseData.customLeadInStr || !reviseData.customLeadInStr.length)
      reviseData.customLeadInStr = "?";
    if (!origData.customLeadInStr || (reviseData.customLeadInStr != origData.customLeadInStr))
    {
      for (var ix = customLeadInNode.childNodes.length; ix > 0 ; --ix)
        editor.deleteNode(customLeadInNode.childNodes[ix-1]);
      editor.insertHTMLWithContext(reviseData.customLeadInStr, "", "", "", null, customLeadInNode, 0, false);
    }
  }
  else
  {
    if (customLeadInNode)
      editor.deleteNode(customLeadInNode);
  }
}

var msiZoomInCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    try
    {
      ZoomManager.enlarge();
      var editorElement = msiGetActiveEditorElement();
      msiSetSavedViewPercent(editorElement, ZoomManager.zoom * 100);
    }
    catch (e) {
      finalThrow(cmdFailString('zoomin'), e.message);
    }
  }
};

var msiZoomOutCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    try
    {
      ZoomManager.reduce();
      var editorElement = msiGetActiveEditorElement();
      msiSetSavedViewPercent(editorElement, ZoomManager.zoom * 100);
    }
    catch (e) {
      finalThrow(cmdFailString('zoomin'), e.message);
    }
  }
};

var msiZoomResetCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    try
    {
      ZoomManager.reset();
      var editorElement = msiGetActiveEditorElement();
      msiSetSavedViewPercent(editorElement, 100);
    }
    catch (e) {
      finalThrow(cmdFailString('zoomin'), e.message);
    }
  }
};

var msiShowHelpCommand =
{
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },

  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand)
  {
    try
    {
      //alert('Show help');
    }
    catch (e) {
      finalThrow(cmdFailString('showhelp'), e.message);
    }
  }
}

var msiMakeTitleCommand = {
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },
  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand) {
    try {
      msiInsertLaTeXmarker('maketitle');
    }
    catch (e) {
      finalThrow(cmdFailString('maketitle'), e.message);
    }
  }
}

var msiMakeTOCCommand = {
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },
  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand) {
    try {
      msiInsertLaTeXmarker('maketoc');
    }
    catch (e) {
      finalThrow(cmdFailString('maketoc'), e.message);
    }
  }
}

var msiMakeLOTCommand = {
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },
  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand) {
    try {
      msiInsertLaTeXmarker('makelot');
    }
    catch (e) {
      finalThrow(cmdFailString('makelot'), e.message);
    }
  }
}

var msiMakeLOFCommand = {
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },
  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand) {
    try {
      msiInsertLaTeXmarker('makelof');
    }
    catch (e) {
      finalThrow(cmdFailString('makelof'), e.message);
    }
  }
}

var msiAppendixCommand = {
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },
  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand) {
    try {
      msiInsertLaTeXmarker('appendix');
    }
    catch (e) {
      finalThrow(cmdFailString('appendix'), e.message);
    }
  }
}

var msiMainMatterCommand = {
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },
  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand) {
    try {
      msiInsertLaTeXmarker('mainmatter');
    }
    catch (e) {
      finalThrow(cmdFailString('mainmatter'), e.message);
    }
  }
}

var msiBackMatterCommand = {
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },
  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand) {
    try {
      msiInsertLaTeXmarker('backmatter');
    }
    catch (e) {
      finalThrow(cmdFailString('backmatter'), e.message);
    }
  }
}

var msiFrontMatterCommand = {
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },
  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand) {
    try {
      msiInsertLaTeXmarker('frontmatter');
    }
    catch (e) {
      finalThrow(cmdFailString('frontmatter'), e.message);
    }
  }
}

var msiPrintIndexCommand = {
  isCommandEnabled: function(aCommand, dummy)
  {
    return true;
  },
  getCommandStateParams: function(aCommand, aParams, aRefCon) {},
  doCommandParams: function(aCommand, aParams, aRefCon) {},

  doCommand: function(aCommand) {
    try {
      msiInsertLaTeXmarker('printindex');
    }
    catch (e) {
      finalThrow(cmdFailString('printindex'), e.message);
    }
  }
}



function msiInsertTag(tagname){
  var editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  var node = editor.document.createElement(tagname);
  editor.insertElementAtSelection(node,true);
  var sel = editor.selection;
  var insertedNode = sel.focusNode;
  while (insertedNode && insertedNode.tagName != tagname) insertedNode = insertedNode.parentNode;
  if (insertedNode && insertedNode.tagName == tagname)
  {
    editor.setCaretAfterElement(insertedNode);
  }
}

function msiInsertLaTeXmarker(tagname) {
  return msiInsertTag(tagname);
  var editorElement = msiGetActiveEditorElement();
  var editor = msiGetEditor(editorElement);
  focusOnEditor();
  var dataString="&lt;"+tagname+"/&gt;";
  var contextString = "";
  var infoString="(0,0)";
  editor.insertHTMLWithContext(dataString,
                              contextString, infoString, "text/html",
                              null,null,0,true);
}

function defineSelection()
{
  const kOutputSelectionOnly = Components.interfaces.nsIDocumentEncoder.OutputSelectionOnly;
  var selectionString;
  var editor = msiGetCurrentEditor();
  selectionString = editor.outputToString("text/plain", kOutputSelectionOnly);
  window.location.href = "dict:///" + selectionString;
}
