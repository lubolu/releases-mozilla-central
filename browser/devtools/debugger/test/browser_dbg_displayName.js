/*
 * Any copyright is dedicated to the Public Domain.
 * http://creativecommons.org/publicdomain/zero/1.0/
 */

// Tests that anonymous functions appear in the stack frame list with either
// their displayName property or a SpiderMonkey-inferred name.

var gPane = null;
var gTab = null;
var gDebuggee = null;
var gDebugger = null;

const TAB_URL = EXAMPLE_URL + "browser_dbg_displayName.html";

function test() {
  debug_tab_pane(TAB_URL, function(aTab, aDebuggee, aPane) {
    gTab = aTab;
    gDebuggee = aDebuggee;
    gPane = aPane;
    gDebugger = gPane.panelWin;

    testAnonCall();
  });
}

function testAnonCall() {
  gDebugger.DebuggerController.activeThread.addOneTimeListener("framesadded", function() {
    Services.tm.currentThread.dispatch({ run: function() {

      let frames = gDebugger.DebuggerView.StackFrames.widget._list;

      is(gDebugger.DebuggerController.activeThread.state, "paused",
        "Should only be getting stack frames while paused.");

      is(frames.querySelectorAll(".dbg-stackframe").length, 3,
        "Should have three frames.");

      is(frames.querySelector("#stackframe-0 .dbg-stackframe-title").getAttribute("value"),
        "anonFunc", "Frame name should be anonFunc");

      testInferredName();
    }}, 0);
  });

  gDebuggee.evalCall();
}

function testInferredName() {
  gDebugger.DebuggerController.activeThread.addOneTimeListener("framesadded", function() {
    Services.tm.currentThread.dispatch({ run: function() {

      let frames = gDebugger.DebuggerView.StackFrames.widget._list;

      is(gDebugger.DebuggerController.activeThread.state, "paused",
        "Should only be getting stack frames while paused.");

      is(frames.querySelectorAll(".dbg-stackframe").length, 3,
        "Should have three frames.");

      is(frames.querySelector("#stackframe-0 .dbg-stackframe-title").getAttribute("value"),
        "a/<", "Frame name should be a/<");

      resumeAndFinish();
    }}, 0);
  });

  gDebugger.DebuggerController.activeThread.resume();
}

function resumeAndFinish() {
  gDebugger.DebuggerController.activeThread.resume(function() {
    removeTab(gTab);
    gPane = null;
    gDebuggee = null;
    finish();
  });
}
