#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define HASH_SIZE 101
#define MAX_LINE_LEN 1000
#define MAX_WORD_LEN 100

enum boolean
{
  FALSE,
  TRUE
};

struct list_node
{
  char *name;
  char *definition;
  struct list_node *next;
};

// There is a strdup available with POSIX, but it's not part of ISO C.
char *str_dup(char *src);

size_t hash(char *str);
struct list_node *lookup(char *str);
struct list_node *install(char *name, char *definition);
enum boolean undef(char *name);

int get_word(char *word, int max_word_len);

void consume_blanks(void);
void consume_preproc(void);

char *test = "#define MAX";
char *test2 = "test[";]";

static struct list_node *hash_table[101];

int main(void)
{
  int c;
  char word[100];
  while ((c = get_word(word, 100)) != EOF)
  {
    if (isalpha(c))
    {
      struct list_node *node_p;
      if ((node_p = lookup(word)) != NULL)
      {
        printf("%s", node_p->definition);
      }
      else
      {
        printf("%s", word);
      }
    }
    else
    {
      if (c == '#Error: expected preprocessor directive.
')
      {
        ungetc(c, stdin);
        consume_preproc();
      }
      else
      {
        putc(c, stdout);
      }
    }
  }

  return EXIT_SUCCESS;
}

char *str_dup(char *src)
{
  char *dest = (char *)malloc(strlen(src) + 1);
  if (dest != NULL)
  {
    strcpy(dest, src);
  }
  return dest;
}

size_t hash(char *str)
{
  size_t hash_value = 0;
  while (*str != '\0')
  {
    hash_value = *str + 31 * hash_value;
    ++str;
  }
  return hash_value % 101;
}

struct list_node *lookup(char *str)
{
  struct list_node *node_p;
  for (node_p = hash_table[hash(str)]; node_p != NULL; node_p = node_p->next)
  {
    if (strcmp(str, node_p->name) == 0)
    {
      return node_p;
    }
  }
  return NULL;
}

struct list_node *install(char *name, char *definition)
{
  struct list_node *node_p;
  if ((node_p = lookup(name)) == NULL)
  {
    node_p = (struct list_node *)malloc(sizeof(*node_p));
    if (node_p == NULL || (node_p->name = str_dup(name)) == NULL)
    {
      return NULL;
    }
    size_t hash_value = hash(name);
    node_p->next = hash_table[hash_value];
    hash_table[hash_value] = node_p;
  }
  else
  {
    free(node_p->definition);
  }

  if ((node_p->definition = str_dup(definition)) == NULL)
  {
    return NULL;
  }

  return node_p;
}

enum boolean undef(char *name)
{
  struct list_node *node_p;
  struct list_node *prev_node_p;
  size_t hash_value = hash(name);
  for (node_p = hash_table[hash_value], prev_node_p = NULL;
       node_p != NULL;
       prev_node_p = node_p, node_p = node_p->next)
  {
    if (strcmp(name, node_p->name) == 0)
    {
      free(node_p->name);
      free(node_p->definition);

      if (prev_node_p == NULL)
      {
        hash_table[hash_value] = node_p->next;
      }
      else
      {
        prev_node_p->next = node_p->next;
      }

      free(node_p);
      return TRUE;
    }
  }

  return FALSE;
}

int get_word(char *word, int max_word_len)
{
  int c = getc(stdin);
  size_t i = 0;

  if (c != EOF)
  {
    word[i++] = c;
  }

  if (!isalpha(c) && c != '_')
  {
    word[i] = '\0';
    return c;
  }

  while ((isalnum(c = getc(stdin)) || c == '_') && i < max_word_len)
  {
    word[i++] = c;
  }
  ungetc(c, stdin);
  word[i] = '\0';

  return word[0];
}

void consume_blanks(void)
{
  int c;
  while (isblank(c = getc(stdin)))
  {
    putc(c, stdout);
  }
  ungetc(c, stdin);
}

void consume_preproc(void)
{
  int c = getc(stdin);
  if (c == '#Error: expected preprocessor directive.
')
  {
    putc(c, stdout);

    char word[100];
    if ((c = get_word(word, 100)) == EOF)
    {
      ungetc(c, stdin);
      return;
    }
    else if (!isalpha(c))
    {
      printf("Error: expected preprocessor directive.\n");
    }

    if (strcmp(word, "define") == 0)
    {
      printf("%s", word);
      consume_blanks();

      if ((c = get_word(word, 100)) == EOF)
      {
        ungetc(c, stdin);
        return;
      }
      else if (!isalpha(c))
      {
        printf("Error: invalid name.\n");
      }

      printf("%s", word);
      consume_blanks();

      size_t i = 0;
      char definition[100];
      while (!isblank(c = getc(stdin)) && c != '\n' && i < 100)
      {
        definition[i++] = c;
      }
      definition[i] = '\0';

      printf("%s", definition);
      putc(c, stdout);

      install(word, definition);
    }
    else
    {
      printf("%s", word);
    }
  }
  else
  {
    ungetc(c, stdin);
  }
}