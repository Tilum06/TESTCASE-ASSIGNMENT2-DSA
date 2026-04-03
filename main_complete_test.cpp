#define TESTING

#include <iostream>
#include <sstream>
#include <functional>
#include <vector>
#include <string>
#include <typeinfo>
#include <initializer_list>
#include <list>
#include <algorithm>
#include <cmath>

#include "AVL.h"
#include "ThreadedAVL.h"
#include "Playlist.h"

using namespace std;

// ===================== Color (ANSI) =====================
#define C_RESET  "\033[0m"
#define C_RED    "\033[31m"
#define C_GREEN  "\033[32m"
#define C_YELLOW "\033[33m"
#define C_CYAN   "\033[36m"
#define C_GRAY   "\033[90m"

static string escape_visible(const string& s) {
    string out;
    out.reserve(s.size());
    for (unsigned char ch : s) {
        switch (ch) {
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (ch < 32) {
                    out += "\\x";
                    const char* hex = "0123456789ABCDEF";
                    out += hex[(ch >> 4) & 0xF];
                    out += hex[ch & 0xF];
                } else {
                    out.push_back((char)ch);
                }
        }
    }
    return out;
}

// ===================== Mini test framework =====================
static int g_pass = 0;
static int g_fail = 0;
static int g_suite_id = 0;
static int g_case_id = 0;
static string g_suite_name;

static void PASS(const string& msg) {
    g_case_id++;
    cout << C_GREEN << "[PASS]" << C_RESET << " "
         << g_suite_id << "." << g_case_id
         << " - " << msg << "\n";
    g_pass++;
}

static void FAIL_MSG(const string& msg) {
    g_case_id++;
    cout << C_RED << "[FAIL]" << C_RESET << " "
         << g_suite_id << "." << g_case_id
         << " - " << msg << "\n";
    g_fail++;
}

static void EXPECT_TRUE_IMPL(bool cond,
                             const string& msg,
                             const char* file,
                             int line) {
    if (cond) PASS(msg);
    else {
        cout << C_RED << "[FAIL] " << C_RESET << msg << "\n";
        cout << C_GRAY << "   Location: " << file << ":" << line << C_RESET << "\n";
        g_fail++;
    }
}

#define EXPECT_TRUE(cond, msg) \
    EXPECT_TRUE_IMPL((cond), (msg), __FILE__, __LINE__)

template <typename T>
static void EXPECT_EQ_IMPL(const T& actual,
                           const T& expected,
                           const string& msg,
                           const char* file,
                           int line) {
    if (actual == expected) {
        PASS(msg);
    } else {
        cout << C_RED << "[FAIL] " << C_RESET << msg << "\n";
        cout << C_GRAY << "   Location: " << file << ":" << line << C_RESET << "\n";
        cout << C_YELLOW << "   Expected: " << C_RESET << expected << "\n";
        cout << C_YELLOW << "   Actual  : " << C_RESET << actual << "\n";
        g_fail++;
    }
}

#define EXPECT_EQ(actual, expected, msg) \
    EXPECT_EQ_IMPL((actual), (expected), (msg), __FILE__, __LINE__)

static void EXPECT_EQ_STR_IMPL(const string& actual,
                               const string& expected,
                               const string& msg,
                               const char* file,
                               int line) {
    if (actual == expected) {
        PASS(msg);
    } else {
        cout << C_RED << "[FAIL] " << C_RESET << msg << "\n";
        cout << C_GRAY << "   Location: " << file << ":" << line << C_RESET << "\n";
        cout << C_YELLOW << "   Expected: " << C_RESET << "[" << escape_visible(expected) << "]\n";
        cout << C_YELLOW << "   Actual  : " << C_RESET << "[" << escape_visible(actual) << "]\n";
        cout << C_GRAY << "   (len expected=" << expected.size()
             << ", len actual=" << actual.size() << ")" << C_RESET << "\n";
        g_fail++;
    }
}

#define EXPECT_EQ_STR(actual, expected, msg) \
    EXPECT_EQ_STR_IMPL((actual), (expected), (msg), __FILE__, __LINE__)

static void RUN_TEST(const string& name, const function<void()>& fn) {
    g_suite_id++;
    g_case_id = 0;
    g_suite_name = name;

    cout << "\n========== "
         << g_suite_id << ". " << name
         << " ==========\n";

    try {
        fn();
    } catch (const exception& e) {
        FAIL_MSG("Uncaught exception: " + string(e.what()));
    } catch (...) {
        FAIL_MSG("Uncaught unknown exception");
    }
}

// ===================== Helpers =====================
class TestHelper {
public:
    static int currentIndex(const Playlist& p) { return p.currentIndex; }
#ifdef USE_THREADED_AVL
    static bool hasCurrent(const Playlist& p) { return p.hasCurrent; }
#endif
    static const string& artist(const Song& s) { return s.artist; }
    static int duration(const Song& s) { return s.duration; }
};

template <typename K, typename V>
class ExposedAVL : public AVL<K, V> {
public:
    using Node = typename AVL<K, V>::Node;
    Node* root() const { return this->pRoot; }
};

template <typename K, typename V>
class ExposedThreadedAVL : public ThreadedAVL<K, V> {
public:
    using Node = typename AVL<K, V>::Node;
    Node* root() const { return this->pRoot; }
};

template <typename T>
static string join_list(const list<T>& lst) {
    ostringstream oss;
    bool first = true;
    for (const auto& x : lst) {
        if (!first) oss << ",";
        first = false;
        oss << x;
    }
    return oss.str();
}

static string key_to_string(const SongKey& k) {
    return k.first + "#" + to_string(k.second);
}

static string join_songkey_list(const list<SongKey>& lst) {
    ostringstream oss;
    bool first = true;
    for (const auto& k : lst) {
        if (!first) oss << ",";
        first = false;
        oss << key_to_string(k);
    }
    return oss.str();
}

static Song* mkSong(int id,
                    const string& title,
                    int score = 0,
                    int duration = 180,
                    const string& artist = "Artist",
                    const string& album = "Album") {
    return new Song(id, title, artist, album, duration, score, "url");
}

struct SArg {
    int id;
    const char* title;
    int score;
    int duration;
};

static void addSongs(Playlist& p, initializer_list<SArg> a) {
    for (const auto& x : a) {
        p.addSong(mkSong(x.id, x.title, x.score, x.duration));
    }
}

static string song_brief(Song* s) {
    if (!s) return "(null)";
    return s->getTitle() + "#" + to_string(s->getId());
}

template <typename Node>
static int calc_height(Node* p) {
    if (!p) return 0;
    return 1 + max(calc_height(p->pLeft), calc_height(p->pRight));
}

template <typename K, typename Node>
static bool is_bst(Node* p, const K* low, const K* high) {
    if (!p) return true;
    if (low && !(*low < p->key)) return false;
    if (high && !(p->key < *high)) return false;
    return is_bst<K>(p->pLeft, low, &p->key) &&
           is_bst<K>(p->pRight, &p->key, high);
}

template <typename Node>
static bool is_avl_balanced(Node* p) {
    if (!p) return true;
    int lh = calc_height(p->pLeft);
    int rh = calc_height(p->pRight);
    if (abs(lh - rh) > 1) return false;
    return is_avl_balanced(p->pLeft) && is_avl_balanced(p->pRight);
}

template <typename K, typename V>
static void EXPECT_VALID_AVL(const ExposedAVL<K, V>& t, const string& msgPrefix) {
    auto* r = t.root();
    EXPECT_TRUE(is_bst<K>(r, nullptr, nullptr), msgPrefix + ": BST property holds");
    EXPECT_TRUE(is_avl_balanced(r), msgPrefix + ": AVL balance property holds");
}

template <typename K, typename V>
static void EXPECT_VALID_TAVL(const ExposedThreadedAVL<K, V>& t, const string& msgPrefix) {
    auto* r = t.root();
    EXPECT_TRUE(is_bst<K>(r, nullptr, nullptr), msgPrefix + ": BST property holds");
    EXPECT_TRUE(is_avl_balanced(r), msgPrefix + ": AVL balance property holds");
}

static vector<int> iterate_forward(ThreadedAVL<int, string>& t) {
    vector<int> out;
    for (auto it = t.beginIt(); !it.isNull(); ++it) out.push_back(it.key());
    return out;
}

static vector<int> iterate_backward(ThreadedAVL<int, string>& t) {
    vector<int> out;
    for (auto it = t.rbeginIt(); !it.isNull(); --it) out.push_back(it.key());
    return out;
}

static string join_vector_int(const vector<int>& v) {
    ostringstream oss;
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) oss << ",";
        oss << v[i];
    }
    return oss.str();
}

// ===================== Testcases =====================
static void suite_song_basic() {
    Song s(10, "Hello", "Adele", "25", 295, 9, "u");
    EXPECT_EQ(s.getId(), 10, "Song: getId");
    EXPECT_EQ_STR(s.getTitle(), "Hello", "Song: getTitle");
    EXPECT_EQ(s.getScore(), 9, "Song: getScore");
    EXPECT_EQ_STR(TestHelper::artist(s), "Adele", "Song: artist stored correctly");
    EXPECT_EQ(TestHelper::duration(s), 295, "Song: duration stored correctly");
    EXPECT_TRUE(!s.toString().empty(), "Song: toString non-empty");
    EXPECT_TRUE(s.toString().find("Hello") != string::npos,
                "Song: toString should mention title");
}

static void suite_avl_basic() {
    ExposedAVL<int, string> t;
    EXPECT_TRUE(t.empty(), "AVL: new tree is empty");
    EXPECT_EQ(t.size(), 0, "AVL: size == 0");
    EXPECT_EQ(t.height(), 0, "AVL: height of empty tree == 0");

    EXPECT_TRUE(t.insert(50, "v50"), "AVL: insert 50");
    EXPECT_TRUE(t.insert(30, "v30"), "AVL: insert 30");
    EXPECT_TRUE(t.insert(70, "v70"), "AVL: insert 70");
    EXPECT_TRUE(t.insert(20, "v20"), "AVL: insert 20");
    EXPECT_TRUE(t.insert(40, "v40"), "AVL: insert 40");
    EXPECT_TRUE(t.insert(60, "v60"), "AVL: insert 60");
    EXPECT_TRUE(t.insert(80, "v80"), "AVL: insert 80");
    EXPECT_TRUE(!t.insert(30, "dup"), "AVL: duplicate key returns false");

    EXPECT_TRUE(!t.empty(), "AVL: not empty after inserts");
    EXPECT_EQ(t.size(), 7, "AVL: size after 7 unique inserts");
    EXPECT_EQ(t.height(), 3, "AVL: height of perfect 7-node tree == 3");
    EXPECT_TRUE(t.contains(20), "AVL: contains existing key");
    EXPECT_TRUE(!t.contains(999), "AVL: does not contain missing key");

    string* p40 = t.find(40);
    EXPECT_TRUE(p40 != nullptr, "AVL: find existing returns non-null");
    if (p40) EXPECT_EQ_STR(*p40, "v40", "AVL: find returns correct value");
    EXPECT_TRUE(t.find(999) == nullptr, "AVL: find missing returns nullptr");

    EXPECT_EQ_STR(join_list(t.ascendingList()), "20,30,40,50,60,70,80",
                  "AVL: ascendingList inorder");
    EXPECT_EQ_STR(join_list(t.descendingList()), "80,70,60,50,40,30,20",
                  "AVL: descendingList reverse inorder");
    EXPECT_VALID_AVL(t, "AVL basic tree");

    EXPECT_TRUE(t.erase(20), "AVL: erase leaf node");
    EXPECT_EQ(t.size(), 6, "AVL: size after erasing leaf");
    EXPECT_EQ_STR(join_list(t.ascendingList()), "30,40,50,60,70,80",
                  "AVL: inorder after erasing leaf");
    EXPECT_VALID_AVL(t, "AVL after erasing leaf");

    EXPECT_TRUE(t.erase(30), "AVL: erase one-child node");
    EXPECT_EQ(t.size(), 5, "AVL: size after erasing one-child node");
    EXPECT_EQ_STR(join_list(t.ascendingList()), "40,50,60,70,80",
                  "AVL: inorder after erasing one-child node");
    EXPECT_VALID_AVL(t, "AVL after erasing one-child node");

    EXPECT_TRUE(t.erase(70), "AVL: erase two-child node");
    EXPECT_EQ(t.size(), 4, "AVL: size after erasing two-child node");
    EXPECT_EQ_STR(join_list(t.ascendingList()), "40,50,60,80",
                  "AVL: inorder after erasing two-child node");
    EXPECT_VALID_AVL(t, "AVL after erasing two-child node");

    EXPECT_TRUE(!t.erase(999), "AVL: erase missing key returns false");
    EXPECT_EQ(t.size(), 4, "AVL: size unchanged after erase missing");

    t.clear();
    EXPECT_TRUE(t.empty(), "AVL: clear -> empty");
    EXPECT_EQ(t.size(), 0, "AVL: clear -> size 0");
    EXPECT_EQ(t.height(), 0, "AVL: clear -> height 0");
    EXPECT_EQ_STR(join_list(t.ascendingList()), "", "AVL: clear -> ascendingList empty");
}

static void suite_avl_rotations_and_height() {
    {
        ExposedAVL<int, int> t;
        t.insert(30, 30); t.insert(20, 20); t.insert(10, 10);
        EXPECT_TRUE(t.root() != nullptr && t.root()->key == 20,
                    "AVL LL rotation: root becomes 20");
        EXPECT_EQ(t.height(), 2, "AVL LL rotation: height == 2");
        EXPECT_VALID_AVL(t, "AVL LL rotation");
    }
    {
        ExposedAVL<int, int> t;
        t.insert(10, 10); t.insert(20, 20); t.insert(30, 30);
        EXPECT_TRUE(t.root() != nullptr && t.root()->key == 20,
                    "AVL RR rotation: root becomes 20");
        EXPECT_EQ(t.height(), 2, "AVL RR rotation: height == 2");
        EXPECT_VALID_AVL(t, "AVL RR rotation");
    }
    {
        ExposedAVL<int, int> t;
        t.insert(30, 30); t.insert(10, 10); t.insert(20, 20);
        EXPECT_TRUE(t.root() != nullptr && t.root()->key == 20,
                    "AVL LR rotation: root becomes 20");
        EXPECT_EQ(t.height(), 2, "AVL LR rotation: height == 2");
        EXPECT_VALID_AVL(t, "AVL LR rotation");
    }
    {
        ExposedAVL<int, int> t;
        t.insert(10, 10); t.insert(30, 30); t.insert(20, 20);
        EXPECT_TRUE(t.root() != nullptr && t.root()->key == 20,
                    "AVL RL rotation: root becomes 20");
        EXPECT_EQ(t.height(), 2, "AVL RL rotation: height == 2");
        EXPECT_VALID_AVL(t, "AVL RL rotation");
    }
    {
        ExposedAVL<int, int> t;
        for (int i = 1; i <= 15; ++i) t.insert(i, i);
        EXPECT_EQ(t.size(), 15, "AVL sorted inserts: size 15");
        EXPECT_TRUE(t.height() <= 5,
                    "AVL sorted inserts: height remains logarithmic (<=5)");
        EXPECT_EQ_STR(join_list(t.ascendingList()),
                      "1,2,3,4,5,6,7,8,9,10,11,12,13,14,15",
                      "AVL sorted inserts: ascending order preserved");
        EXPECT_VALID_AVL(t, "AVL sorted inserts large");
    }
}

static void suite_threaded_avl_basic() {
    ExposedThreadedAVL<int, string> t;
    EXPECT_TRUE(t.beginIt().isNull(), "TAVL: beginIt on empty is null");
    EXPECT_TRUE(t.rbeginIt().isNull(), "TAVL: rbeginIt on empty is null");
    EXPECT_EQ_STR(join_list(t.ascendingList()), "", "TAVL: ascendingList empty");

    t.insert(50, "v50");
    t.insert(30, "v30");
    t.insert(70, "v70");
    t.insert(20, "v20");
    t.insert(40, "v40");
    t.insert(60, "v60");
    t.insert(80, "v80");

    EXPECT_EQ_STR(join_list(t.ascendingList()), "20,30,40,50,60,70,80",
                  "TAVL: ascendingList via threads");
    EXPECT_EQ_STR(join_list(t.descendingList()), "80,70,60,50,40,30,20",
                  "TAVL: descendingList via threads");
    EXPECT_EQ_STR(join_vector_int(iterate_forward(t)), "20,30,40,50,60,70,80",
                  "TAVL: Iterator ++ traverses inorder");
    EXPECT_EQ_STR(join_vector_int(iterate_backward(t)), "80,70,60,50,40,30,20",
                  "TAVL: Iterator -- traverses reverse inorder");

    auto it40 = t.findIt(40);
    EXPECT_TRUE(!it40.isNull(), "TAVL: findIt existing key");
    if (!it40.isNull()) {
        EXPECT_EQ(it40.key(), 40, "TAVL: findIt key() correct");
        EXPECT_EQ_STR(it40.value(), "v40", "TAVL: findIt value() correct");
    }
    EXPECT_TRUE(t.findIt(999).isNull(), "TAVL: findIt missing key is null");
    EXPECT_VALID_TAVL(t, "TAVL basic tree");
}

static void suite_threaded_avl_erase_and_threads() {
    ExposedThreadedAVL<int, string> t;
    for (int k : {50, 30, 70, 20, 40, 60, 80, 35}) {
        t.insert(k, "v" + to_string(k));
    }
    EXPECT_EQ_STR(join_vector_int(iterate_forward(t)), "20,30,35,40,50,60,70,80",
                  "TAVL: initial iterator order");

    EXPECT_TRUE(t.erase(30), "TAVL: erase node with two children");
    EXPECT_EQ_STR(join_vector_int(iterate_forward(t)), "20,35,40,50,60,70,80",
                  "TAVL: threads correct after erase two-children node");
    EXPECT_VALID_TAVL(t, "TAVL after erase two-children node");

    EXPECT_TRUE(t.erase(20), "TAVL: erase head node");
    EXPECT_EQ_STR(join_vector_int(iterate_forward(t)), "35,40,50,60,70,80",
                  "TAVL: threads correct after erase head");
    EXPECT_TRUE(!t.beginIt().isNull() && t.beginIt().key() == 35,
                "TAVL: new head is 35");

    EXPECT_TRUE(t.erase(80), "TAVL: erase tail node");
    EXPECT_EQ_STR(join_vector_int(iterate_forward(t)), "35,40,50,60,70",
                  "TAVL: threads correct after erase tail");
    EXPECT_TRUE(!t.rbeginIt().isNull() && t.rbeginIt().key() == 70,
                "TAVL: new tail is 70");
    EXPECT_VALID_TAVL(t, "TAVL after head/tail erasures");

    t.clear();
    EXPECT_TRUE(t.beginIt().isNull(), "TAVL: clear -> beginIt null");
    EXPECT_TRUE(t.rbeginIt().isNull(), "TAVL: clear -> rbeginIt null");
    EXPECT_EQ_STR(join_list(t.ascendingList()), "", "TAVL: clear -> ascendingList empty");
}

static void suite_playlist_basic_order() {
    Playlist p("P");
    EXPECT_TRUE(p.empty(), "PL: new playlist empty");
    EXPECT_EQ(p.getSize(), 0, "PL: size == 0");
    EXPECT_EQ(TestHelper::currentIndex(p), -1, "PL: currentIndex starts at -1");
#ifdef USE_THREADED_AVL
    EXPECT_TRUE(!TestHelper::hasCurrent(p), "PL: hasCurrent starts false");
#endif

    addSongs(p, {
        {3, "Beta", 5, 180},
        {2, "Alpha", 7, 200},
        {1, "Alpha", 4, 210},
        {4, "Delta", 2, 150}
    });

    EXPECT_TRUE(!p.empty(), "PL: not empty after addSong");
    EXPECT_EQ(p.getSize(), 4, "PL: size after addSong");
    EXPECT_EQ_STR(song_brief(p.getSong(0)), "Alpha#1", "PL: getSong(0) sorted by title then id");
    EXPECT_EQ_STR(song_brief(p.getSong(1)), "Alpha#2", "PL: getSong(1) same title tie-break by id");
    EXPECT_EQ_STR(song_brief(p.getSong(2)), "Beta#3",  "PL: getSong(2)");
    EXPECT_EQ_STR(song_brief(p.getSong(3)), "Delta#4", "PL: getSong(3)");
    EXPECT_TRUE(p.getSong(-1) == nullptr, "PL: getSong(-1) -> nullptr");
    EXPECT_TRUE(p.getSong(4) == nullptr, "PL: getSong(size) -> nullptr");

    p.removeSong(1);
    EXPECT_EQ(p.getSize(), 3, "PL: removeSong(valid) decreases size");
    EXPECT_EQ_STR(song_brief(p.getSong(0)), "Alpha#1", "PL: order after removeSong index 1 (0)");
    EXPECT_EQ_STR(song_brief(p.getSong(1)), "Beta#3",  "PL: order after removeSong index 1 (1)");
    EXPECT_EQ_STR(song_brief(p.getSong(2)), "Delta#4", "PL: order after removeSong index 1 (2)");

    p.clear();
    EXPECT_TRUE(p.empty(), "PL: clear -> empty");
    EXPECT_EQ(p.getSize(), 0, "PL: clear -> size 0");
    EXPECT_TRUE(p.getSong(0) == nullptr, "PL: clear -> getSong(0) is nullptr");
    EXPECT_EQ(TestHelper::currentIndex(p), -1, "PL: clear -> currentIndex reset");
#ifdef USE_THREADED_AVL
    EXPECT_TRUE(!TestHelper::hasCurrent(p), "PL: clear -> hasCurrent false");
#endif
}

static void suite_playlist_playback_navigation() {
    Playlist p("Nav");
    addSongs(p, {
        {1, "Alpha",   1, 100},
        {2, "Beta",    2, 100},
        {3, "Charlie", 3, 100}
    });

    EXPECT_TRUE(p.playPrevious() == nullptr,
                "PL: playPrevious before starting playback -> nullptr");
    EXPECT_EQ(TestHelper::currentIndex(p), -1,
              "PL: currentIndex still -1 before playback starts");

    Song* s = p.playNext();
    EXPECT_EQ_STR(song_brief(s), "Alpha#1", "PL: first playNext starts from first song");
    EXPECT_EQ(TestHelper::currentIndex(p), 0, "PL: currentIndex after first playNext == 0");
#ifdef USE_THREADED_AVL
    EXPECT_TRUE(TestHelper::hasCurrent(p), "PL: hasCurrent true after playNext");
#endif

    EXPECT_TRUE(p.playPrevious() == nullptr,
                "PL: playPrevious at first song -> nullptr");
    EXPECT_EQ(TestHelper::currentIndex(p), 0,
              "PL: currentIndex stays at 0 after playPrevious on first song");

    EXPECT_EQ_STR(song_brief(p.playNext()), "Beta#2", "PL: second playNext");
    EXPECT_EQ(TestHelper::currentIndex(p), 1, "PL: currentIndex after second playNext == 1");

    EXPECT_EQ_STR(song_brief(p.playNext()), "Charlie#3", "PL: third playNext");
    EXPECT_EQ(TestHelper::currentIndex(p), 2, "PL: currentIndex at last song == 2");

    EXPECT_TRUE(p.playNext() == nullptr, "PL: playNext at last song -> nullptr");
    EXPECT_EQ(TestHelper::currentIndex(p), 2,
              "PL: currentIndex stays at last after playNext(nullptr)");

    EXPECT_EQ_STR(song_brief(p.playPrevious()), "Beta#2", "PL: playPrevious from last song");
    EXPECT_EQ(TestHelper::currentIndex(p), 1, "PL: currentIndex after playPrevious == 1");

    p.clear();
    EXPECT_TRUE(p.playNext() == nullptr, "PL: playNext on empty playlist -> nullptr");
    EXPECT_TRUE(p.playPrevious() == nullptr, "PL: playPrevious on empty playlist -> nullptr");
}

static void suite_playlist_remove_and_reset() {
    // Deleting current song should reset playback state.
    {
        Playlist p("ResetCurrent");
        addSongs(p, {
            {1, "Alpha",   1, 100},
            {2, "Beta",    2, 100},
            {3, "Charlie", 3, 100},
            {4, "Delta",   4, 100}
        });

        EXPECT_EQ_STR(song_brief(p.playNext()), "Alpha#1", "PL reset: start Alpha");
        EXPECT_EQ_STR(song_brief(p.playNext()), "Beta#2",  "PL reset: advance to Beta");
        EXPECT_EQ(TestHelper::currentIndex(p), 1, "PL reset: currentIndex is 1 before deletion");

        p.removeSong(1); // remove current song (Beta)
        EXPECT_EQ(p.getSize(), 3, "PL reset: size after deleting current song");
        EXPECT_EQ(TestHelper::currentIndex(p), -1,
                  "PL reset: currentIndex reset after deleting current song");
#ifdef USE_THREADED_AVL
        EXPECT_TRUE(!TestHelper::hasCurrent(p), "PL reset: hasCurrent false after deleting current song");
#endif
        EXPECT_EQ_STR(song_brief(p.playNext()), "Alpha#1",
                      "PL reset: next playback restarts from first song after reset");
    }

    // Deleting a song before current should shift currentIndex but keep current song logically.
    {
        Playlist p("ShiftIndex");
        addSongs(p, {
            {1, "Alpha",   1, 100},
            {2, "Beta",    2, 100},
            {3, "Charlie", 3, 100},
            {4, "Delta",   4, 100}
        });

        p.playNext(); // Alpha
        p.playNext(); // Beta
        p.playNext(); // Charlie (index 2)
        EXPECT_EQ(TestHelper::currentIndex(p), 2, "PL shift: currentIndex is 2 before deletion");

        p.removeSong(0); // remove Alpha, Charlie should now be index 1
        EXPECT_EQ(p.getSize(), 3, "PL shift: size after deleting earlier song");
        EXPECT_EQ(TestHelper::currentIndex(p), 1,
                  "PL shift: currentIndex decremented when deleting earlier song");
        EXPECT_EQ_STR(song_brief(p.playPrevious()), "Beta#2",
                      "PL shift: playPrevious still reaches previous logical song");
    }

    // Deleting the only song empties the playlist and resets playback.
    {
        Playlist p("OnlyOne");
        p.addSong(mkSong(1, "Only", 8, 100));
        p.playNext();
        p.removeSong(0);
        EXPECT_TRUE(p.empty(), "PL only-one: playlist becomes empty after delete");
        EXPECT_EQ(TestHelper::currentIndex(p), -1,
                  "PL only-one: currentIndex reset to -1");
        EXPECT_TRUE(p.playNext() == nullptr, "PL only-one: playNext on empty -> nullptr");
    }
}

static void suite_playlist_score_compare() {
    {
        Playlist p("Score");
        addSongs(p, {
            {1, "A", 4, 100},
            {2, "B", 5, 100},
            {3, "C", 3, 100}
        });
        EXPECT_EQ(p.getTotalScore(), 12, "PL: getTotalScore sums all scores");
    }
    {
        Playlist p1("P1");
        addSongs(p1, {
            {1, "A", 4, 100},
            {2, "B", 5, 100},
            {3, "C", 3, 100}
        });
        Playlist p2("P2");
        addSongs(p2, {
            {4, "A", 3, 100},
            {5, "B", 4, 100},
            {6, "Z", 10, 100}
        });
        EXPECT_TRUE(p1.compareTo(p2, 2), "PL: compareTo first 2 songs true when current >= other");
        EXPECT_TRUE(!p2.compareTo(p1, 2), "PL: compareTo first 2 songs false when current < other");
    }
    {
        Playlist p1("Eq1");
        addSongs(p1, {
            {1, "A", 2, 100},
            {2, "B", 3, 100}
        });
        Playlist p2("Eq2");
        addSongs(p2, {
            {3, "A", 1, 100},
            {4, "B", 4, 100}
        });
        EXPECT_TRUE(p1.compareTo(p2, 2), "PL: compareTo equality should return true");
    }
    {
        Playlist empty("Empty");
        EXPECT_EQ(empty.getTotalScore(), 0, "PL: empty playlist total score == 0");
    }
}

static void suite_playlist_random_and_approximate() {
    Playlist p("Jump");
    addSongs(p, {
        {1, "A", 1, 100},
        {2, "B", 2, 100},
        {3, "C", 3, 100},
        {4, "D", 4, 100},
        {5, "E", 5, 100}
    });

    p.playRandom(2); // C
    EXPECT_EQ(TestHelper::currentIndex(p), 2, "PL: playRandom sets currentIndex");
#ifdef USE_THREADED_AVL
    EXPECT_TRUE(TestHelper::hasCurrent(p), "PL: playRandom sets hasCurrent true");
#endif
    EXPECT_EQ_STR(song_brief(p.playNext()), "D#4",
                  "PL: playNext continues from playRandom-selected song");
    EXPECT_EQ_STR(song_brief(p.playPrevious()), "C#3",
                  "PL: playPrevious returns to song selected by playRandom");

    p.playRandom(1); // B
    EXPECT_EQ(TestHelper::currentIndex(p), 1, "PL: playRandom can jump backward too");

    EXPECT_EQ(p.playApproximate(2), 3,
              "PL: playApproximate(+2) moves forward by 2 positions");
    EXPECT_EQ(TestHelper::currentIndex(p), 3,
              "PL: currentIndex updated after playApproximate(+2)");
    EXPECT_EQ_STR(song_brief(p.playPrevious()), "C#3",
                  "PL: after approx to D, previous song is C");

    EXPECT_EQ(p.playApproximate(-1), 1,
              "PL: playApproximate(-1) moves backward by 1 position");
    EXPECT_EQ(TestHelper::currentIndex(p), 1,
              "PL: currentIndex updated after playApproximate(-1)");
    EXPECT_EQ(p.playApproximate(0), 1,
              "PL: playApproximate(0) keeps current position");
}

static void suite_stress() {
    const int N = 2000;

    // Sorted insert stress for AVL.
    ExposedAVL<int, int> t;
    for (int i = 1; i <= N; ++i) t.insert(i, i);
    EXPECT_EQ(t.size(), N, "Stress AVL: size after sorted inserts");
    EXPECT_TRUE(t.height() <= 20,
                "Stress AVL: height remains logarithmic for 2000 sorted inserts");
    EXPECT_VALID_AVL(t, "Stress AVL tree");

    for (int i = 1; i <= N; i += 2) t.erase(i);
    EXPECT_EQ(t.size(), N / 2, "Stress AVL: size after erasing all odd keys");
    EXPECT_TRUE(t.height() <= 20,
                "Stress AVL: height remains logarithmic after many deletes");
    EXPECT_VALID_AVL(t, "Stress AVL after deletes");

    // Playlist smoke test.
    Playlist p("StressPlaylist");
    for (int i = N; i >= 1; --i) {
        ostringstream title;
        title << "Song";
        if (i < 1000) title << '0';
        if (i < 100)  title << '0';
        if (i < 10)   title << '0';
        title << i;
        p.addSong(mkSong(i, title.str(), i % 11, 120));
    }
    EXPECT_EQ(p.getSize(), N, "Stress PL: size after many addSong");
    EXPECT_EQ_STR(p.getSong(0)->getTitle(), "Song0001",
                  "Stress PL: songs still sorted alphabetically");
    p.playRandom(999);
    EXPECT_EQ(TestHelper::currentIndex(p), 999, "Stress PL: playRandom large valid index");
    EXPECT_EQ(p.playApproximate(100), 1099, "Stress PL: playApproximate large positive step");
}

// ===================== main =====================
int main() {
    RUN_TEST("Song", suite_song_basic);
    RUN_TEST("AVL Basic", suite_avl_basic);
    RUN_TEST("AVL Rotations & Height", suite_avl_rotations_and_height);
    RUN_TEST("ThreadedAVL Basic", suite_threaded_avl_basic);
    RUN_TEST("ThreadedAVL Erase & Threads", suite_threaded_avl_erase_and_threads);
    RUN_TEST("Playlist Basic Order", suite_playlist_basic_order);
    RUN_TEST("Playlist Playback Navigation", suite_playlist_playback_navigation);
    RUN_TEST("Playlist Remove & Reset", suite_playlist_remove_and_reset);
    RUN_TEST("Playlist Score & Compare", suite_playlist_score_compare);
    RUN_TEST("Playlist Random & Approximate", suite_playlist_random_and_approximate);
    RUN_TEST("Stress", suite_stress);

    cout << "\n" << C_CYAN << "===== SUMMARY =====" << C_RESET << "\n";
    cout << C_GREEN << "PASS: " << g_pass << C_RESET << "\n";
    if (g_fail == 0) cout << C_GREEN << "FAIL: 0" << C_RESET << "\n";
    else cout << C_RED << "FAIL: " << g_fail << C_RESET << "\n";

    return (g_fail == 0) ? 0 : 1;
}
