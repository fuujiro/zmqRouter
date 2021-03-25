import sys
import threading
import time
import ctypes
# 指定动态链接库
lib = ctypes.cdll.LoadLibrary('./workerEntity.so')
#需要指定返回值的类型，默认是int
lib.my_sout.restype = ctypes.c_char_p

import zmq

# Worker Class
class Worker(threading.Thread):
    def __init__(self, id):
        threading.Thread.__init__(self)
        self.id = ("worker_"+str(id))
        # 动态链接对象
        self.obj = lib.test_new()
    
    def my_sout(self, id):
        res = str(lib.my_sout(self.obj, id), 'utf-8')
        return res

    #将python类型转换成c类型，支持int, float,string的变量和数组的转换
    def convert_type(self, input):
        ctypes_map = {int:ctypes.c_int,
                float:ctypes.c_double,
                str:ctypes.c_char_p
                }
        input_type = type(input)
        if input_type is list:
            length = len(input)
            if length==0:
                print("convert type failed...input is "+input)
                return null
            else:
                arr = (ctypes_map[type(input[0])] * length)()
                for i in range(length):
                    arr[i] = bytes(input[i],encoding="utf-8") if (type(input[0]) is str) else input[i]
                return arr
        else:
            if input_type in ctypes_map:
                return ctypes_map[input_type](bytes(input,encoding="utf-8") if type(input) is str else input)
            else:
                print("convert type failed...input is "+input)
                return null

    def run(self):
        # Init socket
        context = zmq.Context()
        socket = context.socket(zmq.DEALER)
        socket.identity = self.id.encode("ascii")
        socket.connect("tcp://localhost:5581")
        
        print(self.id + " started")
        while True:
            # Receive
            client, msg = socket.recv_multipart()
            print(self.id + " received: " + str(msg, 'utf-8'))
            # Send
            if msg.decode() == "Client Ready":
                socket.send_multipart([client, ("Worker Ready").encode()])
            if msg.decode() == "Client Stopping":
                break
            else:
                socket.send_multipart([client, (worker.my_sout(worker.convert_type(self.id)) + self.id).encode()])
            print("")
        
        # Clean Up
        print("Stopping " + str(self.id))
        socket.close()
        context.term()


# Main
if __name__ == "__main__":
    if len(sys.argv) == 1:
        print("Need to start worker with client UUID")
    else:
        worker = Worker(sys.argv[1])
        worker.start()