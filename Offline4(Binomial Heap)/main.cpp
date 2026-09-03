#include <algorithm>
#include <cmath>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

using namespace std;

class OutputWriter {
    ofstream file_;
public:
    explicit OutputWriter(const string& path) : file_(path) {
        if (!file_) throw runtime_error("Cannot open " + path);
    }
    void line(const string& s) { cout << s << '\n'; file_ << s << '\n'; }
};

class BinomialHeap {
public:
    struct Node {
        long long key;
        int degree = 0;
        Node* parent = nullptr;
        Node* child = nullptr;
        Node* sibling = nullptr;
        explicit Node(long long value) : key(value) {}
    };

    struct LinkEvent { long long parentKey, childKey; int oldDegree, newDegree; };

private:
    Node* head_ = nullptr;
    size_t size_ = 0;
    unordered_map<long long, Node*> location_;
    unordered_set<Node*> owned_;

    static Node* mergeRootLists(Node* a, Node* b) {
        Node dummy(0), *tail = &dummy;
        while (a && b) {
            if (a->degree <= b->degree) { tail->sibling = a; a = a->sibling; }
            else { tail->sibling = b; b = b->sibling; }
            tail = tail->sibling;
        }
        tail->sibling = a ? a : b;
        return dummy.sibling;
    }

    static void linkTrees(Node* child, Node* parent) {
        child->parent = parent;
        child->sibling = parent->child;
        parent->child = child;
        ++parent->degree;
    }

    void consolidate(const function<void(const LinkEvent&)>* onLink) {
        if (!head_) return;
        Node *prev = nullptr, *curr = head_, *next = curr->sibling;
        while (next) {
            if (curr->degree != next->degree ||
                (next->sibling && next->sibling->degree == curr->degree)) {
                prev = curr; curr = next;
            } else if (curr->key <= next->key) {
                int d = curr->degree;
                curr->sibling = next->sibling;
                linkTrees(next, curr);
                if (onLink) (*onLink)({curr->key, next->key, d, d + 1});
            } else {
                int d = curr->degree;
                if (prev) prev->sibling = next; else head_ = next;
                linkTrees(curr, next);
                if (onLink) (*onLink)({next->key, curr->key, d, d + 1});
                curr = next;
            }
            next = curr->sibling;
        }
    }

    void swapKeys(Node* a, Node* b) {
        swap(a->key, b->key);
        location_[a->key] = a;
        location_[b->key] = b;
    }

public:
    BinomialHeap() = default;
    BinomialHeap(const BinomialHeap&) = delete;
    BinomialHeap& operator=(const BinomialHeap&) = delete;
    ~BinomialHeap() { for (Node* p : owned_) delete p; }

    Node* head() const { return head_; }
    size_t size() const { return size_; }
    bool empty() const { return head_ == nullptr; }
    bool contains(long long key) const { return location_.count(key); }

    void insert(long long key) {
        Node* n = new Node(key);
        BinomialHeap one;
        one.head_ = n; one.size_ = 1; one.location_[key] = n; one.owned_.insert(n);
        meld(one);
    }

    void meld(BinomialHeap& other, const function<void(const LinkEvent&)>* onLink = nullptr) {
        if (this == &other) return;
        head_ = mergeRootLists(head_, other.head_);
        size_ += other.size_;
        location_.insert(other.location_.begin(), other.location_.end());
        owned_.insert(other.owned_.begin(), other.owned_.end());
        other.head_ = nullptr; other.size_ = 0;
        other.location_.clear(); other.owned_.clear();
        consolidate(onLink);
    }

    long long minimum() const {
        if (!head_) throw runtime_error("minimum on empty heap");
        Node* best = head_;
        for (Node* p = head_->sibling; p; p = p->sibling)
            if (p->key < best->key) best = p;
        return best->key;
    }

    long long extractMin() {
        if (!head_) throw runtime_error("extractMin on empty heap");
        Node *minRoot = head_, *minPrev = nullptr, *prev = nullptr;
        for (Node* p = head_; p; prev = p, p = p->sibling)
            if (p->key < minRoot->key) { minRoot = p; minPrev = prev; }
        if (minPrev) minPrev->sibling = minRoot->sibling;
        else head_ = minRoot->sibling;

        Node* reversed = nullptr;
        for (Node* c = minRoot->child; c;) {
            Node* next = c->sibling;
            c->parent = nullptr; c->sibling = reversed; reversed = c; c = next;
        }
        head_ = mergeRootLists(head_, reversed);
        long long answer = minRoot->key;
        location_.erase(answer); owned_.erase(minRoot); delete minRoot; --size_;
        consolidate(nullptr);
        return answer;
    }

    void decreaseKey(long long oldKey, long long newKey) {
        Node* x = location_.at(oldKey);
        location_.erase(oldKey); x->key = newKey; location_[newKey] = x;
        while (x->parent && x->key < x->parent->key) {
            swapKeys(x, x->parent);
            x = x->parent;
        }
    }

    void removeKey(long long key) {
        static constexpr long long SENTINEL = -1000000001LL;
        decreaseKey(key, SENTINEL);
        extractMin();
    }

    void print(const string& name, OutputWriter& out) const {
        out.line("Printing Binomial Heap " + name);
        out.line("Heap size: " + to_string(size_));
        if (!head_) { out.line("Heap " + name + " is empty."); return; }
        for (Node* root = head_; root; root = root->sibling) {
            out.line("Binomial Tree, B" + to_string(root->degree));
            vector<Node*> level{root}; int depth = 0;
            while (!level.empty()) {
                vector<long long> keys; vector<Node*> next;
                for (Node* n : level) {
                    keys.push_back(n->key);
                    for (Node* c = n->child; c; c = c->sibling) next.push_back(c);
                }
                sort(keys.begin(), keys.end());
                ostringstream line; line << "Level " << depth << ":";
                for (long long k : keys) line << ' ' << k;
                out.line(line.str()); level.swap(next); ++depth;
            }
        }
    }
};

class HeapVisualizer {
    using Node = BinomialHeap::Node;
    struct Point { double x, y; };
    static string esc(const string& s) {
        string r; for (char c : s) { if (c == '&') r += "&amp;"; else if (c == '<') r += "&lt;"; else r += c; } return r;
    }
    static int leaves(Node* n) {
        if (!n->child) return 1;
        int sum = 0; for (Node* c = n->child; c; c = c->sibling) sum += leaves(c); return sum;
    }
    static int subtreeSize(Node* n) {
        int sum = 1; for (Node* c = n->child; c; c = c->sibling) sum += subtreeSize(c); return sum;
    }
    static double place(Node* n, int depth, double left, map<Node*, Point>& pos) {
        constexpr double SX = 78, SY = 92;
        if (!n->child) { pos[n] = {left + SX / 2, 55.0 + depth * SY}; return left + SX; }
        double cursor = left, first = 0, last = 0; bool begin = true;
        for (Node* c = n->child; c; c = c->sibling) {
            double right = place(c, depth + 1, cursor, pos);
            if (begin) { first = pos[c].x; begin = false; } last = pos[c].x; cursor = right;
        }
        pos[n] = {(first + last) / 2, 55.0 + depth * SY}; return cursor;
    }
public:
    static string svg(const BinomialHeap& heap, const string& label) {
        map<Node*, Point> pos; double cursor = 40; vector<Node*> roots;
        int maxDepth = 0;
        for (Node* r = heap.head(); r; r = r->sibling) {
            roots.push_back(r); place(r, 0, cursor, pos); cursor += leaves(r) * 78 + 55; maxDepth = max(maxDepth, r->degree);
        }
        double width = max(420.0, cursor), height = 125.0 + maxDepth * 92;
        long long minKey = heap.empty() ? 0 : heap.minimum();
        ostringstream s;
        s << "<svg xmlns='http://www.w3.org/2000/svg' width='100%' viewBox='0 0 " << width << ' ' << height << "'>"
          << "<style>.edge{stroke:#64748b;stroke-width:2}.node{stroke:#1e293b;stroke-width:2}.key{font:700 14px sans-serif;text-anchor:middle;fill:white}.tag{font:600 13px sans-serif;fill:#334155}.title{font:700 18px sans-serif;fill:#0f172a}</style>"
          << "<text x='18' y='24' class='title'>" << esc(label) << " — size " << heap.size() << "</text>";
        for (auto& [n, p] : pos) for (Node* c = n->child; c; c = c->sibling)
            s << "<line class='edge' x1='" << p.x << "' y1='" << p.y << "' x2='" << pos[c].x << "' y2='" << pos[c].y << "'/>";
        for (Node* r : roots) s << "<text class='tag' x='" << pos[r].x << "' y='43'>B" << r->degree << "</text>";
        for (auto& [n, p] : pos) {
            string color = n->key == minKey ? "#e11d48" : "#2563eb";
            s << "<g><title>key=" << n->key << ", degree=" << n->degree << ", subtree nodes=" << subtreeSize(n) << "</title>"
              << "<circle class='node' fill='" << color << "' cx='" << p.x << "' cy='" << p.y << "' r='22'/>"
              << "<text class='key' x='" << p.x << "' y='" << (p.y + 5) << "'>" << n->key << "</text></g>";
        }
        s << "</svg>"; return s.str();
    }

    static void saveHeap(const BinomialHeap& heap, const string& name, const string& filename) {
        ofstream f(filename); if (!f) throw runtime_error("Cannot create " + filename);
        f << svg(heap, "Binomial Heap " + name);
    }

    static void saveUnionTrace(const string& filename, const string& h1, const string& h2,
                               const string& before1, const string& before2,
                               const vector<pair<BinomialHeap::LinkEvent,string>>& stages,
                               const string& finalSvg) {
        ofstream f(filename); if (!f) throw runtime_error("Cannot create " + filename);
        f << "<!doctype html><meta charset='utf-8'><title>Union trace</title><style>body{font-family:system-ui;background:#f1f5f9;color:#0f172a;margin:24px}.card{background:white;margin:18px auto;padding:18px;border-radius:14px;max-width:1100px;box-shadow:0 3px 14px #0002}h1,h2{margin:4px 0 12px}.note{color:#475569}.pair{display:grid;grid-template-columns:1fr 1fr;gap:12px}@media(max-width:700px){.pair{grid-template-columns:1fr}}</style>";
        f << "<h1>Binomial Heap Union: " << h1 << " + " << h2 << "</h1><p class='note'>Red node = current minimum. Hover over a node for key, degree, and subtree size.</p>";
        f << "<section class='card'><h2>Before union</h2><div class='pair'><div>" << before1 << "</div><div>" << before2 << "</div></div></section>";
        int i = 1; for (auto& st : stages) f << "<section class='card'><h2>Link " << i++ << ": B" << st.first.oldDegree << " + B" << st.first.oldDegree << " → B" << st.first.newDegree << "</h2><p>Root " << st.first.parentKey << " becomes parent; root " << st.first.childKey << " becomes its first child.</p>" << st.second << "</section>";
        f << "<section class='card'><h2>Final heap</h2>" << finalSvg << "</section>";
    }
};

class HeapApplication {
    BinomialHeap heaps_[2];
    OutputWriter output_{"output.txt"};
    int unionCount_ = 0;
    static int index(int h) { if (h != 1 && h != 2) throw runtime_error("Heap id must be 1 or 2"); return h - 1; }
public:
    void run(const string& inputPath) {
        ifstream in(inputPath); if (!in) throw runtime_error("Cannot open " + inputPath);
        string command;
        while (in >> command) {
            if (command == "I") { int h; long long x; in >> h >> x; heaps_[index(h)].insert(x); }
            else if (command == "F") { int h; in >> h; output_.line("Find Min returned: " + to_string(heaps_[index(h)].minimum())); }
            else if (command == "E") { int h; in >> h; output_.line("Extract Min returned: " + to_string(heaps_[index(h)].extractMin())); }
            else if (command == "D") { int h; long long x,y; in >> h >> x >> y; heaps_[index(h)].decreaseKey(x,y); }
            else if (command == "R") { int h; long long x; in >> h >> x; heaps_[index(h)].removeKey(x); }
            else if (command == "P") { int h; in >> h; heaps_[index(h)].print("H" + to_string(h), output_); }
            else if (command == "V") {
                int h; in >> h; HeapVisualizer::saveHeap(heaps_[index(h)], "H" + to_string(h), "heap_H" + to_string(h) + ".svg");
            } else if (command == "U") {
                int a,b; in >> a >> b; int ia=index(a), ib=index(b);
                string beforeA=HeapVisualizer::svg(heaps_[ia], "H"+to_string(a));
                string beforeB=HeapVisualizer::svg(heaps_[ib], "H"+to_string(b));
                vector<pair<BinomialHeap::LinkEvent,string>> stages;
                function<void(const BinomialHeap::LinkEvent&)> capture = [&](const BinomialHeap::LinkEvent& e) {
                    stages.push_back({e, HeapVisualizer::svg(heaps_[ia], "Current H"+to_string(a))});
                };
                heaps_[ia].meld(heaps_[ib], &capture);
                string file="union_trace_"+to_string(++unionCount_)+".html";
                HeapVisualizer::saveUnionTrace(file,"H"+to_string(a),"H"+to_string(b),beforeA,beforeB,stages,HeapVisualizer::svg(heaps_[ia],"Final H"+to_string(a)));
            } else throw runtime_error("Unknown command: " + command);
        }
    }
};

int main() {
    try { HeapApplication app; app.run("input.txt"); }
    catch (const exception& e) { cerr << "Error: " << e.what() << '\n'; return 1; }
    return 0;
}
