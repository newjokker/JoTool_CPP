


#include <iostream>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "ucd_tools.hpp"
#include <netdb.h>



std::string get_ip_address() 
{
    struct ifaddrs *ifaddr, *ifa;
    int family, s;
    char host[NI_MAXHOST];
    std::string ip_address;

    if (getifaddrs(&ifaddr) == -1) 
    {
        std::cerr << "Failed to get network interface information." << std::endl;
        return ip_address;
    }

    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr)
            continue;

        family = ifa->ifa_addr->sa_family;

        if (family == AF_INET) {
            s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, nullptr, 0, NI_NUMERICHOST);
            if (s != 0) {
                std::cerr << "Failed to get IP address: " << gai_strerror(s) << std::endl;
                return ip_address;
            }

            if (std::strcmp(ifa->ifa_name, "lo") != 0) {
                ip_address = host;
                break;
            }
        }
    }

    freeifaddrs(ifaddr);

    return ip_address;
}






