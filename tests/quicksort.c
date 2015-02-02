#include <stdlib.h>
#include <stdio.h>

#define ARRAY_SIZE 1024

static void swap(void *x, void *y, size_t l) {
  char *a = x, *b = y, c;
  while(l--) {
    c = *a;
    *a++ = *b;
    *b++ = c;
  }
}

int count = 0;
static void sort(char *array, size_t size, int (*cmp)(void*,void*), int begin, int end) {
  count++;
  if (end > begin) {
    void *pivot = array + begin;
    int l = begin + size;
    int r = end;
    while(l < r) {
      if (cmp(array+l,pivot) <= 0) {
        l += size;
      } else if ( cmp(array+r, pivot) > 0 )  {
        r -= size;
      } else if ( l < r ) {
        swap(array+l, array+r, size);
      }
    }
    l -= size;
    swap(array+begin, array+l, size);
    sort(array, size, cmp, begin, l);
    sort(array, size, cmp, r, end);
  }
}

void quicksort(void *array, size_t nitems, size_t size, int (*cmp)(void*,void*)) {
  sort(array, size, cmp, 0, nitems*size);
}

typedef int type;

int type_cmp(void *a, void *b){ return (*(type*)a)-(*(type*)b); }

int main(void) {
  int array[ARRAY_SIZE];
  int len=sizeof(array)/sizeof(type);
  char *sep="";
  int i;

  srandom(time(NULL));
  for (i=0; i<len; i++) {
    array[i] = random() % 100;
  }

  quicksort(array, len, sizeof(type), type_cmp);

  printf("sorted_num_list={");
  for(i=0; i<len; i++){
    printf("%s%d", sep, array[i]);
    sep=", ";
  }
  printf("};\n");
  printf("count: %d\n", count);
  return 0;
}
