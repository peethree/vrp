#include <cpr/cpr.h>
#include <iostream>

int main(int argc, char** argv) {
    // std::cout << "Hello, world" << std::endl;

    cpr::Response r = cpr::Get(cpr::Url{"https://api.github.com/repos/whoshuu/cpr/contributors"},
                      cpr::Authentication{"user", "pass", cpr::AuthMode::BASIC},
                      cpr::Parameters{{"anon", "true"}, {"key", "value"}});
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
    // nlohmann/json parse JSON

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