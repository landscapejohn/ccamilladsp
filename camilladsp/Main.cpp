#ifdef _WIN32
#pragma comment( lib, "ws2_32" )
#include <WinSock2.h>
#endif
#include <stdio.h>
#include <iostream>
#include <string>
#include "cCamillaDSP.hpp"

int main()
{
#ifdef _WIN32
    INT rc;
    WSADATA wsaData;

    rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (rc) {
        printf("WSAStartup Failed.\n");
        return 1;
    }
#endif

    connect("192.168.1.113", 1234);

    tuple<int, int, int> library_v = get_library_version();
    tuple<int, int, int> cdsp_v = get_version();

    cout << "Library version: " + to_string(get<0>(library_v)) + "." + to_string(get<1>(library_v)) + "." + to_string(get<2>(library_v)) + "\n";
    cout << "CamillaDSP version: " + to_string(get<0>(cdsp_v)) + "." + to_string(get<1>(cdsp_v)) + "." + to_string(get<2>(cdsp_v)) + "\n";

    if (is_connected())
    {
        disconnect();
    }
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
