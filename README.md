# Sockets Programming in C

This repository contains several examples of socket programming in **C**, including basic chat servers, file servers, and solutions to common synchronization problems in socket communication. 

## Projects

1. **Calculator**: An example of a simple calculator server-client application.
2. **Chat Server**: A chat server where the server and client can send and receive messages simultaneously.
3. **File Server**: A file server that resolves synchronization issues and supports sending larger files.

## Installation

To run any of these projects, you need to have a C compiler (like GCC) and basic knowledge of socket programming.

## Steps to run:

1. Clone the repository:
    ```bash
    git clone https://github.com/ddas09/sockets.git
    cd sockets
    ```

2. Compile the desired project:
    ```bash
    gcc -o chat_server chat_server.c
    ```

3. Run the compiled server and client.

    In one terminal window, run the server:

    ```bash
    ./chat_server
    ```

    In another terminal window, run the client:

    ```bash
    ./chat_client
    ```
4. Enter message in the console to send and check other console

## Contributing

1. Fork the repository.
2. Create a new branch:
   ```bash
   git checkout -b feature/your-feature
   ```
3. Make your changes.
4. Commit your changes 
   ```bash
   git commit -a 'Add new feature'
   ```
5. Push to the branch 
   ```bash
   git push origin feature/your-feature
   ```
6. Create a new Pull Request.

## License
This project is open-source and licensed under the [MIT License](https://opensource.org/license/mit).