#include <iostream>

#include <dht22.h>
#include <sleep.h>

int main(void)
{
  try {
    DHT22 dht22("/dev/gpiochip0", 4);

    do{
      dht22.UpdateTimed();
      std::cout << dht22.Reads() << " (" << dht22.Reads() << "/" << dht22.BadReads() << " bad): " << dht22.Temperature() << " " << dht22.Humidity() << std::endl;
    }
    while (true);

    return 0;
  }
  catch (...) {
    std::cout << "Caught exception" << std::endl;
    return 1;
  }
}