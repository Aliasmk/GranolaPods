# GranolaPods :: Frontend Website, Backend Server, Unity Module Hardware and Firmware

The Unity Module is an I2C-based modular control board for the GranolaPods smart trail-mix making machine.
This was developed for University of Waterloo Mechatronics FYDP team #26.

Why the name "Unity"? The board and code is designed to be modular - all devices in the GranolaPods system (e.g. the Gantry Cart, the Dispensers, and any future devices such as mixers or binders) use the same PCB and the same code.

This repo contains the PCB and code for the Unity modules, as well as major updates to the frontend web interface and the backend server.
The front end and server are both run locally on a Raspberry Pi. In the GranolaPods system we are using a 7" touchscreen that will display the user interface via a web browser. Submitting a request will generate an HTTP POST, and pass values to the python backend. The backend running on the Pi will sequentially coordinate the Unity modules down the chain to create the snack. Communication between the Pi master and n Unity slaves are done with i2c.

The Unity modules are powered by an ATmega328P. Programming is done through the Arduino IDE via ICSP pins.

To see how the GranolaPods system was envisioned to work check out [this video](https://www.youtube.com/watch?v=Usi5u81KuWI). Unfortunatly due to time constraints we had to reduce the scope and remove the mixing and binding steps and make trail mix rather than granola bars.

For a demonstration of the Unity system you can check out [this video](https://www.youtube.com/watch?v=NxC_GapGs20).

To run the server, simply run ```python ./server/server.py```, then navigate to ```localhost```.

Currently the system will crash if a GranolaPod is requested with no Unity boards attached. It would probably crash even more spectacularly if you tried running it on something other than a Raspberry Pi.
