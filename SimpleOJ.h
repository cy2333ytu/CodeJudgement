#pragma once

#include <string>

class SimpleOJ {
public:
    SimpleOJ();
    void run();
    
private:
    static void static_timer_handler(int signum);
    void timer_handler(int signum);
    void copy_tail_to_user_program(const std::string& tail_path, const std::string& user_program_path);
    bool compare_files(const std::string& file1, const std::string& file2);
    void display_problem_description(const std::string& problem_path);
    void perform_evaluation(const std::string& problem_path);
    void display_problems(const std::string& config_path);
    static SimpleOJ* instance;

};
