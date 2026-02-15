/**********************************************************************
 * algorithm that implements FM-index search                          *
 * fmSearch.c                                                         *
 * Aleksandr Means                                                    *
 **********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

typedef struct occTable_S {
  char alph [127];
  int map [127];
  int alphn;
  int n;
  int data[];
} occTable;

occTable * makeOccTable(char *, size_t);
int getOcc(occTable *, char, int);

char * sAlph(char * s, size_t n);

int * suffixArray(char *, size_t);
char * BWtable(char *, int *, size_t);
int * Ctable(char *, size_t, int *);
void range(char *, char *, size_t, size_t);

int main(int argc, char ** argv) {
  bool findrange = false;
  char * q = NULL;
  if (argc < 2 || argc >= 4) {
    fprintf(stdout, "malformed arguments\n");
    return 1;
  } else if (argc == 3) {
    findrange = true;
    q = argv[2];
  }

  char * s = NULL;   // the S string, and the pointer to the entire char buffer
  FILE * pFile = fopen(argv[1], "r"); // pointer to the given file
  if (pFile == NULL) {
    fprintf(stderr, "error reading file at %s\n", argv[1]);
    return 1;
  }

  fseek(pFile, 0L, SEEK_END);
  size_t fsize = ftell(pFile); // the size of the file
  fseek(pFile, 0L, SEEK_SET);

  // We will now read the file into the text string
  s = malloc(fsize);
  char * next = s; // pointer to the next position in the string to read the file line into

  while (!feof(pFile)) {
    // the upper bound to read characters into the text string
    size_t n = fsize - (next - s);

    // read the next file line into the text string at 'next'
    getline(&next, &n, pFile);

    // find the next instance of \n, \r, >, or ';', and set that to next
    // This will have the effect of overwriting the string starting at any
    // of these characters. This will erase comments, and will remove the
    // carriage returns or newline characters.
    next = strpbrk(next, "\n\r>;");
  }

  // sets the final newline/carriage return to 0 if it exists.
  if (next) {
    next[0] = '$';
    next[1] = 0;
  }
  else {
    fprintf(stdout, "malformed file at %s\n", argv[1]);
    free(s);
    fclose(pFile);
    return 1;
  }
  size_t n = strlen(s);
  fclose(pFile);
#ifdef DEBUG
  fprintf(stdout, "%llu\n", n);
  fprintf(stdout, "%s\n", s);
#endif
  int * SA = suffixArray(s, n);
  char * BW = BWtable(s, SA, n);
  int * C = Ctable(s, n, SA);
  char * alph = sAlph(s, n);
  occTable * table = makeOccTable(BW, n);
  
  if (BW) fprintf(stdout, "BW = %s\n\n", BW);
  if (C) {
    int alphn = strlen(alph);
    for (int i = 1; i < alphn; i++) {
      fprintf(stdout, "C[%c] = %d\n", alph[i], C[alph[i]]);
    }
  }
  fprintf(stdout, "\n");
  if (table) {
    int width = log10(n) + 1;
    fprintf(stdout, "OCC:\n");
    fprintf(stdout, "%*s  ", width, "");
    
    // print the header
    for (int i = 0; i < table->alphn; i++) {
      fprintf(stdout, "%*c", width+1, table->alph[i]);
    }
    fprintf(stdout, "\n\n");

    // print the body
    for (int i = 0; i < table->n; i++) {
      fprintf(stdout, "%*d: ", width, i);
      for (int j = 0; j < table->alphn; j++) {
        fprintf(stdout, "%*d", width+1, getOcc(table, table->alph[j], i));
      }
      fprintf(stdout, "\n");
    }
  }
  if (findrange) {
    fprintf(stdout, "\n");
    range(s, q, n, strlen(q));
  }

  if (SA) free(SA);
  if (BW) free(BW);
  if (C)  free(C);
  if (alph) free(alph);
  if (table) free(table);
}

/**
 * Helper functions
 */

// sorts the array using the provided string and pointers
void sort(char * s, int * arr, int l, int r) {
  if (l >= r) return;
  int tmp [r - l + 1];
  // store the left and right as backups
  int lb = l;
  int rb = r;
  int pivot = (lb + rb) / 2;
  for (int i = lb; i <= rb; i++) {
    // don't need to sort the pivot
    if (i == pivot) continue;
    // if the string pointed to by the value in the array at i
    // is less than the value pointed to by the pivot, push
    // its index to the far left of the temporary array, otherwise
    // push it to the right. Then, update the l/r value to point
    // to the furthest available spot.
    int res = strcmp(&s[arr[i]], &s[arr[pivot]]);
    if (res < 0) {
      tmp[l - lb] = arr[i];
      l += 1;
    } else {
      tmp[r - lb] = arr[i];
      r -= 1;
    }
  }
  // either l or r will point to the middle, this is arbitrary.
  // put the pivot in that spot
  arr[l] = arr[pivot];
  for (int i = lb; i <= rb; i++) {
    // fill the main array with the values stored in the temp
    // array, skipping the pivot's position, of course.
    if (i == l) continue;
    arr[i] = tmp[i - lb];
  }
  // recursive calls to the subarrays
  sort(s, arr, lb, r - 1);
  sort(s, arr, l + 1, rb);
}

// Gets the alphabet of a given string
char * sAlph(char * s, size_t n) {
  bool memo [128];
  memset(memo, false, sizeof(bool) * 128);
  int size = 0;

  for (int i = 0; i < n; i++) {
    memo[s[i]] = true;
  }

#ifdef DEBUG
  for (int i = 0; i < 128; i++) {
    fprintf(stdout, "%3d: %s\n", i, memo[i] ? "true" : "false");
  }
#endif

  char * alph = malloc(sizeof(char) * 128);

  int index = 0;
  for (int i = 0; i < 128; i++) {
    if (memo[i]) {
      alph[index++] = i;
    }
  }
  alph[index] = 0;
  return alph;
}


/**
 * primary calls
 */

// takes a string and turns it into a suffix array with sorting
int * suffixArray(char * s, size_t n) {
  int * arr = malloc(sizeof(int) * n);
  // fill the array with 0 - n. These represent addresses within
  // the string array that will be used for comparison later.
  for (int i = 0; i < n; i++) {
    arr[i] = i;
  }
  sort(s, arr, 0, n - 1);
#ifdef DEBUG
  for (int i = 0; i < n; i++) {
    fprintf(stdout, "% 2d ", arr[i]);
  }
  fprintf(stdout, "\n");
#endif
  return arr;
}

// Turns an array into a BW string
char * BWtable(char * s, int * SA, size_t n) {
  char * BW = malloc(sizeof(char) * (n + 1));
  for (int i = 0; i < n; i++) {
    int index = SA[i] - 1;
    if (index < 0) index = n - 1;
    BW[i] = s[index];
  }
  BW[n] = 0;
  return BW;
}

// Gets the count table from a string using a suffix array
int * Ctable(char * s, size_t n, int * SA) {
  int * C = malloc(sizeof(int) * 128);
  memset(C, 0, sizeof(int) * 128);
  int * SAp = SA;
  if (!SA) {
    SAp = suffixArray(s, n);
  }

  char lc = s[SAp[0]];
  C[lc + 1] += 1;
  for (int i = 1; i < n; i++) {
    char c = s[SAp[i]];
    if (c >= 127) break;
    if (c == lc) {
      C[c + 1] += 1;
      continue;
    }
    int j = lc + 2;
    for (; j <= c + 1; j++) {
      C[j] = C[j - 1];
    }
    C[c + 1] += 1;
    lc = c;
  }
  for (int i = lc + 2; i < 128; i++) {
    C[i] = C[i - 1];
  }

  if (!SA) {
    free(SAp);
  }
  return C;
}

// returns an occurrence table struct for a given string 
occTable * makeOccTable(char * s, size_t n) {
  char * alph = sAlph(s, n);
  int alphn = strlen(alph);
  occTable * table = malloc(sizeof(occTable) + sizeof(int) * (alphn * n));
  memset(table, 0, sizeof(occTable) + sizeof(int) * (alphn * n));
  table->alphn = alphn;
  table->n = n;
  memcpy(table->alph, alph, sizeof(char) * alphn);
  for (int i = 0; i < alphn; i++) {
    table->map[alph[i]] = i;
  }
  for (int i = 0; i < n; i++) {
    table->data[n * table->map[s[i]] + i]++;
    if (i == 0) continue;
    for (int j = 0; j < alphn; j++) {
      table->data[n * j + i] += table->data[n * j + i - 1];
    }
  }
  return table;
}

// gets a character's occurrence given a particular table
int getOcc(occTable * table, char c, int i) {
  if (i < 0) return 0;
  if (i >= table->n) i = table->n - 1;
  return table->data[table->map[c] * table->n + i];
}

// The FMsearch algorithm. prints the range of the given pattern q in the string
// s.
void range(char * s, char * q, size_t n, size_t m) {
  int * C = Ctable(s, n, NULL);
  int * SA = suffixArray(s, n);
  char * BW = BWtable(s, SA, n);
  occTable * table = makeOccTable(BW, n);
  int st = 0;
  int ed = n - 1;
  for (int i = m - 1; i >= 0 && st <= ed; i--){
    char c = q[i];
    st = C[c] + getOcc(table, c, st - 1);
    ed = C[c] + getOcc(table, c, ed) - 1;
#ifdef DEBUG
    fprintf(stdout, "Step %d: x = %c, st = %d, ed = %d\n", m - i, c, st, ed);
#endif
  }
  fprintf(stdout, "S = %s\n", s);
  if (st <= ed) {
    fprintf(stdout, "range(S, %s) = [%d, %d]\n", q, st, ed);
  } else {
    fprintf(stdout, "%s not found\n", q);
  }
  free(table);
  free(C);
}
