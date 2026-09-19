# HTTP server in C from Scratch

As for the project, it is a simple HTTP server built from scratch using the C programming language; this server is notable for using TCP sockets directly.

## Requirements
To test this server, you must have the following installed:
- GCC

## BUild
Once you have all the requirements installed and working properly, you will need to compile it; the process has been simplified, and you simply need to follow these steps:

```bash
chmod +x ./build.sh
```

Start the compilation
```bash
./build.sh
```

Finally, you can run the program:
```bash
./build/server
```

### Startup parameters
`[port]` ->  You indicate at what point it needs to be listened to.
`[file_html]` -> Indicates the html file to render

<hr>
MIT [LICENSE](./LICENSE)