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

#ifndef __CONFIG_ME_H_
#define __CONFIG_ME_H_

// User must set these for own context:

#define BUF_SIZE_FOR_N_MAX_SOCKREAD (10)
#define MAX_WNC_SOCKREAD_PAYLOAD (1500)

// This is the server's base URL name. Pubnub SDKs refer to this as "the origin".
// In general, you should leave this to "pubsub.pubnub.com", but in some cases
// you may want to user a different origin.
static const char * MY_SERVER_URL       = "pubsub.pubnub.com";


/** Put your publish key here. If you don't have one, "demo" is the default
    public key. But, it has limitations.
    */
#define PUBNUB_PUBLISH_KEY "demo"

/** Put your subscribe key here. If you don't have one, "demo" is the default
    public key.
    */
#define PUBNUB_SUBSCRIBE_KEY "demo"

/** Define the Pubnub channel to use here. For the "demo/demo" keys, a well-known
    channel is "hello_world", but you can use any you like, channels are dynamic
    on Pubnub
*/
#define PUBNUB_CHANNEL "hello_world"

// This identifier specifies a "device name" to be sent in the JSON message.
// You can also use it as the UUID, if you wish.
#define THE_DEVICE_NAME                "vstarterkit001"

// This constant defines how often sensors are read and sent up to FLOW
#define SENSOR_UPDATE_INTERVAL_MS       5000; //5 seconds

// Specify here how many sensor parameters you want reported to FLOW.
// You can use only the temperature and humidity from the shield HTS221
// or you can add the reading of the FXO8700CQ motion sensor on the FRDM-K64F board
// or if you have a SiLabs PMOD plugged into the shield, you can add its proximity sensor,
// UV light, visible ambient light and infrared ambient light readings
// If you run the Windows "Sensor Simulator" utility, 8 additional virtual
// sensors can also be made available via USB.
#define TEMP_HUMIDITY_ONLY                                      1
#define TEMP_HUMIDITY_ACCELEROMETER                             2
#define TEMP_HUMIDITY_ACCELEROMETER_GPS                         3
#define TEMP_HUMIDITY_ACCELEROMETER_PMODSENSORS                 4
#define TEMP_HUMIDITY_ACCELEROMETER_PMODSENSORS_VIRTUALSENSORS  5
static int iSensorsToReport = TEMP_HUMIDITY_ACCELEROMETER; //modify this to change your selection

// This is the APN name for the cellular network, you will need to change this, check the instructions included with your SIM card kit:
static const char * MY_APN_STR          = "m2m.com.attz";

//This is for normal HTTP.  If you want to use TCP to a specific port, change that here:
static const char * MY_PORT_STR         = "80";

#endif
