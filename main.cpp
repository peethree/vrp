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
};

int main(int argc, char** argv) {
    // parse the address data from the json file
    // TODO: gain access to the roster/planning (hopefully as json) on ecologieconnect.nl
    std::ifstream inputFile("../address.json");

    if (!inputFile.is_open()) {
        std::cerr << "can't open address.json" << std::endl;
        return 1;
    }
    
    json j;
    inputFile >> j;

    inputFile.close();

    // vector in which addresses will be stored
    std::vector<Address> addresses;

    for (const auto& item: j){
        std:: string name = item["name"];
        std:: string address = item["address"];
        std:: string city = item["city"];

        // make an Address object, using employee data and push it into the addresses vector
        addresses.push_back(Address{name, address, city});
    }

    // get api key from env
    const char* apiKey = std::getenv("LIQ_API_KEY");
    // if (apiKey) {
    //     std::cout << "API_KEY: " << apiKey << std::endl;
    // }

    std::cout << "forward geolocating employees' addresses..." << std::endl;

    for (const auto& addr : addresses) {
        // std::cout << "sending request for: " << addr.name << ", " << addr.address << ", " << addr.city << std::endl;    
        
        // https://eu1.locationiq.com/v1/search?key=YOUR_API_KEY&q=Statue%20of%20Liberty,%20New%20York&format=json
        
        std::string query = addr.city + ", " + addr.address + ", Netherlands";

        // TODO: make sure the query isn't ambiguous (can return multiple objects)
        // consider using country, postal code as well.
        cpr::Response r = cpr::Get(
            cpr::Url{"https://eu1.locationiq.com/v1/search"},
            cpr::Parameters{
                {"key", apiKey}, 
                {"q", query},
                {"format", "json"}
            });
        r.status_code;                  
        // r.header["content-type"];       // application/json; charset=utf-8
        r.text;
        
        // parse the response and get lon/lat
        if (r.status_code == 200) {            
            try {
                json response = json::parse(r.text);
                if (!response.empty()) {
                    std::string lat = response[0]["lat"];
                    std::string lon = response[0]["lon"];                

                    std::cout << "name: " << addr.name << "lat: " << lat << ", lon: " << lon << std::endl;
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