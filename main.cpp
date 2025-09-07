#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <iostream>
#include <bitset>
#include <fstream>
#include <unordered_map>
#include <sstream>

using namespace std;

class Instruction
{
public:
    // Constructor
    Instruction()
    {
        cout << "Instruction Class Constructor Called" << endl;
    }   

    ~Instruction()
    {
        // destructor: work not started yet on this
        // cout << "Instruction Class Destructor Called" << endl;
    }
    string convertRTypeToBinary(string MIPS_Line)
    {
        // funct Map for R-Type Instructions
        unordered_map<string, int> functMap =
        {
            {"add", 32}, {"sub", 34}, {"and", 36}, {"or", 37}, {"slt", 42}, {"sll", 0}, {"srl", 2}
        };

        string result = "";
        fields F;
        F.opcode = 0; // opcode for R-Type is always 0

        // Tokeninzing the String:

        for (char& c : MIPS_Line) 
            if (c == ',' || c == '(' || c == ')') 
                c = ' ';
        char delimiter = ' ';
        cout << MIPS_Line << endl;

        stringstream ss(MIPS_Line);
        string token;
        vector<string> tokens;

        while (getline(ss, token, delimiter)) {
            if (!token.empty())
                tokens.push_back(token);
        }

        if(tokens[0] == "sll" || tokens[0] == "srl") 
        {
            // operation $rd, $rt, shamt
            F.rd = registerMap.find(tokens[1])->second;
            F.rt = registerMap.find(tokens[2])->second;
            F.shamt = stoi(tokens[3]);
            F.rs = 0; // default value
            F.funct = functMap.find(tokens[0])->second;
        }
        else
        {
            // for add, sub, etc.
            // operation $rd, $rs, $rt
            F.rd = registerMap.find(tokens[1])->second;
            F.rs = registerMap.find(tokens[2])->second;
            F.rt = registerMap.find(tokens[3])->second;
            F.shamt = 0; // default value
            F.funct = functMap.find(tokens[0])->second;
        }

        result += F.opcode.to_string();
        result += F.rs.to_string();
        result += F.rt.to_string();
        result += F.rd.to_string();
        result += F.shamt.to_string();
        result += F.funct.to_string();

        return result;
    }
    string convertITypeToBinary(string MIPS_Line)
    {
        return "I-Type Binary Not Implemented Yet";
    }

private:
    // MIPS Instruction Fields:
    struct fields
    {
        bitset<6> opcode;
        bitset<5> rs;
        bitset<5> rt;
        bitset<5> rd;
        bitset<5> shamt;
        bitset<6> funct;
        bitset<16> immediate;
        bitset<26> address;
    };

    // MIPS Register Mapping:
    unordered_map<string, int> registerMap = 
    {
        {"$zero", 0}, {"$at", 1}, {"$v0", 2}, {"$v1", 3}, {"$a0", 4}, {"$a1", 5}, {"$a2", 6}, {"$a3", 7}, {"$t0", 8}, {"$t1", 9}, {"$t2", 10}, {"$t3", 11}, {"$t4", 12}, {"$t5", 13}, {"$t6", 14}, {"$t7", 15}, {"$s0", 16}, {"$s1", 17}, {"$s2", 18}, {"$s3", 19}, {"$s4", 20}, {"$s5", 21}, {"$s6", 22}, {"$s7", 23}, {"$t8", 24}, {"$t9", 25}, {"$k0", 26}, {"$k1", 27}, {"$gp", 28}, {"$sp", 29}, {"$fp", 30}, {"$ra", 31}
    };

    // Struct for Opcode Info
    struct OpcodeInfo 
    {
        int code;
        char type;
    };

    // MIPS Opcode Mapping:
    unordered_map<string, OpcodeInfo> opcodeMap = 
    {
        {"add",  {0, 'R'}}, {"sub",  {0, 'R'}}, {"and",  {0, 'R'}}, {"or",   {0, 'R'}}, {"slt",  {0, 'R'}}, {"sll",  {0, 'R'}}, {"srl",  {0, 'R'}},      // R-Type Instructions
        {"addi", {8, 'I'}}, {"andi", {12, 'I'}}, {"ori",  {13, 'I'}}, {"lw",  {35, 'I'}}, {"sw",   {43, 'I'}}, {"beq",  {4, 'I'}}, {"bne",  {5, 'I'}},   // I-Type Instructions
        {"j",    {2, 'J'}}, {"jal",  {3, 'J'}}                                                                                                           // J-Type Instructions
    };
    
};  

class Assembler
{
public:

    // Constructor
    Assembler(string filename)
    {
        // Opening the File & Assigning it to the "inputfile" Object
        inputfile.open(filename);

        if (!inputfile.is_open())
        {
            cerr << "Error Opening File: " << filename << endl;
            exit(EXIT_FAILURE);
        }
        cout << "File Opened Successfully: " << filename << endl;
    }

    ~Assembler()
    {
        // destructor: work not started yet on this
        cout << "Destructor not Created yet" << endl;
    }

    // Main method to drive the assembly process
    void assemble()
    {
        firstPass();
        // secondPass(); // This would be called next
    }

    // First Pass: State Memory Allocation & Instruction Label Address Mapping
    void firstPass()
    {
        // Defining Variable for Current Segment Type Storage
        enum Segment
        {
            UNKNOWN,
            TEXT,
            DATA
        };
        Segment currentSegment = UNKNOWN;

        string line;

        // Going to beginning of file for first pass
        inputfile.clear();
        inputfile.seekg(0, ios::beg);

        cout << "\nStarting First Pass..." << endl;

        while (getline(inputfile, line))
        {
            // Removing trailing and leading whitespaces
            line.erase(0, line.find_first_not_of(" \t\n\r"));
            line.erase(line.find_last_not_of(" \t\n\r") + 1);

            // Skipping empty lines and comments
            if (line.empty() || line[0] == '#' || line.find(".globl") != string::npos)
                continue; 

            // Checking for Segment Type
            if (line.find(".data") != string::npos)
            {
                currentSegment = DATA;
                continue;
            }
            if (line.find(".text") != string::npos)
            {
                currentSegment = TEXT;
                continue;
            }
            if (currentSegment == UNKNOWN)
                continue;

            // Checking for an Instruction Label
            size_t colonPos = line.find(":");
            if (colonPos != string::npos)
            {
                string label = line.substr(0, colonPos);
                // Assigning Label the Address from Current Segment
                symbolTable[label] = (currentSegment == TEXT) ? textAddress : dataAddress;

                // Removing label to check for any instruction on the same line:

                line = line.substr(colonPos + 1);
                line.erase(0, line.find_first_not_of(" \t"));
                
                if (line.empty())
                    continue; // If line now empty, it was only a label
            }

            if (currentSegment == TEXT)
            {
                textAddress += 4; // All MIPS instructions are 4 bytes
            }

            else if (currentSegment == DATA)
            {
                stringstream ss(line);
                string firstToken;
                ss >> firstToken;

                // Handling .data directives for static memory allocation planning
                if (firstToken == ".word")
                {
                    // Planning for a 4-byte word
                    dataAddress += 4;
                }
                else if (firstToken == ".asciiz")
                {
                    // Planning space for a string
                    size_t start = line.find("\"") + 1;
                    size_t end = line.rfind("\"");
                    if (start != string::npos && end != string::npos)
                    {
                        // Allocating space for each character plus the null terminator
                        dataAddress += (end - start + 1);
                    }
                }
                else if (firstToken == ".space")
                {
                    // Planning for a block of empty space
                    string spaceVal;
                    ss >> spaceVal;
                    dataAddress += stoi(spaceVal);
                }
            }
        }
        cout << "First Pass Complete." << endl;
    }

    // Utility to print the generated symbol table for verification
    void printSymbolTable()
    {
        cout << "\n--- Symbol Table ---" << endl;
        cout << "Label -> Address" << endl;
        cout << "--------------------" << endl;
        for (const auto& pair : symbolTable)
        {
            cout << pair.first << " -> 0x" << hex << pair.second << endl;
        }
        cout << "--------------------" << endl;
    }


private:
    ifstream inputfile;

    // Symbol Table: maps a label string to a memory address
    unordered_map<string, unsigned int> symbolTable;

    // Address counters for the text and data segments
    unsigned int textAddress = 0x00400000; // MIPS text segment starts here
    unsigned int dataAddress = 0x10010000; // MIPS data segment starts here
};

int main(int argc, char *argv[])
{
    cout << "MIPS to Binary Code Convertor" << endl;

    // Assembler compiler("mips1_add.asm");
    // cout << compiler.foundMainSegment() << endl;
    Instruction instruction;
    string binary = instruction.convertRTypeToBinary("sll $t0, $t1, 3");
    cout << "Binary: " << binary << endl;

    // delete compiler;

    return 0;
}
