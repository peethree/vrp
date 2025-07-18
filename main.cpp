#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <string>

using json = nlohmann::json;

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

        // std::cout << "name: " << name << std::endl;        
        // std::cout << "address: " << address << std::endl;
        // std::cout << "city: " << city << std::endl;

        addresses.push_back(Address{name, address, city});
    }

    // consider sending all the addresses in 1 big batch to limit network requests
    // TODO: api requests to locationiq
    for (const auto& addr : addresses) {
        std::cout << "sending request for: " << addr.name << ", " << addr.address << ", " << addr.city << std::endl;        
    }

    // get api key from env
    const char* apiKey = std::getenv("LIQ_API_KEY");
    if (apiKey) {
        std::cout << "API_KEY: " << apiKey << std::endl;
    }
    
    // https://us1.locationiq.com/v1/search?key=YOUR_API_KEY&q=Statue%20of%20Liberty,%20New%20York&format=json

    // format a test query
    std::string first = addresses[0].city;
    std::string second = addresses[0].address;
    std::string query = first + ", " + second;    

    cpr::Response r = cpr::Get(
        cpr::Url{"https://us1.locationiq.com/v1/search"},
        cpr::Parameters{
            {"key", apiKey}, 
            {"q", query},
            {"format", "json"}
        });
    r.status_code;                  // 200
    r.header["content-type"];       // application/json; charset=utf-8
    r.text;
    
    if (r.status_code == 200) {
        // Print the response text
        std::cout << "Response Text: " << r.text << std::endl;
    } else {
        std::cout << "Request failed with status code: " << r.status_code << std::endl;
    }

    return 0;
}

// http request geocoding API -> home address to long/lat

    // cpr HTTP req
    // parse the addresses (json) with nlohmann/json
    // send a geocoding api request with the data of the address

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