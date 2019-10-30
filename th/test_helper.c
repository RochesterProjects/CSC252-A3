#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

int th_check(int check, const char *message, ...) {
  int sz = 1024, szr;
  char buffer[sz];
  va_list argp;

  va_start(argp, message);
  szr = vsnprintf(buffer, sz, message, argp);
  va_end(argp);

  if(szr >= sz) {
	fprintf(stderr, "ERROR: Ran out of buffer space in th_check (needed: %d)\n", szr);
    exit(1);
  }

  fprintf(stderr, "%s: %s\n", check ? "PASS" : "FAIL", buffer);

  return check;
}
