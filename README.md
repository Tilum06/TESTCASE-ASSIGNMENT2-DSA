## 📌 Setup

Replace the provided files with your implementation:
- `AVL.h`
- `ThreadedAVL.h`
- `Playlist.h`
- `Playlist.cpp`


## ⚙️ Build

### Basic AVL mode
```bash
g++ -std=c++17 -I . include tests/assignment2_test.cpp src/Playlist.cpp -o test_basic
```

### Threaded mode (bonus):
```bash
g++ -std=c++17 -DUSE_THREADED_AVL -I include tests/assignment2_test.cpp src/Playlist.cpp -o test_threaded
```

## ▶️ Run

### Basic AVL mode:
```bash
./test_basic
```

### Threaded mode (bonus):
```bash
./test_threaded
```