/*===----------------------------------------------------------------------===
 * isa32lm_softrt.c - Runtime de software para ISA32_LM
 *
 * Implementa las funciones que LLVM llama automáticamente cuando marca una
 * operación como "Expand -> LibCall" en ISA32_LMISelLowering.cpp:
 *
 *   32 bits (int):            __divsi3, __modsi3, __umodsi3
 *   64 bits (long long):      __udivdi3, __umoddi3, __divdi3, __moddi3,
 *                             __muldi3
 *
 * IMPORTANTE sobre el diseño:
 *   El hardware ISA32_LM todavía NO tiene instrucciones de suma/resta con
 *   acarreo (ADDC/SUBC) a nivel de 32 bits, así que el compilador no puede
 *   generar código para `long long + long long` ni `long long - long long`.
 *   Por eso, TODAS las funciones de 64 bits de este archivo evitan usar
 *   +, -, <<, >> directamente sobre `unsigned long long`/`long long` en el
 *   camino de cálculo: en vez de eso, cada valor de 64 bits se parte a mano
 *   en dos mitades de 32 bits (hi, lo) y el acarreo/préstamo se calcula con
 *   comparaciones normales de 32 bits (que sí funcionan). Los únicos lugares
 *   donde se toca un `long long` directamente son partirlo (>> 32, cast) y
 *   reconstruirlo al final (<< 32, |) — esos casos son shifts por exactamente
 *   32, que el legalizador de LLVM resuelve como simple movimiento de datos,
 *   sin necesitar ADDC/SUBC.
 *
 *   Tampoco hay instrucción de multiplicación que devuelva la mitad alta del
 *   producto (MULH), así que el 32x32->64 completo (necesario para el 64x64)
 *   también está armado a mano, partiendo cada operando en mitades de 16 bits.
 *
 * Cuando en algún momento agregues ADDC/SUBC de hardware, este archivo se
 * puede simplificar bastante (los helpers de abajo dejarían de hacer falta
 * y se podría escribir todo con aritmética normal de 64 bits).
 *===----------------------------------------------------------------------===*/

typedef unsigned int u32;
typedef unsigned long long u64;
typedef long long s64;

/*===----------------------------------------------------------------------===
 * Helpers de 64 bits construidos a mano sobre pares (hi, lo) de 32 bits.
 * Ninguno de estos usa +, -, <<, >> nativos de 64 bits.
 *===----------------------------------------------------------------------===*/

/* Partir un u64 en sus dos mitades de 32 bits. El shift es por exactamente
 * 32 bits, así que LLVM lo resuelve como simple movimiento de registros. */
static u32 hi32(u64 x) { return (u32)(x >> 32); }
static u32 lo32(u64 x) { return (u32)x; }

/* Reconstruir un u64 a partir de sus dos mitades. Mismo caso: shift de
 * exactamente 32 bits + OR, sin necesidad de ADDC. */
static u64 join64(u32 hi, u32 lo) { return ((u64)hi << 32) | (u64)lo; }

/* (hi,lo) += (ahi,alo), usando solo ADD/SUB/comparación de 32 bits. */
static void add64p(u32 *hi, u32 *lo, u32 ahi, u32 alo) {
  u32 nlo = *lo + alo;
  u32 carry = (nlo < *lo) ? 1u : 0u; /* hubo overflow de 32 bits al sumar */
  *lo = nlo;
  *hi = *hi + ahi + carry;
}

/* (hi,lo) -= (shi,slo), usando solo ADD/SUB/comparación de 32 bits. */
static void sub64p(u32 *hi, u32 *lo, u32 shi, u32 slo) {
  u32 borrow = (*lo < slo) ? 1u : 0u; /* hace falta "pedir prestado" a hi */
  *lo = *lo - slo;
  *hi = *hi - shi - borrow;
}

/* Negación en complemento a 2 de (hi,lo): ~x + 1, propagando el acarreo
 * del +1 a mano (solo hay acarreo si lo original era 0). */
static void neg64p(u32 *hi, u32 *lo) {
  u32 nlo = (~(*lo)) + 1u;
  u32 carry = (nlo == 0u) ? 1u : 0u;
  *hi = (~(*hi)) + carry;
  *lo = nlo;
}

/* (hi,lo) <<= 1, pasando el bit más alto de lo al bit más bajo de hi. */
static void shl64p_1(u32 *hi, u32 *lo) {
  u32 carry = (*lo >> 31) & 1u;
  *hi = (*hi << 1) | carry;
  *lo = (*lo << 1);
}

/* ¿(ahi,alo) >= (bhi,blo), sin signo? */
static int uge64p(u32 ahi, u32 alo, u32 bhi, u32 blo) {
  if (ahi != bhi)
    return ahi > bhi;
  return alo >= blo;
}

/*===----------------------------------------------------------------------===
 * Multiplicación 32x32 -> 64 bits completa, sin instrucción MULH.
 * Se parte cada operando en mitades de 16 bits para que cada producto
 * parcial entre siempre en un MUL de 32 bits nativo, sin perder bits.
 *===----------------------------------------------------------------------===*/
static u64 umul32to64(u32 a, u32 b) {
  u32 a_lo = a & 0xFFFFu, a_hi = a >> 16;
  u32 b_lo = b & 0xFFFFu, b_hi = b >> 16;

  u32 p00 = a_lo * b_lo; /* bit 0  */
  u32 p01 = a_lo * b_hi; /* bit 16 */
  u32 p10 = a_hi * b_lo; /* bit 16 */
  u32 p11 = a_hi * b_hi; /* bit 32 */

  u32 cross = p01 + p10;
  u32 cross_carry = (cross < p01) ? 1u : 0u;

  u32 res_hi = 0, res_lo = p00;
  add64p(&res_hi, &res_lo, (cross >> 16) | (cross_carry << 16), cross << 16);
  add64p(&res_hi, &res_lo, p11, 0);

  return join64(res_hi, res_lo);
}

/*===----------------------------------------------------------------------===
 * División/módulo sin signo de 64 bits: long division bit a bit (64
 * iteraciones). No es la más rápida, pero es correcta y usa solo
 * operaciones de 32 bits que ya funcionan.
 *===----------------------------------------------------------------------===*/
static void udivmod64_core(u32 a_hi, u32 a_lo, u32 b_hi, u32 b_lo, u32 *q_hi,
                            u32 *q_lo, u32 *r_hi, u32 *r_lo) {
  u32 qh = 0, ql = 0, rh = 0, rl = 0;
  int i;
  for (i = 63; i >= 0; i--) {
    u32 bit = (i >= 32) ? ((a_hi >> (i - 32)) & 1u) : ((a_lo >> i) & 1u);

    shl64p_1(&rh, &rl);
    rl |= bit;

    if (uge64p(rh, rl, b_hi, b_lo)) {
      sub64p(&rh, &rl, b_hi, b_lo);
      if (i >= 32)
        qh |= (1u << (i - 32));
      else
        ql |= (1u << i);
    }
  }
  *q_hi = qh;
  *q_lo = ql;
  *r_hi = rh;
  *r_lo = rl;
}

/*===----------------------------------------------------------------------===
 * Funciones que LLVM llama directamente (nombres fijos, no cambiar)
 *===----------------------------------------------------------------------===*/

/* ---- 32 bits: ADD/SUB/UDIV de hardware ya funcionan, esto es simple ---- */

int __divsi3(int a, int b) {
  u32 ua = (u32)a, ub = (u32)b;
  int neg = 0;
  if (a < 0) { ua = 0u - ua; neg = !neg; }
  if (b < 0) { ub = 0u - ub; neg = !neg; }
  u32 uq = ua / ub; /* instrucción DIV de hardware (sin signo) */
  return neg ? (int)(0u - uq) : (int)uq;
}

int __modsi3(int a, int b) {
  u32 ua = (u32)a, ub = (u32)b;
  int neg = (a < 0);
  if (a < 0) ua = 0u - ua;
  if (b < 0) ub = 0u - ub;
  u32 uq = ua / ub;
  u32 ur = ua - uq * ub; /* evita % a propósito, para no recursar */
  return neg ? (int)(0u - ur) : (int)ur;
}

unsigned __umodsi3(unsigned a, unsigned b) {
  unsigned q = a / b;
  return a - q * b; /* evita % a propósito, para no recursar */
}

/* ---- 64 bits: armadas a mano por la falta de ADDC/SUBC de hardware ---- */

u64 __udivdi3(u64 a, u64 b) {
  u32 q_hi, q_lo, r_hi, r_lo;
  udivmod64_core(hi32(a), lo32(a), hi32(b), lo32(b), &q_hi, &q_lo, &r_hi,
                 &r_lo);
  return join64(q_hi, q_lo);
}

u64 __umoddi3(u64 a, u64 b) {
  u32 q_hi, q_lo, r_hi, r_lo;
  udivmod64_core(hi32(a), lo32(a), hi32(b), lo32(b), &q_hi, &q_lo, &r_hi,
                 &r_lo);
  return join64(r_hi, r_lo);
}

s64 __divdi3(s64 a, s64 b) {
  u32 a_hi = hi32((u64)a), a_lo = lo32((u64)a);
  u32 b_hi = hi32((u64)b), b_lo = lo32((u64)b);
  u32 q_hi, q_lo, r_hi, r_lo;
  int neg = 0;

  if ((int)a_hi < 0) { neg64p(&a_hi, &a_lo); neg = !neg; }
  if ((int)b_hi < 0) { neg64p(&b_hi, &b_lo); neg = !neg; }

  udivmod64_core(a_hi, a_lo, b_hi, b_lo, &q_hi, &q_lo, &r_hi, &r_lo);
  if (neg) neg64p(&q_hi, &q_lo);

  return (s64)join64(q_hi, q_lo);
}

s64 __moddi3(s64 a, s64 b) {
  u32 a_hi = hi32((u64)a), a_lo = lo32((u64)a);
  u32 b_hi = hi32((u64)b), b_lo = lo32((u64)b);
  u32 q_hi, q_lo, r_hi, r_lo;
  int neg = 0; /* el resto toma el signo del dividendo, como en C */

  if ((int)a_hi < 0) { neg64p(&a_hi, &a_lo); neg = 1; }
  if ((int)b_hi < 0) { neg64p(&b_hi, &b_lo); }

  udivmod64_core(a_hi, a_lo, b_hi, b_lo, &q_hi, &q_lo, &r_hi, &r_lo);
  if (neg) neg64p(&r_hi, &r_lo);

  return (s64)join64(r_hi, r_lo);
}

/* La multiplicación truncada a 64 bits da igual con o sin signo (misma
 * representación en complemento a 2), así que no hace falta separar
 * __muldi3 en signed/unsigned. */
s64 __muldi3(s64 a, s64 b) {
  u32 a_hi = hi32((u64)a), a_lo = lo32((u64)a);
  u32 b_hi = hi32((u64)b), b_lo = lo32((u64)b);

  u64 result = umul32to64(a_lo, b_lo); /* producto exacto de las mitades bajas */
  u32 cross = a_hi * b_lo + a_lo * b_hi; /* solo importan sus 32 bits bajos */

  u32 res_hi = hi32(result), res_lo = lo32(result);
  add64p(&res_hi, &res_lo, cross, 0); /* 'cross' vale en el bit 32 */

  return (s64)join64(res_hi, res_lo);
}
