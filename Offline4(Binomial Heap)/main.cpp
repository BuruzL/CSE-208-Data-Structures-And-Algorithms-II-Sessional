#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <algorithm>
#include <string>
using namespace std;

class Node
{
public:
    int key;
    int degree;
    Node *parent;
    vector<Node *> children;

    Node(int value)
    {
        key = value;
        degree = 0;
        parent = NULL;
    }
};

class BinomialHeap
{
private:
    vector<Node *> roots;
    int heapSize;

    Node *linkTrees(Node *a, Node *b)
    {
        if (a->key < b->key)
        {
            b->parent = a;
            a->children.insert(a->children.begin(), b);
            a->degree++;
            return a;
        }
        a->parent = b;
        b->children.insert(b->children.begin(), a);
        b->degree++;
        return b;
    }
    Node *findInTree(Node *node, int value)
    {
        if (node->key == value)
            return node;

        for (int i = 0; i < (int)node->children.size(); i++)
        {
            Node *answer = findInTree(node->children[i], value);
            if (answer != NULL)
                return answer;
        }
        return NULL;
    }

    void showTree(Node *node, string prefix, bool last, ostream &out)
    {
        out << prefix;
        if (!prefix.empty())
            out << (last ? "`-- " : "|-- ");
        out << node->key << "(degree " << node->degree << ")\n";

        string nextPrefix = prefix;
        if (!prefix.empty())
            nextPrefix += (last ? "    " : "|   ");
        else
            nextPrefix = "    ";

        for (int i = 0; i < (int)node->children.size(); i++)
        {
            bool isLast = (i == (int)node->children.size() - 1);
            showTree(node->children[i], nextPrefix, isLast, out);
        }
    }

public:
    BinomialHeap()
    {
        heapSize = 0;
    }
    int size()
    {
        return heapSize;
    }
    Node *findNode(int value)
    {
        for (int i = 0; i < (int)roots.size(); i++)
        {
            Node *answer = findInTree(roots[i], value);
            if (answer != NULL)
                return answer;
        }
        return NULL;
    }
    // unionWIth er code here
    void unionWith(BinomialHeap &other, ostream *visualOutput = NULL)
    {
        vector<Node*> merged;
        int i = 0, j = 0;
        while(i<(int)roots.size() && j<(int)other.roots.size()){
            if (roots[i]->degree <= other.roots[j]->degree)
                merged.push_back(roots[i++]);
                
            else
                merged.push_back(other.roots[j++]);
        }
    while (i < (int)roots.size())
            merged.push_back(roots[i++]);
    while (j < (int)other.roots.size())
            merged.push_back(other.roots[j++]);

         roots = merged;
        heapSize += other.heapSize;
        other.roots.clear();
        other.heapSize = 0;

        i=0;
        while(i+1<(int)roots.size()){
            bool differentDegrees=roots[i]->degree!=roots[i+1]->degree;
            bool threeSameDegrees=(i+2<(int)roots.size()
        && roots[i]->degree==roots[i+2]->degree);
        if(differentDegrees||threeSameDegrees){
            i++;
        }else{
            Node* first=roots[i];
            Node* second=roots[i+1];
            if(visualOutput!=NULL){
                *visualOutput<<"Link B"<<
                first->degree<<"+B"<<
                second->degree<<"-> B"
                <<first->degree+1<<"(roots"<<
                first->key<<"and"<<second->key<<")"<<endl;
            }
            Node* combined=linkTrees(first, second);
            roots[i]=combined;
            roots.erase(roots.begin()+i+1);
        }
        }

    }
    void insertKey(int value)
    {
        BinomialHeap oneNodeHeap;
        oneNodeHeap.roots.push_back(new Node(value));
        oneNodeHeap.heapSize = 1;
        unionWith(oneNodeHeap);
    }
    int findMin()
    {
        int answer = roots[0]->key;
        for (int i = 1; i < (int)roots.size(); i++)
        {
            if (roots[i]->key < answer)
                answer = roots[i]->key;
        }
        return answer;
    }
    int extractMin()
    {
        int minIndex = 0;
        for (int i = 1; i < (int)roots.size(); i++)
        {
            if (roots[i]->key < roots[minIndex]->key)
                minIndex = i;
        }
        Node *minimum = roots[minIndex];
        int answer = minimum->key;
        roots.erase(roots.begin() + minIndex);
        BinomialHeap childHeap;
        for (int i = (int)minimum->children.size() - 1; i >= 0; i--)
        {
            minimum->children[i]->parent = NULL;
            childHeap.roots.push_back(minimum->children[i]);
        }
        childHeap.heapSize = (1 << minimum->degree) - 1;
        heapSize = heapSize - childHeap.heapSize - 1;
        minimum->children.clear();
        delete minimum;
        unionWith(childHeap);
        return answer;
    }
    void decreaseKey(int oldValue, int newValue)
    {
        Node *current = findNode(oldValue);
        current->key = newValue;
        while (current->parent != NULL &&
               current->key < current->parent->key)
        {
            int temp = current->key;
            current->key = current->parent->key;
            current->parent->key = temp;
            current = current->parent;
        }
    }
    void removeKey(int value)
    {
        decreaseKey(value, -1000000001);
        extractMin();
    }
    void printHeap(int heapNumber, ostream &out)
    {
        out << "Printing Binomial Heap H" << heapNumber << "\n";
        out << "Heap size: " << heapSize << "\n";
        if (heapSize == 0)
        {
            out << "Heap H" << heapNumber << " is empty.\n";
            return;
        }
        for (int r = 0; r < (int)roots.size(); r++)
        {
            Node *root = roots[r];
            out << "Binomial Tree,B" << root->degree << endl;
            queue<Node *> q;
            q.push(root);
            int level = 0;
            while (!q.empty())
            {
                int nodesThisLevel = q.size();
                vector<int> keys;
                for (int i = 0; i < nodesThisLevel; i++)
                {
                    Node *current = q.front();
                    q.pop();
                    keys.push_back(current->key);
                    for (int c = 0; c < (int)current->children.size(); c++)
                        q.push(current->children[c]);
                }
                sort(keys.begin(), keys.end());
                out << "Level " << level << ":";
                for (int i = 0; i < (int)keys.size(); i++)
                    out << " " << keys[i];
                out << "\n";
                level++;
            }
        }
    }
    void visualize(int heapNumber, ostream &out)
    {
        out << "Visualizing Binomial Heap H" << heapNumber << "\n";
        if (roots.empty())
        {
            out << "Heap is empty.\n";
            return;
        }
        for (int i = 0; i < (int)roots.size(); i++)
        {
            out << "Tree B" << roots[i]->degree
                << ",root = " << roots[i]->key << "\n";
            showTree(roots[i], "", true, out);
        }
    }
};

void writeLine(const string &text, ofstream &output)
{
    cout << text << endl;
    output << text << endl;
}
int main()
{
    ifstream input("input.txt");
    ofstream output("output.txt");
    if (!input.is_open())
    {
        return 0;
    }
    BinomialHeap heaps[3];
    char command;
    while (input >> command)
    {
        int h, h1, h2, x, y;

        if (command == 'I')
        {
            input >> h >> x;
            heaps[h].insertKey(x);
        }
        else if (command == 'F')
        {
            input >> h;
            int answer = heaps[h].findMin();
            writeLine("Find Min returned: " + to_string(answer), output);
        }
        else if (command == 'E')
        {
            input >> h;
            int answer = heaps[h].extractMin();
            writeLine("Extract Min returned: " + to_string(answer), output);
        }
        else if (command == 'D')
        {
            input >> h >> x >> y;
            heaps[h].decreaseKey(x, y);
        }
        else if (command == 'R')
        {
            input >> h >> x;
            heaps[h].removeKey(x);
        }
        else if (command == 'U')
        {
            input >> h1 >> h2;
            heaps[h1].unionWith(heaps[h2]);
        }
        else if (command == 'P')
        {
            input >> h;
            heaps[h].printHeap(h, cout);
            heaps[h].printHeap(h, output);
        }
        else if (command == 'V')
        {
            input >> h;
            heaps[h].visualize(h, cout);
        }
        else if (command == 'W')
        {
            input >> h1 >> h2;
            cout << "Before Union:\n";
            heaps[h1].visualize(h1, cout);
            heaps[h2].visualize(h2, cout);
            cout << "Links made during Union:\n";
            heaps[h1].unionWith(heaps[h2], &cout);
            cout << "After Union:\n";
            heaps[h1].visualize(h1, cout);
        }
    }

    input.close();
    output.close();
    return 0;
}
