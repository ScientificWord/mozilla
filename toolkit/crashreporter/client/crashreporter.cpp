/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * The Original Code is Mozilla Toolkit Crash Reporter
 *
 * The Initial Developer of the Original Code is
 *   Mozilla Corporation
 * Portions created by the Initial Developer are Copyright (C) 2006-2007
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *  Ted Mielczarek <ted.mielczarek@gmail.com>
 *  Dave Camp <dcamp@mozilla.com>
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

#include "crashreporter.h"

#ifdef _MSC_VER
// Disable exception handler warnings.
# pragma warning( disable : 4530 )
#endif

#include <fstream>
#include <sstream>
#include <memory>
#include <time.h>
#include <string.h>

using std::string;
using std::istream;
using std::ifstream;
using std::istringstream;
using std::ostringstream;
using std::ostream;
using std::ofstream;
using std::vector;
using std::auto_ptr;

namespace CrashReporter {

StringTable  gStrings;
string       gSettingsPath;
int          gArgc;
char**       gArgv;

static auto_ptr<ofstream> gLogStream(NULL);
static string             gDumpFile;
static string             gExtraFile;

static string kExtraDataExtension = ".extra";

void UIError(const string& message)
{
  string errorMessage;
  if (!gStrings[ST_CRASHREPORTERERROR].empty()) {
    char buf[2048];
    UI_SNPRINTF(buf, 2048,
                gStrings[ST_CRASHREPORTERERROR].c_str(),
                message.c_str());
    errorMessage = buf;
  } else {
    errorMessage = message;
  }

  UIError_impl(errorMessage);
}

static string Unescape(const string& str)
{
  string ret;
  for (string::const_iterator iter = str.begin();
       iter != str.end();
       iter++) {
    if (*iter == '\\') {
      iter++;
      if (*iter == '\\'){
        ret.push_back('\\');
      } else if (*iter == 'n') {
        ret.push_back('\n');
      } else if (*iter == 't') {
        ret.push_back('\t');
      }
    } else {
      ret.push_back(*iter);
    }
  }

  return ret;
}

static string Escape(const string& str)
{
  string ret;
  for (string::const_iterator iter = str.begin();
       iter != str.end();
       iter++) {
    if (*iter == '\\') {
      ret += "\\\\";
    } else if (*iter == '\n') {
      ret += "\\n";
    } else if (*iter == '\t') {
      ret += "\\t";
    } else {
      ret.push_back(*iter);
    }
  }

  return ret;
}

bool ReadStrings(istream& in, StringTable& strings, bool unescape)
{
  string currentSection;
  while (!in.eof()) {
    string line;
    std::getline(in, line);
    int sep = line.find('=');
    if (sep >= 0) {
      string key, value;
      key = line.substr(0, sep);
      value = line.substr(sep + 1);
      if (unescape)
        value = Unescape(value);
      strings[key] = value;
    }
  }

  return true;
}

bool ReadStringsFromFile(const string& path,
                         StringTable& strings,
                         bool unescape)
{
  ifstream* f = UIOpenRead(path);
  bool success = false;
  if (f->is_open()) {
    success = ReadStrings(*f, strings, unescape);
    f->close();
  }

  delete f;
  return success;
}

bool WriteStrings(ostream& out,
                  const string& header,
                  StringTable& strings,
                  bool escape)
{
  out << "[" << header << "]" << std::endl;
  for (StringTable::iterator iter = strings.begin();
       iter != strings.end();
       iter++) {
    out << iter->first << "=";
    if (escape)
      out << Escape(iter->second);
    else
      out << iter->second;

    out << std::endl;
  }

  return true;
}

bool WriteStringsToFile(const string& path,
                        const string& header,
                        StringTable& strings,
                        bool escape)
{
  ofstream* f = UIOpenWrite(path.c_str());
  bool success = false;
  if (f->is_open()) {
    success = WriteStrings(*f, header, strings, escape);
    f->close();
  }

  delete f;
  return success;
}

void LogMessage(const std::string& message)
{
  if (gLogStream.get()) {
    char date[64];
    time_t tm;
    time(&tm);
    if (strftime(date, sizeof(date) - 1, "%c", localtime(&tm)) == 0)
        date[0] = '\0';
    (*gLogStream) << "[" << date << "] " << message << std::endl;
  }
}

static void OpenLogFile()
{
  string logPath = gSettingsPath + UI_DIR_SEPARATOR + "submit.log";
  gLogStream.reset(UIOpenWrite(logPath.c_str(), true));
}

static bool ReadConfig()
{
  string iniPath;
  if (!UIGetIniPath(iniPath))
    return false;

  if (!ReadStringsFromFile(iniPath, gStrings, true))
    return false;

  // See if we have a string override file, if so process it
  char* overrideEnv = getenv("MOZ_CRASHREPORTER_STRINGS_OVERRIDE");
  if (overrideEnv && *overrideEnv && UIFileExists(overrideEnv))
    ReadStringsFromFile(overrideEnv, gStrings, true);

  return true;
}

static string GetExtraDataFilename(const string& dumpfile)
{
  string filename(dumpfile);
  int dot = filename.rfind('.');
  if (dot < 0)
    return "";

  filename.replace(dot, filename.length() - dot, kExtraDataExtension);
  return filename;
}

static string Basename(const string& file)
{
  int slashIndex = file.rfind(UI_DIR_SEPARATOR);
  if (slashIndex >= 0)
    return file.substr(slashIndex + 1);
  else
    return file;
}

static bool MoveCrashData(const string& toDir,
                          string& dumpfile,
                          string& extrafile)
{
  if (!UIEnsurePathExists(toDir)) {
    UIError(gStrings[ST_ERROR_CREATEDUMPDIR]);
    return false;
  }

  string newDump = toDir + UI_DIR_SEPARATOR + Basename(dumpfile);
  string newExtra = toDir + UI_DIR_SEPARATOR + Basename(extrafile);

  if (!UIMoveFile(dumpfile, newDump)) {
    UIError(gStrings[ST_ERROR_DUMPFILEMOVE]);
    return false;
  }

  if (!UIMoveFile(extrafile, newExtra)) {
    UIError(gStrings[ST_ERROR_EXTRAFILEMOVE]);
    return false;
  }

  dumpfile = newDump;
  extrafile = newExtra;

  return true;
}

static bool AddSubmittedReport(const string& serverResponse)
{
  StringTable responseItems;
  istringstream in(serverResponse);
  ReadStrings(in, responseItems, false);

  if (responseItems.find("StopSendingReportsFor") != responseItems.end()) {
    // server wants to tell us to stop sending reports for a certain version
    string reportPath =
      gSettingsPath + UI_DIR_SEPARATOR + "EndOfLife" +
      responseItems["StopSendingReportsFor"];

    ofstream* reportFile = UIOpenWrite(reportPath);
    if (reportFile->is_open()) {
      // don't really care about the contents
      *reportFile << 1 << "\n";
      reportFile->close();
    }
    delete reportFile;
  }

  if (responseItems.find("CrashID") == responseItems.end())
    return false;

  string submittedDir =
    gSettingsPath + UI_DIR_SEPARATOR + "submitted";
  if (!UIEnsurePathExists(submittedDir)) {
    return false;
  }

  string path = submittedDir + UI_DIR_SEPARATOR +
    responseItems["CrashID"] + ".txt";

  ofstream* file = UIOpenWrite(path);
  if (!file->is_open()) {
    delete file;
    return false;
  }

  char buf[1024];
  UI_SNPRINTF(buf, 1024,
              gStrings["CrashID"].c_str(),
              responseItems["CrashID"].c_str());
  *file << buf << "\n";

  if (responseItems.find("ViewURL") != responseItems.end()) {
    UI_SNPRINTF(buf, 1024,
                gStrings["CrashDetailsURL"].c_str(),
                responseItems["ViewURL"].c_str());
    *file << buf << "\n";
  }

  file->close();
  delete file;

  return true;
}

void DeleteDump()
{
  const char* noDelete = getenv("MOZ_CRASHREPORTER_NO_DELETE_DUMP");
  if (!noDelete || *noDelete == '\0') {
    if (!gDumpFile.empty())
      UIDeleteFile(gDumpFile);
    if (!gExtraFile.empty())
      UIDeleteFile(gExtraFile);
  }
}

bool SendCompleted(bool success, const string& serverResponse)
{
  if (success) {
    DeleteDump();
    return AddSubmittedReport(serverResponse);
  }
  return true;
}

} // namespace CrashReporter

using namespace CrashReporter;

void RewriteStrings(StringTable& queryParameters)
{
  // rewrite some UI strings with the values from the query parameters
  string product = queryParameters["ProductName"];
  string vendor = queryParameters["Vendor"];
  if (vendor.empty()) {
    // Assume Mozilla if no vendor is specified
    vendor = "Mozilla";
  }

  char buf[4096];
  UI_SNPRINTF(buf, sizeof(buf),
              gStrings[ST_CRASHREPORTERVENDORTITLE].c_str(),
              vendor.c_str());
  gStrings[ST_CRASHREPORTERTITLE] = buf;


  string str = gStrings[ST_CRASHREPORTERPRODUCTERROR];
  // Only do the replacement here if the string has two
  // format specifiers to start.  Otherwise
  // we assume it has the product name hardcoded.
  string::size_type pos = str.find("%s");
  if (pos != string::npos)
    pos = str.find("%s", pos+2);
  if (pos != string::npos) {
    // Leave a format specifier for UIError to fill in
    UI_SNPRINTF(buf, sizeof(buf),
                gStrings[ST_CRASHREPORTERPRODUCTERROR].c_str(),
                product.c_str(),
                "%s");
    gStrings[ST_CRASHREPORTERERROR] = buf;
  }
  else {
    // product name is hardcoded
    gStrings[ST_CRASHREPORTERERROR] = str;
  }

  UI_SNPRINTF(buf, sizeof(buf),
              gStrings[ST_CRASHREPORTERDESCRIPTION].c_str(),
              product.c_str());
  gStrings[ST_CRASHREPORTERDESCRIPTION] = buf;

  UI_SNPRINTF(buf, sizeof(buf),
              gStrings[ST_CHECKSUBMIT].c_str(),
              vendor.c_str());
  gStrings[ST_CHECKSUBMIT] = buf;

  UI_SNPRINTF(buf, sizeof(buf),
              gStrings[ST_RESTART].c_str(),
              product.c_str());
  gStrings[ST_RESTART] = buf;


  UI_SNPRINTF(buf, sizeof(buf),
              gStrings[ST_ERROR_ENDOFLIFE].c_str(),
              product.c_str());
  gStrings[ST_ERROR_ENDOFLIFE] = buf;
}

bool CheckEndOfLifed(string version)
{
  string reportPath =
    gSettingsPath + UI_DIR_SEPARATOR + "EndOfLife" + version;
  return UIFileExists(reportPath);
}

int main(int argc, char** argv)
{
  gArgc = argc;
  gArgv = argv;

  if (!ReadConfig()) {
    UIError("Couldn't read configuration.");
    return 0;
  }

  if (!UIInit())
    return 0;

  if (argc > 1) {
    gDumpFile = argv[1];
  }

  if (gDumpFile.empty()) {
    // no dump file specified, run the default UI
    UIShowDefaultUI();
  } else {
    gExtraFile = GetExtraDataFilename(gDumpFile);
    if (gExtraFile.empty()) {
      UIError(gStrings[ST_ERROR_BADARGUMENTS]);
      return 0;
    }

    if (!UIFileExists(gExtraFile)) {
      UIError(gStrings[ST_ERROR_EXTRAFILEEXISTS]);
      return 0;
    }

    StringTable queryParameters;
    if (!ReadStringsFromFile(gExtraFile, queryParameters, true)) {
      UIError(gStrings[ST_ERROR_EXTRAFILEREAD]);
      return 0;
    }

    if (queryParameters.find("ProductName") == queryParameters.end()) {
      UIError(gStrings[ST_ERROR_NOPRODUCTNAME]);
      return 0;
    }

    // There is enough information in the extra file to rewrite strings
    // to be product specific
    RewriteStrings(queryParameters);

    if (queryParameters.find("ServerURL") == queryParameters.end()) {
      UIError(gStrings[ST_ERROR_NOSERVERURL]);
      return 0;
    }

    // Hopefully the settings path exists in the environment. Try that before
    // asking the platform-specific code to guess.
    static const char kDataDirKey[] = "MOZ_CRASHREPORTER_DATA_DIRECTORY";
    const char *settingsPath = getenv(kDataDirKey);
    if (settingsPath && *settingsPath) {
      gSettingsPath = settingsPath;
    }
    else {
      string product = queryParameters["ProductName"];
      string vendor = queryParameters["Vendor"];
      if (!UIGetSettingsPath(vendor, product, gSettingsPath)) {
        gSettingsPath.clear();
      }
    }

    if (gSettingsPath.empty() || !UIEnsurePathExists(gSettingsPath)) {
      UIError(gStrings[ST_ERROR_NOSETTINGSPATH]);
      return 0;
    }

    OpenLogFile();

    if (!UIFileExists(gDumpFile)) {
      UIError(gStrings[ST_ERROR_DUMPFILEEXISTS]);
      return 0;
    }

    string pendingDir = gSettingsPath + UI_DIR_SEPARATOR + "pending";
    if (!MoveCrashData(pendingDir, gDumpFile, gExtraFile)) {
      return 0;
    }

    string sendURL = queryParameters["ServerURL"];
    // we don't need to actually send this
    queryParameters.erase("ServerURL");

    // re-set XUL_APP_FILE for xulrunner wrapped apps
    const char *appfile = getenv("MOZ_CRASHREPORTER_RESTART_XUL_APP_FILE");
    if (appfile && *appfile) {
      const char prefix[] = "XUL_APP_FILE=";
      char *env = (char*) malloc(strlen(appfile)+strlen(prefix));
      if (!env) {
        UIError("Out of memory");
        return 0;
      }
      strcpy(env, prefix);
      strcat(env, appfile);
      putenv(env);
      free(env);
    }

    vector<string> restartArgs;

    ostringstream paramName;
    int i = 0;
    paramName << "MOZ_CRASHREPORTER_RESTART_ARG_" << i++;
    const char *param = getenv(paramName.str().c_str());
    while (param && *param) {
      restartArgs.push_back(param);

      paramName.str("");
      paramName << "MOZ_CRASHREPORTER_RESTART_ARG_" << i++;
      param = getenv(paramName.str().c_str());
    };

    // allow override of the server url via environment variable
    //XXX: remove this in the far future when our robot
    // masters force everyone to use XULRunner
    char* urlEnv = getenv("MOZ_CRASHREPORTER_URL");
    if (urlEnv && *urlEnv) {
      sendURL = urlEnv;
    }

     // see if this version has been end-of-lifed
     if (queryParameters.find("Version") != queryParameters.end() &&
         CheckEndOfLifed(queryParameters["Version"])) {
       UIError(gStrings[ST_ERROR_ENDOFLIFE]);
       DeleteDump();
       return 0;
     }

    if (!UIShowCrashUI(gDumpFile, queryParameters, sendURL, restartArgs))
      DeleteDump();
  }

  UIShutdown();

  return 0;
}

#if defined(XP_WIN) && !defined(__GNUC__)
// We need WinMain in order to not be a console app.  This function is unused
// if we are a console application.
int WINAPI wWinMain( HINSTANCE, HINSTANCE, LPWSTR args, int )
{
  char** argv = static_cast<char**>(malloc(__argc * sizeof(char*)));
  for (int i = 0; i < __argc; i++) {
    argv[i] = strdup(WideToUTF8(__wargv[i]).c_str());
  }

  // Do the real work.
  return main(__argc, argv);
}
#endif
