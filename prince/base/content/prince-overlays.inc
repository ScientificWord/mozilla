<?xul-overlay href="chrome://prince/content/msiEditorContextMenuOverlay.xul"?>
<?xul-overlay href="chrome://global/content/globalOverlay.xul"?>
<?xul-overlay href="chrome://prince/content/menuOverlay.xul" ?>
##<?xul-overlay href="chrome://prince/content/basemenuOverlay.xul" ?>
<?xul-overlay href="chrome://prince/content/toolbarsOverlay.xul" ?>
<?xul-overlay href="chrome://prince/content/MathmlOverlay.xul" ?>
##ifdef SWP
<?xul-overlay href="chrome://prince/content/computeOverlay.xul" ?>
##else
##ifdef SNB
#<?xul-overlay href="chrome://prince/content/computeOverlay.xul" ?>
##endif
##ifdef SWP
<?xul-overlay href="chrome://prince/content/typesetOverlay.xul" ?>
##else
##ifdef SW
#<?xul-overlay href="chrome://prince/content/typesetOverlay.xul" ?>
##endif
<?xul-overlay href="chrome://cascades/content/cascadesOverlay.xul" ?>
