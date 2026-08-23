#include<iostream>
#include<vector>
#include<list>
#include<string>
#include<random>
#include<unordered_set>
#include<numeric>
#include<iomanip>
#include<algorithm>
#include<optional>
#include<utility>
#include<sstream>
using namespace std;


const int INITIAL_TABLE_SIZE = 13;
const double MAX_LOAD_FACTOR = 0.50;
const double MIN_LOAD_FACTOR = 0.25;
const int C1 = 1;
const int C2 = 1;
const int CHAINING=1;
const int DOUBLE_HASHING=2;
const int CUSTOM_PROBING=3;
const unsigned long long UNIVERSAL_PRIME = 1000000007ULL;

template<class Key, class Value>
class HashTable{
    struct Entry{
        Key key;
        Value value;
        bool deleted=false;
    };

    int method;
    int hashChoice;
    int initialSize, tableSize, elementCount=0;
    double maxLoad, minLoad;
    unsigned long long universalA, universalB;

    vector<list<pair<Key,Value>>> chains;
    vector<optional<Entry>> slots;

    long long collisions=0;
    int insertionsSinceExpansion=0, deletionsSinceCompaction=0;
    bool hasExpanded=false, hasCompacted=false;

    static bool isPrime(int x){
        if(x<2)return false;
        if(x%2==0)return x==2;
        for(int d=3; d<=x/d; d+=2){
            if(x%d==0)return false;
        }
        return true;
    }

    static int nextPrime(int x){
        while(!isPrime(x))++x;
        return x;
    }

    static int previousPrime(int x){
        while(x>2 && !isPrime(x))--x;
        return x;
    }

       static string keyAsString(const Key& key) {
        ostringstream output;
        output<<key;
        return output.str();
    }

static unsigned long long stringToNumberModulo(
    const Key& key,
    unsigned long long modulus
) {
    ostringstream output;
    output << key;

    string s = output.str();

    unsigned long long number = 0;

    for (unsigned char c : s) {
        number = (number * 128ULL + c) % modulus;
    }

    return number;
}

    int divisionHash(const Key& key)const{
        return (int)stringToNumberModulo(key, tableSize);
    }

    int universalHash(const Key& key)const{
        unsigned long long k=stringToNumberModulo(key, UNIVERSAL_PRIME);
        unsigned long long hashValue=(universalA*k+universalB)%UNIVERSAL_PRIME;

        return (int)(hashValue%tableSize);
    }

    int primary(const Key& key)const{
        return hashChoice==1?divisionHash(key):universalHash(key);
    }

    int auxiliary(const Key& key)const{
        return 1+(int)stringToNumberModulo(key, tableSize-1);
    }

    int getIndex(const Key& key, int i)const{
        unsigned long long h=primary(key), step=auxiliary(key);
          unsigned long long attempt = (unsigned long long)i;

    if (method==DOUBLE_HASHING) {
        return (h + attempt * step) % tableSize;
    }

        return(h+C1*i*step+C2*i*i)%tableSize;
    }

    bool insertWithoutResize(const Key& key, const Value& value, bool countCollision){
        if(method==CHAINING){
            int p=primary(key);
            for(auto& kv: chains[p]){
                if(kv.first==key){
                    kv.second=value;
                    return false;
                }
            }
            if(countCollision && !chains[p].empty())++collisions;
            chains[p].push_back({key, value});
            ++elementCount;
            return true;
        }

        optional<int> firstDeleted;
        for(int i=0; i<tableSize; ++i){
            int p=getIndex(key, i);
            if(!slots[p].has_value()){
                int target=firstDeleted.value_or(p);
                slots[target]=Entry{key, value, false};
                ++elementCount;
                return true;
            }
            if(slots[p]->deleted){
                if(!firstDeleted)firstDeleted=p;
            }else if(slots[p]->key==key){
                slots[p]->value=value;
                return false;
            }else if(countCollision){
                ++collisions;
            }
        }
        if(firstDeleted){
            slots[*firstDeleted]=Entry{key, value, false};
            ++elementCount;
            return true;
        }
        return false;
    }

    vector<pair<Key, Value>> allItems() const{
        vector<pair<Key, Value>> items;
        items.reserve(elementCount);
        if(method==CHAINING){
            for(const auto& bucket: chains){
                for(const auto& kv: bucket)items.push_back(kv);
            }
        }else{
            for(const auto& e: slots)
            if(e&&!e->deleted)items.push_back({e->key, e->value});
        }
        return items;
    }

    void rehash(int newSize){
        auto items=allItems();
        tableSize=nextPrime(max(initialSize, newSize));
        chains.clear();
        slots.clear();
        if(method==CHAINING)chains.resize(tableSize);
        else slots.resize(tableSize);
        elementCount=0;
        for(const auto& [k,v]: items)insertWithoutResize(k,v,false);
    }
    void considerExpansion(){
        if((double)elementCount/tableSize<=maxLoad)return;
        if(hasExpanded && 2LL*insertionsSinceExpansion<elementCount)return;
        rehash(nextPrime(2*tableSize+1));
        hasExpanded=true;
        insertionsSinceExpansion=0;
    }
    void considerCompaction(){
        if(tableSize==initialSize ||
        (double)elementCount/tableSize>=minLoad)return;
        if(hasCompacted && 2LL*deletionsSinceCompaction<elementCount)return;
        int candidate=previousPrime((tableSize-1)/2);
        rehash(max(initialSize, candidate));
        hasCompacted=true;
        deletionsSinceCompaction=0;
    }
    public:
    HashTable(int m,
         int whichHash,
          int startSize=INITIAL_TABLE_SIZE,
    double upper=MAX_LOAD_FACTOR,
     double lower=MIN_LOAD_FACTOR,
    unsigned long long a=151ULL,
unsigned long long b=263ULL):method(m), hashChoice(whichHash),
initialSize(nextPrime(startSize)),tableSize(initialSize),maxLoad(upper), minLoad(lower),
universalA(a), universalB(b){
    if(method==CHAINING)chains.resize(tableSize);
    else slots.resize(tableSize);
}

bool insert(const Key& key, const Value& value){
    long long ignored;
    if(search(key, ignored)!=nullptr){
        insertWithoutResize(key, value, false);
        return false;
    }
    considerExpansion();
    while(!insertWithoutResize(key, value, true)){
        rehash(nextPrime(2*tableSize+1));
        hasExpanded=true;
        insertionsSinceExpansion=0;
    }
    ++insertionsSinceExpansion;
    considerExpansion();
    return true;
}

Value* search(const Key& key, long long& hits){
    hits=0;
    if(method==CHAINING){
        int p=primary(key);
        for(auto& kv: chains[p]){
            ++hits;
            if(kv.first==key)return &kv.second;
        }
        return nullptr;
    }
    for(int i=0;i<tableSize; ++i){
        int p=getIndex(key, i);
        ++hits;
        if(!slots[p])return nullptr;
        if(!slots[p]->deleted && slots[p]->key==key)return &slots[p]->value;
    }
    return nullptr;
}

bool erase(const Key& key){
  if (method == CHAINING) {
            int p = primary(key);
            for (auto it=chains[p].begin(); it != chains[p].end(); ++it) {
                if (it->first == key) {
                    chains[p].erase(it); --elementCount; ++deletionsSinceCompaction;
                    considerCompaction(); return true;
                }
            }
            return false;
        }
        for (int i = 0;i < tableSize; ++i) {
            int p = getIndex(key,i);
            if (!slots[p]) return false;
            if (!slots[p]->deleted && slots[p]->key == key) {
                slots[p]->deleted = true; 
                --elementCount; 
                ++deletionsSinceCompaction;
                considerCompaction(); 
                return true;
            }
        }
        return false;
}
int hashIndex(const Key& key) const {
    return primary(key);
}
long long collisionCount() const{
    return collisions;
}
int size() const{
    return elementCount;
}
int capacity() const{
    return tableSize;
}
};

vector<string> generateUniqueWords(int count, int length, mt19937& rng ){
    const string alphabet="abcdefghijklmnopqrstuvwxyz";
    uniform_int_distribution<int> pick(0,25);
    unordered_set<string> used;
   
    vector<string> words;
    words.reserve(count);
    while((int)words.size()<count){
        string s(length, 'a');
        for(char& c:s)c=alphabet[pick(rng)];
        if(used.insert(s).second)words.push_back(s);
    }
    return words;
}
string methodName(int m){
    if(m==CHAINING)return "Chaining";
    if(m==DOUBLE_HASHING)return "Double Hashing";
    return "Custom Probing";
}
struct ReportResult {
    long long collisions = 0;
    double averageHits = 0.0;
    int uniqueHashValues = 0;
};

int main(){
    const int WORD_COUNT=10000;
    const int REPORT_WORD_LENGTH=10;
    const int SEARCH_COUNT=1000;
    const int DEMO_WORD_COUNT=10;

    int userWordLength;
    cout << "Enter word length for demonstration: ";
    cin>>userWordLength;
     mt19937 rng(42);

    uniform_int_distribution<unsigned long long> chooseA(
        1,
        UNIVERSAL_PRIME - 1
    );
    uniform_int_distribution<unsigned long long> chooseB(
        0,
        UNIVERSAL_PRIME - 1
    );
    unsigned long long universalA = chooseA(rng);
    unsigned long long universalB = chooseB(rng);
    vector<string> demoWords = generateUniqueWords(
        DEMO_WORD_COUNT,
        userWordLength,
        rng
    );
     HashTable<string, int> demoTable(
        CHAINING,
        1,
        INITIAL_TABLE_SIZE,
        MAX_LOAD_FACTOR,
        MIN_LOAD_FACTOR,
        universalA,
        universalB
    );
        for (int i = 0; i < (int)demoWords.size(); i++) {
        demoTable.insert(demoWords[i], i + 1);
    }
     cout << "Generated and inserted "
         << demoWords.size()
         << " unique words of length "
         << userWordLength
         << ".\n\n";
    vector<string> words = generateUniqueWords(
        WORD_COUNT,
        REPORT_WORD_LENGTH,
        rng
    );
    vector<int> sample(WORD_COUNT);
    iota(sample.begin(), sample.end(), 0);
    shuffle(sample.begin(), sample.end(), rng);
    sample.resize(SEARCH_COUNT);
    vector<int> methods = {
        CHAINING,
        DOUBLE_HASHING,
        CUSTOM_PROBING
    };
     vector<vector<ReportResult>> results(
        3,
        vector<ReportResult>(2)
    );
     for (
        int methodIndex = 0;
        methodIndex < (int)methods.size();
        methodIndex++
    ) {
        for (int hashNo = 1; hashNo <= 2; hashNo++) {
            HashTable<string, int> table(
                methods[methodIndex],
                hashNo,
                INITIAL_TABLE_SIZE,
                MAX_LOAD_FACTOR,
                MIN_LOAD_FACTOR,
                universalA,
                universalB
            );
 for (int i = 0; i < (int)words.size(); i++) {
                if (!table.insert(words[i], i + 1)) {
                    cerr << "Insertion failed.\n";
                    return 1;
                }
            }
            long long totalHits = 0;

            for (int index : sample) {
                long long hits = 0;

                if (table.search(words[index], hits) == nullptr) {
                    cerr << "Internal error: key not found.\n";
                    return 1;
                }
                    totalHits += hits;
            }
             vector<bool> seen(table.capacity(), false);

            int uniqueHashValues = 0;

            for (const string& word : words) {
                int index = table.hashIndex(word);

                if (!seen[index]) {
                    seen[index] = true;
                    uniqueHashValues++;
                }
            }

                       if (uniqueHashValues * 100 < WORD_COUNT * 60) {
                cerr << "Hash"
                     << hashNo
                     << " failed the 60% unique-hash requirement.\n";

                return 1;
            }
             results[methodIndex][hashNo - 1].collisions =
                table.collisionCount();

            results[methodIndex][hashNo - 1].averageHits =
                (double)totalHits / SEARCH_COUNT;

            results[methodIndex][hashNo - 1].uniqueHashValues =
                uniqueHashValues;
        }
    }
    cout << left
         << setw(20) << ""
         << setw(38) << "Hash1"
         << "Hash2\n";

    cout << left
         << setw(20) << "Method"
         << setw(22) << "Number of Collisions"
         << setw(16) << "Average Hits"
         << setw(22) << "Number of Collisions"
         << "Average Hits\n";

    cout << string(96, '-') << '\n';

    for (
        int methodIndex = 0;
        methodIndex < (int)methods.size();
        methodIndex++
    ) {
        ReportResult first = results[methodIndex][0];
        ReportResult second = results[methodIndex][1];

        cout << left
             << setw(20) << methodName(methods[methodIndex])
             << setw(22) << first.collisions
             << setw(16) << fixed << setprecision(3)
             << first.averageHits
             << setw(22) << second.collisions
             << fixed << setprecision(3)
             << second.averageHits
             << '\n';
    }

    cout << "\nHash quality verification:\n";

    for (int hashNo = 1; hashNo <= 2; hashNo++) {
        int uniqueCount =
            results[0][hashNo - 1].uniqueHashValues;

        double percentage =
            100.0 * uniqueCount / WORD_COUNT;

        cout << "Hash"
             << hashNo
             << ": "
             << uniqueCount
             << " unique hash values out of "
             << WORD_COUNT
             << " keys ("
             << fixed
             << setprecision(2)
             << percentage
             << "%) - PASS\n";
    }

    return 0;


}

// g++ -std=c++17 -O2 Hashing.cpp -o Hashing.exe
// .\Hashing.exe
