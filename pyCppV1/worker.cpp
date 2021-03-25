#include <zmq.hpp>
#include <string>
#include <iostream>
#include <unistd.h>
// #include "uuid.hpp"
using namespace std;

inline static std::string
s_recv (zmq::socket_t & socket, int flags = 0) {

    zmq::message_t message;
    socket.recv(&message, flags);

    return std::string(static_cast<char*>(message.data()), message.size());
}

inline static bool
s_send (zmq::socket_t & socket, const std::string & string, int flags = 0) {

    zmq::message_t message(string.size());
    memcpy (message.data(), string.data(), string.size());

    bool rc = socket.send (message, flags);
    return (rc);
}

inline static bool
s_sendmore (zmq::socket_t & socket, const std::string & string) {

    zmq::message_t message(string.size());
    memcpy (message.data(), string.data(), string.size());

    bool rc = socket.send (message, ZMQ_SNDMORE);
    return (rc);
}

int main(int argc, char *argv[])
{

    string id = "worker_1";
    zmq::context_t context(1);
    zmq::socket_t worker(context, ZMQ_DEALER);

    const string endpoint = "tcp://localhost:5581";

    worker.setsockopt(ZMQ_IDENTITY, "worker_1", 8);
    worker.connect(endpoint);
    cout << id << " connecting to Router " << endpoint << "..." << endl;

    while (true)
    {
        string msg = s_recv(worker);
        cout << id << " received: " << msg << endl;
        if (msg == "Client Ready") {
            s_send(worker,"Worker Ready");
        }
        if (msg == "Client Stopping")
        {
            break;
        } else
        {
            s_send(worker,"World from " + id);
        }
        cout<<""<<endl;
    }

    cout << "Stopping " << id;
    
}