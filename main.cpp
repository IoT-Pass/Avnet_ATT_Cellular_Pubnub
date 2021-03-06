/* ===================================================================
Copyright © 2016, AVNET Inc.  

Licensed under the Apache License, Version 2.0 (the "License"); 
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, 
software distributed under the License is distributed on an 
"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
either express or implied. See the License for the specific 
language governing permissions and limitations under the License.

======================================================================== */

#include "mbed.h" 
#include <cctype>
#include <string>
#include "config_me.h"
#include "sensors.h"
#include "cell_modem.h"
#include "hardware.h"

#include "pubnub.h"

I2C i2c(PTC11, PTC10);    //SDA, SCL -- define the I2C pins being used
MODSERIAL pc(USBTX, USBRX, 256, 256); // tx, rx with default tx, rx buffer sizes
MODSERIAL mdm(PTD3, PTD2, 4096, 4096);
DigitalOut led_green(LED_GREEN);
DigitalOut led_red(LED_RED);
DigitalOut led_blue(LED_BLUE);


//********************************************************************************************************************************************
//* Create string with sensor readings that can be sent to flow as an HTTP get
//********************************************************************************************************************************************
K64F_Sensors_t  SENSOR_DATA =
{
    .Temperature        = "0",
    .Humidity           = "0",
    .AccelX             = "0",
    .AccelY             = "0",
    .AccelZ             = "0",
    .MagnetometerX      = "0",
    .MagnetometerY      = "0",
    .MagnetometerZ      = "0",
    .AmbientLightVis    = "0",
    .AmbientLightIr     = "0",
    .UVindex            = "0",
    .Proximity          = "0",
    .Temperature_Si7020 = "0",
    .Humidity_Si7020    = "0",
    .Virtual_Sensor1    = "0",
    .Virtual_Sensor2    = "0",
    .Virtual_Sensor3    = "0",
    .Virtual_Sensor4    = "0",
    .Virtual_Sensor5    = "0",
    .Virtual_Sensor6    = "0",
    .Virtual_Sensor7    = "0",
    .Virtual_Sensor8    = "0",
    .GPS_Satellites     = "0",
    .GPS_Latitude       = "0",
    .GPS_Longitude      = "0",
    .GPS_Altitude       = "0",
    .GPS_Speed          = "0",
    .GPS_Course         = "0"
};

void display_app_firmware_version(void)
{
    PUTS("\r\n\r\nApp Firmware: Release 1.0 - built: "__DATE__" "__TIME__"\r\n\r\n");
}


static void GeneratePubnubJSON(char *s, unsigned n)
{
    switch (iSensorsToReport) {
    case TEMP_HUMIDITY_ONLY:
        snprintf(s, n, "{\"serial\":\"%s\",\"temp\":%s,\"humidity\":%s}", THE_DEVICE_NAME, SENSOR_DATA.Temperature, SENSOR_DATA.Humidity);
        break;
    case TEMP_HUMIDITY_ACCELEROMETER:
        snprintf(s, n, "{\"serial\":\"%s\",\"temp\":%s,\"humidity\":%s,\"accelX\":%s,\"accelY\":%s,\"accelZ\":%s}", THE_DEVICE_NAME, SENSOR_DATA.Temperature, SENSOR_DATA.Humidity, SENSOR_DATA.AccelX, SENSOR_DATA.AccelY, SENSOR_DATA.AccelZ);
        break;
    case TEMP_HUMIDITY_ACCELEROMETER_PMODSENSORS:
        snprintf(s, n, "{\"serial\":\"%s\",\"temp\":%s,\"humidity\":%s,\"accelX\":%s,\"accelY\":%s,\"accelZ\":%s,\"proximity\":%s,\"light_uv\":%s,\"light_vis\":%s,\"light_ir\":%s}", THE_DEVICE_NAME, SENSOR_DATA.Temperature, SENSOR_DATA.Humidity, SENSOR_DATA.AccelX,SENSOR_DATA.AccelY,SENSOR_DATA.AccelZ, SENSOR_DATA.Proximity, SENSOR_DATA.UVindex, SENSOR_DATA.AmbientLightVis, SENSOR_DATA.AmbientLightIr);
        break;
    case TEMP_HUMIDITY_ACCELEROMETER_PMODSENSORS_VIRTUALSENSORS:
        snprintf(s, n, "{\"serial\":\"%s\",\"temp\":%s,\"humidity\":%s,\"accelX\":%s,\"accelY\":%s,\"accelZ\":%s,\"proximity\":%s,\"light_uv\":%s,\"light_vis\":%s,\"light_ir\":%s,\"virt_sens1\":%s,\"virt_sens2\":%s,\"virt_sens3\":%s,\"virt_sens4\":%s,\"virt_sens5\":%s,\"virt_sens6\":%s,\"virt_sens7\":%s,\"virt_sens8\":%s}", 
            THE_DEVICE_NAME, 
            SENSOR_DATA.Temperature, SENSOR_DATA.Humidity, 
            SENSOR_DATA.AccelX,SENSOR_DATA.AccelY,SENSOR_DATA.AccelZ, 
            SENSOR_DATA.Proximity, SENSOR_DATA.UVindex, SENSOR_DATA.AmbientLightVis, SENSOR_DATA.AmbientLightIr, 
            SENSOR_DATA.Virtual_Sensor1, SENSOR_DATA.Virtual_Sensor2, 
            SENSOR_DATA.Virtual_Sensor3, SENSOR_DATA.Virtual_Sensor4, 
            SENSOR_DATA.Virtual_Sensor5, SENSOR_DATA.Virtual_Sensor6, 
            SENSOR_DATA.Virtual_Sensor7, SENSOR_DATA.Virtual_Sensor8);
        break;
    default:
        snprintf(s, n, "\"Invalid sensors selected: %d\"", iSensorsToReport);
        break;
    }
}


static void print_pubnub_result(pubnub_ctx::result r)
{
    switch (r) {
    case pubnub_ctx::format_error:
        PRINTF(RED "Pubnub response format error" DEF "\r\n");
        break;
    case pubnub_ctx::response_too_short:
        PRINTF(RED "Pubnub response too short" DEF "\r\n");
        break;
    case pubnub_ctx::missing_open_bracket:
        PRINTF(RED "Pubnub response missing open bracket `[`" DEF "\r\n");
        break;
    case pubnub_ctx::missing_close_bracket:
        PRINTF(RED "Pubnub response missing close bracket `]`" DEF "\r\n");
        break;
    case pubnub_ctx::missing_time_token:
        PRINTF(RED "Pubnub subscribe response missing time token" DEF "\r\n");
        break;
    case pubnub_ctx::bad_time_token:
        PRINTF(RED "Pubnub subscribe response bad time token" DEF "\r\n");
        break;
    case pubnub_ctx::publish_failed:
        PRINTF(RED "Pubnub publish failed" DEF "\r\n");
        break;
    case pubnub_ctx::ok:
        PRINTF(GRN "Pubnub transaction success" DEF "\r\n");
        break;
    default:
        PRINTF(RED "Unknown Pubnub erorr %d" DEF "\r\n", static_cast<int>(r));
        break;
    }
}
            
            
//Periodic timer
Ticker OneMsTicker;
volatile bool bTimerExpiredFlag = false;
int OneMsTicks = 0;
int iTimer1Interval_ms = 1000;
//********************************************************************************************************************************************
//* Periodic 1ms timer tick
//********************************************************************************************************************************************
void OneMsFunction() 
{
    OneMsTicks++;
    if ((OneMsTicks % iTimer1Interval_ms) == 0)
    {
        bTimerExpiredFlag = true;
    }            
} //OneMsFunction()

//********************************************************************************************************************************************
//* Set the RGB LED's Color
//* LED Color 0=Off to 7=White.  3 bits represent BGR (bit0=Red, bit1=Green, bit2=Blue) 
//********************************************************************************************************************************************
void SetLedColor(unsigned char ucColor)
{
    //Note that when an LED is on, you write a 0 to it:
    led_red = !(ucColor & 0x1); //bit 0
    led_green = !(ucColor & 0x2); //bit 1
    led_blue = !(ucColor & 0x4); //bit 2
} //SetLedColor()

//********************************************************************************************************************************************
//* Process the JSON response.  In this example we are only extracting a LED color. 
//********************************************************************************************************************************************
bool parse_JSON(char const* json_string)
{
    char const* beginquote;
    char token[] = "\"LED\":\"";
    beginquote = strstr(json_string, token );
    if ((beginquote != 0))
    {
        char cLedColor = beginquote[strlen(token)];
        PRINTF(GRN "LED Found : %c" DEF "\r\n", cLedColor);
        switch(cLedColor)
        {
            case 'O':
            { //Off
                SetLedColor(0);
                break;
            }
            case 'R':
            { //Red
                SetLedColor(1);
                break;
            }
            case 'G':
            { //Green
                SetLedColor(2);
                break;
            }
            case 'Y':
            { //Yellow
                SetLedColor(3);
                break;
            }
            case 'B':
            { //Blue
                SetLedColor(4);
                break;
            }
            case 'M':
            { //Magenta
                SetLedColor(5);
                break;
            }
            case 'T':
            { //Turquoise
                SetLedColor(6);
                break;
            }
            case 'W':
            { //White
                SetLedColor(7);
                break;
            }
            default:
            {
                break;
            }
        } //switch(cLedColor)
        return true;
    }
    else
    {
        return false;
    }
} //parse_JSON


int main()
{
    static unsigned ledOnce = 0;
    //delay so that the debug terminal can open after power-on reset:
    wait (5.0);
    pc.baud(115200);
    
    display_app_firmware_version();
    
    PRINTF(GRN "Hello World from the Cellular IoT Kit!\r\n\r\n");

    //Initialize the I2C sensors that are present
    sensors_init();
    read_sensors();

    // Set LED to RED until init finishes
    SetLedColor(0x1); //Red
    // Initialize the modem
    PRINTF("\r\n");
    cell_modem_init();
    display_wnc_firmware_rev();

    // Set LED BLUE for partial init
    SetLedColor(0x4); //Blue

    //Create a 1ms timer tick function:
    iTimer1Interval_ms = SENSOR_UPDATE_INTERVAL_MS;
    OneMsTicker.attach(OneMsFunction, 0.001f) ;

    // Create the Pubnub context and message vector
    pubnub_ctx pb(PUBNUB_PUBLISH_KEY, PUBNUB_SUBSCRIBE_KEY);
    std::vector<std::string> messages;
    
    // If you wish, you can set the UUID to use - which will make that UUID
    // visible for Presence related Pubnub features
    // pb.set_uuid(THE_DEVICE_NAME);

    // Send and receive data perpetually
    while(1) {
#ifdef USE_VIRTUAL_SENSORS
        ProcessUsbInterface();
#endif
        if  (bTimerExpiredFlag)
        {
            bTimerExpiredFlag = false;
            read_sensors(); //read available external sensors from a PMOD and the on-board motion sensor

            char json_string[512];
            GeneratePubnubJSON(json_string, sizeof json_string);
            print_pubnub_result(pb.publish(PUBNUB_CHANNEL, json_string));
            
            messages.clear();
            print_pubnub_result(pb.subscribe(PUBNUB_CHANNEL, messages));
            if (!ledOnce && !messages.empty()) {
                ledOnce = 1;
                SetLedColor(0x2);
            }
            for (std::vector<std::string>::iterator it = messages.begin(); it != messages.end(); ++it) {
                char const *s = it->c_str();
                PRINTF(BLU "Pubnub message: %s" DEF "\n", s);
                parse_JSON(s);
            }
        } //bTimerExpiredFlag
    } //forever loop
}
