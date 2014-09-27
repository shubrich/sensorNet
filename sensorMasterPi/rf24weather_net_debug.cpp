#include <cstdlib>
#include <iostream>
#include <RF24/RF24.h>
#include <RF24Network/RF24Network.h>

using namespace std;

// init network
RF24 radio(RPI_V2_GPIO_P1_15, BCM2835_SPI_CS0, BCM2835_SPI_SPEED_8MHZ);
RF24Network network(radio);
const uint16_t this_node = 0;

// define struct for char array
struct payload_t
{
    char readings[32];
};

int main(int argc, char** argv)
{
    // init radio
    radio.begin();
    delay(5);
    network.begin(90, this_node);
    radio.setDataRate(RF24_250KBPS);
    radio.setPALevel(RF24_PA_MAX);
    cout("Start listenting");

    while(1)
    {
        // pump network
        network.update();
        while (network.available())
        {
            // get payload
            RF24NetworkHeader header;
            payload_t payload;
            network.read(header,&payload,sizeof(payload));
            
            // get time - can be passed to time.localtime()
            time_t rawtime;
            time (&rawtime);

            // print payload and time
            cout << payload.readings << ',' << rawtime << endl;
        }
        sleep(1);
    }
    return 0;
}
