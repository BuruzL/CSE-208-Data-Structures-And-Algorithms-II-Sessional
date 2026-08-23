#include <bits/stdc++.h>
using namespace std;

const int CHAINING = 1;
const int DOUBLE_HASHING = 2;
const int CUSTOM_PROBING = 3;

template<class Key, class Value>
class HashTable {
    struct Entry { Key key; Value value; bool deleted = false; };

    int method;
    int hashChoice;
    int initialSize, tableSize, elementCount = 0;
    double maxLoad, minLoad;
    vector<list<pair<Key, Value>>> chains;
    vector<optional<Entry>> slots;
    long long collisions = 0;
    int insertionsSinceExpansion = 0, deletionsSinceCompaction = 0;
    bool hasExpanded = false, hasCompacted = false;

    static bool isPrime(int x) {
        if (x < 2) return false;
        if (x % 2 == 0) return x == 2;
        for (int d = 3; d <= x / d; d += 2)
            if (x % d == 0) return false;
        return true;
    }
    static int nextPrime(int x) {
        while (!isPrime(x)) ++x;
        return x;
    }
    static int previousPrime(int x) {
        while (x > 2 && !isPrime(x)) --x;
        return x;
    }

    // Two standard string hashes: polynomial rolling and FNV-1a.
    static unsigned long long hash1Raw(const Key& key) {
        string s = key;
        unsigned long long h = 0;
        for (unsigned char c : s) h = h * 131ULL + c;
        return h;
    }
    static unsigned long long hash2Raw(const Key& key) {
        string s = key;
        unsigned long long h = 1469598103934665603ULL;
        for (unsigned char c : s) { h ^= c; h *= 1099511628211ULL; }
        return h;
    }
    unsigned long long primaryRaw(const Key& key) const {
        return hashChoice == 1 ? hash1Raw(key) : hash2Raw(key);
    }
    int primary(const Key& key) const { return primaryRaw(key) % tableSize; }
    int auxiliary(const Key& key) const {
        // tableSize is prime, so this step is coprime with tableSize.
        return 1 + hash2Raw(key) % (tableSize - 1);
    }
    int probeIndex(const Key& key, int i) const {
        unsigned long long h = primary(key), step = auxiliary(key);
        if (method == DOUBLE_HASHING)
            return (h + i * step) % tableSize;
        constexpr unsigned long long C1 = 1, C2 = 1;
        return (h + C1 * i * step + C2 * i * i) % tableSize;
    }

    bool insertWithoutResize(const Key& key, const Value& value, bool countCollision) {
        if (method == CHAINING) {
            int p = primary(key);
            for (auto& kv : chains[p]) {
                if (kv.first == key) { kv.second = value; return false; }
            }
            if (countCollision && !chains[p].empty()) ++collisions;
            chains[p].push_back({key, value});
            ++elementCount;
            return true;
        }

        optional<int> firstDeleted;
        for (int i = 0; i < tableSize; ++i) {
            int p = probeIndex(key, i);
            if (!slots[p].has_value()) {
                int target = firstDeleted.value_or(p);
                slots[target] = Entry{key, value, false};
                ++elementCount;
                return true;
            }
            if (slots[p]->deleted) {
                if (!firstDeleted) firstDeleted = p;
            } else if (slots[p]->key == key) {
                slots[p]->value = value;
                return false;
            } else if (countCollision) {
                ++collisions;
            }
        }
        if (firstDeleted) {
            slots[*firstDeleted] = Entry{key, value, false};
            ++elementCount;
            return true;
        }
        return false; // A custom quadratic probe sequence may not visit every slot.
    }

    vector<pair<Key, Value>> allItems() const {
        vector<pair<Key, Value>> items;
        items.reserve(elementCount);
        if (method == CHAINING) {
            for (const auto& bucket : chains)
                for (const auto& kv : bucket) items.push_back(kv);
        } else {
            for (const auto& e : slots)
                if (e && !e->deleted) items.push_back({e->key, e->value});
        }
        return items;
    }

    void rehash(int newSize) {
        auto items = allItems();
        tableSize = nextPrime(max(initialSize, newSize));
        chains.clear(); slots.clear();
        if (method == CHAINING) chains.resize(tableSize);
        else slots.resize(tableSize);
        elementCount = 0;
        for (const auto& [k, v] : items) insertWithoutResize(k, v, false);
    }

    void considerExpansion() {
        if ((double)elementCount / tableSize <= maxLoad) return;
        // Before the first expansion there is no previous expansion to wait for.
        if (hasExpanded && insertionsSinceExpansion < elementCount / 2) return;
        rehash(nextPrime(2 * tableSize + 1));
        hasExpanded = true;
        insertionsSinceExpansion = 0;
    }
    void considerCompaction() {
        if (tableSize == initialSize || (double)elementCount / tableSize >= minLoad) return;
        // Before the first compaction there is no previous compaction to wait for.
        if (hasCompacted && deletionsSinceCompaction < elementCount / 2) return;
        int candidate = previousPrime(tableSize / 2 - 1);
        rehash(max(initialSize, candidate));
        hasCompacted = true;
        deletionsSinceCompaction = 0;
    }

public:
    HashTable(int m, int whichHash, int startSize = 13,
              double upper = 0.50, double lower = 0.25)
        : method(m), hashChoice(whichHash), initialSize(nextPrime(startSize)),
          tableSize(initialSize), maxLoad(upper), minLoad(lower) {
        if (method == CHAINING) chains.resize(tableSize);
        else slots.resize(tableSize);
    }

    bool insert(const Key& key, const Value& value) {
        // Update an existing key without changing size/counters.
        long long ignored;
        if (search(key, ignored) != nullptr) {
            insertWithoutResize(key, value, false);
            return false;
        }
        considerExpansion();
        while (!insertWithoutResize(key, value, true)) {
            rehash(nextPrime(2 * tableSize + 1));
            hasExpanded = true;
            insertionsSinceExpansion = 0;
        }
        ++insertionsSinceExpansion;
        // Insertion itself may push the load factor over the threshold.
        considerExpansion();
        return true;
    }

    Value* search(const Key& key, long long& hits) {
        hits = 0;
        if (method == CHAINING) {
            int p = primary(key);
            for (auto& kv : chains[p]) {
                ++hits;
                if (kv.first == key) return &kv.second;
            }
            return nullptr;
        }
        for (int i = 0; i < tableSize; ++i) {
            int p = probeIndex(key, i);
            ++hits;
            if (!slots[p]) return nullptr;
            if (!slots[p]->deleted && slots[p]->key == key) return &slots[p]->value;
        }
        return nullptr;
    }

    bool erase(const Key& key) {
        if (method == CHAINING) {
            int p = primary(key);
            for (auto it = chains[p].begin(); it != chains[p].end(); ++it) {
                if (it->first == key) {
                    chains[p].erase(it); --elementCount; ++deletionsSinceCompaction;
                    considerCompaction(); return true;
                }
            }
            return false;
        }
        for (int i = 0; i < tableSize; ++i) {
            int p = probeIndex(key, i);
            if (!slots[p]) return false;
            if (!slots[p]->deleted && slots[p]->key == key) {
                slots[p]->deleted = true; --elementCount; ++deletionsSinceCompaction;
                considerCompaction(); return true;
            }
        }
        return false;
    }

    long long collisionCount() const { return collisions; }
    int size() const { return elementCount; }
    int capacity() const { return tableSize; }
};

vector<string> generateUniqueWords(int count, int length, mt19937& rng) {
    const string alphabet = "abcdefghijklmnopqrstuvwxyz";
    uniform_int_distribution<int> pick(0, 25);
    unordered_set<string> used;
    vector<string> words;
    words.reserve(count);
    while ((int)words.size() < count) {
        string s(length, 'a');
        for (char& c : s) c = alphabet[pick(rng)];
        if (used.insert(s).second) words.push_back(s);
    }
    return words;
}

string methodName(int m) {
    if (m == CHAINING) return "Chaining";
    if (m == DOUBLE_HASHING) return "Double Hashing";
    return "Custom Probing";
}

int main() {
    constexpr int WORD_COUNT = 10000;
    constexpr int WORD_LENGTH = 10;
    constexpr int SEARCH_COUNT = 1000;
    constexpr int INITIAL_SIZE = 13;
    constexpr double MAX_LOAD = 0.50, MIN_LOAD = 0.25;

    mt19937 rng(42); // fixed seed makes the report reproducible
    vector<string> words = generateUniqueWords(WORD_COUNT, WORD_LENGTH, rng);
    vector<int> sample(WORD_COUNT);
    iota(sample.begin(), sample.end(), 0);
    shuffle(sample.begin(), sample.end(), rng);
    sample.resize(SEARCH_COUNT);

    vector<int> methods = {CHAINING, DOUBLE_HASHING, CUSTOM_PROBING};
    cout << left << setw(18) << "Method" << setw(8) << "Hash"
         << setw(18) << "Collisions" << "Average Hits\n";
    cout << string(58, '-') << '\n';

    for (int method : methods) {
        for (int hashNo = 1; hashNo <= 2; ++hashNo) {
            HashTable<string, int> table(method, hashNo, INITIAL_SIZE,
                                         MAX_LOAD, MIN_LOAD);
            for (int i = 0; i < (int)words.size(); ++i)
                table.insert(words[i], static_cast<int>(i + 1));

            long long totalHits = 0;
            for (int index : sample) {
                long long hits;
                if (!table.search(words[index], hits)) {
                    cerr << "Internal error: key not found\n";
                    return 1;
                }
                totalHits += hits;
            }
            cout << left << setw(18) << methodName(method)
                 << setw(8) << ("Hash" + to_string(hashNo))
                 << setw(18) << table.collisionCount()
                 << fixed << setprecision(3)
                 << (double)totalHits / SEARCH_COUNT << '\n';
        }
    }
}
