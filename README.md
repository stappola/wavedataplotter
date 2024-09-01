# Wavedataplotter
Displays frequency data received from a server in time-domain. The data seems to represent amplitude
changes over time, which would make them look more or less like a sine wave when visualized as a graph.

# Build & run client1
cd client1  
mkdir build  
cd build  
cmake ..  
make
./client1

The application can be stopped by pressing 'Enter' or Ctrl+C.

# Build & run client2
cd client2  
mkdir build  
cd build  
cmake ..  
make
./client2

The application can be stopped by pressing 'Enter' or Ctrl+C.

# Build & run client2 unit tests
cd client2/unittest  
mkdir build  
cd build  
cmake ..  
make  
./client2_unittest.app

# Design principles
My goal was to try to keep the application as responsive as possible so as to keep the data receiving
flow as quickly as possible. This I tried to achieve by making each client socket run in its own thread
and synchronizing their input for printing with mutexes.

Another central design descision was to employ abstract C++ interfaces in places where they might help
in writing unit tests. These places are mainly the data input and output points that one usually wants
to "mock" in unit tests.

I tried to verify the work (according to the provided requirements and my own assumptions) mainly with
unit tests and inspecting the data flow on Linux console. Maybe the biggest assumption I made was to
regard the "glitch chance" value as a propability as to how likely it is for random errors to occur in
the data flow. That is why, on client2, this value is set to 0 in the constructor of the printer thread.

In the end, mainly due to lack of time, I decided to make one concession concerning the reponsiveness of
the client: the control channel should, ideally, also run in its own thread, mostly sleeping, and waking
up only to deliver the UDP packets to server's control channel receiver. UDP traffic is, after all, mostly
"fire and forget" operations, which is why I deemed this omission to not be a major hindrance.


