# CMPT_360_Assignment_3

Student Name: Jeffrey Moniz
Student ID: 3148591
Submission Date: February 20th 2026
File Name: README.md

# 1. Academic Integrity
“I certify that this submission represents entirely my own work.”

# 2. Overview
This program is designed to take an input from the user terminal. The program will check the mode depending on whether the user uses mode seg or mode bb. If the user inputs mode bb then the user will also have to input a base, limit and trace file, an example input would be: './vmsim --mode=bb --base=4096 --limit=64 --trace=tests/bb/t1.txt'. If the user inputs mode seg then the user will also have to input a config file and a trace file, an example input would be: './vmsim --mode=seg --config=tests/seg/bad-config.ini --trace=tests/seg/bad-trace.txt'. The program will then error check and the program will print the results of the mode bb or mode seg along with a stats overview of the accesses, ok's, fault bounds, seg hits, etc. 

# 3. Build
In order to run the program you need to type 'make' in the command line, then type ./vmsim --mode=bb --base=(decimal) --limit=(decimal) --trace=(trace file name) or type ./vmsim --mode=seg --config=(config file name) --trace=(trace file name) to get the program to run and print the desired results. typing 'make clean' will get rid of the output files.

# 4. Status
The program also runs perfectly fine in both Git and the student server