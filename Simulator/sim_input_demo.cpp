#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>

// --- usage printer ---
void printUsage(const std::string& msg = "") {
    if (!msg.empty()) {
        std::cerr << "[ERROR] " << msg << "\n";
    }
    std::cerr << "Usage:\n"
              << "Comparative mode:\n"
              << "  ./sim_input_demo -comparative game_map=<file> "
                 "game_managers_folder=<folder> algorithm1=<file> algorithm2=<file> "
                 "[num_threads=<n>] [-verbose]\n"
              << "Competition mode:\n"
              << "  ./sim_input_demo -competition game_maps_folder=<folder> "
                 "game_manager=<file> algorithms_folder=<folder> "
                 "[num_threads=<n>] [-verbose]\n";
}

// --- main ---
int main(int argc, char* argv[]) {
    bool comparative = false;
    bool competition = false;
    bool verbose = false;
    std::unordered_map<std::string,std::string> args;
    std::vector<std::pair<std::string,std::string>> orderedArgs;

    for (int i = 1; i < argc; ++i) {
        std::string token(argv[i]);
        if (token == "-comparative") {
            comparative = true;
        } else if (token == "-competition") {
            competition = true;
        } else if (token == "-verbose") {
            verbose = true;
        } else {
            auto pos = token.find('=');
            if (pos == std::string::npos) {
                printUsage("Unsupported argument: " + token);
                return 1;
            }
            std::string key = token.substr(0,pos);
            std::string val = token.substr(pos+1);

            // trim spaces
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t")+1);
            val.erase(0, val.find_first_not_of(" \t"));
            val.erase(val.find_last_not_of(" \t")+1);

            args[key] = val;
            orderedArgs.push_back({key, val});
        }
    }

    if (comparative == competition) {
        printUsage("You must specify exactly one of -comparative or -competition.");
        return 1;
    }

    // --- Mandatory arg validation ---
    if (comparative) {
        std::vector<std::string> needed = {"game_map","game_managers_folder","algorithm1","algorithm2"};
        std::vector<std::string> missing;
        for (auto& k : needed) {
            if (!args.count(k)) missing.push_back(k);
        }
        if (!missing.empty()) {
            std::string msg = "Missing args:";
            for (auto& m : missing) msg += " " + m;
            printUsage(msg);
            return 1;
        }

        // check for unsupported keys
        std::unordered_map<std::string,bool> allowed;
        for (auto& k : needed) allowed[k] = true;
        allowed["num_threads"] = true;
        for (auto& [k,v] : args) {
            if (!allowed.count(k)) {
                printUsage("Unsupported argument key: " + k);
                return 1;
            }
        }

        std::cout << "[MODE] Comparative\n";
    }

    if (competition) {
        std::vector<std::string> needed = {"game_maps_folder","game_manager","algorithms_folder"};
        std::vector<std::string> missing;
        for (auto& k : needed) {
            if (!args.count(k)) missing.push_back(k);
        }
        if (!missing.empty()) {
            std::string msg = "Missing args:";
            for (auto& m : missing) msg += " " + m;
            printUsage(msg);
            return 1;
        }

        // check for unsupported keys
        std::unordered_map<std::string,bool> allowed;
        for (auto& k : needed) allowed[k] = true;
        allowed["num_threads"] = true;
        for (auto& [k,v] : args) {
            if (!allowed.count(k)) {
                printUsage("Unsupported argument key: " + k);
                return 1;
            }
        }

        std::cout << "[MODE] Competition\n";
    }

    if (verbose) {
        std::cout << "[FLAG] Verbose enabled\n";
    }

    for (auto& [k,v] : orderedArgs) {
        std::cout << "[ARG] " << k << " = " << v << "\n";
    }

    return 0;
}


// g++ -std=c++17 -O2 Simulator/sim_input_demo.cpp -o sim_input_demo

//  ./sim_input_demo -comparative game_map=map1.txt game_managers_folder=managers algorithm1=a1.so algorithm2=a2.so
// >> 
// [MODE] Comparative
// [ARG] game_map = map1.txt
// [ARG] game_managers_folder = managers
// [ARG] algorithm1 = a1.so
// [ARG] algorithm2 = a2.so
// PS C:\Users\shaicool\Desktop\advanced\3\Tanks---task-1> ./sim_input_demo algorithm2=a2.so -verbose game_managers_folder=managers -comparative num_threads=8 algorithm1=a1.so game_map=map1.txt    
// >>
// [MODE] Comparative
// [FLAG] Verbose enabled
// [ARG] algorithm2 = a2.so
// [ARG] game_managers_folder = managers
// [ARG] num_threads = 8
// [ARG] algorithm1 = a1.so
// [ARG] game_map = map1.txt
// PS C:\Users\shaicool\Desktop\advanced\3\Tanks---task-1> ./sim_input_demo -competition game_maps_folder=maps game_manager=gm.so algorithms_folder=algs
// >>
// [MODE] Competition
// [ARG] game_maps_folder = maps
// [ARG] game_manager = gm.so
// [ARG] algorithms_folder = algs
// PS C:\Users\shaicool\Desktop\advanced\3\Tanks---task-1> ./sim_input_demo num_threads=2 -verbose algorithms_folder=algs game_maps_folder=maps -competition game_manager=gm.so
// >>
// [MODE] Competition
// [FLAG] Verbose enabled
// [ARG] num_threads = 2
// [ARG] algorithms_folder = algs
// [ARG] game_maps_folder = maps
// [ARG] game_manager = gm.so
// PS C:\Users\shaicool\Desktop\advanced\3\Tanks---task-1> ./sim_input_demo -comparative game_map=map1.txt game_managers_folder=managers algorithm1=a1.so
// >>
// [ERROR] Missing args: algorithm2
// Usage:
// Comparative mode:
//   ./sim_input_demo -comparative game_map=<file> game_managers_folder=<folder> algorithm1=<file> algorithm2=<file> [num_threads=<n>] [-verbose]
// Competition mode:
//   ./sim_input_demo -competition game_maps_folder=<folder> game_manager=<file> algorithms_folder=<folder> [num_threads=<n>] [-verbose]
// PS C:\Users\shaicool\Desktop\advanced\3\Tanks---task-1> ./sim_input_demo -competition maps=folder algorithms_folder=algs game_manager=gm.so
// >>
// [ERROR] Missing args: game_maps_folder
// Usage:
// Comparative mode:
//   ./sim_input_demo -comparative game_map=<file> game_managers_folder=<folder> algorithm1=<file> algorithm2=<file> [num_threads=<n>] [-verbose]
// Competition mode:
//   ./sim_input_demo -competition game_maps_folder=<folder> game_manager=<file> algorithms_folder=<folder> [num_threads=<n>] [-verbose]