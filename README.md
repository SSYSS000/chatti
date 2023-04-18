# chatti

## Build instructions

You will need CMake 3.0 or higher, a C compiler and make or Ninja or something else.

To build with make:

    cmake -B build -G 'Unix Makefiles' .
    cd build
    make -j$(nproc)

To build with Ninja:

    cmake -B build -G Ninja .
    cd build
    ninja

## Running chatti

Once you've built chatti, you need to set up the server. Set up
the server by running the following command (replace $SERVER_PORT with a port number, e.g. 14000).

    ./chatti-server $SERVER_PORT

Then connect to the server. 

    ./chatti-client $SERVER_ADDRESS $SERVER_PORT $USERNAME

Again, replace the environment variables with appropriate values. Example:

    ./chatti-client 192.168.8.100 14000 Joe

Now, assuming the connection was established, you should be able to type and send messages.

## Running tests

In the build directory, run `ctest`. Make sure you've built the tests first. If you haven't see the build instructions above.

