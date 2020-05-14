#define SLOTS 10

typedef struct Vendor {
  int slots[SLOTS]; 
  int next_empty;
  int next_full;
} vendor_t;

