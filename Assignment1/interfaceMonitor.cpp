#include <fcntl.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>

using namespace std;

const int MAXBUF=512;
bool isRunning=false;

int main(int argc, char *argv[])
{
    struct ifreq ifr;
    char interface[MAXBUF];
    char statPath[MAXBUF];
    int retVal=0;
    int sockfd, n;
    char msg[MAXBUF];
    
    strncpy(interface, argv[1], MAXBUF);

    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0){
        cerr << "Error while creating socket" << endl;
        return 1;
    }
    
    struct sockaddr_un serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, argv[2]);

    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
        cerr << "Error connecting to socket" << endl;
        close(sockfd);
        return 1;
    }
    cout << "Socket for interface " << argv[1] << " connected" << endl;
    
    char ready_msg[64] = "interface ";
    strcat(ready_msg, interface);
    strcat(ready_msg, " ready");
    send(sockfd, ready_msg, strlen(ready_msg), 0);

    while(1){
        recv(sockfd, msg, sizeof(msg), 0);
        if (strcmp(msg, "Monitor") == 0){
            isRunning = true;
            const char monitoring[] = "Monitoring";
            send(sockfd, monitoring, sizeof(monitoring), 0);
            break;
        }
    }

    bool msgSent = false;
    bool isDown = false;
    while(isRunning) {
        string state;
        int carrier_up_count=0;
        int carrier_down_count=0;
        int rx_bytes=0;
        int rx_dropped=0;
        int rx_errors=0;
        int rx_packets=0;
        int tx_bytes=0;
        int tx_dropped=0;
        int tx_errors=0;
        int tx_packets=0;
        isDown = false;
        msgSent = false;

        memset(msg, 0, MAXBUF);
        recv(sockfd, msg, sizeof(msg), 0);
        cout << interface << " recv msg: " << msg <<endl;
        if (strcmp(msg, "Set Link Up") == 0){
            memset(&ifr, 0, sizeof(ifr));
            strncpy(ifr.ifr_name, interface, IFNAMSIZ);
            
            if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0){
                cerr << "ioctl error" << endl;
                strcpy(msg, "ioctl error");
                send(sockfd, msg, sizeof(msg), 0);
                msgSent = true;
            };
            ifr.ifr_flags |= IFF_UP;
            if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0){
                cerr << "ioctl error" << endl;
                strcpy(msg, "ioctl error");
                send(sockfd, msg, sizeof(msg), 0);
                msgSent = true;
            } else {
                carrier_up_count++;
            }
        }

        ifstream infile;
        sprintf(statPath, "/sys/class/net/%s/operstate", interface);
        infile.open(statPath);
        if(infile.is_open()){
            getline(infile, state);
            infile.close();
        }
        if (state.compare("down") == 0){
            memset(msg, 0, MAXBUF);
            strcpy(msg, "Link Down");
            send(sockfd, msg, strlen(msg), 0);
            isDown = true;
            msgSent = true;
        }
        sprintf(statPath, "/sys/class/net/%s/carrier_up_count", interface);
        infile.open(statPath);
	    if(infile.is_open()) {
	        infile>>carrier_up_count;
	        infile.close();
	    }
        sprintf(statPath, "/sys/class/net/%s/carrier_down_count", interface);
        infile.open(statPath);
	    if(infile.is_open()) {
	        infile>>carrier_down_count;
	        infile.close();
	    }
        if(isDown == true){
            ofstream outfile(statPath);
            if (outfile.is_open()){
                carrier_down_count++;
                outfile << carrier_down_count;
                outfile.close();
            }
        }

        sprintf(statPath, "/sys/class/net/%s/statistics/rx_bytes", interface);
	    infile.open(statPath);
	    if(infile.is_open()) {
	        infile>>rx_bytes;
	        infile.close();
	    }
        sprintf(statPath, "/sys/class/net/%s/statistics/rx_dropped", interface);
	    infile.open(statPath);
	    if(infile.is_open()) {
	        infile>>rx_dropped;
	        infile.close();
	    }
        sprintf(statPath, "/sys/class/net/%s/statistics/rx_errors", interface);
	    infile.open(statPath);
	    if(infile.is_open()) {
	        infile>>rx_errors;
	        infile.close();
	    }
        sprintf(statPath, "/sys/class/net/%s/statistics/rx_packets", interface);
	    infile.open(statPath);
	    if(infile.is_open()) {
	        infile>>rx_packets;
	        infile.close();
	    }
        
        sprintf(statPath, "/sys/class/net/%s/statistics/tx_bytes", interface);
	    infile.open(statPath);
	    if(infile.is_open()) {
	        infile>>tx_bytes;
	        infile.close();
	    }
        sprintf(statPath, "/sys/class/net/%s/statistics/tx_dropped", interface);
	    infile.open(statPath);
	    if(infile.is_open()) {
	        infile>>tx_dropped;
	        infile.close();
	    }
        sprintf(statPath, "/sys/class/net/%s/statistics/tx_errors", interface);
	    infile.open(statPath);
	    if(infile.is_open()) {
	        infile>>tx_errors;
	        infile.close();
	    }
        sprintf(statPath, "/sys/class/net/%s/statistics/tx_packets", interface);
	    infile.open(statPath);
	    if(infile.is_open()) {
	        infile>>tx_packets;
	        infile.close();
	    }

        if (strcmp(msg, "Monitor") == 0 || msgSent == false){
            char data[MAXBUF];
            int len = sprintf(data, "Interface: %s state: %s up_count: %d down_count: %d\nrx_bytes: %d rx_dropped: %d rx_errors: %d rx_packets: %d\ntx_bytes: %d tx_dropped: %d tx_errors: %d tx_packets: %d\n", interface, state.c_str(), carrier_up_count, carrier_down_count, rx_bytes, rx_dropped, rx_errors, rx_packets, tx_bytes, tx_dropped, tx_dropped, tx_packets);
            send(sockfd, data, len, 0);
        }

        if (strcmp(msg, "Shut Down") == 0){
            cout << "Shutting down child socket"<<endl;
            const char done[] = "Done";
            send(sockfd, done, sizeof(done), 0);
            isRunning = false;
            msgSent = true;
        }    
        sleep(1);
    }
    cout << "Child socket closed" << endl;
    close(sockfd);
    return 0;
}
