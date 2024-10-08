#include <stddef.h>
#include <unistd.h>
#include <termio.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>

#include "start_debugger.h"
#include "data.h"
#include "parse_debugger_output.h"
#include "utilities.h"
#include "plugins.h"
#include "insert_output_marker.h"
#include "update_window_data/_update_window_data.h"
#include "update_window_data/get_binary_path_time.h"

static int configure_debugger (debugger_t*);
static int start_debugger_proc (state_t*);



int
start_debugger (state_t *state)
{
    int ret;

    ret = configure_debugger (state->debugger);
    if (ret == FAIL) {
        pfemr ("Failed to configure debugger");
    }

    ret = start_debugger_proc (state);
    if (ret == FAIL) {
        pfemr ("Failed to start debugger process");
    }

    ret = insert_output_end_marker (state);
    if (ret == FAIL) {
        pfemr (ERR_OUT_MARK);
    }

    ret = parse_debugger_output (state);
    if (ret == FAIL) {
        pfemr (ERR_DBG_PARSE);
    }

    state->plugins[Dbg]->win->buff_data->new_data = true;
    ret = update_window (Dbg, state);
    if (ret == FAIL) {
        pfemr (ERR_UPDATE_WIN);
    }

    ret = get_binary_path_time (state);
    if (ret == FAIL) {
        pfemr ("Failed to get binary path, time");
    }

    return A_OK;
}



static int
configure_debugger (debugger_t *debugger)
{
    if ((debugger->format_buffer = (char*) malloc (sizeof (char) * ORIG_BUF_LEN)) == NULL) {
        pfem ("malloc error: %s", strerror (errno));
        pemr ("Format buffer allocation failed");
    }
    debugger->format_buffer[0] = '\0';
    debugger->format_len = ORIG_BUF_LEN;
    debugger->format_pos = 0;
    debugger->format_times_doubled = 0;

    if ((debugger->data_buffer = (char*) malloc (sizeof (char) * ORIG_BUF_LEN)) == NULL) {
        pfem ("malloc error: %s", strerror (errno));
        pemr ("Data buffer allocation failed");
    }
    debugger->data_buffer[0] = '\0';
    debugger->data_len = ORIG_BUF_LEN;
    debugger->data_pos = 0;
    debugger->data_times_doubled = 0;

    if ((debugger->cli_buffer = (char*) malloc (sizeof (char) * ORIG_BUF_LEN)) == NULL) {
        pfem ("malloc error: %s", strerror (errno));
        pemr ("CLI buffer allocation failed");
    }
    debugger->cli_buffer[0] = '\0';
    debugger->cli_len = ORIG_BUF_LEN;;
    debugger->cli_pos = 0;
    debugger->cli_times_doubled = 0;

    if ((debugger->program_buffer = (char*) malloc (sizeof (char) * ORIG_BUF_LEN)) == NULL) {
        pfem ("malloc error: %s", strerror (errno));
        pemr ("Program out buffer allocation failed");
    }
    debugger->program_buffer[0] = '\0';
    debugger->program_len = ORIG_BUF_LEN;;
    debugger->program_pos = 0;
    debugger->program_times_doubled = 0;

    if ((debugger->async_buffer = (char*) malloc (sizeof (char) * ORIG_BUF_LEN)) == NULL) {
        pfem ("malloc error: %s", strerror (errno));
        pemr ("Async buffer allocation failed");
    }
    debugger->async_buffer[0] = '\0';
    debugger->async_len = ORIG_BUF_LEN;
    debugger->async_pos = 0;
    debugger->async_times_doubled = 0;

    return A_OK;
}



static int
start_debugger_proc (state_t *state)
{
    pid_t   debugger_pid;
    int     i,
            debug_in_pipe  [2],
            debug_out_pipe [2];

    debugger_t *debugger = state->debugger;
    
    // create pipes
    if (pipe (debug_in_pipe)  == -1 || 
        pipe (debug_out_pipe) == -1)
    {
        pfemr ("Debugger pipe creation failed");
    }
    debugger->stdin_pipe  = debug_in_pipe [PIPE_WRITE];
    debugger->stdout_pipe = debug_out_pipe [PIPE_READ];

    // fork
    debugger_pid = fork ();
    if (debugger_pid  == -1)
    {
        pfemr ("Debugger fork failed");
    }

    // start debugger
    if (debugger_pid == 0) {

        dup2  (debug_in_pipe  [PIPE_READ], STDIN_FILENO);
        close (debug_in_pipe  [PIPE_READ]);
        close (debug_in_pipe  [PIPE_WRITE]);

        dup2  (debug_out_pipe [PIPE_WRITE], STDOUT_FILENO);
        close (debug_out_pipe [PIPE_WRITE]);
        close (debug_out_pipe [PIPE_READ]);

        execvp (state->command[0], state->command);

        i = 0;
        do {
            strcat (state->debugger->format_buffer, state->command[i++]);
            strcat (state->debugger->format_buffer, " ");
        } while (state->command [i] != NULL);
        pfemr ("execvp error with command: \"%s\"", state->debugger->format_buffer);
    }

    if (debugger_pid > 0) {
    
        debugger->running = true;

        close (debug_in_pipe   [PIPE_READ]);
        close (debug_out_pipe  [PIPE_WRITE]);
    }

    return A_OK;
}

