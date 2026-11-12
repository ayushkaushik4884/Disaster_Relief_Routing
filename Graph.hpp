#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <vector>
#include <unordered_map>
#include <iostream>
#include <cmath>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>


struct Node{
    int id;
    double x;
    double y;
    bool is_operational;
    bool is_station;

    Node() : id(-1), x(0.0), y(0.0), is_operational(true), is_station(false) {}
    Node(int id, double x, double y, bool is_station = false)
        : id(id), x(x), y(y), is_operational(true), is_station(is_station) {}
};


struct Edge {
    int edge_id;
    int src;
    int dest;
    double weight;
    bool is_operational;

    Edge(int edge_id, int src, int dest, double weight)
        : edge_id(edge_id), src(src), dest(dest), weight(weight), is_operational(true) {}
};



struct ScheduledClosure{
    int edge_id;
    double start_time;
    double end_time;
};


class Graph{
private:
    std::unordered_map<int, Node> nodes;
    std::unordered_map<int, std::vector<Edge>> adj;
    std::vector<ScheduledClosure> scheduled_closures;

public:
    Graph() = default;

    void addNode(int id, double x, double y, bool is_station = false){
        nodes[id] = Node(id, x, y, is_station);
        if(adj.find(id) == adj.end()){
            adj[id] = std::vector<Edge>();
        }

    }

    void addEdge(int edge_id, int u, int v, double weight, bool bidirectional = true){
        if(nodes.find(u) == nodes.end() || nodes.find(v) == nodes.end()){
            std::cerr << "Error: Source or destination node does not exist.\n";
            return;
        }
        adj[u].push_back(Edge(edge_id, u, v, weight));
        if (bidirectional){
            adj[v].push_back(Edge(edge_id, v, u, weight));
        }
    }

    void addScheduledClosure(int edge_id, double start_time, double end_time){
        scheduled_closures.push_back({edge_id, start_time, end_time});

    }

    bool isEdgeAvailable(const Edge& edge, double currentTime) const {
        if(!edge.is_operational) return false;

        for(const auto& closure : scheduled_closures){
            if(closure.edge_id == edge.edge_id){
                if(closure.start_time <= closure.end_time){
                    if(currentTime >= closure.start_time && currentTime <= closure.end_time){
                        return false;
                    }
                } else {
                    if(currentTime >= closure.start_time || currentTime <= closure.end_time){
                        return false;
                    }
                }
            }
        }
        return true;
    }

    void setNodeOperational(int id, bool status){
        if(nodes.find(id) != nodes.end()){
            nodes[id].is_operational = status;
        }
    }


    void setEdgeOperational(int edge_id, bool status){
        for(auto& pair : adj){
            for(auto& edge : pair.second){
                if(edge.edge_id == edge_id){
                    edge.is_operational = status;
                }
            }
        }
    }

    const std::unordered_map<int, Node>& getNode() const {return nodes;}
    const std::unordered_map<int, std::vector<Edge>>& getAdjList() const {return adj; }

    const Node* getNode(int id) const {
        auto it = nodes.find(id);
        return (it != nodes.end()) ? &(it->second) : nullptr;

    }

    double getEuclideanDistance(int u_id, int v_id) const {
        const Node* u = getNode(u_id);
        const Node* v = getNode(v_id);
        if(!u || !v) return 0.0;
        double dx = u->x - v->x;
        double dy = u->y - v->y;
        return std::sqrt(dx * dx + dy * dy);
    }

    void displayCityGraph() const {
        std::cout<< "\n======= CURRENT CITY GRAPH STATE =======\n";
        for(const auto& pair : nodes){
            const Node& n = pair.second;
            std::cout << "Node [" << n.id << "] (" << n.x << ", " << n.y << ")"
                      << " | Type: " << (n.is_station ? "RELIEF STATION" : "Intersection")
                      << " | Status: " << (n.is_operational ? "OPERATIONAL" : "BLOCKED") << "\n";

            auto it = adj.find(n.id);
            if(it != adj.end()){
                for(const auto& edge : it->second){
                    std::cout << "   ---> Destination Node: " << edge.dest 
                              << " | Weight: " << edge.weight 
                              << " | Edge ID: " << edge.edge_id 
                              << " | Status: " << (edge.is_operational ? "ACTIVE" : "DAMAGED") << "\n";
                }
            }

        }
    }

    bool loadFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open map file '" << filename << "'.\n";
            return false;
        }

        nodes.clear();
        adj.clear();
        scheduled_closures.clear();

        std::string line;
        while (std::getline(file, line)) {
            // Skip empty lines and comments
            if (line.empty() || line[0] == '#') continue;

            std::istringstream iss(line);
            std::string type;
            iss >> type;

            if (type == "NODE") {
                int id, is_station;
                double x, y;
                if (iss >> id >> x >> y >> is_station) {
                    addNode(id, x, y, is_station == 1);
                }
            } else if (type == "EDGE") {
                int edge_id, src, dest, bidirectional;
                double weight;
                if (iss >> edge_id >> src >> dest >> weight >> bidirectional) {
                    addEdge(edge_id, src, dest, weight, bidirectional == 1);
                }
            } else if (type == "CLOSURE") {
                int edge_id;
                double start_time, end_time;
                if (iss >> edge_id >> start_time >> end_time) {
                    addScheduledClosure(edge_id, start_time, end_time);
                }
            }
        }

        file.close();
        return true;
    }

};








#endif