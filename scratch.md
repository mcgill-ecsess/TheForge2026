## Controller library for Arduino R4

To install the library, download the `Controller` folder and import it into your Arduino IDE by going to Sketch > Include Library > Add .ZIP Library and selecting the downloaded folder.

Mac OS users should look for the following tab (similar on Windows / Linux)

![Image include](scratch.png)

To include the library into your sketch, you need the following statement near the top of your sketch : 

```cpp
#include <Controller.h>
```

This will make sure Arduino links the library to your sketch and allows you to use the functions defined in the library.

The library utilizes the Arduino R4 WiFi capabilities to create a web interface for controlling the robot. The library defines a `Controller` class that can be instantiated in your sketch to create a controller object. The constructor of the `Controller` class takes two parameters : the SSID and password of the WiFi network you want to connect to.

To initialize the controller, you can use the following code in your `setup()` function : 

```cpp
Controller controller("SSID", "password");
```

