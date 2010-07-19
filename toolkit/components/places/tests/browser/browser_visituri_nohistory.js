/**
 * Any copyright is dedicated to the Public Domain.
 * http://creativecommons.org/publicdomain/zero/1.0/
 */

Components.utils.import("resource://gre/modules/XPCOMUtils.jsm");
Components.utils.import("resource://gre/modules/Services.jsm");

const INITIAL_URL = "http://example.com/tests/toolkit/components/places/tests/browser/begin.html";
const FINAL_URL = "http://example.com/tests/toolkit/components/places/tests/browser/final.html";

let gTab = gBrowser.selectedTab = gBrowser.addTab();

/**
 * One-time observer callback.
 */
function waitForObserve(name, callback)
{
  let observer = {
    QueryInterface: XPCOMUtils.generateQI([Ci.nsIObserver]),
    observe: function(subject, topic, data)
    {
      Services.obs.removeObserver(observer, name);
      callback(subject, topic, data);
    }
  };

  Services.obs.addObserver(observer, name, false);
}

/**
 * One-time DOMContentLoaded callback.
 */
function waitForLoad(callback)
{
  gTab.linkedBrowser.addEventListener("load", function()
  {
    gTab.linkedBrowser.removeEventListener("load", arguments.callee, true);
    callback();
  }, true);
}

/**
 * Clears history invoking callback when done.
 */
function waitForClearHistory(aCallback)
{
  let observer = {
    observe: function(aSubject, aTopic, aData)
    {
      Services.obs.removeObserver(this, PlacesUtils.TOPIC_EXPIRATION_FINISHED);
      aCallback(aSubject, aTopic, aData);
    }
  };
  Services.obs.addObserver(observer, PlacesUtils.TOPIC_EXPIRATION_FINISHED, false);
  PlacesUtils.bhistory.removeAllPages();
}

function test()
{
  waitForExplicitFinish();

  Services.prefs.setBoolPref("places.history.enabled", false);

  waitForObserve("uri-visit-saved", function(subject, topic, data)
  {
    let uri = subject.QueryInterface(Ci.nsIURI);
    is(uri.spec, FINAL_URL, "received expected visit");
    if (uri.spec != FINAL_URL)
      return;
    gBrowser.removeCurrentTab();
    waitForClearHistory(finish);
  });

  Services.prefs.setBoolPref("places.history.enabled", false);
  content.location.href = INITIAL_URL;
  waitForLoad(function()
  {
    try {
      Services.prefs.clearUserPref("places.history.enabled");
    } catch(ex) {}    
    content.location.href = FINAL_URL;
  });
}
