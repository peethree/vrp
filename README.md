# vrp

trying to solve constrained assignment optimization for EC field work.

converts human readable addresses of both the target and the employees' addresses into geolocations. Then tries to calculate the shortest distance for all combinations of locations + batches of employees. For example: if 4 out of the 50 employees have to attend at a location, then the remaining resources have to be as efficient as possible for the rest of the assignments of that evening/night etc.

Distance between employee and target won't be entirely accurate since it's calculated with haversine formula, but should be adequate enough given the extensive road network in my country.

Once all the necessary information is gathered, OR-tools from google are used to calculate the desired computation. Currently it has an option that searches for the combination that leads to the least amount of kilometers travelled: "assignEmployees" and another option to get a more balanced distribution, with less strong outliers, but this will lead to an overall longer distance travelled: "assignEmployeesBalanced".

![](ss.png)

#### dependencies
- cpr
- nlohmann/json
- [google or-tools](https://developers.google.com/optimization/install/cpp/binary_linux)
- [locationiq](https://locationiq.com/) api key - the free tier of this api has a rate limit of 2 requests/s and 5000 requests/day. 

In my case cpr and nlohmann/json were installed with vcpkg and or-tools was installed manually in the home directory.
The api key needs to be retrievable from env (with std::getenv("LIQ_API_KEY")). In my case I added the following to 
.basrc: 'export LIQ_API_KEY="..."' The executable is made with CMake. 

#### future TODOs
- make something of a GUI
- cache lon/lats from employees so the program is bottlenecked less by the api request rate limit
- export the assignments into a document.
- add support for assigning an employee twice on the same day (evening + night shift)