#ifndef HELPER_H
#define HELPER_H

#include <vector>
#include <string>

// add country???
struct Employee {
    int id;
    std::string name;
    std::string address;
    std::string city;
    std::vector<std::string> no_pair;
    std::vector<std::string> friends;
    float lon;
    float lat;
};

struct No_pair {
    int first;
    int second;

    // constructor to initialize the fields
    No_pair(int f, int s) : first(f), second(s) {}
};

struct Target {
    int target_number;
    std::string address;
    std::string city;
    std::string country;
    int req_employees;
    float lon;
    float lat;
    // TODO: add a time slot to the struct (evening, night) so employees can be assigned to evening + night in the future
};

struct Distance {
    Employee employee;
    Target target;
    float distance;
};

namespace operations_research {
    void assignEmployees(std::vector<Distance>& distances, std::vector<Employee>& employees, std::vector<Target>& targets);
    void assignEmployeesBalanced(std::vector<Distance>& distances, std::vector<Employee>& employees, std::vector<Target>& targets);
    void assignEmployeesEnemiesAndFriends(std::vector<Distance>& distances, std::vector<Employee>& employees, std::vector<Target>& targets, std::vector<No_pair>& conflicts, std::vector<std::pair<int, std::vector<int>>> &friend_groups);
}

#endif 