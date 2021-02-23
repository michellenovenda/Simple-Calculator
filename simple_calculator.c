#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAXLEN 256
#define SIZE 65536

typedef enum {UNKNOWN, END, INT, ID, ORANDXOR, ADDSUB, MULDIV, ASSIGN, LPAREN, RPAREN, ENDFILE} TokenSet;

typedef struct _Type{
  char alphabet[MAXLEN];
  int value;
} type;

typedef struct _Node {
    char lexeme[MAXLEN];
    TokenSet token;
    int val;
    struct _Node *left, *right;
} BTNode ;

BTNode* factor(void);
BTNode* term(void);
BTNode* term_tail(BTNode *);
BTNode* expr(void);
BTNode* expr_tail(BTNode *);
void statement(void);
char* getLexeme(void) ;
TokenSet getToken(void) ;
void advance(void) ;
void error() ;
int match(TokenSet) ;
int evaluateTree(BTNode *) ;
void printPrefix(BTNode *) ;
void freeTree(BTNode *);
BTNode* makeNode(TokenSet, const char *);
int getval(void);
int setval(char *, int);
void not_var(BTNode *);
int division_error(BTNode *);

TokenSet lookahead = UNKNOWN;
char lexeme[MAXLEN];
type arr[SIZE];
int totalvar;
int idx;
int flag = 0;
int get_flag = 0;
int count = 0;
int cnt = 3;
int i = 0;
int division = 0;
int div_flag = 0;
int min = 0;

int main(void)
{
    arr[0].alphabet[0] = 'x';
    arr[1].alphabet[0] = 'y';
    arr[2].alphabet[0] = 'z';

    arr[0].alphabet[1] = '\0';
    arr[1].alphabet[1] = '\0';
    arr[2].alphabet[1] = '\0';

    arr[0].value = 1;
    arr[1].value = 1;
    arr[2].value = 1;

    totalvar = 3;

    printf("MOV r0 [0]\nMOV r1 [4]\nMOV r2 [8]\n");

    while (1)
    {
      statement();
    }
  return 0;
}

void statement(void)
{
    BTNode* retp;

    if (match(ENDFILE))
    {
      printf("EXIT 0\n");
      exit(0);
    }

    else if (match(END))
    {
      advance();
    }

    else
    {
        retp = expr();

         if(retp->lexeme[0] != '=')
           error();

        if (match(END))
        {
            not_var(retp->right);
            division_error(retp->right);
            evaluateTree(retp);
            freeTree(retp);
            count = 0;
            div_flag = 0;
            advance();
        }
    }
}

void advance(void)
{
    lookahead = getToken();
}

int match(TokenSet token)
{
    if (lookahead == UNKNOWN)
      advance();

    return token == lookahead;
}

char* getLexeme(void)
{
    return lexeme;
}


int getval(void)
{
  get_flag = 0;
  int retval;
  i = 0;

  if(match(INT))
  {
    retval = atoi(getLexeme());
  }

  if(match(ID))
  {
    while(i < totalvar && !get_flag)
    {
      if(strcmp(getLexeme(), arr[i].alphabet) == 0)
      {
        get_flag = 1;
        retval = arr[i].value;
        break;
      }

      else
        i++;
    }

    if(!get_flag)
    {
      if(totalvar < SIZE)
      {
        strcpy(arr[totalvar].alphabet, getLexeme());
        arr[totalvar].value = 0;
        totalvar++;
      }

      else
        error();
    }
  }

  if(min)
  {
    retval *= -1;
    min = 0;
  }
  return retval;
}

int setval(char *str, int val)
{
  int retval = 0;
  i = 0;

  while(i < totalvar)
    {
      if(strcmp(arr[i].alphabet, str) == 0)
      {
        // arr[i].value = val;
        retval = val;
        break;
      }

      else
        i++;
    }
  return retval;
}

BTNode* makeNode(TokenSet tok, const char *lexe)
{
    BTNode *node = (BTNode*) malloc(sizeof(BTNode));
    strcpy(node->lexeme, lexe);
    node->token = tok;
    node->val = 0;
    node->left = NULL;
    node->right = NULL;
    return node;
}

BTNode* factor(void)
{
    BTNode* retp = NULL;
    char tmpstr[MAXLEN];

    if (match(INT))
    {
        retp = makeNode(INT, getLexeme());
        retp->val = getval();
        advance();
    }

    else if (match(ID))
    {
        BTNode* left = makeNode(ID, getLexeme());
        left->val = getval();
        strcpy(tmpstr, getLexeme());
        advance();

        if (match(ASSIGN))
        {
          count += 1;
          retp = makeNode(ASSIGN, getLexeme());
          advance();
          retp->right = expr();
          retp->left = left;
        }

        else
        {
            retp = left;
        }
    }

    else if (match(ADDSUB))
    {
      if(strcmp(getLexeme(), "-") == 0)
        min = 1;

    	strcpy(tmpstr, getLexeme());
      advance();

        if(match(INT))
        {
          strcpy(tmpstr, getLexeme());
          retp = makeNode(INT, tmpstr);
          retp->val = getval();
          advance();
        }

        else if(match(ID))
        {
          retp = makeNode(ADDSUB, tmpstr);

          retp->right = makeNode(ID, getLexeme());
          retp->right->val = getval();

          retp->left = makeNode(INT, "0");
          retp->left->val = 0;

          advance();
        }

        else if(match(LPAREN))
        {
          min = 0;
          retp = makeNode(ADDSUB, tmpstr);
          advance();
          retp->right = expr();
          retp->left = makeNode(INT, "0");
          retp->left->val = 0;

          if (match(RPAREN))
          {
            advance();
          }

          else
          {
            error();
          }
        }

        else
        {
            error();
        }
    }

    else if(match(ORANDXOR))
    {
      strcpy(tmpstr, getLexeme());
      advance();

      if(match(ID))
      {
        retp = makeNode(ORANDXOR, tmpstr);

        retp->right = makeNode(ID, getLexeme());
        retp->right->val = getval();

        retp->left = makeNode(INT, "0");
        retp->left->val = 0;

        advance();
      }

      if(match(INT))
      {
        retp = makeNode(ORANDXOR, tmpstr);

        retp->right = makeNode(INT, getLexeme());
        retp->right->val = getval();

        retp->left = makeNode(INT, "0");
        retp->left->val = 0;

        advance();
      }

      else
        error();
    }

    else if (match(LPAREN))
    {
        advance();
        retp = expr();

        if (match(RPAREN))
        {
            advance();
        }

        else
        {
            error();
        }
    }

    else
    {
        error();
    }

    if(count > 1)
      error();

    return retp;
}

TokenSet getToken(void)
{
    int i;
    char c;

    while ( (c = fgetc(stdin)) == ' ' || c== '\t' );

    if (isdigit(c))
    {
        lexeme[0] = c;
        c = fgetc(stdin);
        i = 1;

        while (isdigit(c) && i < MAXLEN)
        {
            lexeme[i] = c;
            ++i;
            c = fgetc(stdin);
        }

        ungetc(c, stdin);
        lexeme[i] = '\0';
        return INT;
    }

    else if (c == '+' || c == '-')
    {
        lexeme[0] = c;
        lexeme[1] = '\0';
        return ADDSUB;
    }

    else if (c == '|' || c == '&' || c =='^')
    {
        lexeme[0] = c;
        lexeme[1] = '\0';
        return ORANDXOR;
    }

    else if (c == '*' || c == '/')
    {
        if(c == '/')
          div_flag++;

        lexeme[0] = c;
        lexeme[1] = '\0';
        return MULDIV;
    }

    else if (c == '\n')
    {
        lexeme[0] = '\0';
        return END;
    }

    else if (c == '=')
    {
        strcpy(lexeme, "=");
        return ASSIGN;
    }

    else if (c == '(')
    {
        strcpy(lexeme, "(");
        return LPAREN;
    }

    else if (c == ')')
    {
        strcpy(lexeme, ")");
        return RPAREN;
    }

    else if (isalpha(c) || c == '_')
    {
        lexeme[0] = c;
        c = fgetc(stdin);
        i = 1;

        while (isalpha(c) || isdigit(c) || c == '_')
        {
            lexeme[i] = c;
            ++i;
            c = fgetc(stdin);
        }
        ungetc(c, stdin);
        lexeme[i] = '\0';
        return ID;
    }

    else if (c == EOF)
      return ENDFILE;

    else
      return UNKNOWN;
}

int evaluateTree(BTNode *root)
{
    int retval = 0, lv, rv, i;

    if (root != NULL)
    {
      switch (root->token)
      {
        case ID:
          retval = root->val;
          i=0;

          while(i < totalvar)
          {
            if(strcmp(root->lexeme, arr[i].alphabet) == 0)
              break;

            else
              i++;
          }

          if(flag == 0)
          {
            arr[i].value = 1;
            break;
          }

          idx = i*4;

          if(idx == 0)
            printf("MOV r%d r%d\n", cnt++, i);

          else if(idx == 4)
            printf("MOV r%d r%d\n", cnt++, i);

          else if(idx == 8)
            printf("MOV r%d r%d\n", cnt++, i);

          else
          printf("MOV r%d [%d]\n", cnt++, idx);

          break;

        case INT:
            retval = root->val;

             if(flag == 0)
               break;

            printf("MOV r%d %d\n", cnt++, retval);

            break;

        case ASSIGN:
        case ADDSUB:
        case MULDIV:
        case ORANDXOR:
           if(flag == 0 && root->lexeme[0] != '=')
             error();

            lv = evaluateTree(root->left);
            flag = 1;
            rv = evaluateTree(root->right);

            if (strcmp(root->lexeme, "+") == 0)
            {
              retval = lv + rv;
              printf("ADD r%d r%d\n", cnt-2, cnt-1);
              cnt--;
            }

            else if (strcmp(root->lexeme, "-") == 0)
            {
              retval = lv - rv;
              printf("SUB r%d r%d\n", cnt-2, cnt-1);
              cnt--;
            }

            else if (strcmp(root->lexeme, "*") == 0)
            {
              retval = lv * rv;
              printf("MUL r%d r%d\n", cnt-2, cnt-1);
              cnt--;
            }

            else if (strcmp(root->lexeme, "/") == 0)
            {
              retval = lv / rv;
              printf("DIV r%d r%d\n", cnt-2, cnt-1);
              cnt--;
            }

            else if (strcmp(root->lexeme, "=") == 0)
            {
              retval = setval(root->left->lexeme, rv);
              i = 0;

              while(i < totalvar)
              {
                if(strcmp(root->left->lexeme, arr[i].alphabet) == 0)
                  break;

                else
                  i++;
              }

              idx = i*4;

              if(idx == 0)
                printf("MOV r%d r%d\n", i, cnt-1);

              else if(idx == 4)
                printf("MOV r%d r%d\n", i, cnt-1);

              else if(idx == 8)
                printf("MOV r%d r%d\n", i, cnt-1);

              else
              printf("MOV [%d] r%d\n", idx, cnt-1);

              cnt--;
              flag = 0;
            }

            else if (strcmp(root->lexeme, "&") == 0)
            {
              printf("AND r%d r%d\n", cnt-2, cnt-1);
              cnt--;
            }

            else if (strcmp(root->lexeme, "|") == 0)
            {
              printf("OR r%d r%d\n", cnt-2, cnt-1);
              cnt--;
            }

            else if (strcmp(root->lexeme, "^") == 0)
            {
              printf("XOR r%d r%d\n", cnt-2, cnt-1);
              cnt--;
            }
            break;

        default:
            retval = 0;
      }
    }
    return retval;
}

BTNode* term(void)
{
    BTNode *node;
    node = factor();
    return term_tail(node);
}

BTNode* term_tail(BTNode *left)
{
    BTNode *node;

    if (match(MULDIV))
    {
        if(strcmp(getLexeme(), "/") == 0)
          division = 1;

        node = makeNode(MULDIV, getLexeme());
        advance();

        node->left = left;
        node->right = factor();

        if(strcmp(node->right->lexeme, "0") == 0 && division)
          error();

        division = 0;

        return term_tail(node);
    }
    else
        return left;
}

BTNode* expr(void)
{
  BTNode *node;
  node = term();
  return expr_tail(node);
}

BTNode* expr_tail(BTNode *left)
{
  BTNode *node;

  if (match(ADDSUB))
  {
    node = makeNode(ADDSUB, getLexeme());
    advance();

    node->left = left;
    node->right = term();

    return expr_tail(node);
  }

  if (match(ORANDXOR))
  {
    node = makeNode(ORANDXOR, getLexeme());
    advance();

    node->left = left;
    node->right = term();

    return expr_tail(node);
  }

  else
    return left;
}

void error()
{
  printf("EXIT 1\n");
  exit(0);
}

void not_var(BTNode *root)
{
  if (root->left != NULL)
    not_var(root->left);

  if (root->right != NULL)
    not_var(root->right);

  if (root->token == ID)
  {
    i = 0;
    while(i < totalvar)
    {
      if (strcmp(root->lexeme, arr[i].alphabet) == 0)
        break;

      else
        i++;
    }
    if (arr[i].value == 0)
      error();
  }
}

int division_error(BTNode *root)
{
  int retval = 0, lv, rv, i;

  if(root != NULL)
  {
    lv = division_error(root->left);
    rv = division_error(root->right);

    if(root->token == INT || root->token == ID)
      retval = root->val;

    if (strcmp(root->lexeme, "+") == 0)
      retval = lv + rv;

    else if (strcmp(root->lexeme, "-") == 0)
      retval = lv - rv;

    else if (strcmp(root->lexeme, "*") == 0)
      retval = lv * rv;

    else if (strcmp(root->lexeme, "/") == 0 && div_flag == 1)
    {
      if(rv == 0)
        error();
    }

    else if(strcmp(root->lexeme, "/") == 0 && div_flag > 1)
    {
      retval = lv/rv;
      div_flag--;
    }
  }
  return retval;
}

void freeTree(BTNode *root)
{
    if (root!=NULL)
    {
        freeTree(root->left);
        freeTree(root->right);
        free(root);
    }
}
