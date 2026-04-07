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
#ifdef USE_THREADED_AVL
#include "ThreadedAVL.h"
#endif
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
    static int id(const Song& s) { return s.id; }
    static const string& title(const Song& s) { return s.title; }
    static const string& artist(const Song& s) { return s.artist; }
    static int duration(const Song& s) { return s.duration; }
    static int score(const Song& s) { return s.score; }
};

template <typename K, typename V>
class ExposedAVL : public AVL<K, V> {
public:
    using Node = typename AVL<K, V>::Node;
    Node* root() const { return this->pRoot; }
};

#ifdef USE_THREADED_AVL
template <typename K, typename V>
class ExposedThreadedAVL : public ThreadedAVL<K, V> {
public:
    using Node = typename AVL<K, V>::Node;
    Node* root() const { return this->pRoot; }
};
#endif

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

static string capture_cout(function<void()> fn) {
    ostringstream oss;
    streambuf* old = cout.rdbuf(oss.rdbuf());
    fn();
    cout.rdbuf(old);
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
    return TestHelper::title(*s) + "#" + to_string(TestHelper::id(*s));
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

#ifdef USE_THREADED_AVL
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
#endif

// ===================== Testcases =====================
static void suite_song_basic() {
    Song s(10, "Hello", "Adele", "25", 295, 9, "u");
    EXPECT_EQ(TestHelper::id(s), 10, "Song: id stored correctly");
    EXPECT_EQ_STR(TestHelper::title(s), "Hello", "Song: title stored correctly");
    EXPECT_EQ_STR(TestHelper::artist(s), "Adele", "Song: artist stored correctly");
    EXPECT_EQ(TestHelper::duration(s), 295, "Song: duration stored correctly");
    EXPECT_EQ(TestHelper::score(s), 9, "Song: score stored correctly");
    EXPECT_TRUE(!s.toString().empty(), "Song: toString non-empty");
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

#ifdef USE_THREADED_AVL
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
#endif

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

    p.removeSong(-1);
    EXPECT_EQ(p.getSize(), 4, "PL: removeSong(-1) does nothing");
    p.removeSong(4);
    EXPECT_EQ(p.getSize(), 4, "PL: removeSong(size) does nothing");

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

    EXPECT_EQ_STR(song_brief(p.playNext()), "Alpha#1",
                  "PL: first playNext starts from first song");
    EXPECT_EQ(TestHelper::currentIndex(p), 0,
              "PL: currentIndex after first playNext == 0");
#ifdef USE_THREADED_AVL
    EXPECT_TRUE(TestHelper::hasCurrent(p), "PL: hasCurrent true after playNext");
#endif

    EXPECT_EQ_STR(song_brief(p.playNext()), "Beta#2", "PL: second playNext");
    EXPECT_EQ(TestHelper::currentIndex(p), 1,
              "PL: currentIndex after second playNext == 1");

    EXPECT_EQ_STR(song_brief(p.playNext()), "Charlie#3", "PL: third playNext");
    EXPECT_EQ(TestHelper::currentIndex(p), 2,
              "PL: currentIndex at last song == 2");

    EXPECT_EQ_STR(song_brief(p.playNext()), "Alpha#1",
                  "PL: playNext wraps from last song to first song");
    EXPECT_EQ(TestHelper::currentIndex(p), 0,
              "PL: currentIndex wraps to 0 after playNext at end");

    EXPECT_EQ_STR(song_brief(p.playPrevious()), "Charlie#3",
                  "PL: playPrevious wraps from first song to last song");
    EXPECT_EQ(TestHelper::currentIndex(p), 2,
              "PL: currentIndex wraps to last after playPrevious at start");

    EXPECT_EQ_STR(song_brief(p.playPrevious()), "Beta#2",
                  "PL: playPrevious continues backward after wrap");
    EXPECT_EQ(TestHelper::currentIndex(p), 1,
              "PL: currentIndex after playPrevious == 1");

    p.clear();
    EXPECT_TRUE(p.playNext() == nullptr, "PL: playNext on empty playlist -> nullptr");
    EXPECT_TRUE(p.playPrevious() == nullptr, "PL: playPrevious on empty playlist -> nullptr");
}

static void suite_playlist_remove_current_behavior() {
    {
        Playlist p("RemoveBasic");
        addSongs(p, {
            {1, "Alpha",   1, 100},
            {2, "Beta",    2, 100},
            {3, "Charlie", 3, 100}
        });

        p.removeSong(1);
        EXPECT_EQ(p.getSize(), 2, "PL remove: removing middle index decreases size");
        EXPECT_EQ_STR(song_brief(p.getSong(0)), "Alpha#1",
                      "PL remove: first remaining song is Alpha");
        EXPECT_EQ_STR(song_brief(p.getSong(1)), "Charlie#3",
                      "PL remove: second remaining song is Charlie");
    }

    {
        Playlist p("RemoveCurrent");
        addSongs(p, {
            {1, "Alpha",   1, 100},
            {2, "Beta",    2, 100},
            {3, "Charlie", 3, 100},
            {4, "Delta",   4, 100}
        });

        EXPECT_EQ_STR(song_brief(p.playNext()), "Alpha#1", "PL remove-current: start Alpha");
        EXPECT_EQ_STR(song_brief(p.playNext()), "Beta#2",  "PL remove-current: advance to Beta");
        p.removeSong(1);
        EXPECT_EQ(p.getSize(), 3, "PL remove-current: size decreases after deleting current song");
        EXPECT_TRUE(p.getSong(3) == nullptr, "PL remove-current: old last index becomes invalid");
        EXPECT_TRUE(TestHelper::currentIndex(p) >= -1 && TestHelper::currentIndex(p) < p.getSize(),
                    "PL remove-current: currentIndex remains within valid range or resets to -1");
    }

    {
        Playlist p("OnlyOne");
        p.addSong(mkSong(1, "Only", 8, 100));
        p.playNext();
        p.removeSong(0);
        EXPECT_TRUE(p.empty(), "PL remove-current: deleting only song empties playlist");
        EXPECT_EQ(TestHelper::currentIndex(p), -1,
                  "PL remove-current: currentIndex reset after deleting only song");
        EXPECT_TRUE(p.playNext() == nullptr, "PL remove-current: playNext on empty -> nullptr");
    }
}

static void suite_playlist_remove_current_head() {
    Playlist p("CurrentHead");
    addSongs(p, {
        {1, "Alpha", 1, 100},
        {2, "Beta", 2, 100},
        {3, "Charlie", 3, 100}
    });

    EXPECT_EQ_STR(song_brief(p.playNext()), "Alpha#1", "PL remove-head: start at first song");
    EXPECT_EQ(TestHelper::currentIndex(p), 0, "PL remove-head: currentIndex = 0 before deletion");

    p.removeSong(0);

    EXPECT_EQ(p.getSize(), 2, "PL remove-head: size decreases after deleting current head");
    EXPECT_EQ_STR(song_brief(p.getSong(0)), "Beta#2",
                  "PL remove-head: new first song is Beta");
    EXPECT_EQ_STR(song_brief(p.getSong(1)), "Charlie#3",
                  "PL remove-head: second song is Charlie");
    EXPECT_TRUE(TestHelper::currentIndex(p) >= -1 && TestHelper::currentIndex(p) < p.getSize(),
                "PL remove-head: currentIndex remains within valid range or resets to -1");
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
    {
        Playlist p("Random");
        addSongs(p, {
            {1, "A", 1, 100},
            {2, "B", 2, 100},
            {3, "C", 3, 100},
            {4, "D", 4, 100},
            {5, "E", 5, 100}
        });

        string out = capture_cout([&]() { p.playRandom(2); });
        EXPECT_EQ_STR(out, "", "PL: playRandom(valid) prints nothing");
        EXPECT_EQ(TestHelper::currentIndex(p), 2, "PL: playRandom sets currentIndex");
#ifdef USE_THREADED_AVL
        EXPECT_TRUE(TestHelper::hasCurrent(p), "PL: playRandom sets hasCurrent true");
#endif
        EXPECT_EQ_STR(song_brief(p.playNext()), "D#4",
                      "PL: playNext continues from playRandom-selected song");
        EXPECT_EQ_STR(song_brief(p.playPrevious()), "C#3",
                      "PL: playPrevious returns to song selected by playRandom");
    }

    {
        Playlist p("ApproxTeacher");
        for (int i = 0; i < 10; ++i) {
            string title = string("Song_") + char('A' + i);
            p.addSong(mkSong(i, title, 50 + i, 100));
        }

        EXPECT_EQ(TestHelper::currentIndex(p), -1,
                  "PL: currentIndex starts at -1 before playApproximate");
        EXPECT_EQ(p.playApproximate(3), 3,
                  "PL: playApproximate(3) from no current treats start as index 0");
        EXPECT_EQ(TestHelper::currentIndex(p), 3,
                  "PL: currentIndex becomes 3 after first playApproximate");
        EXPECT_EQ(p.playApproximate(4), 7,
                  "PL: second playApproximate accumulates from current position");
        EXPECT_EQ_STR(song_brief(p.getSong(7)), "Song_H#7",
                      "PL: getSong(new index) returns expected song after playApproximate");
    }

    {
        Playlist p("ApproxWrap");
        addSongs(p, {
            {1, "A", 1, 100},
            {2, "B", 2, 100},
            {3, "C", 3, 100},
            {4, "D", 4, 100},
            {5, "E", 5, 100}
        });

        p.playRandom(4);
        EXPECT_EQ(p.playApproximate(2), 1,
                  "PL: playApproximate positive step wraps around");
        EXPECT_EQ(TestHelper::currentIndex(p), 1,
                  "PL: currentIndex updated after positive wrap-around");

        p.playRandom(0);
        EXPECT_EQ(p.playApproximate(-1), 4,
                  "PL: playApproximate negative step wraps around");
        EXPECT_EQ(TestHelper::currentIndex(p), 4,
                  "PL: currentIndex updated after negative wrap-around");
    }
}

static void suite_playlist_approximate_large_steps() {
    Playlist p("ApproxLarge");
    addSongs(p, {
        {1, "A", 1, 100},
        {2, "B", 2, 100},
        {3, "C", 3, 100},
        {4, "D", 4, 100},
        {5, "E", 5, 100}
    });

    EXPECT_EQ(p.playApproximate(12), 2,
              "PL approx: from no current, start at 0 then wrap +12 -> index 2");
    EXPECT_EQ(TestHelper::currentIndex(p), 2,
              "PL approx: currentIndex correct after large positive step");

    EXPECT_EQ(p.playApproximate(-13), 4,
              "PL approx: wrap correctly on large negative step");
    EXPECT_EQ(TestHelper::currentIndex(p), 4,
              "PL approx: currentIndex correct after large negative step");
}

static void suite_playlist_compare_edge_cases() {
    {
        Playlist p1("P1");
        Playlist p2("P2");
        addSongs(p1, {{1, "A", 5, 100}});
        addSongs(p2, {{2, "B", 9, 100}});
        EXPECT_TRUE(p1.compareTo(p2, 0),
                    "PL compare: compare first 0 songs should be true (0 >= 0)");
    }

    {
        Playlist p1("P1");
        Playlist p2("P2");
        addSongs(p1, {{1, "A", 5, 100}, {2, "B", 6, 100}});
        addSongs(p2, {{3, "A", 5, 100}});
        EXPECT_TRUE(p1.compareTo(p2, 10),
                    "PL compare: numSong larger than size sums all available songs");
    }

    {
        Playlist empty("Empty");
        Playlist p("P");
        addSongs(p, {{1, "A", 1, 100}});
        EXPECT_TRUE(!empty.compareTo(p, 1),
                    "PL compare: empty playlist is smaller than non-empty for first 1 song");
    }
}

static void suite_playlist_duplicate_key() {
    Playlist p("Dup");
    p.addSong(mkSong(1, "Alpha", 5, 100));
    p.addSong(mkSong(1, "Alpha", 9, 200)); // same title, same id => duplicate SongKey

    EXPECT_EQ(p.getSize(), 1, "PL dup: duplicate SongKey should not increase size");
    EXPECT_EQ_STR(song_brief(p.getSong(0)), "Alpha#1", "PL dup: original song remains");
    EXPECT_TRUE(p.getSong(1) == nullptr, "PL dup: only one song stored");
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
    EXPECT_EQ_STR(TestHelper::title(*p.getSong(0)), "Song0001",
                  "Stress PL: songs still sorted alphabetically");
    p.playRandom(1999);
    EXPECT_EQ(TestHelper::currentIndex(p), 1999, "Stress PL: playRandom large valid index");
    EXPECT_EQ(p.playApproximate(100), 99,
              "Stress PL: playApproximate wraps around on large positive step");
}

// ===================== main =====================
int main(int argc, char* argv[]) {
    string mode = (argc > 1) ? argv[1] : "all";

    auto print_summary = []() {
        cout << "\n" << C_CYAN << "===== SUMMARY =====" << C_RESET << "\n";
        cout << C_GREEN << "PASS: " << g_pass << C_RESET << "\n";
        if (g_fail == 0) cout << C_GREEN << "FAIL: 0" << C_RESET << "\n";
        else cout << C_RED << "FAIL: " << g_fail << C_RESET << "\n";
    };

    if (mode == "all") {
        RUN_TEST("Song", suite_song_basic);
        RUN_TEST("AVL Basic", suite_avl_basic);
        RUN_TEST("AVL Rotations & Height", suite_avl_rotations_and_height);
    #ifdef USE_THREADED_AVL
        RUN_TEST("ThreadedAVL Basic", suite_threaded_avl_basic);
        RUN_TEST("ThreadedAVL Erase & Threads", suite_threaded_avl_erase_and_threads);
    #endif
        RUN_TEST("Playlist Basic Order", suite_playlist_basic_order);
        RUN_TEST("Playlist Playback Navigation", suite_playlist_playback_navigation);
        RUN_TEST("Playlist Remove Current Behavior", suite_playlist_remove_current_behavior);
        RUN_TEST("Playlist Remove Current Head", suite_playlist_remove_current_head);
        RUN_TEST("Playlist Score & Compare", suite_playlist_score_compare);
        RUN_TEST("Playlist Random & Approximate", suite_playlist_random_and_approximate);
        RUN_TEST("Playlist Approximate Large Steps", suite_playlist_approximate_large_steps);
        RUN_TEST("Playlist Compare Edge Cases", suite_playlist_compare_edge_cases);
        RUN_TEST("Playlist Duplicate Key", suite_playlist_duplicate_key);
        RUN_TEST("Stress", suite_stress);
    }
    else if (mode == "song") {
        RUN_TEST("Song", suite_song_basic);
    }
    else if (mode == "avl") {
        RUN_TEST("AVL Basic", suite_avl_basic);
        RUN_TEST("AVL Rotations & Height", suite_avl_rotations_and_height);
    }
    else if (mode == "threaded") {
    #ifdef USE_THREADED_AVL
        RUN_TEST("ThreadedAVL Basic", suite_threaded_avl_basic);
        RUN_TEST("ThreadedAVL Erase & Threads", suite_threaded_avl_erase_and_threads);
    #else
        cout << "ThreadedAVL not enabled. Rebuild with -DUSE_THREADED_AVL\n";
    #endif
    }
    else if (mode == "playlist") {
        RUN_TEST("Playlist Basic Order", suite_playlist_basic_order);
        RUN_TEST("Playlist Playback Navigation", suite_playlist_playback_navigation);
        RUN_TEST("Playlist Remove Current Behavior", suite_playlist_remove_current_behavior);
        RUN_TEST("Playlist Remove Current Head", suite_playlist_remove_current_head);
        RUN_TEST("Playlist Score & Compare", suite_playlist_score_compare);
        RUN_TEST("Playlist Random & Approximate", suite_playlist_random_and_approximate);
        RUN_TEST("Playlist Approximate Large Steps", suite_playlist_approximate_large_steps);
        RUN_TEST("Playlist Compare Edge Cases", suite_playlist_compare_edge_cases);
        RUN_TEST("Playlist Duplicate Key", suite_playlist_duplicate_key);
    }
    else if (mode == "stress") {
        RUN_TEST("Stress", suite_stress);
    }

    // More detailed modes for debugging specific test suites.
    else if (mode == "avl-basic") {
        RUN_TEST("AVL Basic", suite_avl_basic);
    }
    else if (mode == "avl-rotation") {
        RUN_TEST("AVL Rotations & Height", suite_avl_rotations_and_height);
    }
    else if (mode == "playlist-order") {
        RUN_TEST("Playlist Basic Order", suite_playlist_basic_order);
    }
    else if (mode == "playlist-nav") {
        RUN_TEST("Playlist Playback Navigation", suite_playlist_playback_navigation);
    }
    else if (mode == "playlist-remove-current") {
        RUN_TEST("Playlist Remove Current Behavior", suite_playlist_remove_current_behavior);
    }
    else if (mode == "playlist-remove-head") {
        RUN_TEST("Playlist Remove Current Head", suite_playlist_remove_current_head);
    }
    else if (mode == "playlist-score") {
        RUN_TEST("Playlist Score & Compare", suite_playlist_score_compare);
    }
    else if (mode == "playlist-jump") {
        RUN_TEST("Playlist Random & Approximate", suite_playlist_random_and_approximate);
    }
    else if (mode == "playlist-approx-large") {
        RUN_TEST("Playlist Approximate Large Steps", suite_playlist_approximate_large_steps);
    }
    else if (mode == "playlist-compare-edge") {
        RUN_TEST("Playlist Compare Edge Cases", suite_playlist_compare_edge_cases);
    }
    else if (mode == "playlist-dup") {
        RUN_TEST("Playlist Duplicate Key", suite_playlist_duplicate_key);
    }
    else if (mode == "help") {
    cout << "Usage:\n";
    cout << "  ./test_basic [mode]       (AVL mode)\n";
    cout << "  ./test_threaded [mode]    (ThreadedAVL mode)\n\n";

    cout << "Execution modes (runtime):\n";
    cout << "  all (default)   - run all tests\n";
    cout << "  song            - test Song class\n";
    cout << "  avl             - test AVL (insert, delete, balance)\n";
    cout << "  playlist        - test Playlist functionality\n";
    cout << "  stress          - run stress tests\n";
    cout << "  threaded        - test ThreadedAVL (only in threaded build)\n\n";

    cout << "Detailed modes (for debugging):\n";
    cout << "  avl-basic\n";
    cout << "  avl-rotation\n";
    cout << "  playlist-order\n";
    cout << "  playlist-nav\n";
    cout << "  playlist-remove-current\n";
    cout << "  playlist-remove-head\n";
    cout << "  playlist-score\n";
    cout << "  playlist-jump\n\n";
    cout << "  playlist-approx-large\n";
    cout << "  playlist-compare-edge\n";
    cout << "  playlist-dup\n\n";

    cout << "Notes:\n";
    cout << "  - test_basic uses AVL implementation\n";
    cout << "  - test_threaded enables ThreadedAVL (compile with -DUSE_THREADED_AVL)\n";
    cout << "  - 'threaded' mode only works in test_threaded\n";
    }
    else {
        cout << "Unknown mode: " << mode << "\n";
        cout << "Use './test_basic help' or './test_threaded help'\n";
    }

    print_summary();
    return (g_fail == 0) ? 0 : 1;
}
