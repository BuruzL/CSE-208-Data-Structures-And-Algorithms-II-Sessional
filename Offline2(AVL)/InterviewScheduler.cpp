#include<algorithm>
#include<chrono>
#include<iostream>
#include<string>
#include<cstdint>
#include<limits>
#include<sstream>
#include<unordered_map>
#include<utility>
#include<vector>
#include<fstream>
using namespace std;

class IntervalScheduler{
    private:
    struct Node{
        int id;
        int start;
        int end;
        int height;
        int maxEnd;
        Node* left;
        Node* right;

        Node(int idValue, int s, int e):
        id(idValue), start(s), end(e), height(1), maxEnd(e),
        left(nullptr), right(nullptr){}
    };
    Node* root=nullptr;
    int nextId=1;

    unordered_map<int, pair<int, int>> byId;

    static int heightOf(Node* node){
        return node?node->height:0;
    }

    static int maxEndOf(Node* node){
        return node?node->maxEnd: numeric_limits<int>::min();
    }

    static int balanceOf(Node* node){
        return node?heightOf(node->left)-heightOf(node->right):0;
    }

    static bool keyLess(int s1, int id1, int s2, int id2){
        if(s1!=s2)return s1<s2;
        return id1<id2;
    }

    static void updateMetadata(Node* node){
        if(!node)return;
        node->height=1+max(heightOf(node->left), heightOf(node->right));
        node->maxEnd=max(node->end, max(maxEndOf(node->left), maxEndOf(node->right)));
    }

    static Node* rotateRight(Node* y){
        Node* x=y->left;
        Node* t2=x->right;

        x->right=y;
        y->left=t2;

        updateMetadata(y);
        updateMetadata(x);
        return x;
    }

    static Node* rotateLeft(Node* x){
        Node* y=x->right;
        Node* t2=y->left;

        y->left=x;
        x->right=t2;

        updateMetadata(x);
        updateMetadata(y);
        return y;
    }

    static Node* rebalance(Node* node){
        if(!node)return nullptr;

        updateMetadata(node);
        int BF=balanceOf(node);

        if(BF>1){
            if(balanceOf(node->left)<0){
                node->left=rotateLeft(node->left);
            }
            return rotateRight(node);
        }
        if(BF<-1){
            if(balanceOf(node->right)>0){
                node->right=rotateRight(node->right);
            }
            return rotateLeft(node);
        }
        return node;
    }
    static Node* insertNode(Node* node, int id, int start, int end){
        if(!node)return new Node(id, start, end);

        if(keyLess(start, id, node->start, node->id)){
            node->left=insertNode(node->left, id, start, end);
        }else{
            node->right=insertNode(node->right, id, start, end);
        }
        return rebalance(node);
    }
    static Node* minNode(Node* node){
        Node* current=node;
        while(current&&current->left)current=current->left;
        return current;
    }
    static Node* eraseNode(Node* node, int start, int id, bool& erased){
        if(!node) return nullptr;

        if(keyLess(start, id, node->start, node->id)){
            node->left=eraseNode(node->left, start, id, erased);
        }else if(keyLess(node->start, node->id, start, id)){
            node->right=eraseNode(node->right, start, id, erased);
        }else{
            erased=true;

            if(!node->left || !node->right){
                Node* child=node->left? node->left: node->right;
                delete node;
                return child;
            }

            Node* successor=minNode(node->right);
            node->id=successor->id;
            node->start=successor->start;
            node->end=successor->end;

            bool dummy=false;
            node->right=eraseNode(node->right, successor->start, successor->id,dummy);
        }
        return rebalance(node);
    }
    static bool overlaps(int aStart, int aEnd, int bStart, int bEnd){
        return aStart<bEnd && bStart<aEnd;
    }
    static bool anyConflict(Node* node, int queryStart, int queryEnd){
        Node* current=node;

        while(current){
            if(overlaps(current->start, current->end, queryStart, queryEnd)){
                return true;
            }
            if(current->left && current->left->maxEnd>queryStart){
                current=current->left;
            }else{
                current=current->right;
            }
        }
        return false;
    }
    static void collectOverlaps(Node* node, int queryStart, int queryEnd, vector<int>& ids){
        if(!node)return;
        if(node->left && node->left->maxEnd>queryStart){
            collectOverlaps(node->left, queryStart, queryEnd,ids);
        }
        if(overlaps(node->start, node->end, queryStart, queryEnd)){
            ids.push_back(node->id);
        }
        if(node->right && node->start<queryEnd && node->right->maxEnd>queryStart){
            collectOverlaps(node->right, queryStart, queryEnd, ids);
        }
    }
    static void collectAt(Node* node, int t , vector<int>& ids){
        if(!node)return;
        if(node->left&&node->left->maxEnd>t){
            collectAt(node->left,t,ids);
        }
        if(node->start<=t && t<node->end){
            ids.push_back(node->id);
        }
         if (node->right && node->start <= t && node->right->maxEnd > t) {
            collectAt(node->right, t, ids);
        }
    }
    static Node* nextNode(Node* node, int t){
        Node* current=node;
        Node* best=nullptr;

        while(current){
            if(current->start>=t){
                best=current;
                current=current->left;
            }else{
                current=current->right;
            }
        }
        return best;
    }

    static string toString(Node* node){
        if(!node)return "";
        if(!node->left && !node->right){
            return to_string(node->id);
        }
        return to_string(node->id)+"("+toString(node->left)+","+
        toString(node->right)+")";
    }
    static void destroy(Node* node){
        if(!node)return;
        destroy(node->left);
        destroy(node->right);
        delete node;
    }

    public:
    struct NextResult{
        bool found=false;
        int id=0;
        int start=0;
        int end=0;
    };
//eigula ektu recheck deoya lagbe
    IntervalScheduler() = default;
    IntervalScheduler(const IntervalScheduler&) = delete;
    IntervalScheduler& operator=(const IntervalScheduler&) = delete;
    ~IntervalScheduler() {
        destroy(root);
    }
    int add(int start, int end){
        const int id=nextId++;
        root=insertNode(root, id, start, end);
        byId[id]={start, end};
        return id;
    }

    bool remove(int id){
        auto it=byId.find(id);
        if(it==byId.end())return false;
        const int start=it->second.first;
        bool erased=false;
        root=eraseNode(root, start, id, erased);
        if(erased){
            byId.erase(it);
        }
        return erased;
    }
    bool update(int id, int newStart, int newEnd){
        auto it=byId.find(id);
        if(it==byId.end())return false;
        const int oldStart=it->second.first;
        bool erased=false;
        root=eraseNode(root, oldStart, id, erased);
        if(!erased)return false;
        root=insertNode(root, id, newStart, newEnd);
        it->second={newStart, newEnd};
        return true;
    }
    bool conflict(int start, int end) const{
        return anyConflict(root, start, end);
    }
    vector<int> overlapsQuery(int start, int end)const{
        vector<int> ids;
        collectOverlaps(root, start, end, ids);
        return ids;
    }
     vector<int> at(int t) const {
        vector<int> ids;
        collectAt(root, t, ids);
        return ids;
    }
    NextResult next(int t)const{
        Node* node=nextNode(root, t);
        if(!node)return{};
        return{true, node->id, node->start, node->end};
    }
    string serialize() const {
    return toString(root);
}

}; 
    struct TimingStat{
        //eta valo moto dekha lagbe
        uint64_t count=0;
        uint64_t totalNs=0;

        void add(uint64_t ns){
            ++count;
            totalNs+=ns;
        }

    };
    static void printTimingRow(const string& name, const TimingStat& stat){
        cout<<name<<','<<stat.count<<','<<stat.totalNs<<',';
        if(stat.count==0){
            cout<<"N/A\n";
        }else{
            cout<<(stat.totalNs/stat.count)<<'\n';
        }
    }

    static void writeIdList(ofstream& out, const vector<int>& ids){
        if(ids.empty()){
            out<<"none\n";
            return;
        }
        for(size_t i=0; i<ids.size(); ++i){
            if(i) out<<' ';
            out<<ids[i];
        }
        out<<'\n';
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

        IntervalScheduler scheduler;

        TimingStat addStat,removeStat, updateStat, conflictStat;
        TimingStat overlapsStat, atStat, nextStat;

        string line;
        while(getline(input, line)){
            if(line.empty())continue;
             istringstream iss(line);
            string command;
            iss >> command;

            if(command=="ADD"){
                int s,e;
                iss>>s>>e;

                const auto begin = chrono::steady_clock::now();
            scheduler.add(s, e);
            const auto finish = chrono::steady_clock::now();
            addStat.add(chrono::duration_cast<chrono::nanoseconds>(finish - begin).count());

            output << scheduler.serialize() << '\n';
            }else if(command=="REMOVE"){
                int id;
                iss>>id;

                const auto begin = chrono::steady_clock::now();
            bool removed = scheduler.remove(id);
            const auto finish = chrono::steady_clock::now();
            removeStat.add(chrono::duration_cast<chrono::nanoseconds>(finish - begin).count());

            if(removed){
                output<<scheduler.serialize()<<'\n';
            }else{
                output<<"not found\n";
            }
            }else if(command=="UPDATE"){
                int id,s,e;
                iss>>id>>s>>e;

                const auto begin = chrono::steady_clock::now();
            bool updated = scheduler.update(id, s, e);
            const auto finish = chrono::steady_clock::now();
            updateStat.add(chrono::duration_cast<chrono::nanoseconds>(finish - begin).count());


            if(updated){
                output<<scheduler.serialize()<<'\n';
            }else{
                output<<"not found\n";
            }
            }else if(command=="CONFLICT"){
                int s,e;
                iss>>s>>e;

                 const auto begin = chrono::steady_clock::now();
            bool hasConflict = scheduler.conflict(s, e);
            const auto finish = chrono::steady_clock::now();
            conflictStat.add(chrono::duration_cast<chrono::nanoseconds>(finish - begin).count());
             output << (hasConflict ? "yes" : "no") << '\n';
            }else if(command=="OVERLAPS"){
                 int s, e;
            iss >> s >> e;

            const auto begin = chrono::steady_clock::now();
            vector<int> ids = scheduler.overlapsQuery(s, e);
            const auto finish = chrono::steady_clock::now();
            overlapsStat.add(chrono::duration_cast<chrono::nanoseconds>(finish - begin).count());

            writeIdList(output, ids);
            }else if(command=="AT"){
                 int t;
            iss >> t;

            const auto begin = chrono::steady_clock::now();
            vector<int> ids = scheduler.at(t);
            const auto finish = chrono::steady_clock::now();
            atStat.add(chrono::duration_cast<chrono::nanoseconds>(finish - begin).count());

            writeIdList(output, ids);
            }else if(command=="NEXT"){
                 int t;
            iss >> t;

            const auto begin = chrono::steady_clock::now();
            IntervalScheduler::NextResult result = scheduler.next(t);
            const auto finish = chrono::steady_clock::now();
            nextStat.add(chrono::duration_cast<chrono::nanoseconds>(finish - begin).count());

            if (!result.found) {
                output << "none\n";
            } else {
                output << result.id << ' ' << result.start << ' ' << result.end << '\n';
            }
            }
        }

            cout << "operation,count,total_ns,average_ns\n";
    printTimingRow("add", addStat);
    printTimingRow("remove", removeStat);
    printTimingRow("update", updateStat);
    printTimingRow("conflict", conflictStat);
    printTimingRow("overlaps", overlapsStat);
    printTimingRow("at", atStat);
    printTimingRow("next", nextStat);

    return 0; 

}
