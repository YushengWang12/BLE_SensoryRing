// The code is modified based on Adafruit examples, to program nRF52 with customized functions.

/*********************************************************************
 This is an example for our nRF52 based Bluefruit LE modules

 Pick one up today in the adafruit shop!

 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/

#include <Tlv493d.h>

const int LIG_Input = A0;
const int LIG_1_PIN = A1;
const int In_cap = A2;
const int Out_cap = A6; // define pins for resistance and capacitance measurement


const int DAC_PRECISION = 1024;
const int VDD = 3;
const int R_REF = 100;

unsigned long time_milli;

// Tlv493d Opject
Tlv493d Tlv493dMagnetic3DSensor = Tlv493d();
float x = 0;
float y = 0;
float z = 0; 

#include <bluefruit.h>
#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>

// BLE Service
BLEDfu  bledfu;  // OTA DFU service
BLEDis  bledis;  // device information
BLEUart bleuart; // uart over ble
BLEBas  blebas;  // battery
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          
void setup()
{
  time_milli = micros();
  
  Serial.begin(115200);
  Serial.println("Start");
  Tlv493dMagnetic3DSensor.begin();
  Tlv493dMagnetic3DSensor.setAccessMode(Tlv493dMagnetic3DSensor.ULTRALOWPOWERMODE);
  Tlv493dMagnetic3DSensor.enableTemp();

#if CFG_DEBUG
  // Blocking wait for connection when debug mode is enabled via IDE
//  while ( !Serial ) yield();
#endif
  
  Serial.println("Bluefruit52 BLEUART Example");
  Serial.println("---------------------------\n");

  // Setup the BLE LED to be enabled on CONNECT
  // Note: This is actually the default behavior, but provided
  // here in case you want to control this LED manually via PIN 19
  Bluefruit.autoConnLed(true);

  // Config the peripheral connection with maximum bandwidth 
  // more SRAM required by SoftDevice
  // Note: All config***() function must be called before begin()
  Bluefruit.configPrphBandwidth(BANDWIDTH_MAX);


  Bluefruit.begin();
  Bluefruit.setName("Stent");
  Bluefruit.setTxPower(0);    // Supported tx_power values depending on mcu:
                              // - nRF52832: -40dBm, -20dBm, -16dBm, -12dBm, -8dBm, -4dBm, 0dBm, +3dBm and +4dBm.
                              // testing shows dropping 4 dBm reduces range by 2.5m
                              // test condition: behind room with limited gap
  //Bluefruit.setName(getMcuUniqueID()); // useful testing with multiple central connections
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

  // To be consistent OTA DFU should be added first if it exists
  bledfu.begin();

  // Configure and Start Device Information Service
  bledis.setManufacturer("Adafruit Industries");
  bledis.setModel("Bluefruit Feather52");
  bledis.begin();

  // Configure and Start BLE Uart Service
  bleuart.begin();

  // Start BLE Battery Service
  blebas.begin();
  blebas.write(100);

  // Set up and start advertising
  startAdv();

  Serial.println("Please use Adafruit's Bluefruit LE app to connect in UART mode");
  Serial.println("Once connected, enter character(s) that you wish to send");

  pinMode(In_cap, OUTPUT);
  pinMode(Out_cap, OUTPUT);
  pinMode(LIG_Input, OUTPUT); // initialize input and output pins

}

void startAdv(void)
{
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();

  // Include bleuart 128-bit uuid
  Bluefruit.Advertising.addService(bleuart);

  // Secondary Scan Response packet (optional)
  // Since there is no room for 'Name' in Advertising packet
  Bluefruit.ScanResponse.addName(); 
  
  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   * 
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html   
   */
  Bluefruit.Advertising.restartOnDisconnect(true);
//  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms

  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
}

void loop()
{

  if (true)
  {
    float capacitance = readCAP(In_cap,Out_cap); // capacitance measurement for liquid layer thickness sensing

    delay(30);
    digitalWrite(LIG_Input,HIGH);
    delay(30);
    float LIG1 = analogRead(LIG_1_PIN);  // resistance measurement for liquid viscosity sensing
    digitalWrite(LIG_Input,LOW);
    delay(40);

    Tlv493dMagnetic3DSensor.updateData();
    x = Tlv493dMagnetic3DSensor.getX();
    y = Tlv493dMagnetic3DSensor.getY();
    z = Tlv493dMagnetic3DSensor.getZ(); // collect magnetic field data
    float temperature = Tlv493dMagnetic3DSensor.getTemp();  // collect temperature data
    delay(100);
    String msg =  String(x) + "," + 
                  String(y) + "," +
                  String(z) + "," +
                  String(capacitance) + "," +
                  String(temperature) + "," +
                  String(LIG1) ;
    
    bleuart.println( msg );
    
  }

}

//calculate and return resistance
float readLIG(int PIN){
  //read voltage
  float LIG = analogRead(PIN);
  float LIGV = LIG / DAC_PRECISION * VDD;
  //calculate resistance
  float LIGR = R_REF*(VDD-LIGV)/LIGV;

  return LIGR;
}

float readCAP(int INP,int OUTP){
    pinMode(INP, INPUT);
    pinMode(OUTP, OUTPUT);
    
    digitalWrite(OUTP, HIGH);
    int val = analogRead(INP);
    digitalWrite(OUTP, LOW);

    pinMode(INP, OUTPUT);
    
    float capacitance = 0.0363*exp(0.009*val);  // This expression is based on a calibration process
    return capacitance;
}

// callback invoked when central connects
void connect_callback(uint16_t conn_handle)
{
  // Get the reference to current connection
  BLEConnection* connection = Bluefruit.Connection(conn_handle);

  char central_name[32] = { 0 };
  connection->getPeerName(central_name, sizeof(central_name));

  Serial.print("Connected to ");
  Serial.println(central_name);
}

/**
 * Callback invoked when a connection is dropped
 * @param conn_handle connection where this event happens
 * @param reason is a BLE_HCI_STATUS_CODE which can be found in ble_hci.h
 */
void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  Serial.println();
  Serial.print("Disconnected, reason = 0x"); Serial.println(reason, HEX);
}
