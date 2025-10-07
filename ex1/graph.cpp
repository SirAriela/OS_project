#include "graph.hpp"
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <set>

namespace graph {
    
    // Constructor
    Graph::Graph(int n) : numVertices(n) {
        if (n <= 0) {
            throw std::invalid_argument("Number of vertices must be positive");
        }
        adjList = new Neighbor*[n];
        for (int i = 0; i < n; i++) {
            adjList[i] = nullptr;
        }
    }
    
    // Copy constructor  
    Graph::Graph(const Graph& other) : numVertices(other.numVertices) {
        adjList = new Neighbor*[numVertices];
        for (int i = 0; i < numVertices; i++) {
            adjList[i] = nullptr;
            Neighbor* current = other.adjList[i];
            Neighbor** tail = &adjList[i];
            while (current != nullptr) {
                *tail = new Neighbor(current->dest, current->weight);
                tail = &((*tail)->next);
                current = current->next;
            }
        }
    }
    
    // Assignment operator
    Graph& Graph::operator=(const Graph& other) {
        if (this != &other) {
            for (int i = 0; i < numVertices; i++) {
                Neighbor* current = adjList[i];
                while (current != nullptr) {
                    Neighbor* temp = current;
                    current = current->next;
                    delete temp;
                }
            }
            delete[] adjList;
            
            numVertices = other.numVertices;
            adjList = new Neighbor*[numVertices];
            
            for (int i = 0; i < numVertices; i++) {
                adjList[i] = nullptr;
                Neighbor* current = other.adjList[i];
                Neighbor** tail = &adjList[i];
                while (current != nullptr) {
                    *tail = new Neighbor(current->dest, current->weight);
                    tail = &((*tail)->next);
                    current = current->next;
                }
            }
        }
        return *this;
    }
    
    // Add edge
    void Graph::addEdge(int src, int dest, int weight) {
        if (src < 0 || src >= numVertices || dest < 0 || dest >= numVertices) {
            throw std::out_of_range("Vertex index out of range");
        }
        if (src == dest) {
            throw std::invalid_argument("Self loops are not allowed");
        }
        
        if (hasEdge(src, dest)) {
            Neighbor* current = adjList[src];
            while (current != nullptr && current->dest != dest) {
                current = current->next;
            }
            if (current != nullptr) {
                current->weight = weight;
            }
            current = adjList[dest];
            while (current != nullptr && current->dest != src) {
                current = current->next;
            }
            if (current != nullptr) {
                current->weight = weight;
            }
        } else {
            adjList[src] = new Neighbor(dest, weight, adjList[src]);
            adjList[dest] = new Neighbor(src, weight, adjList[dest]);
        }
    }
    
    // Remove edge
    void Graph::removeEdge(int src, int dest) {
        if (src < 0 || src >= numVertices || dest < 0 || dest >= numVertices) {
            throw std::out_of_range("Vertex index out of range");
        }
        if (!hasEdge(src, dest)) {
            throw std::runtime_error("Edge not found");
        }
        
        Neighbor** current = &adjList[src];
        while (*current != nullptr) {
            if ((*current)->dest == dest) {
                Neighbor* temp = *current;
                *current = (*current)->next;
                delete temp;
                break;
            }
            current = &((*current)->next);
        }
        
        current = &adjList[dest];
        while (*current != nullptr) {
            if ((*current)->dest == src) {
                Neighbor* temp = *current;
                *current = (*current)->next;
                delete temp;
                break;
            }
            current = &((*current)->next);
        }
    }
    
    // Print graph
    void Graph::print_graph() const {
        for (int i = 0; i < numVertices; i++) {
            std::cout << "Vertex " << i << " -> ";
            Neighbor* current = adjList[i];
            while (current != nullptr) {
                std::cout << "(" << current->dest << ", weight: " << current->weight << ") ";
                current = current->next;
            }
            std::cout << std::endl;
        }
    }
    
    // Get number of vertices
    int Graph::getNumVertices() const {
        return numVertices;
    }
    
    // Get neighbors
    Neighbor* Graph::getNeighbors(int vertex) const {
        if (vertex < 0 || vertex >= numVertices) {
            throw std::out_of_range("Vertex index out of range");
        }
        return adjList[vertex];
    }
    
    // Check if edge exists
    bool Graph::hasEdge(int src, int dest) const {
        if (src < 0 || src >= numVertices || dest < 0 || dest >= numVertices) {
            throw std::out_of_range("Vertex index out of range");
        }
        Neighbor* current = adjList[src];
        while (current != nullptr) {
            if (current->dest == dest) {
                return true;
            }
            current = current->next;
        }
        return false;
    }
    
    // Get edge weight
    int Graph::getEdgeWeight(int src, int dest) const {
        if (src < 0 || src >= numVertices || dest < 0 || dest >= numVertices) {
            throw std::out_of_range("Vertex index out of range");
        }
        Neighbor* current = adjList[src];
        while (current != nullptr) {
            if (current->dest == dest) {
                return current->weight;
            }
            current = current->next;
        }
        throw std::runtime_error("Edge not found");
    }
    
    // Get degree
    int Graph::getDegree(int vertex) const {
        if (vertex < 0 || vertex >= numVertices) {
            throw std::out_of_range("Vertex index out of range");
        }
        int degree = 0;
        Neighbor* current = adjList[vertex];
        while (current != nullptr) {
            degree++;
            current = current->next;
        }
        return degree;
    }
    
    // Get number of edges
    int Graph::getNumEdges() const {
        int edges = 0;
        for (int i = 0; i < numVertices; i++) {
            Neighbor* current = adjList[i];
            while (current != nullptr) {
                edges++;
                current = current->next;
            }
        }
        return edges / 2;
    }
    
    // DFS helper
    void Graph::dfs(int vertex, std::vector<bool>& visited) const {
        visited[vertex] = true;
        Neighbor* current = adjList[vertex];
        while (current != nullptr) {
            if (!visited[current->dest]) {
                dfs(current->dest, visited);
            }
            current = current->next;
        }
    }
    
    // Check connectivity
    bool Graph::isConnected() const {
        int start = -1;
        for (int i = 0; i < numVertices; i++) {
            if (getDegree(i) > 0) {
                start = i;
                break;
            }
        }
        if (start == -1) {
            return true;
        }
        
        std::vector<bool> visited(numVertices, false);
        dfs(start, visited);
        
        for (int i = 0; i < numVertices; i++) {
            if (getDegree(i) > 0 && !visited[i]) {
                return false;
            }
        }
        return true;
    }
    
    // Check Euler circuit
    bool Graph::hasEulerCircuit() const {
        if (!isConnected()) {
            return false;
        }
        for (int i = 0; i < numVertices; i++) {
            if (getDegree(i) % 2 != 0) {
                return false;
            }
        }
        return true;
    }
    
    // Hierholzer DFS
    void Graph::hierholzerDFS(int vertex, std::vector<int>& circuit, Graph& tempGraph) const {
        while (tempGraph.getDegree(vertex) > 0) {
            Neighbor* neighbor = tempGraph.getNeighbors(vertex);
            if (neighbor == nullptr) break;
            int next = neighbor->dest;
            tempGraph.removeEdge(vertex, next);
            hierholzerDFS(next, circuit, tempGraph);
        }
        circuit.push_back(vertex);
    }
    
    // Find Euler circuit
    std::vector<int> Graph::findEulerCircuit() const {
        std::vector<int> circuit;
        if (!hasEulerCircuit()) {
            return circuit;
        }
        
        int start = -1;
        for (int i = 0; i < numVertices; i++) {
            if (getDegree(i) > 0) {
                start = i;
                break;
            }
        }
        if (start == -1) {
            return circuit;
        }
        
        Graph tempGraph(*this);
        hierholzerDFS(start, circuit, tempGraph);
        std::reverse(circuit.begin(), circuit.end());
        return circuit;
    }
    
    // Generate random graph
    Graph Graph::generateRandomGraph(int vertices, int edges, unsigned int seed) {
        Graph graph(vertices);
        std::mt19937 gen(seed);
        std::uniform_int_distribution<> dis(0, vertices - 1);
        
        int edgesAdded = 0;
        int maxPossibleEdges = (vertices * (vertices - 1)) / 2;
        if (edges > maxPossibleEdges) {
            edges = maxPossibleEdges;
        }
        
        std::set<std::pair<int, int>> edgeSet;
        while (edgesAdded < edges) {
            int u = dis(gen);
            int v = dis(gen);
            if (u != v) {
                if (u > v) std::swap(u, v);
                if (edgeSet.find({u, v}) == edgeSet.end()) {
                    graph.addEdge(u, v);
                    edgeSet.insert({u, v});
                    edgesAdded++;
                }
            }
        }
        return graph;
    }
    
    // Display
    void Graph::display() const {
        std::cout << "Graph with " << numVertices << " vertices and " << getNumEdges() << " edges:\n";
        print_graph();
    }
    
    // Destructor
    Graph::~Graph() {
        for (int i = 0; i < numVertices; i++) {
            Neighbor* current = adjList[i];
            while (current != nullptr) {
                Neighbor* temp = current;
                current = current->next;
                delete temp;
            }
        }
        delete[] adjList;
    }
    
} 
