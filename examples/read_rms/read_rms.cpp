#include "cCamillaDSP.hpp"
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif
#include <iostream>
int main(int argc, char **argv)
{
    try
    {
        int port = stoi(argv[1]);

        connect("127.0.0.1", port);

        printf("Reading playback signal RMS, press Ctrl+c to stop\n");

        list<float> playback_rms;
        float channel_1_rms, channel_2_rms;

        while (true)
        {
            playback_rms = get_playback_signal_rms();

            auto pb_front = playback_rms.begin();
            channel_1_rms = *pb_front;
            advance(pb_front, 1);
            channel_2_rms = *pb_front;

            printf("Channel 1: %f\tChannel 2: %f\n", channel_1_rms, channel_2_rms);
            sleep(1);
        }

        if (is_connected())
        {
            disconnect();
        }
    }
    catch (...)
    {
        printf("Usage: start CamillaDSP with the socketserver enabled:\n");
        printf("\t> camilladsp -p4321 yourconfig.yml\n");
        printf("Then read the signal level\n");
        printf("\t> ./read_rms 4321\n");
    }
}
