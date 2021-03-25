#include <zmq.hpp>
#include <string>
#include <iostream>
#include <unistd.h>

inline static bool
s_send (zmq::socket_t & socket, const std::string & string, int flags = 0) {

    zmq::message_t message(string.size());
    memcpy (message.data(), string.data(), string.size());

    bool rc = socket.send (message, flags);
    return (rc);
}

inline static void
s_dump (zmq::socket_t & socket)
{
    std::cout << "----------------------------------------" << std::endl;

    while (1) {
        //  Process all parts of the message
        zmq::message_t message;
        socket.recv(&message);

        //  Dump the message as text or binary
        size_t size = message.size();
        std::string data(static_cast<char*>(message.data()), size);

        bool is_text = true;

        size_t char_nbr;
        unsigned char byte;
        for (char_nbr = 0; char_nbr < size; char_nbr++) {
            byte = data [char_nbr];
            if (byte < 32 || byte > 127)
                is_text = false;
        }
        std::cout << "[" << std::setfill('0') << std::setw(3) << size << "]";
        for (char_nbr = 0; char_nbr < size; char_nbr++) {
            if (is_text)
                std::cout << (char)data [char_nbr];
            else
                std::cout << std::setfill('0') << std::setw(2)
                   << std::hex << (unsigned int) data [char_nbr];
        }
        std::cout << std::endl;

        int more = 0;           //  Multipart detection
        size_t more_size = sizeof (more);
        socket.getsockopt (ZMQ_RCVMORE, &more, &more_size);
        if (!more)
            break;              //  Last message part
    }
}

int main () {
    zmq::context_t context(1);

    zmq::socket_t sink(context, ZMQ_ROUTER);
    sink.bind( "inproc://example");

    //  First allow 0MQ to set the identity
    zmq::socket_t anonymous(context, ZMQ_REQ);
    anonymous.connect( "inproc://example");

    s_send (anonymous, "ROUTER uses a generated 5 byte identity");
    s_dump (sink);

    //  Then set the identity ourselves
    zmq::socket_t identified (context, ZMQ_REQ);
    identified.setsockopt( ZMQ_IDENTITY, "PEER2", 5);
    identified.connect( "inproc://example");

    s_send (identified, "ROUTER socket uses REQ's socket identity");
    s_dump (sink);

    return 0;

}