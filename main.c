#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define FlagDef(ID) (1LL << ((ID)-1))
#define HasFlag(flags, flag) (((flags) & (flag)) != 0)
#define HasNoFlag(flags, flag) (((flags) & (flag)) == 0)
#define AddFlag(flags, flag) ((flags) |= (flag))
#define RemoveFlag(flags, flag) ((flags) &= ~(flag))
#define ToggleFlag(flags, flag) ((flags) ^= (flag))
#define SetFlag(flags, flag, condition) ((condition) ? AddFlag(flags, flag) : RemoveFlag(flags, flag))

#define true 1
#define false 0

typedef char bool;

typedef enum {
   Unset = 0,
   One = '1',
   Zero = '0',
   Opt = '~'
} Term;

typedef enum {
   Minterm,
   Maxterm,
   Expression,
   Unknown
} ExprTableType;

typedef struct {
   ExprTableType type;
   unsigned int var_count;
   unsigned int* terms;
   unsigned int terms_count;
   unsigned int* opt_terms;
   unsigned int opt_terms_count;
} Expr;

typedef struct {
   unsigned __int64 taggedVariablesFlag;
   unsigned __int64 flippedVariablesFlag;
}  PrimeImplicant;

//*DEBUG*//
void printTable(unsigned int variableCount, const Term* terms) {
   if (!terms) return;
   const char v[5] = {'A', 'B', 'C', 'D', 'F'};
   // char t[32] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'V', 'W'};
   char t[32] = {0};
   for (int i = 0; i < 1<<variableCount; i++) {
      switch (terms[i]) {
         case One:
            t[i] = '1';
            break;
         case Zero:
            t[i] = '0';
            break;
         case Opt:
            t[i] = '~';
            break;
         case Unset:
         default:
            t[i] = ' ';
      }
   }

   const char* cTable2x2 =
   "      %c\n"
   "     --\n"
   "   %c  %c\n"
   "%c| %c  %c\n";

   const char* cTable4x2 =
   "      %c\n"
   "     --\n"
   "   %c  %c\n"
   "   %c  %c |\n"
   " | %c  %c |%c\n"
   "%c| %c  %c\n";

   const char* cTable4x4 =
   "           %c\n"
   "         ----\n"
   "   %c  %c  %c  %c\n"
   "   %c  %c  %c  %c |\n"
   " | %c  %c  %c  %c |%c\n"
   "%c| %c  %c  %c  %c\n"
   "      ----    \n"
   "      %c\n";

   switch (variableCount) {
      case 2:
         printf(cTable2x2, v[1], t[0], t[1], v[0], t[2], t[3]);
         break;
      case 3:
         printf(cTable4x2, v[2], t[0], t[1], t[2], t[3], t[6], t[7], v[1], v[0], t[4], t[5]);
         break;
      case 4:
         printf(cTable4x4,v[2], t[0], t[1], t[3], t[2], t[4], t[5], t[7], t[6], t[12], t[13], t[15], t[14], v[1], v[0], t[8], t[9], t[11], t[10], v[3]);
         break;
      case 5:
         // break;
      default:
         break;
   }
}

void printEmptyTable(unsigned int variableCount) {
   const char v[5] = {'A', 'B', 'C', 'D', 'F'};
   char t[32] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'V', 'W'};

   const char* cTable2x2 =
   "      %c\n"
   "     --\n"
   "   %c  %c\n"
   "%c| %c  %c\n";

   const char* cTable4x2 =
   "      %c\n"
   "     --\n"
   "   %c  %c\n"
   "   %c  %c |\n"
   " | %c  %c |%c\n"
   "%c| %c  %c\n";

   const char* cTable4x4 =
   "           %c\n"
   "         ----\n"
   "   %c  %c  %c  %c\n"
   "   %c  %c  %c  %c |\n"
   " | %c  %c  %c  %c |%c\n"
   "%c| %c  %c  %c  %c\n"
   "      ----    \n"
   "      %c\n";

   switch (variableCount) {
      case 2:
         printf(cTable2x2, v[1], t[0], t[1], v[0], t[2], t[3]);
         break;
      case 3:
         printf(cTable4x2, v[2], t[0], t[1], t[2], t[3], t[6], t[7], v[1], v[0], t[4], t[5]);
         break;
      case 4:
         printf(cTable4x4,v[2], t[0], t[1], t[3], t[2], t[4], t[5], t[7], t[6], t[12], t[13], t[15], t[14], v[1], v[0], t[8], t[9], t[11], t[10], v[3]);
         break;
      case 5:
         // break;
      default:
         break;
   }
}

void printPrimeImplicant(PrimeImplicant primeImplicant,unsigned int count) {
   const char* primeNames[64] = {
      "a","b","c","d","e","f","g",
      "h","i","j","k","l","m","n",
      "o","p","q","r","s","t","u",
      "v","w","x","y","z",
      "A","B","C","D","E","F","G",
      "H","I","J","K","L","M","N",
      "O","P","Q","R","S","T","U",
      "V","W","X","Y","Z",
      "A1","B1","C1","D1","E1","F1","G1","H1","I1","J1","K1","L1"
   };
   printf("id?: %llu| ", primeImplicant.taggedVariablesFlag);
   for (int j = 0; j < count; ++j) {
      if (HasFlag(primeImplicant.taggedVariablesFlag, 1 << j)) {
         printf("%s", primeNames[count - j - 1]);
         if (j < count-1) printf("*");
      }
   }
   for (int j = 0; j < count; ++j) {
      if (HasFlag(primeImplicant.flippedVariablesFlag, 1 << j)) {
         printf("(-%s)", primeNames[count - j - 1]);
         if (j < count-1) printf("*");
      }
   }
   printf("\n");
}

void printFullBinary(unsigned long long x, int bytes) {
   int bits = bytes * 8;
   for (int i = 0; i < bits; ++i) {
      unsigned bit = (x >> i) & 1ULL;
      putchar(bit ? '1' : '0');
   }
}

void printBinaryBits(unsigned long long x, int bits) {
   for (int i = 0; i < bits; ++i) {
      unsigned bit = (x >> i) & 1ULL;
      putchar(bit ? '1' : '0');
   }
}
//*~DEBUG*//


//Expression Exp a b c d e f g h i j k, || && ^
//MinTerm    Min[var count](minterms)(nonmandatory)
//MaxTerm    Max[var count](maxterms)(nonmandatory)
//*PARSE INPUT*//
static int countInts(const char* str) {
   int count = 0;
   while (*str) {
      while (*str && !isdigit(*str) && *str != '-') str++;
      if (!*str) break;
      count++;
      if (*str == '-') str++;
      while (isdigit(*str)) str++;
   }
   return count;
}

static unsigned int* parse_int_list(const char* str, unsigned int* count) {
   *count = countInts(str);
   unsigned int* out = malloc(sizeof(int) * *count);
   int i = 0;
   while (*str) {
      while (*str && !isdigit(*str)) str++;
      if (!*str) break;
      out[i++] = atoi(str);
      while (*str && isdigit(*str)) str++;
   }
   if (*count != i) {
      free(out);
      return NULL;
   }
   return out;
}

ExprTableType parseExprTableType(const char* s) {
   if (strlen(s) < 3) return Unknown;
   if (s[0] == 'M' && s[1] == 'i' && s[2] == 'n') {
      return Minterm;
   }
   if (s[0] == 'M' && s[1] == 'a' && s[2] == 'x') {
      return Maxterm;
   }
   if (s[0] == 'E' && s[1] == 'x' && s[2] == 'r') {
      return Expression;
   }
   return Unknown;
}

int parse_expr(const char* s, Expr* out) {
   if (strlen(s) < 10) return 0;

   out->type = parseExprTableType(s);
   if (out->type == Unknown) return 0;

   const char* p1 = strchr(s, '[');
   const char* p2 = strchr(s, ']');
   if (!p1 || !p2) return 0;
   out->var_count = atoi(p1 + 1);

   const char* m1 = strchr(p2, '(');
   if (!m1) return 0;
   const char* m2 = strchr(m1 + 1, ')');
   if (!m2) return 0;

   long long len = m2 - (m1 + 1);
   char* temp = malloc(len+1);
   strncpy(temp, m1 + 1, len);
   temp[len] = 0;
   out->terms = parse_int_list(temp, &out->terms_count);
   free(temp);

   const char* o1 = strchr(m2 + 1, '(');
   if (!o1) return 0;
   const char* o2 = strchr(o1 + 1, ')');
   if (!o2) return 0;

   out->opt_terms_count = 0;
   len = o2 - (o1 + 1);
   temp = malloc(len+1);
   strncpy(temp, o1 + 1, len);
   temp[len] = 0;
   out->opt_terms = parse_int_list(temp, &out->opt_terms_count);
   free(temp);

   return 1;
}
//*~PARSE INPUT*//

//*UTILS*//
unsigned int countOnes(unsigned int x) {
   unsigned int count = 0;
   while (x) {
      x &= x - 1;  // Clears the rightmost set bit
      count++;
   }
   return count;
}

unsigned int fillOnes(unsigned int mask, int id) {
   unsigned int out = 0;

   for (int bit = 0; bit < 32 && mask; bit++) {
      unsigned int lowest = mask & -mask;

      if (id & (1u << bit)) out |= lowest;

      mask &= mask - 1;
   }

   return out;
}
//*~UTILS*///

Term* parse_term(Expr expr) {
   unsigned int cc = 1 << expr.var_count;
   Term* out = malloc(sizeof(Term) * cc);
   if (expr.type == Minterm) {
      for (int i = 0; i < cc; i++) out[i] = Zero;
      for (int i = 0; i < expr.terms_count; i++) {
         out[expr.terms[i]] = One;
      }
      for (int i = 0; i < expr.opt_terms_count; i++) {
         out[expr.opt_terms[i]] = Opt;
      }
   } else if (expr.type == Maxterm) {
      for (int i = 0; i < cc; i++) out[i] = One;
      for (int i = 0; i < expr.terms_count; i++) {
         out[cc-expr.terms[i]-1] = Zero;
      }
      for (int i = 0; i < expr.opt_terms_count; i++) {
         out[cc-expr.opt_terms[i]-1] = Opt;
      }
   }



   return out;
}

PrimeImplicant* parse_prime(const Term* terms, unsigned int count, unsigned int* implicantCount) {
   if (!terms) return NULL;
   *implicantCount = 0;
   for (unsigned int i = 0; i < (1 << count); i++) {
      if (terms[i] == One) (*implicantCount)++;
   }

   PrimeImplicant* out = malloc(sizeof(PrimeImplicant) * (*implicantCount));
   unsigned int j = 0;
   for (unsigned int i = 0; i < (1 << count); i++) {
      int term = terms[i];
      if (term == One) {
         out[j].taggedVariablesFlag = i;
         out[j].flippedVariablesFlag = ~i;
         j++;
      }
   }
   return out;
}

PrimeImplicant growImplicant(PrimeImplicant implicant, const Term* terms, unsigned int count) {
   //számjegyes minimalizálás
   if (!terms) return implicant;
   for (int i = 0; i < count; i++) {
      unsigned int removeVar = 1 << i;

      if (HasFlag(implicant.taggedVariablesFlag, removeVar) && HasFlag(implicant.flippedVariablesFlag, removeVar)) continue;
      unsigned int holes = ~implicant.taggedVariablesFlag & ~implicant.flippedVariablesFlag & (1 << count)-1;
      unsigned int emptyCount = countOnes(holes);

      if (HasFlag(implicant.taggedVariablesFlag, removeVar)) {
         unsigned int body = (implicant.taggedVariablesFlag | ~implicant.flippedVariablesFlag) & ~removeVar & ((1 << count)-1) & ~holes;

         bool allowGrow = true;
         for (int j = 0; j < 1 << emptyCount; ++j) {
            unsigned int comb = fillOnes(holes, j);
            unsigned int termID = body | comb;

            if (terms[termID] == Zero) {
               allowGrow = false;
               break;
            }
         }

         if (allowGrow) {
            RemoveFlag(implicant.taggedVariablesFlag, removeVar);
         }
      } else if (HasFlag(implicant.flippedVariablesFlag, removeVar)) {
         unsigned int body = (((implicant.taggedVariablesFlag) | ~implicant.flippedVariablesFlag) | removeVar) & ((1 << count)-1) & ~holes;

         bool allowGrow = true;
         for (int j = 0; j < 1 << emptyCount; ++j) {
            unsigned int comb = fillOnes(holes, j);
            unsigned int termID = body | comb;

            if (terms[termID] == Zero) {
               allowGrow = false;
               break;
            }
         }

         if (allowGrow) {
            RemoveFlag(implicant.flippedVariablesFlag, removeVar);
         }
      }
   }

   return implicant;
}

void growAllImplicants(PrimeImplicant* implicants, unsigned int implicantCount, const Term* terms, unsigned int count) {
   if (!implicants) return;
   for (int i = 0; i < implicantCount; i++) {
      implicants[i] = growImplicant(implicants[i], terms, count);
   }
}

PrimeImplicant* removeDuplicates(const PrimeImplicant* implicants, unsigned int count, unsigned int* out_count) {
   if (count <= 0) {
      *out_count = 0;
      return NULL;
   }

   PrimeImplicant* out = malloc(count * sizeof(PrimeImplicant));
   if (!out || !implicants) {
      *out_count = 0;
      return NULL;
   }

   int unique = 0;

   for (int i = 0; i < count; i++) {
      int dup = 0;

      for (int j = 0; j < unique; j++) {
         if (implicants[i].taggedVariablesFlag == out[j].taggedVariablesFlag && implicants[i].flippedVariablesFlag == out[j].flippedVariablesFlag) {
            dup = 1;
            break;
         }
      }

      if (!dup) {
         out[unique++] = implicants[i];
      }
   }

   out = realloc(out, unique * sizeof(PrimeImplicant));

   *out_count = unique;

   return out;
}




int main(void) {
   const char* input = "Min[4](0,1,3,2,4,5,7)(13)";
   Expr expr;

   if (parse_expr(input, &expr)) {
   }
   Term* terms = parse_term(expr);
   unsigned int implicantCount = 0;
   PrimeImplicant* primeImplicants = parse_prime(terms, expr.var_count, &implicantCount);

   printEmptyTable(expr.var_count);
   printf("\n");
   printTable(expr.var_count, terms);

   growAllImplicants(primeImplicants, implicantCount, terms, expr.var_count);
   free(terms);


   PrimeImplicant* cutImplicants = removeDuplicates(primeImplicants, implicantCount, &implicantCount);
   free(primeImplicants);

   for (int i = 0; i < implicantCount; i++) {
      printPrimeImplicant(cutImplicants[i], expr.var_count);
   }

   free(cutImplicants);

   return 0;
}
