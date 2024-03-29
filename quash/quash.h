/**
 * @file quash.h
 *
 * Quash essential functions and structures.
 */

#ifndef QUASH_H
#define QUASH_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <strings.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <inttypes.h>


/**
 * Specify the maximum number of characters accepted by the command string
 */
#define MAX_COMMAND_LENGTH (4096)

//Specify maximum number of background jobs allowed
#define MAX_BG_JOBS (64)

/**
 * Holds information about a command.
 */
typedef struct command_t {
  char cmdstr[MAX_COMMAND_LENGTH]; ///< character buffer to store the
                                   ///< command string. You may want
                                   ///< to modify this to accept
                                   ///< arbitrarily long strings for
                                   ///< robustness.
  size_t cmdlen;                   ///< length of the cmdstr character buffer

  // Extended with args to tokenize into
  char** args; //Memory will be realloc'ed as needed.
  int nArgs;
} command_t;

/**
 * Query if quash should accept more input or not.
 *
 * @return True if Quash should accept more input and false otherwise
 */
bool is_running();

/**
 * Causes the execution loop to end.
 */
void terminate();

/**
 *  Read in a command and setup the #command_t struct. Also perform some minor
 *  modifications to the string to remove trailing newline characters.
 *
 *  @param cmd - a command_t structure. The #command_t.cmdstr and
 *               #command_t.cmdlen fields will be modified
 *  @param in - an open file ready for reading
 *  @return True if able to fill #command_t.cmdstr and false otherwise
 */
bool get_command(command_t* cmd, FILE* in);

/**
 *  Runs the user input command.
 *
 *  @param cmd - a command_t structure that indicates the command and params
 */
void exec_cmd(command_t cmd);

/**
 *  Change location of working directory using cd
 *
 *  @param path - path of the new working directory
 */
void change_dir(char* path);

/**
 *  Break a string into its token parts
 *
 *  @param input - the string to tokenize
 *  @param nTkns - a integer that is modified when tokenizing
 *  @return - An array of tokens and number of tokens in nTkns
 */
char** tokenize(char* input, int* nTkns);

/**
 *  Set environment variables such as HOME or PATH.
 *
 *  @param var - envrionment variable to set, can be anything
 *  @param val - the value that should be set for var
 *
*/
void set_var(char* var, char* val);

/**
 *  Gets the environment variables such as HOME or PATH.
 *
 *  @param var - name of the variable to retrieve
 *
*/
void echo_var(char* var);

/**
 *  Run external commands/programs with or without parameters
 *
 * @ param cmd - The entire command variable
*/
void exec_extern(command_t cmd);

/**
 *  Runs an executable in background while adding it to the jobs list
 *  Keeps track of background process.
 *
 *  @param cmd - name of the variable to retrieve
 *
*/
void run_in_background(command_t cmd);

/**
 *  Prints the jobs in background
*/
void print_jobs();

/**
 *  Flushes zombie/finished processes from the jobs list.
*/
void flush_jobs();

/**
 *  Kills the jobs by PID in background
*/
void kill_ps(char* sig, char* pid);

/**
 *  Handles file redirection with > or <
 *
 *  @param cmd - The entire command variable
 *
*/
void file_redirection(command_t cmd, int redirLoc);

/**
 *  Handles execution piping with |
 *
 *  @param cmd - The entire command variable
 *
*/
void pipe_execution(command_t cmd, int redirLoc);

#endif // QUASH_H
