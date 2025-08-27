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
#include "../common/ActionRequest.h"
#include "../common/Player.h"
#include "../common/TankAlgorithm.h"
#include "../common/GameResult.h"
#include "AlgorithmRegistrar.h"
#include "Board.h"
#include "GameObject.h"
#include "SatelliteViewImpl.h"

namespace fs = std::filesystem;

// -------------------------------
// Loader helpers
// -------------------------------
using init_t = void (*)();

void loadSO(const std::string& so_path, const std::string& name) {
    void* handle = dlopen(so_path.c_str(), RTLD_LAZY);
    if (!handle) {
        std::cerr << "[ERROR] dlopen failed for " << so_path << ": " << dlerror() << "\n";
        return;
    }

    dlerror(); // clear errors
    init_t init_func = (init_t)dlsym(handle, "init");
    const char* err = dlerror();
    if (err) {
        std::cerr << "[ERROR] dlsym failed in " << so_path << ": " << err << "\n";
        dlclose(handle);
        return;
    }

    auto& registrar = AlgorithmRegistrar::getAlgorithmRegistrar();
    registrar.createAlgorithmFactoryEntry(name);
    init_func();

    try {
        registrar.validateLastRegistration();
    } catch (AlgorithmRegistrar::BadRegistrationException& e) {
        std::cerr << "[ERROR] BadRegistrationException for: " << e.name << "\n";
        registrar.removeLast();
        dlclose(handle);
    }
}

// -------------------------------
// Map loader
// -------------------------------
struct LoadedMap {
    SatelliteViewImpl view;
    std::unique_ptr<game_board> board;
    std::unique_ptr<Player> p1;
    std::unique_ptr<Player> p2;
    size_t width{}, height{}, maxSteps{}, numShells{};
};

LoadedMap buildMapFromFile(const std::string& filename, PlayerFactory& pf) {
    LoadedMap lm;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "[ERROR] Failed to open map: " << filename << "\n";
        return lm;
    }

    std::string line;
    getline(file, line); // skip description
    int maxSteps=0,numShells=0,rows=0,cols=0;
    getline(file,line); sscanf(line.c_str(),"MaxSteps = %d",&maxSteps);
    getline(file,line); sscanf(line.c_str(),"NumShells = %d",&numShells);
    getline(file,line); sscanf(line.c_str(),"Rows = %d",&rows);
    getline(file,line); sscanf(line.c_str(),"Cols = %d",&cols);

    std::vector<std::vector<cell>> arr;
    arr.reserve(cols);
    for (int i=0;i<cols;i++) {
        std::vector<cell> col;
        col.reserve(rows);
        for (int j=0;j<rows;j++) col.emplace_back(i,j);
        arr.push_back(std::move(col));
    }
    auto board = std::make_unique<game_board>(cols,rows,std::move(arr));

    std::set<char> valid = {'#','1','2',' ','@'};
    std::vector<int> tank_counters(2,0);

    for (int j=0;j<rows;j++) {
        getline(file,line);
        for (int i=0;i<cols;i++) {
            char ch = (i<(int)line.size())?line[i]:' ';
            if (!valid.count(ch)) ch=' ';
            cell& c = board->get_cell(i,j);
            if (ch=='#') c.add_Object(std::make_shared<wall>('#',&c));
            else if (ch=='@') c.add_Object(std::make_shared<mine>('@',&c));
            else if (ch=='1' || ch=='2') {
                int pnum = (ch=='1')?0:1;
                int tnum = ++tank_counters[pnum];
                int dirx = (pnum==0)?-1:1;
                auto t = std::make_shared<tank>(ch,pnum+1,tnum,dirx,0,&c,nullptr);
                board->tanks.push_back(t);
                c.add_Object(t);
            }
        }
    }
    file.close();

    auto p1 = pf(1,cols,rows,maxSteps,numShells);
    auto p2 = pf(2,cols,rows,maxSteps,numShells);

    lm.width=cols; lm.height=rows; lm.maxSteps=maxSteps; lm.numShells=numShells;
    lm.view.updateCopy(*board);
    lm.board=std::move(board); lm.p1=std::move(p1); lm.p2=std::move(p2);
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

// -------------------------------
// MAIN
// -------------------------------
int main(int argc, char* argv[]) {
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

    if (comparative==competition) { printUsage("Must specify exactly one of -comparative or -competition"); return 1;}

    // -------------------------------
    // Comparative mode
    // -------------------------------
    if (comparative) {
        if (!args.count("game_map")||!args.count("game_managers_folder")||!args.count("algorithm1")||!args.count("algorithm2")) {
            printUsage("Missing args for comparative"); return 1;
        }
        std::string mapfile=args["game_map"];
        std::string gmFolder=args["game_managers_folder"];
        std::string algo1=args["algorithm1"], algo2=args["algorithm2"];
        size_t numThreads = args.count("num_threads") ? std::stoul(args["num_threads"]) : 1;

        std::cout<<"[MODE] Comparative\n";
        if (verbose) std::cout<<"[FLAG] Verbose enabled\n";

        // load the 2 algorithms once
        AlgorithmRegistrar::getAlgorithmRegistrar().clear();
        loadSO(algo1,algo1);
        loadSO(algo2,algo2);
        auto& registrar = AlgorithmRegistrar::getAlgorithmRegistrar();
        if (registrar.count() < 2) {
            std::cerr<<"Need two algorithms\n";
            return 1;
        }
        auto it = registrar.begin();
        auto algoEntry1 = *it;
        auto algoEntry2 = *(++it);

        // scan for all GMs
        std::vector<std::string> gmFiles;
        for (auto& entry: fs::directory_iterator(gmFolder)) {
            if (entry.path().extension()==".so") gmFiles.push_back(entry.path().string());
        }
        if (gmFiles.empty()) { std::cerr<<"No game managers in "<<gmFolder<<"\n"; return 1; }

        std::mutex resultsMutex;
        std::vector<GameResult> allResults;
        std::vector<std::thread> threads;

        auto worker = [&](const std::string& gmFile){
            loadSO(gmFile, gmFile);
            auto& gmReg = GameManagerRegistrar::get();
            if (gmReg.empty()) return;
            auto gm = gmReg.last()();

            PlayerFactory pf = [&](int p, size_t x, size_t y, size_t maxS, size_t numS){
                if (p==1) return algoEntry1.createPlayer(p,x,y,maxS,numS);
                else      return algoEntry2.createPlayer(p,x,y,maxS,numS);
            };
            TankAlgorithmFactory tankFactory1=[&](int p,int t){ return algoEntry1.createTankAlgorithm(p,t); };
            TankAlgorithmFactory tankFactory2=[&](int p,int t){ return algoEntry2.createTankAlgorithm(p,t); };

            LoadedMap map=buildMapFromFile(mapfile,pf);

           GameResult res = gm->run(
            map.width, map.height, map.view, mapfile,
            map.maxSteps, map.numShells,
            *map.p1, "Algorithm1",
            *map.p2, "Algorithm2",
            tankFactory1, tankFactory2
                );

            std::lock_guard<std::mutex> lk(resultsMutex);
            allResults.push_back(std::move(res));
        };

        size_t active=0;
        for (auto& gmFile: gmFiles) {
            threads.emplace_back(worker,gmFile);
            if (++active==numThreads) {
                for (auto& t: threads) t.join();
                threads.clear(); active=0;
            }
        }
        for (auto& t: threads) t.join();

        std::cout<<"=== Comparative Results ===\n";
        for (auto& r: allResults) {
            std::cout<<"Winner: "<<r.winner<<" Reason: "<<r.reason<<"\n";
        }
    }

    // -------------------------------
    // Competition mode
    // -------------------------------
    if (competition) {
        if (!args.count("game_maps_folder")||!args.count("game_manager")||!args.count("algorithms_folder")) {
            printUsage("Missing args for competition"); return 1;
        }
        std::string mapsFolder=args["game_maps_folder"];
        std::string gmFile=args["game_manager"];
        std::string algFolder=args["algorithms_folder"];
        int numThreads=args.count("num_threads")?std::stoi(args["num_threads"]):1;


        std::cout<<"[MODE] Competition\n";
        if (verbose) std::cout<<"[FLAG] Verbose enabled\n";

        // load GM
        loadSO(gmFile,gmFile);
        auto& gmReg = GameManagerRegistrar::get();
        if (gmReg.empty()) {
            std::cerr << "No game managers registered!\n";
            return 1;
        }
        auto gm = gmReg.last()();  

        // load algorithms
        std::vector<std::string> algFiles;
        for (auto& entry: fs::directory_iterator(algFolder)) {
            if (entry.path().extension()==".so")
                algFiles.push_back(entry.path().string());
        }
        if (algFiles.empty()) { std::cerr<<"No algorithms in "<<algFolder<<"\n"; return 1; }

        // maps
        std::vector<std::string> maps;
        for (auto& entry: fs::directory_iterator(mapsFolder)) {
            if (entry.path().extension()==".txt") maps.push_back(entry.path().string());
        }

        std::mutex scoreMutex;
        std::map<std::string,int> scores;
        std::vector<std::thread> threads;

        auto worker = [&](const std::string& mapFile,const std::string& algoFile){
            AlgorithmRegistrar::getAlgorithmRegistrar().clear();
            loadSO(algoFile,algoFile);
            auto& reg=AlgorithmRegistrar::getAlgorithmRegistrar();
            if (reg.count() == 0) return;
            auto& algoEntry=*(reg.end()-1);

            PlayerFactory pf=[&](int p,size_t x,size_t y,size_t maxS,size_t numS){
                return algoEntry.createPlayer(p,x,y,maxS,numS);
            };
            TankAlgorithmFactory tankFactory=[&](int p,int t){ return algoEntry.createTankAlgorithm(p,t); };

            LoadedMap map=buildMapFromFile(mapFile,pf);

            GameResult res = gm->run(
            map.width, map.height, map.view, mapFile,
            map.maxSteps, map.numShells,
            *map.p1, algoFile,
            *map.p2, algoFile,
            tankFactory, tankFactory
        );

            std::lock_guard<std::mutex> lk(scoreMutex);
            if (res.winner==1) scores[algoFile]+=3;
            else if (res.winner==2) scores[algoFile]+=3;
            else scores[algoFile]+=1;
        };

        for (auto& m: maps) {
            for (auto& a: algFiles) {
                threads.emplace_back(worker,m,a);
                if (threads.size()>=(size_t)numThreads) {
                    for (auto& t: threads) t.join();
                    threads.clear();
                }
            }
        }
        for (auto& t: threads) t.join();

        std::cout<<"=== Competition Results ===\n";
        for (auto& [alg,score]: scores) {
            std::cout<<alg<<": "<<score<<"\n";
        }
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