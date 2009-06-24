<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd" [
<!ENTITY % brandDTD SYSTEM "chrome://global/locale/brand.dtd" >
%brandDTD;

 ]>
<?xml-stylesheet href="resource://app/res/css/helpFileLayout.css" type="text/css"?>
<html xmlns="http://www.w3.org/1999/xhtml"> <head>
<title>Creating Web Pages with Scientific WorkPlace</title>
<link rel="stylesheet" type="text/css" href="chrome://help/skin/helpFileLayout.css"/>
</head>

<body> <section><sectiontitle>Creating Web Pages
with Scientific WorkPlace</sectiontitle>

<p>Scientific WorkPlace lets you create your own web pages and publish them
on the web. You don't have to know HTML to use Scientific WorkPlace; it is
as easy to use as a word processor.</p>

<p>Toolbar buttons let you add lists, tables, images, links to other
pages, colors, and font styles. You can see what your document will
look like on the Web as you create it, and you can easily share your
document with other users, no matter what type of browser or
HTML-capable email program they use.</p>

<div class="contentsBox">In this section:
<ul>
  <li><a href="#starting_a_new_page">Starting a New Page</a></li>

  <li><a href="#formatting_your_web_pages">Formatting Your Web
  Pages</a></li>

  <li><a href="#using_style_sheets_for_your_web_pages">Using Style
  Sheets to Style Your Web Pages</a></li>

  <li><a href="#adding_tables_to_your_web_page">Adding Tables to
  Your Web Page</a></li>

  <li><a href="#adding_pictures_to_your_web_page">Adding Pictures
  (Images) to Your Web Page</a></li>

  <li><a href="#working_with_table_of_contents_on_your_web_page">Working with
  Table of Contents on Your Web Page</a></li>

  <li><a href="#setting_page_properties">Setting Page
  Properties</a></li>

  <li><a href="#creating_links_in_composer">Creating Links in
  Scientific WorkPlace</a></li>

  <li><a href="publishing_help.xhtml">Publishing
  Your Pages on the Web</a></li>

  <li><a href="#composer_preferences">Scientific WorkPlace
  Preferences</a></li>
</ul> </div>

<subsection><sectiontitle>Starting a New Page</sectiontitle>

<subsubsection><sectiontitle>Creating a New Page</sectiontitle>

<p>Scientific WorkPlace is an HTML (Hypertext Markup Language) editor that
allows you to create and edit web pages. Scientific WorkPlace is a
<em>WYSIWYG</em> (What You See Is What You Get) editor, so you can
display how your page will look to the reader as you're creating
it. It is not necessary for you to know HTML, since most of the basic
HTML functions are available as commands from the toolbars and
menus.</p>

<p>Scientific WorkPlace also lets you edit the HTML source if you want. To
view or edit the HTML source code, open the View menu, and choose HTML
Source, or click the &lt;HTML&gt; Source tab in the Edit Mode toolbar
at the bottom of the Scientific WorkPlace window.</p>

<p>To create a web page, use one of the methods described below.  Once
you've started a page, you can add and edit text just as you would in
a word processor.</p>

<p><bold>To create a new page in Scientific WorkPlace</bold>:</p>

<ul>
  <li>Click the New button in Scientific WorkPlace's toolbar.</li>
</ul>

<p><strong>To start from an HTML file stored on your local
drive</strong>:</p>

<ol>
  <li>Open the Window menu and choose Scientific WorkPlace. You see the
  Scientific WorkPlace window.</li>

  <li>Open the File menu and choose Open File. You see the Open HTML
  File dialog box.</li>

  <li>On your local drive, locate the file that you want to edit.</li>

  <li>Click Open to display the specified file in a Scientific WorkPlace
  window.</li>
</ol>

<p><strong>To edit a web page</strong>:</p>

<ol>
  <li>Open Scientific WorkPlace</li> <li>Click on File menu and choose
  Open Web Location. You will see the Open Web Location dialog box.</li>

  <li>Type in the URL of the page to edit (for example,
  <tt>http://www.mozilla.org</tt>) and click Create button. You will see
  the page displayed in Scientific WorkPlace.</li> 
</ol>

<p><strong>Tip</strong>: In the Scientific WorkPlace window you can
quickly open the most recent file you've been working on by opening
the File menu, choosing Recent Pages, and then selecting the file you
want from the list.</p>

<p>[ <a href="#starting_a_new_page">Return to beginning of section</a>
]</p>

</subsubsection><subsubsection><sectiontitle>Saving and Browsing Your
New Page</sectiontitle>

<p>You can save Scientific WorkPlace documents in HTML or text-only
format.  Saving a document in HTML format preserves the document's
formatting, such as text styles (for example, bold or italic), tables,
links, and images. Saving a document in text-only format removes all
the HTML tags but preserves the document's text.</p>

<p>To save a document as an HTML file:</p>

<ul> 
  <li>Open the File menu and choose Save or click the Save button
  on the Composition toolbar.

  <p>If you haven't already given your page a title, Scientific WorkPlace
  prompts you to do so. Scientific WorkPlace displays the page title in the
  browser window's title bar when you view the page in the browser. The
  document's page title also appears in your list of bookmarks if you
  bookmark the page.</p>

  <p>Scientific WorkPlace then prompts you to enter a filename and specify
  the location where you want to save the file. Make sure you preserve
  the .html extension in the filename.</p>
  </li>
</ul>

<p>To change the filename or location of an existing HTML file:</p>

<ul>
  <li>Choose Save As and select a different filename or
  location.</li>
</ul>

<p>When you save a page in Scientific WorkPlace, all parts of the page
(the HTML, images and other files, such as sound files and style
sheets), are saved locally on your hard drive. If you only want to
save the HTML part of the page, you must change the Scientific WorkPlace
preference for saving pages. See <a href="#composer_preferences">Scientific WorkPlace Preferences</a> for more
information on changing Scientific WorkPlace's setting for saving
pages.</p>

<p>If an image location is absolute (starts with "http://") and you
are connected to the Internet, you will still see that image in the
document in Scientific WorkPlace and Navigator. However, if the image
location is relative to the page location (starts with "file:///"),
then you won't see the image in the local version of the document.</p>

<p>To save a document as a text-only file:</p>

<ol>
  <li>Open the File menu and choose Export to Text.</li>

  <li>Enter the filename and specify the location where you want to save
  the file.</li>
</ol>

<p><strong>Note</strong>: Images do not appear in documents saved in
the text-only format.</p>

<p><strong>Tip</strong>: You can choose Revert from the File menu to
retrieve the most recently saved copy of the document in which you're
working. Keep in mind that your current changes will be lost.</p>

<p>To view your page in a browser window in order to test your
links:</p>

<ul>
  <li>Open the File menu and choose Browse Page (or click Browse in
  the Composition toolbar). If you have not yet saved your document,
  Scientific WorkPlace prompts you to enter a page title, filename, and
  location.  The Scientific WorkPlace window remains open behind the new
  Navigator window.</li>
</ul>

<p>[ <a href="#starting_a_new_page">Return to beginning of
section</a> ]</p>

</subsubsection></subsection><subsection><sectiontitle>Formatting Your Web Pages</sectiontitle>

<subsubsection><sectiontitle>Formatting
Paragraphs, Headings, and Lists</sectiontitle>

<p>To apply a format to a paragraph, begin from the Scientific WorkPlace
window:</p>

<ol>
  <li>Click to place the insertion point where you want the format
  to begin, or select the text you want to format.</li>

  <li>Choose a paragraph format using the drop-down list in the Format
  toolbar:</li>

  <li style="list-style-type: none; list-style-position: outside; list-style-image: none;"> <ul> <li><strong>Body Text</strong>: Applies
      the application default font and style for regular text, without
      affecting the spacing before or after the text.</li>

      <li><strong>Paragraph</strong>: Inserts a paragraph tag (use this to
      begin a new paragraph). The paragraph includes top and bottom
      margins.</li>

      <li><strong>Heading 1</strong> - <strong>Heading 6</strong>: Formats
      the paragraph as a heading. Heading 1 is the highest-level heading,
      while Heading 6 is the lowest-level heading.</li>

      <li><strong>Address</strong>: Can be used for a web page "signature"
      that indicates the author of the page and the person to contact for
      more information, for example: <tt>user@example.com</tt>

      <p>You might want to include the date and a copyright notice. This
      format usually appears at the bottom of the web page under a
      horizontal line. Navigator displays the address format in italics.</p>
      </li>

      <li><strong>Preformat</strong>: This is useful for elements such as
      code examples, column data, and mail messages that you want displayed
      in a fixed-width font. In normal text, most browsers remove extra
      spaces, tabs, and paragraph returns. However, text that uses the
      Preformatted style is displayed with the white space intact,
      preserving the layout of the original text.</li>
      </ul>
  </li>
</ol>

<p>To format text as a heading:</p>

<ol>
  <li>Click to place the insertion point anywhere within the text
  that you want to format.</li>

  <li>Using the drop-down list in the Format toolbar, choose the level
  of heading you want, from 1 (largest) to 6 (smallest). Choose "Heading
  1" for your main heading, "Heading 2" for the next level, and so
  forth.</li>
</ol>

<p>To apply a list item format:</p>

<ol>
  <li>Click to place the insertion point within the line of text
  that you want to format.</li>

  <li>Open the Format menu and choose List.</li>

  <li>Choose the list style:</li>

  <li style="list-style-type: none; list-style-position: outside; list-style-image: none;"> <ul> <li><strong>Bulleted</strong>: Each
      item has a bullet (dot) next to it (as in this list).</li>

      <li><strong>Numbered</strong>: Items are numbered.</li>

      <li><strong>Term</strong> and <strong>Definition</strong>: These two
      styles work together, creating a glossary-style appearance. Use the
      Term tag for the word being defined, and the Definition tag for the
      definition. The Term text appears flush left, and the Definition text
      appears indented.</li>
      </ul>
  </li>
</ol>

<p><strong>Tip</strong>: You can quickly apply a list style to a block
of text by selecting the text
and clicking the Numbered List

<img height="21" width="21" src="images/numbers.gif" alt=""/>
or Bulleted List
<img height="20" width="20" src="images/bullets.gif" alt=""/>
buttons on the Format toolbar.</p>

<p>To change the style of bullets or numbers:</p>

<ol>
  <li>Click to place the insertion point within the text of the
  list item you want to change, or select one or more items in the list
  if you want to apply a new style to the entire list.</li>

  <li>Open the Format menu and choose List Properties.</li>

  <li>Select a bullet or number style from the drop-down list. For
  numbered lists, you can specify a starting number. For bulleted lists,
  you can change the bullet style.</li>
</ol>

<p><strong>Tip</strong>: You can also double-click on a bullet or
number in a list to display the List Properties dialog box.</p>

<p>To align a paragraph or text in your page, for example, centering
or aligning to the left or right:</p>

<ol>
  <li>Click to place the insertion point within the paragraph or
  line of text you want to align.</li>

  <li>Open the Format menu and choose Align; then choose an alignment
  option.</li>
</ol>

<p><strong>Note</strong>: You can also use the Format toolbar to align
text.</p>

<p class="returnToSection">[ <a href="#formatting_your_web_pages">Return to beginning of section</a>
]</p>

</subsubsection><subsubsection><sectiontitle>Working with Lists</sectiontitle>

<p>To end a list and continue typing body text:</p>

<ul>
  <li>Click to place the insertion point at the end of the last
  list item and press Enter (Return on Mac OS) twice to end the
  list.</li>
</ul>

<p>To change one or more list items to body text:</p>

<ol>
  <li>Click to place the insertion point within the list item, or
  select the list items.</li>

  <li>In a numbered list, click the numbered list button (or in a
  bulleted list, click the bulleted list button) in the Format
  toolbar.</li>
</ol>

<p>To position indented text below a list item:</p>

<ol>
  <li>Click to place the insertion point within the list item.</li>

  <li>Press Shift-Enter to create the hanging indent.</li>

  <li>Type the text you want to indent.</li>

  <li>Press Shift-Enter to create another indented paragraph, or press
  Return to create the next list item.</li>
</ol>

<p id="increase_or_decrease_the_indentation_of_list_items"><strong>Tip</strong>:
You can increase or decrease the indentation of list items by clicking
anywhere in a list item and then clicking the Indent or Outdent button
on the Format toolbar. Alternatively, click anywhere in a list item
and press Tab to indent one level.  Press Shift+Tab to outdent one
level.</p>

<p>To merge two adjacent lists:</p>

<ol>
  <li>Select the two lists that you want to merge. Be sure to
  select all of the elements in both lists. Note that any text in
  between the two lists will also become part of the merged list.</li>

  <li>Click the bulleted or numbered list button in the Format toolbar
  to merge the lists.</li>
</ol>

<p class="returnToSection">[ <a href="#formatting_your_web_pages">Return to beginning of section</a>
]</p>

</subsubsection><subsubsection><sectiontitle>Changing Text Color,
Style, and Font</sectiontitle>

<p>To change the style, color, or font of selected text:</p>

<ol>
  <li>Select the text you want to format.</li>

  <li>Open the Format menu and choose one of the following:</li>

  <li style="list-style-type: none; list-style-position: outside; list-style-image: none;">
    <ul>
      <li><strong>Font</strong>: Use this to
      choose a font. If you prefer to use fonts specified by the reader's
      browser, select Variable Width or Fixed Width.

      <p><strong>Note</strong>: Not all fonts installed on your computer
      appear.  Instead of specifying a font that may not be available to all
      who view your web page, it's generally best to select one of the fonts
      provided in the menu since these fonts work on every computer. For
      example, the fonts Helvetica, Arial, Times, and Courier generally look
      the same when viewed on different computers. If you select a different
      font, it may not look the same when viewed using a different
      computer.</p> </li>

      <li><strong>Size</strong>: Use this to choose a <em>relative</em> font
      size or select an option to increase or decrease text size (relative
      to the surrounding text).</li>

      <li><strong>Text Style</strong>: Use this to select a style, such as
      italic, bold, or underline, or to apply a structured style, for
      example, Code.</li>

      <li><strong>Text Color</strong>: Use this to choose a color from the
      color picker. If you are familiar with HTML hexadecimal color codes,
      you can type a specific code or you can just type a color name (for
      example, "blue"). You'll find a handy color code converter <a href="http://builder.cnet.com/webbuilding/pages/QuickReference/Color/converter.html?tag=st.bl.7650.ref_l.bl_converter" target="_blank">here</a>.</li> 
    </ul>
  </li>
</ol>

<p>To change the background color of the page:</p>

<ol>
  <li>Click anywhere in the page.</li>

  <li>Click the background color block in the Format toolbar.</li>

  <li>Choose a background color from the Block Background Color dialog
  box.</li>

  <li>Click OK.</li>
</ol>

<p><strong>Tip</strong>: To quickly change the color of text to the
color last used, select the text, then press Shift and click on the
text color block in the Format toolbar. This is useful when you want
to use one color for separate lines of text.</p>

<p>You can also use an image as a background. See <a href="#setting_page_colors_and_backgrounds">Setting Page Colors and
Backgrounds</a>.</p>

<p class="returnToSection">[ <a href="#formatting_your_web_pages">Return to beginning of section</a>
]</p>

</subsubsection><subsubsection><sectiontitle>Removing or
Discontinuing Text Styles</sectiontitle>

<p>To remove all text styles (bold, italic, and so on) from selected
text:</p>

<ol>
  <li>Select the text.</li>

  <li>Open the Format menu and choose Remove All Text Styles.</li>

  <li>Continue typing.</li>
</ol>

<p>To continue typing text with all text styles removed:</p>

<ol>
  <li>Place the insertion point where you want to discontinue the
  text styles.</li>

  <li>Open the Format menu and choose Discontinue Text Styles.</li>

  <li>Continue typing.</li>
</ol>

<p class="returnToSection">[ <a href="#formatting_your_web_pages">Return to beginning of section</a>
]</p>

</subsubsection><subsubsection><sectiontitle>Finding and Replacing Text</sectiontitle>

<p>To find text in the page you're currently working on:</p>

<ol>
  <li>Click to place the insertion point where you want to begin your
  search.</li>

  <li>Open the Edit menu and choose Find and Replace. You see the Find
  and Replace dialog
  box.</li>

  <li>Type the text you want to locate in the "Find what" field. To
  narrow the search, check
  one or more of the following options:
  <p><br/></p>
  <ul>
    <li><strong>Match upper/lower case</strong>: Use this to specify
    whether the search is for
    case-sensitive text.</li>

    <li><strong>Wrap around</strong>: Use this to search to the end
    of the page and then start
    again from the top or bottom, depending on whether you are
    searching forward or
    backwards.</li>

    <li><strong>Search backwards</strong>: Use this to search back
    from the insertion point to the
    beginning of the page.</li>

  </ul>
  </li>
  <li>Click Find Next to begin searching. When Scientific WorkPlace
  locates the first occurrence of the
  text, click Find Next to search for the next occurrence.</li>
  <li>Click Close when you are done.</li>
</ol>

<p>To find and replace text in the page you're currently working
on:</p>

<ol>
  <li>Click to place the insertion point where you want to begin your
  search.</li>

  <li>Open the Edit menu and choose Find and Replace. You see the Find
  and Replace dialog
  box.</li>

  <li>Type the text you want to find and then type the replacement
  text.</li>

  <li>To narrow the search, check one or more of the following
  options:

  <ul>
    <li><strong>Match upper/lower case</strong>: Use this to specify
    whether the search is for
    case-sensitive text. If you don't select this option, the
    search will find matching
    text in both upper and lower case.</li>

    <li><strong>Wrap around</strong>: Use this to search to the end
    of the page and then start again
    from the top.</li>

    <li><strong>Search backwards</strong>: Use this to search from
    the end to the beginning of the
    page.</li>

  </ul>
  </li>

  <li>Click Find Next to search for the next
  occurrence. Scientific WorkPlace selects the next occurrence
  of the text.</li>

  <li>Click Replace to replace the selected text with the replacement
  text. Click Replace and
  Find to replace the selected text and find the next
  occurrence. Click Replace All to
  replace every occurrence in the document with the replacement
  text.</li>

  <li>Click Close when you are done.</li>
</ol>

<p class="returnToSection">[ <a href="#formatting_your_web_pages">Return to beginning of section</a>
]</p>

</subsubsection><subsubsection><sectiontitle>Inserting Horizontal Lines</sectiontitle>

<p>Horizontal lines are typically used to separate different sections
of a document visually. To insert a horizontal line (also called a
<em>rule</em>) in your page, begin from the Scientific WorkPlace
window:</p>

<ol>
  <li>Click to place the insertion point where you want the line to
  appear.</li>

  <li>Open the Insert menu and choose Horizontal Line.</li>
</ol>

<h3 id="setting_horizontal_line_properties">Setting Horizontal Line
Properties</h3>

<p>You can customize a line's height, length, width, alignment, and
shading.</p>

<ol>
  <li>Double-click the line to display the Horizontal Line Properties
  dialog box.</li>

  <li>Edit any of these properties:

  <ul>
    <li><strong>Width</strong>: Enter the width and then choose "%
    of window" or "pixels." If you
    specify width as a percentage, the line's width changes
    whenever the Scientific WorkPlace
    window's or browser window's width changes.</li>

    <li><strong>Height</strong>: Type a number for the line's height
    (in pixels).</li>

    <li><strong>3-D Shading</strong>: Select this to add depth to
    the line by adding a bevel
    shading.</li>

    <li><strong>Alignment</strong>: Specify where you want to place
    the line (left, center, or
    right).</li>
  </ul>
  </li>

  <li>Click Use as Default to use these settings as the default the
  next time you insert a
  horizontal line.</li>

  <li>To edit the properties of a horizontal line manually, click
  Advanced Edit. See the
  section, <a href="#using_the_advanced_property_editor">Advanced
  Property Editor</a>, for details.</li>
</ol>

<p><strong>Tip:</strong> You can select "Show All Tags" from the View
menu to show all the HTML
elements in yellow boxes. Click any yellow box to select everything
within that HTML tag
or element. Double-click any yellow box to display the
<a href="#using_the_advanced_property_editor">Advanced Property
Editor</a> dialog box for that HTML tag or
element.</p>

<p class="returnToSection">[ <a href="#formatting_your_web_pages">Return to beginning of section</a>
]</p>

</subsubsection><subsubsection><sectiontitle>Inserting Special
Characters</sectiontitle>

<p>To insert special characters such as accent marks, copyrights, or
currency symbols:</p>

<ol>
  <li>Click to place the insertion point where you want the special
  character to appear.</li>

  <li>Open the Insert menu and choose Characters and Symbols. You see
  the Insert Character dialog box.</li>

  <li>Select a category of characters.

  <ul> <li>If you choose Accent Uppercase or Accent Lowercase, then open
  the Letter drop-down list and select the letter you wish to apply an
  accent to. (Note: not all letters have accented forms.) Select Common
  Symbols to insert special characters such as copyright symbols or
  fractions.</li>
  </ul>
  </li>

  <li>From the Character drop-down list, select the character you want
  to insert.</li>

  <li>Click Insert.

  <p>You can continue typing in your document (or in a mail compose
  window) while you keep this dialog box open, in case you want to use
  it again.</p> </li>

  <li>Click Close when you are done inserting special characters.</li>
</ol>

<p class="returnToSection">[ <a href="#formatting_your_web_pages">Return to beginning of section</a>
]</p>

</subsubsection><subsubsection><sectiontitle>Inserting HTML
Elements and Attributes</sectiontitle>

<p>If you understand how to work with HTML source code, you can insert
additional tags, style attributes, and JavaScript into your page. If
you are not sure how to work with HTML source code, it's best not to
change it. To work with HTML code, use one of these methods:</p>

<ul>
  <li>Place the insertion point where you want to insert the HTML
  code, or select the text you want to edit, and then open the Insert
  menu and choose HTML. In the Insert HTML dialog box, enter HTML tags
  and text, and then click Insert.</li>

  <li>Select an element such as a table, named anchor, image, link, or
  horizontal line. Double-click the element to open the associated
  properties dialog box for that item. Click Advanced Edit to open the
  Advanced Property Editor. You can use the Advanced Property Editor to
  add HTML attributes, JavaScript, and CSS to objects.</li>

  <li>Open the View menu, and choose HTML Source, or click the
  &lt;HTML&gt; Source tab in the Edit Mode toolbar at the bottom of the
  Scientific WorkPlace window. (If you don't see the Edit Mode toolbar, open
  the View menu and choose Show/Hide; then make sure the Edit Mode
  Toolbar is checked.)</li>
</ul>

</subsubsection><subsubsection><sectiontitle>Using the Advanced
Property Editor</sectiontitle>

<p>To add HTML attributes and JavaScript to objects such as tables,
images, and horizontal lines, you can use the Advanced Property
Editor.</p>

<p><strong>Note</strong>: Unless you clearly understand how to add,
delete, or modify HTML attributes and their associated values, it's
best not to do so.</p>

<p>If you are not currently viewing the Advanced Property Editor
dialog box, follow these steps:</p>

<ol>
  <li>From the View menu (or the Edit Mode toolbar), choose Show All
  Tags.</li>

  <li>Double-click the object that you want to modify to open its
  Properties dialog box.</li>

  <li>Click Advanced Edit to open the object's Advanced Property
  Editor. The Advanced Property Editor has three tabs, each of which
  lists the current properties for the selected object:

  <ul>
    <li><strong>HTML Attributes</strong>: Click this tab to view or
    enter additional HTML
    attributes.</li>

    <li><strong>Inline Style</strong>: Click this tab to view or
    enter additional CSS
    (cascading style sheet) properties through the
    &lt;style&gt; attribute.
    For more information on using CSS styles in
    Scientific WorkPlace, see
    <a href="#composer_preferences_composer">Scientific WorkPlace
    Preferences</a>.</li>

    <li><strong>JavaScript Events</strong>: Click this tab to view
    or enter JavaScript events.</li>

  </ul>
  </li>

  <li>To edit a property or attribute in any of the three lists,
  select the attribute you
  want to edit. You can then edit the attribute's name or value
  using the editable
  Attribute and Value fields at the bottom of the dialog box. To
  add a new attribute,
  type it in the Attribute field at the bottom of the dialog
  box. The new attribute is
  automatically added when you click in the Value field. To
  remove an attribute, select
  it in the list, and click Remove Attribute.
  <p><strong>Note</strong>: Required attributes are highlighted in
  the Attribute list.</p></li>

  <li>Click OK to apply your changes to the Advanced Property Editor
  dialog box.</li>

  <li>Click OK again to exit the Properties dialog box.</li>
</ol>

<p>Scientific WorkPlace automatically places quotation marks around any
attribute text.</p>

<p class="returnToSection">[ <a href="#formatting_your_web_pages">Return to beginning of section</a>
]</p>

</subsubsection><subsubsection><sectiontitle>Validating the HTML</sectiontitle>

<p>Before you put your document on a web server so that others can see
it, you should first check the document's HTML formatting to make sure
it conforms to web standards. Documents containing validated HTML are
less likely to cause problems when viewed by different browsers. Just
visually checking your web pages in Navigator doesn't ensure that your
document will appear correctly when viewed in other web browsers.</p>

<p>Scientific WorkPlace provides a convenient way for you to check that
your document conforms to W3C (World Wide Web Consortium) HTML
standards. Scientific WorkPlace uses the W3C HTML Validation Service,
which checks your document's HTML syntax for compliance with HTML 4.01
standards.  This service also provides information on how to correct
errors.</p>

<p><strong>Note</strong>: You must be connected to the Internet to use
this feature.</p>

<p>To validate your document's HTML syntax:</p>

<ol>
  <li>Open the Tools menu, and choose Validate HTML. If you have
  unsaved changes, Scientific WorkPlace asks you to save them before
  proceeding.</li>

  <li>Scientific WorkPlace will open another window with results for your
  page displayed in it after the W3C Validation Service program parses
  your html file.</li>

</ol>

<p class="returnToSection">[ <a href="#formatting_your_web_pages">Return to beginning of section</a>
]</p>

</subsubsection><subsubsection><sectiontitle>Choosing the Right Editing
Mode</sectiontitle>

<p>Typically, you won't need to change the editing mode from the
default (Normal). However, if you want to work with the document's
HTML source code, you may want to change editing modes.</p>

<p>Scientific WorkPlace allows you to quickly switch between four editing
modes or views. Each editing mode allows you to continue working on
your document, but displays varying levels of HTML tags (and tag
icons).</p>

<p>Before you choose an editing mode:</p>

<ul>
  <li>Open the View menu, choose Show/Hide, and then make sure
  there is a checkmark next to Edit Mode Toolbar.</li>
</ul>

<p>The Edit Mode toolbar has four tabs:</p>

<ul>
  <li><strong>Normal</strong>: Choose this editing mode to see how
  the document will look online while you are creating it. Choose this
  mode to show table borders and named anchor icons. All other HTML tag
  icons are hidden.</li>

  <li><strong>Show All Tags</strong>: Choose this mode to show all HTML
  tag icons.</li>

  <li><strong>&lt;HTML&gt; Source</strong>: Choose this mode to view and
  edit the document as unformatted HTML source code. When you save the
  document, the Normal mode reappears.</li>

  <li><strong>Preview</strong>: Choose this mode to display and edit the
  document exactly as it would appear in a browser window, except that
  links and JavaScript functions will not be active.</li>
</ul>

<p><strong>Note</strong>: JavaScript functions, frames, links, Java,
embedded objects and animated GIF files are not active in any of the
editing modes. To display these items in their active state, click the
Browse button on the Composition toolbar to load the page into a
browser window.</p>

<p class="returnToSection">[ <a href="#formatting_your_web_pages">Return to beginning of section</a>
]</p>

</subsubsection><subsubsection><sectiontitle>Using Status
Bar for Formatting Your Web Page</sectiontitle>

<p>Scientific WorkPlace has an innovative feature of editing/applying
inline styles and applying class or id tags to selected text via its
status bar. Styling using the status bar is possible in all modes
except <strong>&lt;HTML&gt; Source</strong> viewing mode.</p>

<ul>
  <li><strong>Select</strong> Clicking on "Select" will select the
entire text bounded by the style tag.</li>
  <li><strong>Remove Tag</strong> Clicking on "Remove Tag" will remove
the style tag, and in turn all the styles for that tag will be
removed.</li>

  <li><strong>Change Tag</strong> Using this option, the user can
change the tag used for the text with the least effort. Clicking on
"Change Tag" makes the text for the corresponding tag to be editable
in the status bar. Type in the desired tag and press "Enter". The
default properties of the tag will be applied to the text in the web
page.</li>

  <li><strong>Inline Styles</strong>While in any of the Normal, HTML Tags, or Preview viewing modes the tags surrounding the current position of the cursor are shown in the
  status bar of Scientific WorkPlace's window. Style properties of any of
  the tags can be changed by choosing an option from the context
  menu. To change inline style properties:

  <ol>
    <li>Right click on the tag you wish to edit.</li>
    <li>Click on "Inline Styles" </li>
    <li>Select the properties section you want to edit:
    <ul>
      <li>Text Properties</li>
      <li>Border Properties</li>
      <li>Background Properties</li>
      <li>Box Properties</li>
      <li>Aural Properties</li>
      <li>Extract and create Generic Style</li>
    </ul></li>
  </ol>
  <p>Secting any of the options except the last one will open the
corresponding tab section from CaScadeS CSS editor (more about
CaScadeS in <a href="#using_style_sheets_for_your_web_pages">Using
Style Sheets</a> section). With the corresponding section open, the
user can define his/her own styles which will be saved according to
W3C CSS coding style.</p>

  <p>The <strong>Extract and create Generic Style</strong> option
allows the user to extract the style information into an style rule
and save it in the external/internal style sheet.</p></li>

  <li><strong>Templates</strong></li>

  <li><strong>ID</strong></li>
  <li><strong>Class</strong></li>

</ul>

<p class="returnToSection">[ <a href="#formatting_your_web_pages">Return to beginning of section</a>
]</p>

</subsubsection><subsubsection><sectiontitle>Using Style Sheets for
Your Web Pages</sectiontitle>

</subsubsection><subsubsection><sectiontitle>Using Cascade Style Sheets</sectiontitle>

<p>Cascade Style Sheets(CSS) can be used to style an HTML document in
three ways:</p>
<ol>
  <li>Using inline styles.</li>

  <li>Using internal style sheet.</li>

  <li>Using external style sheets.</li>
</ol>

<p>Scientific WorkPlace has an inbuilt style sheet editor called
<strong>CaScadeS</strong>. CaScadeS can be used to produce either an
internal style sheet or an external one. As opposed to inline styles,
internal or external style sheets help to keep the content and style
information separate.</p>

<p>To style the html document you are editing, CaScadeS can be started
by clicking on Tools and choosing CSS Editor. CaScadeS allows two
modes of style sheet editing:</p>

<ol>
  <li><strong>Beginner Mode</strong>: This mode allows to create
  rules associated to class selectors or type element selectors.</li>

  <li><strong>Expert Mode</strong>: This mode allows to create rules
  without any restriction.</li>
</ol>

<p>In case there is no style sheet, a new style sheet will be
automatically created.</p>

<p class="returnToSection">[ <a href="#using_style_sheets_for_your_web_pages">Return to beginning of
section</a> ]</p>

</subsubsection><subsubsection><sectiontitle>Creating Style
Sheets with Scientific WorkPlace</sectiontitle>

<p>Using CaScadeS, one can create either internal style sheets or
external style sheets. To create an internal style sheet:</p>
<ol>
  <li>Click on Style elt. button
  <ul>
    <li>(Optional) Media list and Title can also be filled in.</li>
  </ul>
  </li>

  <li>Click on Create Stylesheet.</li>
</ol>

<p>To create an external stylesheet:</p>

<ol>
  <li>Click on Link elt. button</li>

  <li>Fill in the URL of the stylesheet in the right pane. A new file
  with be created on the local filesystem, if it doesnot already exist.

  <ul>
    <li>(Optional) Media list and Title can also be filled in.</li>
  </ul>
  </li>

  <li>Check the "check to create alternate stylesheet" if this is an
  alternate one.</li>
</ol>

<p><strong>Tip:</strong> Always save the html document before
attaching local style sheet.</p> <p><strong>Tip:</strong> Use Refresh
button in the left pane, if stylesheet is not immediately
downloaded.</p>

<p class="returnToSection">[ <a href="#using_style_sheets_for_your_web_pages">Return to beginning of
section</a> ]</p>

</subsubsection><subsubsection><sectiontitle>Creating Rules for
Stylesheets</sectiontitle>

<p>After creating one or more stylesheets for the html document, rules
can be created for each stylesheet individually. To use a particular
stylesheet for creating or modifying rules, highlight the stylesheet
in the left pane by clicking with left mouse button. The right pane
will show the detials of the stylesheet in the General Tab. To create
new rules:</p>

<ol>
  <li>Click on the Rule button in the left pane.</li>

  <li>Right pane shows options as to what kind of rule to
  create. Choose one of:

  <ul>
    <li>named style (enter class name below)</li>

    <li>style applied to all elements of type (enter type
    below)</li>

    <li>style applied to all elements matching the following
    selector</li>

  </ul>
  </li>
  <li>Fill in the name of the rule.</li>

  <li>Click on Create Style Rule button.</li>
</ol>

<p>Rules can be defined using the styling tabs (Text, Background,
Border, Box and Aural) in the right pane. To see all the definitions
of a style rule, highlight the rule in the left pane and click on
"General" tab in the right pane. The "General" tab shows all
definitions currently applied to the rule.</p>

<p class="returnToSection">[ <a href="#using_style_sheets_for_your_web_pages">Return to beginning of
section</a> ]</p>

</subsubsection></subsection><subsection><sectiontitle>Adding Tables to Your Web
Page</sectiontitle>

<subsubsection><sectiontitle>Inserting a Table</sectiontitle>

<p>Tables are useful for organizing text, pictures, and data into
formatted rows and
columns. To insert a table:</p>

<ol>
  <li>Click to place the insertion point where you want the table to
  appear.</li>

  <li>Click the Table button
  <img height="26" width="25" src="images/table.gif" alt=""/> on
  the
  Composition toolbar. The Insert Table dialog box appears.</li>

  <li>Type the number of rows and columns you want.
  <ul>
    <li>(Optional) Enter a size for the table width, and select
    either percentage of
    the window or pixels.</li>
  </ul>
  </li>

  <li>Enter a number for the border thickness (in pixels); enter zero
  for no border.
  <p><strong>Note</strong>: Scientific WorkPlace uses a red dotted line
  to indicate tables with a zero border;
  the dotted line disappears when the page is viewed in a
  browser.</p></li>

  <li>To apply additional table attributes or JavaScript, click
  Advanced Edit to display
  the <a href="#using_the_advanced_property_editor">Advanced
  Property Editor</a>.</li>

  <li>Click OK to confirm your settings and view your new table.</li>
</ol>

<p>To change additional properties for your new table, see
<a href="#changing_a_tables_properties">Changing a Table's
Properties</a>.</p>

<p><strong>Tip</strong>: To insert a table within a table, open the
Insert menu and choose Table.</p>

<p class="returnToSection">[ <a href="#adding_tables_to_your_web_page">Return to beginning of
section</a> ]</p>

</subsubsection><subsubsection><sectiontitle>Changing a Table's
Properties</sectiontitle>

<p>This section describes how to modify properties that apply to an
entire table as well
as the rows, columns, or individual cells within a table. If you are
not currently
viewing the Table Properties dialog box, follow these steps:</p>

<ol>
  <li>Select the table, or click anywhere inside it.</li>

  <li>Click the Table button
  <img height="26" width="25" src="images/table.gif" alt=""/> on
  the toolbar,
  or open the Table menu and choose Table Properties. The Table
  Properties dialog box
  contains two tabs: Table and Cells.</li>

  <li>Click the Table tab to edit these properties:
  <ul>
    <li><strong>Size</strong>: Use this to specify the number of
    rows and columns. Enter the width
    of the table and then choose "% of window" or "pixels." If
    you specify width as a
    percentage, the table's width changes whenever the
    Scientific WorkPlace window's or browser
    window's width changes.</li>

    <li><strong>Borders and Spacing</strong>: Use this to specify,
    in pixels, the border line width,
    the space between cells, and the cell padding (the space
    between the contents of the
    cell and its border).
    <p><strong>Note</strong>: Scientific WorkPlace uses a dotted
    outline to display tables with a zero border;
    the dotted line disappears when the page is viewed
    in a browser.</p></li>

    <li><strong>Table Alignment</strong>: Use this to align the
    table within the page. Choose an
    option from the drop-down list.</li>

    <li><strong>Caption</strong>: Choose the caption placement from
    the drop-down list.</li>

    <li><strong>Background Color</strong>: Use this to choose a
    color for the table background, or
    leave it as transparent.</li>

  </ul>
  </li>

  <li>To apply additional attributes or JavaScript events, click
  Advanced Edit to display
  the <a href="#formatting_paragaphs_headings_and_lists">Advanced
  Property Editor</a>.</li>

  <li>Click Apply to preview your changes without closing the dialog
  box, or click OK to
  confirm them.</li>
</ol>

<p>To view, change, or add properties for one or more cells:</p>

<ol>
  <li>Select the row, column, or cell, then open the Table menu and
  choose Table Properties.
  The Table Properties dialog box appears.</li>

  <li>Click the Cells tab to edit the following properties:
  <ul>
    <li><strong>Selection</strong>: Choose Cell, Row, or Column from
    the drop-down list. Click
    Previous or Next to move through rows, columns, or
    cells.</li>

    <li><strong>Size</strong>: Type a number for Height and Width,
    and then choose "% of table"
    or "pixels."</li>

    <li><strong>Content Alignment</strong>: Select a vertical and
    horizontal alignment type for
    the text or data inside each cell.</li>

    <li><strong>Cell Style</strong>: Select Header from the
    drop-down list for column or row
    headers (which centers and bolds the text in the cell);
    otherwise choose Normal.</li>

    <li><strong>Text Wrap</strong>: Select "Don't wrap" from the
    drop-down list to keep text from
    wrapping to the next line unless you insert a paragraph
    break. Otherwise, choose
    Wrap.</li>

    <li><strong>Background Color</strong>: Select a color for the
    cell background or leave it as
    transparent.
    <p><strong>Note</strong>: To apply additional attributes or
    JavaScript events, click Advanced
    Edit to display the
    <a href="#using_the_advanced_property_editor">Advanced Property
    Editor</a></p></li>
  </ul>
  </li>
  <li>Click Apply to preview your changes without closing the dialog
  box, or click OK to
  confirm them.</li>
</ol>

<p><strong>Tip</strong>: To change the text color or background color
of one or more selected cells or the entire table, select the cells or
click anywhere in the table and then click the text color or
background color icon in the Format toolbar.</p>

<p><strong>Tip</strong>: To change the color of cells to the color
last used, select the cell, then press Shift and click on the
background color picker. This is useful when you want to use one color
for individual cells.</p>

<p class="returnToSection">[ <a href="#adding_tables_to_your_web_page">Return to beginning of
section</a> ]</p>

</subsubsection><subsubsection><sectiontitle>Adding and
Deleting Rows, Columns, and Cells</sectiontitle>

<p>Scientific WorkPlace allows you to quickly add or delete one or more
cells, columns, or rows in a table. In addition, you can set options
that allow you to maintain the original rectangular structure or
layout of the table while you perform editing tasks.</p>

<p>To add a cell, row, or column to your table:</p>

<ol>
  <li>Click inside the table where you want to add a cell (or
  cells).</li>

  <li>Open the Table menu and then choose Insert.</li>

  <li>Choose one of the cell groupings. (You can also insert a new table
  within a table cell.)</li>
</ol>

<p>To delete a cell, row, or column:</p>

<ol>
  <li>Click a row, column, or cell to place the insertion
  point. Or, select neighboring cells to delete more than one row at a
  time. To select neighboring cells, drag over the cells you want to
  select.  To select individual cells in a table, hold down the Ctrl key
  (Windows, Linux or Unix) or the Command key (Mac OS) and click on the
  cells you want to select.</li>

  <li>Open the Table menu and choose Delete.</li>

  <li>Choose the item you want to delete.</li>
</ol>

<p>To join (or merge) a cell with the cell on its right:</p>

<ul>
  <li>Click inside the cell on the left, open the Table menu, and
  choose Join with Cell to the Right.</li>
</ul>

<p>To join (or merge) adjacent cells:</p>

<ul>
  <li>Select adjacent cells by dragging over them.</li>

  <li>Open the Table menu, and choose Join Selected Cells.</li> 
</ul>

<p>To split a joined cell back into two or more cells:</p>

<ul>
  <li>Click inside the joined cell, open the Table menu, and then
  choose Split Cell. Scientific WorkPlace puts the entire contents of the
  joined cell into the first of the two cells.</li>
</ul>

<p>Refer to <a href="#selecting_table_elements">Selecting Table
Elements</a> for information on how to select non-adjacent cells,
rows, and columns.</p>

</subsubsection><subsubsection><sectiontitle>Changing
the Default Table Editing Behavior</sectiontitle>

<p>By default, when you delete one or more cells, Scientific WorkPlace
preserves the table's structure by adding cells at the end of a row,
wherever needed. This allows you to delete one or more cells but still
maintain the table's original rectangular layout, or
structure. Otherwise, deleting cells can result in a table with empty
spaces, or whose outline appears irregular due to an uneven number of
cells.</p>

<p>To change the default table editing behavior, begin from the
Scientific WorkPlace window:</p>

<ol>
  <li>Open the Edit menu (Mozilla menu on Mac OS X), choose
  Preferences, and then choose Scientific WorkPlace.</li>

  <li>Under Table Editing, set the following preference:

  <ul>
    <li>Make sure that "Maintain table layout when inserting or
    deleting cells" is checked to ensure that you don't get an irregularly
    shaped table.</li>
  </ul>
  </li>

  <li>Click OK.</li> 
</ol>

<p>See also <a href="#composer_preferences_composer">Setting General
Scientific WorkPlace Preferences</a>.</p>

<p class="returnToSection">[ <a href="#adding_tables_to_your_web_page">Return to beginning of
section</a> ]</p>

</subsubsection><subsubsection><sectiontitle>Selecting Table Elements</sectiontitle>

<p>You can use one of two ways to quickly select a table, cell, or
group of cells:</p>

<ul>
  <li>Click in the table, open the Table menu, choose Select, and
  then choose an item from the submenu. For example, to select a table,
  click anywhere inside the table, open the Table menu, choose Select,
  and then choose Table.</li>

  <li>Or, you can use the mouse as a selection tool:</li>

  <li style="list-style-type: none; list-style-position: outside; list-style-image: none;"> <ul> <li>To select a group of adjacent
      cells: click in a cell, and then drag to select the cells you
      want. Drag the mouse left or right to select a row; up or down to
      select a column.</li>

      <li>To select non-adjacent cells: press Ctrl (Windows, Linux or Unix)
      or Command (Mac OS) and then click inside a cell. Keep pressing Ctrl
      (Windows, Linux or Unix) or Command (Mac OS) as you click to select
      additional cells.</li>

      <li>To extend a selection to include adjacent cells: click inside a
      cell and then drag over additional cells to extend the selection.</li>

      <li>To select one or more adjacent columns or rows: drag up or down to
      select the first column or row, and then drag left or right to select
      additional adjacent columns or rows. Press Shift and drag to the right
      to select an entire row. Press Shift and drag up or down to select an
      entire column.</li>
      </ul>
  </li>
</ul>

<p class="returnToSection">[ <a href="#adding_tables_to_your_web_page">Return to beginning of
section</a> ]</p>

</subsubsection><subsubsection><sectiontitle>Moving, Copying, and
Deleting Tables</sectiontitle>

<p>To move a table:</p>

<ol>
  <li>Click inside the table.</li>

  <li>Open the Table menu, choose Select, and then choose Table.</li>
</ol>

<ul> 
  <li>To copy or move the table: Use the Edit menu's cut, copy, and
  paste options.</li>

  <li>To delete the table: Open the Table menu again, choose Delete, and
  then choose Table.</li>
</ul>

</subsubsection><subsubsection><sectiontitle>Converting Text into a
Table</sectiontitle>

<p>To convert text into a table:</p>

<ol>
  <li>Select the text that you want to convert into a table. Keep
  in mind that Scientific WorkPlace creates a new table row for each
  paragraph in the selection.</li>

  <li>Open the Table menu and choose Create Table from Selection. You
  see the Convert to Table dialog box.</li>

  <li>Choose the character Scientific WorkPlace uses to separate the
  selection into columns, or specify a different character to use. If
  you choose Space as the separator for columns, choose whether or not
  you want Scientific WorkPlace to ignore multiple space and treat them as
  one space.</li>

  <li>Leave "Delete separator character" checked to have
  Scientific WorkPlace remove the separator character when it converts the
  text into a table. If you don't want Scientific WorkPlace to delete the
  separator character, uncheck this option.</li>

  <li>Click OK.</li>
</ol>

<p><strong>Note</strong>: Text formatting is removed when the selected
text is converted to a table.</p>

<p class="returnToSection">[ <a href="#adding_tables_to_your_web_page">Return to beginning of
section</a> ]</p>

</subsubsection></subsection><subsection><sectiontitle>Adding Pictures (Images) to
Your Web Page</sectiontitle>

<subsubsection><sectiontitle>Inserting an Image into
Your Page</sectiontitle>

<p>You can insert GIF, JPEG, BMP, and PNG (Portable Network Graphics)
images into your web page. You can also use them to <a href="#using_images_as_links">create links</a>. When you insert an
image, Scientific WorkPlace saves a reference to the image in your
page.</p>

<p><strong>Note</strong>: If you plan to publish your pages to the
web, it's best not to use BMP images in your pages.</p>

<p><strong>Tip</strong>: It's best to first save or publish your page
before you insert images into it. This allows Scientific WorkPlace to
automatically use relative references to images once you insert
them.</p>

<p>To insert an image:</p>

<ol>
  <li>Click to place the insertion point where you want the image
  to appear.</li>

  <li>Click the Image button <img height="25" width="23" src="images/image.gif" alt=""/> on the toolbar, or open the Insert menu and
  choose Image. You see the Image Properties dialog box.</li>

  <li>Type the location and filename of the image file, or click Choose
  File to search for an image file on your hard drive or network.</li>

  <li>Type a simple description of your image as the alternate text that
  will appear in text-only browsers (as well as other browsers)
  when an image is loading or when image loading is disabled.

  <p>Alternatively, you can choose not to include alternate text.</p>
  </li>

  <li>If needed, click other tabs so you can adjust the settings (for
  example, alignment) in the <a href="#editing_image_properties">Image
Properties</a> dialog box.</li> </ol>

<p><strong>Tip</strong>: To quickly insert an image: Drag and drop it
onto your page.</p>

<p><strong>Tip</strong>: To insert a line break after all images in a
paragraph, choose Break Below Images from the Insert menu.</p>

<p class="returnToSection">[ <a href="#adding_pictures_to_your_web_page">Return to beginning of
section</a> ]</p>

</subsubsection><subsubsection><sectiontitle>Editing Image Properties</sectiontitle>

<p>Once you've inserted an image into your page, you can edit its
properties and customize
the layout in your page, such as the height, width, spacing, and
text alignment. If you
are not currently viewing the Image Properties dialog box, follow
these steps:</p>

<p>To edit the properties for a selected image:</p>

<ol>

  <li>Double-click the image, or select it and click the Image button
  <img height="25" width="23" src="images/image.gif" alt=""/> on
  the toolbar
  to display the Image Properties dialog box.</li>

  <li>Click the Location tab to edit these properties:

  <ul>
    <li><strong>Image Location</strong>: Type the filename and
    location of the image file. Click
    Choose File to search for an image file on your hard drive
    or network.</li>
    
    <li><strong>URL is relative to page location</strong>: If checked,
    Scientific WorkPlace converts the URL to
    be relative to the page's location. This is especially
    useful if you plan to publish
    your pages on a web server so that others can view
    them. Using relative URLs allows
    you to keep all your linked files in the same place relative
    to each other, regardless
    of their location on your hard disk or a web server.
    <p>Unchecking this box causes Scientific WorkPlace to convert the
    URL to a full (absolute) URL.
    You typically use absolute URLs when linking to images on
    other web servers
    (not stored locally on your hard disk).</p>
    <p>If you have never saved or published the page, you must first
    save the page in
    order to enable this checkbox. (This checkbox is not
    available if you open the Image
    Properties dialog box in a message compose
    window.)</p></li>

    <li><strong>Alternate Text</strong>: Enter text that will display
    in place of the original image;
    for example, a caption or a brief description of the
    image. It's a good practice to
    specify alternate text for readers who use text-only web
    browsers or who have image
    loading turned off.</li>
    
    <li><strong>Don't use alternate text</strong>: Choose this option
    if the image does not
    require alternate text or if you don't want to include
    it.</li>
    
  </ul>
  </li>
  <li>Click the Dimensions tab to edit these properties:
  <ul>
    <li><strong>Actual Size</strong>: Select this option to undo any
    changes you've made to the
    dimensions and return the image to its original size.</li>
    
    <li><strong>Custom Size</strong>: Select this option and specify
    the new height and width, in
    pixels or as a percentage. This setting doesn't affect the
    original image file,
    just the image inserted in your page.</li>
    
    <li><strong>Constrain</strong>: If you change the image size,
    it's a good idea to select this
    in order to maintain the image's aspect ratio (so that it
    doesn't appear distorted).
    If you choose this option, then you only need to
    change the height or width, but not
    both.</li>
    
  </ul>
  </li>
  <li>Click the Appearance Tab to edit these properties:
  <ul>
    <li><strong>Spacing</strong>: Specify the amount of space
    surrounding the image; between the
    image and adjoining text. You can also put a solid black
    border around the image
    and specify its width in pixels. Specify zero for no
    border.</li>

    <li><strong>Align Text to Image</strong>: If you've placed your
    image next to any text, select
    an alignment icon to indicate how you want text positioned
    relative to the image.</li>

    <li><strong>Image Map</strong>: Click Remove to remove any image
    map settings.</li>

  </ul>
  </li>
  <li>Click the Link tab to edit these properties:
  <ul>
    <li><strong>Enter a web page location</strong>: If you want to
    define a link for this image,
    enter the URL of a remote or local page, or select a named
    anchor or heading from
    the drop-down list. Click Choose File to search for an
    image file on your hard
    drive or network.</li>
    
    <li><strong>URL is relative to page location</strong>: If
    checked, Scientific WorkPlace converts the URL to
    be relative to the page's location. This is especially
    useful if you plan to publish
    your pages to a web server so that others can view
    them. Using relative URLs allows
    you to keep all your linked files in the same place
    relative to each other,
    regardless of their location on your hard disk or a
    web server.
    <p>Unchecking this box causes Scientific WorkPlace to convert the
    URL to a full (absolute) URL.
    You typically use absolute URLs when linking to
    images on other web servers (not
    stored locally on your hard disk).</p>
    <p>If you have unsaved changes, you must first save the page
    in order to enable this
    checkbox. (This checkbox is not available if you
    open the Image Properties dialog
    box in a message compose window.)</p></li>

    <li><strong>Show border around linked image</strong>: If
    checked, displays the link
    highlight color around the image.</li>
  </ul>
  </li>
  
  <li>To apply additional attributes or JavaScript events, click
  Advanced Edit to display
  the <a href="#using_the_advanced_property_editor">Advanced
  Property Editor</a>.</li>

  <li>Click OK to confirm your changes.</li>
</ol>

<p class="returnToSection">[ <a href="#adding_pictures_to_your_web_page">Return to beginning of
section</a> ]</p><br/></subsubsection></subsection></section></body>
</html>