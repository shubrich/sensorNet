#include <cstdlib>
#include <iostream>
#include <fstream>
#include <RF24/RF24.h>
#include <RF24Network/RF24Network.h>

// g++ -Ofast -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s -L/usr/local/lib -I/usr/local/include -lrf24-bcm -lrf24network ./rf24weather_net.cpp -o rf24weather_net
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

            // write to file and close
            ofstream myfile;
            myfile.open ("/var/tmp/rf24weather.csv");
            myfile << payload.readings << ',' << rawtime << endl;
            myfile.close();
        }
        // pause for a minute to reduce cpu/disk load
        sleep(60);
    }
    return 0;
}