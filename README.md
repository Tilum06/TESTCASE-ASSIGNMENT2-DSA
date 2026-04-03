Replace the old AVL.h, ThreadedAVL.h, Playlist.h and Playlist.cpp files with your files.

## Build
Basic AVL mode:
```bash
g++ -std=c++17 -I . main_complete_test.cpp Playlist.cpp -o test_basic
```

Threaded mode (bonus):
```bash
g++ -std=c++17 -DUSE_THREADED_AVL -I . main_complete_test.cpp Playlist.cpp -o test_threaded
```

## Run

Basic AVL mode:
```bash
./test_basic
```

Threaded mode (bonus):
```bash
./test_threaded
```