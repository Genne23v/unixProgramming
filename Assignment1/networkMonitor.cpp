#include <fcntl.h>
#include <fstream>
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string>
#include <vector>

using namespace std;

const int MAXBUF = 512;    
vector<int> new_sock;
int numInterface = 0;
bool isRunning = true;

bool endsWithReady(const char* str);
void signal_handler(int sig);

int main() {
    char sock_file[] = "/tmp/network_monitor_socket_XXXXXX";
    int cli_len, serv_sock, temp, max_sd, numBytes; 
    struct sockaddr_un serv_addr, cli_addr;
    struct sigaction sa;
    
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) == -1){
        cerr << "sigaction error" << endl;
        exit(EXIT_FAILURE);
    }
    cout << "How many interfaces do you have to monitor? ";
    cin >> numInterface;
    new_sock.resize(numInterface, 0);

    char msg[numInterface][MAXBUF];
    pid_t child[numInterface];
    vector<string> interfaces(numInterface);
    for (int i=0; i<numInterface; ++i){
        cout << "Enter interface name to monitor: ";
        string name;
        cin >> name;
        interfaces[i] = name;
    }

    serv_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (serv_sock < 0){
        cerr << "Error while creating socket " << endl;
        exit(EXIT_FAILURE);
    }
    cout << "Socket created" << endl;

    int sockfd = mkstemp(sock_file);
    if (sockfd < 0){
        cerr << "Error while creating socket file" << endl;
        exit(EXIT_FAILURE);
    }

    int flags = fcntl(sockfd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    if (setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &flags, sizeof(flags)) < 0){
        cerr << "setsockopt error" << endl;
        exit(EXIT_FAILURE);
    }
    unlink(sock_file);
    serv_addr.sun_family = AF_UNIX;
    strncpy(serv_addr.sun_path, sock_file, sizeof(serv_addr.sun_path) -1);
    
    if (bind(serv_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
        cout << "Socket bound to " << sock_file << endl;
    }

    listen(serv_sock, 5);

    cout << "Waiting for incoming connection..." << endl;

    for(int i=0; i<numInterface; i++){
        child[i] = fork();
        if (child[i] == -1){
            cerr << "Process could not be forked" << endl;
            exit(EXIT_FAILURE);
        } else if (child[i] == 0) {
            execlp("./interfaceMonitor", "./interfaceMonitor", interfaces[i].c_str(), sock_file, NULL);            
        }
    }
    cout << "Initiated interface monitors" << endl;

    fd_set readfds, writefds;
    FD_ZERO(&readfds);
    FD_SET(serv_sock, &readfds);
    cli_len = sizeof(cli_addr);
    max_sd = serv_sock;
    for(int i=0; i<numInterface; i++){
        temp = accept(serv_sock, (struct sockaddr*)&cli_addr, (socklen_t*)&cli_len);
        if (temp < 0){
           cerr << "Error while accepting connection" << endl;
           exit(EXIT_FAILURE);
        }
        FD_SET(temp, &readfds);
        new_sock[i] = temp;

        if (temp> max_sd){
            max_sd = temp;
        }
        cout << "Connection accepted via socket: " << new_sock[i] << endl;
    }

    while(isRunning){        
        // FD_ZERO(&readfds);
        // FD_SET(serv_sock, &readfds);        
        int ready = select(max_sd+1, &readfds, NULL, NULL, NULL);
        // cout << "Loop start" <<endl;
        if (ready < 0){
            cerr << "select error" << endl;
        }
        if (ready > 0 ){
            const char monitor[] = "Monitor";
            for(int i=0; i<numInterface; i++){
                memset(msg[i], 0, MAXBUF);
                if(FD_ISSET(new_sock[i], &readfds)){
                    numBytes = read(new_sock[i], msg[i], sizeof(msg[i]));
                    if (numBytes > 0){
                        if (endsWithReady(msg[i])){
                            send(new_sock[i], monitor, sizeof(monitor), 0);
                        } else if (strcmp(msg[i], "Link Down") == 0){
                            const char link_up[] = "Set Link Up";
                            send(new_sock[i], link_up, sizeof(link_up), 0);
                        } else if (strcmp(msg[i], "Done") == 0){
                            isRunning = false;
                        } else {
                            send(new_sock[i], monitor, sizeof(monitor), 0);
                        }
                        cout << "<Socket " << new_sock[i] << ">" << endl;
                        cout << msg[i] << endl;
                    } else if (numBytes == 0){
                        // FD_CLR(new_sock[i], &readfds);
                    } else {
                        cerr << "read error" << endl;
                        // isRunning = false;
                    }
                }
            }
        }
        if (ready == 0){
            cout << "Wating for something..."<< endl;
        }
        sleep(1); 
    }
    
//You might prefer that the network monitor send a SIGINT to each interface monitor instead. 
//Or you might want to set the socket read on the interface monitor 
//as non-blocking or blocking with a timeout using setsockopt.
    cout << "Socket is being closed" << endl;
    close(sockfd);

    return 0;
}

bool endsWithReady(const char* cstr){
    string str(cstr);
    string end = str.substr(str.length() -5);
    return end == "ready";
}

void signal_handler(int sig)
{
    const char* shutdown = "Shut Down";
    char temp[MAXBUF];
    switch (sig)
    {
        case SIGINT:
        cout<<"Shutting down" << endl;
            for(int i=0; i<numInterface; i++){
                read(new_sock[i], temp, sizeof(temp));
                cout << "last msg: " << temp <<endl;
                send(new_sock[i], shutdown, sizeof(shutdown), 0);
            }
            break;
        default:
            cout << "interface monitor: undefined signal" << endl;
            break;
    }
}
