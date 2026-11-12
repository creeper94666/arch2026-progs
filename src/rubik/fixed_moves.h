/* Generated fixed moves. Valid orientations0..2; src/dst disjoint. */
static void fixed_apply_move(const state_t *src, state_t *dst, unsigned move0)
{
    switch (move0) {
    default:
        /* Valid search moves are 0..8. Keep the destination initialized even if
         * a private caller supplies another value; this also makes data flow
         * explicit to compilers checking the unrolled switch under
         * -Wmaybe-uninitialized. */
        *dst = *src;
        return;
    case 0: /* R */
        dst->p[0] = src->p[1];
        {
            int o = src->o[1] + 1;
            if (o >= 3)
                o -= 3;
            dst->o[0] = (uint8_t) o;
        }
        dst->p[1] = src->p[4];
        {
            int o = src->o[4] + 2;
            if (o >= 3)
                o -= 3;
            dst->o[1] = (uint8_t) o;
        }
        dst->p[2] = src->p[2];
        dst->o[2] = src->o[2];
        dst->p[3] = src->p[0];
        {
            int o = src->o[0] + 2;
            if (o >= 3)
                o -= 3;
            dst->o[3] = (uint8_t) o;
        }
        dst->p[4] = src->p[3];
        {
            int o = src->o[3] + 1;
            if (o >= 3)
                o -= 3;
            dst->o[4] = (uint8_t) o;
        }
        dst->p[5] = src->p[5];
        dst->o[5] = src->o[5];
        dst->p[6] = src->p[6];
        dst->o[6] = src->o[6];
        return;
    case 1: /* R2 */
        dst->p[0] = src->p[4];
        dst->o[0] = src->o[4];
        dst->p[1] = src->p[3];
        dst->o[1] = src->o[3];
        dst->p[2] = src->p[2];
        dst->o[2] = src->o[2];
        dst->p[3] = src->p[1];
        dst->o[3] = src->o[1];
        dst->p[4] = src->p[0];
        dst->o[4] = src->o[0];
        dst->p[5] = src->p[5];
        dst->o[5] = src->o[5];
        dst->p[6] = src->p[6];
        dst->o[6] = src->o[6];
        return;
    case 2: /* R' */
        dst->p[0] = src->p[3];
        {
            int o = src->o[3] + 1;
            if (o >= 3)
                o -= 3;
            dst->o[0] = (uint8_t) o;
        }
        dst->p[1] = src->p[0];
        {
            int o = src->o[0] + 2;
            if (o >= 3)
                o -= 3;
            dst->o[1] = (uint8_t) o;
        }
        dst->p[2] = src->p[2];
        dst->o[2] = src->o[2];
        dst->p[3] = src->p[4];
        {
            int o = src->o[4] + 2;
            if (o >= 3)
                o -= 3;
            dst->o[3] = (uint8_t) o;
        }
        dst->p[4] = src->p[1];
        {
            int o = src->o[1] + 1;
            if (o >= 3)
                o -= 3;
            dst->o[4] = (uint8_t) o;
        }
        dst->p[5] = src->p[5];
        dst->o[5] = src->o[5];
        dst->p[6] = src->p[6];
        dst->o[6] = src->o[6];
        return;
    case 3: /* B */
        dst->p[0] = src->p[0];
        dst->o[0] = src->o[0];
        dst->p[1] = src->p[1];
        dst->o[1] = src->o[1];
        dst->p[2] = src->p[2];
        dst->o[2] = src->o[2];
        dst->p[3] = src->p[4];
        {
            int o = src->o[4] + 1;
            if (o >= 3)
                o -= 3;
            dst->o[3] = (uint8_t) o;
        }
        dst->p[4] = src->p[5];
        {
            int o = src->o[5] + 2;
            if (o >= 3)
                o -= 3;
            dst->o[4] = (uint8_t) o;
        }
        dst->p[5] = src->p[6];
        {
            int o = src->o[6] + 1;
            if (o >= 3)
                o -= 3;
            dst->o[5] = (uint8_t) o;
        }
        dst->p[6] = src->p[3];
        {
            int o = src->o[3] + 2;
            if (o >= 3)
                o -= 3;
            dst->o[6] = (uint8_t) o;
        }
        return;
    case 4: /* B2 */
        dst->p[0] = src->p[0];
        dst->o[0] = src->o[0];
        dst->p[1] = src->p[1];
        dst->o[1] = src->o[1];
        dst->p[2] = src->p[2];
        dst->o[2] = src->o[2];
        dst->p[3] = src->p[5];
        dst->o[3] = src->o[5];
        dst->p[4] = src->p[6];
        dst->o[4] = src->o[6];
        dst->p[5] = src->p[3];
        dst->o[5] = src->o[3];
        dst->p[6] = src->p[4];
        dst->o[6] = src->o[4];
        return;
    case 5: /* B' */
        dst->p[0] = src->p[0];
        dst->o[0] = src->o[0];
        dst->p[1] = src->p[1];
        dst->o[1] = src->o[1];
        dst->p[2] = src->p[2];
        dst->o[2] = src->o[2];
        dst->p[3] = src->p[6];
        {
            int o = src->o[6] + 1;
            if (o >= 3)
                o -= 3;
            dst->o[3] = (uint8_t) o;
        }
        dst->p[4] = src->p[3];
        {
            int o = src->o[3] + 2;
            if (o >= 3)
                o -= 3;
            dst->o[4] = (uint8_t) o;
        }
        dst->p[5] = src->p[4];
        {
            int o = src->o[4] + 1;
            if (o >= 3)
                o -= 3;
            dst->o[5] = (uint8_t) o;
        }
        dst->p[6] = src->p[5];
        {
            int o = src->o[5] + 2;
            if (o >= 3)
                o -= 3;
            dst->o[6] = (uint8_t) o;
        }
        return;
    case 6: /* D */
        dst->p[0] = src->p[0];
        dst->o[0] = src->o[0];
        dst->p[1] = src->p[2];
        dst->o[1] = src->o[2];
        dst->p[2] = src->p[5];
        dst->o[2] = src->o[5];
        dst->p[3] = src->p[3];
        dst->o[3] = src->o[3];
        dst->p[4] = src->p[1];
        dst->o[4] = src->o[1];
        dst->p[5] = src->p[4];
        dst->o[5] = src->o[4];
        dst->p[6] = src->p[6];
        dst->o[6] = src->o[6];
        return;
    case 7: /* D2 */
        dst->p[0] = src->p[0];
        dst->o[0] = src->o[0];
        dst->p[1] = src->p[5];
        dst->o[1] = src->o[5];
        dst->p[2] = src->p[4];
        dst->o[2] = src->o[4];
        dst->p[3] = src->p[3];
        dst->o[3] = src->o[3];
        dst->p[4] = src->p[2];
        dst->o[4] = src->o[2];
        dst->p[5] = src->p[1];
        dst->o[5] = src->o[1];
        dst->p[6] = src->p[6];
        dst->o[6] = src->o[6];
        return;
    case 8: /* D' */
        dst->p[0] = src->p[0];
        dst->o[0] = src->o[0];
        dst->p[1] = src->p[4];
        dst->o[1] = src->o[4];
        dst->p[2] = src->p[1];
        dst->o[2] = src->o[1];
        dst->p[3] = src->p[3];
        dst->o[3] = src->o[3];
        dst->p[4] = src->p[5];
        dst->o[4] = src->o[5];
        dst->p[5] = src->p[2];
        dst->o[5] = src->o[2];
        dst->p[6] = src->p[6];
        dst->o[6] = src->o[6];
        return;
    }
}
