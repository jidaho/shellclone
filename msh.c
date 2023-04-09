// The MIT License (MIT)
//
// Copyright (c) 2016 Trevor Bakker
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n" // We want to split our command line up into tokens
                           // so we need to define what delimits our tokens.
                           // In this case  white space
                           // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255 // The maximum command-line size

#define MAX_NUM_ARGUMENTS 5 // Mav shell only supports four arguments
#define MAX_HISTORY_SIZE 15



int main()
{

  char *command_string = (char *)malloc(MAX_COMMAND_SIZE);
  char *history[MAX_HISTORY_SIZE];
  int count = 0;
  int pcount = 0;
  int pid_arr[MAX_HISTORY_SIZE];
  char *ptr;
  long n;

  while (1)
  {
    // Print out the msh prompt
    printf("msh> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while (!fgets(command_string, MAX_COMMAND_SIZE, stdin))
      ;
        start: ;

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    for (int i = 0; i < MAX_NUM_ARGUMENTS; i++)
    {
      token[i] = NULL;
    }
    int token_count = 0;

    // Pointer to point to the token
    // parsed by strsep
    char *argument_ptr = NULL;
    char *working_string = strdup(command_string);

    // we are going to move the working_string pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *head_ptr = working_string;

    // Tokenize the input strings with whitespace used as the delimiter
    while (((argument_ptr = strsep(&working_string, WHITESPACE)) != NULL) &&
           (token_count < MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup(argument_ptr, MAX_COMMAND_SIZE);
      // putting a copy of the command string into the history array
      history[count] = strdup(command_string);
      if (strlen(token[token_count]) == 0)
      {
        token[token_count] = NULL;
      }
      token_count++;
    }
    count++;

    // Now print the tokenized input as a debug check
    // \TODO Remove this for loop and replace with your shell functionality
    // checking for blank input
    if (token[0] != NULL && strcmp(token[0], "\n") == 0)
    {
      continue;
    }
    // checking for exit 
    else if (token[0] != NULL && strcmp(token[0], "exit") == 0)
    {
      exit(0);
    }
    // checking for quit 
    else if (token[0] != NULL && strcmp(token[0], "quit") == 0)
    {
      exit(0);
    }
    // check for cd
    else if (token[0] != NULL && strcmp(token[0], "cd") == 0)
    {
      if (chdir(token[1]) == 0)
      {
        chdir(token[1]);
        // putting -1 in array for cd
        pid_arr[pcount++] = -1;
      }
      else
      {
        perror("cd failed");
      }
      // putting -1 in array for cd
      pid_arr[pcount++] = -1;
    }
    // check for history
    else if (token[0] != NULL && strcmp(token[0], "history") == 0)
    {
      if (token[1] != NULL && strcmp(token[1], "-p") == 0)
      {
          // printing pid history
        for (int k = 0; k < count; k++)
        {
          printf("%d: %d\n", k, pid_arr[k]);
          // putting -1 in array for history
          pid_arr[pcount++] = -1;
        }
      }
      else
      {
        // printing regular history. Doesn't work past 8 for some reason.
        for (int j = 0; j < count; j++)
        {
          printf("%d: %s", j, history[j]);
          // putting -1 in array for history
          pid_arr[pcount++] = -1;
        }
      }
    }
    // checking for a ! in string
    else if (token[0] != NULL && strstr(token[0], "!"))
    {
      // getting the number from the !n
      n = strtol(token[0]+1, &ptr, 10); 
      if (n < 0 || n > count)
      {
        printf("Command not in history.");
      }
      else
      {
        // re-executing the command by going back to the beginning of loop 
        // with the chosen command saved in the command string
        command_string = history[n];
        goto start;
      }
    }
    else
    {
      pid_t pid = fork();
      if (pid == 0)
      {
        // everything else in execvp
        int ret = execvp(token[0], token);
        if (ret == -1)
        {
          printf("%s: Command not found.\n", token[0]);
          exit(0);
        }
      }
      else
      {
        int status;
        wait(&status);
      }
      // putting pid from parent in the array
      pid_arr[pcount++] = pid;
    }

    // Cleanup allocated memory
    for (int i = 0; i < MAX_NUM_ARGUMENTS; i++)
    {
      if (token[i] != NULL)
      {
        free(token[i]);
      }
    }
    free(head_ptr);
  }
  free(command_string);

  return 0;
  // e2520ca2-76f3-90d6-0242ac120003
}
