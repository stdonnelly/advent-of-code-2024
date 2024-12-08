#include <stdio.h>
#include <stdlib.h>

// Define point using shorts instead of int or size_t because the real input is only 50x50
// This allows us to make other structures that are smaller than 64-bit for efficiency
typedef struct Point
{
    short row;
    short col;
} Point;

int main(int argc, char const *argv[])
{
    if (argc < 5)
        return 0;

    Point antenna1 = {atoi(argv[1]), atoi(argv[2])};
    Point antenna2 = {atoi(argv[3]), atoi(argv[4])};
    Point antinode1;
    Point antinode2;

    /* Find both antinodes
       For example:
       - (3,4),(5,5) -> (1,3),(7,6)
       - (4,8),(5,5) -> (6,2),(3,11)
    */
   short row_diff = antenna2.row - antenna1.row;
   short col_diff = antenna2.col - antenna1.col;
   antinode1.row = antenna1.row - row_diff;
   antinode1.col = antenna1.col - col_diff;
   antinode2.row = antenna2.row + row_diff;
   antinode2.col = antenna2.col + col_diff;

    printf("Antinodes: (%hd,%hd),(%hd,%hd)\n", antinode1.row, antinode1.col, antinode2.row, antinode2.col);
    return 0;
}
