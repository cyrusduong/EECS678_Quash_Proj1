/**
 * @file quash.c
 *
 * Quash's main file
 */

/**************************************************************************
 * Included Files
 **************************************************************************/
#include "quash.h" // Putting this above the other includes allows us to ensure
                   // this file's headder's #include statements are self
                   // contained.

#include <string.h>
#include <unistd.h>
#include <sys/types.h>

/**************************************************************************
 * Private Variables
 **************************************************************************/
/**
 * Keep track of whether Quash should request another command or not.
 */
// NOTE: "static" causes the "running" variable to only be declared in this
// compilation unit (this file and all files that include it). This is similar
// to private in other languages.
static bool running;

//Define buffer for current working directory
char myCwd[1024];

//Define buffer for environment variables
char *envHome;
char *envUser;
char *envPath;

//Define structure of a job
struct job {
    int jid;
    int pid;
    char *com;
};
//Define jobList
struct job jobs[MAX_BG_JOBS];
int *nJobs;

/**************************************************************************
 * Private Functions
 **************************************************************************/
/**
 * Start the main loop by setting the running flag to true
 */
static void start() {
  running = true;
}

/**************************************************************************
 * Public Functions
 **************************************************************************/
bool is_running() {
  return running;
}

void terminate() {
  running = false;
}

bool get_command(command_t* cmd, FILE* in) {
  //Grab current working directory
  getcwd(myCwd, 1024); //Find path upto 1024.
  //Command Indicator
  printf("%s > ", myCwd);

  if (fgets(cmd->cmdstr, MAX_COMMAND_LENGTH, in) != NULL) {
    size_t len = strlen(cmd->cmdstr);
    char last_char = cmd->cmdstr[len - 1];

    if (last_char == '\n' || last_char == '\r') {
      // Remove trailing new line character.
      cmd->cmdstr[len - 1] = '\0';
      cmd->cmdlen = len - 1;
    } else {
      cmd->cmdlen = len;
    }

    //Tokenize and place into args of the command_t.
    //    - Pass &nArgs to recieve correct number of tokens
    cmd->args = tokenize(cmd->cmdstr, &cmd->nArgs);

    return true;
  }
  else
    return false;
}

void change_dir(char* path) {
  //Nice one liner that sets appropriate directory
  if ( ((!path) ? chdir(envHome) : chdir(path)) == -1 )
    printf("'%s'\n", strerror(errno));
}

void exec_cmd(command_t* cmd) {
  if (cmd->args[cmd->nArgs] == '&') {
    //'Parse' out '&'
    cmd->nArgs = cmd->nArgs - 1;
    run_in_background(cmd);
  } else if (!strcmp(cmd->cmdstr, "exit")) {
    terminate(); // Exit Quash
  } else if (!strcmp(cmd->cmdstr, "quit")) {
    terminate(); // Quit Quash
  } else if (!strcmp(cmd->cmdstr, "pwd")) {
    printf("%s\n", myCwd);
  } else if (!strcmp(cmd->cmdstr, "cd")) {
    change_dir(cmd->args[1]);
  } else if (!strcmp(cmd->cmdstr, "set")) {
    set_var(cmd->args[1], cmd->args[2]);
  } else if (!strcmp(cmd->cmdstr, "echo")) {
    echo_var(cmd->args[1]);
  } else if (!strcmp(cmd->cmdstr, "jobs")) {
    printJobs();
  } else {
    exec_extern(&cmd);
  }
}

char** tokenize(char *input, int* nTkns) {
  size_t nSpaces = 0;
  char **retTokens = NULL;
  char* tok = strtok(input, " ");
  while (tok) {
    //Reallocate memory based on # of tokens
    retTokens = realloc( retTokens, sizeof(char*) * ++nSpaces);
    //Check to see if allocation failed
    if (retTokens == NULL) {
      printf("\nMALLOC FAIL FOR COMMAND '%s'\n", input);
      return retTokens;
    }

    //If token is a string put string into retTokens and remove "quotes".
  	if (tok[0] == '"') {
  		++tok;
  		sprintf(tok, "%s%s%s", tok, " ", strtok(NULL, "\""));
  	}

    //Place token into array of tokens to return
    retTokens[nSpaces-1] = tok;
    // No new input just parse rest of input from earlier,
    // use = as delimiter for set function.
    // use $ as delimiter for echo variables
    tok = strtok(NULL, " =$");
  }

  //Set nTkns to correct # of tokens
  *nTkns = nSpaces;

  // Cyrus - Took out because seemed unnecessary
  // retTokens = realloc( retTokens, sizeof(char*) * ++nSpaces);
  // retTokens[nSpaces-1] = 0;

  return retTokens;
}

void set_var(char* var, char* val) {
  if (var != NULL && val != NULL) {
    if ( setenv(var, val, 0) == -1 ) {
      printf("'%s'\n", strerror(errno));
    }

    char* envVal = getenv(var);
    //Check if set correctly, notify user if so
    if (strcmp(val, envVal) == 0) {
      printf("%s successfully set to %s\n", var, envVal);
    } else {
      printf("%s is set to %s\n", var, envVal);
      printf("Would you like to overwrite this value? (y/n): ");

      char input = 0;
      input = getchar();
      while( !( input == 'y' || input == 'n' ) ) {
        printf("Please input either y or n: ");
        input = getchar();
      }

      if ( input == 'y' ) {
        if ( setenv(var, val, 1) == -1 ) {
          printf("'%s'\n", strerror(errno));
        }
        envVal = getenv(var);
        if (strcmp(val, envVal) == 0) {
          printf("%s successfully set to %s\n", var, envVal);
        }
      } else {
        printf("%s left set to %s\n", var, envVal);
      }
      //Eat \n from buffer to not be read by parser
      input = getchar();
    }
  }
}

void echo_var(char* var) {
  char* envVal = getenv(var);
  if (envVal == NULL) {
    printf("Variable has not been set or does not exist\n");
  } else {
    printf("%s\n", envVal);
  }
}

void exec_extern(command_t cmd) {
  pid_t pid = fork();
  int status;

  if (pid == 0) {
    // Any number of arguments can be taken
    if (execvp(cmd.args[0], cmd.args) < 0) {
      printf("%s: No such file, directory, or command\n", cmd.args[0]);
      exit(-1);
    }
  } else {
    waitpid(pid, &status, 0);

    if (status == 1) {
      fprintf(stderr, "%s", "Error");
void run_in_background(command_t* cmd) {
  pid_t pid, sid;
  pid = fork();

  if (pid == 0) {
    sid = setsid();

    if (sid < 0) {
      printf("Failed to create child process\n");
      exit(EXIT_FAILURE);
    }

    printf("[%d] %d is running in the background\n", getpid(), *nJobs);
    exec_cmd(cmd);
    printf("\n[%d] finished\n", getpid());

    kill(getpid(), -9); //KILL SIG 9
    exit(EXIT_SUCCESS);
  } else {
    struct job currentJob = {
  		.jid = pid,
  		.pid = *nJobs,
  		.com = cmd->cmdstr
  	};

    jobs[*nJobs] = currentJob;
    *nJobs = *nJobs + 1;

    while(waitid(pid, NULL, WEXITED | WNOHANG) > 0) {}
  }
}

void printJobs() {
  for (size_t i = 0; i < *nJobs; ++i) {
    if (kill(jobs[i].pid, 0) == 0) {
      printf("[%d]\t%d\t%s\n", jobs[i].jid, jobs[i].pid, jobs[i].com);
    }
  }
}

/**
 * Quash entry point
 *
 * @param argc argument count from the command line
 * @param argv argument vector from the command line
 * @return program exit status
 */
int main(int argc, char** argv) {
  command_t cmd; //< Command holder argument

  start();

  puts("Welcome to Quash!");
  puts("Type \"exit\" or \"quit\" to quit");

  // Main execution loop
  while (is_running() && get_command(&cmd, stdin)) {
    // Get environment variables
    envHome = getenv("HOME");
    envUser = getenv("USER");
    envPath = getenv("PATH");

    // The commands should be parsed, then executed.
    if (!strcmp(cmd.cmdstr, "exit")) {
      terminate(); // Exit Quash
    } else if (!strcmp(cmd.cmdstr, "quit")) {
      terminate(); // Quit Quash
    } else if (!strcmp(cmd.cmdstr, "pwd")) {
      printf("%s\n", myCwd);
    } else if (!strcmp(cmd.cmdstr, "cd")) {
      change_dir(cmd.args[1]);
    } else if (!strcmp(cmd.cmdstr, "set")) {
      set_var(cmd.args[1], cmd.args[2]);
    } else if (!strcmp(cmd.cmdstr, "echo")) {
      echo_var(cmd.args[1]);
    } else {
      exec_extern(cmd);
    }
  }

  return EXIT_SUCCESS;
}
