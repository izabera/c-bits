/*
You have an array of integers, and for each index you want to find the product
of every integer except the integer at that index.
Write a function get_products_of_all_ints_except_at_index() that takes an array
of integers and returns an array of the products.

For example, given: [1, 7, 3, 4]
your function would return: [84, 12, 28, 21]
by calculating: [7*3*4, 1*3*4, 1*7*4, 1*7*3]

Do not use division in your solution
*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void prods(unsigned int *arr, size_t len) {
  unsigned int pre[len], post[len];
  unsigned int tmp = 1, i;
  for (i = 0; i < len; i++) {
    tmp *= arr[i];
    pre[i] = tmp;
  }
  tmp = 1;
  for (i = 0; i < len; i++) {
    tmp *= arr[len-i-1];
    post[len-i-1] = tmp;
  }
  arr[0] = post[1];
  arr[len-1] = pre[len-2];
  for (i = 1; i < len - 1; i++)
    arr[i] = pre[i-1] * post[i+1];
}

int main() {
  srand(time(0));
#define NELEM 10
  unsigned int arr[NELEM], i;
  for (i = 0; i < NELEM; i++) arr[i] = rand() % 100;
  for (i = 0; i < NELEM; i++) printf("%d: %u\n", i, arr[i]);
  prods(arr, NELEM);
  for (i = 0; i < NELEM; i++) printf("%d: %u\n", i, arr[i]);
  return 0;
}
