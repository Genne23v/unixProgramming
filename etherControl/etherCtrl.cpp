#include <errno.h>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iomanip>

#define NAME_SIZE 16

using namespace std;

int main()
{
    int fd;
    int ret;
    int selection;
    struct ifreq ifr;
    char if_name[NAME_SIZE];
    unsigned char *mac=NULL;
    struct sockaddr_in *addr=NULL;
    char ip_address[15];

    cout << "Enter the interface name: ";
    cin >> if_name;

    size_t if_name_len=strlen(if_name);
    if (if_name_len<sizeof(ifr.ifr_name)) {
        memcpy(ifr.ifr_name, if_name, if_name_len);
        ifr.ifr_name[if_name_len]=0;//NULL terminate
    } else {
        cout << "Interface name is too long!" << endl;
	    return -1;
    }

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd<0) {
        cout << strerror(errno);
	    return -1;
    }
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);

    system("clear");
    do {
        cout << "Choose from the following:" << endl;
        cout << "1. Hardware address" << endl;
        cout << "2. IP address" << endl;
        cout << "3. Network mask" << endl;
        cout << "4. Broadcast address" << endl;
        cout << "0. Exit" << endl << endl;
        cin >> selection;

        switch(selection) {
            case 1:
                ioctl(fd, SIOCGIFHWADDR, &ifr);
                for (int i = 0; i < 6; i++) {
                    cout << setw(2) << setfill('0') << hex << (unsigned int)(unsigned char)ifr.ifr_hwaddr.sa_data[i];
                    if (i < 5) {
                        cout << ":";
                    }
                }
                cout << endl;
                break;
            case 2:
                ioctl(fd, SIOCGIFADDR, &ifr);
                cout << "IP Address: " << inet_ntoa(((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr) << endl; 
                break;
            case 3:
                ioctl(fd, SIOCGIFNETMASK, &ifr);
                cout << "Network Mask: " << inet_ntoa(((struct sockaddr_in*)&ifr.ifr_netmask)->sin_addr) << endl; 
                break;
            case 4:
                ioctl(fd, SIOCGIFBRDADDR, &ifr);
                cout << "Broadcast Address: " << inet_ntoa(((struct sockaddr_in*)&ifr.ifr_broadaddr)->sin_addr) << endl;
                break;
            default:
                break;
            }
            
        if(selection!=0) {
            char key;
            cout << "Press any key to continue: ";
            cin >> key;
            system("clear");
        }
    } while (selection!=0);

    close(fd);
    return 0;
}
