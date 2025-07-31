#include "assignment.h"
#include "ortools/linear_solver/linear_solver.h"

#define FAVOR_COEFFICIENT 100.0

namespace operations_research {
/* This version of the assignment function tries to compute the combination of assignments that would
lead to the least amount of kilometers travelled. This will have some outliers. People with very short 
and very long distances */
void assignEmployees(std::vector<Distance>& distances, std::vector<Employee>& employees, std::vector<Target>& targets)
{
    int num_employees = employees.size();
    int num_targets = targets.size();

    MPSolver solver("EmployeeAssignment", MPSolver::SCIP_MIXED_INTEGER_PROGRAMMING);

    // x[i][j] = 1 if employee i is assigned to target j
    std::vector<std::vector<const MPVariable*>> x(num_employees, std::vector<const MPVariable*>(num_targets, nullptr));

    // map employee ID to its index
    std::unordered_map<int, int> id_to_index;
    for (int i = 0; i < num_employees; ++i) {
        id_to_index[employees[i].id] = i; 
    }

    // same for target (using .target_number instead of .id)
    std::unordered_map<int, int> tar_num_to_index;
    for (int i = 0; i < num_targets; ++i) {
        tar_num_to_index[targets[i].target_number] = i; 
    }

    // create the decision variables
    for (const auto& d : distances) {
        int employee_index = id_to_index[d.employee.id];
        int target_index = tar_num_to_index[d.target.target_number];

        x[employee_index][target_index] = solver.MakeIntVar(0, 1, "x_" + std::to_string(employee_index) + "_" + std::to_string(target_index));
    }

    // constraint: each employee is assigned at most once
    for (int i = 0; i < num_employees; ++i) {
        LinearExpr expr;
        for (int j = 0; j < num_targets; ++j) {
            expr += x[i][j];
        }
        solver.MakeRowConstraint(expr <= 1);
    }

    // constraint: each target has a required number of people that need to be on location
    for (int j = 0; j < num_targets; ++j) {
        LinearExpr expr;
        for (int i = 0; i < num_employees; ++i) {
            expr += x[i][j];
        }
        solver.MakeRowConstraint(expr == targets[j].req_employees);
    }

    // objective: minimize the total distance
    MPObjective* objective = solver.MutableObjective();
    for (const auto& d : distances) {
        int employee_index = id_to_index[d.employee.id];
        int target_index = tar_num_to_index[d.target.target_number];

        objective->SetCoefficient(x[employee_index][target_index], d.distance);
    }
    
    objective->SetMinimization();

    // solve
    MPSolver::ResultStatus result_status = solver.Solve();

    if (result_status == MPSolver::OPTIMAL) {
        std::cout << "Optimal assignment found!" << std::endl;
        for (const auto& d : distances) {
            int employee_index = id_to_index[d.employee.id];
            int target_index = tar_num_to_index[d.target.target_number];

            if (x[employee_index][target_index]->solution_value() > 0.5) {
                std::cout << "employee " << employees[employee_index].name << " assigned to Target:" << targets[target_index].target_number << " - "
                          << targets[target_index].address << " (distance = " << d.distance << " km)" << std::endl;
            }
        }
        std::cout << "total cost: " << objective->Value() << " km" << std::endl;
    } else {
        std::cout << "no optimal solution found..." << std::endl;
    }
}

/* This version of the assignment function tries to get a more balanced solution.
More uniform results, less outliers with very high distances, but will result in a higher overall distance travelled*/
void assignEmployeesBalanced(std::vector<Distance>& distances, std::vector<Employee>& employees, std::vector<Target>& targets)
{
    int num_employees = employees.size();
    int num_targets = targets.size();

    MPSolver solver("EmployeeAssignment", MPSolver::SCIP_MIXED_INTEGER_PROGRAMMING);

    // x[i][j] = 1 if employee i is assigned to target j
    std::vector<std::vector<const MPVariable*>> x(num_employees, std::vector<const MPVariable*>(num_targets, nullptr));

    // map employee ID to its index
    std::unordered_map<int, int> id_to_index;
    for (int i = 0; i < num_employees; ++i) {
        id_to_index[employees[i].id] = i; 
    }

    // same for target (using .target_number instead of .id)
    std::unordered_map<int, int> tar_num_to_index;
    for (int i = 0; i < num_targets; ++i) {
        tar_num_to_index[targets[i].target_number] = i; 
    }

    // create the decision variables
    for (const auto& d : distances) {
        int employee_index = id_to_index[d.employee.id];
        int target_index = tar_num_to_index[d.target.target_number];

        x[employee_index][target_index] = solver.MakeIntVar(0, 1, "x_" + std::to_string(employee_index) + "_" + std::to_string(target_index));
    }

    // constraint: each employee is assigned at most once
    for (int i = 0; i < num_employees; ++i) {
        LinearExpr expr;
        for (int j = 0; j < num_targets; ++j) {
            expr += x[i][j];
        }
        solver.MakeRowConstraint(expr <= 1);
    }

    // constraint: each target has a required number of people that need to be on location
    for (int j = 0; j < num_targets; ++j) {
        LinearExpr expr;
        for (int i = 0; i < num_employees; ++i) {
            expr += x[i][j];
        }
        solver.MakeRowConstraint(expr == targets[j].req_employees);
    }

    // calculate the average distance
    float total_distance = 0.0;
    for (const auto& d : distances) {
        total_distance += d.distance;
    }
    float average_distance = total_distance / distances.size();

    // objective: minimize the sum of squared differences from the average distance
    MPObjective* objective = solver.MutableObjective();
    for (const auto& d : distances) {
        int employee_index = id_to_index[d.employee.id];
        int target_index = tar_num_to_index[d.target.target_number];

        // calculate the squared difference from the average distance
        float squared_difference = (d.distance - average_distance) * (d.distance - average_distance);
        objective->SetCoefficient(x[employee_index][target_index], squared_difference);
    }
    
    objective->SetMinimization();

    // solve
    MPSolver::ResultStatus result_status = solver.Solve();

    if (result_status == MPSolver::OPTIMAL) {
        std::cout << "Balanced assignment found!" << std::endl;
        for (const auto& d : distances) {
            int employee_index = id_to_index[d.employee.id];
            int target_index = tar_num_to_index[d.target.target_number];

            if (x[employee_index][target_index]->solution_value() > 0.5) {
                std::cout << "employee " << employees[employee_index].name << " assigned to Target:" << targets[target_index].target_number << " - "
                          << targets[target_index].address << " (distance = " << d.distance << " km)" << std::endl;
            }
        }
        std::cout << "total average distance: " << average_distance << " km" << std::endl;
    } else {
        std::cout << "no balanced solution found..." << std::endl;
    }
}

// constraint: some employees are favored to be paired together (based on favor_coefficient)
// which reduces the total distance assigned (artificially just to favor certain pairings 
// to be assigned to the same location)
void addFriendConstraint(MPSolver& solver, MPObjective* objective, std::vector<std::pair<int, std::vector<int>>>& friend_groups,
    std::unordered_map<int, int>& id_to_index, std::vector<std::vector<const MPVariable*>>& x, int num_targets)
{
    for (const auto& group : friend_groups) {
        // the person who has the friend group
        const auto& main_character = group.first;
        // the people inside that person's friend group
        const auto& friends = group.second;

        if (!id_to_index.count(group.first)) continue;
        int main_character_id = id_to_index[main_character];
    
        for (const auto& frend : friends) {
            if (!id_to_index.count(frend)) continue;
            int friend_id = id_to_index[frend];

            for (int t = 0; t < num_targets; ++t) {
                const MPVariable* x1 = x[main_character_id][t]; 
                const MPVariable* x2 = x[friend_id][t];
                if (!x1 || !x2) continue;

                // y = 1 if both x1 and x2 are assigned to this target
                MPVariable* y = solver.MakeIntVar(0, 1, "y_" + std::to_string(main_character_id) + "_" + std::to_string(friend_id) + "_" + std::to_string(t));

                // y <= x1
                MPConstraint* c1 = solver.MakeRowConstraint(-MPSolver::infinity(), 0);
                c1->SetCoefficient(y, 1);
                c1->SetCoefficient(x1, -1);

                // y <= x2
                MPConstraint* c2 = solver.MakeRowConstraint(-MPSolver::infinity(), 0);
                c2->SetCoefficient(y, 1);
                c2->SetCoefficient(x2, -1);

                // y >= x1 + x2 - 1  -->  y - x1 - x2 >= -1
                MPConstraint* c3 = solver.MakeRowConstraint(-1, MPSolver::infinity());
                c3->SetCoefficient(y, 1);
                c3->SetCoefficient(x1, -1);
                c3->SetCoefficient(x2, -1);

                // reward in objective
                objective->SetCoefficient(y, -FAVOR_COEFFICIENT);                    
            }
        }
    }
}


void assignEmployeesEnemiesAndFriends(std::vector<Distance>& distances, std::vector<Employee>& employees, 
    std::vector<Target>& targets, std::vector<No_pair>& conflicts, std::vector<std::pair<int, std::vector<int>>> &friend_groups)
{
    int num_employees = employees.size();
    int num_targets = targets.size();

    MPSolver solver("EmployeeAssignment", MPSolver::SCIP_MIXED_INTEGER_PROGRAMMING);

    // x[i][j] = 1 if employee i is assigned to target j
    std::vector<std::vector<const MPVariable*>> x(num_employees, std::vector<const MPVariable*>(num_targets, nullptr));

    // map employee ID to its index
    std::unordered_map<int, int> id_to_index;
    for (int i = 0; i < num_employees; ++i) {
        id_to_index[employees[i].id] = i; 
    }

    // same for target (using .target_number instead of .id)
    std::unordered_map<int, int> tar_num_to_index;
    for (int i = 0; i < num_targets; ++i) {
        tar_num_to_index[targets[i].target_number] = i; 
    }

    // create the decision variables
    for (const auto& d : distances) {
        int employee_index = id_to_index[d.employee.id];
        int target_index = tar_num_to_index[d.target.target_number];

        x[employee_index][target_index] = solver.MakeIntVar(0, 1, "x_" + std::to_string(employee_index) + "_" + std::to_string(target_index));
    }

    // constraint: each employee is assigned at most once
    for (int i = 0; i < num_employees; ++i) {
        LinearExpr expr;
        for (int j = 0; j < num_targets; ++j) {
            expr += x[i][j];
        }
        solver.MakeRowConstraint(expr <= 1);
    }

    // constraint: each target has a required number of people that need to be on location
    for (int j = 0; j < num_targets; ++j) {
        LinearExpr expr;
        for (int i = 0; i < num_employees; ++i) {
            expr += x[i][j];
        }
        solver.MakeRowConstraint(expr == targets[j].req_employees);
    }

    // constraint: some employees hate one another, don't pair them
    for (const auto& conflict : conflicts) {
        if (id_to_index.count(conflict.first) && id_to_index.count(conflict.second)) {
            int first_employee = id_to_index[conflict.first];
            int second_employee = id_to_index[conflict.second];

            for (int k = 0; k < num_targets; ++k) {
                if (x[first_employee][k] && x[second_employee][k]) {
                    LinearExpr expr;
                    expr += x[first_employee][k]; 
                    expr += x[second_employee][k];
                    solver.MakeRowConstraint(expr <= 1);
                }
            }
        }
    }  

    // objective: minimize the total distance
    MPObjective* objective = solver.MutableObjective();
    for (const auto& d : distances) {
        int employee_index = id_to_index[d.employee.id];
        int target_index = tar_num_to_index[d.target.target_number];
        objective->SetCoefficient(x[employee_index][target_index], d.distance);
    }

    addFriendConstraint(solver, objective, friend_groups, id_to_index, x, num_targets);
    
    objective->SetMinimization();

    // since we decrease the objective->Value() by the FAVOR_COEFFICIENT every time we pair friends
    // keep track of the total distance of the assignment by summing up the distance traveled by every employee
    float km_sum = 0;

    // solve
    MPSolver::ResultStatus result_status = solver.Solve();

    if (result_status == MPSolver::OPTIMAL) {
        std::cout << "Optimal assignment found!" << std::endl;
        for (const auto& d : distances) {
            int employee_index = id_to_index[d.employee.id];
            int target_index = tar_num_to_index[d.target.target_number];

            if (x[employee_index][target_index]->solution_value() > 0.5) {
                std::cout << "employee " << employees[employee_index].name << " assigned to Target:" << targets[target_index].target_number << " - "
                          << targets[target_index].address << " (distance = " << d.distance << " km)" << std::endl;

                km_sum += d.distance;
            }
        }
        std::cout << "total cost: " << km_sum << " km" << std::endl;
    } else {
        std::cout << "no optimal solution found..." << std::endl;
    }
}
}
