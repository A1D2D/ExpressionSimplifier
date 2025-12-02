//*OLD_IMPL*//
/*
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
*/
//*~OLD_IMPL*//