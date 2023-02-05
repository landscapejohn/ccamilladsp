#include "cCamillaDSP.hpp"

int main(int argc, char **argv)
{
    try
    {
        int port = stoi(argv[1]);
        float new_vol = stof(argv[2]);

        connect("127.0.0.1", port);
        float current_vol = get_volume();

        printf("Current volume: {%f} dB\n", current_vol);
        printf("Changing volume to: {%f} dB\n", new_vol);
        set_volume(new_vol);

        if (is_connected())
        {
            disconnect();
        }
    }
    catch (...)
    {
        printf("Usage: Make sure that your pipeline includes Volume filters for each channel.\n");
        printf("Then start CamillaDSP with the websocket server enabled:\n");
        printf("\t> camilladsp -p4321 yourconfig.yml\n");
        printf("Then set the volume\n");
        printf("\t> ./set_volume 4321 -12.3\n");
    }

}
