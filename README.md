This is a basic HTTP server implemented in C++ using only the basic libraries (socket and pthread).

### How to Run

```bash
g++ -pthread server.cpp -o server && ./server
```

### Features

1. Handles GET requests.
2. Serves a basic webpage that generates a random whole number depending on the range provided by the user.
3. Supports multithreading.

### References

1. https://medium.com/from-the-scratch/http-server-what-do-you-need-to-know-to-build-a-simple-http-server-from-scratch-d1ef8945e4fa
2. https://github.com/kayacanv/Basic-Http-Server-cpp
