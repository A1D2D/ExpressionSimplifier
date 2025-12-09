#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "Helpers.h"
#include "debugmalloc.h"

#include "Queues.h"

#define BASIC_DEBUG 1

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
   bool hazardFree;
} Expr;

typedef struct {
   unsigned __int64 taggedVariablesFlag;
   unsigned __int64 flippedVariablesFlag;
}  PrimeImplicant;

typedef struct {
   Queue terms;
} NPrimeImplicant;

typedef struct {
   Queue terms;
} Dif;

typedef struct {
   Dif dif;
   NPrimeImplicant nPrime;
   bool used;
} Row;

typedef struct {
   Queue rows;
} Bucket;

typedef struct {
   Queue terms;
   bool covered;
} TermGroup;

MAKE_QUEUE(bucket, Bucket)
MAKE_QUEUE(row, Row)
MAKE_QUEUE(nPrim, NPrimeImplicant)
MAKE_QUEUE(tg, TermGroup)

//*DEBUG*//
void printTable(unsigned int variableCount, const Term* terms) {
   if (!terms) return;
   if (variableCount > 4) return;
   const char v[5] = {'A', 'B', 'C', 'D', 'F'};
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
   if (variableCount > 4) return;
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

void printPrimeImplicant(PrimeImplicant primeImplicant, unsigned int count, ExprTableType type) {
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
   if (type == Maxterm) {
      unsigned long long temp = primeImplicant.flippedVariablesFlag;
      primeImplicant.flippedVariablesFlag = primeImplicant.taggedVariablesFlag;
      primeImplicant.taggedVariablesFlag = temp;
   }
   for (int j = 0; j < count; ++j) {
      if (HasFlag(primeImplicant.taggedVariablesFlag, 1 << j)) {
         printf("%s", primeNames[count - j - 1]);
         if (j < count-1 && !type == Maxterm) printf(" & ");
         if (j < count-1 && type == Maxterm) printf(" | ");
      }
   }
   for (int j = 0; j < count; ++j) {
      if (HasFlag(primeImplicant.flippedVariablesFlag, 1 << j)) {
         printf("(!%s)", primeNames[count - j - 1]);
         if (j < count-1 && !type == Maxterm) printf(" & ");
         if (j < count-1 && type == Maxterm) printf(" | ");
      }
   }
   if (primeImplicant.taggedVariablesFlag == 0) printf("1");
   if (primeImplicant.flippedVariablesFlag == 0) printf("0");
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

void printBuckets(Queue buckets) {
   for (int i = 0; i < buckets.length; i++) {
      printf("Bucket: \n");
      Bucket b;
      queue_bucket_get(&buckets, i, &b);
      for (int j = 0; j < b.rows.length; j++) {
         printf("Row: ");
         Row r;
         queue_row_get(&b.rows, j, &r);
         for (int k = 0; k < r.nPrime.terms.length; ++k) {
            unsigned int term = 0;
            queue_u_get(&r.nPrime.terms, k, &term);
            printf(" %u" , term);
         }
         printf("(");
         for (int k = 0; k < r.dif.terms.length; ++k) {
            unsigned int term = 0;
            queue_u_get(&r.dif.terms, k, &term);
            printf(" %u" , term);
         }
         printf(")");
         printf("\n");
      }
      printf("\n");
   }
}

void printPrimesNumbers(NPrimeImplicant prime) {
   printf("Prime: ");
   for (int i = 0; i < prime.terms.length; ++i) {
      unsigned int term = 0;
      queue_u_get(&prime.terms, i, &term);
      printf("%u " , term);
   }
   printf("\n");
}
//*~DEBUG*//


//Expression Exp a b c d e f g h i j k, || && ^
//MinTerm    Min[var count](minterms)(nonmandatory)
//MaxTerm    Max[var count](maxterms)(nonmandatory)
//*PARSE INPUT*//
int countInts(const char* str) {
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

bool isTwoPower(const unsigned int x) {
   return (bool)(x != 0 && (x & (x - 1)) == 0);
}

unsigned int* parse_int_list(const char* str, unsigned int* count) {
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
   if (out->var_count > 15) return 0;
   const char* p = p2 + 1;
   if (*p == 'H') {
      out->hazardFree = true;
   } else {
      out->hazardFree = false;
   }

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
   if (!o1) {
      out->opt_terms = NULL;
      out->opt_terms_count = 0;
      return 1;
   }
   const char* o2 = strchr(o1 + 1, ')');
   if (!o2) {
      return 0;
      free(out->terms);
      free(out->opt_terms);
   }

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

bool contains(Queue a, Queue b) {
   bool returnVal = true;
   Queue alreadyUsed;
   queue_init(&alreadyUsed);
   for (int i = 0; i < b.length; ++i) {
      unsigned int vx;
      queue_u_get(&b, i, &vx);

      bool found = false;
      for (int j = 0; j < a.length; ++j) {
         bool continueOuter = false;
         for (int k = 0; k < alreadyUsed.length; ++k) {
            unsigned int au;
            queue_u_get(&alreadyUsed, k, &au);
            if (au == j) continueOuter = true;
         }
         if (continueOuter) continue;
         unsigned int vy;
         queue_u_get(&a, j, &vy);
         if (vx == vy) {
            queue_u_push(&alreadyUsed, j);
            found = true;
            break;
         }
      }
      if (!found) {
         returnVal = false;
         break;
      }
   }
   queue_free(&alreadyUsed);
   return returnVal;
}

bool isSimilar(Queue x, Queue y) {
   if (x.length != y.length) return false;
   return contains(x, y);
}

PrimeImplicant convertToNormal(NPrimeImplicant prime) {
   PrimeImplicant out;
   out.flippedVariablesFlag = 0;
   out.taggedVariablesFlag = 0;

   if (prime.terms.length == 0) return out;
   unsigned int diff = 0;
   unsigned int first = 0;
   queue_u_get(&prime.terms, 0, &first);
   for (int i = 1; i < prime.terms.length; ++i) {
      unsigned int item = 0;
      queue_u_get(&prime.terms, i, &item);
      diff |= first ^ item;
   }
   diff = ~diff;

   out.taggedVariablesFlag = diff & first;
   out.flippedVariablesFlag = diff & (~first);
   return out;
}
//*~UTILS*///

//*IMPL*//
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

void flipTerms(Term* terms, const unsigned int count) {
   for (int i = 0; i < 1 << count; i++) {
      switch (terms[i]) {
         case Zero:
            terms[i] = One;
            break;
         case One:
            terms[i] = Zero;
            break;
         default:
            terms[i] = Opt;
      }
   }
}

void fillBuckets(Queue* buckets, const Term* terms, unsigned int count) {
   if (!terms) return;
   int termsUsed = 0;
   for (int i = 0; termsUsed <= (1 << count)-1; i++) {
      Bucket bucket;
      queue_init(&bucket.rows);

      for (int j = 0; j < (1 << count); j++) {
         if (countOnes(j) != i) continue;
         termsUsed++;
         if (terms[j] == Zero) continue;
         Row row;
         row.used = false;
         queue_init(&row.dif.terms);
         queue_init(&row.nPrime.terms);
         queue_u_push(&row.nPrime.terms, j);
         queue_row_push(&bucket.rows, row);
      }

      queue_bucket_push(buckets, bucket);
   }
}

void collectUnUsed(const Queue input, Queue* primes) {
   for (int i = 0; i < input.length; i++) {
      Bucket x;
      queue_bucket_get(&input, i, &x);

      for (int j = 0; j < x.rows.length; j++) {
         Row row;
         queue_row_get(&x.rows, j, &row);
         if (!row.used) {
            NPrimeImplicant prime;
            queue_init(&prime.terms);
            for (int k = 0; k < row.nPrime.terms.length; ++k) {
               unsigned int term = 0;
               queue_u_get(&row.nPrime.terms, k, &term);
               queue_u_push(&prime.terms, term);
            }
            queue_nPrim_push(primes, prime);
         }
      }
   }
}

void mergePrimes(Queue input, Queue* output, const Term* terms, unsigned int count, Queue* primes) {
   for (int i = 0; i < input.length-1; i++) {
      Bucket x;
      Bucket y;
      queue_bucket_get(&input, i, &x);
      queue_bucket_get(&input, i+1, &y);

      Bucket out;
      queue_init(&out.rows);

      for (int j = 0; j < x.rows.length; j++) {
         Row* rowX = queue_row_get_ptr(&x.rows, j);

         for (int k = 0; k < y.rows.length; k++) {
            Row* rowY = queue_row_get_ptr(&y.rows, k);
            if (rowX->dif.terms.length > 0) {
               if (!isSimilar(rowX->dif.terms, rowY->dif.terms)) continue;
            }

            unsigned int tX = 0;
            unsigned int tY = 0;
            queue_u_get(&rowX->nPrime.terms, 0, &tX);
            queue_u_get(&rowY->nPrime.terms, 0, &tY);

            if (tX >= tY) continue;
            unsigned int dif = tY-tX;
            if (!isTwoPower(dif)) continue;

            Row rowO;
            rowO.used = false;
            queue_init(&rowO.dif.terms);
            queue_init(&rowO.nPrime.terms);

            rowX->used = true;
            rowY->used = true;

            for (int l = 0; l < rowX->nPrime.terms.length; ++l) {
               unsigned int cTerm;
               queue_u_get(&rowX->nPrime.terms, l, &cTerm);
               queue_u_push(&rowO.nPrime.terms, cTerm);
            }
            for (int l = 0; l < rowY->nPrime.terms.length; ++l) {
               unsigned int cTerm;
               queue_u_get(&rowY->nPrime.terms, l, &cTerm);
               queue_u_push(&rowO.nPrime.terms, cTerm);
            }
            for (int l = 0; l < rowX->dif.terms.length; ++l) {
               unsigned int cTerm;
               queue_u_get(&rowX->dif.terms, l, &cTerm);
               queue_u_push(&rowO.dif.terms, cTerm);
            }
            queue_u_push(&rowO.dif.terms, dif);

            bool found = false;
            for (int l = 0; l < out.rows.length; ++l) {
               Row check;
               queue_row_get(&out.rows, l, &check);
               if (isSimilar(check.nPrime.terms, rowO.nPrime.terms) && isSimilar(check.dif.terms, rowO.dif.terms)) {
                  found = true;
                  break;
               }
            }
            if (!found) {
               queue_row_push(&out.rows, rowO);
            } else {
               queue_free(&rowO.nPrime.terms);
               queue_free(&rowO.dif.terms);
            }
         }
      }
      queue_bucket_push(output, out);
   }

   collectUnUsed(input, primes);
}

bool isEmpty(Queue input) {
   bool empty = true;
   for (int i = 0; i < input.length; i++) {
      Bucket b;
      queue_bucket_get(&input, i, &b);
      if (b.rows.length > 0) empty = false;
   }
   return empty;
}

void freeBuckets(Queue* buckets) {
   for (int i = 0; i < buckets->length; i++) {
      Bucket b;
      queue_bucket_get(buckets, i, &b);
      for (int j = 0; j < b.rows.length; j++) {
         Row r;
         queue_row_get(&b.rows, j, &r);
         queue_free(&r.nPrime.terms);
         queue_free(&r.dif.terms);
      }
      queue_free(&b.rows);
   }
   queue_free(buckets);
}

void freePrimeNumbers(Queue* primes) {
   for (int i = 0; i < primes->length; i++) {
      NPrimeImplicant prime;
      queue_nPrim_get(primes, i, &prime);
      queue_free(&prime.terms);
   }
   queue_free(primes);
}

void freeExpr(Expr* expr) {
   free(expr->terms);
   free(expr->opt_terms);
}

void quineMcCluskey(const Term* terms, unsigned int count, Queue* primes) {
   Queue prev;
   queue_init(&prev);
   fillBuckets(&prev, terms, count);

   while (!isEmpty(prev)) {
      Queue next;
      queue_init(&next);
      mergePrimes(prev, &next, terms, count, primes);
      freeBuckets(&prev);
      prev = next;
   }
   freeBuckets(&prev);
}

void selectRequired(const Term* terms, unsigned int count, Queue* primes, bool hazardFree) {
   if (!terms || !primes) return;
   Queue usedPIDs;
   queue_init(&usedPIDs);
   Queue table;
   queue_init(&table);


   //fill selection table
   if (hazardFree) {
      //iterate through all terms
      for (int i = 0; i < 1<<count; i++) {
         if (terms[i] != One) continue;
         bool found = false;
         for (int j = 0; j < count; j++) {
            //find all its neighbor terms
            unsigned int neighbour = (1 << j) ^ i;
            if (terms[neighbour] != One) continue;
            found = true;
            bool skip = false;
            Queue check;
            queue_init(&check);
            queue_u_push(&check, neighbour);
            queue_u_push(&check, i);
            //check if its already added to the list
            for (int k = 0; k < table.length; k++) {
               TermGroup tg;
               queue_tg_get(&table, k, &tg);
               if (contains(tg.terms, check)) {
                  skip = true;
                  break;
               }
            }
            queue_free(&check);
            if (skip) continue;
            //add the pair neighbor pair to the table
            TermGroup tg;
            tg.covered = false;
            queue_init(&tg.terms);
            queue_u_push(&tg.terms, i);
            queue_u_push(&tg.terms, neighbour);
            queue_tg_push(&table, tg);
         }
         if (!found) {
            //if nothing got added, add the term only
            TermGroup tg;
            tg.covered = false;
            queue_init(&tg.terms);
            queue_u_push(&tg.terms, i);
            queue_tg_push(&table, tg);
         }
      }
   } else {
      //push all One terms to the list
      for (int i = 0; i < 1<<count; i++) {
         if (terms[i] != One) continue;
         TermGroup tg;
         tg.covered = false;
         queue_init(&tg.terms);
         queue_u_push(&tg.terms, i);
         queue_tg_push(&table, tg);
      }
   }

   //Mandatory pass
   for (int i = 0; i < table.length; i++) {
      TermGroup tg;
      queue_tg_get(&table, i, &tg);
      if (tg.covered) continue;
      unsigned int firstPrimeIndex = 0;

      int found = 0;
      for (int j = 0; j < primes->length; j++) {
         NPrimeImplicant prime;
         queue_nPrim_get(primes, j, &prime);
         if (contains(prime.terms, tg.terms)) {
            firstPrimeIndex = j;
            found++;
         }
         if (found > 1) break;
      }
      if (found > 1) continue;

      queue_u_push(&usedPIDs, firstPrimeIndex);
      NPrimeImplicant prime;
      queue_nPrim_get(primes, firstPrimeIndex, &prime);

      for (int j = 0; j < table.length; j++) {
         TermGroup* checkPtr = queue_tg_get_ptr(&table, j);
         if (contains(prime.terms, checkPtr->terms)) checkPtr->covered = true;
      }
   }

   //Cover Pass
   while (usedPIDs.length < primes->length) {
      unsigned int bestPrimeIndex = 0;
      unsigned int bestValue = 0;
      for (int i = 0; i < primes->length; ++i) {
         bool used = false;
         for (int j = 0; j < usedPIDs.length; ++j) {
            unsigned int pID;
            queue_u_get(&usedPIDs, j, &pID);
            if (pID == i) {
               used = true;
               break;
            }
         }
         if (used) continue;
         NPrimeImplicant prime;
         queue_nPrim_get(primes, i, &prime);

         int value = 0;
         for (int j = 0; j < table.length; j++) {
            TermGroup check;
            queue_tg_get(&table, j, &check);
            if (check.covered) continue;
            if (contains(prime.terms, check.terms)) value++;
         }
         if (value > bestValue) {
            bestValue = value;
            bestPrimeIndex = i;
         }
      }
      if (bestValue == 0) break;
      queue_u_push(&usedPIDs, bestPrimeIndex);

      NPrimeImplicant prime;
      queue_nPrim_get(primes, bestPrimeIndex, &prime);

      for (int j = 0; j < table.length; j++) {
         TermGroup* checkPtr = queue_tg_get_ptr(&table, j);
         if (contains(prime.terms, checkPtr->terms)) checkPtr->covered = true;
      }
   }

   //DBG
   for (int i = 0; i < table.length; ++i) {
      TermGroup tg;
      queue_tg_get(&table, i, &tg);
      printf("Group: %d %s: ", i, tg.covered ? "covered" : "not covered");
      for (int j = 0; j < tg.terms.length; ++j) {
         unsigned int term;
         queue_u_get(&tg.terms, j, &term);
         printf("%d ", term);
      }
      printf("\n");
   }

   printf("\n");
   for (int i = 0; i < usedPIDs.length; ++i) {
      unsigned int use;
      queue_u_get(&usedPIDs, i, &use);
      printf("%d ", use);
   }
   printf("\n");

   Queue primesClone;
   queue_init(&primesClone);
   for (int i = 0; i < primes->length; ++i) {
      bool used = false;
      for (int j = 0; j < usedPIDs.length; ++j) {
         unsigned int pID;
         queue_u_get(&usedPIDs, j, &pID);
         if (pID == i) {
            used = true;
            break;
         }
      }
      if (!used) continue;
      NPrimeImplicant prime;
      queue_nPrim_get(primes, i, &prime);
      NPrimeImplicant primeClone;
      queue_init(&primeClone.terms);
      for (int j = 0; j < prime.terms.length; j++) {
         unsigned int term;
         queue_u_get(&prime.terms, j, &term);
         queue_u_push(&primeClone.terms, term);
      }
      queue_nPrim_push(&primesClone, primeClone);
   }
   freePrimeNumbers(primes);
   *primes = primesClone;

   //free containers
   for (int i = 0; i < table.length; ++i) {
      TermGroup tg;
      queue_tg_get(&table, i, &tg);
      queue_free(&tg.terms);
   }
   queue_free(&table);
   queue_free(&usedPIDs);
}
//*~IMPL*//

int main(void) {
   const char* input = "Min[4]H(0,1,2,3)()";
   Expr expr;

   if (!parse_expr(input, &expr)) return 1;
   Term* terms = parse_term(expr);
   freeExpr(&expr);

   printEmptyTable(expr.var_count);
   printf("\n");
   printTable(expr.var_count, terms);

   if (expr.type == Maxterm) flipTerms(terms, expr.var_count);

   printf("\n");
   Queue primes;
   queue_init(&primes);

   quineMcCluskey(terms, expr.var_count, &primes);

   for (int i = 0; i < primes.length; ++i) {
      NPrimeImplicant prime;
      queue_nPrim_get(&primes, i, &prime);
      printPrimesNumbers(prime);
      PrimeImplicant p = convertToNormal(prime);
      printPrimeImplicant(p, expr.var_count, expr.type);
      printf("\n");
   }
   printf("\n");

   selectRequired(terms, expr.var_count, &primes, expr.hazardFree);
   for (int i = 0; i < primes.length; ++i) {
      NPrimeImplicant prime;
      queue_nPrim_get(&primes, i, &prime);
      printPrimesNumbers(prime);
      PrimeImplicant p = convertToNormal(prime);
      printPrimeImplicant(p, expr.var_count, expr.type);
      printf("\n");
   }
   printf("\n");

   freePrimeNumbers(&primes);
   free(terms);
   return 0;
}
