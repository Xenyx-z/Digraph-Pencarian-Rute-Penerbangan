# 🛫 SISTEM PENCARIAN RUTE PENERBANGAN
## Studi Kasus: Directed Graph dengan BFS, DFS, dan Dijkstra


## 📖 DESKRIPSI PROYEK

Sistem pencarian rute penerbangan menggunakan **Directed Graph** dengan implementasi **BFS**, **DFS**, dan **Dijkstra**. Program ini mendemonstrasikan perbedaan ketiga algoritma dalam konteks real-world: route planning.

### 🎯 Tujuan:
- Memahami cara kerja graph dalam konteks nyata
- Membandingkan BFS vs DFS vs Dijkstra
- Menampilkan trade-off: **minimum stops** vs **minimum distance**

### 🔑 Key Features:
- ✅ **6,072 airports**, **66,934 routes** - Real-world data
- ✅ **Full airport information** - Name, City, Country, Coordinates
- ✅ **Haversine distance** - Auto-calculated from coordinates
- ✅ **3 algorithms** path finding (BFS, DFS, Dijkstra)
- ✅ **11 visual flowcharts** - Complete algorithm visualization
- ✅ **Perbandingan side-by-side** dengan analisis lengkap
- ✅ **5 fungsi graph operations** (removeEdge, hasVertex, hasEdge, indegree, outdegree)
- ✅ **VSCode ready** - Full IDE integration
- ✅ **Robust input handling** dengan validasi komprehensif
- ✅ **Professional UX** dengan error messages yang jelas

---

## ✨ FITUR UTAMA

### 1. **Load Data dari CSV**
- import bandara dari `assets/airports.csv`
- Import rute dari `assets/routes.csv`
- Automatic validation
- File existence check

### 2. **Path Finding - 3 Algoritma**
- **BFS**: Minimum stops (optimal untuk penumpang)
- **DFS**: Exploratory (tidak dijamin optimal)
- **Dijkstra**: Minimum distance (optimal untuk fuel efficiency)

### 3. **🆕 Perbandingan Algoritma**
- Jalankan ketiga algoritma sekaligus
- Tabel perbandingan side-by-side
- Analisis detail dengan visualisasi
- Rekomendasi penggunaan

### 4. **Graph Traversal**
- BFS & DFS traversal
- Menampilkan semua bandara reachable

### 5. **🆕 Graph Operations (5 Fungsi Baru!)**
- **removeEdge**: Hapus rute dari graph
- **hasVertex**: Cek apakah bandara exist
- **hasEdge**: Cek apakah rute exist
- **indegree**: Hitung rute yang menuju ke bandara
- **outdegree**: Hitung rute yang berangkat dari bandara

### 6. **Advanced Features**
- Multiple path search dengan max stops
- Graph statistics (degree analysis)
- Airport information detail

---

## 📊 DATASET

### **Real-World OpenFlights Data:**

#### **airports.csv:**
```
Total di File:      7,699 airports worldwide
Valid IATA:         6,072 airports (filtered)
Coverage:           Global (all continents)
Info:               Name, City, Country, Coordinates
```

#### **routes.csv:**
```
Total di File:      67,663 routes worldwide
Valid Routes:       66,934 routes (filtered)
Coverage:           Global network
Distance:           Auto-calculated (Haversine formula)
```

### **Data Filtering:**
- ✅ Only airports with valid 3-letter IATA codes
- ✅ Routes between existing airports only
- ✅ Distance calculated from coordinates
- ✅ 729 routes skipped (invalid airports)
- ✅ 1,627 airports skipped (no IATA code)

### **Sample Data:**
```
Airport: CGK - Soekarno-Hatta International Airport
City:    Jakarta, Indonesia
Coords:  -6.1256, 106.6559

Routes from CGK:
  → SIN (Changi Airport)                894 km
  → DPS (Ngurah Rai International)      1150 km
  → BKK (Suvarnabhumi Airport)          2300 km
```

### **Top Hubs (by total degree):**
- **ATL** (Atlanta): 1000+ connections
- **ORD** (Chicago): 900+ connections
- **DFW** (Dallas): 800+ connections
- **CGK** (Jakarta): 166 connections (indegree: 78, outdegree: 88)

---

## 🧠 ALGORITMA YANG DIIMPLEMENTASIKAN

### 1. **BFS (Breadth-First Search)**
**Strategi:** Level-by-level (seperti riak air)

✅ **MENJAMIN** minimum stops
- Uses Queue (FIFO)
- Complexity: O(V+E)
- **Best for:** Penumpang (minimize transit)

### 2. **DFS (Depth-First Search)**
**Strategi:** Depth-first (seperti maze solver)

❌ **TIDAK MENJAMIN** optimal
- Uses Stack (LIFO)
- Complexity: O(V+E)
- **Best for:** Connectivity check only
- ⚠️ **JANGAN untuk route planning!**

### 3. **Dijkstra**
**Strategi:** Greedy - minimize distance

✅ **MENJAMIN** minimum distance
- Uses Priority Queue
- Complexity: O((V+E)logV)
- **Best for:** Airlines (fuel efficiency)

---

## 🚀 CARA COMPILE & RUN

### **Compile:**
```bash
g++ -std=c++17 -O2 flight_route_system.cpp -o flight_system
```

### **Run:**
```bash
./flight_system
```

### **Quick Start:**
```bash
# Load data
Pilihan: 1
Masukkan file: routes.csv

# Try comparison
Pilihan: 6
Asal: CGK
Tujuan: SYD
```

---

## 📖 PANDUAN PENGGUNAAN MENU

### **Menu:**
```
1. Load data dari CSV
2. Tampilkan rute langsung dari bandara
3. Info bandara
4. Cek bandara exist (hasVertex)
5. Cek rute exist (hasEdge)
6. Hitung indegree bandara
7. Hitung outdegree bandara
8. Cari path (BFS)
9. Cari path (DFS)
10. Cari shortest path (Dijkstra)
11. Bandingkan BFS vs DFS vs Dijkstra
12. Traverse dari bandara (BFS)
13. Traverse dari bandara (DFS)
14. Cari semua path dengan max stops
15. Tampilkan statistik graph
16. Hapus rute (removeEdge)
0. Keluar
```

### **Menu 6: Perbandingan (FITUR UTAMA)**

**Input:**
```
Bandara asal: BPN
Bandara tujuan: SYD
```

**Output:**
```
===============================================================================
           PERBANDINGAN ALGORITMA: BFS vs DFS vs DIJKSTRA
===============================================================================

ALGORITMA   STATUS    STOPS     TOTAL JARAK       PATH
-------------------------------------------------------------------------------
BFS         ✓ Found   2         6840 km           BPN → CGK → SYD
DFS         ✓ Found   3         8305 km           BPN → SUB → SIN → SYD
Dijkstra    ✓ Found   3         5880 km           BPN → SUB → DPS → SYD

📊 ANALISIS DETAIL:

1️⃣  BFS: 2 transit ✅ OPTIMAL STOPS
2️⃣  DFS: 3 transit ❌ SUB-OPTIMAL (1 stop lebih banyak!)
3️⃣  Dijkstra: 5880 km ✅ OPTIMAL DISTANCE

🏆 KESIMPULAN:
   • BFS lebih optimal untuk MINIMIZE TRANSIT
   • DFS sub-optimal (tidak dijamin!)
   • Dijkstra optimal untuk MINIMIZE JARAK

📚 REKOMENDASI:
   • BFS: Terbaik untuk penumpang ✈️
   • Dijkstra: Terbaik untuk maskapai ⛽
   • DFS: TIDAK untuk route planning!
===============================================================================
```

---

## 🆚 PERBANDINGAN ALGORITMA

### **Comparison Table:**

| Aspek | BFS | DFS | Dijkstra |
|-------|-----|-----|----------|
| **Goal** | Min stops | Explore | Min distance |
| **Optimal?** | ✅ Stops | ❌ No | ✅ Distance |
| **Structure** | Queue | Stack | PriorityQueue |
| **Complexity** | O(V+E) | O(V+E) | O((V+E)logV) |
| **Best For** | Passengers | Connectivity | Airlines |

### **Real Example: BPN → SYD**

| Algo | Stops | Distance | Winner |
|------|-------|----------|--------|
| BFS | 2 | 6840 km | ✅ **Stops** |
| DFS | 3 | 8305 km | ❌ Sub-opt |
| Dijkstra | 3 | 5880 km | ✅ **Distance** |

**Trade-off:** Fewer stops (BFS) vs Shorter distance (Dijkstra)

---

## 🎓 LEARNING OUTCOMES

✅ **Graph Representation** - Adjacency list
✅ **BFS vs DFS** - Fundamental differences
✅ **Dijkstra** - Shortest path algorithm
✅ **Trade-offs** - Optimization goals
✅ **Real-world Application** - Route planning
✅ **Complexity Analysis** - Time/space

---

## 💡 KEY TAKEAWAYS

- 🔵 **BFS** = Best for passengers (min stops)
- 🔴 **DFS** = NOT for route planning (unreliable)
- 🟢 **Dijkstra** = Best for airlines (min distance/fuel)

**No algorithm is "best" for everything - it depends on your goal!**

---

## 🎯 TESTING

### **Input Validation:**
- ✅ Case-insensitive (cgk = CGK)
- ✅ Whitespace tolerant
- ✅ Invalid input handling
- ✅ File existence check
- ✅ Clear error messages

### **Tested Scenarios:**
- ✅ Normal flow
- ✅ Invalid inputs (huruf, simbol, dll)
- ✅ Empty inputs
- ✅ Out of range inputs
- ✅ Non-existent files
- ✅ Non-existent airports

---

## 📞 TROUBLESHOOTING

**Problem:** File tidak ditemukan
```
Solution: Pastikan routes.csv ada di directory yang sama
```

**Problem:** Input huruf di menu angka
```
Solution: Program otomatis retry dengan error message
```

**Problem:** Kode bandara tidak ditemukan
```
Solution: Gunakan kode IATA 3 huruf (CGK, SIN, DPS, dll)
```

*Last Updated: December 17, 2025*
*Version: 3.0 (Enhanced Edition)*