#include "SimpleOJ.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <sys/resource.h>
#include <signal.h>

namespace fs = std::filesystem;
SimpleOJ* SimpleOJ::instance = nullptr;

SimpleOJ::SimpleOJ() {}

void SimpleOJ::timer_handler(int /*signum*/) {
    std::cerr << "Program terminated due to time limit exceeded.\n";
    exit(EXIT_FAILURE);
}

void SimpleOJ::static_timer_handler(int signum) {
    instance->timer_handler(signum);
}
void SimpleOJ::copy_tail_to_user_program(const std::string& tail_path, const std::string& user_program_path) {
        std::cout<< "tail.cpp path is: "<<tail_path<<std::endl;

    std::ifstream tail_file(tail_path);
    
    if (!tail_file.is_open()) {
        std::cerr << "Error: Unable to open tail file\n";
        return;
    }

    std::ofstream user_program_file(user_program_path, std::ios_base::app); // Open in append mode
    if (!user_program_file.is_open()) {
        std::cerr << "Error: Unable to open user program file\n";
        return;
    }

    std::string line;
    while (std::getline(tail_file, line)) {
        user_program_file << line << "\n";
    }

    tail_file.close();
    user_program_file.close();
}

bool SimpleOJ::compare_files(const std::string& file1, const std::string& file2) {
        std::ifstream stream1(file1);
    std::ifstream stream2(file2);

    if (!stream1.is_open() || !stream2.is_open()) {
        return false;
    }

    std::string line1, line2;
    while (std::getline(stream1, line1) && std::getline(stream2, line2)) {
        if (line1 != line2) {
            return false;
        }
    }

    return true;
}

void SimpleOJ::display_problem_description(const std::string& problem_path) {
        std::ifstream desc_file(problem_path + "/desc.txt");
    if (!desc_file.is_open()) {
        std::cerr << "Error: Unable to open description file\n";
        return;
    }

    std::string line;
    while (std::getline(desc_file, line)) {
        std::cout << line << "\n";
    }
}

void SimpleOJ::perform_evaluation(const std::string& problem_path) {
    std::ifstream desc_file(problem_path + "/desc.txt");
    if (!desc_file.is_open()) {
        std::cerr << "Error: Unable to open description file\n";
        return;
    }

    std::ifstream header_file(problem_path + "/header.cpp");
    if (!header_file.is_open()) {
        std::cerr << "Error: Unable to open header file\n";
        return;
    }

    // 将头文件内容复制到用户程序文件中
    std::ofstream user_program_file("user_code/user_program.cpp");
    if (!user_program_file.is_open()) {
        std::cerr << "Error: Unable to open user program file\n";
        return;
    }

    std::string line;
    while (std::getline(header_file, line)) {
        user_program_file << line << "\n";
    }
    user_program_file.close();

    std::cout << "You can start writing your code in user_code/user_program.cpp\n";
    std::cout << "When you are done, enter 'over' to perform evaluation: ";
    std::string user_input;
    std::cin >> user_input;

    if (user_input == "over") {
        std::string user_program_path = "user_code/user_program.cpp";
        std::string tail_path = problem_path + "/tail.cpp";
        copy_tail_to_user_program(tail_path, user_program_path);

        struct rlimit mem_limit;
        mem_limit.rlim_cur = 100 * 1024 * 1024; // 100 MB in bytes
        mem_limit.rlim_max = 100 * 1024 * 1024; // 100 MB in bytes
        if (setrlimit(RLIMIT_AS, &mem_limit) != 0) {
            std::cerr << "Error setting memory limit\n";
            return;
        }

        // Set time limit to 7 seconds
        struct sigaction sa;
        sa.sa_flags = SA_RESTART;
        sa.sa_handler = static_timer_handler; // Use the static handler
        sigaction(SIGALRM, &sa, NULL);

        timer_t timerid;
        struct itimerspec its;
        its.it_value.tv_sec = 7;  // 7 seconds
        its.it_value.tv_nsec = 0;
        its.it_interval.tv_sec = 0;
        its.it_interval.tv_nsec = 0;

        if (timer_create(CLOCK_REALTIME, NULL, &timerid) == -1) {
            std::cerr << "Error creating timer\n";
            return;
        }

        if (timer_settime(timerid, 0, &its, NULL) == -1) {
            std::cerr << "Error setting timer\n";
            return;
        }
        
        // Compile the combined code
        std::ifstream user_code_stream("user_code/user_program.cpp");
        std::string combined_code = "";
        while (std::getline(user_code_stream, line)) {
            combined_code += line + "\n";
        }

        // Write the combined code to a temporary file
        std::string combined_program_path = "user_code/combined_program.cpp";
        std::ofstream combined_program_file(combined_program_path);
        combined_program_file << combined_code;
        combined_program_file.close();

        // Compile the combined code
        int compile_status = system(("g++ -std=c++17 -Wall -Wextra -o user_program_combined " + combined_program_path).c_str());
        if (compile_status == 0) {
            std::cout << "Compilation successful. Running test cases:\n";
            int exit_status;
            if ((exit_status = system("./user_program_combined")) == -1) {
                std::cerr << "Error running the program\n";
            } else if (WIFSIGNALED(exit_status) && WTERMSIG(exit_status) == SIGALRM) {
                std::cout << "Program terminated due to time limit exceeded.\n";
            } else if (exit_status == 0) {
                std::cout << "All test cases tested.\n";
            } else {
                std::cout << "Some test cases failed.\n";
            }
            
        } else {
            std::cerr << "Compilation failed.\n";
        }

        // Clean up the timer
        timer_delete(timerid);

    } else {
        std::cout << "Evaluation aborted\n";
    }
    
    std::cout << "Do you want to continue testing other problems? (yes/no): ";
    std::string continue_testing;
    std::cin >> continue_testing;

    if (continue_testing == "yes") {
        run(); // 重新运行程序以继续选择问题和测试
    } else {
        std::cout << "Exiting SimpleOJ. Goodbye!\n";
    }
}


void SimpleOJ::display_problems(const std::string& config_path) {
        std::ifstream config_file(config_path);
    if (!config_file.is_open()) {
        std::cerr << "Error: Unable to open oj_config.cfg\n";
        return;
    }

    std::string line;
    while (std::getline(config_file, line)) {
        std::istringstream iss(line);
        std::string problem_num, problem_name, problem_difficulty, problem_path;
        iss >> problem_num >> problem_name >> problem_difficulty >> problem_path;
        std::cout << problem_num << "\t" << problem_name << "\t" << problem_difficulty << "\n";
    }
}

void SimpleOJ::run() {
    std::cout << "Available problems:\n";
    std::cout << "Number\tName\tDifficulty\n";
    std::cout << "------------------------\n";

    display_problems("oj_data/oj_config.cfg");

    std::cout << "\nEnter the number of the problem you want to solve: ";
    int chosen_problem;
    std::cin >> chosen_problem;

    std::ifstream config_file("oj_data/oj_config.cfg");
    if (!config_file.is_open()) {
        std::cerr << "Error: Unable to open oj_config.cfg\n";
        return;
    }

    std::string line;
    std::string problem_path;
    while (std::getline(config_file, line)) {
        std::istringstream iss(line);
        std::string problem_num, problem_name, problem_difficulty;
        iss >> problem_num >> problem_name >> problem_difficulty >> problem_path;

        if (std::stoi(problem_num) == chosen_problem) {
            display_problem_description(problem_path);
            perform_evaluation(problem_path);  // Pass the problem path to the function
            break;
        }
    }
}
