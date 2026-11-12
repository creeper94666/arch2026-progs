#ifndef BLOOM_BANKS
#define BLOOM_BANKS 3
#endif
#ifdef HOST_STATS
unsigned long bloom_calls, bloom_rejects;
#define BLOOM_COUNT(x) (++bloom_##x)
#else
#define BLOOM_COUNT(x) ((void) 0)
#endif
#ifndef GOAL_FIRST
#define GOAL_FIRST 1
#endif
#ifndef ROLLING
#define ROLLING 1
#endif
#ifndef LEAF_SKIP
#define LEAF_SKIP 1
#endif
/* Readable C integration of the verified hash and combined search controls. */
#ifndef NO_INVERSE
#define NO_INVERSE 1
#endif
#include "rubik.h"

enum {
    CUBIES = 7,
    MOVES = 9,
    MAX_DEPTH = 11,
    PERMUTATIONS = 5040,
    ORIENTATIONS = 729
};

/* Same fixed-corner representation and turning rules as solver.c.
 * apply_move reads one state and writes another via pointers. */
typedef struct {
    uint8_t p[CUBIES], o[CUBIES];
} state_t;


static const char *const move_names[MOVES] = {"R",  "R2", "R'", "B", "B2",
                                              "B'", "D",  "D2", "D'"};
/* Fixed move compositions, generated on the host: 126 bytes total.
 * face: R=0, B=1, D=2; turns: 1=90, 2=180, 3=270 degrees.
 * src and dst must be distinct; each result is written once. */
static void copy_state(state_t *dst, const state_t *src)
{
    for (int i = 0; i < CUBIES; ++i) {
        dst->p[i] = src->p[i];
        dst->o[i] = src->o[i];
    }
}

static uint8_t move_face(uint8_t move)
{
    return (uint8_t) (move < 3 ? 0 : (move < 6 ? 1 : 2));
}

static uint8_t move_turns(uint8_t move, uint8_t face)
{
    return (uint8_t) (move + 1 - ((face << 1) + face));
}

#include "fixed_moves.h"
static void apply_move(const state_t *src,
                       state_t *dst,
                       uint8_t face,
                       uint8_t turns)
{
    fixed_apply_move(src, dst, ((unsigned) face << 1) + face + turns - 1);
}

static int is_solved(const state_t *state)
{
    for (int i = 0; i < CUBIES; ++i)
        if (state->p[i] != i || state->o[i] != 0)
            return 0;
    return 1;
}

/* Compact coordinate encoders used by the target heuristic. */
/* Lehmer code: map the seven-corner permutation to 0..5039. */
static int permutation_index(const state_t *state)
{
    int rank = 0;
    for (int i = 0; i < CUBIES; ++i) {
        int smaller = 0;
        for (int j = i + 1; j < CUBIES; ++j)
            if (state->p[j] < state->p[i])
                ++smaller;
        /* Multiply by the remaining radix using RV32I shifts and adds. */
        switch (CUBIES - i) {
        case 7:
            rank = (rank << 2) + (rank << 1) + rank;
            break;
        case 6:
            rank = (rank << 2) + (rank << 1);
            break;
        case 5:
            rank = (rank << 2) + rank;
            break;
        case 4:
            rank <<= 2;
            break;
        case 3:
            rank = (rank << 1) + rank;
            break;
        case 2:
            rank <<= 1;
            break;
        case 1:
            break;
        }
        rank += smaller;
    }
    return rank;
}

/* The seventh orientation follows from the sum-modulo-three invariant. */
static int orientation_index(const state_t *state)
{
    int rank = 0;
    for (int i = 0; i < CUBIES - 1; ++i)
        rank = (rank << 1) + rank + state->o[i];
    return rank;
}

/* AI-assisted experiment: direct state lookup into host-generated abstract
 * distances. Seven-cubie permutations are uniquely identified by six values.
 * No move table or distance construction is needed on target.
 */
#include "search_tables.h"
#ifndef FIVE_STEP
#define FIVE_STEP 1
#endif
static int heuristic(const state_t *state)
{
    int hp = compact_perm_dist[permutation_index(state)];
    int ho = compact_orient_dist[orientation_index(state)];
    return hp > ho ? hp : ho;
}

/* Return a hash SLOT, not a sorted-table index; -1 means not within five moves.
 */
static int endgame_lookup(const state_t *state)
{
    unsigned key = 0;
    for (int i = 0; i < 6; ++i)
        key = (key << 5) | ((unsigned) state->p[i] << 2) | state->o[i];

    unsigned h = endgame_hash(key);
    unsigned slot = h & 16383u;
    unsigned step = endgame_step(h);

    for (;;) {
        unsigned stored_key = endgame_keys[slot];
        if (stored_key == key)
            return (int) slot;
        if (stored_key == 0xffffffffu)
            return -1;
        slot = (slot + step) & 16383u;
    }
}

#if BLOOM_BANKS > 0
#include "bloom_bits.h"
static int bloom6_maybe(const state_t *state)
{
    BLOOM_COUNT(calls);
    unsigned key = 0;
    for (int i = 0; i < 6; ++i)
        key = (key << 5) | ((unsigned) state->p[i] << 2) | state->o[i];
    unsigned h = endgame_hash(key), step = (h >> 16) | 1u;
    for (unsigned bank = 0; bank < BLOOM_BANKS; ++bank, h += step) {
        unsigned bit = h & 65535u;
        if (!(bloom6_bits[bank * 8192 + (bit >> 3)] & (1u << (bit & 7)))) {
            BLOOM_COUNT(rejects);
            return 0;
        }
    }
    return 1;
}
#endif

/* Optional host diagnostics compile out of the target. */
#ifndef SOLVER7_SEARCH_COUNT
#define SOLVER7_SEARCH_COUNT(kind) ((void) 0)
#endif
static int search_bound(const state_t *state, int remaining)
{
    SOLVER7_SEARCH_COUNT(bounds);
    int h = heuristic(state);
    if (h > remaining)
        return h;

#if NO_INVERSE
    return h;
#else
    /* The inverse cube is equally far from solved. Its orientation abstraction
     * can expose a lower bound hidden by the forward orientation projection.
     * Reuse orient_dist; no extra pattern database is needed. */
    SOLVER7_SEARCH_COUNT(inverse);
    uint8_t inverse_orientation[CUBIES];
    for (int i = 0; i < CUBIES; ++i)
        inverse_orientation[state->p[i]] = state->o[i] ? 3 - state->o[i] : 0;
    int oi = 0;
    for (int i = 0; i < 6; ++i)
        oi = (oi << 1) + oi + inverse_orientation[i];
    int hi = compact_orient_dist[oi];
    return hi > h ? hi : h;
#endif
}

#include "sequences.h"
#include "small_rolling.h"

/* Precondition: start is valid; solution has room for MAX_DEPTH integers.
 * Return the number of moves (0 for solved), or -1 if none fits in 11.
 * Only solution[0..return_value-1] is written, using move numbers 1..9.
 * No recursion, heap, or target BFS queue; optional exact endgame table.
 */
static int solve(const state_t *start, int solution[MAX_DEPTH])
{
    state_t states[MAX_DEPTH + 1];
    /* 每層目前試到的走法；0 表示尚未嘗試，1..9 表示走法編號。 */
    int moves[MAX_DEPTH + 1] = {0};
    unsigned forbidden[MAX_DEPTH + 1];

    if (is_solved(start))
        return 0;

    /* 第一層：由起點下界開始，逐步增加允許的步數。 */
    for (int limit = search_bound(start, MAX_DEPTH); limit <= MAX_DEPTH;
         ++limit) {
#if BLOOM_BANKS > 0
        /* Root limit6: test once, before expanding its children. */
        if (limit == 6 && !bloom6_maybe(start))
            continue;
#endif
        int depth = 0;
        copy_state(&states[0], start);
        moves[0] = 0;
        forbidden[0] = 0;

        /* 第二層：沿目前路徑前進，走不下去就回溯。 */
        while (depth >= 0) {
            if ((!GOAL_FIRST || limit - depth > 5) &&
                is_solved(&states[depth])) {
                for (int i = 0; i < depth; ++i)
                    solution[i] = moves[i];
                return depth;
            }

#if FIVE_STEP
            if (limit - depth <= 5) {
                /* 1. Hash lookup of the current complete state. */
                int ix = endgame_lookup(&states[depth]);
                /* 2. The matching slot stores distance and next move. */
                int distance = ix < 0 ? 6 : endgame_data[ix] >> 4;

                if (distance <= limit - depth) {
                    for (int i = 0; i < depth; ++i)
                        solution[i] = moves[i];
                    state_t tail = states[depth];
                    int total = depth + distance;
                    while (distance > 0) {
                        int move = endgame_data[ix] & 15;
                        solution[depth++] = move;
                        state_t next;
                        uint8_t face = move_face((uint8_t) (move - 1));
                        apply_move(&tail, &next, face,
                                   move_turns((uint8_t) (move - 1), face));
                        tail = next;
                        --distance;
                        /* 3. Hash the updated tail state for its next move. */
                        if (distance)
                            ix = endgame_lookup(&tail);
                    }
                    return total;
                }
                --depth;
                continue;
            }
#endif

            if (depth == limit) {
                --depth;
                continue;
            }

            int descended = 0;
            /* 第三層：從上次尚未試過的走法，依序試到 9。 */
            for (int move = moves[depth] + 1; move <= MOVES; ++move) {
                /* 保存目前走法；失敗或回溯後，從下一種繼續試。 */
                moves[depth] = move;

                /* Reject same-face moves and noncanonical suffixes before
                 * constructing a cube or consulting any distance table. */
                if ((forbidden[depth] >> (move - 1)) & 1U) {
                    SOLVER7_SEARCH_COUNT(sequences);
                    continue;
                }

                state_t *candidate = &states[depth + 1];
                uint8_t face = move_face((uint8_t) (move - 1));
                uint8_t turns = move_turns((uint8_t) (move - 1), face);
                SOLVER7_SEARCH_COUNT(candidates);
                apply_move(&states[depth], candidate, face, turns);
                int new_depth = depth + 1;
                if ((!FIVE_STEP || limit - new_depth > 5) &&
                    new_depth + search_bound(candidate, limit - new_depth) >
                        limit)
                    continue;

#if BLOOM_BANKS > 0
                /* Check a remaining6 candidate once, before child metadata. */
                if (limit - new_depth == 6 && !bloom6_maybe(candidate))
                    continue;
#endif
                depth = new_depth;
                if (!LEAF_SKIP || limit - depth > 5) {
                    moves[depth] = 0;
                    forbidden[depth] =
                        ROLLING
                            ? rolling_forbidden(forbidden[depth - 1],
                                                moves[depth - 1], depth, moves)
                            : sequence_forbidden(moves, depth);
                }
                descended = 1;
                break;
            }

            /* 九種走法都處理完：父層狀態已在 states 中，不需逆轉。 */
            if (!descended)
                --depth;
        }
    }
    return -1;
}

static int parse_state(const char *input, state_t *state)
{
    unsigned length = 0;
    while (length < 15 && input[length])
        ++length;
    if (length != 14)
        return 0;
    unsigned seen = 0;
    int sum = 0;
    for (int i = 0; i < CUBIES; ++i) {
        if (input[i] < '1' || input[i] > '7' || input[i + CUBIES] < '1' ||
            input[i + CUBIES] > '3')
            return 0;
        state->p[i] = (uint8_t) (input[i] - '1');
        state->o[i] = (uint8_t) (input[i + CUBIES] - '1');
        unsigned bit = 1U << state->p[i];
        if (seen & bit)
            return 0;
        seen |= bit;
        sum += state->o[i];
    }
    while (sum >= 3)
        sum -= 3;
    return sum == 0;
}


/* Parse before writing out: invalid input must leave every output byte intact.
 * Measure the complete text before writing so short buffers only get out[0]=0.
 */
int rubik_solve(const char *input, char *out, size_t cap)
{
    state_t start;
    int solution[MAX_DEPTH];
    if (!parse_state(input, &start))
        return -1;
    int length = solve(&start, solution);
    if (length < 0)
        return -1;
    size_t needed = 1;
    for (int i = 0; i < length; ++i) {
        const char *name = move_names[solution[i] - 1];
        needed += (name[1] ? 2u : 1u) + (i != 0);
    }
    if (cap < needed) {
        if (cap)
            out[0] = '\0';
        return -2;
    }
    char *p = out;
    for (int i = 0; i < length; ++i) {
        if (i)
            *p++ = ' ';
        const char *name = move_names[solution[i] - 1];
        *p++ = name[0];
        if (name[1])
            *p++ = name[1];
    }
    *p = '\0';
    return length;
}
