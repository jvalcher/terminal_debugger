#include <string.h>
#include <stddef.h>
#include <stdbool.h>
#include <unistd.h>
#include <ctype.h>

#include "data.h"

char *flush_cmd  = "call fflush (0)\n";
char *set_tty = "-inferior-tty-set /dev/pts/6\n";


/*
    Codes
    -----

termide_back,       "Bak"
termide_builds,     "Bld"
termide_layouts,    "Lay"

gdb_assembly,       "Asm"
gdb_breakpoints,    "Brk"
gdb_continue,       "Con"
gdb_finish,         "Fin"
gdb_kill,           "Kil"
gdb_local_vars,     "LcV"
gdb_next,           "Nxt"
gdb_output,         "Out"
gdb_prompt,         "Prm"
gdb_print,          "Prn"
gdb_registers,      "Reg"
gdb_run,            "Run"
gdb_src_file,       "Src"
gdb_step,           "Stp"
gdb_watches         "Wat"

    TODO:
    ----

    - Add mutex that sets when writing to pipe and doesn't unlock until (gdb) prompt seen in output process

    -interpreter-exec  for typed commands
*/

int gdb_run (debug_state_t *state)
{
    ssize_t  bytes    = 0;
    char    *set_tty  = "-inferior-tty-set ";
    char    *gdb_run  = "-exec-run\n";

    // set program stdout to current tty
    char *tty_device = ttyname(STDIN_FILENO);
    bytes += write (state->input_pipe, set_tty,  strlen (set_tty));
    bytes += write (state->input_pipe, tty_device,  strlen (tty_device));
    bytes += write (state->input_pipe, "\n",  strlen ("\n"));

    // run program
    bytes += write (state->input_pipe, gdb_run, strlen (gdb_run));

    return (int)bytes;
}


int gdb_set_breakpoint (debug_state_t *state)
{
    ssize_t bytes       = 0;
    char *set_break_cmd = "-break-insert ",
         *break_loc     = state->break_point;

    // set breakpoint
    bytes += write (state->input_pipe, set_break_cmd, strlen (set_break_cmd));
    bytes += write (state->input_pipe, break_loc, strlen (break_loc));
    bytes += write (state->input_pipe, "\n", strlen ("\n"));

    return (int)bytes;
}


int gdb_next (debug_state_t *state)
{
    ssize_t bytes;
    char *next_cmd = "-exec-next\n";

    bytes = write (state->input_pipe, next_cmd, strlen (next_cmd));

    return (int)bytes;
}


int gdb_continue (debug_state_t *state)
{
    ssize_t bytes;
    char *cont_cmd = "-exec-continue\n";

    bytes = write (state->input_pipe, cont_cmd, strlen (cont_cmd));

    return (int)bytes;
}


int gdb_exit (debug_state_t *state)
{
    ssize_t bytes;
    char *exit_cmd = "-gdb-exit\n";

    bytes =  write (state->input_pipe, exit_cmd, strlen (exit_cmd));

    return (int)bytes;
}


/*
   Non-plugin commands
*/


int gdb_get_local_vars (debug_state_t *state)
{
    ssize_t bytes;
    char *exit_cmd = "info locals\n";

    bytes = write (state->input_pipe, exit_cmd, strlen (exit_cmd));

    return (int)bytes;
}


/*
    Update window data
*/



/*
    Parse GDB output
*/
void gdb_parse_output (char *in_buffer, char *debug_out_buffer, char *program_out_buffer)
{
    char ch;
    int  i = 0,
         d = 0,
         p = 0;
    bool is_gdb_output = false,
         is_prog_output = false,
         is_newline = true,
         is_new_string = false;

    while (1) {

        ch = in_buffer[i++];

        // set type of output for new line
        if (is_newline && ch != '\n') {
            is_newline = false;
            if (ch == '~') {
                is_gdb_output = true;
                is_new_string = true;
            } 
            if (isalpha (ch)) {
                is_prog_output = true;
                --i;
            }
        }

        // end of line
        else if (ch == '\n') {
            is_newline = true;
            if (is_gdb_output) {
                is_gdb_output = false;
                if (debug_out_buffer [d-1] == '\"') {     // remove trailing \"
                    --d;
                }
                debug_out_buffer [d++] = ch;
            }
            if (is_prog_output) {
                is_prog_output = false;
                program_out_buffer [p++] = ch;
            }
        }

        // gdb output
        else if (is_gdb_output) {
            if (is_new_string) {            // remove leading \"
                is_new_string = false;
                if (ch == '\"') {
                    goto skip_char;
                }
            }
            debug_out_buffer [d++] = ch;
            skip_char:
        }

        // program output
        else if (is_prog_output) {
            program_out_buffer [p++] = ch;
        }

        // end of buffer
        else if (ch == '\0') {
            debug_out_buffer [d] = '\0';
            program_out_buffer [p] = '\0';
            break;
        }
    }
}


