do_import_script("netwerk/test/httpserver/httpd.js");

var server;
const BUGID = "263127";

var listener = {
  QueryInterface: function(iid) {
    if (!iid.equals(nsIDownloadObserver) &&
        !iid.equals(nsISupports))
      throw Components.results.NS_ERROR_NO_INTERFACE;

    return this;
  },

  onDownloadComplete: function(downloader, request, ctxt, status, file) {
    server.stop();

    if (!file)
      do_throw("Download failed");

    try {
      file.remove(false);
    }
    catch (e) {
      do_throw(e);
    }

    do_check_false(file.exists());

    do_test_finished();
  }
}

function run_test() {
  // start server
  server = new nsHttpServer();
  server.start(4444);

  // Initialize downloader
  var channel = Cc["@mozilla.org/network/io-service;1"]
                  .getService(Ci.nsIIOService)
                  .newChannel("http://localhost:4444/", null, null);

  var targetFile = Cc["@mozilla.org/file/directory_service;1"]
                     .getService(Ci.nsIProperties)
                     .get("TmpD", Ci.nsIFile);
  targetFile.append("bug" + BUGID + ".test");
  if (targetFile.exists())
    targetFile.remove(false);

  var downloader = Cc["@mozilla.org/network/downloader;1"]
                     .createInstance(Ci.nsIDownloader);
  downloader.init(listener, targetFile);

  // Start download
  channel.asyncOpen(downloader, null);

  do_test_pending();
}
