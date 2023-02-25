// PARTICLE FUNCTIONS
// These will communicate with the photon via particle cloud functions

// SETUP PARAMETERS
var props = PropertiesService.getScriptProperties().getProperties();
var opts = {
  contentType: 'application/x-www-form-urlencoded',
  headers: { Authorization: props.auth},
  method: 'get'
}

// GET MEASUREMENT
// uses GAS's URLFetch App
function getMeasurement() {
  var httpResp;
  var results = {};
  
  // First call getMeasurement()
  opts.method = 'post';
  opts.payload = 'arg="json"';
  httpResp = UrlFetchApp.fetch(props.purl + '/sampleMeasurement', opts);
  jlog('getMeasurement: '+httpResp.getContentText());
  
  // Now fetch the results
  opts.method = 'get';
  opts.payload = '';
  httpResp = UrlFetchApp.fetch(props.purl + '/statusMsg', opts);
  if (httpResp.getResponseCode() == 200) {
    results = JSON.parse(httpResp.getContentText());
    jlog('jsonMeasurement(): '+JSON.stringify(results));
    logIt(JSON.parse(results.result));
  }
}

// SET LOGGING PERIOD
function setLoggingPeriod(period) {
  opts.method = 'post';
  opts.payload = 'arg='+period;
  jlog('setLoggingPeriod: Sending: '+JSON.stringify(opts));
  var httpResp = UrlFetchApp.fetch(props.purl + '/setPeriod', opts);
  jlog('setLoggingPeriod: Response: '+httpResp.getContentText());
}

function showLPSidebar() {
  var sidebar = HtmlService.createHtmlOutputFromFile('setLogPeriod')
    .setTitle('Set Logging Period')
    .setWidth(300);
  
  SpreadsheetApp.getUi().showSidebar(sidebar);
}