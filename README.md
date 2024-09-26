# BLE Battery Level Beacon
This Arduino sketch shows how to set up a generic ESP32 as a Bluetooth Low Energy (BLE) beacon with deep sleep capability. All it does is announce itself and offer to communicate a fictitious battery level characteristic. As configured, it will stay up and accept connections for one minute. After that, it goes into deep sleep for two minutes, wakes up, and repeats. Sleep and wake times are easily configurable with #define statements at the top of the code.

See the [Bill of Materials](Bill_Of_Materials.txt) for the parts and software required.

## Why?
I've been setting up Home Assistant with various devices around the house and I can see myself eventually wanting to communicate with sensors that are outdoors. I have a handful of ESP32s and a 3D printer. I'm ready to build some stuff.

Just use [ESPHome](esphome.io), you say! It's easy!

I do. I've got Bluetooth Proxy running on an ESP32 flashed with ESPHome. But that's inside the house. I'm not thrilled with the idea of having my WiFi credentials stored on some DHT22-equipped DIY device that's not physically secured inside my home.

Use a [BTHome](https://bthome.io/) device! No WiFi required!

Sure. That Bluetooth Proxy I have running on ESPHome is reporting temperature and humidity from Xaiomi sensors flashed with BTHome firmware. It's fabulous... until I have to replace batteries. And the Xaiomi devices aren't built to stand up to the elements.

## What do I want?
Since I've already dismissed ESPHome and BTHome as possible solutions, I'm ready to DIY. So what exactly is it I'm looking for? Let's start with the requirements.
* I want a device I can plug into AC outlets on my front and back porch so I don't have to constantly toss CR2032 coin cells at it.
* I want it to be cheap in case it gets chewed on by squirrels, run over by a lawn mower, or pooped on by birds.
* I don't want my WiFi credentials on it in case it gets stolen by a roving gang of juvenile hacker wannabes.
* I want to be able to outfit it with sensors that will stand up to the extremes of Wisconsin winters.

## What use case will this battery level indicator solve?
The battery level beacon is a proof of concept. At this point, there's not even a battery to report the level of. It just returns a mocked-up value. But, it should prove a few design points.
* Test a do-it-yourself ArduinoBLE ESP32 device integration with Home Assistant.
* Test ESP deep sleep for power conservation and self-heating mitigation.
* Test Home Assistant's tolerance for an intermittently available device.
* Test the signal range for BLE devices that are outdoors.

## What's Next?
First in my plans is an outdoor temperature & humidity sensor. There are easily accessible outdoor outlets on my front and back porch, but the real test will be how well the power adapters hold up to the temperature extremes. (The ESP32 itself is rated for -40C to +125C, so no problems there.) There's also the task of designing a suitable 3D printed enclosure: something that won't trap heat, but still keeps the rain and snow off the electronics.

The next project will probably be a vehicle presence detector using an ESP32 with a rechargable Lithium Polymer battery plugged into the car's USB outlet. The LiPo battery will charge when the car is running, but how long will it last if the car is parked? How will everything hold up in the summer heat and winter cold?

But first, the proof of concept battery indicator.

## First Try (Denial)
I created [proof_of_concept_1.ino](https://github.com/DavesCodeMusings/BLE-Battery-Beacon/blob/main/proof_of_concept_1.ino) as an initial attempt to solve the problem by offering the battery level as a Bluetooth Low Energy (BLE) Generic Attribute (GATT) characteristic. ESPHome is configured using [esphome_config_1.yml](https://github.com/DavesCodeMusings/BLE-Battery-Beacon/blob/main/esphome_config_1.yml) to link the device with Home Assistant. It works well when the beacon first boots up. ESPHome finds the beacon in its scans and reads the battery level (a fictitious 100% at this stage). Home Assistant shows this battery level as an entity.

So far, so good.

Then the deep sleep kicks in, and it all falls apart.

The Arduino sketch in this first proof of concept is configured so that the ESP32 indiscriminantly goes into deep sleep after 60 seconds. If ESPHome is trying to read battery level at 59 seconds, too bad. It goes to sleep and disconnects ESPHome mid-request. If ESPHome tries to reconnect, the beacon is sleeping, so it never responds. ESPHome then sets the battery level to _NaN_ (not a number.) Home Assistant interprets this as _unknown_. And since the beacon is configured to spend more time asleep than awake, it causes a lot of _unknown_ messages.

Interestingly, the Home Assistant _presence_ of the beacon remains contantly in a _Home_ state, rather than _Away_, even though the ESP32 is sleeping for the majority of the time.

## Second Try (Anger)
The changes in [proof_of_concept_2.ino](https://github.com/DavesCodeMusings/BLE-Battery-Beacon/blob/main/proof_of_concept_2.ino) are my attempt to fix the problem of the battery constantly showing _unknown_.

I remember reading about how some battery-operated home automation devices will often send sensor readings in their BLE advertising messages. (I think it was a write-up concerning the stock firmware on the Xaiomi Mijia temperature / humidity sensors I have.) And, the BLE advertisement is what ESPHome was using for presence detection. Presence was the one entity in Home Asistant that was not showing _unknown_ when the beacon went to sleep.

This led me to looking for a way to communicate battery level information in the BLE advertisement. And it turns out there is a field called _manufacturer data_ that device makers (like Xaiomi) will use to send temperature and humidity readings along with their BLE advertisements. Unfortunately, there's no standard way of doing it. But, Arduino's BLE library includes the function `BLE.setManufacturerData()`for writing to this _manufacturer data_ field.

So in proof_of_concept_2.ino, I've created a string of ASCII characters that spells out _BATT:100%_ and stuck it in the manufacturer data field. Using a [BLE scanner](https://play.google.com/store/search?q=nrf+connect&c=apps) on my phone, I can see the advertisement from my ESP32 beacon. And if I switch it to a text representation of the manufacturer data, I see _BATT:100%_. Problem solved! Right...?

Maybe not. How can I read this from ESPHome to send to Home Assistant as an entity?

It's technically possible, but probably involves a lambda function to dig into some debug info. In short, it's not the simple solution I was hoping for.

## Third Try (Bargaining)
In looking for examples of sending sensor readings in the beacon's BLE advertisement, I stumbled upon the specification for [the format used by BTHome](https://bthome.io/format/). This project has already laid out their way of sending measurements in a BLE advertisement. So rather than creating my own, or trying to reverse-engineer some proprietary format like what's used by Xaiomi, I'll re-write my sketch to conform to BTHome's data format.

It looks like the BTHome data format can be implemented using the functions provided by the Arduino library, though I have yet to find any example code for that. But, as Brendon Urie of the band Panic at the Disco is fond of saying, "I've got high hopes!"

The advantage of this should be easy integration with ESPHome and Home Assistant. My Xaiomi Mijia devices are already flashed with a 3rd party firmware that uses BTHome (at least that's how they show up in Home Assistant.) And they _just work_. No lambda functions, etc. I'm hoping to have the same results when I'm done.

Oops! That didn't work.

Remember that scene in Star Wars when they're escaping the Death Star in the Millenium Falcon and Han says, "I sure hope the old man got that tractor beam out of commission or this is going to be a real short trip." Well guess what...?

It turns out BTHome sends its sensor updates in the _service data_ part of the advertisement, not the _manufacturer data_. Maybe the stock Xiaomi firmware was doing this too. I don't know. Why does it matter? Because the Arduino BLE library can write to _manufacturer data_, but I could not find any functions that would let me write arbitrary _service data_. So it's back to ESPHome lambda functions.

Fortunately, it's not too difficult to set up using the esp32_ble_tracker's [on_ble_manufacturer_data_advertise](https://esphome.io/components/esp32_ble_tracker.html#on-ble-manufacturer-data-advertise-trigger) trigger. I was able to pretty easily create an ESPHome configuration to send a mock value of 100% battery level to Home Assistant.

[esphome_config_3.yml](https://github.com/DavesCodeMusings/BLE-Battery-Beacon/blob/main/esphome_config_3.yml) contains this configuration.

## Fourth Try (Depression)
With [proof_of_concept_4.ino](https://github.com/DavesCodeMusings/BLE-Battery-Beacon/blob/main/proof_of_concept_4.ino), I created a battery variable that is decremented once a second by the timer interrupt. This is to simulate a draining battery instead of the constant 100% I was sending before. I also changed it to dynamically update the _manufacturer data_ part of the advertisement. And best of all, the changing battery percentage is showing up in my nRF Connect BLE scanner, just like I expected.

But ESPHome is still configured to send mock data of 100% all the time, so Home Assistant still reports 100%. My next task is to decode the data from the ESP32 beacon's advertisement and use it to update the Home Assistant entity.

It did not take long to realize I've made things difficult by storing the battery level as a human readable string, "BATT:100%". On the ESP32 side, I have an integer value that I've converted to a nine-byte string. And now in the ESPHome configuration, I'm finding I need to parse the string to get back to the integer value expected by Home Assistant. I should have left it alone and just put it in _manufacturer data_ as an integer.

And while I'm thinking about code changes, about half the lines in my C program are for dealing with client connections. Since a true beacon broadcasts it's data as part of the advertisement (and I've already determined Home Assistant does not deal well with the deep sleep disconnects when reading client data) there's really no need for my program to include logic for client connections at all.

So in the end, my fourth try has told me it's time for a bit of code clean-up.

## Fifth Try (Acceptance)
[proof_of_concept_5.ino](https://github.com/DavesCodeMusings/BLE-Battery-Beacon/blob/main/proof_of_concept_5.ino) really streamlines the code. And, I can still monitor the fictious battery level with nRF Connect. It's just a descending hex value instead of a more human-friendly string. But, that makes my ESPHome lambda function simpler and the task of getting data to Home Assistant easier.

The lambda function went from this mock-up...

```
id(esp32devkit1_battery).publish_state(100);
```

To this working sensor...

```
id(esp32devkit1_battery).publish_state(x[0]);
```

That's it! Changing the hard-coded mock value of 100 to the variable `x[0]` is all it takes. That's because for `on_ble_manufacturer_data_advertise:` ESPHome makes a variable (x) available that contains the _manufacturer data_ part of the BLE advertisement. The first two bytes for the manufacturer ID are left out, so `x[0]` corresponds to what my Arduino sketch calls `data[2]`, which is where I stashed the battery level as a single-byte value.

I can see it working by watching the ESPHome log output. A sample is shown below.

```
[01:13:22][D][sensor:094]: 'ESP32Devkit1 Battery': Sending state 62.00000 % with 0 decimals of accuracy
[01:13:22][D][sensor:094]: 'ESP32Devkit1 Battery': Sending state 62.00000 % with 0 decimals of accuracy
[01:13:24][D][sensor:094]: 'ESP32Devkit1 Battery': Sending state 60.00000 % with 0 decimals of accuracy
[01:13:25][D][sensor:094]: 'ESP32Devkit1 Battery': Sending state 59.00000 % with 0 decimals of accuracy
[01:13:27][D][sensor:094]: 'ESP32Devkit1 Battery': Sending state 57.00000 % with 0 decimals of accuracy
[01:13:28][D][sensor:094]: 'ESP32Devkit1 Battery': Sending state 56.00000 % with 0 decimals of accuracy
[01:13:28][D][sensor:094]: 'ESP32Devkit1 Battery': Sending state 56.00000 % with 0 decimals of accuracy
```

The battery level is descending just like it did when I watched the advertisements coming into nRF Connect.

Home Assistant is reporting the battery percentage as well. There's a little bit of lag, and the percentage drops in 2% or 3% increments after a bit of a pause. But, I've mocked up a battery that drops 1% every second, so it's not the most realistic situation to begin with.

Best of all, I have yet to see any _unknown_ values for my battery level. When the ESP32 goes into deep sleep, Home Assistant continues to report the last battery percentage reported. I'm sure there's a timeout in there somewhere and, if no advertisements are seen for a long while, it may go into an _unknown_ state.
