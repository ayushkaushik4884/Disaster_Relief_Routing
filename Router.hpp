#ifndef ROUTER_HPP
#define ROUTER_HPP

#include "Graph.hpp"
#include "MinHeap.hpp"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <limits>
#include <algorithm>
#include <iostream>
#include <chrono>

struct RouteResult {
    std::vector<int> path;
    double total_cost = 0.0;
    bool found = false;
    int nodes_expanded = 0;
    double execution_time_us = 0.0;
};

class Router {
public:
    // 1. Find Nearest Operational Relief Station from Disaster Node
    static int findNearestReliefStation(const Graph& graph, int disasterNodeId, double currentTime) {
        const Node* disasterNode = graph.getNode(disasterNodeId);
        if (!disasterNode || !disasterNode->is_operational) {
            return -1;
        }

        MinHeap pq;
        std::unordered_map<int, double> dist;
        std::unordered_map<int, bool> visited;

        dist[disasterNodeId] = 0.0;
        pq.push(disasterNodeId, 0.0);

        while (!pq.isEmpty()) {
            HeapNode current = pq.pop();
            int u = current.id;

            if (visited[u]) continue;
            visited[u] = true;

            const Node* node = graph.getNode(u);
            if (node && node->is_station && node->is_operational && u != disasterNodeId) {
                return u; // First operational relief station reached
            }

            auto adjIt = graph.getAdjList().find(u);
            if (adjIt == graph.getAdjList().end()) continue;

            for (const auto& edge : adjIt->second) {
                int v = edge.dest;
                const Node* destNode = graph.getNode(v);

                if (!destNode || !destNode->is_operational) continue;
                if (!graph.isEdgeAvailable(edge, currentTime)) continue;

                double newDist = dist[u] + edge.weight;
                if (dist.find(v) == dist.end() || newDist < dist[v]) {
                    dist[v] = newDist;
                    pq.push(v, newDist);
                }
            }
        }
        return -1;
    }

    // 2. Standard Dijkstra Shortest Path
    static RouteResult computeDijkstra(const Graph& graph, int startId, int goalId, double currentTime,
                                       const std::unordered_set<int>& maskedEdges = {}) {
        auto startTime = std::chrono::high_resolution_clock::now();
        RouteResult result;

        const Node* startNode = graph.getNode(startId);
        const Node* goalNode = graph.getNode(goalId);

        if (!startNode || !goalNode || !startNode->is_operational || !goalNode->is_operational) {
            return result;
        }

        MinHeap pq;
        std::unordered_map<int, double> dist;
        std::unordered_map<int, int> parent;
        std::unordered_map<int, bool> visited;

        dist[startId] = 0.0;
        pq.push(startId, 0.0);

        while (!pq.isEmpty()) {
            HeapNode current = pq.pop();
            int u = current.id;

            if (visited[u]) continue;
            visited[u] = true;
            result.nodes_expanded++;

            if (u == goalId) {
                result.found = true;
                result.total_cost = dist[goalId];
                break;
            }

            auto adjIt = graph.getAdjList().find(u);
            if (adjIt == graph.getAdjList().end()) continue;

            for (const auto& edge : adjIt->second) {
                if (maskedEdges.count(edge.edge_id)) continue;
                int v = edge.dest;
                const Node* destNode = graph.getNode(v);

                if (!destNode || !destNode->is_operational) continue;
                if (!graph.isEdgeAvailable(edge, currentTime)) continue;

                double newDist = dist[u] + edge.weight;
                if (dist.find(v) == dist.end() || newDist < dist[v]) {
                    dist[v] = newDist;
                    parent[v] = u;
                    pq.push(v, newDist);
                }
            }
        }

        if (result.found) {
            int curr = goalId;
            while (curr != startId) {
                result.path.push_back(curr);
                curr = parent[curr];
            }
            result.path.push_back(startId);
            std::reverse(result.path.begin(), result.path.end());
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        result.execution_time_us = std::chrono::duration<double, std::micro>(endTime - startTime).count();

        return result;
    }

    // 3. A* Search Algorithm (Euclidean Distance Heuristic)
    static RouteResult computeAStar(const Graph& graph, int startId, int goalId, double currentTime,
                                    const std::unordered_set<int>& maskedEdges = {}) {
        auto startTime = std::chrono::high_resolution_clock::now();
        RouteResult result;

        const Node* startNode = graph.getNode(startId);
        const Node* goalNode = graph.getNode(goalId);

        if (!startNode || !goalNode || !startNode->is_operational || !goalNode->is_operational) {
            return result;
        }

        MinHeap pq;
        std::unordered_map<int, double> gScore;
        std::unordered_map<int, int> parent;
        std::unordered_map<int, bool> visited;

        gScore[startId] = 0.0;
        double hStart = graph.getEuclideanDistance(startId, goalId);
        pq.push(startId, 0.0 + hStart);

        while (!pq.isEmpty()) {
            HeapNode current = pq.pop();
            int u = current.id;

            if (visited[u]) continue;
            visited[u] = true;
            result.nodes_expanded++;

            if (u == goalId) {
                result.found = true;
                result.total_cost = gScore[goalId];
                break;
            }

            auto adjIt = graph.getAdjList().find(u);
            if (adjIt == graph.getAdjList().end()) continue;

            for (const auto& edge : adjIt->second) {
                if (maskedEdges.count(edge.edge_id)) continue;
                int v = edge.dest;
                const Node* destNode = graph.getNode(v);

                if (!destNode || !destNode->is_operational) continue;
                if (!graph.isEdgeAvailable(edge, currentTime)) continue;

                double tentative_g = gScore[u] + edge.weight;
                if (gScore.find(v) == gScore.end() || tentative_g < gScore[v]) {
                    gScore[v] = tentative_g;
                    parent[v] = u;
                    double fScore = tentative_g + graph.getEuclideanDistance(v, goalId);
                    pq.push(v, fScore);
                }
            }
        }

        if (result.found) {
            int curr = goalId;
            while (curr != startId) {
                result.path.push_back(curr);
                curr = parent[curr];
            }
            result.path.push_back(startId);
            std::reverse(result.path.begin(), result.path.end());
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        result.execution_time_us = std::chrono::duration<double, std::micro>(endTime - startTime).count();

        return result;
    }

    // 4. Alternative Backup Routes via Edge-Masking
    static std::vector<RouteResult> computeBackupRoutes(const Graph& graph, int startId, int goalId, 
                                                        double currentTime, int k = 2) {
        std::vector<RouteResult> routes;
        RouteResult primary = computeAStar(graph, startId, goalId, currentTime);

        if (!primary.found) return routes;
        routes.push_back(primary);

        std::unordered_set<int> maskedEdges;

        for (int i = 1; i <= k; ++i) {
            const auto& lastPath = routes.back().path;
            if (lastPath.size() < 2) break;

            int midIdx = lastPath.size() / 2;
            int u = lastPath[midIdx - 1];
            int v = lastPath[midIdx];

            auto adjIt = graph.getAdjList().find(u);
            if (adjIt != graph.getAdjList().end()) {
                for (const auto& edge : adjIt->second) {
                    if (edge.dest == v) {
                        maskedEdges.insert(edge.edge_id);
                    }
                }
            }

            RouteResult backup = computeAStar(graph, startId, goalId, currentTime, maskedEdges);
            if (backup.found) {
                routes.push_back(backup);
            } else {
                break;
            }
        }

        return routes;
    }

    // 5. Active Route Validation Check
    static bool isRouteValid(const Graph& graph, const std::vector<int>& path, double currentTime) {
        if (path.empty()) return false;

        for (size_t i = 0; i < path.size(); ++i) {
            const Node* node = graph.getNode(path[i]);
            if (!node || !node->is_operational) return false;

            if (i < path.size() - 1) {
                int u = path[i];
                int v = path[i + 1];
                bool edgeFoundAndAvailable = false;

                auto adjIt = graph.getAdjList().find(u);
                if (adjIt != graph.getAdjList().end()) {
                    for (const auto& edge : adjIt->second) {
                        if (edge.dest == v) {
                            if (graph.isEdgeAvailable(edge, currentTime)) {
                                edgeFoundAndAvailable = true;
                                break;
                            }
                        }
                    }
                }
                if (!edgeFoundAndAvailable) return false;
            }
        }
        return true;
    }
};

#endif // ROUTER_HPP