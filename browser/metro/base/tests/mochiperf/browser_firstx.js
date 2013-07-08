/* Any copyright is dedicated to the Public Domain.
   http://creativecommons.org/publicdomain/zero/1.0/ */

function test() {
  let testDir = gTestPath.substr(0, gTestPath.lastIndexOf("/"));
  Services.scriptloader.loadSubScript(testDir + "/perfhelpers.js", this);
  runTests();
}

gTests.push({
  desc: "first x metrics",
  run: function run() {
    PerfTest.declareTest("5F2A456E-2BB2-4073-A751-936F222FEAE0",
                         "startup perf metrics", "browser", "ux",
                         "Tracks various metrics reported by nsIAppStartup.getStartupInfo(). Values are in msec.");

    let startup = Cc["@mozilla.org/toolkit/app-startup;1"].getService(Ci.nsIAppStartup).getStartupInfo();
    PerfTest.declareNumericalResults([
      { value: startup['start'] - startup.process, desc: "start" },
      { value: startup['main'] - startup.process, desc: "main" },
      { value: startup['startupCrashDetectionBegin'] - startup.process, desc: "startupCrashDetectionBegin" },
      { value: startup['firstPaint'] - startup.process, desc: "firstPaint" },
      { value: startup['sessionRestored'] - startup.process, desc: "sessionRestored" },
      { value: startup['createTopLevelWindow'] - startup.process, desc: "createTopLevelWindow" },
      { value: startup['firstLoadURI'] - startup.process, desc: "firstLoadURI" },
    ]);
  }
});

