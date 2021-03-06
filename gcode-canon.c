/*
 ============================================================================
 Name        : gcode-canon.c
 Author      : Radu - Eosif Mihailescu
 Version     : 1.0 (2012-08-11)
 Copyright   : (C) 2012 Radu - Eosif Mihailescu <radu.mihailescu@linux360.ro>
 Description : Canonical G-Code Interpreter Main Program
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

#include "gcode-commons.h"
#include "gcode-parameters.h"
#include "gcode-tools.h"
#include "gcode-input.h"
#include "gcode-machine.h"
#include "gcode-state.h"
#include "gcode-stacks.h"
#include "gcode-cycles.h"
#include "gcode-queue.h"
#include "gcode-checker.h"


int main(int argc, char *argv[]) {
  FILE *parFile = fopen(GCODE_PARAMETER_STORE, "r");
  FILE *inputFile = (argc > 1 ? fopen(argv[1], "r") : stdin);
  char line[0xFF];

  init_parameters(parFile);
  init_machine(NULL);
  init_stacks(NULL);
  init_tools(NULL);
  init_input(inputFile);
  //TODO: align API, add done_gcode_state().
  init_gcode_state(NULL);
  init_cycles(NULL);
  //TODO: align API, make it take a pointer to init data.
  init_queue();
  init_checker(NULL);

  while(machine_running() && gcode_running() && fetch_line_input(line)) {
    if(gcode_check(line)) update_gcode_state(line);
    move_machine_queue();
  }
  /* Flush movement queue */
  while(move_machine_queue());

  done_checker();
  done_queue();
  done_cycles();
  done_input();
  done_tools();
  done_stacks();
  done_machine();
  done_parameters();

  return 0;
}
