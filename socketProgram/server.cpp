#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

using namespace std;

char socket_path[] = "/tmp/lab6";
bool is_running;
const int BUF_LEN=256;

int main(int argc, char *argv[])
{
    struct sockaddr_un addr;
    // socklen_t cli_len;
    char buf[BUF_LEN];
    int fd, cli, rc;
    bool isRunning = true;
    
    memset(&addr, 0, sizeof(addr));
    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
        cout << "server: " << strerror(errno) << endl;
        exit(-1);
    }

    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) -1);
    unlink(socket_path);

    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        cout << "server: " << strerror(errno) << endl;
        close(fd);
        exit(-1);
    }

    if (listen(fd, 5) == -1)
    {
        cout << "server: " << strerror(errno) << endl;
        unlink(socket_path);
        close(fd);
        exit(-1);
    }

    cout << "Waiting for the client..." << endl;

    if ((cli = accept(fd, NULL, NULL)) == -1)
    {
        cout << "server: " << strerror(errno) << endl;
        unlink(socket_path);
        close(fd);
        exit(-1);
    } 
    cout << "client connected to the server" << endl;
    cout << "server: accept()" << endl;
    
    cout << "The server requests the client's pid" << endl;
    char msg[] = "Pid";
    strncpy(buf, msg, sizeof(msg));
    send(cli, buf, strlen(buf), 0);

    rc = recv(cli, buf, sizeof(buf), 0);
    if (rc < 0) 
    {
        cerr << "Error receving data" << strerror(errno) << endl;
    }
    else if (rc == 0)
    {
        cout << "Client socket has closed the connection" << endl;
    }
    else 
    {
        cout << "server: " << buf << endl;
    }

    cout << "The server requests the client to sleep" << endl;
    memset(buf, 0, sizeof(buf));
    strncpy(buf, "Sleep", 5);
    send(cli, buf, strlen(buf), 0);

    rc = recv(cli, buf, sizeof(buf), 0);
    if (rc < 0) 
    {
        cerr << "Error receving data" << strerror(errno) << endl;
    }
    else if (rc == 0)
    {
        cout << "Client socket has closed the connection" << endl;
    }
    else 
    {
        if ((strncmp(buf, "Done", 4)) == 0)
        {
            cout << "The server requests the client to quit" << endl;
            send(cli, "Quit", 4, 0);
        }
    }
    
    close(fd);
    unlink(socket_path);

    return 0;
}