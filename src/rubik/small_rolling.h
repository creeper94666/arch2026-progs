static const unsigned short rolling_times729[9] = {0,    729,  1458, 2187, 2916,
                                                   3645, 4374, 5103, 5832};
static unsigned rolling_forbidden(unsigned parent,
                                  int move,
                                  int depth,
                                  const int *history)
{
    unsigned index = parent >> 9;
    if (depth >= 5)
        index -= rolling_times729[history[depth - 5] - 1];
    index = index * 9 + (unsigned) (move - 1);
    unsigned shift = move <= 3 ? 0 : (move <= 6 ? 3 : 6), mask = 7u << shift;
    if (depth >= 2) {
        unsigned byte = depth == 2   ? sequence_reject3[index]
                        : depth == 3 ? sequence_reject4[index]
                                     : sequence_reject5[index];
        unsigned low = (1u << shift) - 1;
        mask |= (byte & low) | ((byte & ~low) << 3);
    }
    return mask | (index << 9);
}
