// This is an ES5 JavaScript editor and code
// 190319/v0.15: Added UI
// 220404/v0.16: updated to DHT (vs SHT) IoT device
// 230225/v0.17: Added F & C

// Globals
const version = 'IoTDemo-v0.17: ';
const debug = true;

// Setup spreadsheet
function onOpen() {
  SpreadsheetApp.getUi()
    .createMenu('Scripts')
    .addItem('Clear Log', 'clearSheet')
    .addItem('Fetch Measurement', 'getMeasurement')
    .addItem('Set Logging Period', 'showLPSidebar')
    .addToUi();
}

// WEBAPP

// Create an object to send a JSON response.
var usage = {
  version: version,
  message: 'Accepts only json POSTs: ',
  example: {
    timestamp: '2019-02-23T00:34:32.731Z',
    tempearture: '28.6',
    humidity: '56.4'
  }
}

// Helper functions
function jlog(msg) {
  if (debug) Logger.log(version+msg);
}

function returnUsage() {
  const response = ContentService.createTextOutput(); // create the response object 
  response.setContent(JSON.stringify(usage)); // convert the usage object to JSON
  response.setMimeType(ContentService.MimeType.JSON); // set the mime type of the response to JSON
  
  return response; // this is what gets returned to caller on a GET.
}

function returnOkay(){
  const response = ContentService.createTextOutput();
  response.setMimeType(ContentService.MimeType.JSON);
  response.setContent(JSON.stringify({status: "okay"}));
  jlog(version+'request: \n'+JSON.stringify(response));
  console.log(version+'request: \n'+JSON.stringify(response));

  return response;
}

function logIt(data) {
  // get current sheet
  const log = SpreadsheetApp.getActiveSheet();
  
  // create an array with the row of data we want to log
  const row = [];
  row.push(data.timestamp);
  row.push(data.temperature); // Celcius
  row.push(Number(data.temperature)*9/5 + 32); // Fahrenheit 
  row.push(data.humidity);
  
  // append row to sheet
  log.appendRow(row);
  
  return true;
}

// CLEAR SPREADSHEET
function clearSheet() {
  // get the current sheet
  const log = SpreadsheetApp.getActiveSheet();
  // clear the sheet from (2,1) to (latRow-1, 3)
  log.getRange(2,1, log.getLastRow()-1, log.getLastColumn()).clear();
}

// WEB SERVICE
// GET
// This script is set to accept only a POST so tell the caller that we need a POST.
function doGet(reqeust) {
  return returnUsage();
}

// POST
// This is our main function. Our webhook will call this.
function doPost(request) {
  // Grab JSON Object
  const post = JSON.parse(request.postData.contents);
  debug && console.log(version+'Received post: '+JSON.stringify(post));
  
  // Do we have what we need?
  var data = JSON.parse(post.data);
  if(!data.temperature) {
    console.log(version+'Error: No temperature data found: '+JSON.stringify(data));
    return returnUsage();
  }
  
  // Okay we have what we need now log it.
  if (logIt(data)) return returnOkay();
  // implicit else
  return returnUsage();
}