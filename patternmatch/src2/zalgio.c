/***********************************************
 * Z-ALGORITHM for a pattern string and a file *
 *                                             *
 * Aleksandr Means                             *
 ***********************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

size_t z_pattern_match(char * p, char * t);

int main(int argc, char ** argv) {
  // early return if the arguments aren't formatted correctly
  if (argc < 3 || argc > 3) return 0;

  char * p = argv[1]; // the pattern string
  char * t = NULL;   // the text string
  FILE * pFile = fopen(argv[2], "r"); // pointer to the given file
  if (pFile == NULL) {
    fprintf(stderr, "error reading file at %s\n", argv[2]);
    return 1;
  }
  

  fseek(pFile, 0L, SEEK_END);
  size_t fsize = ftell(pFile); // the size of the file
  fseek(pFile, 0L, SEEK_SET);

  // We will now read the file into the text string
  t = malloc(fsize);
  char * next = t; // pointer to the next position in the string to read the file line into

  while (!feof(pFile)) {
    // the upper bound to read characters into the text string
    size_t n = fsize - (next - t); 

    // read the next file line into the text string at 'next'
    getline(&next, &n, pFile); 

    // find the next instance of \n, \r, >, or ';', and set that to next
    // This will have the effect of overwriting the string starting at any
    // of these characters. This will erase comments, and will remove the 
    // carriage returns or newline characters.
    next = strpbrk(next, "\n\r>;");
  }
  
  // sets the final newline/carriage return to 0 if it exists.
  if (next) next[0] = 0;

  size_t index = z_pattern_match(p, t);

  // after doing the z algorithm, print results.
  size_t a_l = strlen(p);
  size_t b_l = strlen(t);

  if (index == b_l) {
    fprintf(stdout, "pattern did not match\n");
  } else {
    fprintf(stdout, "pattern matched at index %llu\n", index);
    for (int i = 0; i < b_l; i++) {
      if (i == index) fprintf(stdout, " [");
      if (i == index + a_l) fprintf(stdout, "] ");
      fprintf(stdout, "%c", t[i]);
    }
    fprintf(stdout, "\n");
  }
  free(t);
  return 0;
}

size_t z_pattern_match(char * p, char * t) {
  if (p == NULL || t == NULL) return (size_t) -1;
  size_t t_l = strlen(t);
  size_t p_l = strlen(p);

  size_t l = 0;
  size_t r = 0;

  size_t max_value = 0;

  size_t * z_values = malloc(p_l * sizeof(size_t));
  
  // initialize the z values for the pattern string
  for (int i = 1; i < p_l; i++) {
    z_values[i] = 0;
    for (int j = 0; j < p_l - i; j++) {
      if (p[j] == p[i + j]) z_values[i]++;
      else break;
    }
  }
  size_t i = 0;
  //  This is the main pattern matching loop
  for (; i < t_l; i++) {
    //  We check if i is still inside the frame
    if (i < r) {
      //  When in the frame we check if the pattern at the z value (given by i - k) 
      //  is equal to the end of the frame.
      if (z_values[i - l] + i == r) {
        //  If it is, then we pattern match starting at the right side of the frame to 
        //  see if the pattern continues in the string from i. Furthermore, we bring the 
        //  beginning of the frame to the current index.
        l = i;
        for (size_t j = r - i; j < t_l - i && j < p_l; j++) {
          if (p[j] != t[i + j]) break;
          r++;
        }
      }
    } else {
      //  When outside of the frame, we need to find the next frame to search through
      //  we start by setting l and r to i, thereby 'initializing' a new frame with size 0.
      l = i;
      r = i;
      //  We will iterate over the characters, incrementing the r value for each matching 
      //  character.
      for (size_t j = 0; j < t_l - i && j < p_l; j++) {
        if (p[j] != t[i + j]) break;
        r++;
      }
    }
    if (r - l == p_l) {
      break;
    }
  }
  free(z_values);
  return i;
}
