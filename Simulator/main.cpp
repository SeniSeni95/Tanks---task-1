#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <thread>
#include <mutex>
#include <fstream>
#include <sstream>
#include <chrono>
#include <dlfcn.h>
#include <map>
#include <set>
#include "GameManagerRegistrar.h"
#include "../common/AbstractGameManager.h"
#include "../common/SymbolObject.h"
#include "../common/ActionRequest.h"
#include "../common/Player.h"
#include "../common/TankAlgorithm.h"
#include "../common/GameResult.h"
#include "AlgorithmRegistrar.h"
#include "Board.h"
#include "GameObject.h"
#include "SatelliteViewImpl.h"

namespace fs = std::filesystem;

// Debug control - set to true to enable debugging, false to disable
 bool DEBUG_MAIN = true;

// -------------------------------
// Loader helpers
// -------------------------------
using init_t = void (*)();

void loadAlgorithmSO(const std::string& so_path, const std::string& name) {
    auto& registrar = AlgorithmRegistrar::getAlgorithmRegistrar();
    registrar.createAlgorithmFactoryEntry(name);
    void* handle = dlopen(so_path.c_str(), RTLD_NOW);
    if (!handle) { std::cerr << "[ERROR] dlopen failed: " << dlerror() << "\n"; registrar.removeLast(); return; }
    try {
        registrar.validateLastRegistration();
    } catch (...) {
        registrar.removeLast();
        dlclose(handle);
    }
}

void loadGameManagerSO(const std::string& so_path) {
    void* handle = dlopen(so_path.c_str(), RTLD_NOW);
    if (!handle) {
        std::cerr << "[ERROR] GM dlopen failed: " << dlerror() << "\n";
        return;
    }
    // no AlgorithmRegistrar here — only GameManagerRegistrar should be used
    if (DEBUG_MAIN) std::cout << "[DEBUG] GM dlopen successful: " << so_path << "\n";
}

// -------------------------------
// Map loader
// -------------------------------
struct LoadedMap {
    int width = 0;
    int height = 0;
    int maxSteps = 0;
    int numShells = 0;

    SatelliteViewImpl view;
    std::unique_ptr<Player> p1;
    std::unique_ptr<Player> p2;
};

LoadedMap buildMapFromFile(const std::string& filename, PlayerFactory& pf) {
    if (DEBUG_MAIN) {
        std::cout << "[DEBUG] Building map from file: " << filename << "\n";
    }

    LoadedMap lm;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "[ERROR] Failed to open map: " << filename << "\n";
        return lm;
    }

    std::string line;
    getline(file, line); // skip description
    if (DEBUG_MAIN) {
        std::cout << "[DEBUG] Map description: " << line << "\n";
    }

    int maxSteps=0,numShells=0,rows=0,cols=0;
    getline(file,line); sscanf(line.c_str(),"MaxSteps = %d",&maxSteps);
    getline(file,line); sscanf(line.c_str(),"NumShells = %d",&numShells);
    getline(file,line); sscanf(line.c_str(),"Rows = %d",&rows);
    getline(file,line); sscanf(line.c_str(),"Cols = %d",&cols);

    if (DEBUG_MAIN) {
        std::cout << "[DEBUG] Map parameters: MaxSteps=" << maxSteps
                  << ", NumShells=" << numShells
                  << ", Rows=" << rows
                  << ", Cols=" << cols << "\n";
    }

    // Build a temporary board
    std::vector<std::vector<cell>> arr;
    arr.reserve(cols);
    for (int i = 0; i < cols; i++) {
        std::vector<cell> col;
        col.reserve(rows);
        for (int j = 0; j < rows; j++) {
            col.emplace_back(i, j);
        }
        arr.emplace_back(std::move(col));
    }
    auto board = std::make_unique<game_board>(cols, rows, std::move(arr));

    if (DEBUG_MAIN) {
        std::cout << "[DEBUG] Board created with dimensions "
                  << cols << "x" << rows << "\n";
    }

    std::set<char> valid = {'#','1','2',' ','@'};
    std::vector<int> tank_counters(2,0);

    for (int j=0;j<rows;j++) {
        getline(file,line);
        if (DEBUG_MAIN) {
            std::cout << "[DEBUG] Row " << j << ": " << line << "\n";
        }

        for (int i=0;i<cols;i++) {
            char ch = (i<(int)line.size())?line[i]:' ';
            if (!valid.count(ch)) ch=' ';
            cell& c = board->get_cell(i,j);

            if (ch != ' ') {
                c.add_Object(std::make_shared<SymbolObject>(i, j, ch));

                if (ch=='1' || ch=='2') {
                    int pnum = (ch=='1')?0:1;
                    int tnum = ++tank_counters[pnum];
                    if (DEBUG_MAIN) {
                        std::cout << "[DEBUG] Found tank symbol P" << (pnum+1)
                                  << " T" << tnum
                                  << " at (" << i << "," << j << ")\n";
                    }
                }
            }
        }
    }
    file.close();

    if (DEBUG_MAIN) {
        std::cout << "[DEBUG] Total tanks (by symbol): P1=" << tank_counters[0]
                  << ", P2=" << tank_counters[1] << "\n";
    }

    auto p1 = pf(1,cols,rows,maxSteps,numShells);
    auto p2 = pf(2,cols,rows,maxSteps,numShells);

    if (DEBUG_MAIN) {
        std::cout << "[DEBUG] Players created successfully\n";
    }

    lm.width = cols;
    lm.height = rows;
    lm.maxSteps = maxSteps;
    lm.numShells = numShells;

    // SatelliteView keeps its own internal copy
    lm.view.updateSymbols(*board);

    // discard dummy board
    board.reset();

    lm.p1 = std::move(p1);
    lm.p2 = std::move(p2);

    if (DEBUG_MAIN) {
        std::cout << "[DEBUG] Map loading completed successfully\n";
    }

    return lm;
}
// -------------------------------
// Usage
// -------------------------------
void printUsage(const std::string& msg="") {
    if (!msg.empty()) std::cerr << "[ERROR] " << msg << "\n";
    std::cerr << "Usage:\n"
              << "Comparative mode:\n"
              << "  ./simulator -comparative game_map=<file> game_managers_folder=<folder> algorithm1=<file> algorithm2=<file> [num_threads=<n>] [-verbose]\n"
              << "Competition mode:\n"
              << "  ./simulator -competition game_maps_folder=<folder> game_manager=<file> algorithms_folder=<folder> [num_threads=<n>] [-verbose]\n";
}


struct AlgorithmEntry {
    std::string file;
    AlgorithmRegistrar::Entry factories;

    AlgorithmEntry(const std::string& f,
                   const AlgorithmRegistrar::Entry& fac)
        : file(f), factories(fac) {}
};

// -------------------------------
// MAIN
// -------------------------------
int main(int argc, char* argv[]) {
    if (DEBUG_MAIN) {
        std::cout << "[DEBUG] Simulator starting with " << argc << " arguments\n";
        for (int i = 0; i < argc; i++) {
            std::cout << "[DEBUG] argv[" << i << "] = " << argv[i] << "\n";
        }
    }
    
    bool comparative=false, competition=false, verbose=false;
    std::unordered_map<std::string,std::string> args;

    for (int i=1;i<argc;i++) {
        std::string tok(argv[i]);
        if (tok=="-comparative") comparative=true;
        else if (tok=="-competition") competition=true;
        else if (tok=="-verbose") verbose=true;
        else {
            auto pos=tok.find('=');
            if (pos==std::string::npos) { printUsage("Unsupported arg: "+tok); return 1;}
            args[tok.substr(0,pos)] = tok.substr(pos+1);
        }
    }

    if (DEBUG_MAIN) {
        std::cout << "[DEBUG] Mode flags: comparative=" << comparative 
                  << ", competition=" << competition 
                  << ", verbose=" << verbose << "\n";
        std::cout << "[DEBUG] Parsed arguments:\n";
        for (const auto& [key, value] : args) {
            std::cout << "[DEBUG]   " << key << " = " << value << "\n";
        }
    }

    if (comparative==competition) { printUsage("Must specify exactly one of -comparative or -competition"); return 1;}

   // -------------------------------
// Comparative mode
// -------------------------------
if (comparative) {
    if (DEBUG_MAIN) {
        std::cout << "[DEBUG] Entering comparative mode\n";
    }
        
    if (!args.count("game_map") || !args.count("game_managers_folder") ||
        !args.count("algorithm1") || !args.count("algorithm2")) {
        printUsage("Missing args for comparative");
        return 1;
    }

    std::string mapfile   = args["game_map"];
    std::string gmFolder  = args["game_managers_folder"];
    std::string algo1File = args["algorithm1"];
    std::string algo2File = args["algorithm2"];
    size_t numThreads     = args.count("num_threads") ? std::stoul(args["num_threads"]) : 1;

    std::cout << "[MODE] Comparative\n";
    if (verbose) std::cout << "[FLAG] Verbose enabled\n";
    if (DEBUG_MAIN) {
        std::cout << "[DEBUG] Threads: " << numThreads << "\n";
        std::cout << "[DEBUG] Map: " << mapfile << "\n";
        std::cout << "[DEBUG] GM Folder: " << gmFolder << "\n";
        std::cout << "[DEBUG] Algorithm 1: " << algo1File << "\n";
        std::cout << "[DEBUG] Algorithm 2: " << algo2File << "\n";
    }

    // --- load the 2 algorithms ---
    AlgorithmRegistrar::getAlgorithmRegistrar().clear();
    if (DEBUG_MAIN) std::cout << "[DEBUG] Loading algorithms...\n";

    loadAlgorithmSO(algo1File, algo1File);
    loadAlgorithmSO(algo2File, algo2File);

    auto& registrar = AlgorithmRegistrar::getAlgorithmRegistrar();
    if (registrar.count() < 2) {
        std::cerr << "[ERROR] Need two algorithms\n";
        return 1;
    }
    auto it = registrar.begin();
    auto algoEntry1 = *it;
    auto algoEntry2 = *(++it);

    if (DEBUG_MAIN) std::cout << "[DEBUG] Algorithms loaded successfully\n";

    // --- scan GM folder ---
    std::vector<std::string> gmFiles;
    for (auto& entry : fs::directory_iterator(gmFolder)) {
        if (entry.path().extension() == ".so")
            gmFiles.push_back(entry.path().string());
    }
    if (gmFiles.empty()) {
        std::cerr << "[ERROR] No game managers in " << gmFolder << "\n";
        return 1;
    }
    if (DEBUG_MAIN) {
        std::cout << "[DEBUG] Found " << gmFiles.size() << " game managers:\n";
        for (const auto& gmFile : gmFiles) std::cout << "[DEBUG]   " << gmFile << "\n";
    }

    std::mutex resultsMutex;
    std::vector<GameResult> allResults;
    std::vector<std::thread> threads;

    auto worker = [&](const std::string& gmFile) {
        if (DEBUG_MAIN) {
            std::cout << "[DEBUG] Worker thread started for GM: " << gmFile << "\n";
        }

        GameManagerRegistrar::get().clear();
        loadGameManagerSO(gmFile);   // ✅ GM loader

        auto& gmReg = GameManagerRegistrar::get();
        if (gmReg.empty()) {
            std::cerr << "[ERROR] No game managers registered for: " << gmFile << "\n";
            return;
        }

        std::unique_ptr<AbstractGameManager> gm;
        try {
            auto factory = gmReg.last();
            gm = factory(verbose);
            if (!gm) {
                std::cerr << "[ERROR] GM factory returned null for: " << gmFile << "\n";
                return;
            }
            if (DEBUG_MAIN) std::cout << "[DEBUG] Game manager created for: " << gmFile << "\n";
        }
        catch (...) {
            std::cerr << "[ERROR] Exception while creating GM from " << gmFile << "\n";
            return;
        }

        PlayerFactory pf = [&](int p, size_t x, size_t y, size_t maxS, size_t numS) {
            return (p == 1)
                ? algoEntry1.createPlayer(p, x, y, maxS, numS)
                : algoEntry2.createPlayer(p, x, y, maxS, numS);
        };
        TankAlgorithmFactory tf1 = [&](int p, int t) { return algoEntry1.createTankAlgorithm(p, t); };
        TankAlgorithmFactory tf2 = [&](int p, int t) { return algoEntry2.createTankAlgorithm(p, t); };

        LoadedMap map = buildMapFromFile(mapfile, pf);

        if (DEBUG_MAIN) std::cout << "[DEBUG] Running game with GM: " << gmFile << "\n";

        GameResult res;
        try {
            res = gm->run(
                map.width, map.height, map.view, mapfile,
                map.maxSteps, map.numShells,
                *map.p1, "Algorithm1",
                *map.p2, "Algorithm2",
                tf1, tf2
            );
        }
        catch (...) {
            std::cerr << "[ERROR] Exception during run with GM: " << gmFile << "\n";
            return;
        }

        if (DEBUG_MAIN) {
            std::cout << "[DEBUG] Game completed for GM: " << gmFile
                      << " Winner: " << res.winner
                      << " Rounds: " << res.rounds << "\n";
        }

        std::lock_guard<std::mutex> lk(resultsMutex);
        allResults.push_back(std::move(res));
    };

    // --- thread batching like competition ---
    for (auto& gmFile : gmFiles) {
        threads.emplace_back(worker, gmFile);
        if (threads.size() >= numThreads) {
            for (auto& t : threads) t.join();
            threads.clear();
        }
    }
    for (auto& t : threads) t.join();

    if (DEBUG_MAIN)
        std::cout << "[DEBUG] All comparative games completed. Results: " << allResults.size() << "\n";

    std::cout << "=== Comparative Results ===\n";
    for (size_t i = 0; i < allResults.size(); i++) {
        auto& r = allResults[i];
        std::cout << "Game " << (i+1) << " - Winner: " << r.winner
                  << " Reason: " << r.reason
                  << " Rounds: " << r.rounds << "\n";
    }
}

// -------------------------------
// Competition mode
// -------------------------------
if (competition) {
    if (DEBUG_MAIN) {
        std::cout << "[DEBUG] Entering competition mode\n";
    }

    if (!args.count("game_maps_folder") || !args.count("game_manager") || !args.count("algorithms_folder")) {
        printUsage("Missing args for competition");
        return 1;
    }
    std::string mapsFolder = args["game_maps_folder"];
    std::string gmFile     = args["game_manager"];
    std::string algFolder  = args["algorithms_folder"];
    int numThreads         = args.count("num_threads") ? std::stoi(args["num_threads"]) : 1;

    std::cout << "[MODE] Competition\n";
    if (verbose) std::cout << "[FLAG] Verbose enabled\n";
    if (DEBUG_MAIN) {
        std::cout << "[DEBUG] Maps folder: " << mapsFolder << "\n";
        std::cout << "[DEBUG] Game manager: " << gmFile << "\n";
        std::cout << "[DEBUG] Algorithms folder: " << algFolder << "\n";
        std::cout << "[DEBUG] Threads: " << numThreads << "\n";
    }

    // --- load GM ---
    if (DEBUG_MAIN) std::cout << "[DEBUG] Loading game manager: " << gmFile << "\n";
    loadGameManagerSO(gmFile);  // ✅ GM uses its own loader
    auto& gmReg = GameManagerRegistrar::get();
    if (gmReg.empty()) {
        std::cerr << "[ERROR] No game managers registered!\n";
        return 1;
    }
    std::unique_ptr<AbstractGameManager> gm;
    try {
        gm = gmReg.last()(verbose);
        if (!gm) {
            std::cerr << "[ERROR] GM factory returned null for: " << gmFile << "\n";
            return 1;
        }
        if (DEBUG_MAIN) std::cout << "[DEBUG] Game manager created for: " << gmFile << "\n";
    }
    catch (const std::exception& e) {
        std::cerr << "[ERROR] Exception while creating GM from " << gmFile
                  << ": " << e.what() << "\n";
        return 1;
    }

    // --- algorithm entries ---
    struct AlgorithmEntry {
        std::string file;
        const AlgorithmRegistrar::Entry* factories;
    };

    std::vector<AlgorithmEntry> loadedAlgorithms;
    for (auto& entry : fs::directory_iterator(algFolder)) {
        if (entry.path().extension() != ".so") continue;

        std::string algoFile = entry.path().string();
        AlgorithmRegistrar::getAlgorithmRegistrar().clear();
        try {
            loadAlgorithmSO(algoFile, algoFile);   // ✅ algorithms use algo loader
            AlgorithmRegistrar::getAlgorithmRegistrar().validateLastRegistration();
            auto& fac = *(AlgorithmRegistrar::getAlgorithmRegistrar().end() - 1);
            loadedAlgorithms.push_back({algoFile, &fac});
            if (DEBUG_MAIN) std::cout << "[DEBUG] Loaded algorithm: " << algoFile << "\n";
        }
        catch (...) {
            std::cerr << "[ERROR] Bad or invalid algorithm: " << algoFile << "\n";
            AlgorithmRegistrar::getAlgorithmRegistrar().removeLast();
        }
    }

    if (loadedAlgorithms.empty()) {
        std::cerr << "[ERROR] No valid algorithms loaded!\n";
        return 1;
    }

    // --- maps ---
    std::vector<std::string> maps;
    for (auto& entry : fs::directory_iterator(mapsFolder)) {
        if (entry.path().extension() == ".txt")
            maps.push_back(entry.path().string());
    }
    if (DEBUG_MAIN) {
        std::cout << "[DEBUG] Found " << maps.size() << " maps:\n";
        for (auto& m : maps) std::cout << "[DEBUG]   " << m << "\n";
    }

    // --- competition runs ---
    std::mutex scoreMutex;
    std::map<std::string, int> scores;
    std::vector<std::thread> threads;

    auto worker = [&](const std::string& mapFile, const AlgorithmEntry& algoPair) {
        const std::string& algoFile = algoPair.file;
        const auto* entry = algoPair.factories;

        if (DEBUG_MAIN) {
            std::cout << "[DEBUG] Competition worker started: "
                      << algoFile << " on " << mapFile << "\n";
        }

        PlayerFactory pf = [&](int p, size_t x, size_t y, size_t maxS, size_t numS) {
            return entry->createPlayer(p, x, y, maxS, numS);
        };
        TankAlgorithmFactory tf = [&](int p, int t) {
            return entry->createTankAlgorithm(p, t);
        };

        LoadedMap map;
        try {
            map = buildMapFromFile(mapFile, pf);
        }
        catch (const std::exception& e) {
            std::cerr << "[ERROR] Failed to build map " << mapFile
                      << ": " << e.what() << "\n";
            return;
        }

        GameResult res;
        try {
            res = gm->run(
                map.width, map.height, map.view, mapFile,
                map.maxSteps, map.numShells,
                *map.p1, algoFile,
                *map.p2, algoFile,
                tf, tf
            );
        }
        catch (...) {
            std::cerr << "[ERROR] Exception while running GM with "
                      << algoFile << " on " << mapFile << "\n";
            return;
        }

        if (DEBUG_MAIN) {
            std::cout << "[DEBUG] Competition game completed: "
                      << algoFile << " Winner: " << res.winner
                      << " Rounds: " << res.rounds << "\n";
        }

        std::lock_guard<std::mutex> lk(scoreMutex);
        if (res.winner == 1) scores[algoFile] += 3;
        else if (res.winner == 2) scores[algoFile] += 3;
        else scores[algoFile] += 1;
    };

    if (DEBUG_MAIN) {
        std::cout << "[DEBUG] Starting competition with "
                  << (maps.size() * loadedAlgorithms.size())
                  << " total games\n";
    }

    for (auto& m : maps) {
        for (auto& a : loadedAlgorithms) {
            auto aCopy = a;
            threads.emplace_back(worker, m, aCopy);
            if (threads.size() >= (size_t)numThreads) {
                for (auto& t : threads) t.join();
                threads.clear();
            }
        }
    }
    for (auto& t : threads) t.join();

    if (DEBUG_MAIN) std::cout << "[DEBUG] All competition games completed\n";

    std::cout << "=== Competition Results ===\n";
    for (auto& [alg, score] : scores) {
        std::cout << alg << ": " << score << "\n";
    }
}





    if (DEBUG_MAIN) {
        std::cout << "[DEBUG] Simulator exiting normally\n";
    }

    return 0;
}

/*

./Simulator/simulator -comparative   game_map=maps/map1.txt   game_managers_folder=class_gm   algorithm1=class_algo/Algorithm_206038929_314620071.so   algorithm2=class_algo/Algorithm_207174533_321321945.so   -verbose

./Simulator/simulator -competition \
  game_maps_folder=maps \
  game_manager=class_gm/GameManager_209277367_322542887.so \
  algorithms_folder=class_algo \
  num_threads=4 \
  -verbose


*/