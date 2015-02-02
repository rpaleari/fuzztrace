#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE 1024

static int bisect(int v[], int size, int key) {
  int start, end, middle, pos;

  start = 0;
  end = size-1;
  pos = -1;
  while (start <= end) {
    middle = (end+start)/2;

#if 0
    printf("Searching [%d - %d] for key %d, middle %d\n",
           start, end, key, middle);
#endif

    if (v[middle] > key) {
      end = middle-1;
    } else if (v[middle] < key) {
      start = middle+1;
    } else {
      pos = middle;
      break;
    }
  }

  return pos;
}

static void dump_array(int v[], int size) {
  const int blocksize = 32;
  int i, blockno;

  assert((size % blocksize) == 0);

  for (blockno=0; blockno<size/blocksize; blockno++) {
    printf("[pos]  ");
    for (i=0; i<blocksize; i++) {
      printf("%4d ", i+blocksize*blockno);
    }
    putchar('\n');

    printf("[elem] ");
    for (i=0; i<blocksize; i++) {
      printf("%4d ", v[i+blocksize*blockno]);
    }
    putchar('\n');
  }

  putchar('\n');
}

static int compare(const void *a, const void *b) {
  return *(int*) a - *(int*) b;
}

int main(int argc, char **argv) {
  // int array[] = {5, 55, 12, 0, 42, 14, 2, 1, 59, 14, 4, 11, 18, 19, 50, 0};
  int array[ARRAY_SIZE];
  int n = sizeof(array) / sizeof(int);
  int elem, i;

  if (argc > 1) {
    elem = atoi(argv[1]);
  } else {
    elem = 42;
    printf("No element specified, searching for %d\n", elem);
  }

  srandom(time(NULL));
  for (i=0; i<n; i++) {
    array[i] = random() % 100;
  }
  array[random() % n] = elem;
  
  printf("Initial array:\n");
  dump_array(array, n);

  qsort(array, n, sizeof(int), compare);

  printf("Sorted array:\n");
  dump_array(array, n);

  printf("Search for element '%d' returned: %d\n",
         elem, bisect(array, n, elem));

  return 0;
}
