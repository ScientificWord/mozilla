/* run some tests on the data: protocol handler */

const Cc = Components.classes;
const Ci = Components.interfaces;
const Cr = Components.results;

// The behaviour wrt spaces is:
// - Textual content keeps all spaces
// - Other content strips unescaped spaces
// - Base64 content strips escaped and unescaped spaces
var urls = [
  ["data:,foo",                                     "text/plain",               "foo"],
  ["data:application/octet-stream,foo bar",         "application/octet-stream", "foobar"],
  ["data:application/octet-stream,foo%20bar",       "application/octet-stream", "foo bar"],
  ["data:application/xhtml+xml,foo bar",            "application/xhtml+xml",    "foo bar"],
  ["data:application/xhtml+xml,foo%20bar",          "application/xhtml+xml",    "foo bar"],
  ["data:text/plain,foo%00 bar",                    "text/plain",               "foo\x00 bar"],
  ["data:text/plain;base64,Zm9 vI%20GJ%0Dhc%0Ag==", "text/plain",               "foo bar"]
];

function run_next_test() {
  test_array[test_index++]();
}

function run_test() {
  dump("*** run_test\n");

  function on_read_complete(request, data, idx) {
    dump("*** run_test.on_read_complete\n");

    if (request.nsIChannel.contentType != urls[idx][1])
      do_throw("Type mismatch! Is <" + chan.contentType + ">, should be <" + urls[idx][1] + ">");

    /* read completed successfully.  now compare the data. */
    if (data != urls[idx][2])
      do_throw("Stream contents do not match with direct read!");
    do_test_finished();
  }

  var ios = Cc["@mozilla.org/network/io-service;1"].
            getService(Ci.nsIIOService);
  for (var i = 0; i < urls.length; ++i) {
    dump("*** opening channel " + i + "\n");
    do_test_pending();
    var chan = ios.newChannel(urls[i][0], "", null);
    chan.contentType = "foo/bar"; // should be ignored
    chan.asyncOpen(new ChannelListener(on_read_complete, i), null);
  }
}

