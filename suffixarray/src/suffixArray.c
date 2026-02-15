/**********************************************************************
 * suffix array algorithm based on sorting suffixes                   *
 * suffixArray.c                                                      *
 * Aleksandr Means                                                    *
 **********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

int * suffixArray(char *, size_t);

int main(int argc, char ** argv) {
  if (argc != 2) {
    fprintf(stdout, "malformed arguments\n");
    return 1;
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
  uint32_t * SA = suffixArray(s, n);
  
  
  if (SA) {
    for (uint32_t i = 0; i < n; i++) {
      fprintf(stdout, "SA[%llu] = %llu\n", i, SA[i]);
    }
  }

  if (SA) free(SA);
}

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
