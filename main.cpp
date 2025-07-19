#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <string>
#include <chrono>
#include <thread>

using json = nlohmann::json;

// add country???
struct Address {
    std::string name;
    std::string address;
    std::string city;
    float lon;
    float lat;
};

struct Target {
    int target_number;
    std::string address;
    std::string city;
    std::string country;
    float lon;
    float lat;
};

json loadJsonFile(const std::string& path) 
{
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "can't open " << path << "..." << std::endl;
        exit(1);
    }    

    json j;
    file >> j;
    return j;
}

cpr::Response forwardGeolocate(std::string query, const char* apiKey)
{
    // TODO: make sure the query isn't ambiguous (can return multiple objects)
    // consider using country, postal code as well.
    cpr::Response r = cpr::Get(
        cpr::Url{"https://eu1.locationiq.com/v1/search"},
        cpr::Parameters{
            {"key", apiKey}, 
            {"q", query},
            {"format", "json"}
        });
    return r;
}

int main(int argc, char** argv) {
    // parse the address data from the json file
    // TODO: gain access to the roster/planning (hopefully as json) on ecologieconnect.nl
    // this could be a daily json with the roster of available people

    // I/O addresses + targets
    json j = loadJsonFile("../address.json");  
    json jt = loadJsonFile("../target.json");    

    // vectors in which addresses/targets will be stored
    std::vector<Address> addresses;
    std::vector<Target> targets;

    // populate addresses vector
    for (const auto& item: j){
        std::string name = item["name"];
        std::string address = item["address"];
        std::string city = item["city"];

        // make an Address object, using employee data and push it into the addresses vector
        addresses.push_back(Address{name, address, city});
    }

    // populate target vector
    for (const auto& item: jt){
        int target = item["target_number"];
        std::string address = item["address"];
        std::string city = item["city"];
        std::string country = item["country"];

        targets.push_back(Target{target, address, city, country});
    }

    // get api key from env
    const char* apiKey = std::getenv("LIQ_API_KEY");
    // if (apiKey) {
    //     std::cout << "API_KEY: " << apiKey << std::endl;
    // }

    std::cout << "forward-geolocating target addresses..." << std::endl;

    for (auto& tar : targets) {
        // std::cout << "tar: " << tar.target_number << ", address: " << tar.address << ", city: " << tar.city << ", country: " << tar.country << ",lon: " << tar.lon << ",lat: " << tar.lat << std::endl;
        std::string query = tar.city + ", " + tar.address + ", " + tar.country;

        cpr::Response rt = forwardGeolocate(query, apiKey); 
        rt.status_code;
        rt.text;

        if (rt.status_code == 200) {            
            try {
                json response = json::parse(rt.text);

                if (!response.empty()) {
                    // api returns lon/lat as string, convert to float (maybe useful for later)
                    tar.lat = std::stof(response[0]["lat"].get<std::string>());
                    tar.lon = std::stof(response[0]["lon"].get<std::string>());               

                    std::cout << "tar#: " << tar.target_number << ", lat: " << tar.lat << ", lon: " << tar.lon << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "JSON parse error: " << e.what() << std::endl;
            }
        } else {
            std::cout << "request failed, code: " << rt.status_code << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1001));
    }

    std::cout << "forward-geolocating employees' addresses..." << std::endl;

    for (auto& addr : addresses) {
        // std::cout << "sending request for: " << addr.name << ", " << addr.address << ", " << addr.city << std::endl;    
        
        // https://eu1.locationiq.com/v1/search?key=YOUR_API_KEY&q=Statue%20of%20Liberty,%20New%20York&format=json
        
        std::string query = addr.city + ", " + addr.address + ", Netherlands";

        cpr::Response r = forwardGeolocate(query, apiKey);
        r.status_code;
        r.text;
        
        // parse the response and get lon/lat
        if (r.status_code == 200) {            
            try {
                json response = json::parse(r.text);

                if (!response.empty()) {
                    addr.lat = std::stof(response[0]["lat"].get<std::string>());
                    addr.lon = std::stof(response[0]["lon"].get<std::string>());               

                    std::cout << "name: " << addr.name << ", lat: " << addr.lat << ", lon: " << addr.lon << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "JSON parse error: " << e.what() << std::endl;
            }
        } else {
            std::cout << "request failed, code: " << r.status_code << std::endl;
        }

        // TODO: add lon / lat fields to Address and populate them.
        // save result to file or csv.
        // in the future, check file beforehand and only 
        // make this request if lon/lat isn't known
        // (so in the case of a new employee. be mindful of employee moving)

        // sleep to obey api rate limit
        std::this_thread::sleep_for(std::chrono::milliseconds(1001));        
    } 

    // TODO: add some targets to compare with all the addresses of the employees.    

    return 0;
}

// http request geocoding API -> home address to long/lat

    // cpr HTTP req
    // parse the addresses (json) with nlohmann/json
    // send a geocoding api request with the data of the addresses

// OSRM /table api for distance/time matrix

    // http://localhost:5000/table/v1/driving/<long1>,<lat1>;<long2>,<lat2>;... ?annotations=distance
    /*

    result:
    {
    "distances": [
            [0, 4000, 7000],
            [4000, 0, 3000],
            [7000, 3000, 0]
        ]
    }

    */
    
    // use this to compute total dist

    // optimize the routes. greedy algo or google OR-tools VRP solution

    // print assignments.

    // keep track of the people who haven't been assigned and list them
    // in case someone can't make it.