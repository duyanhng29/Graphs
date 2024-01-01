#include "Graph.h"
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <queue>
#include <iostream>
#include <set>
#include <tuple>
#include <ostream>
#include <stack>



Graph::Graph(const char* const & edgelist_csv_fn) {
    this->numNodes = 0;
    this->numEdges = 0;

    ifstream my_file(edgelist_csv_fn);
    string line;
    while(getline(my_file, line)){
        istringstream ss(line);
        string label1, label2, distance;
        double dis = 0;

        getline(ss, label1, ',');
        getline(ss, label2, ',');
        getline(ss, distance, '\n');

        dis = stod(distance);

        unordered_map<string, double> m1, m2;
        
        if(this->map.find(label1) == this->map.end() && 
            this->map.find(label2) == this->map.end()){
            m1[label2] = dis;
            m2[label1] = dis;

            GNode* node1 = new GNode(label1, m1);
            GNode* node2 = new GNode(label2, m2);

            this->map[label1] = node1;
            this->map[label2] = node2;

            this->numEdges+=1;
        }
        else if(this->map.find(label1) != this->map.end() &&
                this->map.find(label2) == this->map.end()){
            m2[label1] = dis;

            GNode* node2 = new GNode(label2, m2);

            this->map[label2] = node2;
            this->map[label1]->neighbors[label2] = dis;

            this->numEdges+=1;
        }
        else if(this->map.find(label1) == this->map.end() &&
                this->map.find(label2) != this->map.end()){
            m1[label2] = dis;

            GNode* node1 = new GNode(label1, m1);

            this->map[label1] = node1;

            this->map[label2]->neighbors[label1] = dis;

            this->numEdges+=1;
        }
        else{
            this->map[label1]->neighbors[label2] = dis;
            this->map[label2]->neighbors[label1] = dis;
            this->numEdges +=1;
        }

    }
    my_file.close();
}

Graph::~Graph(){
    for(auto pair: this->map){
        delete(this->map[pair.first]);
    }
    numEdges = 0;
    this->map.clear();
}
unsigned int Graph::num_nodes() {
    return this->map.size();
}

vector<string> Graph::nodes() {
    vector<string> result;
    for(auto pair : this->map){
        result.push_back(pair.first);
    }
    return result;
}

unsigned int Graph::num_edges() {
    return this->numEdges;
}

unsigned int Graph::num_neighbors(string const & node_label) {
    return this->map[node_label]->neighbors.size();
}

double Graph::edge_weight(string const & u_label, string const & v_label) {
    if(this->map[u_label]->neighbors.find(v_label) == this->map[u_label]->neighbors.end()){
        return -1;
    }
    return this->map[u_label]->neighbors[v_label];
}

vector<string> Graph::neighbors(string const & node_label) {
    vector<string> result;
    for(auto pair: this->map[node_label]->neighbors){
        result.push_back(pair.first);
    }
    return result;
}

Graph::GNode* Graph::BFS_Helper(string const & start, string const & end){
    queue<Graph::GNode*> q;
    q.push(this->map[start]);
    GNode* finish = nullptr;
    while(!q.empty()){
        Graph::GNode* n = q.front();
        n->visisted = true;
        q.pop();
        if(n->label == end){
            finish = n;
            return finish;
        }
        for(auto pair: n->neighbors){

            if(this->map[pair.first]->visisted == false){
                this->map[pair.first]->visisted = true;
                this->map[pair.first]->prev = n;

                q.push(this->map[pair.first]);
            }
        }
    }
    return finish;
}

vector<string> Graph::shortest_path_unweighted(string const & start_label, string const & end_label) {
    vector<string>back;
    vector<string>result;

    if(start_label == end_label){
        result.push_back(end_label);
        return result;
    }
    GNode* finish = Graph::BFS_Helper(start_label, end_label);
    back.push_back(end_label);
    if(finish == nullptr){
        for(auto pair : this->map){
            pair.second->visisted = false;
            pair.second->prev = nullptr;
        }
        return result;
    }
   
    GNode* curr = finish->prev;

    while(curr != nullptr){
        back.push_back(curr->label);
        curr = curr->prev;
    }

    for(int i = back.size() - 1; i >=0; i--){
        result.push_back(back[i]);
    }

    for(auto pair : this->map){
        pair.second->visisted = false;
        pair.second->prev = nullptr;
    }
    return result;
}

vector<tuple<string,string,double>> Graph::shortest_path_weighted(string const & start_label, string const & end_label) {
   priority_queue<GNode*, vector<GNode*>, minH> pq;
    vector<tuple<string, string, double>> result;

    
   GNode* finish = nullptr;
    GNode* startNode = this->map[start_label];
    startNode->currDist = 0;
   pq.push(startNode);
   while(pq.empty() == false){
       GNode* current = pq.top();
       pq.pop();
       if(current->label == end_label){
           finish = current;
           break;
       }
       if(current->visisted == false){
           current->visisted = true;
           for(auto pair : current->neighbors){
               double d = current->currDist + pair.second;
               if(d < this->map[pair.first]->currDist){
                   this->map[pair.first]->prev = current;
                   this->map[pair.first]->currDist = d;
                    pq.push(this->map[pair.first]);
               }
           }
       }

   }
   if(start_label == end_label){
        tuple<string, string, double> t1 = make_tuple(start_label,end_label,-1);
        result.push_back(t1);
        return result;
    }
    stack<tuple<string, string, double>> s;
    if(finish == nullptr){
        for(auto pair : this->map){
            pair.second->visisted = false;
            pair.second->prev = nullptr;
            pair.second->currDist = numeric_limits<double>::max();
        }
        return result;
    }
    GNode* curr = finish;
    while(curr != nullptr && curr->label != start_label){
        tuple<string, string, double> t = make_tuple(curr->prev->label, curr->label, this->map[curr->prev->label]->neighbors[curr->label]);
        s.push(t);
        curr = curr->prev;

    }
    while(!s.empty()){
        result.push_back(s.top());
        s.pop();
    }
    for(auto pair : this->map){
        pair.second->visisted = false;
        pair.second->prev = nullptr;
        pair.second->currDist = numeric_limits<double>::max();

    }
    return result;
}

vector<vector<string>> Graph::connected_components(double const & threshold) {
    queue<Graph::GNode*> q;
    vector<string> sub;
    vector<vector<string>> result;

    for(auto pair : this->map){
        if(this->map[pair.first]->visisted == false){
            sub.push_back(pair.first);
            q.push(this->map[pair.first]);
            while(!q.empty()){
                Graph::GNode* n = q.front();
                n->visisted = true;
                q.pop();       
                for(auto pair: n->neighbors){
                    if(this->map[pair.first]->visisted == false && 
                        pair.second <= threshold){
                        this->map[pair.first]->visisted = true;
                        this->map[pair.first]->prev = n;
                        sub.push_back(pair.first);
                        q.push(this->map[pair.first]);
                    }
                }
            }
            result.push_back(sub);
            sub.clear();
        }       
    }
    return result;        
}

double Graph::smallest_connecting_threshold(string const & start_label, string const & end_label) {
    return 0;
}
