# 🧪 DSA Assignment 2 Test Suite (HCMUT - Semester 252)

A comprehensive test suite for DSA Assignment 2.

This project supports **multiple execution modes** for flexible and targeted testing.

---


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

### 🔹 Run all tests

```bash
./test_basic
./test_threaded
```


### 🔹 Run by category

```bash
./test_basic [mode]        # AVL mode
./test_threaded [mode]     # ThreadedAVL mode
```

#### Available modes:

| Mode        | Description |
|------------|------------|
| `all` (default) | Run all tests |
| `song`     | Test `Song` class |
| `avl`      | Test AVL (insert, delete, balance) |
| `playlist` | Test Playlist functionality |
| `stress`   | Run stress tests |
| `threaded` | Test ThreadedAVL *(only for `test_threaded`)* |
| `help`     | Show usage and available test modes |

### 🔹 Run specific test suites

You can also run individual test suites:

- `avl-basic`
- `avl-rotation`
- `playlist-order`
- `playlist-nav`
- `playlist-remove-current`
- `playlist-score`
- `playlist-jump`

## 🧪 Testcase Guide
This test suite validates both **correct output and internal behavior** of your implementation.

Each test provides:
- PASS / FAIL status
- Expected vs actual result
- Code location of failure

### 🔍 How to debug

1. Run the test and locate the **first FAIL**
2. Read the **test message** (it describes expected behavior)
3. Check the corresponding function in your code
4. Compare:
   - Return value
   - Internal state (e.g. `currentIndex`)
5. Fix and rerun

### 🧠 What is being tested

#### AVL / ThreadedAVL
- BST property (ordering)
- AVL balance (height difference ≤ 1)
- Correct insert / erase / traversal
- Iterator correctness (ThreadedAVL)

#### Playlist
- Correct sorting (by title, then id)
- Playback logic:
  - `playNext`, `playPrevious`
- State management:
  - `currentIndex`
  - `hasCurrent` (ThreadedAVL mode)
- Edge cases:
  - Empty playlist
  - Invalid index

#### Advanced features
- `getTotalScore` (sum of scores)
- `compareTo` (compare first k songs)
- `playRandom` (update playback state only)
- `playApproximate` (move index with step)

#### Stress tests
- Large input size
- Ensures no crash and correct complexity

---

### ⚠️ Note

This test suite was generated and refined with the assistance of AI.  
While it aims to follow the official assignment specification and forum clarifications, it **may still contain inaccuracies or incomplete assumptions**.

If you encounter inconsistencies, please refer to:
- The official assignment description
- Instructor announcements / forum discussions

👉 Use this test suite as a **supporting tool**, not the ultimate source of truth.
