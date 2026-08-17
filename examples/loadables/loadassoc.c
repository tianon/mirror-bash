/* loadassoc - treat the arguments as key-value pairs and assign them
 	       sequentially to the associative array supplied as an
 	       argument to the -a option. Very similar to the `kv' builtin. */

/*
   This allows you to copy an existing associative array `assoc' like:
	loadassoc -A copy "${assoc[@]@k}"
*/

/*
   Copyright (C) 2026 Free Software Foundation, Inc.

   This file is part of GNU Bash.
   Bash is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Bash is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <config.h>

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif
#include "bashansi.h"
#include <stdio.h>

#include "loadables.h"

#define LOADASSOC_ARRAY_DEFAULT	"ASSOC"

static int
kvlist (SHELL_VAR *var, WORD_LIST *list)
{
  WORD_LIST *k, *v;
  char *key, *val;
  int r;

  r = 0;
  for (k = list; k; k = v->next)
    {
      v = k->next;

      key = savestring (k->word->word);
      if (key == 0 || *key == '\0')
	{
	  err_badarraysub (k->word->word);
	  free (key);
          continue;
	}

      val = v ? savestring (v->word->word) : 0;
      if (val == 0)
	{
	  val = (char *)xmalloc (1);
	  val[0] = '\0';
	}

      r += bind_assoc_variable (var, name_cell (var), key, val, 0) != 0;

      free (val);

      if (v == 0)
	break;
    }

  return r;
}

int
loadassoc_builtin (WORD_LIST *list)
{
#if defined (ARRAY_VARS)
  int opt, rval, unset;
  char *array_name;
  SHELL_VAR *v;

  array_name = 0;
  rval = EXECUTION_SUCCESS;
  unset = 1;

  reset_internal_getopt ();
  while ((opt = internal_getopt (list, "A:a")) != -1)
    {
      switch (opt)
	{
	case 'A':
	  array_name = list_optarg;
	  break;
	case 'a':
	  unset = 0;
	  break;
	CASE_HELPOPT;
	default:
	  builtin_usage ();
	  return (EX_USAGE);
	}
    }
  list = loptend;

  if (array_name == 0)
    array_name = LOADASSOC_ARRAY_DEFAULT;

  if (valid_identifier (array_name) == 0)
    {
      sh_invalidid (array_name);
      return (EXECUTION_FAILURE);
    }

  v = find_or_make_array_variable (array_name, 3);
  if (v == 0 || readonly_p (v) || noassign_p (v))
    {
      if (v && readonly_p (v))
	err_readonly (array_name);
      return (EXECUTION_FAILURE);
    }
  else if (assoc_p (v) == 0)
    {
      builtin_error ("%s: not an associative array", array_name);
      return (EXECUTION_FAILURE);
    }
  if (invisible_p (v))
    VUNSETATTR (v, att_invisible);

  if (unset)
    assoc_flush (assoc_cell (v));

  rval = list ? kvlist (v, list) : 1;	/* no args is ok */

  return (rval > 0 ? EXECUTION_SUCCESS : EXECUTION_FAILURE);
#else
  builtin_error ("arrays not available");
  return (EXECUTION_FAILURE);
#endif
}

char *loadassoc_doc[] = {
	"Assign arguments as keys and values of an associative array.",
	"",
	"Take arguments and assign them as keys and corresponding values to",
	"an associative array. The array name is supplied as the argument to",
	"the -A option. ASSOC is the default associative array name.",
	"",
	"If the -a option is supplied, the values are added to the existing",
	"value of the array; if it is not supplied, the array is unset before",
	"assigning any values.",
	"",
	"The return status is true if the assignment is performed successfully;",
	"false if an error occurs or if the array variable is invalid or",
	"readonly.",
	
	(char *)NULL
};

struct builtin loadassoc_struct = {
	"loadassoc",		/* builtin name */
	loadassoc_builtin,	/* function implementing the builtin */
	BUILTIN_ENABLED,	/* initial flags for builtin */
	loadassoc_doc,		/* array of long documentation strings. */
	"loadassoc [-A aname] [-a] key value ...",	/* usage synopsis; becomes short_doc */
	0			/* reserved for internal use */
};
