#define SLOTS 10
#define KEY 906 /* A random number.*/

typedef struct Vendor {
  int slots[SLOTS]; 
  int next_empty;
  int next_full;
} vendor_t;

