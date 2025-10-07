#include <iostream>
#include <string>
#include <memory>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <vector>
#include <algorithm>
#include <thread>
#include <mutex>
#include <chrono>
#include <condition_variable>  // For Leader-Follower synchronization
#include <queue>              // For work queue
#include "graph.hpp"
#include "graph_algorithm.hpp"

#define BACKLOG 5
#define THREAD_POOL_SIZE 4    // Fixed number of worker threads

// Global variables for server control
volatile sig_atomic_t running = 1;
int listen_socket_fd = -1;

void analyzeGraph(const graph::Graph& g) {
    std::cout << "\n" << std::string(50, '=') << "\n";
    std::cout << "GRAPH ANALYSIS\n";
    std::cout << std::string(50, '=') << "\n";
    
    g.display();
    
    std::cout << "\nVertex degrees:\n";
    bool allEven = true;
    for (int i = 0; i < g.getNumVertices(); i++) {
        int degree = g.getDegree(i);
        std::cout << "Vertex " << i << ": degree " << degree;
        if (degree % 2 != 0) {
            std::cout << " (odd)";
            allEven = false;
        } else if (degree > 0) {
            std::cout << " (even)";
        }
        std::cout << "\n";
    }
    
    std::cout << "\nConnectivity: " << (g.isConnected() ? "Connected" : "Disconnected") << "\n";
    std::cout << "All degrees even: " << (allEven ? "Yes" : "No") << "\n";
    
    std::cout << "\n" << std::string(30, '-') << "\n";
    std::cout << "EULER CIRCUIT ANALYSIS\n";
    std::cout << std::string(30, '-') << "\n";
    
    if (g.hasEulerCircuit()) {
        std::cout << "✓ Euler circuit EXISTS!\n";
        std::cout << "Finding Euler circuit...\n";
        
        auto start = std::chrono::high_resolution_clock::now();
        std::vector<int> circuit = g.findEulerCircuit();
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "Euler circuit found in " << duration.count() << " microseconds:\n";
        std::cout << "Circuit: ";
        for (size_t i = 0; i < circuit.size(); i++) {
            std::cout << circuit[i];
            if (i < circuit.size() - 1) std::cout << " → ";
        }
        std::cout << "\n";
        std::cout << "Circuit length: " << circuit.size() << " vertices\n";
    } else {
        std::cout << "✗ No Euler circuit exists\n";
        if (!g.isConnected()) {
            std::cout << "Reason: Graph is not connected\n";
        } else if (!allEven) {
            std::cout << "Reason: Not all vertices have even degree\n";
        }
    }
    
    std::cout << std::string(50, '=') << "\n";
}

std::string processGraphRequest(const std::string &request)
{
    // Parse request like "-e 5 -v 4 -s 43" (no algorithm parameter needed)
    int edges = -1, vertices = -1, seed = -1;

    // Parse parameters
    size_t pos = request.find("-e ");
    if (pos != std::string::npos)
    {
        edges = std::stoi(request.substr(pos + 3));
    }

    pos = request.find("-v ");
    if (pos != std::string::npos)
    {
        vertices = std::stoi(request.substr(pos + 3));
    }

    pos = request.find("-s ");
    if (pos != std::string::npos)
    {
        seed = std::stoi(request.substr(pos + 3));
    }

    if (edges < 0 || vertices <= 0)
    {
        return "ERROR: Invalid parameters. Use format: -e <edges> -v <vertices> -s <seed>\n"
               "Server will run all algorithms on the generated graph.";
    }

    try
    {
        // Generate random graph
        graph::Graph graph = graph::Graph::generateRandomGraph(vertices, edges, seed);
        
        std::string result = "=== COMPLETE GRAPH ANALYSIS ===\n";
        result += "Graph Parameters:\n";
        result += "Vertices: " + std::to_string(vertices) + "\n";
        result += "Edges: " + std::to_string(edges) + "\n";
        result += "Seed: " + std::to_string(seed) + "\n\n";
        
        // Run all algorithms on the same graph
        
        // 1. Euler Circuit Analysis
        result += "=== 1. EULER CIRCUIT ANALYSIS ===\n";
        if (graph.hasEulerCircuit())
        {
            std::vector<int> circuit = graph.findEulerCircuit();
            result += "SUCCESS: Graph has Euler circuit!\n";
            result += "Circuit: ";
            for (size_t i = 0; i < circuit.size(); ++i)
            {
                if (i > 0)
                    result += " -> ";
                result += std::to_string(circuit[i]);
            }
            result += "\n";
        }
        else
        {
            result += "RESULT: Graph does NOT have Euler circuit\n";
            result += "Reason: Graph is not connected or has odd-degree vertices\n";
        }
        result += "\n";
        
        // 2. MST Weight Algorithm
        result += "=== 2. MST WEIGHT ANALYSIS ===\n";
        auto mstAlgo = graph::AlgorithmFactory::createAlgorithm(graph::AlgorithmFactory::AlgorithmType::MST_WEIGHT);
        if (mstAlgo) {
            result += mstAlgo->execute(graph) + "\n";
        }
        result += "\n";
        
        // 3. Strongly Connected Components
        result += "=== 3. STRONGLY CONNECTED COMPONENTS ===\n";
        auto sccAlgo = graph::AlgorithmFactory::createAlgorithm(graph::AlgorithmFactory::AlgorithmType::SCC);
        if (sccAlgo) {
            result += sccAlgo->execute(graph) + "\n";
        }
        result += "\n";
        
        // 4. Max Flow Algorithm
        result += "=== 4. MAX FLOW ANALYSIS ===\n";
        auto maxFlowAlgo = graph::AlgorithmFactory::createAlgorithm(graph::AlgorithmFactory::AlgorithmType::MAX_FLOW);
        if (maxFlowAlgo) {
            result += maxFlowAlgo->execute(graph) + "\n";
        }
        result += "\n";
        
        // 5. Max Clique Algorithm
        result += "=== 5. MAX CLIQUE ANALYSIS ===\n";
        auto maxCliqueAlgo = graph::AlgorithmFactory::createAlgorithm(graph::AlgorithmFactory::AlgorithmType::MAX_CLIQUE);
        if (maxCliqueAlgo) {
            result += maxCliqueAlgo->execute(graph) + "\n";
        }
        result += "\n";
        
        result += "=== ANALYSIS COMPLETE ===\n";
        
        // Display detailed graph info on server console
        analyzeGraph(graph);
        
        return result;
    }
    catch (const std::exception &e)
    {
        return "ERROR: " + std::string(e.what());
    }
}

// Leader-Follower pattern implementation
class LeaderFollowerServer {
private:
    // Work item structure - now only contains socket and IP, request is read by worker
    struct WorkItem {
        int client_fd;
        std::string client_ip;
        
        WorkItem(int fd, const std::string& ip) 
            : client_fd(fd), client_ip(ip) {}
    };
    
    // Synchronization primitives for Leader-Follower pattern
    std::mutex leader_mutex;           // Protects leader selection
    std::condition_variable leader_cv; // Notifies when leader role is available
    std::mutex work_queue_mutex;       // Protects work queue
    std::condition_variable work_cv;   // Notifies when work is available
    
    // Core data structures
    std::queue<WorkItem> work_queue;   // Queue of pending work items
    std::vector<std::thread> thread_pool; // Fixed pool of worker threads
    bool leader_available;             // Flag indicating if leader role is available
    int current_leader_id;             // ID of current leader thread
    
public:
    LeaderFollowerServer() : leader_available(true), current_leader_id(-1) {
        // Create fixed thread pool
        std::cout << "Creating Leader-Follower server with " << THREAD_POOL_SIZE << " threads\n";
        for (int i = 0; i < THREAD_POOL_SIZE; ++i) {
            thread_pool.emplace_back(&LeaderFollowerServer::workerThread, this, i);
        }
    }
    
    ~LeaderFollowerServer() {
        shutdown();
    }
    
    // Add work item to the queue - only socket and IP, no pre-read request
    void addWork(int client_fd, const std::string& client_ip) {
        {
            std::lock_guard<std::mutex> lock(work_queue_mutex);
            work_queue.emplace(client_fd, client_ip);
            std::cout << "Added work to queue. Queue size: " << work_queue.size() << std::endl;
        }
        work_cv.notify_one(); // Wake up one waiting worker
    }
    
    // Shutdown the server and join all threads
    void shutdown() {
        std::cout << "Shutting down Leader-Follower server...\n";
        running = 0;
        work_cv.notify_all();   // Wake up all waiting workers
        leader_cv.notify_all(); // Wake up all waiting threads
        
        // Join all worker threads
        for (auto& thread : thread_pool) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        std::cout << "All worker threads finished\n";
    }
    
private:
    // Main function for each worker thread - implements Leader-Follower pattern
    void workerThread(int thread_id) {
        std::cout << "Worker thread " << thread_id << " started\n";
        
        while (running) {
            // Step 1: Try to become the leader
            {
                std::unique_lock<std::mutex> lock(leader_mutex);
                // Wait until leader role is available
                leader_cv.wait(lock, [this] { return leader_available || !running; });
                
                if (!running) break; // Server is shutting down
                
                // Become the leader
                leader_available = false;
                current_leader_id = thread_id;
            }
            
            std::cout << "Thread " << thread_id << " became LEADER\n";
            
            // Step 2: As leader, wait for work
            WorkItem work_item(0, "");
            bool got_work = false;
            
            {
                std::unique_lock<std::mutex> lock(work_queue_mutex);
                // Wait for work to arrive
                work_cv.wait(lock, [this] { return !work_queue.empty() || !running; });
                
                if (!running) {
                    // Release leadership before exiting
                    {
                        std::lock_guard<std::mutex> leader_lock(leader_mutex);
                        leader_available = true;
                        current_leader_id = -1;
                    }
                    leader_cv.notify_one();
                    break;
                }
                
                // Get work from queue
                if (!work_queue.empty()) {
                    work_item = work_queue.front();
                    work_queue.pop();
                    got_work = true;
                    std::cout << "Thread " << thread_id << " got work. Queue size: " << work_queue.size() << std::endl;
                }
            }
            
            if (got_work) {
                std::cout << "Thread " << thread_id << " got work, becoming FOLLOWER\n";
                
                // Step 3: Release leadership immediately - become follower
                {
                    std::lock_guard<std::mutex> lock(leader_mutex);
                    leader_available = true;
                    current_leader_id = -1;
                }
                leader_cv.notify_one(); // Wake up another thread to become leader
                
                // Step 4: Process the work as follower
                processRequest(thread_id, work_item);
            }
        }
        
        std::cout << "Worker thread " << thread_id << " finished\n";
    }
    
    // Process a single client request - now reads request from socket
    void processRequest(int thread_id, const WorkItem& work_item) {
        std::cout << "Thread " << thread_id << " handling connection from " << work_item.client_ip << std::endl;
        
        // Read request from client socket
        char buffer[1024];
        ssize_t bytes_received = recv(work_item.client_fd, buffer, sizeof(buffer) - 1, 0);
        
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            std::string request(buffer);
            
            std::cout << "Thread " << thread_id << " processing request from " 
                      << work_item.client_ip << ": " << request << std::endl;
            
            // Process the graph request using existing function
            std::string response = processGraphRequest(request);
            
            // Send response to client
            if (send(work_item.client_fd, response.c_str(), response.length(), 0) < 0) {
                perror("send");
            }
            
            std::cout << "Thread " << thread_id << " completed processing for " 
                      << work_item.client_ip << std::endl;
        } else if (bytes_received == 0) {
            std::cout << "Thread " << thread_id << " - Client " << work_item.client_ip << " disconnected before sending request\n";
        } else {
            perror("recv");
        }
        
        // Close connection
        close(work_item.client_fd);
    }
};

// Global server instance
std::unique_ptr<LeaderFollowerServer> lf_server;

// Signal handler for graceful shutdown
void signal_handler(int sig)
{
    running = 0;
    std::cout << "\nReceived signal " << sig << ", shutting down server" << std::endl;
    
    // Shutdown Leader-Follower server
    if (lf_server) {
        lf_server->shutdown();
    }

    if (listen_socket_fd != -1)
    {
        close(listen_socket_fd);
        listen_socket_fd = -1;
    }
}

int main(int argc, char *argv[])
{
    signal(SIGINT, signal_handler);

    // TCP Server setup
    int tcp_port;

    if (argc != 2)
    {
        std::cerr << "Error: Number of parameters is incorrect\n";
        std::cerr << "Usage: " << argv[0] << " <port>\n";
        return 1;
    }
    else
    {
        tcp_port = atoi(argv[1]);
    }

    if (tcp_port <= 0 || tcp_port > 65535)
    {
        std::cerr << "Error: Invalid port number" << std::endl;
        return 1;
    }

    // Create socket
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
    {
        perror("socket");
        return 1;
    }

    listen_socket_fd = listen_fd;

    // Set socket options
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Bind socket
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(tcp_port);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(listen_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("bind");
        close(listen_fd);
        return 1;
    }

    if (listen(listen_fd, BACKLOG) < 0)
    {
        perror("listen");
        close(listen_fd);
        return 1;
    }

    std::cout << "Leader-Follower Server listening on port " << tcp_port << std::endl;
    std::cout << "Thread pool size: " << THREAD_POOL_SIZE << std::endl;
    std::cout << "Waiting for connections..." << std::endl;

    // Initialize Leader-Follower server
    lf_server = std::make_unique<LeaderFollowerServer>();

    // Main accept loop - now only handles connection acceptance
    while (running)
    {
        // Use select to check for incoming connections with timeout
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(listen_fd, &readfds);

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 100000; // 100ms timeout

        int activity = select(listen_fd + 1, &readfds, NULL, NULL, &tv);

        if (activity < 0)
        {
            if (running) perror("select");
            continue;
        }

        if (!running)
        {
            break;
        }

        // Check if there's a new connection ready
        if (activity == 0 || !FD_ISSET(listen_fd, &readfds))
        {
            continue;
        }

        // Accept new connection
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len);
        
        if (client_fd < 0)
        {
            perror("accept");
            continue;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        std::cout << "New connection from " << client_ip << ":" << ntohs(client_addr.sin_port) << std::endl;

        // Add work to Leader-Follower queue - socket will be read by worker thread
        lf_server->addWork(client_fd, client_ip);
    }

    close(listen_fd);
    std::cout << "Server shutdown complete." << std::endl;
    return 0;
}