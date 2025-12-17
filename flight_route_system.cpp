#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <queue>
#include <stack>
#include <set>
#include <algorithm>
#include <limits>
#include <iomanip>
#include <cmath>

// Define M_PI if not defined
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using namespace std;

// ==================== STRUKTUR DATA ====================

struct Airport {
    string code;        // IATA code (3 letters)
    string name;        // Full airport name
    int id;            // Airport ID
    string city;       // City
    string country;    // Country
    double latitude;   // Latitude for distance calculation
    double longitude;  // Longitude for distance calculation
    
    Airport() : code(""), name(""), id(-1), city(""), country(""), latitude(0), longitude(0) {}
    Airport(string c, string n, int i, string ct = "", string co = "", double lat = 0, double lon = 0) 
        : code(c), name(n), id(i), city(ct), country(co), latitude(lat), longitude(lon) {}
};

struct Route {
    string destination;
    int distance;
    string airline;
    
    Route(string dest, int dist, string air) 
        : destination(dest), distance(dist), airline(air) {}
};

struct NodeDistance {
    string airport;
    int distance;
    
    NodeDistance(string a, int d) : airport(a), distance(d) {}
    
    bool operator>(const NodeDistance& other) const {
        return distance > other.distance;
    }
};

struct Statistics {
    int totalAirports;
    int totalRoutes;
    double avgDegree;
    int maxDegree;
    int minDegree;
    string maxDegreeAirport;
    string minDegreeAirport;
};

// Struct untuk hasil pencarian path dengan detail lengkap
struct PathResult {
    vector<string> path;
    int stops;
    int totalDistance;
    string algorithm;
    bool found;
    
    PathResult() : stops(0), totalDistance(0), algorithm(""), found(false) {}
};

// ==================== FORWARD DECLARATIONS ====================
double calculateDistance(double lat1, double lon1, double lat2, double lon2);
vector<string> parseCSVLine(const string& line);
string toUpperCase(string str);
string trim(const string& str);
bool isValidAirportCode(const string& code);

// ==================== CLASS FLIGHT ROUTE GRAPH ====================

class FlightRouteGraph {
private:
    unordered_map<string, vector<Route>> adjacencyList;
    unordered_map<string, Airport> vertexMap;
    
    vector<string> reconstructPath(const unordered_map<string, string>& parent, 
                                   const string& start, const string& end) {
        vector<string> path;
        string current = end;
        
        while (current != start) {
            path.push_back(current);
            if (parent.find(current) == parent.end()) {
                return vector<string>();
            }
            current = parent.at(current);
        }
        path.push_back(start);
        reverse(path.begin(), path.end());
        
        return path;
    }
    
    // Hitung total jarak dari path
    int calculatePathDistance(const vector<string>& path) {
        if (path.size() < 2) return 0;
        
        int totalDistance = 0;
        for (size_t i = 0; i < path.size() - 1; i++) {
            string from = path[i];
            string to = path[i + 1];
            
            for (const Route& route : adjacencyList[from]) {
                if (route.destination == to) {
                    totalDistance += route.distance;
                    break;
                }
            }
        }
        return totalDistance;
    }

public:
    // ==================== FUNGSI DASAR GRAPH ====================
    
    void addVertex(const string& airportCode, const string& airportName, int airportID,
                   const string& city = "", const string& country = "", 
                   double latitude = 0, double longitude = 0) {
        if (vertexMap.find(airportCode) != vertexMap.end()) {
            return;
        }
        
        vertexMap[airportCode] = Airport(airportCode, airportName, airportID, city, country, latitude, longitude);
        adjacencyList[airportCode] = vector<Route>();
    }
    
    void addEdge(const string& from, const string& to, int distance, const string& airline) {
        if (vertexMap.find(from) == vertexMap.end()) {
            cerr << "Error: Bandara asal '" << from << "' tidak ditemukan!" << endl;
            return;
        }
        if (vertexMap.find(to) == vertexMap.end()) {
            cerr << "Error: Bandara tujuan '" << to << "' tidak ditemukan!" << endl;
            return;
        }
        
        // Check untuk duplicate route (from->to) dengan jarak yang sama
        // Kita tetap simpan jika maskapai berbeda untuk displayNeighbors
        // Tapi untuk pathfinding, kita hanya perlu satu edge per (from, to) pair
        bool routeExists = false;
        for (const Route& r : adjacencyList[from]) {
            if (r.destination == to && r.distance == distance) {
                routeExists = true;
                break;
            }
        }
        
        // Untuk pathfinding, simpan hanya satu route per destination
        // Tapi simpan semua untuk displayNeighbors
        adjacencyList[from].push_back(Route(to, distance, airline));
    }
    
    // ==================== FUNGSI BARU: GRAPH OPERATIONS ====================
    
    /**
     * removeEdge - Menghapus edge (rute) dari graph
     * @param from: Bandara asal
     * @param to: Bandara tujuan
     * @return: true jika berhasil dihapus, false jika tidak ditemukan
     */
    bool removeEdge(const string& from, const string& to) {
        // Check apakah vertex exist
        if (vertexMap.find(from) == vertexMap.end()) {
            cerr << "Error: Bandara asal '" << from << "' tidak ditemukan!" << endl;
            return false;
        }
        if (vertexMap.find(to) == vertexMap.end()) {
            cerr << "Error: Bandara tujuan '" << to << "' tidak ditemukan!" << endl;
            return false;
        }
        
        // Cari dan hapus edge
        auto& routes = adjacencyList[from];
        bool found = false;
        
        for (auto it = routes.begin(); it != routes.end(); ) {
            if (it->destination == to) {
                it = routes.erase(it);
                found = true;
                // Tidak break karena bisa ada multiple routes dengan airline berbeda
            } else {
                ++it;
            }
        }
        
        if (!found) {
            cerr << "Warning: Rute " << from << " → " << to << " tidak ditemukan!" << endl;
        }
        
        return found;
    }
    
    /**
     * hasVertex - Mengecek apakah vertex (bandara) ada dalam graph
     * @param airportCode: Kode bandara (contoh: "CGK")
     * @return: true jika ada, false jika tidak
     */
    bool hasVertex(const string& airportCode) const {
        return vertexMap.find(airportCode) != vertexMap.end();
    }
    
    /**
     * hasEdge - Mengecek apakah edge (rute) ada dalam graph
     * @param from: Bandara asal
     * @param to: Bandara tujuan
     * @return: true jika rute exist, false jika tidak
     */
    bool hasEdge(const string& from, const string& to) const {
        // Check apakah vertex exist
        if (vertexMap.find(from) == vertexMap.end()) {
            return false;
        }
        
        // Check apakah edge exist
        auto it = adjacencyList.find(from);
        if (it == adjacencyList.end()) {
            return false;
        }
        
        const vector<Route>& routes = it->second;
        for (const Route& route : routes) {
            if (route.destination == to) {
                return true;
            }
        }
        
        return false;
    }
    
    /**
     * indegree - Menghitung jumlah edge yang MASUK ke vertex
     * @param airportCode: Kode bandara
     * @return: Jumlah rute yang menuju ke bandara ini
     */
    int indegree(const string& airportCode) const {
        // Check apakah vertex exist
        if (vertexMap.find(airportCode) == vertexMap.end()) {
            cerr << "Error: Bandara '" << airportCode << "' tidak ditemukan!" << endl;
            return -1;
        }
        
        int count = 0;
        
        // Iterasi semua vertices
        for (const auto& pair : adjacencyList) {
            const string& from = pair.first;
            const vector<Route>& routes = pair.second;
            
            // Count routes yang menuju ke airportCode
            for (const Route& route : routes) {
                if (route.destination == airportCode) {
                    count++;
                }
            }
        }
        
        return count;
    }
    
    /**
     * outdegree - Menghitung jumlah edge yang KELUAR dari vertex
     * @param airportCode: Kode bandara
     * @return: Jumlah rute yang berangkat dari bandara ini
     */
    int outdegree(const string& airportCode) const {
        // Check apakah vertex exist
        if (vertexMap.find(airportCode) == vertexMap.end()) {
            cerr << "Error: Bandara '" << airportCode << "' tidak ditemukan!" << endl;
            return -1;
        }
        
        // Return jumlah routes dari bandara ini
        auto it = adjacencyList.find(airportCode);
        if (it == adjacencyList.end()) {
            return 0;
        }
        
        return it->second.size();
    }
    
    // ==================== END FUNGSI BARU ====================
    
    vector<Route> getNeighbors(const string& airportCode) {
        if (adjacencyList.find(airportCode) != adjacencyList.end()) {
            return adjacencyList[airportCode];
        }
        return vector<Route>();
    }
    
    // ==================== BFS - DENGAN RETURN PATHRESULT ====================
    PathResult findPathBFS(const string& start, const string& end) {
        PathResult result;
        result.algorithm = "BFS (Breadth-First Search)";
        
        if (vertexMap.find(start) == vertexMap.end() || 
            vertexMap.find(end) == vertexMap.end()) {
            result.found = false;
            return result;
        }
        
        unordered_map<string, string> parent;
        unordered_map<string, bool> visited;
        queue<string> q;
        
        q.push(start);
        visited[start] = true;
        
        while (!q.empty()) {
            string current = q.front();
            q.pop();
            
            if (current == end) {
                result.path = reconstructPath(parent, start, end);
                result.stops = result.path.size() - 1;
                result.totalDistance = calculatePathDistance(result.path);
                result.found = true;
                return result;
            }
            
            // Gunakan set untuk avoid duplicate neighbors
            set<string> uniqueNeighbors;
            for (const Route& route : adjacencyList[current]) {
                uniqueNeighbors.insert(route.destination);
            }
            
            for (const string& neighbor : uniqueNeighbors) {
                if (!visited[neighbor]) {
                    visited[neighbor] = true;
                    parent[neighbor] = current;
                    q.push(neighbor);
                }
            }
        }
        
        result.found = false;
        return result;
    }
    
    // ==================== DFS - DENGAN RETURN PATHRESULT ====================
    PathResult findPathDFS(const string& start, const string& end) {
        PathResult result;
        result.algorithm = "DFS (Depth-First Search)";
        
        if (vertexMap.find(start) == vertexMap.end() || 
            vertexMap.find(end) == vertexMap.end()) {
            result.found = false;
            return result;
        }
        
        unordered_map<string, string> parent;
        unordered_map<string, bool> visited;
        stack<string> s;
        
        s.push(start);
        visited[start] = true;
        
        while (!s.empty()) {
            string current = s.top();
            s.pop();
            
            if (current == end) {
                result.path = reconstructPath(parent, start, end);
                result.stops = result.path.size() - 1;
                result.totalDistance = calculatePathDistance(result.path);
                result.found = true;
                return result;
            }
            
            // Gunakan set untuk avoid duplicate neighbors
            set<string> uniqueNeighbors;
            for (const Route& route : adjacencyList[current]) {
                uniqueNeighbors.insert(route.destination);
            }
            
            for (const string& neighbor : uniqueNeighbors) {
                if (!visited[neighbor]) {
                    visited[neighbor] = true;
                    parent[neighbor] = current;
                    s.push(neighbor);
                }
            }
        }
        
        result.found = false;
        return result;
    }
    
    // Wrapper untuk kompatibilitas dengan kode lama
    vector<string> findPath(const string& start, const string& end, const string& method) {
        if (method == "BFS") {
            return findPathBFS(start, end).path;
        } else if (method == "DFS") {
            return findPathDFS(start, end).path;
        } else if (method == "DIJKSTRA") {
            return findShortestPath(start, end).path;
        }
        return vector<string>();
    }
    
    // ==================== DIJKSTRA - DENGAN RETURN PATHRESULT ====================
    PathResult findShortestPath(const string& start, const string& end) {
        PathResult result;
        result.algorithm = "Dijkstra (Shortest Path)";
        
        if (vertexMap.find(start) == vertexMap.end() || 
            vertexMap.find(end) == vertexMap.end()) {
            result.found = false;
            return result;
        }
        
        unordered_map<string, int> distance;
        unordered_map<string, string> parent;
        priority_queue<NodeDistance, vector<NodeDistance>, greater<NodeDistance>> pq;
        
        for (const auto& pair : vertexMap) {
            distance[pair.first] = numeric_limits<int>::max();
        }
        distance[start] = 0;
        
        pq.push(NodeDistance(start, 0));
        
        while (!pq.empty()) {
            NodeDistance current = pq.top();
            pq.pop();
            
            string u = current.airport;
            int dist = current.distance;
            
            if (u == end) {
                break;
            }
            
            if (dist > distance[u]) {
                continue;
            }
            
            for (const Route& route : adjacencyList[u]) {
                string v = route.destination;
                int alt = distance[u] + route.distance;
                
                if (alt < distance[v]) {
                    distance[v] = alt;
                    parent[v] = u;
                    pq.push(NodeDistance(v, alt));
                }
            }
        }
        
        if (distance[end] == numeric_limits<int>::max()) {
            result.found = false;
            return result;
        }
        
        result.path = reconstructPath(parent, start, end);
        result.stops = result.path.size() - 1;
        result.totalDistance = distance[end];
        result.found = true;
        
        return result;
    }
    
    // ==================== FITUR BARU: COMPARE ALGORITHMS ====================
    void comparePathFindingAlgorithms(const string& start, const string& end) {
        cout << "\n" << string(95, '=') << endl;
        cout << "           PERBANDINGAN ALGORITMA: BFS vs DFS vs DIJKSTRA" << endl;
        cout << string(95, '=') << endl;
        cout << "Dari: " << start << " (" << vertexMap[start].name << ")" << endl;
        cout << "Ke  : " << end << " (" << vertexMap[end].name << ")" << endl;
        cout << string(95, '=') << endl;
        
        // Jalankan semua algoritma
        PathResult bfsResult = findPathBFS(start, end);
        PathResult dfsResult = findPathDFS(start, end);
        PathResult dijkstraResult = findShortestPath(start, end);
        
        // Header tabel
        cout << "\n" << left 
             << setw(12) << "ALGORITMA"
             << setw(10) << "STATUS"
             << setw(10) << "STOPS"
             << setw(18) << "TOTAL JARAK"
             << setw(45) << "PATH" << endl;
        cout << string(95, '-') << endl;
        
        // BFS Result
        cout << left << setw(12) << "BFS" 
             << setw(10) << (bfsResult.found ? "✓ Found" : "✗ Failed");
        if (bfsResult.found) {
            cout << setw(10) << bfsResult.stops
                 << setw(18) << (to_string(bfsResult.totalDistance) + " km");
            for (size_t i = 0; i < bfsResult.path.size(); i++) {
                cout << bfsResult.path[i];
                if (i < bfsResult.path.size() - 1) cout << " → ";
            }
        }
        cout << endl;
        
        // DFS Result
        cout << left << setw(12) << "DFS"
             << setw(10) << (dfsResult.found ? "✓ Found" : "✗ Failed");
        if (dfsResult.found) {
            cout << setw(10) << dfsResult.stops
                 << setw(18) << (to_string(dfsResult.totalDistance) + " km");
            for (size_t i = 0; i < dfsResult.path.size(); i++) {
                cout << dfsResult.path[i];
                if (i < dfsResult.path.size() - 1) cout << " → ";
            }
        }
        cout << endl;
        
        // Dijkstra Result
        cout << left << setw(12) << "Dijkstra"
             << setw(10) << (dijkstraResult.found ? "✓ Found" : "✗ Failed");
        if (dijkstraResult.found) {
            cout << setw(10) << dijkstraResult.stops
                 << setw(18) << (to_string(dijkstraResult.totalDistance) + " km");
            for (size_t i = 0; i < dijkstraResult.path.size(); i++) {
                cout << dijkstraResult.path[i];
                if (i < dijkstraResult.path.size() - 1) cout << " → ";
            }
        }
        cout << endl;
        cout << string(95, '-') << endl;
        
        // Analisis Detail
        cout << "\n📊 ANALISIS DETAIL:" << endl;
        
        if (bfsResult.found) {
            cout << "\n1️⃣  BFS (Breadth-First Search):" << endl;
            cout << "   • Strategi: Menjelajah LEVEL DEMI LEVEL (seperti riak air)" << endl;
            cout << "   • Stops: " << bfsResult.stops << " transit" << endl;
            cout << "   • Total Jarak: " << bfsResult.totalDistance << " km" << endl;
            cout << "   • ✓ MENJAMIN jumlah stops MINIMUM!" << endl;
        }
        
        if (dfsResult.found) {
            cout << "\n2️⃣  DFS (Depth-First Search):" << endl;
            cout << "   • Strategi: Menyelam SEDALAM mungkin (seperti maze solver)" << endl;
            cout << "   • Stops: " << dfsResult.stops << " transit" << endl;
            cout << "   • Total Jarak: " << dfsResult.totalDistance << " km" << endl;
            
            if (dfsResult.stops > bfsResult.stops) {
                cout << "   • ✗ SUB-OPTIMAL: " << (dfsResult.stops - bfsResult.stops) 
                     << " stops LEBIH BANYAK dari BFS!" << endl;
                cout << "   • ⚠️  DFS TIDAK menjamin path terpendek!" << endl;
            } else if (dfsResult.stops == bfsResult.stops) {
                cout << "   • ✓ Sama optimal dengan BFS (kebetulan)" << endl;
                if (dfsResult.totalDistance > bfsResult.totalDistance) {
                    cout << "   • ⚠️  Tapi jarak " << (dfsResult.totalDistance - bfsResult.totalDistance)
                         << " km LEBIH JAUH!" << endl;
                }
            }
        }
        
            if (dijkstraResult.found) {
                cout << "\n3️⃣  Dijkstra (Shortest Path by Distance):" << endl;
                cout << "   • Strategi: Minimalisir TOTAL JARAK" << endl;
                cout << "   • Stops: " << dijkstraResult.stops << " transit" << endl;
                cout << "   • Total Jarak: " << dijkstraResult.totalDistance << " km" << endl;
                cout << "   • ✓ MENJAMIN jarak total MINIMUM!" << endl;
                
                // Improved time estimation
                int flightTime = dijkstraResult.totalDistance / 800; // avg 800 km/h
                int transitTime = dijkstraResult.stops * 2; // assume 2 hours per transit
                int totalTime = flightTime + transitTime;
                
                cout << "   • ✈ Estimasi waktu terbang: ~" << flightTime << " jam" << endl;
                cout << "   • ⏱ Estimasi waktu transit: ~" << transitTime << " jam (" 
                     << dijkstraResult.stops << " stops × 2 jam)" << endl;
                cout << "   • 🕐 Total estimasi: ~" << totalTime << " jam" << endl;
                
                if (dijkstraResult.totalDistance < bfsResult.totalDistance) {
                    cout << "   • 💡 Path berbeda dari BFS (prioritas JARAK, bukan stops)" << endl;
                }
            }
        
        // Kesimpulan
        cout << "\n" << string(95, '=') << endl;
        cout << "🏆 KESIMPULAN:" << endl;
        
        if (bfsResult.found && dfsResult.found) {
            if (bfsResult.stops < dfsResult.stops) {
                cout << "   • ✅ BFS LEBIH OPTIMAL - Lebih sedikit " << (dfsResult.stops - bfsResult.stops) 
                     << " stops" << endl;
                cout << "   • ❌ DFS sub-optimal untuk route planning" << endl;
            } else if (bfsResult.stops == dfsResult.stops) {
                cout << "   • 🤝 BFS dan DFS sama optimal (jumlah stops sama)" << endl;
                cout << "   • ⚠️  Tapi BFS DIJAMIN optimal, DFS tidak" << endl;
                
                if (abs(bfsResult.totalDistance - dfsResult.totalDistance) > 100) {
                    cout << "   • 💡 Path yang diambil berbeda (selisih jarak: " 
                         << abs(bfsResult.totalDistance - dfsResult.totalDistance) << " km)" << endl;
                }
            }
            
            if (dijkstraResult.found) {
                cout << "   • 🎯 Dijkstra optimal untuk MINIMALISIR JARAK (fuel efficiency)" << endl;
            }
        }
        
        cout << "\n📚 REKOMENDASI PENGGUNAAN:" << endl;
        cout << "   • BFS      : Minimalisir transit ✈️  (terbaik untuk penumpang)" << endl;
        cout << "   • DFS      : Hanya cek konektivitas (TIDAK untuk route planning)" << endl;
        cout << "   • Dijkstra : Minimalisir jarak/biaya ⛽ (terbaik untuk maskapai)" << endl;
        cout << string(95, '=') << endl;
    }
    
    // ==================== FUNGSI TRAVERSAL ====================
    
    vector<string> traverse(const string& startAirport, const string& method) {
        if (vertexMap.find(startAirport) == vertexMap.end()) {
            return vector<string>();
        }
        
        if (method == "BFS") {
            return traverseBFS(startAirport);
        } else if (method == "DFS") {
            return traverseDFS(startAirport);
        }
        
        return vector<string>();
    }
    
    vector<string> traverseBFS(const string& start) {
        vector<string> result;
        unordered_map<string, bool> visited;
        queue<string> q;
        
        q.push(start);
        visited[start] = true;
        
        while (!q.empty()) {
            string current = q.front();
            q.pop();
            result.push_back(current);
            
            for (const Route& route : adjacencyList[current]) {
                if (!visited[route.destination]) {
                    visited[route.destination] = true;
                    q.push(route.destination);
                }
            }
        }
        
        return result;
    }
    
    vector<string> traverseDFS(const string& start) {
        vector<string> result;
        unordered_map<string, bool> visited;
        stack<string> s;
        
        s.push(start);
        visited[start] = true;
        
        while (!s.empty()) {
            string current = s.top();
            s.pop();
            result.push_back(current);
            
            for (const Route& route : adjacencyList[current]) {
                if (!visited[route.destination]) {
                    visited[route.destination] = true;
                    s.push(route.destination);
                }
            }
        }
        
        return result;
    }
    
    // ==================== FUNGSI TAMBAHAN ====================
    
    // Helper untuk validasi airport code input
    bool validateAirportInput(const string& code, const string& label) {
        if (vertexMap.find(code) == vertexMap.end()) {
            cout << "\n✗ " << label << " '" << code << "' tidak ditemukan dalam database!" << endl;
            cout << "💡 Tip: Gunakan kode IATA 3 huruf (contoh: CGK, SIN, DPS)" << endl;
            return false;
        }
        return true;
    }
    
    // Load airports dari airports.csv
    int loadAirports(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Error: Tidak dapat membuka file " << filename << endl;
            return 0;
        }
        
        string line;
        int count = 0;
        
        // Skip header
        getline(file, line);
        
        while (getline(file, line)) {
            vector<string> fields = parseCSVLine(line);
            
            if (fields.size() < 8) continue;  // Need at least 8 fields
            
            try {
                int airportID = stoi(fields[0]);
                string name = fields[1];
                string city = fields[2];
                string country = fields[3];
                string iata = fields[4];
                double latitude = stod(fields[6]);
                double longitude = stod(fields[7]);
                
                // Only add airports with valid IATA code
                if (iata != "\\N" && iata.length() == 3) {
                    addVertex(iata, name, airportID, city, country, latitude, longitude);
                    count++;
                }
            } catch (...) {
                // Skip invalid lines
                continue;
            }
        }
        
        file.close();
        return count;
    }
    
    // Load routes dari routes.csv
    int loadRoutes(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Error: Tidak dapat membuka file " << filename << endl;
            return 0;
        }
        
        string line;
        int count = 0;
        int skipped = 0;
        
        while (getline(file, line)) {
            vector<string> fields = parseCSVLine(line);
            
            if (fields.size() < 9) continue;
            
            string airline = fields[0];
            string sourceIATA = fields[2];
            string destIATA = fields[4];
            
            // Check if both airports exist in our graph
            if (vertexMap.find(sourceIATA) == vertexMap.end() || 
                vertexMap.find(destIATA) == vertexMap.end()) {
                skipped++;
                continue;
            }
            
            // Calculate distance using Haversine formula
            Airport& src = vertexMap[sourceIATA];
            Airport& dst = vertexMap[destIATA];
            
            int distance = (int)calculateDistance(src.latitude, src.longitude, 
                                                   dst.latitude, dst.longitude);
            
            addEdge(sourceIATA, destIATA, distance, airline);
            count++;
        }
        
        file.close();
        
        if (skipped > 0) {
            cout << "Info: " << skipped << " rute dilewati (bandara tidak ada dalam database)" << endl;
        }
        
        return count;
    }
    
    // Load both files - convenience function
    pair<int, int> loadDataset(const string& airportsFile, const string& routesFile) {
        cout << "Loading airports dari " << airportsFile << "..." << endl;
        int airportCount = loadAirports(airportsFile);
        
        if (airportCount == 0) {
            return {0, 0};
        }
        
        cout << "✓ " << airportCount << " bandara berhasil dimuat" << endl;
        cout << "\nLoading routes dari " << routesFile << "..." << endl;
        int routeCount = loadRoutes(routesFile);
        
        return {airportCount, routeCount};
    }
    
    Airport getAirportInfo(const string& airportCode) {
        if (vertexMap.find(airportCode) != vertexMap.end()) {
            return vertexMap[airportCode];
        }
        return Airport();
    }
    
    vector<vector<string>> findAllPaths(const string& start, const string& end, int maxStops) {
        vector<vector<string>> allPaths;
        vector<string> currentPath;
        set<string> visited;
        
        currentPath.push_back(start);
        visited.insert(start);
        
        findAllPathsDFS(start, end, maxStops, 0, currentPath, visited, allPaths);
        
        return allPaths;
    }
    
    void findAllPathsDFS(const string& current, const string& end, int maxStops, 
                        int currentStops, vector<string>& path, 
                        set<string>& visited, vector<vector<string>>& allPaths) {
        if (current == end) {
            allPaths.push_back(path);
            return;
        }
        
        if (currentStops >= maxStops) {
            return;
        }
        
        for (const Route& route : adjacencyList[current]) {
            if (visited.find(route.destination) == visited.end()) {
                path.push_back(route.destination);
                visited.insert(route.destination);
                
                findAllPathsDFS(route.destination, end, maxStops, currentStops + 1, 
                              path, visited, allPaths);
                
                path.pop_back();
                visited.erase(route.destination);
            }
        }
    }
    
    Statistics getStatistics() {
        Statistics stats;
        stats.totalAirports = vertexMap.size();
        stats.totalRoutes = 0;
        stats.maxDegree = 0;
        stats.minDegree = numeric_limits<int>::max();
        
        for (const auto& pair : adjacencyList) {
            int degree = pair.second.size();
            stats.totalRoutes += degree;
            
            if (degree > stats.maxDegree) {
                stats.maxDegree = degree;
                stats.maxDegreeAirport = pair.first;
            }
            if (degree < stats.minDegree && degree > 0) {
                stats.minDegree = degree;
                stats.minDegreeAirport = pair.first;
            }
        }
        
        stats.avgDegree = stats.totalAirports > 0 ? 
                         (double)stats.totalRoutes / stats.totalAirports : 0.0;
        
        return stats;
    }
    
    // ==================== DISPLAY FUNCTIONS ====================
    
    void displayNeighbors(const string& airportCode) {
        vector<Route> neighbors = getNeighbors(airportCode);
        
        if (neighbors.empty()) {
            cout << "Tidak ada rute langsung dari " << airportCode << endl;
            return;
        }
        
        // Get source airport info
        Airport srcInfo = vertexMap[airportCode];
        
        cout << "\nRute langsung dari " << airportCode << " (" << srcInfo.name << "):" << endl;
        cout << string(90, '=') << endl;
        cout << left << setw(8) << "Tujuan" 
             << setw(35) << "Nama Bandara"
             << setw(12) << "Jarak (km)" 
             << setw(10) << "Maskapai" << endl;
        cout << string(90, '=') << endl;
        
        for (const Route& route : neighbors) {
            Airport destInfo = vertexMap[route.destination];
            cout << left << setw(8) << route.destination
                 << setw(35) << destInfo.name.substr(0, 33)  // Truncate if too long
                 << setw(12) << route.distance
                 << setw(10) << route.airline << endl;
        }
        cout << "\nTotal rute langsung: " << neighbors.size() << endl;
    }
    
    void displayPath(const vector<string>& path) {
        if (path.empty()) {
            cout << "Path tidak ditemukan!" << endl;
            return;
        }
        
        cout << "\nPath: ";
        for (size_t i = 0; i < path.size(); i++) {
            cout << path[i];
            if (i < path.size() - 1) {
                cout << " -> ";
            }
        }
        cout << endl;
        cout << "Jumlah stops: " << (path.size() - 1) << endl;
    }
    
    void displayStatistics() {
        Statistics stats = getStatistics();
        
        cout << "\n========== STATISTIK GRAPH ==========" << endl;
        cout << "Total Bandara: " << stats.totalAirports << endl;
        cout << "Total Rute: " << stats.totalRoutes << endl;
        cout << "Rata-rata Degree: " << fixed << setprecision(2) 
             << stats.avgDegree << endl;
        cout << "Max Degree: " << stats.maxDegree 
             << " (" << stats.maxDegreeAirport << ")" << endl;
        cout << "Min Degree: " << stats.minDegree 
             << " (" << stats.minDegreeAirport << ")" << endl;
        cout << "=====================================" << endl;
    }
};

// ==================== HELPER FUNCTIONS ====================

// Helper function untuk convert string ke uppercase
// ==================== HELPER FUNCTIONS ====================

// Haversine formula untuk menghitung jarak antara dua koordinat
double calculateDistance(double lat1, double lon1, double lat2, double lon2) {
    const double R = 6371.0; // Earth radius in kilometers
    
    // Convert degrees to radians
    double lat1_rad = lat1 * M_PI / 180.0;
    double lon1_rad = lon1 * M_PI / 180.0;
    double lat2_rad = lat2 * M_PI / 180.0;
    double lon2_rad = lon2 * M_PI / 180.0;
    
    // Haversine formula
    double dlat = lat2_rad - lat1_rad;
    double dlon = lon2_rad - lon1_rad;
    
    double a = sin(dlat/2) * sin(dlat/2) + 
               cos(lat1_rad) * cos(lat2_rad) * 
               sin(dlon/2) * sin(dlon/2);
    double c = 2 * atan2(sqrt(a), sqrt(1-a));
    double distance = R * c;
    
    return distance;
}

// Parse CSV line dengan handling untuk quoted fields
vector<string> parseCSVLine(const string& line) {
    vector<string> fields;
    string field;
    bool inQuotes = false;
    
    for (size_t i = 0; i < line.length(); i++) {
        char c = line[i];
        
        if (c == '"') {
            inQuotes = !inQuotes;
        } else if (c == ',' && !inQuotes) {
            fields.push_back(field);
            field.clear();
        } else if (c != '\r' && c != '\n') {  // Skip CR and LF
            field += c;
        }
    }
    fields.push_back(field);  // Add last field
    
    return fields;
}

string toUpperCase(string str) {
    transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
}

// Helper function untuk trim whitespace
string trim(const string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

// Helper function untuk validasi airport code (3 huruf)
bool isValidAirportCode(const string& code) {
    if (code.length() != 3) return false;
    for (char c : code) {
        if (!isalpha(c)) return false;
    }
    return true;
}

// ==================== MENU SYSTEM ====================

void displayMenu() {
    cout << "\n========================================" << endl;
    cout << "   SISTEM PENCARIAN RUTE PENERBANGAN" << endl;
    cout << "========================================" << endl;
    cout << "1. Load data dari CSV" << endl;
    cout << "2. Tampilkan rute langsung dari bandara" << endl;
    cout << "3. Info bandara" << endl;
    cout << "4. Cek bandara exist (hasVertex)" << endl;
    cout << "5. Cek rute exist (hasEdge)" << endl;
    cout << "6. Hitung indegree bandara" << endl;
    cout << "7. Hitung outdegree bandara" << endl;
    cout << "8. Cari path (BFS)" << endl;
    cout << "9. Cari path (DFS)" << endl;
    cout << "10. Cari shortest path (Dijkstra)" << endl;
    cout << "11. Bandingkan BFS vs DFS vs Dijkstra" << endl;
    cout << "12. Traverse dari bandara (BFS)" << endl;
    cout << "13. Traverse dari bandara (DFS)" << endl;
    cout << "14. Cari semua path dengan max stops" << endl;
    cout << "15. Tampilkan statistik graph" << endl;
    cout << "16. Hapus rute (removeEdge)" << endl;
    cout << "0. Keluar" << endl;
    cout << "========================================" << endl;
    cout << "Pilihan: ";
}

int main() {
    FlightRouteGraph graph;
    int choice;
    bool dataLoaded = false;
    
    cout << "========================================" << endl;
    cout << "           DIRECTED GRAPH" << endl;
    cout << "   Sistem Pencarian Rute Penerbangan" << endl;
    cout << "========================================" << endl;
    
    while (true) {
        displayMenu();
        
        // Improved input handling untuk angka
        string input;
        getline(cin, input);
        
        // Trim whitespace
        input = trim(input);
        
        // Check if empty
        if (input.empty()) {
            cout << "\n⚠️  Input tidak boleh kosong!" << endl;
            cout << "\nTekan Enter untuk melanjutkan...";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }
        
        // Check if all characters are digits (allow negative for exit)
        bool isNumber = true;
        for (size_t i = 0; i < input.length(); i++) {
            if (!isdigit(input[i]) && !(i == 0 && input[i] == '-')) {
                isNumber = false;
                break;
            }
        }
        
        if (!isNumber) {
            cout << "\n❌ Input tidak valid!" << endl;
            cout << "⚠️  Masukkan pilihan angka yang sesuai dengan menu yang tersedia (0-16)." << endl;
            cout << "\nTekan Enter untuk melanjutkan...";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }
        
        // Convert to integer
        try {
            choice = stoi(input);
        } catch (...) {
            cout << "\n❌ Input tidak valid!" << endl;
            cout << "⚠️  Masukkan pilihan angka yang sesuai dengan menu yang tersedia (0-16)." << endl;
            cout << "\nTekan Enter untuk melanjutkan...";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }
        
        // Validate range
        if (choice < 0 || choice > 16) {
            cout << "\n❌ Pilihan tidak valid!" << endl;
            cout << "⚠️  Pilihan harus antara 0-16. Silakan pilih menu yang tersedia." << endl;
            cout << "\nTekan Enter untuk melanjutkan...";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }
        
        switch (choice) {
            case 1: {
                string airportsFile, routesFile;
                
                cout << "=== LOAD DATASET ===" << endl;
                cout << "Masukkan nama file airports [assets/airports.csv]: ";
                getline(cin, airportsFile);
                airportsFile = trim(airportsFile);
                if (airportsFile.empty()) {
                    airportsFile = "assets/airports.csv";
                }
                
                cout << "Masukkan nama file routes [assets/routes.csv]: ";
                getline(cin, routesFile);
                routesFile = trim(routesFile);
                if (routesFile.empty()) {
                    routesFile = "assets/routes.csv";
                }
                
                // Check if files exist
                ifstream testAirports(airportsFile);
                ifstream testRoutes(routesFile);
                
                if (!testAirports.good()) {
                    cout << "\n❌ File '" << airportsFile << "' tidak ditemukan!" << endl;
                    cout << "💡 Pastikan file airports.csv ada di direktori yang sama." << endl;
                    testAirports.close();
                    break;
                }
                
                if (!testRoutes.good()) {
                    cout << "\n❌ File '" << routesFile << "' tidak ditemukan!" << endl;
                    cout << "💡 Pastikan file routes.csv ada di direktori yang sama." << endl;
                    testRoutes.close();
                    testAirports.close();
                    break;
                }
                
                testAirports.close();
                testRoutes.close();
                
                cout << "\n" << string(60, '=') << endl;
                auto result = graph.loadDataset(airportsFile, routesFile);
                cout << string(60, '=') << endl;
                
                if (result.first > 0 && result.second > 0) {
                    cout << "\n✅ DATA BERHASIL DIMUAT!" << endl;
                    cout << "   Airports: " << result.first << " bandara" << endl;
                    cout << "   Routes:   " << result.second << " rute" << endl;
                    dataLoaded = true;
                } else {
                    cout << "\n✗ Gagal memuat data!" << endl;
                }
                break;
            }
            
            case 2: {
                if (!dataLoaded) {
                    cout << "\n✗ Silakan load data terlebih dahulu!" << endl;
                    break;
                }
                string airport;
                cout << "Kode bandara: ";
                getline(cin, airport);
                
                // Trim and validate
                airport = trim(airport);
                if (airport.empty()) {
                    cout << "\n✗ Input tidak boleh kosong!" << endl;
                    break;
                }
                
                // Convert to uppercase
                airport = toUpperCase(airport);
                
                // Validate format
                if (!isValidAirportCode(airport)) {
                    cout << "\n✗ Kode bandara harus 3 huruf! Contoh: CGK, SIN, DPS" << endl;
                    break;
                }
                
                // Check if exists menggunakan fungsi class
                if (!graph.validateAirportInput(airport, "Bandara")) break;
                
                graph.displayNeighbors(airport);
                break;
            }

            case 3: {
                if (!dataLoaded) {
                    cout << "\n✗ Silakan load data terlebih dahulu!" << endl;
                    break;
                }
                string code;
                cout << "Kode bandara: ";
                getline(cin, code);
                
                code = toUpperCase(trim(code));
                if (code.empty()) {
                    cout << "\n✗ Input tidak boleh kosong!" << endl;
                    break;
                }
                if (!isValidAirportCode(code)) {
                    cout << "\n✗ Kode bandara harus 3 huruf!" << endl;
                    break;
                }
                
                Airport info = graph.getAirportInfo(code);
                if (info.id != -1) {
                    cout << "\nInformasi Bandara:" << endl;
                    cout << "Kode: " << info.code << endl;
                    cout << "Nama: " << info.name << endl;
                    cout << "ID: " << info.id << endl;
                    
                    // Tambahan info
                    vector<Route> neighbors = graph.getNeighbors(code);
                    cout << "Jumlah rute langsung: " << neighbors.size() << endl;
                } else {
                    cout << "\n✗ Bandara tidak ditemukan!" << endl;
                }
                break;
            }

            case 4: {// hasVertex
                string code;
                cout << "\n=== CEK BANDARA EXIST ===" << endl;
                cout << "Kode bandara: ";
                getline(cin, code);
                
                code = toUpperCase(trim(code));
                if (code.empty()) {
                    cout << "\n✗ Input tidak boleh kosong!" << endl;
                    break;
                }
                if (!isValidAirportCode(code)) {
                    cout << "\n✗ Kode bandara harus 3 huruf!" << endl;
                    break;
                }
                
                if (graph.hasVertex(code)) {
                    cout << "\n✓ Bandara " << code << " ADA dalam database!" << endl;
                    Airport info = graph.getAirportInfo(code);
                    cout << "   Nama: " << info.name << endl;
                    cout << "   ID: " << info.id << endl;
                } else {
                    cout << "\n✗ Bandara " << code << " TIDAK ADA dalam database!" << endl;
                }
                break;
            }
            
            case 5: {  // hasEdge
                if (!dataLoaded) {
                    cout << "\n✗ Silakan load data terlebih dahulu!" << endl;
                    break;
                }
                
                string from, to;
                cout << "\n=== CEK RUTE EXIST ===" << endl;
                cout << "Bandara asal: ";
                getline(cin, from);
                cout << "Bandara tujuan: ";
                getline(cin, to);
                
                from = toUpperCase(trim(from));
                to = toUpperCase(trim(to));
                
                if (from.empty() || to.empty()) {
                    cout << "\n✗ Input tidak boleh kosong!" << endl;
                    break;
                }
                if (!isValidAirportCode(from) || !isValidAirportCode(to)) {
                    cout << "\n✗ Kode bandara harus 3 huruf!" << endl;
                    break;
                }
                
                if (graph.hasEdge(from, to)) {
                    cout << "\n✓ Rute " << from << " → " << to << " ADA!" << endl;
                    
                    // Show details
                    vector<Route> routes = graph.getNeighbors(from);
                    cout << "\nDetail rute:" << endl;
                    cout << "Tujuan\t\tJarak\t\tMaskapai" << endl;
                    cout << "================================================" << endl;
                    for (const Route& r : routes) {
                        if (r.destination == to) {
                            cout << r.destination << "\t\t" 
                                 << r.distance << " km\t\t" 
                                 << r.airline << endl;
                        }
                    }
                } else {
                    cout << "\n✗ Rute " << from << " → " << to << " TIDAK ADA!" << endl;
                    cout << "💡 Tip: Gunakan Menu 2 untuk melihat semua rute dari " << from << endl;
                }
                break;
            }

            case 6: { // indegree
                if (!dataLoaded) {
                    cout << "\n✗ Silakan load data terlebih dahulu!" << endl;
                    break;
                }
                
                string code;
                cout << "\n=== HITUNG INDEGREE ===" << endl;
                cout << "Kode bandara: ";
                getline(cin, code);
                
                code = toUpperCase(trim(code));
                if (code.empty()) {
                    cout << "\n✗ Input tidak boleh kosong!" << endl;
                    break;
                }
                if (!isValidAirportCode(code)) {
                    cout << "\n✗ Kode bandara harus 3 huruf!" << endl;
                    break;
                }
                if (!graph.validateAirportInput(code, "Bandara")) break;
                
                int inDeg = graph.indegree(code);
                if (inDeg >= 0) {
                    cout << "\n📥 INDEGREE " << code << ": " << inDeg << " rute" << endl;
                    cout << "   → Jumlah penerbangan yang MENUJU ke " << code << endl;
                    
                    if (inDeg == 0) {
                        cout << "\n⚠️  Tidak ada penerbangan yang menuju " << code << "!" << endl;
                        cout << "   (Source node / isolated node)" << endl;
                    } else if (inDeg > 50) {
                        cout << "\n🌟 " << code << " adalah bandara POPULER (banyak penerbangan masuk)!" << endl;
                    }
                }
                break;
            }
            
            case 7: {  // outdegree
                if (!dataLoaded) {
                    cout << "\n✗ Silakan load data terlebih dahulu!" << endl;
                    break;
                }
                
                string code;
                cout << "\n=== HITUNG OUTDEGREE ===" << endl;
                cout << "Kode bandara: ";
                getline(cin, code);
                
                code = toUpperCase(trim(code));
                if (code.empty()) {
                    cout << "\n✗ Input tidak boleh kosong!" << endl;
                    break;
                }
                if (!isValidAirportCode(code)) {
                    cout << "\n✗ Kode bandara harus 3 huruf!" << endl;
                    break;
                }
                if (!graph.validateAirportInput(code, "Bandara")) break;
                
                int outDeg = graph.outdegree(code);
                if (outDeg >= 0) {
                    cout << "\n📤 OUTDEGREE " << code << ": " << outDeg << " rute" << endl;
                    cout << "   → Jumlah penerbangan yang BERANGKAT dari " << code << endl;
                    
                    if (outDeg == 0) {
                        cout << "\n⚠️  Tidak ada penerbangan yang berangkat dari " << code << "!" << endl;
                        cout << "   (Sink node / destination only)" << endl;
                    } else if (outDeg > 50) {
                        cout << "\n🌟 " << code << " adalah HUB besar (banyak tujuan)!" << endl;
                    }
                    
                    // Show indegree juga untuk comparison
                    int inDeg = graph.indegree(code);
                    cout << "\n📊 Comparison:" << endl;
                    cout << "   Indegree:  " << inDeg << " (incoming)" << endl;
                    cout << "   Outdegree: " << outDeg << " (outgoing)" << endl;
                    cout << "   Total degree: " << (inDeg + outDeg) << endl;
                }
                break;
            }
            
            case 8: {
                if (!dataLoaded) {
                    cout << "\n✗ Silakan load data terlebih dahulu!" << endl;
                    break;
                }
                string start, end;
                cout << "Bandara asal: ";
                getline(cin, start);
                cout << "Bandara tujuan: ";
                getline(cin, end);
                
                // Trim and validate
                start = trim(start);
                end = trim(end);
                
                if (start.empty() || end.empty()) {
                    cout << "\n✗ Input tidak boleh kosong!" << endl;
                    break;
                }
                
                // Convert to uppercase
                start = toUpperCase(start);
                end = toUpperCase(end);
                
                // Validate format
                if (!isValidAirportCode(start) || !isValidAirportCode(end)) {
                    cout << "\n✗ Kode bandara harus 3 huruf! Contoh: CGK, SIN, DPS" << endl;
                    break;
                }
                
                // Check if exists using class function
                if (!graph.validateAirportInput(start, "Bandara asal")) break;
                if (!graph.validateAirportInput(end, "Bandara tujuan")) break;
                
                PathResult result = graph.findPathBFS(start, end);
                cout << "\n=== HASIL PENCARIAN BFS ===" << endl;
                cout << "Algoritma: " << result.algorithm << endl;
                if (result.found) {
                    graph.displayPath(result.path);
                    cout << "Total jarak: " << result.totalDistance << " km" << endl;
                } else {
                    cout << "Path tidak ditemukan!" << endl;
                }
                break;
            }
            
            case 9: {
                if (!dataLoaded) {
                    cout << "\n✗ Silakan load data terlebih dahulu!" << endl;
                    break;
                }
                string start, end;
                cout << "Bandara asal: ";
                getline(cin, start);
                cout << "Bandara tujuan: ";
                getline(cin, end);
                
                start = toUpperCase(trim(start));
                end = toUpperCase(trim(end));
                
                if (start.empty() || end.empty()) {
                    cout << "\n✗ Input tidak boleh kosong!" << endl;
                    break;
                }
                if (!isValidAirportCode(start) || !isValidAirportCode(end)) {
                    cout << "\n✗ Kode bandara harus 3 huruf!" << endl;
                    break;
                }
                if (!graph.validateAirportInput(start, "Bandara asal")) break;
                if (!graph.validateAirportInput(end, "Bandara tujuan")) break;
                
                PathResult result = graph.findPathDFS(start, end);
                cout << "\n=== HASIL PENCARIAN DFS ===" << endl;
                cout << "Algoritma: " << result.algorithm << endl;
                if (result.found) {
                    graph.displayPath(result.path);
                    cout << "Total jarak: " << result.totalDistance << " km" << endl;
                } else {
                    cout << "Path tidak ditemukan!" << endl;
                }
                break;
            }
            
            case 10: {
                if (!dataLoaded) {
                    cout << "\n✗ Silakan load data terlebih dahulu!" << endl;
                    break;
                }
                string start, end;
                cout << "Bandara asal: ";
                getline(cin, start);
                cout << "Bandara tujuan: ";
                getline(cin, end);
                
                start = toUpperCase(trim(start));
                end = toUpperCase(trim(end));
                
                if (start.empty() || end.empty()) {
                    cout << "\n✗ Input tidak boleh kosong!" << endl;
                    break;
                }
                if (!isValidAirportCode(start) || !isValidAirportCode(end)) {
                    cout << "\n✗ Kode bandara harus 3 huruf!" << endl;
                    break;
                }
                if (!graph.validateAirportInput(start, "Bandara asal")) break;
                if (!graph.validateAirportInput(end, "Bandara tujuan")) break;
                
                PathResult result = graph.findShortestPath(start, end);
                cout << "\n=== HASIL PENCARIAN DIJKSTRA ===" << endl;
                cout << "Algoritma: " << result.algorithm << endl;
                if (result.found) {
                    graph.displayPath(result.path);
                    cout << "Total jarak: " << result.totalDistance << " km" << endl;
                    
                    int flightTime = result.totalDistance / 800;
                    int transitTime = result.stops * 2;
                    cout << "Estimasi waktu terbang: ~" << flightTime << " jam" << endl;
                    cout << "Estimasi waktu transit: ~" << transitTime << " jam" << endl;
                    cout << "Total estimasi: ~" << (flightTime + transitTime) << " jam" << endl;
                } else {
                    cout << "Path tidak ditemukan!" << endl;
                }
                break;
            }
            
            case 11: {
                if (!dataLoaded) {
                    cout << "\n✗ Silakan load data terlebih dahulu!" << endl;
                    break;
                }
                string start, end;
                cout << "\n=== PERBANDINGAN ALGORITMA ===" << endl;
                cout << "Bandara asal: ";
                getline(cin, start);
                cout << "Bandara tujuan: ";
                getline(cin, end);
                
                start = toUpperCase(trim(start));
                end = toUpperCase(trim(end));
                
                if (start.empty() || end.empty()) {
                    cout << "\n✗ Input tidak boleh kosong!" << endl;
                    break;
                }
                if (!isValidAirportCode(start) || !isValidAirportCode(end)) {
                    cout << "\n✗ Kode bandara harus 3 huruf!" << endl;
                    break;
                }
                if (!graph.validateAirportInput(start, "Bandara asal")) break;
                if (!graph.validateAirportInput(end, "Bandara tujuan")) break;
                
                graph.comparePathFindingAlgorithms(start, end);
                break;
            }
            
            case 12: {  
                if (!dataLoaded) {
                    cout << "\n✗ Silakan load data terlebih dahulu!" << endl;
                    break;
                }
                string start;
                cout << "Bandara awal: ";
                getline(cin, start);
                
                start = toUpperCase(trim(start));
                if (start.empty()) {
                    cout << "\n✗ Input tidak boleh kosong!" << endl;
                    break;
                }
                if (!isValidAirportCode(start)) {
                    cout << "\n✗ Kode bandara harus 3 huruf!" << endl;
                    break;
                }
                if (!graph.validateAirportInput(start, "Bandara")) break;
                
                vector<string> traversal = graph.traverse(start, "BFS");
                cout << "\nBFS Traversal dari " << start << ":" << endl;
                cout << "Bandara yang dapat dijangkau: " << traversal.size() << endl;
                cout << "Urutan: ";
                for (size_t i = 0; i < min(traversal.size(), size_t(20)); i++) {
                    cout << traversal[i];
                    if (i < min(traversal.size(), size_t(20)) - 1) cout << ", ";
                }
                if (traversal.size() > 20) cout << " ...";
                cout << endl;
                break;
            }
            
            case 13: {  
                if (!dataLoaded) {
                    cout << "\n✗ Silakan load data terlebih dahulu!" << endl;
                    break;
                }
                string start;
                cout << "Bandara awal: ";
                getline(cin, start);
                
                start = toUpperCase(trim(start));
                if (start.empty()) {
                    cout << "\n✗ Input tidak boleh kosong!" << endl;
                    break;
                }
                if (!isValidAirportCode(start)) {
                    cout << "\n✗ Kode bandara harus 3 huruf!" << endl;
                    break;
                }
                if (!graph.validateAirportInput(start, "Bandara")) break;
                
                vector<string> traversal = graph.traverse(start, "DFS");
                cout << "\nDFS Traversal dari " << start << ":" << endl;
                cout << "Bandara yang dapat dijangkau: " << traversal.size() << endl;
                cout << "Urutan: ";
                for (size_t i = 0; i < min(traversal.size(), size_t(20)); i++) {
                    cout << traversal[i];
                    if (i < min(traversal.size(), size_t(20)) - 1) cout << ", ";
                }
                if (traversal.size() > 20) cout << " ...";
                cout << endl;
                break;
            }
            
            case 14: {  
                if (!dataLoaded) {
                    cout << "\n✗ Silakan load data terlebih dahulu!" << endl;
                    break;
                }
                string start, end, maxStopsStr;
                int maxStops;
                
                cout << "Bandara asal: ";
                getline(cin, start);
                cout << "Bandara tujuan: ";
                getline(cin, end);
                cout << "Maksimal stops: ";
                getline(cin, maxStopsStr);
                
                start = toUpperCase(trim(start));
                end = toUpperCase(trim(end));
                maxStopsStr = trim(maxStopsStr);
                
                if (start.empty() || end.empty()) {
                    cout << "\n✗ Input tidak boleh kosong!" << endl;
                    break;
                }
                if (!isValidAirportCode(start) || !isValidAirportCode(end)) {
                    cout << "\n✗ Kode bandara harus 3 huruf!" << endl;
                    break;
                }
                
                // Validate maxStops input
                if (maxStopsStr.empty()) {
                    cout << "\n✗ Input maksimal stops tidak boleh kosong!" << endl;
                    break;
                }
                
                // Check if maxStops is numeric
                bool isNumeric = true;
                for (char c : maxStopsStr) {
                    if (!isdigit(c)) {
                        isNumeric = false;
                        break;
                    }
                }
                
                if (!isNumeric) {
                    cout << "\n✗ Maksimal stops harus berupa angka!" << endl;
                    cout << "💡 Contoh: 3, 5, 10" << endl;
                    break;
                }
                
                // Convert to integer
                try {
                    maxStops = stoi(maxStopsStr);
                } catch (...) {
                    cout << "\n✗ Input tidak valid!" << endl;
                    break;
                }
                
                if (maxStops < 1 || maxStops > 10) {
                    cout << "\n✗ Max stops harus antara 1-10!" << endl;
                    break;
                }
                if (!graph.validateAirportInput(start, "Bandara asal")) break;
                if (!graph.validateAirportInput(end, "Bandara tujuan")) break;
                
                vector<vector<string>> allPaths = graph.findAllPaths(start, end, maxStops);
                cout << "\nDitemukan " << allPaths.size() << " path:" << endl;
                for (size_t i = 0; i < min(allPaths.size(), size_t(10)); i++) {
                    cout << (i + 1) << ". ";
                    graph.displayPath(allPaths[i]);
                }
                if (allPaths.size() > 10) {
                    cout << "... dan " << (allPaths.size() - 10) << " path lainnya" << endl;
                }
                break;
            }
            
            case 15: {  
                if (!dataLoaded) {
                    cout << "\n✗ Silakan load data terlebih dahulu!" << endl;
                    break;
                }
                graph.displayStatistics();
                break;
            }
            
            case 16: {  // removeEdge
                if (!dataLoaded) {
                    cout << "\n✗ Silakan load data terlebih dahulu!" << endl;
                    break;
                }
                
                string from, to;
                cout << "\n=== HAPUS RUTE ===" << endl;
                cout << "Bandara asal: ";
                getline(cin, from);
                cout << "Bandara tujuan: ";
                getline(cin, to);
                
                from = toUpperCase(trim(from));
                to = toUpperCase(trim(to));
                
                if (from.empty() || to.empty()) {
                    cout << "\n✗ Input tidak boleh kosong!" << endl;
                    break;
                }
                if (!isValidAirportCode(from) || !isValidAirportCode(to)) {
                    cout << "\n✗ Kode bandara harus 3 huruf!" << endl;
                    break;
                }
                
                // Show existing routes before removal
                if (graph.hasEdge(from, to)) {
                    vector<Route> routes = graph.getNeighbors(from);
                    int count = 0;
                    for (const Route& r : routes) {
                        if (r.destination == to) count++;
                    }
                    cout << "\nDitemukan " << count << " rute dari " << from << " → " << to << endl;
                }
                
                bool success = graph.removeEdge(from, to);
                if (success) {
                    cout << "\n✓ Rute " << from << " → " << to << " berhasil dihapus!" << endl;
                    cout << "💡 Note: Semua rute dengan airlines berbeda juga terhapus." << endl;
                } else {
                    cout << "\n✗ Gagal menghapus rute." << endl;
                }
                break;
            }
            
            // ==================== END MENU FUNGSI BARU ====================
            
            case 0:
                cout << "\nTerima kasih telah menggunakan sistem!" << endl;
                return 0;
                
            default:
                cout << "\nPilihan tidak valid!" << endl;
        }
        
        // Pause before showing menu again
        cout << "\nTekan Enter untuk melanjutkan...";
        cin.get();  // Just wait for Enter, getline() will handle clean state in next iteration
    }
    
    return 0;
}
