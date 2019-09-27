# BlueRobotics Basic ESC Control using Beaglebone Black

Beaglebone Black installed with Debian 9.9 from:

https://beagleboard.org/getting-started

using image from:

https://beagleboard.org/latest-images

Image already comes with Adafruit_BBIO, which we use for direct ESC control.  ESC controller is connected to T100 thrusters according to color code.  Power to ESC was set to 14V, PWM signal connects to pin P9_22 (white), and digital ground (black).  Motor commands can be sent using:

```sh
from Adafruit_BBIO import PWM
PWM.start("P9_22", 3, 2000)
#(pause for 7 seconds or so)
PWM.set_duty_cycle("P9_22", 3.2) #reverse thrust
PWM.set_duty_cycle("P9_22", 3.0) #stop
PWM.set_duty_cycle("P9_22", 2.8) #forward thrust
PWM.stop("P9_22") #stop PWM
PWM.cleanup()
```
