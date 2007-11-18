#include <stdio.h>

int main()
{
  FILE *sparfile = fopen("../lib/etc/sparrank", "r");
  FILE *sparfile2 = fopen("../lib/etc/sparranktwo", "w");
  int err;
  char buf[80];
  char name[32];

  /* We're adding the names that just fought */
  printf("Starting!\n");
  while (fgets(buf, 80, sparfile))
    {
      printf("%s", buf);
      sscanf(buf, "<< Name: %s   Sparrank: %*d >>", name);
      printf("%s\n", name);
      if (strcmp(name, "Eric") != 0)
	fprintf(sparfile2, "%s", buf);
    }
  fclose(sparfile);
  fprintf(sparfile2, "<< Name: %s   Sparrank: %d >>\n", "Bob", 8000);
  fclose(sparfile2);
 
  system("sort -rn +4 ../lib/etc/sparranktwo | head > ../lib/etc/sparrank");
  /* system("rm ../lib/etc/sparranktwo"); */

  return;
}
