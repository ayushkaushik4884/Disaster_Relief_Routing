#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <limits>
#include <cmath>
#include "Graph.hpp"
#include "Router.hpp"

void clearInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

std::string formatTime(double time24) {
    int hours = static_cast<int>(time24) % 24;
    int mins = static_cast<int>(std::round((time24 - static_cast<int>(time24)) * 60.0));
    if (mins == 60) {
        hours = (hours + 1) % 24;
        mins = 0;
    }
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << hours << ":"
        << std::setfill('0') << std::setw(2) << mins;
    return oss.str();
}

void printPath(const std::vector<int>& path) {
    for (size_t i = 0; i < path.size(); ++i) {
        std::cout << path[i] << (i + 1 < path.size() ? " -> " : "");
    }
}

void printRouteDetails(const RouteResult& res, const std::string& label) {
    if (!res.found) {
        std::cout << label << ": [NO VALID ROUTE]\n";
        return;
    }
    std::cout << label << " (Cost: " << std::fixed << std::setprecision(2) << res.total_cost << "): ";
    printPath(res.path);
    std::cout << "\n";
}

bool checkEdgeAvailabilityById(const Graph& graph, int edgeId, double queryTime, bool& exists) {
    exists = false;
    for (const auto& pair : graph.getAdjList()) {
        for (const auto& edge : pair.second) {
            if (edge.edge_id == edgeId) {
                exists = true;
                return graph.isEdgeAvailable(edge, queryTime);
            }
        }
    }
    return false;
}

// Monitors active emergency route after any graph or time change and fails over if needed
void monitorAndFailover(const Graph& graph, double currentTime, int& disasterNode,
                        int& assignedStation, std::vector<RouteResult>& routes, int& activeIdx) {
    if (disasterNode == -1) return;

    const Node* dNode = graph.getNode(disasterNode);
    if (!dNode || !dNode->is_operational) {
        std::cout << "\n[ALERT] Disaster Node " << disasterNode << " is marked non-operational/destroyed.\n";
        return;
    }

    if (activeIdx >= 0 && activeIdx < static_cast<int>(routes.size())) {
        if (Router::isRouteValid(graph, routes[activeIdx].path, currentTime)) {
            std::cout << "\n[MONITOR] Active Route #" << activeIdx << " remains VALID.\n";
            return;
        }
    }

    std::cout << "\n[ALERT] Active route INVALIDATED due to road/node blockage or scheduled closure!\n";

    // Check pre-generated backup routes
    for (int i = 0; i < static_cast<int>(routes.size()); ++i) {
        if (i == activeIdx) continue;
        if (Router::isRouteValid(graph, routes[i].path, currentTime)) {
            activeIdx = i;
            std::cout << "[FAILOVER] Switched automatically to Backup Route #" << activeIdx << ":\n  ";
            printRouteDetails(routes[activeIdx], "Active Route");
            return;
        }
    }

    // No pre-computed backup is valid -> trigger full recalculation
    std::cout << "[RECALCULATING] All cached backup routes blocked. Re-running station & A* search...\n";
    assignedStation = Router::findNearestReliefStation(graph, disasterNode, currentTime);
    if (assignedStation == -1) {
        std::cout << "[CRITICAL] No reachable operational relief station found!\n";
        routes.clear();
        activeIdx = -1;
        return;
    }

    routes = Router::computeBackupRoutes(graph, assignedStation, disasterNode, currentTime, 2);
    if (!routes.empty()) {
        activeIdx = 0;
        std::cout << "[REROUTED] Assigned Station " << assignedStation << ". New ";
        printRouteDetails(routes[0], "Primary Route");
    } else {
        activeIdx = -1;
        std::cout << "[CRITICAL] Could not establish any path from Station " << assignedStation << ".\n";
    }
}

void printMenu(double currentTime, int disasterNode, int assignedStation, int activeIdx) {
    std::cout << "\n==========================================================\n"
              << "       DISASTER RELIEF ROUTING OPTIMIZER (C++ DSA)        \n"
              << "==========================================================\n"
              << " Sim Time: " << formatTime(currentTime)
              << " | Active Disaster: " << (disasterNode == -1 ? "None" : "Node " + std::to_string(disasterNode))
              << " | Station: " << (assignedStation == -1 ? "None" : "Node " + std::to_string(assignedStation));
    if (activeIdx != -1) {
        std::cout << " (Route #" << activeIdx << ")";
    }
    std::cout << "\n----------------------------------------------------------\n"
              << " 1. Display City Graph (Nodes, Roads, Status)\n"
              << " 2. Trigger Disaster at Node (Allocate Station & Routes)\n"
              << " 3. Block / Damage a Road (Edge Failure)\n"
              << " 4. Repair / Restore a Road (Edge Recovery)\n"
              << " 5. Close / Open a Node (Emergency Node Toggle)\n"
              << " 6. Set Simulation Time (Test Scheduled Closures)\n"
              << " 7. Add Scheduled Road Closure & Query Availability\n"
              << " 8. Compare Algorithms: Dijkstra vs A* Benchmark\n"
              << " 9. Reload Map from File\n"
              << " 0. Exit\n"
              << "----------------------------------------------------------\n"
              << "Enter choice: ";
}

int main() {
    Graph city;
    std::string mapFile = "city_map.txt";

    if (!city.loadFromFile(mapFile)) {
        std::cerr << "Failed to load '" << mapFile << "'. Ensure it is in the working directory.\n";
        return 1;
    }
    std::cout << "Successfully loaded city network from '" << mapFile << "'.\n";

    double currentTime = 14.0; // Default: 14:00 (2:00 PM)
    int disasterNode = -1;
    int assignedStation = -1;
    std::vector<RouteResult> activeRoutes;
    int activeRouteIdx = -1;

    int choice;
    while (true) {
        printMenu(currentTime, disasterNode, assignedStation, activeRouteIdx);
        if (!(std::cin >> choice)) {
            clearInput();
            std::cout << "Invalid input. Please enter a number.\n";
            continue;
        }

        if (choice == 0) {
            std::cout << "Exiting Disaster Relief Routing Optimizer.\n";
            break;
        }

        switch (choice) {
            case 1: {
                city.displayCityGraph();
                break;
            }

            case 2: {
                int targetNode;
                std::cout << "Enter Disaster Node ID: ";
                if (!(std::cin >> targetNode)) { clearInput(); break; }

                const Node* n = city.getNode(targetNode);
                if (!n) {
                    std::cout << "Error: Node " << targetNode << " does not exist.\n";
                    break;
                }
                if (!n->is_operational) {
                    std::cout << "Error: Node " << targetNode << " is currently closed/non-operational.\n";
                    break;
                }

                disasterNode = targetNode;
                assignedStation = Router::findNearestReliefStation(city, disasterNode, currentTime);

                if (assignedStation == -1) {
                    std::cout << "ALERT: No reachable relief station from Node " << disasterNode << "!\n";
                    activeRoutes.clear();
                    activeRouteIdx = -1;
                    break;
                }

                std::cout << "\nNearest Operational Relief Station: Node " << assignedStation << "\n";
                activeRoutes = Router::computeBackupRoutes(city, assignedStation, disasterNode, currentTime, 2);

                if (activeRoutes.empty()) {
                    std::cout << "No valid route found.\n";
                    activeRouteIdx = -1;
                } else {
                    activeRouteIdx = 0;
                    printRouteDetails(activeRoutes[0], "Primary Route (#0)");
                    for (size_t i = 1; i < activeRoutes.size(); ++i) {
                        printRouteDetails(activeRoutes[i], "Backup Route  (#" + std::to_string(i) + ")");
                    }
                }
                break;
            }

            case 3: {
                int edgeId;
                std::cout << "Enter Edge ID to block/damage: ";
                if (!(std::cin >> edgeId)) { clearInput(); break; }

                city.setEdgeOperational(edgeId, false);
                std::cout << "Road (Edge " << edgeId << ") marked DAMAGED / UNAVAILABLE.\n";
                monitorAndFailover(city, currentTime, disasterNode, assignedStation, activeRoutes, activeRouteIdx);
                break;
            }

            case 4: {
                int edgeId;
                std::cout << "Enter Edge ID to repair/restore: ";
                if (!(std::cin >> edgeId)) { clearInput(); break; }

                city.setEdgeOperational(edgeId, true);
                std::cout << "Road (Edge " << edgeId << ") REPAIRED and active.\n";

                // Recalculate optimal route if a disaster is currently active
                if (disasterNode != -1) {
                    std::cout << "Re-evaluating optimal routes with restored road...\n";
                    assignedStation = Router::findNearestReliefStation(city, disasterNode, currentTime);
                    if (assignedStation != -1) {
                        activeRoutes = Router::computeBackupRoutes(city, assignedStation, disasterNode, currentTime, 2);
                        activeRouteIdx = activeRoutes.empty() ? -1 : 0;
                        if (activeRouteIdx == 0) {
                            printRouteDetails(activeRoutes[0], "Updated Primary Route (#0)");
                        }
                    }
                }
                break;
            }

            case 5: {
                int nodeId, status;
                std::cout << "Enter Node ID: ";
                if (!(std::cin >> nodeId)) { clearInput(); break; }
                std::cout << "Set Status (1 = Operational, 0 = Closed/Blocked): ";
                if (!(std::cin >> status)) { clearInput(); break; }

                city.setNodeOperational(nodeId, status == 1);
                std::cout << "Node " << nodeId << " is now " << (status == 1 ? "OPERATIONAL" : "BLOCKED") << ".\n";
                monitorAndFailover(city, currentTime, disasterNode, assignedStation, activeRoutes, activeRouteIdx);
                break;
            }

            case 6: {
                double newTime;
                std::cout << "Enter new simulation time in 24h format (e.g., 14.5 for 14:30, 23.5 for 23:30): ";
                if (!(std::cin >> newTime) || newTime < 0.0 || newTime >= 24.0) {
                    clearInput();
                    std::cout << "Invalid time. Enter a value between 0.0 and 23.99.\n";
                    break;
                }
                currentTime = newTime;
                std::cout << "Simulation clock updated to " << formatTime(currentTime) << ".\n";
                monitorAndFailover(city, currentTime, disasterNode, assignedStation, activeRoutes, activeRouteIdx);
                break;
            }

            case 7: {
                int subChoice;
                std::cout << "1. Add Scheduled Closure | 2. Query Road Availability: ";
                if (!(std::cin >> subChoice)) { clearInput(); break; }

                if (subChoice == 1) {
                    int edgeId;
                    double startT, endT;
                    std::cout << "Enter Edge ID, Start Time (0-24), End Time (0-24): ";
                    if (std::cin >> edgeId >> startT >> endT) {
                        city.addScheduledClosure(edgeId, startT, endT);
                        std::cout << "Added scheduled closure on Edge " << edgeId
                                  << " (" << formatTime(startT) << " to " << formatTime(endT) << ").\n";
                        monitorAndFailover(city, currentTime, disasterNode, assignedStation, activeRoutes, activeRouteIdx);
                    } else {
                        clearInput();
                    }
                } else if (subChoice == 2) {
                    int edgeId;
                    double qTime;
                    std::cout << "Enter Edge ID and Time to query (0-24): ";
                    if (std::cin >> edgeId >> qTime) {
                        bool exists = false;
                        bool avail = checkEdgeAvailabilityById(city, edgeId, qTime, exists);
                        if (!exists) {
                            std::cout << "Edge " << edgeId << " not found.\n";
                        } else {
                            std::cout << "Edge " << edgeId << " at " << formatTime(qTime) << " -> "
                                      << (avail ? "AVAILABLE" : "UNAVAILABLE (Closed/Damaged)") << "\n";
                        }
                    } else {
                        clearInput();
                    }
                }
                break;
            }

            case 8: {
                int src, dest;
                std::cout << "Enter Source Node ID (e.g., Station 1) and Destination Node ID (e.g., 15): ";
                if (!(std::cin >> src >> dest)) { clearInput(); break; }

                RouteResult dijkstraRes = Router::computeDijkstra(city, src, dest, currentTime);
                RouteResult astarRes = Router::computeAStar(city, src, dest, currentTime);

                std::cout << "\n================ ALGORITHM BENCHMARK =================\n";
                std::cout << std::left << std::setw(18) << "Metric"
                          << std::setw(20) << "Dijkstra"
                          << std::setw(20) << "A* Search" << "\n";
                std::cout << "------------------------------------------------------\n";
                std::cout << std::left << std::setw(18) << "Path Found"
                          << std::setw(20) << (dijkstraRes.found ? "Yes" : "No")
                          << std::setw(20) << (astarRes.found ? "Yes" : "No") << "\n";
                std::cout << std::left << std::setw(18) << "Total Cost"
                          << std::setw(20) << std::fixed << std::setprecision(2) << dijkstraRes.total_cost
                          << std::setw(20) << std::fixed << std::setprecision(2) << astarRes.total_cost << "\n";
                std::cout << std::left << std::setw(18) << "Nodes Expanded"
                          << std::setw(20) << dijkstraRes.nodes_expanded
                          << std::setw(20) << astarRes.nodes_expanded << "\n";
                std::cout << std::left << std::setw(18) << "Time (us)"
                          << std::setw(20) << std::fixed << std::setprecision(2) << dijkstraRes.execution_time_us
                          << std::setw(20) << std::fixed << std::setprecision(2) << astarRes.execution_time_us << "\n";
                std::cout << "======================================================\n";

                printRouteDetails(dijkstraRes, "Dijkstra Path");
                printRouteDetails(astarRes,    "A* Path      ");
                break;
            }

            case 9: {
                std::cout << "Enter map filename (default: city_map.txt): ";
                std::cin >> mapFile;
                if (city.loadFromFile(mapFile)) {
                    disasterNode = -1;
                    assignedStation = -1;
                    activeRoutes.clear();
                    activeRouteIdx = -1;
                    std::cout << "Reloaded map from '" << mapFile << "' and reset simulation state.\n";
                }
                break;
            }

            default:
                std::cout << "Invalid menu option.\n";
                break;
        }
    }

    return 0;
}