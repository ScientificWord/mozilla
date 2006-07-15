/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Java XPCOM Bindings.
 *
 * The Initial Developer of the Original Code is
 * IBM Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * IBM Corporation. All Rights Reserved.
 *
 * Contributor(s):
 *   Javier Pedemonte (jhpedemonte@gmail.com)
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

package org.mozilla.xpcom;

import java.io.*;
import java.lang.reflect.*;
import java.net.*;
import java.nio.charset.Charset;
import java.util.*;


/**
 * @see http://www.mozilla.org/projects/embedding/GRE.html
 */
public class Mozilla implements IGRE, IXPCOM, IXPCOMError {

  private static Mozilla mozillaInstance = new Mozilla();

  private static final String JAVAXPCOM_JAR = "javaxpcom.jar";

  private IGRE gre = null;

  private IXPCOM xpcom = null;

  /**
   * @return
   */
  public static Mozilla getInstance() {
    return mozillaInstance;
  }

  /**
   * 
   */
  private Mozilla() {
  }

  /**
   * Locates the path of a GRE with the specified properties.  This method
   * will only return GREs that support Java embedding (looks for the
   * presence of "javaxpcom.jar").
   * <p>
   * Currently this uses a "first-fit" algorithm, it does not select
   * the newest available GRE.
   * 
   * @param aVersions        An array of version ranges: if any version range
   *                         matches, the GRE is considered acceptable.
   * @param aProperties      A list of GRE property/value pairs which must
   *                         all be satisfied.  This parameter is ignored on
   *                         Macintosh, because of the manner in which the
   *                         XUL frameworks are installed.
   *
   * @return                 A file object of the appropriate path. If
   *                         the "local" GRE is specified (via the USE_LOCAL_GRE
   *                         environment variable, for example), returns
   *                         <code>null</code>.
   *
   * @throws FileNotFoundException if an appropriate GRE could not be found
   */
  public static File getGREPathWithProperties(GREVersionRange[] aVersions,
          Properties aProperties) throws FileNotFoundException {
    File grePath = null;

    // if GRE_HOME is in the environment, use that GRE
    String env = System.getProperty("GRE_HOME");
    if (env != null) {
      try {
        grePath = new File(env).getCanonicalFile();
      } catch (IOException e) {
        throw new FileNotFoundException("cannot access GRE_HOME");
      }
      if (!grePath.exists()) {
        throw new FileNotFoundException("GRE_HOME doesn't exist");
      }
      return grePath;
    }

    // the Gecko bits that sit next to the application or in the PATH
    env = System.getProperty("USE_LOCAL_GRE");
    if (env != null) {
      return null;
    }

    // Search for GRE in platform specific locations.  We want a GRE that
    // supports Java, so we look for the "javaxpcom" property by default.
    if (aProperties == null) {
      aProperties = new Properties();
    }
    aProperties.setProperty("javaxpcom", "1");

    String osName = System.getProperty("os.name").toLowerCase();
    if (osName.startsWith("mac os x")) {
      grePath = getGREPathMacOSX(aVersions);
    } else if (osName.startsWith("windows")) {
      grePath = getGREPathWindows(aVersions, aProperties);
    } else {
      // assume everything else is Unix/Linux
      grePath = getGREPathUnix(aVersions, aProperties);
    }

    if (grePath == null) {
      throw new FileNotFoundException("GRE not found");
    }

    return grePath;
  }

  /**
   * @param aVersions
   * @return
   */
  private static File getGREPathMacOSX(GREVersionRange[] aVersions) {
    /*
     * Check the application bundle first, for
     * <bundle>/Contents/Frameworks/XUL.framework/libxpcom.dylib.
     */
    File grePath = findGREBundleFramework();
    if (grePath != null) {
      return grePath;
    }

    // Check ~/Library/Frameworks/XUL.framework/Versions/<version>/libxpcom.dylib
    String home = System.getProperty("user.home");
    if (home != null) {
      grePath = findGREFramework(home, aVersions);
      if (grePath != null) {
        return grePath;
      }
    }

    // Check /Library/Frameworks/XUL.framework/Versions/<version>/libxpcom.dylib
    return findGREFramework("", aVersions);
  }

  /**
   * @return
   */
  private static File findGREBundleFramework() {
    /*
     * Use reflection to get Apple's NSBundle class, which can be used
     * to get the bundle's "Frameworks" directory.
     */
    try {
      URL[] urls = new URL[1];
      urls[0] = new File("/System/Library/Java/").toURL();
      ClassLoader loader = new URLClassLoader(urls);
      Class bundleClass = Class.forName("com.apple.cocoa.foundation.NSBundle",
                                        true, loader);

      // Get the bundle for this app.  If this is not executing from
      // a bundle, this will return null.
      Method mainBundleMethod = bundleClass.getMethod("mainBundle", null);
      Object bundle = mainBundleMethod.invoke(null, null);

      if (bundle != null) {
        // Get the path to the bundle's "Frameworks" directory
        Method fwPathMethod = bundleClass.getMethod("privateFrameworksPath",
                                                    null);
        String path = (String) fwPathMethod.invoke(bundle, null);

        // look for libxpcom.dylib
        if (path.length() != 0) {
          File xulDir = new File(path, "XUL.framework");
          if (xulDir.isDirectory()) {
            File xpcomLib = new File(xulDir, "libxpcom.dylib");
            if (xpcomLib.canRead()) {
              File grePath = xpcomLib.getCanonicalFile().getParentFile();

              // Since GRE Properties aren't supported on Mac OS X, we check
              // for the existence of the "javaxpcom.jar" file in the GRE.
              File jar = new File(grePath, JAVAXPCOM_JAR);
              if (jar.canRead()) {
                // found GRE
                return grePath;
              }
            }
          }
        }
      }
    } catch (Exception e) { }

    return null;
  }

  /**
   * @param aRootPath
   * @param aVersions
   * @return
   */
  private static File findGREFramework(String aRootPath,
                                       GREVersionRange[] aVersions) {
    File frameworkDir = new File(aRootPath +
                                 "/Library/Frameworks/XUL.framework/Versions");
    if (!frameworkDir.exists())
      return null;

    File[] files = frameworkDir.listFiles();
    for (int i = 0; i < files.length; i++) {
      if (checkVersion(files[i].getName(), aVersions)) {
        File xpcomLib = new File(files[i], "libxpcom.dylib");

        // Since GRE Properties aren't supported on Mac OS X, we check
        // for the existence of the "javaxpcom.jar" file in the GRE.
        File jar = new File(files[i], JAVAXPCOM_JAR);
        if (xpcomLib.canRead() && jar.canRead()) {
          return files[i];
        }
      }
    }

    return null;
  }

  /**
   * @param aVersions
   * @param aProperties
   * @return
   */
  private static File getGREPathWindows(GREVersionRange[] aVersions,
                                        Properties aProperties) {
    /*
     * Note the usage of the "Software\\mozilla.org\\GRE" subkey - this allows
     * us to have multiple versions of GREs on the same machine by having
     * subkeys such as 1.0, 1.1, 2.0 etc. under it.
     *
     * Please see http://www.mozilla.org/projects/embedding/GRE.html for
     * more info.
     */

    final String greKey = "Software\\mozilla.org\\GRE";

    // See if there is a GRE registered for the current user.
    // If not, look for one on the system.
    String key = "HKEY_CURRENT_USER" + "\\" + greKey;
    File grePath = getGREPathFromRegKey(key, aVersions, aProperties);
    if (grePath == null) {
      key = "HKEY_LOCAL_MACHINE" + "\\" + greKey;
      grePath = getGREPathFromRegKey(key, aVersions, aProperties);
    }

    return grePath;
  }

  /**
   * @param aRegKey
   * @param aVersions
   * @param aProperties
   * @return
   */
  private static File getGREPathFromRegKey(String aRegKey,
          GREVersionRange[] aVersions, Properties aProperties) {
    // create a temp file for the registry export
    File tempFile;
    try {
      tempFile = File.createTempFile("jx_registry", null);
    } catch (IOException e) {
      // failed to create temp file.  ABORT
      return null;
    }

    Process proc;
    try {
      proc = Runtime.getRuntime().exec("regedit /e " + tempFile.getPath() +
                                       " \"" + aRegKey + "\"");
      proc.waitFor();
    } catch (Exception e) {
      // Failed to run regedit.exe.  Length of temp file is zero, and that's
      // handled next.
    }

    // If there is a key by that name in the registry, then the file length
    // will not be zero.
    File grePath = null;
    if (tempFile.length() != 0) {
      grePath = getGREPathFromRegistryFile(tempFile.getPath(),
              aRegKey, aVersions, aProperties);
    }

    tempFile.delete();
    return grePath;
  }

  /**
   * @param aFileName
   * @param aCharset
   * @param aKeyName
   * @param aVersions
   * @param aProperties
   * @return
   */
  private static File getGREPathFromRegistryFile(String aFileName,
          String aKeyName, GREVersionRange[] aVersions,
          Properties aProperties) {
    INIParser parser;
    try {
      parser = new INIParser(aFileName, Charset.forName("UTF-16"));
    } catch (Exception e) {
      // Problem reading from file.  Bail out.
      return null;
    }

    Iterator sectionsIter = parser.getSections();
    while (sectionsIter.hasNext()) {
      // get 'section' name, which will be a registry key name
      String section = (String) sectionsIter.next();

      // Skip over GRE key ("<root>\Software\mozilla.org\GRE")
      int gre_len = aKeyName.length();
      if (section.length() <= gre_len) {
        continue;
      }

      // Get the GRE subkey;  that is, everything after
      // "<root>\Software\mozilla.org\GRE\"
      String subkeyName = section.substring(gre_len + 1);

      // We are only interested in _immediate_ subkeys.  We want
      // "<root>\Software\mozilla.org\GRE\<version>" but not
      // "<root>\Software\mozilla.org\GRE\<version>\<moretext>".
      if (subkeyName.indexOf('\\') != -1) {
        continue;
      }

      // See if this registry key has a "Version" value, and if so, compare
      // it to our desired versions.
      String version = parser.getString(section, "\"Version\"");
      if (version == null) {
        continue;
      }
      // remove quotes around string
      version = version.substring(1, version.length() - 1);
      if (!checkVersion(version, aVersions)) {
        continue;
      }

      // All properties must match, keeping in mind that the propery/value
      // pairs returned by regedit.exe have quotes around them.
      if (aProperties != null) {
        boolean ok = true;
        Enumeration e = aProperties.propertyNames();
        while (ok && e.hasMoreElements()) {
          String prop = (String) e.nextElement();
          String greValue = parser.getString(section, "\"" + prop + "\"");
          if (greValue == null) {
            // No such property is set for this GRE. Go on to next GRE.
            ok = false;
          } else  {
            // See if the value of the property for the GRE matches
            // the given value.
            String value = aProperties.getProperty(prop);
            if (!greValue.equals("\"" + value + "\"")) {
              ok = false;
            }
          }
        }
        if (!ok) {
          continue;
        }
      }

      String pathStr = parser.getString(section, "\"GreHome\"");
      if (pathStr != null) {
        // remove quotes around string
        pathStr = pathStr.substring(1, pathStr.length() - 1);
        File grePath = new File(pathStr);
        if (grePath.exists()) {
          File xpcomLib = new File(grePath, "xpcom.dll");
          if (xpcomLib.canRead()) {
            // found a good GRE
            return grePath;
          }
        }
      }
    }

    return null;
  }

  /**
   * @param aVersions
   * @param aProperties
   * @return
   */
  private static File getGREPathUnix(GREVersionRange[] aVersions,
                                     Properties aProperties) {
    File grePath = null;

    String env = System.getProperty("MOZ_GRE_CONF");
    if (env != null) {
      grePath = getPathFromConfigFile(env, aVersions, aProperties);
      if (grePath != null) {
        return grePath;
      }
    }

    final String greUserConfFile = ".gre.config";
    final String greUserConfDir = ".gre.d";
    final String greConfPath = "/etc/gre.conf";
    final String greConfDir = "/etc/gre.d";

    env = System.getProperty("user.home");
    if (env != null) {
      // Look in ~/.gre.config
      grePath = getPathFromConfigFile(env + File.separator + greUserConfFile,
                                      aVersions, aProperties);
      if (grePath != null) {
        return grePath;
      }

      // Look in ~/.gre.d/*.conf
      grePath = getPathFromConfigDir(env + File.separator + greUserConfDir,
                                     aVersions, aProperties);
      if (grePath != null) {
        return grePath;
      }
    }

    // Look for a global /etc/gre.conf file
    grePath = getPathFromConfigFile(greConfPath, aVersions, aProperties);
    if (grePath != null) {
      return grePath;
    }

    // Look for a group of config files in /etc/gre.d/
    grePath = getPathFromConfigDir(greConfDir, aVersions, aProperties);
    return grePath;
  }

  /**
   * @param aFileName
   * @param aVersions
   * @param aProperties
   * @return
   */
  private static File getPathFromConfigFile(String aFileName,
          GREVersionRange[] aVersions, Properties aProperties) {
    INIParser parser;
    try {
      parser = new INIParser(aFileName);
    } catch (Exception e) {
      // Problem reading from file.  Bail out.
      return null;
    }

    Iterator sectionsIter = parser.getSections();
    while (sectionsIter.hasNext()) {
      // get 'section' name, which will be a version string
      String section = (String) sectionsIter.next();

      // if this isn't one of the versions we are looking for, move
      // on to next section
      if (!checkVersion(section, aVersions)) {
        continue;
      }

      // all properties must match
      if (aProperties != null) {
        boolean ok = true;
        Enumeration e = aProperties.propertyNames();
        while (ok && e.hasMoreElements()) {
          String prop = (String) e.nextElement();
          String greValue = parser.getString(section, prop);
          if (greValue == null) {
            // No such property is set for this GRE. Go on to next GRE.
            ok = false;
          } else  {
            // See if the value of the property for the GRE matches
            // the given value.
            if (!greValue.equals(aProperties.getProperty(prop))) {
              ok = false;
            }
          }
        }
        if (!ok) {
          continue;
        }
      }

      String pathStr = parser.getString(section, "GRE_PATH");
      if (pathStr != null) {
        File grePath = new File(pathStr);
        if (grePath.exists()) {
          File xpcomLib = new File(grePath, "libxpcom.so");
          if (xpcomLib.canRead()) {
            // found a good GRE
            return grePath;
          }
        }
      }
    }

    return null;
  }

  /**
   * @param aDirName
   * @param aVersions
   * @param aProperties
   * @return
   */
  private static File getPathFromConfigDir(String aDirName,
          GREVersionRange[] aVersions, Properties aProperties) {
    /*
     * Open the directory provided and try to read any files in that
     * directory that end with .conf.  We look for an entry that might
     * point to the GRE that we're interested in.
     */

    File dir = new File(aDirName);
    if (!dir.isDirectory()) {
      return null;
    }

    File grePath = null;
    File[] files = dir.listFiles();
    for (int i = 0; i < files.length && grePath == null; i++) {
      // only look for files that end in '.conf'
      if (!files[i].getName().endsWith(".conf")) {
        continue;
      }

      grePath = getPathFromConfigFile(files[i].getPath(), aVersions,
                                      aProperties);
    }

    return grePath;
  }

  /**
   * @param aVersionToCheck
   * @param aVersions
   * @return
   */
  private static boolean checkVersion(String aVersionToCheck,
                                      GREVersionRange[] aVersions) {
    for (int i = 0; i < aVersions.length; i++) {
      if (aVersions[i].check(aVersionToCheck)) {
        return true;
      }
    }
    return false;
  }

  /**
   * Initializes libXUL for embedding purposes.
   * <p>
   * NOTE: This function must be called from the "main" thread.
   * <p>
   * NOTE: At the present time, this function may only be called once in
   *       a given process. Use <code>termEmbedding</code> to clean up and free
   *       resources allocated by <code>initEmbedding</code>.
   *
   * @param aLibXULDirectory   The directory in which the libXUL shared library
   *                           was found.
   * @param aAppDirectory      The directory in which the application components
   *                           and resources can be found. This will map to
   *                           the "resource:app" directory service key.
   * @param aAppDirProvider    A directory provider for the application. This
   *                           provider will be aggregated by a libXUL provider
   *                           which will provide the base required GRE keys.
   *
   * @throws IllegalArgumentException  if <code>aLibXULDirectory</code> is not
   *                           a valid path
   * @throws XPCOMException  if a failure occurred during initialization
   */
  public void initEmbedding(File aLibXULDirectory, File aAppDirectory,
          IAppFileLocProvider aAppDirProvider) throws XPCOMException {
    loadJavaXPCOM(aLibXULDirectory, true);
    gre.initEmbedding(aLibXULDirectory, aAppDirectory, aAppDirProvider);
  }

  /**
   * @param aLibXULDirectory
   * @param aLoadGREImpl
   * @throws XPCOMException
   */
  private void loadJavaXPCOM(File aLibXULDirectory, boolean aLoadGREImpl)
          throws XPCOMException {
    File jar = new File(aLibXULDirectory, JAVAXPCOM_JAR);
    if (!jar.exists()) {
      throw new XPCOMException(NS_ERROR_FILE_INVALID_PATH);
    }

    URL[] urls = new URL[1];
    try {
      urls[0] = jar.toURL();
    } catch (MalformedURLException e) {
      throw new XPCOMException(NS_ERROR_FILE_INVALID_PATH);
    }
    ClassLoader loader = new URLClassLoader(urls,
            this.getClass().getClassLoader());

    try {
      if (aLoadGREImpl) {
        Class greClass = Class.forName("org.mozilla.xpcom.internal.GREImpl",
                                       true, loader);
        gre = (IGRE) greClass.newInstance();
      }
      Class xpcomClass = Class.forName("org.mozilla.xpcom.internal.XPCOMImpl",
                                       true, loader);
      xpcom = (IXPCOM) xpcomClass.newInstance();
    } catch (Exception e) {
      throw new XPCOMException(NS_ERROR_FAILURE,
              "failure creating org.mozilla.xpcom.internal.*");
    }
  }

  /**
   * Terminates libXUL embedding.
   * <p>
   * NOTE: Release any references to XPCOM objects that you may be holding
   *       before calling this function.
   */
  public void termEmbedding() throws XPCOMException {
    try {
      gre.termEmbedding();
    } catch (NullPointerException e) {
      throw new XPCOMException(Mozilla.NS_ERROR_NULL_POINTER,
          "Attempt to use unitialized GRE object");
    } finally {
      gre = null;
      xpcom = null;
    }
  }

  /**
   * Initializes XPCOM. You must call this method before proceeding
   * to use XPCOM.
   *
   * @param aMozBinDirectory The directory containing the component
   *                         registry and runtime libraries;
   *                         or use <code>null</code> to use the working
   *                         directory.
   *
   * @param aAppFileLocProvider The object to be used by Gecko that specifies
   *                         to Gecko where to find profiles, the component
   *                         registry preferences and so on; or use
   *                         <code>null</code> for the default behaviour.
   *
   * @return the service manager
   *
   * @exception XPCOMException <ul>
   *      <li> NS_ERROR_NOT_INITIALIZED - if static globals were not initialied,
   *            which can happen if XPCOM is reloaded, but did not completly
   *            shutdown. </li>
   *      <li> Other error codes indicate a failure during initialisation. </li>
   * </ul>
   */
  public nsIServiceManager initXPCOM(File aMozBinDirectory,
          IAppFileLocProvider aAppFileLocProvider) throws XPCOMException {
    loadJavaXPCOM(aMozBinDirectory, false);
    return xpcom.initXPCOM(aMozBinDirectory, aAppFileLocProvider);
  }

  /**
   * Shutdown XPCOM. You must call this method after you are finished
   * using xpcom.
   *
   * @param aServMgr    The service manager which was returned by initXPCOM.
   *                    This will release servMgr.
   *
   * @exception XPCOMException  if a failure occurred during termination
   */
  public void shutdownXPCOM(nsIServiceManager aServMgr) throws XPCOMException {
    try {
      xpcom.shutdownXPCOM(aServMgr);
    } catch (NullPointerException e) {
      throw new XPCOMException(Mozilla.NS_ERROR_NULL_POINTER,
          "Attempt to use unitialized XPCOM object");
    } finally {
      xpcom = null;
    }
  }

  /**
   * Public Method to access to the service manager.
   *
   * @return the service manager
   *
   * @exception XPCOMException
   */
  public nsIServiceManager getServiceManager() throws XPCOMException {
    try {
      return xpcom.getServiceManager();
    } catch (NullPointerException e) {
      throw new XPCOMException(Mozilla.NS_ERROR_NULL_POINTER,
          "Attempt to use unitialized XPCOM object");
    }
  }

  /**
   * Public Method to access to the component manager.
   *
   * @return the component manager
   *
   * @exception XPCOMException
   */
  public nsIComponentManager getComponentManager() throws XPCOMException {
    try {
      return xpcom.getComponentManager();
    } catch (NullPointerException e) {
      throw new XPCOMException(Mozilla.NS_ERROR_NULL_POINTER,
          "Attempt to use unitialized XPCOM object");
    }
  }

  /**
   * Public Method to access to the component registration manager.
   * 
   * @return the component registration manager
   *
   * @exception XPCOMException
   */
  public nsIComponentRegistrar getComponentRegistrar() throws XPCOMException {
    try {
      return xpcom.getComponentRegistrar();
    } catch (NullPointerException e) {
      throw new XPCOMException(Mozilla.NS_ERROR_NULL_POINTER,
          "Attempt to use unitialized XPCOM object");
    }
  }

  /**
   * Public Method to create an instance of a nsILocalFile.
   *
   * @param aPath         A string which specifies a full file path to a 
   *                      location.  Relative paths will be treated as an
   *                      error (NS_ERROR_FILE_UNRECOGNIZED_PATH).
   * @param aFollowLinks  This attribute will determine if the nsLocalFile will
   *                      auto resolve symbolic links.  By default, this value
   *                      will be false on all non unix systems.  On unix, this
   *                      attribute is effectively a noop.
   *
   * @return an instance of an nsILocalFile that points to given path
   *
   * @exception XPCOMException <ul>
   *      <li> NS_ERROR_FILE_UNRECOGNIZED_PATH - raised for unrecognized paths
   *           or relative paths (must supply full file path) </li>
   * </ul>
   */
  public nsILocalFile newLocalFile(String aPath, boolean aFollowLinks)
          throws XPCOMException {
    try {
      return xpcom.newLocalFile(aPath, aFollowLinks);
    } catch (NullPointerException e) {
      throw new XPCOMException(Mozilla.NS_ERROR_NULL_POINTER,
          "Attempt to use unitialized XPCOM object");
    }
  }

  /**
   * If you create a class that implements nsISupports, you will need to provide
   * an implementation of the <code>queryInterface</code> method.  This helper
   * function provides a simple implementation.  Therefore, if your class does
   * not need to do anything special with <code>queryInterface</code>, your
   * implementation would look like:
   * <pre>
   *      public nsISupports queryInterface(String aIID) {
   *        return XPCOM.queryInterface(this, aIID);
   *      }
   * </pre>
   *
   * @param aObject object to query
   * @param aIID    requested interface IID
   *
   * @return        <code>aObject</code> if the given object supports that
   *                interface;
   *                <code>null</code> otherwise.
   */
  public static nsISupports queryInterface(nsISupports aObject, String aIID) {
    ArrayList classes = new ArrayList();
    classes.add(aObject.getClass());

    while (!classes.isEmpty()) {
      Class clazz = (Class) classes.remove(0);

      // Skip over any class/interface in the "java.*" and "javax.*" domains.
      String className = clazz.getName();
      if (className.startsWith("java.") || className.startsWith("javax.")) {
        continue;
      }

      // If given IID matches that of the current interface, then we
      // know that aObject implements the interface specified by the given IID.
      if (clazz.isInterface() && className.startsWith("org.mozilla")) {
        String iid = Mozilla.getInterfaceIID(clazz);
        if (iid != null && aIID.equals(iid)) {
          return aObject;
        }
      }

      // clazz didn't match, so add the interfaces it implements
      Class[] interfaces = clazz.getInterfaces();
      for (int i = 0; i < interfaces.length; i++ ) {
        classes.add(interfaces[i]);
      }

      // Also add its superclass
      Class superclass = clazz.getSuperclass();
      if (superclass != null) {
        classes.add(superclass);
      }
    }

    return null;
  }

  /**
   * Gets the interface IID for a particular Java interface.  This is similar
   * to NS_GET_IID in the C++ Mozilla files.
   *
   * @param aInterface  interface which has defined an IID
   *
   * @return            IID for given interface
   */
  public static String getInterfaceIID(Class aInterface) {
    // Get short class name (i.e. "bar", not "org.blah.foo.bar")
    StringBuffer iidName = new StringBuffer();
    String fullClassName = aInterface.getName();
    int index = fullClassName.lastIndexOf(".");
    String className = index > 0 ? fullClassName.substring(index + 1)
                                 : fullClassName;

    // Create iid field name
    if (className.startsWith("ns")) {
      iidName.append("NS_");
      iidName.append(className.substring(2).toUpperCase());
    } else {
      iidName.append(className.toUpperCase());
    }
    iidName.append("_IID");

    String iid;
    try {
      Field iidField = aInterface.getDeclaredField(iidName.toString());
      iid = (String) iidField.get(null);
    } catch (NoSuchFieldException e) {
      // Class may implement non-Mozilla interfaces, which would not have an
      // IID method.  In that case, just null.
      iid = null;
    } catch (IllegalAccessException e) {
      // Not allowed to access that field for some reason.  Write out an
      // error message, but don't fail.
      System.err.println("ERROR: Could not get field " + iidName.toString());
      iid = null;
    }

    return iid;
  }
  
  /**
   * @see IGRE#lockProfileDirectory(File, nsISupports)
   */
  public nsISupports lockProfileDirectory(File aDirectory)
    throws XPCOMException
  {
    return gre.lockProfileDirectory(aDirectory);
  }

  /**
   * @see IGRE#notifyProfile()
   */
  public void notifyProfile() {
    gre.notifyProfile();
  }

}

