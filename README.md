# chatti

Chatti provides instant messaging in a terminal window using BSD sockets and ncurses. Chatti was a school project.

I do not recommend sending sensitive information with chatti or even trusting any received piece of information since the communication is currently __unencrypted__. With that being said, I disclaim responsibility for any damage caused by the use of my software.

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


## TODO

- Convert text from network character encoding (UTF-8) to local character encoding and vice versa.
- Fix deletion of multibyte characters in the message input window.
- Much more.
