# BLE Battery Level Beacon
This Arduino sketch shows how to set up a generic ESP32 as a Bluetooth Low Energy (BLE) beacon with deep sleep capability. All it does is announce itself and offer to communicate a fictitious battery level characteristic. As configured, it will stay up and accept connections for one minute. After that, it goes into deep sleep for four minutes, wakes up, and repeats. Sleep and wake times are easily configurable with #define statements at the top of the code.

See the [Bill of Materials](Bill_Of_Materials.txt) for the parts and software libraries required.

## Why?
I've been setting up Home Assistant with various devices around the house and I can see myself eventually wanting to communicate with sensors that are outdoors. I have a handful of ESP32s and a 3D printer. I'm ready to build some stuff.

Just use [ESPHome](esphome.io), you say! It's easy! I do. I've got Bluetooth Proxy running on an ESP32 flashed with ESPHome. But that's inside the house. I'm not thrilled with the idea of having a device with my WiFi credentials on it that's not physically secured inside my home.

Use [BLEHome](https://bthome.io/) then! No WiFi required! Sure. That Bluetooth Proxy I have running on ESPHome is reporting temperature and humidity from Xaiomi sensors flashed with BLEHome firmware. It's fabulous... until I have to replace batteries. And the Xaiomi devices aren't built to stand up to the elements.

## What do I want?
Since I've already dismissed ESPHome and BLEHome as possible solutions, and I'm ready to DIY, what exactly is it I'm looking for? Let's start with the requirements.
* I want a rugged device I can plug into AC outlets on my front and back porch so I don't have to constantly toss CR2032 coin cells in the landfill.
* I want it to be cheap in case it gets chewed on by squirrels, run over by a lawn mower, or pooped on by birds.
* I don't want my WiFi credentials on it in case it gets stolen by a roving gang of juvenile hacker wannabes.
* I want to be able to outfit it with sensors that will stand up to the extremes of Wisconsin winters.

## What use case will this battery level indicator solve?
The battery level beacon is a proof of concept. At this point, there's not even a battery to report the level of. It just returns 100% all the time. But, it should prove a few design points.
* Test do-it-yourself BLE device integration with Home Assistant.
* Test ESP deep sleep for power conservation and self-heating mitigation.
* Test Home Assistant's tolerance for an intermittently available device.
* Test the signal range for BLE devices that are outdoors.

## What's Next?
First in my plans is an outdoor temperature & humidity sensor. There are easily accessible outdoor outlets, but the real test will be how well the power adapters hold up to the temperature extremes. (The ESP32 itself is rated for -40C to +125C, so no problems there.) There's also the task of designing a suitable 3D printed enclosure: something that won't trap heat, but still keeps the rain and snow off the electronics.

The next project will probably be a vehicle presence detector using an ESP32 with a rechargable Lithium Polymer battery plugged into the car. The LiPo battery will charge when the car is running, but how long will it last if the car is parked? How will it hold up in the summer heat and winter cold?

But first, the proof of concept battery indicator.
