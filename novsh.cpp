/* * * * * * * * * * * * * * * * * * * * * * * */
/* Project: CS270 Project 4 - novel shell      */
/* Author(s): Gabe Rivera and Yan Shi          */
/* Date: Nov. 20, 2019                         */
/* Version 1.0                                 */
/* * * * * * * * * * * * * * * * * * * * * * * */

#include <sstream>
#include <iostream>
#include <iomanip>
#include <map>
#include <unistd.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/wait.h>
using namespace std;

/*PROGRAM-CONTROL COMMAND PROTOTYPES*/
void run(vector<string> tokens);
void assignto(vector<string> tokens);
/*PROGRAM-CONTROL COMMAND PROTOTYPES*/

/*BUILT IN COMMAND PROTOTYPES*/
void commentCommand(vector<string> tokens);
void var(vector<string> tokens);
void newprompt(vector<string> tokens);
int dir(vector<string> tokens);
void listprocs(vector<string> tokens);
void bye(vector<string> tokens);
/*BUILT IN COMMAND PROTOTYPES*/

/*OTHER FUCNTION PROTOTYPES*/
void commandList(vector<string> tokens);
bool argumentChecker(vector<string> tokens, int args);
/*OTHER FUCNTION PROTOTYPES*/

bool stillRunning = true;   // boolean var to run the program
map<string, string> variables; // changed from struct to map for efficiency reasons
vector<string> processes; // processes is a vector populated with proccesses
vector<string> writeTokens(string str); // writeTokens is a vector populated with tokens
string prompt = "novsh > "; //prompt initally equals "novsh"
string showtokens = "0";

vector<string> writeTokens(string str) {
    istringstream input(str);
    vector<string> tokens;
    vector<string> tokensEdit;
    string s;
    
    while (input >> s){
        // check for $ in variable name
        if (s.substr(0, 1) == "$") {
            //cout << s.substr(1) << " is being changed to " << variables.find(s.substr(1))->second << endl;
            s = variables.find(s.substr(1))->second;
        }
        //push into tokens array
        tokens.push_back(s);
    }
    // checking for "..." in a variable name
    for(int i = 0; i < tokens.size(); i++){
        if(tokens[i].substr(0, 1) == "\""){
            tokens[i] = tokens[i].substr(1, tokens[i].size()-2);
            //cout << "new token: " <<  tokens[i] << endl;
        }
    }
    
    return tokens;
}
// fucntion containing list of commands. Looks for string that matches any of the built in or p/c commands.
void commandList(vector<string> tokens) {
    // check for no tokens
    if (tokens.size() == 0) {
        cout << endl;
        return;
    }
    
    string built_in_command1 = tokens[0];
    
    // *note* switch statements do NOT work for strings, only for ints, chars, and char arrays
    // result in using if statements
    if (built_in_command1 == "!")   // comment command
        commentCommand(tokens);
    else if (tokens.size() > 1 && tokens[1] == "=") // var "=" command
        var(tokens);
    else if (built_in_command1 == "newprompt")  // newprompt command
        newprompt(tokens);
    else if (built_in_command1 == "dir")    // dir command
        dir(tokens);
    else if (built_in_command1 == "listprocs")  // listprocs command
        listprocs(tokens);
    else if (built_in_command1 == "bye")    // bye or terminating command
        bye(tokens);
    else if (built_in_command1 == "run")    // run command
        run(tokens);
    else if (built_in_command1 == "assignto")   // assignto command
        assignto(tokens);
    else{
        fprintf(stderr, "Command not found... no match for '%s'\n", built_in_command1.c_str()); // error if no command matched by paramter
    }
}

// boolean fucntion to confirm the correct number of args for respective command function
bool argumentChecker(vector<string> tokens, int args) {
    if ((tokens.size() > args || tokens.size() < args) && tokens[args] != "!") {
        fprintf(stderr,"Error: invalid number of arguments for %s...\n", tokens[0].c_str());
        return true;
    }
    return false;
}

/* START OF BUILT IN COMMANDS */

void commentCommand(vector<string> tokens){
    // do nothing, no print statements needed
}

void var(vector<string> tokens) {
    if (argumentChecker(tokens, 3)) {
        return;
    }
    string tokenVar = tokens[1];        // tokenVar = "="
    string callNameVar = tokens[0];     // callName var = a in (a = b)
    string assignmentVar = tokens[2];   // assigmentVar = b in (a = b)
    //cout << "tok[0]: " << callNameVar << ", tok[1]: " << tokenVar << " & tok[2]: " << assignmentVar << endl;
    
    if (isalpha(callNameVar[0])) {  // determine if the first letter of the variable name is valid
        //cout << "Variable name is correct (:";
    }else{
        fprintf(stderr,"Variable names need to start with a character...\n");
        return;
    }
    
    for (int i = 1; i < callNameVar.length(); i++) {    // parse through variable name to confirm it consists of nums and digits
        if (isalpha(callNameVar[i]) || isdigit(callNameVar[i])){
            //cout << "Variable name is correct (:";
        }else{
            fprintf(stderr,"Variable names must only consist of letters and numbers...\n");
            return;
        }
    }
    
    if (callNameVar == "ShowTokens")
        showtokens = assignmentVar;
    else
        variables[callNameVar] = assignmentVar;
    
}

int dir(vector<string> tokens) {
    int directoryChange = chdir(tokens[1].c_str());
    
    if (argumentChecker(tokens, 2)) {
        return -1;
    }
    
    if (directoryChange != 0) {
        fprintf(stderr, "No directory matching \"%s\" was found...\n", tokens[1].c_str());
        return -1;
    }
    return 0;
}

void newprompt(vector<string> tokens) {
    if (argumentChecker(tokens, 2)) {
        return;
    }
    prompt = tokens[1];
}

void listprocs(vector<string> tokens) {
    if(argumentChecker(tokens, 1))
        return;
    for (auto processList: processes) {
        printf("Listing processes: %s", processList.c_str());
    }
}

void bye(vector<string> tokens) {
    if(argumentChecker(tokens, 1)){
        stillRunning = true;
    }else
        stillRunning = false;
    
}
/* END OF BUILT IN COMMANDS */

void assignto(vector<string> tokens) {
    //could not get assignto to work
}

void run(vector<string> tokens) {
    if (tokens.size() < 2) {
        argumentChecker(tokens, 2);
        return;
    }
    
    char* args[tokens.size()];
    int newSize = 1; // new size not counter "run"
    
    /*conversion from vector of cpp strings to an array of c strings from 1 to tokens.size(), args is an array of char*'s. Populate array, tokens[i] grabs c ==
     string and .c_str returns a c string version of cpp string. const_cast converts char* to const char* so it becomes immuatable*/
    for (int i=newSize; i<tokens.size(); i++) {
        args[i-newSize] = const_cast<char*>(tokens[i].c_str());
    }
    args[tokens.size() - newSize] = NULL;   // for array parsing reasons, last element us null
    
    /*Negative Value: creation of a child process was unsuccessful.
     Zero: Returned to the newly created child process.
     Positive value: Returned to parent or caller. The value contains process ID of newly created child process.
     */
    
    int pid = fork();
    if (pid == 0) {
        pid = fork();
    }
    if (pid == 0) {
        execvp(args[0], args);
        fprintf(stderr, "Error calling execvp...\n");
        exit(1);
    }
    waitpid(pid, NULL, 0);
}

int main() {
    variables["PATH"] = "/bin:/usr/bin";
    string command;
    vector<string> parameters;
    while(stillRunning && std::cin) {
        cout << prompt;
        getline(cin, command);
        vector<string> tokens = writeTokens(command);
        commandList(tokens);
        if(showtokens=="1")
            for(auto& str : tokens)
                cout << str << endl;
    }
    return 0;
}
