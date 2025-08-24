#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <filesystem>
#include <algorithm>

// forward decls for your runner functions
int runComparative(const std::unordered_map<std::string,std::string>& args, bool verbose);
int runCompetition(const std::unordered_map<std::string,std::string>& args, bool verbose);

void printUsage(const std::string& msg = "") {
    if (!msg.empty()) {
        std::cerr << "[ERROR] " << msg << "\n";
    }
    std::cerr << "Usage:\n"
              << "Comparative mode:\n"
              << "  ./simulator_<ids> -comparative game_map=<file> "
                 "game_managers_folder=<folder> algorithm1=<file> algorithm2=<file> [num_threads=<n>] [-verbose]\n"
              << "Competition mode:\n"
              << "  ./simulator_<ids> -competition game_maps_folder=<folder> "
                 "game_manager=<file> algorithms_folder=<folder> [num_threads=<n>] [-verbose]\n";
}

int main(int argc, char* argv[]) {
    bool comparative = false;
    bool competition = false;
    bool verbose = false;
    std::unordered_map<std::string,std::string> args;

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
            // trim spaces around key and val
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t")+1);
            val.erase(0, val.find_first_not_of(" \t"));
            val.erase(val.find_last_not_of(" \t")+1);

            args[key] = val;
        }
    }

    if (comparative == competition) { // either none or both
        printUsage("You must specify exactly one of -comparative or -competition.");
        return 1;
    }

    if (comparative) {
        // validate mandatory keys
        std::vector<std::string> needed = {"game_map","game_managers_folder","algorithm1","algorithm2"};
        std::vector<std::string> missing;
        for (auto& k : needed) {
            if (!args.count(k)) missing.push_back(k);
        }
        if (!missing.empty()) {
            printUsage("Missing args: " + missing[0] /* (print them all ideally) */);
            return 1;
        }
        return runComparative(args, verbose);
    }

    if (competition) {
        std::vector<std::string> needed = {"game_maps_folder","game_manager","algorithms_folder"};
        std::vector<std::string> missing;
        for (auto& k : needed) {
            if (!args.count(k)) missing.push_back(k);
        }
        if (!missing.empty()) {
            printUsage("Missing args: " + missing[0]);
            return 1;
        }
        return runCompetition(args, verbose);
    }

    return 0;
}

int runComparative(const std::unordered_map<std::string,std::string>& args, bool verbose) {
    std::cout << "[INFO] Running in comparative mode\n";
    std::cout << "Map: " << args.at("game_map") << "\n";
    std::cout << "Managers folder: " << args.at("game_managers_folder") << "\n";
    std::cout << "Algorithm1: " << args.at("algorithm1") << "\n";
    std::cout << "Algorithm2: " << args.at("algorithm2") << "\n";
    if (verbose) std::cout << "Verbose ON\n";
    // TODO: load .so, run all managers, group results, write comparative_results_<time>.txt
    return 0;
}

int runCompetition(const std::unordered_map<std::string,std::string>& args, bool verbose) {
    std::cout << "[INFO] Running in competition mode\n";
    std::cout << "Maps folder: " << args.at("game_maps_folder") << "\n";
    std::cout << "GameManager .so: " << args.at("game_manager") << "\n";
    std::cout << "Algorithms folder: " << args.at("algorithms_folder") << "\n";
    if (verbose) std::cout << "Verbose ON\n";
    // TODO: load .so, enumerate algorithms, run match schedule, compute scores, write competition_<time>.txt
    return 0;
}
