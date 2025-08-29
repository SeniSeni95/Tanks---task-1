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
 bool DEBUG_MAP  = false;

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
    if (DEBUG_MAP) {
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
    if (DEBUG_MAP) {
        std::cout << "[DEBUG] Map description: " << line << "\n";
    }

    int maxSteps=0,numShells=0,rows=0,cols=0;
    getline(file,line); sscanf(line.c_str(),"MaxSteps = %d",&maxSteps);
    getline(file,line); sscanf(line.c_str(),"NumShells = %d",&numShells);
    getline(file,line); sscanf(line.c_str(),"Rows = %d",&rows);
    getline(file,line); sscanf(line.c_str(),"Cols = %d",&cols);

    if (DEBUG_MAP) {
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

    if (DEBUG_MAP) {
        std::cout << "[DEBUG] Board created with dimensions "
                  << cols << "x" << rows << "\n";
    }

    std::set<char> valid = {'#','1','2',' ','@'};
    std::vector<int> tank_counters(2,0);

    for (int j=0;j<rows;j++) {
        getline(file,line);
        if (DEBUG_MAP) {
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
                    if (DEBUG_MAP) {
                        std::cout << "[DEBUG] Found tank symbol P" << (pnum+1)
                                  << " T" << tnum
                                  << " at (" << i << "," << j << ")\n";
                    }
                }
            }
        }
    }
    file.close();

    if (DEBUG_MAP) {
        std::cout << "[DEBUG] Total tanks (by symbol): P1=" << tank_counters[0]
                  << ", P2=" << tank_counters[1] << "\n";
    }

    auto p1 = pf(1,cols,rows,maxSteps,numShells);
    auto p2 = pf(2,cols,rows,maxSteps,numShells);

    if (DEBUG_MAP) {
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

    if (DEBUG_MAP) {
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

std::string makeComparativeFilename(const std::string& gmFolder) {
    auto now = std::chrono::system_clock::now();
    auto t   = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << gmFolder << "/comparative_results_" << t << ".txt";
    return ss.str();
}



std::string reasonToString(GameResult::Reason r) {
    switch (r) {
        case GameResult::ALL_TANKS_DEAD: return "ALL_TANKS_DEAD";
        case GameResult::MAX_STEPS:      return "MAX_STEPS";
        case GameResult::ZERO_SHELLS:    return "ZERO_SHELLS";
        default:                         return "UNKNOWN";
    }
}
std::string winnerToString(int w) {
    if (w == 1) return "Player 1";
    if (w == 2) return "Player 2";
    return "Tie";
}

void writeComparativeResults(const std::string& gmFolder,
                             const std::string& mapfile,
                             const std::string& algo1Name,
                             const std::string& algo2Name,
                             const std::vector<std::pair<std::string,GameResult>>& results,
                             size_t width, size_t height)
{
    std::string filename = makeComparativeFilename(gmFolder);
    std::ofstream fout(filename);
    std::ostream& out = fout.is_open() ? fout : std::cout;
    if (!fout) {
        std::cerr << "[ERROR] Could not create " << filename << ", writing to stdout instead\n";
    }

    // header
    out << "game_map="   << mapfile   << "\n";
    out << "algorithm1=" << algo1Name << "\n";
    out << "algorithm2=" << algo2Name << "\n\n";

    // group by identical results
    std::vector<bool> used(results.size(), false);
    for (size_t i=0;i<results.size();i++) {
        if (used[i]) continue;
        std::vector<std::string> sameGMs;
        sameGMs.push_back(results[i].first);

        for (size_t j=i+1;j<results.size();j++) {
            if (used[j]) continue;
            const auto& r1 = results[i].second;
            const auto& r2 = results[j].second;
            if (r1.winner==r2.winner &&
                r1.reason==r2.reason &&
                r1.rounds==r2.rounds &&
                r1.gameState && r2.gameState)
            {
                bool identical=true;
                for (size_t x=0;x<width && identical;x++)
                    for (size_t y=0;y<height;y++)
                        if (r1.gameState->getObjectAt(x,y)!=
                            r2.gameState->getObjectAt(x,y)) {
                            identical=false; break;
                        }
                if (identical) {
                    sameGMs.push_back(results[j].first);
                    used[j]=true;
                }
            }
        }

        // a) list of GMs
        for (size_t k=0;k<sameGMs.size();k++) {
            if (k>0) out<<",";
            out<<sameGMs[k];
        }
        out<<"\n";

        // b) result message
        if (results[i].second.winner==0)
            out<<"Tie - "<<reasonToString(results[i].second.reason)<<"\n";
        else
            out<<"Player "<<results[i].second.winner<<" won - "<<reasonToString(results[i].second.reason)<<"\n";

        // c) round
        out<<results[i].second.rounds<<"\n";

        // d) final board
        if (results[i].second.gameState) {
            for (size_t y=0;y<height;y++) {
                for (size_t x=0;x<width;x++)
                    out<<results[i].second.gameState->getObjectAt(x,y);
                out<<"\n";
            }
        }

        out<<"\n";
    }
}


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

    // --- load the 2 algorithms ---
    AlgorithmRegistrar::getAlgorithmRegistrar().clear();
    loadAlgorithmSO(algo1File, fs::path(algo1File).filename().string());
    loadAlgorithmSO(algo2File, fs::path(algo2File).filename().string());

    auto& registrar = AlgorithmRegistrar::getAlgorithmRegistrar();
    if (registrar.count() < 2) {
        std::cerr << "[ERROR] Need two algorithms\n";
        return 1;
    }
    auto it = registrar.begin();
    auto algoEntry1 = *it;
    auto algoEntry2 = *(++it);

    std::string algo1Name = fs::path(algo1File).filename().string();
    std::string algo2Name = fs::path(algo2File).filename().string();

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

    std::mutex resultsMutex;
    struct NamedResult { std::string gmName; GameResult result; };
    std::vector<NamedResult> allResults;
    std::vector<std::thread> threads;

    size_t width=0,height=0;

    auto worker = [&](const std::string& gmFile) {
        std::string gmName = fs::path(gmFile).filename().string();
        GameManagerRegistrar::get().clear();
        loadGameManagerSO(gmFile);

        auto& gmReg = GameManagerRegistrar::get();
        if (gmReg.empty()) {
            std::cerr << "[ERROR] No game managers registered for: " << gmFile << "\n";
            return;
        }

        std::unique_ptr<AbstractGameManager> gm;
        try {
            gm = gmReg.last()(verbose);
            if (!gm) {
                std::cerr << "[ERROR] GM factory returned null for: " << gmFile << "\n";
                return;
            }
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
        width=map.width; height=map.height;

        GameResult res;
        try {
            res = gm->run(
                map.width, map.height, map.view, mapfile,
                map.maxSteps, map.numShells,
                *map.p1, algo1Name,
                *map.p2, algo2Name,
                tf1, tf2
            );
        }
        catch (...) {
            std::cerr << "[ERROR] Exception during run with GM: " << gmFile << "\n";
            return;
        }

        std::lock_guard<std::mutex> lk(resultsMutex);
        allResults.push_back({gmName,std::move(res)});
    };

    // --- thread batching ---
    for (auto& gmFile : gmFiles) {
        threads.emplace_back(worker, gmFile);
        if (threads.size() >= numThreads) {
            for (auto& t : threads) t.join();
            threads.clear();
        }
    }
    for (auto& t : threads) t.join();

    // --- group identical results ---
    std::vector<bool> used(allResults.size(), false);
    auto outFile = [&](){
        auto now = std::chrono::system_clock::now();
        auto t   = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << gmFolder << "/comparative_results_" << t << ".txt";
        return ss.str();
    }();

    std::ofstream fout(outFile);
    std::ostream& out = fout.is_open() ? fout : std::cout;
    if (!fout) {
        std::cerr << "[ERROR] Could not create " << outFile << ", writing to stdout\n";
    }

    out << "game_map="   << mapfile   << "\n";
    out << "algorithm1=" << algo1Name << "\n";
    out << "algorithm2=" << algo2Name << "\n\n";

    for (size_t i=0;i<allResults.size();i++) {
        if (used[i]) continue;
        std::vector<std::string> sameGMs{allResults[i].gmName};
        const auto& ri = allResults[i].result;

        for (size_t j=i+1;j<allResults.size();j++) {
            if (used[j]) continue;
            const auto& rj = allResults[j].result;
            bool identical = (ri.winner==rj.winner &&
                              ri.reason==rj.reason &&
                              ri.rounds==rj.rounds);
            if (identical && ri.gameState && rj.gameState) {
                for (size_t x=0;x<width && identical;x++)
                    for (size_t y=0;y<height;y++)
                        if (ri.gameState->getObjectAt(x,y)!=rj.gameState->getObjectAt(x,y))
                            { identical=false; break; }
            }
            if (identical) {
                sameGMs.push_back(allResults[j].gmName);
                used[j]=true;
            }
        }

        // a) GM list
        for (size_t k=0;k<sameGMs.size();k++) {
            if (k>0) out<<",";
            out<<sameGMs[k];
        }
        out<<"\n";

        // b) Result message
        if (ri.winner==0)
            out<<"Tie - "<<reasonToString(ri.reason)<<"\n";
        else
            out<<"Player "<<ri.winner<<" won - "<<reasonToString(ri.reason)<<"\n";

        // c) round
        out<<ri.rounds<<"\n";

        // d) final board
        if (ri.gameState) {
            for (size_t y=0;y<height;y++) {
                for (size_t x=0;x<width;x++)
                    out<<ri.gameState->getObjectAt(x,y);
                out<<"\n";
            }
        }
        out<<"\n";
    }

    std::cout << "=== Comparative Results written to " << outFile << " ===\n";
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

    // --- load GM ---
    loadGameManagerSO(gmFile);
    auto& gmReg = GameManagerRegistrar::get();
    if (gmReg.empty()) {
        std::cerr << "[ERROR] No game managers registered!\n";
        return 1;
    }
    std::unique_ptr<AbstractGameManager> gm = gmReg.last()(verbose);

    // --- load algorithms ---
    std::vector<AlgorithmEntry> algs;
    for (auto& entry : fs::directory_iterator(algFolder)) {
        if (entry.path().extension() != ".so") continue;
        AlgorithmRegistrar::getAlgorithmRegistrar().clear();
        loadAlgorithmSO(entry.path().string(), entry.path().string());
        if (AlgorithmRegistrar::getAlgorithmRegistrar().count() == 0) continue;

        std::string base = entry.path().filename().string(); // basename only
        algs.push_back({ base, *AlgorithmRegistrar::getAlgorithmRegistrar().begin() });

        if (DEBUG_MAIN) std::cout << "[DEBUG] Loaded algorithm: " << base << "\n";
    }
    size_t N = algs.size();
    if (N < 2) {
        std::cerr << "[ERROR] Need at least 2 algorithms for competition\n";
        return 1;
    }

    // --- collect maps ---
    std::vector<std::string> maps;
    for (auto& entry : fs::directory_iterator(mapsFolder)) {
        if (entry.path().extension() == ".txt")
            maps.push_back(entry.path().string());
    }
    if (maps.empty()) {
        std::cerr << "[ERROR] No maps in " << mapsFolder << "\n";
        return 1;
    }

    // --- scores ---
    std::map<std::string,int> scores;
    std::mutex scoreMutex;
    std::vector<std::thread> threads;

    auto worker = [&](const std::string& mapFile,
                      const AlgorithmEntry& a1,
                      const AlgorithmEntry& a2) {
        PlayerFactory pf = [&](int p, size_t x, size_t y, size_t maxS, size_t numS) {
            return (p==1)
                ? a1.factories.createPlayer(p,x,y,maxS,numS)
                : a2.factories.createPlayer(p,x,y,maxS,numS);
        };
        TankAlgorithmFactory tf1 = [&](int p,int t){ return a1.factories.createTankAlgorithm(p,t); };
        TankAlgorithmFactory tf2 = [&](int p,int t){ return a2.factories.createTankAlgorithm(p,t); };

        LoadedMap map = buildMapFromFile(mapFile, pf);
        GameResult res = gm->run(map.width,map.height,map.view,mapFile,
                                 map.maxSteps,map.numShells,
                                 *map.p1,a1.file,*map.p2,a2.file,
                                 tf1,tf2);

        std::lock_guard<std::mutex> lk(scoreMutex);
        if (res.winner == 1) scores[a1.file]+=3;
        else if (res.winner==2) scores[a2.file]+=3;
        else { scores[a1.file]+=1; scores[a2.file]+=1; }
    };

    // --- run schedule ---
    for (size_t k=0;k<maps.size();k++) {
        for (size_t i=0;i<N;i++) {
            size_t j = (i + 1 + k % (N-1)) % N;
            if (j==i) continue;
            // Avoid duplicate if N even and k==N/2 -1
            if (N%2==0 && k==(N/2 -1) && j==(i+N/2)%N) continue;

            if (numThreads<=1) {
                worker(maps[k], algs[i], algs[j]);
            } else {
                threads.emplace_back(worker,maps[k],algs[i],algs[j]);
                if (threads.size()>= (size_t)numThreads) {
                    for (auto& t:threads) t.join();
                    threads.clear();
                }
            }
        }
    }
    for (auto& t:threads) t.join();

    // --- output file ---
    auto now = std::chrono::system_clock::now();
    auto t   = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << algFolder << "/competition_" << t << ".txt";
    std::ofstream fout(ss.str());
    std::ostream& out = fout.is_open() ? fout : std::cout;
    if (!fout) {
        std::cerr << "[ERROR] Could not create " << ss.str() << ", writing to stdout\n";
    }

    out << "game_maps_folder=" << mapsFolder << "\n";
    out << "game_manager=" << fs::path(gmFile).filename().string() << "\n\n";

    // sort by score desc
    std::vector<std::pair<std::string,int>> sorted(scores.begin(),scores.end());
    std::sort(sorted.begin(),sorted.end(),
              [](auto& a,auto& b){return a.second>b.second;});
    for (auto& [f,s] : sorted) {
        out << f << " " << s << "\n";
    }

    std::cout << "\n=== Competition Results ===\n";
    for (auto& [f,s] : sorted) {
        std::cout << f << ": " << s << "\n";
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