// This file tests channel event sinks (bug 315598 et al)

const sinkCID = Components.ID("{14aa4b81-e266-45cb-88f8-89595dece114}");
const sinkContract = "@mozilla.org/network/unittest/channeleventsink;1";

const categoryName = "net-channel-event-sinks";

const NS_BINDING_ABORTED = 0x804b0002;

/**
 * This object is both a factory and an nsIChannelEventSink implementation (so, it
 * is de-facto a service). It's also an interface requestor that gives out
 * itself when asked for nsIChannelEventSink.
 */
var eventsink = {
  QueryInterface: function eventsink_qi(iid) {
    if (iid.equals(Components.interfaces.nsISupports) ||
        iid.equals(Components.interfaces.nsIFactory) ||
        iid.equals(Components.interfaces.nsIChannelEventSink))
      return this;
    throw Components.results.NS_ERROR_NO_INTERFACE;
  },
  createInstance: function eventsink_ci(outer, iid) {
    if (outer)
      throw Components.results.NS_ERROR_NO_AGGREGATION;
    return this.QueryInterface(iid);
  },
  lockFactory: function eventsink_lockf(lock) {
    throw Components.results.NS_ERROR_NOT_IMPLEMENTED;
  },

  onChannelRedirect: function eventsink_onredir(oldChan, newChan, flags) {
    // veto
    this.called = true;
    throw NS_BINDING_ABORTED;
  },

  getInterface: function eventsink_gi(iid) {
    if (iid.equals(Components.interfaces.nsIChannelEventSink))
      return this;
    throw Components.results.NS_ERROR_NO_INTERFACE;
  },

  called: false
};

var listener = {
  expectSinkCall: true,

  onStartRequest: function test_onStartR(request, ctx) {
    try {
      // Commenting out this check pending resolution of bug 255119
      //if (Components.isSuccessCode(request.status))
      //  do_throw("Channel should have a failure code!");

      // The current URI must be the original URI, as all redirects have been
      // cancelled
      if (!(request instanceof Components.interfaces.nsIChannel) ||
          !request.URI.equals(request.originalURI))
        do_throw("Wrong URI: Is <" + request.URI.spec + ">, should be <" +
                 request.originalURI.spec + ">");

      if (request instanceof Components.interfaces.nsIHttpChannel) {
        // As we expect a blocked redirect, verify that we have a 3xx status
        do_check_eq(Math.floor(request.responseStatus / 100), 3);
        do_check_eq(request.requestSucceeded, false);
      }

      do_check_eq(eventsink.called, this.expectSinkCall);
    } catch (e) {
      do_throw("Unexpected exception: " + e);
    }

    throw Components.results.NS_ERROR_ABORT;
  },

  onDataAvailable: function test_ODA() {
    do_throw("Should not get any data!");
  },

  onStopRequest: function test_onStopR(request, ctx, status) {
    if (this._iteration <= 2)
      run_test_continued();
    do_test_finished();
  },

  _iteration: 1
};

function makeChan(url) {
  var ios = Components.classes["@mozilla.org/network/io-service;1"]
                      .getService(Components.interfaces.nsIIOService);
  var chan = ios.newChannel(url, null, null)
                .QueryInterface(Components.interfaces.nsIHttpChannel);

  return chan;
}

function run_test() {
  start_server(4444);

  Components.manager.nsIComponentRegistrar.registerFactory(sinkCID,
    "Unit test Event sink", sinkContract, eventsink);

  // Step 1: Set the callbacks on the listener itself
  var chan = makeChan("http://localhost:4444/redirect");
  chan.notificationCallbacks = eventsink;

  chan.asyncOpen(listener, null);

  do_test_pending();
}

function run_test_continued() {
  eventsink.called = false;

  var catMan = Components.classes["@mozilla.org/categorymanager;1"]
                         .getService(Components.interfaces.nsICategoryManager);

  var chan;
  if (listener._iteration == 1) {
    // Step 2: Category entry
    catMan.nsICategoryManager.addCategoryEntry(categoryName, "unit test",
                                               sinkContract, false, true);
    chan = makeChan("http://localhost:4444/redirect")
  } else {
    // Step 3: Global contract id
    catMan.nsICategoryManager.deleteCategoryEntry(categoryName, "unit test",
                                                  false);
    listener.expectSinkCall = false;
    chan = makeChan("http://localhost:4444/redirectfile");
  }

  listener._iteration++;
  chan.asyncOpen(listener, null);

  do_test_pending();
}

