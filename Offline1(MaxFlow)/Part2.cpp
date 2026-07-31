#include<iostream>
#include<vector>
#include<queue>
#include<sstream>
using namespace std;

struct Flights{
    string fname;
    string from;
    string to;
    int dep,arr;
};

int time(string t){
    stringstream ss(t);
    int hour, minute;
    char colon;
    ss>>hour>>colon>>minute;
    return hour*60+minute;
}

bool bfs(int source, int sink,  vector<int> &parent, vector<vector<int>> &capacity){
    int n=capacity.size();
    vector<int> visited(n,0);

    queue<int> q;
    q.push(source);
    visited[source]=1;
    parent[source]=-1;

    while(!q.empty()){
        int u=q.front();
        q.pop();

        for(int v=0; v<n; v++){
            if(!visited[v] && capacity[u][v]>0){
                visited[v]=1;
                parent[v]=u;
                if(v==sink)return true;
                q.push(v);
            }
        }
    }
    return false;
}

int EdmondsKarp(int source, int sink, vector<vector<int>> &capacity){
    int n=capacity.size();
    int maxFlow=0;
    while(true){
        vector<int> parent(n,-1);
        
        if(!bfs(source, sink, parent, capacity))break;
        int pathFlow=INT_MAX;
        for(int v=sink; v!=source; v=parent[v]){
            int u=parent[v];
            pathFlow=min(pathFlow, capacity[u][v]);
        }
        for(int v=sink; v!=source; v=parent[v]){
            int u=parent[v];
            capacity[u][v]-=pathFlow;
            capacity[v][u]+=pathFlow;
        }
        maxFlow+=pathFlow;

    }
    return maxFlow;

}

void routes(vector<Flights> &flights,  vector<vector<int>> &capOriginal){
    int n=flights.size();
     int source=2*n;
    int sink=2*n+1;
    vector<vector<int>> capacity=capOriginal;

    int matching=EdmondsKarp(source, sink ,capacity);

    vector<int> next(n,-1);
    vector<int> previous(n,-1);
    for(int i=0; i<n; i++){
       for(int j=0; j<n; j++){
        if(i==j)continue;
        if(capOriginal[i][n+j]==1 && capacity[i][n+j]==0){
            next[i]=j;
            previous[j]=i;
            break;
        }
       }
    }

   cout<<"Number of Aircraft: "<<n-matching<<endl;

   int ctr=1;
   for(int i=0; i<n; i++){
    if(previous[i]==-1){
        cout<<"Aircraft "<<ctr++<<": ";
        int current=i;
        while(current!=-1){
            cout<<flights[current].fname;
            if(next[current]!=-1){
                cout<<"->";
            }
            current=next[current];
        }
        cout<<endl;
    }
   }
}

int main(){
    int f;
    cin>>f;
    vector<Flights> flights(f);

    for(int i=0; i<f; i++){
        string src,dst;
        cin>>flights[i].fname>>flights[i].from>>flights[i].to>>src>>dst;
        int a=time(src);
        int b=time(dst);
        flights[i].dep=a;
        flights[i].arr=b;
    }

    int n=flights.size();
    int source=2*n;
    int sink=2*n+1;
    int totalNodes=2*n+2;
     vector<vector<int>> capacity(totalNodes, vector<int>(totalNodes,0));
    for(int i=0; i<n; i++){
        capacity[source][i]=1;
        capacity[n+i][sink]=1;
    }

    for(int i=0; i<n; i++){
        for(int j=0; j<n; j++){
            if(i==j)continue;
            if(flights[i].to==flights[j].from && flights[i].arr+180<=flights[j].dep){
                capacity[i][n+j]=1;
            }
        }
    }
   
    routes(flights, capacity);
    return 0;
}
