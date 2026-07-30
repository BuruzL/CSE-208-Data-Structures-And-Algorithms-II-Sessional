#include<iostream>
#include<vector>
#include<queue>
#include<climits>

using namespace std;

bool bfs(int source, int sink, vector<vector<int>> &capacity, vector<int> &parent){
    int n=capacity.size();
    vector<int> visited(n,0);
    queue<int> q;
    q.push(source);
    visited[source]=1;
    parent[source]=-1;

    while(!q.empty()){
        int u=q.front();
        q.pop();
        for(int v=1; v<n; v++){
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

int edmonsKarp(int source, int sink, vector<vector<int>>& capacity){
    int maxFlow=0;
    vector<int> parent(capacity.size());

    while(bfs(source,sink, capacity, parent)){
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

int main(){
    int n, m;
    cin>>n>>m;
    int source=0;
    int sink=n-1;
    vector<vector<int>> capacity(n+1, vector<int>(n+1, 0));
    for(int i=0; i<m; i++){
        int u, v, c;
        cin >> u >> v >> c;
        capacity[u][v] += c; 
    }
    cout<<"Maxflow= "<<edmonsKarp(source, sink, capacity)<<endl;
    return 0;
}
