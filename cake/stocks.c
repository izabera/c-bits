/*
Suppose we could access yesterday's stock prices as an array, where:
- The indices are the time in minutes past trade opening time, which was 9:30am local time.
- The values are the price in dollars of Apple stock at that time.

For example, if the stock cost $500 at 10:30am, stock_prices_yesterday[60] = 500.

Write an efficient function that takes stock_prices_yesterday and returns the best profit
I could have made from 1 purchase and 1 sale of 1 Apple stock
yesterday.
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>


int profit(int *stock, size_t len) {
  /*if (len < 2) exit(1);*/
  unsigned int i;
  int profit = stock[1] - stock[0], smallest_so_far;
  if (stock[1] > stock[0])
    smallest_so_far = stock[0];
  else
    smallest_so_far = stock[1];

  for (i = 2; i < len; i++) {
    if (stock[i-1] < smallest_so_far)
      smallest_so_far = stock[i-1];
    if (profit < stock[i] - smallest_so_far)
      profit = stock[i] - smallest_so_far;
  }

  return profit;
}

int main() {
  srand(time(0));
#define NINTERVALS /*(24*60)*/ 10
  int stock[NINTERVALS];
  for (int i = 0; i < NINTERVALS; i++) stock[i] = rand() % 1000;
  for (int i = 0; i < NINTERVALS; i++) printf("%d: %d\n", i, stock[i]);
  printf("best profit: %d$\n", profit(stock, NINTERVALS));
  return 0;
}
