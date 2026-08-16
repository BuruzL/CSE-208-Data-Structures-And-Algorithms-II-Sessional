#include<iostream>
#include<algorithm>
#include<string>
#include<vector>
//time header
#include<chrono>
//file header
#include<fstream>
#include<sstream>
#include<iostream>
using namespace std;

class AVLTree{
    private:
    struct Node{
        int key;
        int height;
        Node* left;
        Node* right;

        Node(int k): key(k), height(1), left(nullptr), right(nullptr){} 
    };

    static Node* copyNode(Node* node) {
    if (node == nullptr) {
        return nullptr;
    }

    Node* newNode = new Node(node->key);

    newNode->height = node->height;
    newNode->left = copyNode(node->left);
    newNode->right = copyNode(node->right);

    return newNode;
}

    Node* root=nullptr;
    static int height(Node* node){
        return node?node->height:0;
    }
    static int BalanceFactor(Node* node){
        return node?height(node->left)-height(node->right):0;
    }
    static void updateHeight(Node* node){
        if(node){
            node->height=1+max(height(node->left), height(node->right));
        }
    }

    //AVL Rotations
    static Node* rotateRight(Node* y){
        Node* x=y->left;
        Node* t2=x->right;

        x->right=y;
        y->left=t2;

        updateHeight(y);
        updateHeight(x);
        return x;
    }

    static Node* rotateLeft(Node* x){
        Node* y=x->right;
        Node* t2=y->left;

        y->left=x;
        x->right=t2;
        updateHeight(x);
        updateHeight(y);
        return y;
    }
    
    //AVL balance
    static Node* rebalance(Node* node){
        if(!node)return nullptr;

        updateHeight(node);
        int bf=BalanceFactor(node);

        if(bf>1){
            if(BalanceFactor(node->left)<0){
                node->left=rotateLeft(node->left);
            }
            return rotateRight(node);
        }

        if(bf<-1){
            if(BalanceFactor(node->right)>0){
                node->right=rotateRight(node->right);
            }
            return rotateLeft(node);
        }
        return node;
    }

    //basic bst insert with rebalance
    static Node* insertNode(Node* node, int key, bool& inserted){
        if(!node){
            inserted=true;
            return new Node(key);
        }
        if(key<node->key){
            node->left=insertNode(node->left, key, inserted);
        }else if(key>node->key){
            node->right=insertNode(node->right, key, inserted);
        }else{
            inserted=false;
            return node;
        }
        return rebalance(node);
    }
    static Node* minNode(Node* node){
        Node* current=node;
        while(current && current->left){
            current=current->left;
        }
        return current;
    }

    //basic bst delete with rebalance
    static Node* deleteNode(Node* node, int key, bool& deleted){
        if(!node)return nullptr;
        if(key<node->key){
            node->left=deleteNode(node->left, key, deleted);
        }else if(key>node->key){
            node->right=deleteNode(node->right, key, deleted);
        }else{
            deleted=true;
            if(!node->left || !node->right){
                Node* child=node->left?node->left:node->right;
                delete node;
                return child;
            }
            Node*succ=minNode(node->right);
            node->key=succ->key;
            bool temp=false;
            node->right=deleteNode(node->right, succ->key, temp);
        }
        return  rebalance(node);
    }

    //basic bst find
    static bool findNode(Node* node, int key){
        while(node){
            if(key<node->key){
                node=node->left;
            }else if(key>node->key){
                node=node->right;
            }else{
                return true;
            }
        }
        return false;
    }
    //basic bst inorder
    static void inorder(Node* node, vector<int> &out){
        if(!node)return;
        inorder(node->left, out);
        out.push_back(node->key);
        inorder(node->right, out);
    }
    
    static string toString(Node* node){
        if(!node)return "";
        string result=to_string(node->key);
        if(node->left==nullptr && node->right==nullptr){
            return result;
        }
        result+="(";
        result+=toString(node->left);
        result+=",";
        result+=toString(node->right);
        result+=")";

        return result;
    }
    //destroys the entire subtree
    static void destroy(Node* node){
        if(!node)return;
        destroy(node->left);
        destroy(node->right);
        delete node;
    }
    public:
    AVLTree(){
        //default
    }
    AVLTree(const AVLTree& other) {
    root = copyNode(other.root);
}
AVLTree& operator=(const AVLTree& other) {
    if (this != &other) {
        destroy(root);
        root = copyNode(other.root);
    }
    return *this;
}

~AVLTree(){
    destroy(root);
}
bool insert(int key){
    bool inserted=false;
    root=insertNode(root, key, inserted);
    return inserted;
}
bool erase(int key){
    bool erased=false;
    root=deleteNode(root, key, erased);
    return erased;
}
bool find(int key)const{
   return findNode(root, key);
}

vector<int> traverse() const{
 vector<int> result;
    inorder(root, result);
    return result;
}
string output() const{
    return toString(root);
}
};




//Time er habijabi
long long getTime(){
    return chrono::duration_cast<chrono::nanoseconds>(
        chrono::steady_clock::now().time_since_epoch()
    ).count();
}

struct TimeInfo{
    long long count=0;
    long long totalTime=0;
    void add(long long time){
        count++;
        totalTime+=time;
    }
};
void printTime(string operation, TimeInfo info){
    cout<<operation<<",";
    cout<<info.count<<",";
    cout<<info.totalTime<<",";
    if(info.count==0){
        cout<<"N/A"<<endl;
    }else{
        cout<<info.totalTime/info.count<<endl;
    }
}
void printVector(ofstream& output, vector<int>& values){
    for(size_t i=0; i<values.size(); i++){
        if(i>0){
            output<<" ";
        }
        output<<values[i];
    }
    output<<endl;
}

int main(int argc, char* argv[]){
if(argc!=3){
    return 1;
}

ifstream input(argv[1]);

if(!input){
    return 1;
}

ofstream output(argv[2]);

if(!output){
    return 1;
}

AVLTree gach;

TimeInfo insertTime;
TimeInfo deleteTime;
TimeInfo findTime;
TimeInfo traverseTime;

string line;

while(getline(input, line)){
    if(line.empty()){
        continue;
    }
    istringstream inputss(line);
    char command;
    inputss>>command;

    if(command=='I'){
        int x;
        inputss>>x;
        long long start=getTime();
        bool inserted=gach.insert(x);
        long long finish=getTime();
        insertTime.add(finish-start);
        if(inserted){
            output<<gach.output()<<endl;
        }else{
            output<<"duplicate"<<endl;
        }
    }

    else if(command=='D'){
        int x;
        inputss>>x;
        long long start=getTime();
        bool deleted=gach.erase(x);
        long long finish=getTime();
        deleteTime.add(finish-start);
        if(deleted){
            output<<gach.output()<<endl;
        }else{
            output<<"not found"<<endl;
        }
    }

    else if(command=='F'){
        int x;
        inputss>>x;
        long long start=getTime();
        bool found=gach.find(x);
        long long finish=getTime();
        findTime.add(finish-start);
        if(found){
            output<<"found"<<endl;
        }else{
            output<<"not found"<<endl;
        }
    }

    else if(command=='T'){
        long long start=getTime();
        vector<int> values=gach.traverse();
        long long finish=getTime();
        traverseTime.add(finish-start);
        printVector(output,values);
    }
}

cout<<"operation,count,total_ns,average_ns"<<endl;

printTime("insert", insertTime);
printTime("delete", deleteTime);
printTime("find",findTime);
printTime("traverse",traverseTime);

return 0;
}
