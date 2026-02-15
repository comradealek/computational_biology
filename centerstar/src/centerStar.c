/**********************************************************************
 * center star algorithm for approximate MSA                          *
 * center_star.c                                                      *
 * Aleksandr Means                                                    *
 **********************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//#define DEBUG

int globalAlignment(char *, char *, int, int, int, char **);
uint32_t minSequenceDistance(char **, uint32_t, int, int);
uint32_t centerStar(char **, uint32_t, int, int, char **);

#define SEEN    0x8
#define LEFT    0x4
#define UPLEFT  0x2
#define UP      0x1



int main(int argc, char ** argv) {
  // early return if there aren't enough arguments
  if (argc != 4) {
    fprintf(stdout, "expected three arguments: alpha, beta, and a filepath\n");
    return 0;
  }

  // convert first 3 arguments to numbers
  int alpha = atoi(argv[1]);
  int beta = atoi(argv[2]);

  char * s = NULL;   // the pointer to the entire char buffer
  FILE * pFile = fopen(argv[3], "r"); // pointer to the given file
  if (pFile == NULL) {
    fprintf(stderr, "error reading file at %s\n", argv[4]);
    return 1;
  }

  fseek(pFile, 0L, SEEK_END);
  size_t fsize = ftell(pFile); // the size of the file
  fseek(pFile, 0L, SEEK_SET);

  // We will now read the file into the text string
  s = malloc(fsize);
  
  char ** t = malloc(sizeof(char *) * 100); // array of string pointers
  uint32_t c_t = 0; // counter for the pointers
  char * next = s; // pointer to the next write position in the string

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

    // if the S string has had data read into it and we see a comment, then we 
    // start reading into the T string. Note that T is just a pointer into the 
    // buffer at S.
    if (next[0] == '>') {
      if (c_t >= 100) {
        fprintf(stdout, "This program does not support processing more than 100 strings\n");
        free(s);
        free(t);
        return 0;
      }
      next[0] = 0;
      if (next != s) next += 1; // we don't want to do this if it's the beginning
      t[c_t++] = next;
    }
  }
  
  // sets the final newline/carriage return to 0 if it exists.
  if (next) next[0] = 0;

  char * aligns [c_t];
  memset(aligns, 0, sizeof(char *) * c_t);
  //globalAlignment(s, t, match, mismatch, indel, &align);
  uint32_t c = centerStar(t, c_t, alpha, beta, aligns);

  int width = (int) log10(c_t) + 1;
  fprintf(stdout, "Alignment for strings 1-%d:\n", c_t);
  for (uint32_t i = 0; i < c_t; i++) {
    fprintf(stdout, "%*d: %s", width, i + 1, aligns[i]);
    if (i == c) fprintf(stdout, " *");
    fprintf(stdout, "\n");
  }
  if (aligns) {
    for (uint32_t i = 0; i < c_t; i++) {
      if (aligns[i]) free(aligns[i]);
    }
  }
  free(s);
  return 0;
}

// The center star algorithm
uint32_t centerStar(
  char **          pStrings, 
  uint32_t         nStrings, 
  int              alpha, 
  int              beta, 
  char **          pReturnString) 

{
  uint32_t c = minSequenceDistance(pStrings, nStrings, alpha, beta);
  // Get the alignments for Sc
  char * pAlignments [nStrings];
  for (uint32_t i; i < nStrings; i++) {
    globalAlignment(pStrings[c], pStrings[i], 0, -alpha, -beta, &pAlignments[i]);
  }
  size_t nSc = strlen(pStrings[c]);

#ifdef DEBUG
  fprintf(stdout, "%d\n", nStrings);
  for (uint32_t i = 0; i < nStrings; i++) {
    fprintf(stdout, "%p\n", pAlignments[i]);
    fprintf(stdout, "%s\n", pAlignments[i]);
  }
#endif

  // Get the insert counts in Sc
  uint32_t * pnInserts = malloc(sizeof(uint32_t) * nSc + 1);
  memset(pnInserts, 0, sizeof(uint32_t) * nSc + 1);
  for (uint32_t i = 0; i < nStrings; i++) {
    uint32_t cLineInserts = 0;
    for (size_t j = 0; j <= nSc; j++) {
      uint32_t cCharInserts = 0;
      while (pAlignments[i][j + cLineInserts + cCharInserts] == '_') {
        cCharInserts++;
      }
      if (cCharInserts > pnInserts[j]) pnInserts[j] = cCharInserts;
      cLineInserts += cCharInserts;
    }
  }

#ifdef DEBUG
  fprintf(stdout, "[");
  for (uint32_t i = 0; i <= nSc; i++) {
    fprintf(stdout, "%d ", pnInserts[i]);
  }
  fprintf(stdout, "\b]\n");
#endif

  // Get the number of inserts that will be made in Sc
  uint32_t cInserts = 0;
  for (uint32_t i = 0; i <= nSc; i++) {
    cInserts += pnInserts[i];
  }

#ifdef DEBUG
  fprintf(stdout, "%d\n", cInserts);
#endif

  // Write the strings with the inserts
  uint32_t lenSA = cInserts + nSc + 1;
  for (uint32_t i = 0; i < nStrings; i++) {
    pReturnString[i] = malloc(sizeof(char) * lenSA);
    char * pos_Ai = pAlignments[i];
    char * pos_Si = strpbrk(pos_Ai, "\n") + 1;
    uint32_t inserts = 0;
    for (uint32_t j = 0; j < nSc + 1; j++) {
      // Write characters if insert is expected; otherwise write insert
      for (uint32_t k = 0; k < pnInserts[j]; k++) {
        if (*pos_Ai == '_') {
          pReturnString[i][j + inserts + k] = *pos_Si;
          pos_Si++;
          pos_Ai++;
        } else {
          pReturnString[i][j + inserts + k] = '_';
        }
      }
      inserts += pnInserts[j];
      // Write the characters
      char c = *pos_Si;
      if (j == nSc) c = 0;
      pReturnString[i][j + inserts] = c;
      pos_Si++;
      pos_Ai++;
    }
#ifdef DEBUG
    fprintf(stdout, "%s\n", *pReturnString);
#endif
  }

#ifdef DEBUG
  fprintf(stdout, "%p\n", *pReturnString);
  fprintf(stdout, "%s\n", *pReturnString);
#endif
  return c;
}

// Finds the minimum sequence and prints the sequence distance table
uint32_t minSequenceDistance(
  char **          pStrings, 
  uint32_t         nStrings, 
  int              alpha, 
  int              beta) 

{
  int T[nStrings][nStrings];
  int places = 0;
  
  // build the table
  memset(T, 0, sizeof(int) * nStrings * nStrings);
  for (int i = 0; i < nStrings; i++) {
    for (int j = i+1; j < nStrings; j++) {
      T[i][j] = -globalAlignment(pStrings[i], pStrings[j], 0, -alpha, -beta, NULL);
      T[j][i] = T[i][j];
      // while we're here, we'll get information for formatting
      int p = (int) log10(T[i][j]) + 1;
      places = p > places ? p : places;
    }
  }
  // find S_C
  int32_t mindist = 0x7fffffff;
  uint32_t idx = 0;
  for (uint32_t i = 0; i < nStrings; i++) {
    int32_t distance = 0;
    for (uint32_t j = 0; j < nStrings; j++) {
      distance += T[i][j];
    }
    if (distance < mindist) {
      mindist = distance;
      idx = i;
    }
  }
  // print the table
  int idx_width = (int) log10(nStrings) + 1;
  places = idx_width > places ? idx_width : places;
  fprintf(stdout, "%*c  ", idx_width, 'S');
  for (uint32_t i = 0; i < nStrings; i++) {
    fprintf(stdout, "%*d ", places, i + 1);
  }
  fprintf(stdout, "\n\n");
  for (uint32_t i = 0; i < nStrings; i++) {
    fprintf(stdout, "%*d  ", idx_width, i + 1);
    for (uint32_t j = 0; j < nStrings; j++) {
      fprintf(stdout, "%*d ",places, T[j][i]);
    }
    if (i == idx) fprintf(stdout, " <-");
    fprintf(stdout, "\n");
  }
  fprintf(stdout, "\n");
  return idx;
}

int globalAlignment(
  char *           str1, 
  char *           str2, 
  int              match, 
  int              mismatch, 
  int              indel, 
  char **          pRetStr)

{
  // get the number of characters in S and T
  size_t nStr1 = strlen(str1);
  size_t nStr2 = strlen(str2);

  // create the V matrix
  int V[nStr1+ 1][nStr2 + 1];
  for (int x = 0; x < nStr1 + 1; x++) {
    for (int y = 0; y < nStr2 + 1; y++) {
      if (x == 0 && y == 0) {
        V[x][y] = 0;
      } else if (x == 0) {
        V[x][y] = V[x][y - 1] + indel;
      } else if (y == 0) {
        V[x][y] = V[x - 1][y] + indel;
      } else {
        int u = V[x - 1][y] + indel;
        int l = V[x][y - 1] + indel;
        int ul = V[x - 1][y - 1] + (str1[x - 1] == str2[y - 1] ? match : mismatch);
        V[x][y] = (u > l ? (u > ul ? u : ul) : (l > ul ? l : ul));
      }
    }
  }
  // If there isn't a string to print to, return the score here
  if (pRetStr == NULL) return V[nStr1][nStr2];

#ifdef DEBUG
  // Print out the V matrix for debugging
  fprintf(stdout, " % 4c", '_');
  for (int y = 0; y < nStr2 + 1; y++) 
    fprintf(stdout, "% 4c", str2[y]);
  fprintf(stdout, "\n");
  for (int x = 0; x < nStr1 + 1; x++) {
    if (x == 0) fprintf(stdout, "_");
    else fprintf(stdout, "%c", str1[x - 1]);
    for (int y = 0; y < nStr2 + 1; y++) {
      fprintf(stdout, "% 4d", V[x][y]);
    }
    fprintf(stdout, "\n");
  }
#endif
  
  //tabulate all paths
  char V_b[nStr1 + 1][nStr2 + 1];
  memset(V_b, 0, (nStr1 + 1) * (nStr2 + 1));
  V_b[nStr1][nStr2] = SEEN;
  for (int x = nStr1; x >= 0; x--) {
    for (int y = nStr2; y >= 0; y--) {
      if (!(V_b[x][y] & SEEN)) continue;
      if (x == 0 && y == 0) break;
      if (x == 0) {
        V_b[x][y] = LEFT;
        V_b[x][y - 1] = SEEN;
      } else if (y == 0) {
        V_b[x][y] = UP;
        V_b[x - 1][y] = SEEN;
      } else {
        if (
          V[x - 1][y - 1] + match == V[x][y] && str1[x - 1] == str2[y - 1] ||
          V[x - 1][y - 1] + mismatch == V[x][y] && str1[x - 1] != str2[y - 1]
        ) {
          V_b[x][y] = UPLEFT;
          V_b[x - 1][y - 1] = SEEN;
        }
        if (V[x][y - 1] + indel == V[x][y]) {
          V_b[x][y] += LEFT;
          V_b[x][y - 1] = SEEN;
        }
        if (V[x - 1][y] + indel == V[x][y]) {
          V_b[x][y] += UP;
          V_b[x - 1][y] = SEEN;
        }
      }
    }
  }


#ifdef DEBUG
  for (int x = 0; x < nStr1 + 1; x++) {
    for (int y = 0; y < nStr2 + 1; y++) {
      if (x == 0 && y == 0) fprintf(stdout, "% 4c", '*');
      else {
        char directions[4] = {
          V_b[x][y] & LEFT      ? 'L' : '-',
          V_b[x][y] & UPLEFT    ? 'D' : '-',
          V_b[x][y] & UP        ? 'U' : '-',
          0
        };
        fprintf(stdout, "% 4s", directions);
      }
    }
    fprintf(stdout, "\n");
  }
#endif

  // create the alignment
  char Sa[nStr1 + nStr2 + 1];
  char Ta[nStr1 + nStr2 + 1];
  uint32_t x = 0;
  uint32_t y = 0;
  uint32_t i = 0;
  while (x < nStr1 || y < nStr2) {
    if (x == nStr1) {
      Sa[i] = '_';
      Ta[i] = str2[y];
      y++;
    } else if (y == nStr2) {
      Sa[i] = str1[x];
      Ta[i] = '_';
      x++;
    } else {
      if (V_b[x][y + 1] & LEFT) {
        Sa[i] = '_';
        Ta[i] = str2[y];
        y++;
      } else if (V_b[x + 1][y] & UP) {
        Sa[i] = str1[x];
        Ta[i] = '_';
        x++;
      } else if (V_b[x + 1][y + 1] & UPLEFT) {
        Sa[i] = str1[x];
        Ta[i] = str2[y];
        x++;
        y++;
      }
    }
    i++;
  }
  Sa[i] = 0;
  Ta[i] = 0;

  * pRetStr = malloc(strlen(Sa) + strlen(Ta) + 2);
  sprintf(* pRetStr, "%s\n%s", Sa, Ta);

  return V[nStr1][nStr2];
}
