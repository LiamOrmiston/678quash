/**
 * @file execute.c
 *
 * @brief Implements interface functions between Quash and the environment and
 * functions that interpret an execute commands.
 *
 * @note As you add things to this file you may want to change the method signature
 */

#include "execute.h"

#include <stdio.h>
#include<stdlib.h>
#include "quash.h"
#include "deque.h"

// Remove this and all expansion calls to it
/**
 * @brief Note calls to any function that requires implementation
 */
#define IMPLEMENT_ME()                                                  \
  fprintf(stderr, "IMPLEMENT ME: %s(line %d): %s()\n", __FILE__, __LINE__, __FUNCTION__)
bool initialized = false;
static int pipes[2][2];
IMPLEMENT_DEQUE_STRUCT(pid_queue, pid_t);
IMPLEMENT_DEQUE(pid_queue, pid_t);
pid_queue pid_q;

typedef struct Job{
  int job_id;
  char* cmd_job;
  pid_queue pid_q;
  pid_t pid;
} Job;

IMPLEMENT_DEQUE_STRUCT(job_queue, struct Job);
IMPLEMENT_DEQUE(job_queue, struct Job);
job_queue job_q;
int job_number = 1;
/***************************************************************************
 * Interface Functions
 ***************************************************************************/

// Return a string containing the current working directory.
char* get_current_directory(bool* should_free) {
  // TODO: Get the current working directory. This will fix the prompt path.
  // HINT: This should be pretty simple

  // Change this to true if necessary
  char* current = malloc(1024);

  getcwd(current,1024);
  *should_free = true;

  return current;
}

// Returns the value of an environment variable env_var
const char* lookup_env(const char* env_var) {
  // TODO: Lookup environment variables. This is required for parser to be able
  // to interpret variables from the command line and display the prompt
  // correctly
  // HINT: This should be pretty simple
  // TODO: Remove warning silencers
  const char* str = getenv(env_var);
  return str;
}

// Check the status of background jobs
void check_jobs_bg_status() {
  // TODO: Check on the statuses of all processes belonging to all background
  // jobs. This function should remove jobs from the jobs queue once all
  // processes belonging to a job have completed.
  struct Job current_job;
  pid_t m_front;
  int num_of_jobs = length_job_queue(&job_q);
  for(int i = 0; i < num_of_jobs;i++){
    current_job = pop_front_job_queue(&job_q);

    int num_of_pids = length_pid_queue(&current_job.pid_q);
    m_front = peek_front_pid_queue(&current_job.pid_q);
    for(int num = 0; num < num_of_pids; num++){
      pid_t current_pid = pop_front_pid_queue(&current_job.pid_q);
      int status;
      if(waitpid(current_pid,&status,WNOHANG)==0){
        push_back_pid_queue(&current_job.pid_q,current_pid);
      }
    }
  // TODO: Once jobs are implemented, uncomment and fill the following line
  if(is_empty_pid_queue(&current_job.pid_q)){
    print_job_bg_complete(current_job.job_id, m_front, current_job.cmd_job);
  }else{
    push_back_job_queue(&job_q,current_job);
  }
}
}

// Prints the job id number, the process id of the first process belonging to
// the Job, and the command string associated with this job
void print_job(int job_id, pid_t pid, const char* cmd) {
  printf("[%d]\t%8d\t%s\n", job_id, pid, cmd);
  fflush(stdout);
}

// Prints a start up message for background processes
void print_job_bg_start(int job_id, pid_t pid, const char* cmd) {
  printf("Background job started: ");
  print_job(job_id, pid, cmd);
}

// Prints a completion message followed by the print job
void print_job_bg_complete(int job_id, pid_t pid, const char* cmd) {
  printf("Completed: \t");
  print_job(job_id, pid, cmd);
}

/***************************************************************************
 * Functions to process commands
 ***************************************************************************/
// Run a program reachable by the path environment variable, relative path, or
// absolute path
void run_generic(GenericCommand cmd) {
  // Execute a program with a list of arguments. The `args` array is a NULL
  // terminated (last string is always NULL) list of strings. The first element
  // in the array is the executable
  char* exec = cmd.args[0];
  char** args = cmd.args;
  // TODO: Remove warning silencers
  // TODO: Implement run generic
  if(execvpe(exec,args)<0){
  }

  perror("ERROR: Failed to execute program");
}

// Print strings
void run_echo(EchoCommand cmd) {
  // Print an array of strings. The args array is a NULL terminated (last
  // string is always NULL) list of strings.
  char** str = cmd.args;

  // TODO: Implement echo
  int i=0;
  while(str[i]!=NULL){
	printf("%s ",str[i]);
	i++;
  }
  printf("\n");
  // Flush the buffer before returning
  fflush(stdout);
}

// Sets an environment variable
void run_export(ExportCommand cmd) {
  // Write an environment variable
  const char* env_var = cmd.env_var;
  const char* val = cmd.val;

  // TODO: Implement export.
  if(setenv(env_var,val,1)==0){
    printf("Successfully set %s = %s\n",env_var,val);
  }

}

// Changes the current working directory
void run_cd(CDCommand cmd) {
  // Get the directory name
  const char* dir = cmd.dir;
  // Check if the directory is valid
  if (dir == NULL) {
    perror("ERROR: Failed to resolve path");
    return;
  }

  // TODO: Change directory
  chdir(dir);
  // TODO: Update the PWD environment variable to be the new current working
  // directory and optionally update OLD_PWD environment variable to be the old
  // working directory.
  setenv("OLD_PWD",lookup_env("PWD"),1);
  setenv("PWD",dir,1);
}

// Sends a signal to all processes contained in a job
void run_kill(KillCommand cmd) {
  int signal = cmd.sig;
  int job_id = cmd.job;

  // TODO: Remove warning silencers
  (void) signal; // Silence unused variable warning
  (void) job_id; // Silence unused variable warning

  // TODO: Kill all processes associated with a background job
  IMPLEMENT_ME();

}


// Prints the current working directory to stdout
void run_pwd() {
  // TODO: Print the current working directory
  char pwd[1024];
  getcwd(pwd,sizeof(pwd));
  printf("%s\n",pwd);
  // Flush the buffer before returning
  fflush(stdout);
}

// Prints all background jobs currently in the job list to stdout
void run_jobs() {
  // TODO: Print background jobs
  int number_of_jobs = length_job_queue(&job_q);
  for(int i=0; i < number_of_jobs;i++){
    struct Job current_job = pop_front_job_queue(&job_q);
    print_job(current_job.job_id,current_job.pid,current_job.cmd_job);
    push_back_job_queue(&job_q,current_job);
  }
  // Flush the buffer before returning
  fflush(stdout);
}

/***************************************************************************
 * Functions for command resolution and process setup
 ***************************************************************************/

/**
 * @brief A dispatch function to resolve the correct @a Command variant
 * function for child processes.
 *
 * This version of the function is tailored to commands that should be run in
 * the child process of a fork.
 *
 * @param cmd The Command to try to run
 *
 * @sa Command
 */
void child_run_command(Command cmd) {
  CommandType type = get_command_type(cmd);

  switch (type) {
  case GENERIC:
    run_generic(cmd.generic);
    break;

  case ECHO:
    run_echo(cmd.echo);
    break;

  case PWD:
    run_pwd();
    break;

  case JOBS:
    run_jobs();
    break;

  case EXPORT:
  case CD:
  case KILL:
  case EXIT:
  case EOC:
    break;

  default:
    fprintf(stderr, "Unknown command type: %d\n", type);
  }
}

/**
 * @brief A dispatch function to resolve the correct @a Command variant
 * function for the quash process.
 *
 * This version of the function is tailored to commands that should be run in
 * the parent process (quash).
 *
 * @param cmd The Command to try to run
 *
 * @sa Command
 */
void parent_run_command(Command cmd) {
  CommandType type = get_command_type(cmd);

  switch (type) {
  case EXPORT:
    run_export(cmd.export);
    break;

  case CD:
    run_cd(cmd.cd);
    break;

  case KILL:
    run_kill(cmd.kill);
    break;

  case GENERIC:
  case ECHO:
  case PWD:
  case JOBS:
  case EXIT:
  case EOC:
    break;

  default:
    fprintf(stderr, "Unknown command type: %d\n", type);
  }
}

/**
 * @brief Creates one new process centered around the @a Command in the @a
 * CommandHolder setting up redirects and pipes where needed
 *
 * @note Processes are not the same as jobs. A single job can have multiple
 * processes running under it. This function creates a process that is part of a
 * larger job.
 *
 * @note Not all commands should be run in the child process. A few need to
 * change the quash process in some way
 *
 * @param holder The CommandHolder to try to run
 *
 * @sa Command CommandHolder
 */
void create_process(CommandHolder holder, int i) {
  // Read the flags field from the parser
  bool p_in  = holder.flags & PIPE_IN;
  bool p_out = holder.flags & PIPE_OUT;
  bool r_in  = holder.flags & REDIRECT_IN;
  bool r_out = holder.flags & REDIRECT_OUT;
  bool r_app = holder.flags & REDIRECT_APPEND; // This can only be true if r_out
                                               // is true

  // TODO: Remove warning silencers
//  (void) p_in;  // Silence unused variable warning
  //(void) p_out; // Silence unused variable warning
  //(void) r_in;  // Silence unused variable warning
  //(void) r_out; // Silence unused variable warning
  //(void) r_app; // Silence unused variable warning

  // TODO: Setup pipes, redirects, and new process
  if(p_out){
    pipe(pipes[i%2]);
  }
  pid_t child_pid;
  child_pid = fork();
  push_back_pid_queue(&pid_q,child_pid);
  if(child_pid==0){
    if(p_in){
      dup2(pipes[(i-1)%2][0],STDIN_FILENO);
      close(pipes[(i-1)%2][0]);
    }
    if(p_out){
      dup2(pipes[i%2][1],STDOUT_FILENO);
      close(pipes[i%2][1]);
    }
    if(r_in){
      FILE* file = fopen(holder.redirect_in, "r");
      dup2(fileno(file),STDIN_FILENO);
    }
    if(r_out){
      if(r_app){
        FILE* file = fopen(holder.redirect_out,"a");
        dup2(fileno(file),STDOUT_FILENO);
      }else{
        FILE* file = fopen(holder.redirect_out,"w");
        dup2(fileno(file),STDOUT_FILENO);
      }
    }
    child_run_command(holder.cmd); // This should be done in the child branch of a fork
    exit(0);
  }else{
    if(p_out){
      close(pipes[(i)%2][1]);
    }
    parent_run_command(holder.cmd); // This should be done in the parent branch of

  }
}

// Run a list of commands
void run_script(CommandHolder* holders) {
  if(!initialized){
    job_q = new_job_queue(1);
    initialized=true;
  }

  pid_q = new_pid_queue(1);
  if (holders == NULL)
    return;
  check_jobs_bg_status();

  if (get_command_holder_type(holders[0]) == EXIT &&
      get_command_holder_type(holders[1]) == EOC) {
    end_main_loop();
    return;
  }

  CommandType type;

  // Run all commands in the `holder` array
  for (int i = 0; (type = get_command_holder_type(holders[i])) != EOC; ++i)
    create_process(holders[i],i);

  if (!(holders[0].flags & BACKGROUND)) {
    // Not a background Job
    // TODO: Wait for all processes under the job to complete
    while(!is_empty_pid_queue(&pid_q)){
      pid_t current_pid = pop_front_pid_queue(&pid_q);
      int current_status;
      waitpid(current_pid,&current_status,0);
    }
    destroy_pid_queue(&pid_q);
  }
  else {
    // A background job.
    // TODO: Push the new job to the job queue
    struct Job current_job;
    current_job.job_id = job_number;
    job_number = job_number+1;
    current_job.pid_q = pid_q;
    current_job.cmd_job = get_command_string();
    current_job.pid = peek_back_pid_queue(&pid_q);
    push_back_job_queue(&job_q,current_job);
    // TODO: Once jobs are implemented, uncomment and fill the following line
    print_job_bg_start(current_job.job_id, current_job.pid, current_job.cmd_job);
  }
}
