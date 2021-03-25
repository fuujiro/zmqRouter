import signal
import sys
import threading
import time
import uuid

import zmq


# Client Class
class Client(threading.Thread):
    def __init__(self, id):
        threading.Thread.__init__(self)
        self.id = ("client_"+str(id))

    def run(self):
        # Connect
        context = zmq.Context()
        socket = context.socket(zmq.DEALER)
        socket.identity = self.id.encode("ascii")
        socket.connect("tcp://localhost:5580")
        print(self.id + " started")
        
        workerIsStarted = False
        print("Waiting for worker...")
        
        self.isStarted = True
        
        # Run Loop
        while self.isStarted:
            # Wait for worker
            while not workerIsStarted:
                socket.send(("Client Ready").encode())
                msg = socket.recv_string()
                print(self.id + " received: " + msg)
                if msg == "Worker Ready":
                    workerIsStarted = True
                    break
                time.sleep(1)
            # Send
            socket.send(("Hello from " + self.id).encode())
            # Receive
            msg = socket.recv_string()
            print(self.id + " received: " + msg + "\n")
            
            time.sleep(1)

        # Clean Up
        print("Dear worker, I don't feel so good")
        print("Stopping " + str(self.id) + "\n")
        socket.send(("Client Stopping").encode())
        socket.close()
        context.term()

            
    # Send a single message to worker
    def send(self, msg):
        # Connect
        context = zmq.Context()
        socket = context.socket(zmq.DEALER)
        socket.identity = self.id.encode("ascii")
        socket.connect("tcp://localhost:5580")
        
        # Send Msg
        socket.send(msg.encode())
        
        # Clean Up
        socket.close()
        context.term()

def test(self, id):

    client = Client(id)
    client.start()

    def signal_handler(*args):
        print("\n\nReceived SIGINT")
        client.isStarted = False
        # client.join()
        sys.exit(0)

    signal.signal(signal.SIGINT, signal_handler)  


# Main
if __name__ == "__main__":
    test()

