// © 2022 Gatorbot LLC
// MIT license

// GLOBALS
#define PIN D3
#define MIN_SAMPLE 5000             // minimum time (ms) between reads
int status = -1;
// -1: uninitialized
// 0: okay
// 1: read_error
String statusMsg ="undefined";      // statusMsg to post to cloud
double temperature = 0.0;           // last read temperature in Celcius
double humidity = 0.0;              // last read humidity in percent
unsigned long sampleTime = 0;       // ms since power of last device read
time_t timeStamp = 0;               // s since epoch of last device read
uint8_t data[5];                    // 40-bit data buffer for device read
unsigned long samplePeriod = 0;     // sampling period in ms. 

// Helpers
String jlog(String msg)
{
    Particle.publish("info", msg);  // push this to cloud as an event.
    return msg;                     // return msg for chaining.
}

// DHT Functions
// readSensor
// This will read the sensor data into the data buffer
int readSensor()
{
    uint8_t checksum = 0;
    uint16_t value = 0;
    int bitcount = 0;
    int word = 0;
    unsigned long period = 0;
    
    // check minimum sample time
    period = millis();
    // check for rollover
    if (sampleTime > period) sampleTime = 0;
    if ((period-sampleTime) < MIN_SAMPLE) return status; // use last results
    sampleTime = period;
    
    // send start signal 
    pinMode(PIN, OUTPUT);
    digitalWrite(PIN, LOW);
    delayMicroseconds(1000);        // start signal
    digitalWrite(PIN, HIGH);
    delayMicroseconds(25);          // pull high and release
    pinMode(PIN, INPUT_PULLUP);
    
    // gather response
    // we left it high wait for sensor to pull low then ACK high for 80 µs
    period = pulseIn(PIN, HIGH);
    if (period == 0) {
        // 3s timeout
        statusMsg = "read_error[1]: No ACK";
        return status = 1;
    }
    
    // now catch data (40-bits)
    while (bitcount < 40) {
        period = pulseIn(PIN, HIGH);
        if (period == 0) { 
            statusMsg = String::format("read_error[2]: Error in data. bitcount = %i.", bitcount);
            return status=2;
        }
        // add 0 bit
        checksum <<= 1;
        //  26 < period < 28 --> 0 || period == 70 --> 1
        if (period > 35)  checksum |= 1;  // set bit
        // check for full word
        if (++bitcount%8 == 0) {
            // push word to buffer
            data[word++] = checksum;
            checksum = 0;
        }
    }
    if (bitcount != 40) {
        statusMsg = String::format("read_error[3]: Expecting 40 bits, got %i", bitcount);
        return status = 3;
    }
    // we have 40 bits do checksum
    checksum = 0;
    for (int i = 0; i < 4; ++i) {
        checksum += data[i];
    }
    if (checksum != data[4]) {
        String msg = String::format("checksum_error: Expected %u, calculated %u.", data[4], checksum);
        msg += "\nData: ";
        for (int i = 0; i<5; ++i) msg += String::format("\ndata[%i]: %X", i, data[i]);
        statusMsg = msg;
        return status = 5;
    }
    // else all is good 
    
    // set globals...
    value = data[0];
    value <<= 8;
    value |= data[1];
    humidity = value/10.0;
    value = 0x7F & data[2]; // clear MSb
    value <<= 8;
    value |= data[3];
    temperature = value/10.0;
    if (data[2] & 0x80) temperature *= -1; // if MSb is set then negative temperature
    
    // set timestamp
    timeStamp = Time.now();
    
    return status = 0;
}

int setPeriod(String str)
{
    samplePeriod = str.toInt();
    if (samplePeriod != 0 && samplePeriod * 1000 < MIN_SAMPLE) samplePeriod = MIN_SAMPLE/1000;
    jlog(String::format("Setting sample period to %u s.", samplePeriod));
    return samplePeriod;
}

String getMeasurementJSON()
{
    if (status == 0) statusMsg = String::format("{\"temperature\": %.2lf, \"humidity\": %.2lf, \"timestamp\": \"%s\"}", temperature, humidity, Time.format(timeStamp).c_str());
    return statusMsg;
}

// 
int sampleMeasurement(String str){
    readSensor();
    jlog(getMeasurementJSON());
    return status;
}

void webhookResponse(const char *event, const char *data){
    // just log it.
    jlog(String::format("{\"event\": \"%s\", \"data\": \"%s\"}", event, data));
}

void setup() {
    Time.setFormat(TIME_FORMAT_ISO8601_FULL);               // Time format expected by Google Sheet
    pinMode(PIN, INPUT_PULLUP);
    Particle.variable("temperature", temperature);
    Particle.variable("status", status);
    Particle.variable("humidity", humidity);
    Particle.variable("samplePeriod", samplePeriod);
    Particle.variable("statusMsg", statusMsg);
    Particle.function("sampleMeasurement", sampleMeasurement);
    Particle.function("setPeriod", setPeriod);
    Particle.subscribe("hook-response/Measurement", webhookResponse);
    // test
    delay(3000);
    readSensor();
    jlog(getMeasurementJSON());
}

void loop() {
    unsigned long tick;
    if (samplePeriod > 0) {
        tick = millis();
        // check for 32-bit rollover
        if (sampleTime > tick) sampleTime = 0;
        if (tick - sampleTime > samplePeriod * 1000){
            // get sample
            readSensor();
            Particle.publish("Measurement", getMeasurementJSON(), PRIVATE);
        }
    }

}