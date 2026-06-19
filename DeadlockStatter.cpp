// DeadlockStatter.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <math.h>
#include <string.h>
#include "include/httplib.h"
#include "include/rapidjson/document.h"     //Line for when we want to start including RapidJSON stuff
using namespace std;

void fetchAndAnalyzePublicMatches() {
    // 1. Set up the HTTP Client
    httplib::Client cli("https://api.deadlock-api.com");

    // 2. Change the GET request to fetch a batch of recent public matches (e.g., limit=100)
    std::string endpoint = "/v1/matches/public?limit=100";
    std::cout << "Requesting bulk public match data...\n";

    if (auto res = cli.Get(endpoint)) {
        if (res->status == 200) {

            // 3. Initialize RapidJSON Document and parse the response body
            rapidjson::Document doc;
            doc.Parse(res->body.c_str());

            // Check for parsing errors
            if (doc.HasParseError()) {
                std::cout << "Error: Failed to parse JSON response. Code: "
                    << doc.GetParseError() << "\n";
                return;
            }

            // Ensure our expected "matches" array exists
            if (!doc.HasMember("matches") || !doc["matches"].IsArray()) {
                std::cout << "Error: Response missing 'matches' array.\n";
                return;
            }

            // Statistical counters
            int correct_predictions = 0;
            int total_analyzed = 0;

            // 4. Loop through the array of matches
            const rapidjson::Value& matchesArray = doc["matches"];
            for (rapidjson::SizeType i = 0; i < matchesArray.Size(); ++i) {
                const auto& match = matchesArray[i];

                // RapidJSON requires explicit type handling (.GetUint64(), .GetString())
                uint64_t match_id = match["match_id"].GetUint64();
                std::string winner = match["winning_team"].GetString();

                if (match.HasMember("timeline") && match["timeline"].IsArray()) {
                    const auto& timelineArray = match["timeline"];

                    // Look for the 10-minute snapshot inside this match
                    for (rapidjson::SizeType j = 0; j < timelineArray.Size(); ++j) {
                        const auto& snapshot = timelineArray[j];

                        if (snapshot["timestamp_seconds"].GetInt() == 600) {
                            int amber_souls = snapshot["amber_team_souls"].GetInt();
                            int sapphire_souls = snapshot["sapphire_team_souls"].GetInt();

                            // Determine who was ahead at 10 minutes
                            std::string leader_at_10 = (amber_souls > sapphire_souls) ? "Amber" : "Sapphire";

                            if (leader_at_10 == winner) {
                                correct_predictions++;
                            }
                            total_analyzed++;
                            break; // Stop looking through this match's timeline
                        }
                    }
                }
            }

            // 5. Output batch analytics results
            std::cout << "\n================ BATCH ANALYSIS ================\n";
            std::cout << "Total Matches Processed: " << total_analyzed << "\n";
            if (total_analyzed > 0) {
                double win_rate = (static_cast<double>(correct_predictions) / total_analyzed) * 100.0;
                std::cout << "Accuracy of 10-min lead predicting winner: " << win_rate << "%\n";
            }
            std::cout << "================================================\n";

        }
        else {
            std::cout << "Error: API returned status code " << res->status << "\n";
        }
    }
    else {
        std::cout << "Error: Could not connect to the API server.\n";
    }
}

int main()
{
    std::cout << "Hello World!\n";
    fetchAndAnalyzePublicMatches();
    //TODO: Create a connection with the Deadlock API
    //TODO: Extract data from the API using an https GET
    //TODO: Parse that data into JSON objects
    //TODO: Create tools to run numerical analysis of the data
    //TODO: Create a console interface to allow the user to interact with the program and display data
    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
