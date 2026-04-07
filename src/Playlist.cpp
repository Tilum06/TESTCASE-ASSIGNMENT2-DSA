#include "Playlist.h"

// =======================
// Song implementation (TODO)
// =======================

Song::Song(int id,
           string title,
           string artist,
           string album,
           int duration,
           int score,
           string url)
    : id(id),
      title(title),
      artist(artist),
      album(album),
      duration(duration),
      score(score),
      url(url),
      play_count(0)
{
}

std::string Song::toString() const {
    std::ostringstream oss;
    oss << "Song[id=" << id 
        << ", title=\"" << title << "\""
        << ", artist=\"" << artist << "\""
        << ", album=\"" << album << "\""
        << ", duration=" << duration
        << ", score=" << score
        << ", url=\"" << url << "\""
        << ", play_count=" << play_count
        << "]";
    return oss.str();
}

// TODO: Student can implement additional methods for Song here

// =======================
// Playlist implementation (TODO)
// =======================

Playlist::Playlist(string name)
    : name(name),
      size(0),
      currentIndex(-1)
#ifdef USE_THREADED_AVL
    , currentIt(songs.endIt()),
      hasCurrent(false)
#endif
{
    // TODO
}

SongKey Playlist::makeKey(Song* /*s*/) const {
    // TODO
    return SongKey("", 0);
}

void Playlist::resetPlayback() {
    // TODO
    currentIndex = -1;
#ifdef USE_THREADED_AVL
    hasCurrent = false;
    currentIt = songs.endIt();
#endif
}

int Playlist::getSize() const {
    // TODO
    return size;
}

bool Playlist::empty() const {
    // TODO
    return size == 0;
}

void Playlist::clear() {
    // TODO: delete Song* if Playlist owns them + clear the tree
}

void Playlist::addSong(Song* /*s*/) {
    // TODO
}

void Playlist::removeSong(int /*index*/) {
    // TODO
}

Song* Playlist::getSong(int /*index*/) const {
    // TODO
    return nullptr;
}

// =======================
// Playing control (TODO)
// =======================

Song* Playlist::playNext() {
    // TODO
    return nullptr;
}

Song* Playlist::playPrevious() {
    // TODO
    return nullptr;
}

// =======================
// Score-related (TODO)
// =======================

int Playlist::getTotalScore() {
    // TODO
    return 0;
}

bool Playlist::compareTo(const Playlist& /*p*/, int /*numSong*/) {
    // TODO
    return false;
}

// =======================
// Advanced playing modes (TODO)
// =======================

void Playlist::playRandom(int /*index*/) {
    // TODO
}

int Playlist::playApproximate(int /*step*/) {
    // TODO
    return -1;
}