#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <cmath>
#include <cstdlib>

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include "assignment.h"

using json = nlohmann::json;

// earth's radius
const float R = 6371.0;

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
    // example api req: https://us1.locationiq.com/v1/search?key=YOUR_API_KEY&q=Statue%20of%20Liberty,%20New%20York&format=json

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

// haversine formula
// distance = earth's radius * c
float haversine(float lat1, float lon1, float lat2, float lon2) 
{
    // this does not take actual roads into account
    // it calculates the distance from point a to b in a direct (albeit CURVED, ASSUMING the earth is round) line
     
    // delta lat / lan (converted to radian)
    float dlat = (lat2 - lat1) * M_PI / 180.0;
    float dlon = (lon2 - lon1) * M_PI / 180.0;

    // lats also converted to radian
    lat1 = lat1 * M_PI / 180.0;
    lat2 = lat2 * M_PI / 180.0;

    float a = sin(dlat / 2) * sin(dlat / 2) + cos(lat1) * cos(lat2) * sin(dlon / 2) * sin(dlon / 2);

    float c = 2 * atan2(sqrt(a), sqrt(1 - a));

    return R * c;
}

int main(int argc, char** argv) {
    // parse the address data from the json file
    // TODO: gain access to the roster/planning (hopefully as json) on ecologieconnect.nl
    // this could be a daily json with the roster of available people

    // I/O employees + targets data
    json j = loadJsonFile("../addresstest.json");  
    json jt = loadJsonFile("../targettest.json");    

    // vectors in which addresses/targets/distances will be stored
    std::vector<Employee> employees;
    std::vector<Target> targets;
    std::vector<Distance> distances;
    std::vector<No_pair> no_pairs;
    std::vector<std::pair<int, std::vector<int>>> friend_groups;

    // populate employees vector
    for (const auto& item : j){
        int id = item["id"];
        std::string name = item["name"];
        std::string address = item["address"];
        std::string city = item["city"];

        std::vector<std::string> no_pair;        
        // get the no-pairs from json if there are any, should be a list in the json file for the biggest freak 
        if (item.contains("no_pair") && item["no_pair"].is_array()) {
            no_pair = item["no_pair"].get<std::vector<std::string>>();
        } else {
            no_pair = {}; 
        }

        std::vector<std::string> friends; 
        // same for friends (or people who'd prefer to work together)
        if (item.contains("friends") && item["friends"].is_array()) {
            friends = item["friends"].get<std::vector<std::string>>();
        } else {
            friends = {}; 
        }

        // make an Employee object, using employee data and push it into the employees vector
        employees.push_back(Employee{id, name, address, city, no_pair, friends});
    }

    // names mapped to id
    std::unordered_map<std::string, int> name_to_id;
    for (const auto& emp : employees) {
        name_to_id[emp.name] = emp.id;
    }

    // print the employees who have enemies and do not wish to work together with said persons
    // as well as their friends 
    for (const auto& emp : employees) {
        if (!emp.friends.empty()) {
            std::cout << "employee: " << emp.name << " has the following friends: ";
            for (const auto& f : emp.friends) {
                std::cout << f << ", ";
            }
            std::cout << std::endl;

            std::vector<int> friend_ids;
            for (const auto& f_name : emp.friends) {
                // check if friend exists in name_to_id map
                if (name_to_id.find(f_name) != name_to_id.end()) {
                    friend_ids.push_back(name_to_id[f_name]);
                }
            }
            friend_groups.emplace_back(emp.id, friend_ids);
        }
        if (!emp.no_pair.empty()) {
            std::cout << "employee: " << emp.name << " does not like the following people: ";
            for (const auto& p : emp.no_pair) {
                std::cout << p << ", ";
            }
            std::cout << std::endl;

            for (const auto& p_name : emp.no_pair) {
                for (const auto& other_emp : employees) {
                    if (p_name == other_emp.name) {
                        std::cout << p_name << " found in the shit-list of : " << emp.name << std::endl;
                        no_pairs.emplace_back(emp.id, other_emp.id);
                    }
                }
            }
        }
    }

    // these pairs ought not be at the same location:
    for (const auto& np : no_pairs) {
        std::cout << "(" << np.first << ", " << np.second << ")" << std::endl;
    }

    // these friends ought to stick together
    for (const auto& f_group : friend_groups) {
        std::cout << "employee id: " << f_group.first << " has friends with IDs: ";
        for (int f_id : f_group.second) {
            std::cout << f_id << ", ";
        }
        std::cout << std::endl;
    }

    // populate target vector
    for (const auto& item : jt){
        int target_number = item["target_number"];
        std::string address = item["address"];
        std::string city = item["city"];
        std::string country = item["country"];
        int req_emp = item["req_employees"];

        targets.push_back(Target{target_number, address, city, country, req_emp});
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

        std::this_thread::sleep_for(std::chrono::milliseconds(501));
    }

    std::cout << "forward-geolocating employees' addresses..." << std::endl;

    for (auto& emp : employees) {
        // std::cout << "sending request for: " << emp.name << ", " << emp.address << ", " << emp.city << std::endl;            
        std::string query = emp.city + ", " + emp.address + ", Netherlands";

        cpr::Response r = forwardGeolocate(query, apiKey);
        r.status_code;
        r.text;
        
        // parse the response and set emp.lon/lat
        if (r.status_code == 200) {            
            try {
                json response = json::parse(r.text);

                if (!response.empty()) {
                    emp.lat = std::stof(response[0]["lat"].get<std::string>());
                    emp.lon = std::stof(response[0]["lon"].get<std::string>());               

                    std::cout << "name: " << emp.name << ", lat: " << emp.lat << ", lon: " << emp.lon << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "JSON parse error: " << e.what() << std::endl;
            }
        } else {
            std::cout << "request failed, code: " << r.status_code << std::endl;
        }

        // TODO:
        // save result to file or csv.
        // in the future, check file beforehand and only 
        // make this request if lon/lat isn't known
        // (so in the case of a new employee. be mindful of employee moving)

        // sleep to obey api rate limit (2/s)
        std::this_thread::sleep_for(std::chrono::milliseconds(501));        
    } 

    // calc the distance using haversine helper func and populate distances vector
    for (const auto& tar : targets) {
        for (const auto& emp : employees) {
            Distance distance;
            distance.target = tar;
            distance.employee = emp;
            distance.distance = haversine(tar.lat, tar.lon, emp.lat, emp.lon);

            distances.push_back(distance);
        }
    }

    // log distance for every combination (can be ommitted for perf)
    for (const auto& d : distances) {
        std::cout << "distance between: " << "(target_number: " << d.target.target_number << "- req." << d.target.req_employees << ") " << d.target.address << " and " << "(" << d.employee.name << ":" << d.employee.id << ") " << d.employee.address << ": " << d.distance << "km" << std::endl;
    }

    // before assignment, check if there are enough employees available to hit every target requirement
    int num_employees = employees.size();
    int sum = 0;

    for (const auto& t : targets) {
        sum += t.req_employees; 
    }

    if (sum > num_employees) {
        std::cout << "Not enough resources! The total employee requirement for the targets is: " << sum << " and the total available employees is: " << num_employees << std::endl;
    } else {
        // use google OR tools for assignment optimization
        // TODO: based on command line input, run different function   
             
        // operations_research::assignEmployeesBalanced(distances, employees, targets);    
        // operations_research::assignEmployees(distances, employees, targets);       
        operations_research::assignEmployeesEnemiesAndFriends(distances, employees, targets, no_pairs, friend_groups);
    }

    return 0;
}