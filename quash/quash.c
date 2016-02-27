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
    // also use = as delimiter for set function.
    tok = strtok(NULL, " =");
  }

  //Set nTkns to correct # of tokens
  *nTkns = nSpaces;

  retTokens = realloc( retTokens, sizeof(char*) * ++nSpaces);
  retTokens[nSpaces-1] = 0;

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

  // Get environment variables
  envHome = getenv("HOME");
  envUser = getenv("USER");
  envPath = getenv("PATH");

  // Main execution loop
  while (is_running() && get_command(&cmd, stdin)) {
    // NOTE: I would not recommend keeping anything inside the body of
    // this while loop. It is just an example.

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
    } else {
      puts(cmd.cmdstr); // Echo the input string
    }
  }

  return EXIT_SUCCESS;
}
