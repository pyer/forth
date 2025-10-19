/* history.c -- standalone history library */

/* Copyright (C) 1989-2011 Free Software Foundation, Inc.

   This file contains the GNU History Library (History), a set of
   routines for managing the text of previously typed lines.

   History is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   History is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with History.  If not, see <http://www.gnu.org/licenses/>.
*/

/* The goal is to make the implementation transparent, so that you
   don't have to know what data types are used, just what functions
   you can call.  I think I have done that. */
#define READLINE_LIBRARY

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>

#include "history.h"
//#include "histlib.h"
//#include "xmalloc.h"

#ifndef savestring
#define savestring(x) strcpy (xmalloc (1 + strlen (x)), (x))
#endif

/* Possible definitions for what style of writing the history file we want. */
#define HISTORY_APPEND 0
#define HISTORY_OVERWRITE 1

/* **************************************************************** */
/*                                    */
/*           Memory Allocation and Deallocation.            */
/*                                    */
/* **************************************************************** */
#ifdef __STDC__
#  define PTR_T    void *
#else
#  define PTR_T    char *
#endif

static void memory_error_and_abort ( char *fname )
{
  fprintf (stderr, "%s: out of virtual memory\n", fname);
  exit (2);
}

/* Return a pointer to free()able block of memory large enough
   to hold BYTES number of bytes.  If the memory cannot be allocated,
   print an error message and abort. */
PTR_T xmalloc ( size_t bytes )
{
  PTR_T temp;

  temp = malloc (bytes);
  if (temp == 0)
    memory_error_and_abort ("xmalloc");
  return (temp);
}

PTR_T xrealloc ( PTR_T pointer, size_t bytes )
{
  PTR_T temp;

  temp = pointer ? realloc (pointer, bytes) : malloc (bytes);

  if (temp == 0)
    memory_error_and_abort ("xrealloc");
  return (temp);
}

#define FREE(x)    if (x) free (x)

/* **************************************************************** */
/* The number of slots to increase the_history by. */
#define DEFAULT_HISTORY_GROW_SIZE 50

/* An array of HIST_ENTRY.  This is where we store the history. */
static HIST_ENTRY **the_history = (HIST_ENTRY **)NULL;

/* The current number of slots allocated to the input_history. */
static int history_size;

/* If HISTORY_STIFLED is non-zero, then this is the maximum number of
   entries to remember. */
int history_max_entries;
int max_input_history;    /* backwards compatibility */

/* The current location of the interactive history pointer.  Just makes
   life easier for outside callers. */
int history_offset;

/* The number of strings currently stored in the history list. */
int history_length = 0;

/* The logical `base' of the history array.  It defaults to 1. */
int history_base = 1;

/* During tokenization, if this character is seen as the first character
   of a word, then it, and all subsequent characters upto a newline are
   ignored.  For a Bourne shell, this should be '#'.  Bash special cases
   the interactive comment character to not be a comment delimiter. */
char history_comment_char = '\0';

/* The name of the current history file */
char history_filename[210];

/* **************************************************************** */
/*                                    */
/*            History initialization                */
/*                                    */
/* **************************************************************** */

/* Begin a session in which the history functions might be used.  This
   initializes interactive variables. */
void using_history (void)
{
  const char *home;

  strcpy (history_filename, ".");
  home = getenv("HOME");
  if ( home != NULL && strlen(home) < 200 )
      strcpy (history_filename, home);
  strcat (history_filename, "/.pf_history");

  history_offset = history_length = 0;
}

/* **************************************************************** */
/*                                    */
/*            History file managment                */
/*                                    */
/* **************************************************************** */

/* Read a range of lines from FILENAME, adding them to the history list.
   Start reading at the FROM'th line and end at the TO'th.  If FROM
   is zero, start at the beginning.  If TO is less than FROM, read
   until the end of the file.  If FILENAME is NULL, then read from
   ~/.history.  Returns 0 if successful, or errno if not. */
int read_history_range ( const char *filename, int from, int to )
{
  register char *line_start, *line_end, *p;
  char *input, *buffer, *bufend;
  int file, current_line, chars_read;
  struct stat finfo;
  size_t file_size;
#if defined (EFBIG)
  int overflow_errno = EFBIG;
#elif defined (EOVERFLOW)
  int overflow_errno = EOVERFLOW;
#else
  int overflow_errno = EIO;
#endif

  buffer = (char *)NULL;
  input = history_filename;
  file = input ? open (input, O_RDONLY, 0666) : -1;

  if ((file < 0) || (fstat (file, &finfo) == -1))
    goto error_and_exit;

  file_size = (size_t)finfo.st_size;

  /* check for overflow on very large files */
  if (file_size != finfo.st_size || file_size + 1 < file_size)
    {
      errno = overflow_errno;
      goto error_and_exit;
    }

  buffer = (char *)malloc (file_size + 1);
  if (buffer == 0)
    {
      errno = overflow_errno;
      goto error_and_exit;
    }

  chars_read = read (file, buffer, file_size);
  if (chars_read < 0)
    {
  error_and_exit:
      if (errno != 0)
    chars_read = errno;
      else
    chars_read = EIO;
      if (file >= 0)
    close (file);

      FREE (buffer);
      return (chars_read);
    }

  close (file);

  /* Set TO to larger than end of file if negative. */
  if (to < 0)
    to = chars_read;

  /* Start at beginning of file, work to end. */
  bufend = buffer + chars_read;
  current_line = 0;

  /* Skip lines until we are at FROM. */
  for (line_start = line_end = buffer; line_end < bufend && current_line < from; line_end++)
    if (*line_end == '\n')
      {
          p = line_end + 1;
    current_line++;
    line_start = p;
      }

  /* If there are lines left to gobble, then gobble them now. */
  for (line_end = line_start; line_end < bufend; line_end++)
    if (*line_end == '\n')
      {
    /* Change to allow Windows-like \r\n end of line delimiter. */
    if (line_end > line_start && line_end[-1] == '\r')
      line_end[-1] = '\0';
    else
      *line_end = '\0';

    if (*line_start)
        add_history (line_start);

    current_line++;
    if (current_line >= to)
      break;

    line_start = line_end + 1;
      }

  FREE (buffer);
  history_length = current_line;
  history_offset = current_line;
  return (0);
}

/* Add the contents of FILENAME to the history list, a line at a time.
   If FILENAME is NULL, then read from ~/.history.  Returns 0 if
   successful, or errno if not. */
int read_history (void)
{
  return (read_history_range (NULL, 0, -1));
}

/* Workhorse function for writing history.  Writes NELEMENT entries
   from the history list to FILENAME.  OVERWRITE is non-zero if you
   wish to replace FILENAME with the entries. */
static int history_do_write ( const char *filename, int nelements, int overwrite )
{
  register int i;
  char *output;
  int file, mode, rv;
  mode = overwrite ? O_WRONLY|O_CREAT|O_TRUNC : O_WRONLY|O_APPEND;
  output = history_filename;

  file = output ? open (output, mode, 0600) : -1;
  rv = 0;

  if (file == -1)
    {
      rv = errno;
      return (rv);
    }

  if (nelements > history_length)
    nelements = history_length;

  /* Build a buffer of all the lines to write, and write them in one syscall.
     Suggested by Peter Ho (peter@robosts.oxford.ac.uk). */
  {
    HIST_ENTRY **the_history;    /* local */
    register int j;
    int buffer_size;
    char *buffer;

    the_history = history_list ();
    /* Calculate the total number of bytes to write. */
    for (buffer_size = 0, i = history_length - nelements; i < history_length; i++)
      {
    buffer_size += strlen (the_history[i]->line) + 1;
      }

    /* Allocate the buffer, and fill it. */
    buffer = (char *)malloc (buffer_size);
    if (buffer == 0)
      {
          rv = errno;
    close (file);
    return rv;
      }

    for (j = 0, i = history_length - nelements; i < history_length; i++)
      {
    strcpy (buffer + j, the_history[i]->line);
    j += strlen (the_history[i]->line);
    buffer[j++] = '\n';
      }

    if (write (file, buffer, buffer_size) < 0)
      rv = errno;
    FREE (buffer);
  }

  if (close (file) < 0 && rv == 0)
    rv = errno;

  return (rv);
}

/* Append NELEMENT entries to FILENAME.  The entries appended are from
   the end of the list minus NELEMENTs up to the end of the list. */
int append_history ( int nelements, const char *filename )
{
  return (history_do_write (filename, nelements, HISTORY_APPEND));
}

/* Overwrite FILENAME with the current history.  If FILENAME is NULL,
   then write the history list to ~/.history.  Values returned
   are as in read_history ().*/
int write_history (void)
{
  return (history_do_write (NULL, history_length, HISTORY_OVERWRITE));
}

/* **************************************************************** */
/*                                    */
/*            History Functions                */
/*                                    */
/* **************************************************************** */

/* Returns the magic number which says what history element we are
   looking at now.  In this implementation, it returns history_offset. */
int
where_history ()
{
  return (history_offset);
}

/* Return the current history array.  The caller has to be careful, since this
   is the actual array of data, and could be bashed or made corrupt easily.
   The array is terminated with a NULL pointer. */
HIST_ENTRY ** history_list (void)
{
  return (the_history);
}

/* Return the history entry at the current position, as determined by
   history_offset.  If there is no entry there, return a NULL pointer. */
HIST_ENTRY * current_history (void)
{
  return ((history_offset == history_length) || the_history == 0)
        ? (HIST_ENTRY *)NULL
        : the_history[history_offset];
}

/* Back up history_offset to the previous history entry, and return
   a pointer to that entry.  If there is no previous entry then return
   a NULL pointer. */
HIST_ENTRY * previous_history (void)
{
  return history_offset ? the_history[--history_offset] : (HIST_ENTRY *)NULL;
}

/* Move history_offset forward to the next history entry, and return
   a pointer to that entry.  If there is no next entry then return a
   NULL pointer. */
HIST_ENTRY * next_history (void)
{
  return (history_offset == history_length) ? (HIST_ENTRY *)NULL : the_history[++history_offset];
}

/* Return the history entry which is logically at OFFSET in the history array.
   OFFSET is relative to history_base. */
HIST_ENTRY * history_get ( int offset )
{
  int local_index;

  local_index = offset - history_base;
  return (local_index >= history_length || local_index < 0 || the_history == 0)
        ? (HIST_ENTRY *)NULL
        : the_history[local_index];
}

HIST_ENTRY * alloc_history_entry ( char *string )
{
  HIST_ENTRY *temp;

  temp = (HIST_ENTRY *)xmalloc (sizeof (HIST_ENTRY));

  temp->line = string ? savestring (string) : string;
  temp->data = (char *)NULL;
  return temp;
}

/* Place STRING at the end of the history list.  The data field
   is  set to NULL. */
void add_history ( char *string )
{
  HIST_ENTRY *temp;

      if (history_size == 0)
    {
      history_size = DEFAULT_HISTORY_GROW_SIZE;
      the_history = (HIST_ENTRY **)xmalloc (history_size * sizeof (HIST_ENTRY *));
      history_length = 1;
    }
      else
    {
      if (history_length == (history_size - 1))
        {
          history_size += DEFAULT_HISTORY_GROW_SIZE;
          the_history = (HIST_ENTRY **)
        xrealloc (the_history, history_size * sizeof (HIST_ENTRY *));
        }
      history_length++;
          history_offset = history_length;
    }
  temp = alloc_history_entry (string);
  the_history[history_length] = (HIST_ENTRY *)NULL;
  the_history[history_length - 1] = temp;
}

