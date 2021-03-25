#include <zmq.hpp>
#include <string>
#include <iostream>
#include <unistd.h>
// #include "uuid.hpp"
using namespace std;

#if __cplusplus <= 199711L
  #error This library needs at least a C++11 compliant compiler
#endif

inline static std::string
s_recv (zmq::socket_t & socket, int flags = 0) {

    zmq::message_t message;
    socket.recv(&message, flags);

    return std::string(static_cast<char*>(message.data()), message.size());
}

inline static bool
s_send (zmq::socket_t & socket, const std::string & string, int flags = 0) {

    zmq::message_t message(string.size());
    memcpy(message.data(), string.data(), string.size());

    bool rc = socket.send (message, flags);
    return (rc);
}

inline static bool
s_sendmore (zmq::socket_t & socket, const std::string & string) {

    zmq::message_t message(string.size());
    memcpy(message.data(), string.data(), string.size());

    bool rc = socket.send (message, ZMQ_SNDMORE);
    return (rc);
}

int main (int argc, char *argv[])
{

    string id = "client_1";
    zmq::context_t context(1);
    zmq::socket_t client(context, ZMQ_DEALER);

    const string endpoint = "tcp://localhost:5580";

    client.setsockopt(ZMQ_IDENTITY, "client_1", 8);
    client.connect(endpoint);
    cout << id << " connecting to Router " << endpoint << "..." << endl;

    //s_sendmore(client, "");
    //s_send(client, "Hello");

    //std::string string = s_recv(client);

    //std::cout << "Received reply " << " [" << string << "]" << std::endl;

    bool workerIsStarted = false;
    cout << "Waiting for worker..." << endl;

    bool isStarted = true;

    while (isStarted == true)
    {
        while (workerIsStarted == false)
        {
            s_send(client,"Client Ready");
            string msg = s_recv(client);
            cout << " received: " + msg << endl;
            if (msg == "Worker Ready") {
                workerIsStarted = true;
                break;
            }
            usleep(1000);
        }
        // Send
        s_send(client,"Hello",5);
        // Receive
        string msg = s_recv(client);
        cout << id << " received: " << msg << endl;
        
        usleep(1000);
    }

    cout << "Dear worker, I don't feel so good" << endl;
    cout << "Stopping " << id << endl;
    s_send(client,"Client Stopping");

    return 0;

}