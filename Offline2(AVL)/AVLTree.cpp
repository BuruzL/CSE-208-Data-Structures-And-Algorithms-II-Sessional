#include <algorithm>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

class AVLTree {
private:
    struct Node {
        int key;
        int height;
        Node* left;
        Node* right;

        explicit Node(int k) : key(k), height(1), left(nullptr), right(nullptr) {}
    };

    Node* root = nullptr;

    static int heightOf(Node* node) {
        return node ? node->height : 0;
    }

    static int balanceOf(Node* node) {
        return node ? heightOf(node->left) - heightOf(node->right) : 0;
    }

    static void updateHeight(Node* node) {
        if (node) {
            node->height = 1 + max(heightOf(node->left), heightOf(node->right));
        }
    }

    static Node* rotateRight(Node* y) {
        Node* x = y->left;
        Node* t2 = x->right;

        x->right = y;
        y->left = t2;

        updateHeight(y);
        updateHeight(x);
        return x;
    }

    static Node* rotateLeft(Node* x) {
        Node* y = x->right;
        Node* t2 = y->left;

        y->left = x;
        x->right = t2;

        updateHeight(x);
        updateHeight(y);
        return y;
    }

    static Node* rebalance(Node* node) {
        if (!node) return nullptr;

        updateHeight(node);
        int bf = balanceOf(node);

        // Left-heavy: LL (child BF >= 0) or LR (child BF < 0)
        if (bf > 1) {
            if (balanceOf(node->left) < 0) {
                node->left = rotateLeft(node->left);
            }
            return rotateRight(node);
        }

        // Right-heavy: RR (child BF <= 0) or RL (child BF > 0)
        if (bf < -1) {
            if (balanceOf(node->right) > 0) {
                node->right = rotateRight(node->right);
            }
            return rotateLeft(node);
        }

        return node;
    }

    static Node* insertNode(Node* node, int key, bool& inserted) {
        if (!node) {
            inserted = true;
            return new Node(key);
        }

        if (key < node->key) {
            node->left = insertNode(node->left, key, inserted);
        } else if (key > node->key) {
            node->right = insertNode(node->right, key, inserted);
        } else {
            inserted = false;
            return node;
        }

        return rebalance(node);
    }

    static Node* minNode(Node* node) {
        Node* current = node;
        while (current && current->left) current = current->left;
        return current;
    }

    static Node* eraseNode(Node* node, int key, bool& erased) {
        if (!node) return nullptr;

        if (key < node->key) {
            node->left = eraseNode(node->left, key, erased);
        } else if (key > node->key) {
            node->right = eraseNode(node->right, key, erased);
        } else {
            erased = true;

            if (!node->left || !node->right) {
                Node* child = node->left ? node->left : node->right;
                delete node;
                return child;
            }

            // Two children: replace with in-order successor, then physically remove it.
            Node* successor = minNode(node->right);
            node->key = successor->key;
            bool dummy = false;
            node->right = eraseNode(node->right, successor->key, dummy);
        }

        return rebalance(node);
    }

    static bool findNode(Node* node, int key) {
        while (node) {
            if (key < node->key) {
                node = node->left;
            } else if (key > node->key) {
                node = node->right;
            } else {
                return true;
            }
        }
        return false;
    }

    static void inorder(Node* node, vector<int>& out) {
        if (!node) return;
        inorder(node->left, out);
        out.push_back(node->key);
        inorder(node->right, out);
    }

    static string serializeNode(Node* node) {
        if (!node) return "";

        if (!node->left && !node->right) {
            return to_string(node->key);
        }

        return to_string(node->key) + "(" + serializeNode(node->left) + "," +
               serializeNode(node->right) + ")";
    }

    static void destroy(Node* node) {
        if (!node) return;
        destroy(node->left);
        destroy(node->right);
        delete node;
    }

public:
    AVLTree() = default;
    AVLTree(const AVLTree&) = delete;
    AVLTree& operator=(const AVLTree&) = delete;

    ~AVLTree() {
        destroy(root);
    }

    bool insert(int key) {
        bool inserted = false;
        root = insertNode(root, key, inserted);
        return inserted;
    }

    bool erase(int key) {
        bool erased = false;
        root = eraseNode(root, key, erased);
        return erased;
    }

    bool find(int key) const {
        return findNode(root, key);
    }

    vector<int> traverse() const {
        vector<int> result;
        inorder(root, result);
        return result;
    }

    string serialize() const {
        return serializeNode(root);
    }
};

struct TimingStat {
    uint64_t count = 0;
    uint64_t totalNs = 0;

    void add(uint64_t ns) {
        ++count;
        totalNs += ns;
    }
};

static void printTimingRow(const string& name, const TimingStat& stat) {
    cout << name << ',' << stat.count << ',' << stat.totalNs << ',';
    if (stat.count == 0) {
        cout << "N/A\n";
    } else {
        cout << (stat.totalNs / stat.count) << '\n';
    }
}

static void writeVectorLine(ofstream& out, const vector<int>& values) {
    for (size_t i = 0; i < values.size(); ++i) {
        if (i) out << ' ';
        out << values[i];
    }
    out << '\n';
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <input-file> <output-file>\n";
        return 1;
    }

    ifstream input(argv[1]);
    if (!input) {
        cerr << "Error: cannot open input file.\n";
        return 1;
    }

    ofstream output(argv[2]);
    if (!output) {
        cerr << "Error: cannot open output file.\n";
        return 1;
    }

    AVLTree tree;
    TimingStat insertStat, deleteStat, findStat, traverseStat;

    string line;
    while (getline(input, line)) {
        if (line.empty()) continue;

        istringstream iss(line);
        char command;
        iss >> command;

        if (command == 'I') {
            int x;
            iss >> x;

            const auto start = chrono::steady_clock::now();
            bool inserted = tree.insert(x);
            const auto stop = chrono::steady_clock::now();
            insertStat.add(chrono::duration_cast<chrono::nanoseconds>(stop - start).count());

            if (inserted) {
                output << tree.serialize() << '\n';
            } else {
                output << "duplicate\n";
            }
        } else if (command == 'D') {
            int x;
            iss >> x;

            const auto start = chrono::steady_clock::now();
            bool erased = tree.erase(x);
            const auto stop = chrono::steady_clock::now();
            deleteStat.add(chrono::duration_cast<chrono::nanoseconds>(stop - start).count());

            if (erased) {
                output << tree.serialize() << '\n';
            } else {
                output << "not found\n";
            }
        } else if (command == 'F') {
            int x;
            iss >> x;

            const auto start = chrono::steady_clock::now();
            bool found = tree.find(x);
            const auto stop = chrono::steady_clock::now();
            findStat.add(chrono::duration_cast<chrono::nanoseconds>(stop - start).count());

            output << (found ? "found" : "not found") << '\n';
        } else if (command == 'T') {
            const auto start = chrono::steady_clock::now();
            vector<int> values = tree.traverse();
            const auto stop = chrono::steady_clock::now();
            traverseStat.add(chrono::duration_cast<chrono::nanoseconds>(stop - start).count());

            writeVectorLine(output, values);
        }
    }

    // Timing summary goes to standard output, not the operation-output file.
    cout << "operation,count,total_ns,average_ns\n";
    printTimingRow("insert", insertStat);
    printTimingRow("delete", deleteStat);
    printTimingRow("find", findStat);
    printTimingRow("traverse", traverseStat);

    return 0;
}
