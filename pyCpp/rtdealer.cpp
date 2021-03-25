//
//  Custom routing Router to Dealer
//

#include <zmq.hpp>
#include <string>
#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <iomanip>
#include <sstream>

#include <time.h>
#include <assert.h>
#include <stdlib.h>        // random()  RAND_MAX
#include <stdio.h>
#include <stdarg.h>
#include <signal.h>

#include <sys/time.h>

// #define _BSD_SOURCE

#define within(num) (int) ((float)((num) * random ()) / (RAND_MAX + 1.0))

inline static int64_t
s_clock (void)
{
#if (defined (WIN32))
	FILETIME fileTime;
	GetSystemTimeAsFileTime(&fileTime);
	unsigned __int64 largeInt = fileTime.dwHighDateTime;
	largeInt <<= 32;
	largeInt |= fileTime.dwLowDateTime;
	largeInt /= 10000; // FILETIME is in units of 100 nanoseconds
	return (int64_t)largeInt;
#else
    struct timeval tv;
    gettimeofday (&tv, NULL);
    return (int64_t) (tv.tv_sec * 1000 + tv.tv_usec / 1000);
#endif
}

inline static void
s_sleep (int msecs)
{
#if (defined (WIN32))
    Sleep (msecs);
#else
    struct timespec t;
    t.tv_sec = msecs / 1000;
    t.tv_nsec = (msecs % 1000) * 1000000;
    nanosleep (&t, NULL);
#endif
}

inline std::string
s_set_id (zmq::socket_t & socket)
{
    std::stringstream ss;
    ss << std::hex << std::uppercase
       << std::setw(4) << std::setfill('0') << within (0x10000) << "-"
       << std::setw(4) << std::setfill('0') << within (0x10000);
    socket.setsockopt(ZMQ_IDENTITY, ss.str().c_str(), ss.str().length());
    return ss.str();
}

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

//  Sends string as 0MQ string, as multipart non-terminal
inline static bool
s_sendmore (zmq::socket_t & socket, const std::string & string) {

    zmq::message_t message(string.size());
    memcpy (message.data(), string.data(), string.size());

    bool rc = socket.send (message, ZMQ_SNDMORE);
    return (rc);
}



static void *
worker_task(void *args)
{
    zmq::context_t context(1);
    zmq::socket_t worker(context, ZMQ_DEALER);

#if (defined (WIN32))
    s_set_id(worker, (intptr_t)args);
#else
    s_set_id(worker);          //  Set a printable identity
#endif

    worker.connect("tcp://localhost:5671");

    int total = 0;
    while (1) {
        //  Tell the broker we're ready for work
        s_sendmore(worker, "");
        s_send(worker, "Hi Boss");

        //  Get workload from broker, until finished
        s_recv(worker);     //  Envelope delimiter
        std::string workload = s_recv(worker);
        //  .skip
        if ("Fired!" == workload) {
            std::cout << "Completed: " << total << " tasks" << std::endl;
            break;
        }
        total++;

        //  Do some random work
        s_sleep(within(500) + 1);
    }

    return NULL;
}

//  .split main task
//  While this example runs in a single process, that is just to make
//  it easier to start and stop the example. Each thread has its own
//  context and conceptually acts as a separate process.
int main() {
    zmq::context_t context(1);
    zmq::socket_t broker(context, ZMQ_ROUTER);

    broker.bind("tcp://*:5671");
    srandom((unsigned)time(NULL));

    const int NBR_WORKERS = 10;
    pthread_t workers[NBR_WORKERS];
    for (int worker_nbr = 0; worker_nbr < NBR_WORKERS; ++worker_nbr) {
        pthread_create(workers + worker_nbr, NULL, worker_task, (void *)(intptr_t)worker_nbr);
    }


    //  Run for five seconds and then tell workers to end
    int64_t end_time = s_clock() + 5000;
    int workers_fired = 0;
    while (1) {
        //  Next message gives us least recently used worker
        std::string identity = s_recv(broker);
        {
            s_recv(broker);     //  Envelope delimiter
            s_recv(broker);     //  Response from worker
        }

        s_sendmore(broker, identity);
        s_sendmore(broker, "");

        //  Encourage workers until it's time to fire them
        if (s_clock() < end_time)
            s_send(broker, "Work harder");
        else {
            s_send(broker, "Fired!");
            if (++workers_fired == NBR_WORKERS)
                break;
        }
    }

    for (int worker_nbr = 0; worker_nbr < NBR_WORKERS; ++worker_nbr) {
        pthread_join(workers[worker_nbr], NULL);
    }

    return 0;
}