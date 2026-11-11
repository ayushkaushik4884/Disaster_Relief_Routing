#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <vector>
#include <unordered_map>
#include <iostream>
#include <cmath>
#include <iomanip>

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

    void addNote(int id, double x, double y, bool is_station = false){
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

};








#endif