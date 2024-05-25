// Authors: Joshua Simpson
// This code implements a simple command line interpreter that processes and executes common Linux commands.

#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <sys/stat.h>
#include <fcntl.h> // Necessary for using the open() system call

// This function takes a command string and splits it into individual arguments, taking into account quoted strings
std::vector<char*> parseCommand(std::string& cmd)
{ 
    std::vector<char*> args;
    std::string arg;
    bool inQuotes = false;

    // Parse each character in the command string
    for (char c : cmd)
    {
        // Toggle state when encountering quotes
        if (c == '\"')
        {
            inQuotes = !inQuotes;
        }

        // Split the command at spaces unless within quotes
        if (c == ' ' && !inQuotes)
        {
            if (!arg.empty())
            {
                args.push_back(strdup(arg.c_str())); // Allocate and store the argument
                arg.clear();
            }
        }
        else
        {
            arg += c; // Append the character to the current argument
        }
    }
    
    // Add the last argument if there is one
    if (!arg.empty())
    {
        args.push_back(strdup(arg.c_str()));
    }
    args.push_back(nullptr); // Null-terminate the list of arguments
    return args;
}

// This function executes a command by splitting it into arguments and then handling each command type
void executeCommand(const std::string& cmd)
{
    auto args = parseCommand(const_cast<std::string&>(cmd)); // Convert command string into argument list

    // Match the first argument with supported commands and execute accordingly
    if (std::string(args[0]) == "mkdir")
    {
        // Create a new directory
        if (mkdir(args[1], 0777) != 0)
        {
            perror("mkdir error"); // Handle errors in directory creation
        }
    }
    else if (std::string(args[0]) == "cd")
    {
        // Change current directory
        if (chdir(args[1]) != 0)
        {
            perror("cd error"); // Handle errors in changing directory
        }
    }
    else if (std::string(args[0]) == "touch")
    {
        // Create a new file
        int fd = open(args[1], O_CREAT | O_WRONLY | O_TRUNC, 0666);
        if (fd == -1)
        {
            perror("open error"); // Handle file creation errors
        }
        else
        {
            close(fd);
        }
    }
    else if (std::string(args[0]) == "rm" && std::string(args[1]) == "-rf")
    {
        // Recursive removal of a directory
        if (args.size() < 3)
        {
            std::cerr << "Syntax error for rm -rf" << std::endl;
            return;
        }

        // Use fork and execlp to handle the removal process
        pid_t pid = fork();
        if (pid == 0) // Child process
        {
            if (execlp("/bin/rm", "/bin/rm", "-rf", args[2], nullptr) == -1)
            {
                perror("execlp error");
                exit(EXIT_FAILURE);
            }
        }
        else if (pid < 0)
        {
            perror("fork error"); // Handle errors in fork system call
        }
        else
        {
            int status;
            waitpid(pid, &status, 0); // Parent waits for child process to complete
        }
    }
    // Additional command handlers can be added here similarly

    // Free the allocated memory for arguments
    for (char* arg : args)
    {
        free(arg);
    }
}

int main()
{
    std::string input;

    while (true)
    {
        std::cout << "Enter command(s) (use ';' to separate multiple commands): ";
        std::getline(std::cin, input);

        if (input == "exit")
        {
            std::cout << "Exiting program... Thank you for using our command line interpreter!" << std::endl;
            break;
        }
        
        std::vector<std::string> commands;
        size_t pos = 0;
        std::string token;

        // Split input into individual commands based on ';' separator
        while ((pos = input.find(";")) != std::string::npos)
        {
            token = input.substr(0, pos);
            commands.push_back(token);
            input.erase(0, pos + 1);
        }
        commands.push_back(input); // Add the last (or only) command

        // Execute each command
        for (auto& cmd : commands)
        {
            executeCommand(cmd);
        }
    }

    return 0;
}
