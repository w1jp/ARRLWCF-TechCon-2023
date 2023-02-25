function testLogIt() {
  if(logIt({timestamp: '2019-02-23T00:34:32.731Z', temperature: 98.6, humidity: 55.4})) 
    jlog('testLogIt: Success');
  else jlog('testLogIt: Failure');
}

function listProps(){
  jlog(JSON.stringify(PropertiesService.getScriptProperties().getProperties()));
}

function setProps(){
  // 220404: update endpoint
  props.purl = "https://api.particle.io/v1/devices/w1jp-iot";
  PropertiesService.getScriptProperties().setProperties(props);
}