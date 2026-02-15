/*********************************
 * Z ALGORITHM for given strings *
 *                               *
 * Aleksandr Means               *
 *********************************/
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

size_t * z_algorithm_n(char * S, size_t size);
size_t z_pattern_match(char * p, char * t);

int main(int argc, char ** argv) {
  // check the number of arguments and take the correct path
  // if it is just one argument, it will do the normal z algorithm
  if (argc == 2) {
    size_t * z_values = z_algorithm_n(argv[1], strlen(argv[1]));
    size_t max_z_value = z_values[0];
    fprintf(stdout, "[%llu", z_values[0]);
    for (size_t i = 1; i < strlen(argv[1]); i++) {
      fprintf(stdout, " %llu", z_values[i]);
      max_z_value = max_z_value > z_values[i] ? max_z_value : z_values[i];
    }
    fprintf(stdout, "]\n");
    fprintf(stdout, "\nMax z-value is %llu\n", max_z_value);
    free(z_values);
  }
  if (argc < 3) return 0;

  // if there is more than one argument, it will do the pattern match
  // using the z algorithm on the first two strings
  uint32_t a_l = strlen(argv[1]);
  uint32_t b_l = strlen(argv[2]);


  size_t index = z_pattern_match(argv[1], argv[2]);

  if (index == b_l) {
    fprintf(stdout, "pattern did not match\n");
  } else {
    fprintf(stdout, "pattern matched at index %llu\n", index);
    for (int i = 0; i < b_l; i++) {
      if (i == index) fprintf(stdout, " [");
      if (i == index + a_l) fprintf(stdout, "] ");
      fprintf(stdout, "%c", argv[2][i]);
    }
    fprintf(stdout, "\n");
  }
  return 0;
}

// The standard z algorithm
size_t * z_algorithm_n(char * S, size_t size) {
  if (S == NULL) return NULL;
  size_t * z_values = malloc(size * sizeof(size_t));
  
  z_values[0] = 0;
  size_t l = 0;
  size_t r = 0;

  for (size_t i = 1; i < size; i++) {
    z_values[i] = 0;
    if (i < r) {
      size_t zip = z_values[i - l];
      if (zip + i > r) z_values[i] = r - i;
      else if (zip + i < r) z_values[i] = zip;
      else {
        l = i;
        for (size_t j = r - l; j < size - i; j++) {
          if (S[j + i] != S[j]) break;
          r++;
          z_values[i]++;
        }
      }
    } else {
      l = i;
      r = i;
      for (size_t j = 0; j < size - i; j++) {
        if (S[j + i] != S[j]) break;
        r++;
        z_values[i]++;
      }
    }
  }
  return z_values;
}

// The pattern match algorithm using the z algorithm
size_t z_pattern_match(char * p, char * t) {
  if (p == NULL || t == NULL) return (size_t) -1;
  size_t t_l = strlen(t);
  size_t p_l = strlen(p);

  size_t l = 0;
  size_t r = 0;

  size_t max_value = 0;

  size_t * z_values = malloc(p_l * sizeof(size_t));

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
