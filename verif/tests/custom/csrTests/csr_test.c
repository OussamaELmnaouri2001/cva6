/* perfmon.c (with new instruction)*/
#include <stdint.h>
#include <stdio.h>

/* ── CSR definitions (CV32A60AX) ─────────────────────────────────────────── */
#define CSR_MCOUNTINHIBIT   0x320
#define CSR_MINSTRET        0xB02
#define CSR_MHPMEVENT3      0x323
#define CSR_MHPMCOUNTER3    0xB03

/* ── CSR helpers ---------------------------------------------------- */
#define stringify(x) #x
#define csr_write(csr, val) \
    asm volatile("csrw " stringify(csr) ", %0" :: "rK"(val))
#define csr_read(csr) ({       \
    uint32_t _v;                \
    asm volatile("csrr %0, " stringify(csr) : "=r"(_v)); \
    _v;                         \
})

/* ── Safe load buffer for wl_loads and pure load ──────────────────────── */
//static const uint32_t load_data[3] = {7, 3, 5};

static void enable_hpm3(void) {
    uint32_t mci = csr_read(CSR_MCOUNTINHIBIT);
    mci &= ~(1u << 3);  // enable MHPMCOUNTER3 
   csr_write(CSR_MCOUNTINHIBIT, mci);
}

/* ── micro-ops ────────────────────────────────────────────────────── */
/*static void wl_stall(void) {
    asm volatile(
        "la   t3, load_data\n\t"
        "lw   t0, 0(t3)\n\t"
        "addi t1, t0, 1\n\t"
        "lw   t2, 4(t3)\n\t"
        "addi t3, t2, 1"
        ::: "t0","t1","t2","t3"
    );
}
static void wl_loads(void) {
    asm volatile(
        "la   t3, load_data\n\t"
        "lw   t0, 0(t3)\n\t"
        "lw   t1, 4(t3)\n\t"
        "lw   t2, 8(t3)\n\t"
        ::: "t0","t1","t2","t3"
    );
}
static void wl_stores(void) {
    asm volatile(
        "sw x0, 0(sp)\n\t"
        "sw x0, 4(sp)"
        :::
    );
}
static void wl_br(void) {
    asm volatile(
        "beq x0, x0, 1f\n\t"
        "nop\n"
        "1:"
    );
}
static void wl_call(void) {
    asm volatile("jal ra,1f\n1:" ::: "ra");
}
static void wl_ret(void) {
    asm volatile(
        "la   t0, 1f\n\t"
        "mv   ra, t0\n\t"
        "jalr x0, ra, 0\n"
        "1:"
        ::: "t0","ra"
    );
}
static void wl_addi(void) {
    asm volatile("addi t0, t0, 42" ::: "t0");
}
static void wl_mul(void) {
    asm volatile("mul t1, t0, t0" ::: "t1");
}
*/
void test_write_and_read_mhpm23(void) {
    printf("\n[Test Write MHPMEVENT3(23) directly]\n");

    asm volatile("li t0, 1\n\t"
                 "csrw 0x323, t0" ::: "t0");

    uint32_t val;
    asm volatile("csrr %0, 0x323" : "=r"(val));
    printf("  MHPM3(23) = %u\n", val);
}


void test_write_bit_23_in_mhpm3(void) {
    printf("\n[Test: Set bit 23 in MHPMEVENT3 (0x323)]\n");
    
    uint32_t bit23 = 1 << 23;

    asm volatile("csrw 0x323, %0" :: "r"(bit23));

    uint32_t val;
    asm volatile("csrr %0, 0x323" : "=r"(val));
    printf("  MHPMEVENT3 = %08x\n", val);
}


 

/*void test_write_and_read_mhpm24(void) {
    printf("\n[Test Write MHPMEVENT4(24) directly]\n");

    asm volatile("li t0, 0\n\t"
                 "csrw 0x324, t0" ::: "t0");

    uint32_t val;
    asm volatile("csrr %0, 0x324" : "=r"(val));
    printf("  MHPM3(24) = %u\n", val);
}*/


/* ── run one HPM test with dynamic reg capture ───────────────────────── */
/*static void run(uint32_t event, const char* label,
                void (*work)(void), uint8_t num_regs) {
    printf("\n[%s] (event %u)\n", label, event);
    csr_write(CSR_MHPMEVENT3, event);
    csr_write(CSR_MHPMCOUNTER3, 0);

    /* init registers t0..t2 */
/*    asm volatile(
        "li t0, 7\n\t"
        "li t1, 3\n\t"
        "li t2, 5"
        ::: "t0","t1","t2"
    );

    work();

    uint32_t cnt = csr_read(CSR_MHPMCOUNTER3);
    csr_write(CSR_MHPMEVENT3, 0);

    /* capture up to 4 regs */
  /* uint32_t r[4] = {0};
    asm volatile("mv %0, t0" : "=r"(r[0]));
    asm volatile("mv %0, t1" : "=r"(r[1]));
    if (num_regs > 2) asm volatile("mv %0, t2" : "=r"(r[2]));
    if (num_regs > 3) asm volatile("mv %0, t3" : "=r"(r[3]));

    printf("  HPM3 = %u\n  regs =", cnt);
    for (uint8_t i = 0; i < num_regs; i++) {
        printf(" t%u=0x%08x(%u)", i, r[i], r[i]);
    }
    printf("\n");
}*/

int main(void) {
    enable_hpm3();
    test_write_bit_23_in_mhpm3();
    test_write_and_read_mhpm23();

   // run(22, "Pipeline stall", wl_stall, 4);
    //test_write_and_read_mhpm24();

/* Measure exactly three loads back‐to‐back */
//csr_write(CSR_MHPMEVENT3, 5);       // Select “Memory LOADs” event
//csr_write(CSR_MHPMCOUNTER3, 0);     // Zero the counter
/*asm volatile(
    "la   t3, load_data\n\t"        // point to your constant buffer
    "lw   t0, 0(t3)\n\t"            // load #1
    "lw   t1, 4(t3)\n\t"            // load #2
    "lw   t2, 8(t3)"               // load #3
    ::: "t0","t1","t2","t3"
);*/
//uint32_t three_loads = csr_read(CSR_MHPMCOUNTER3);
///csr_write(CSR_MHPMEVENT3, 0);      // Disable counting
/*
printf("[Triple Memory LOAD] = %u\n", three_loads);
    run(23,  "Oussama1", wl_write_23, 1);
    run(5,  "Memory LOADs", wl_loads, 4);
    run(23,  "Oussma", wl_loads, 4);
    run(6,  "Memory STOREs", wl_stores, 2);
    run(24, "MHPMEVENT4", wl_write_24, 1);
    run(9,  "Branch instructions", wl_br, 2);
    run(12, "CALL instructions", wl_call, 2);
    run(13, "RETURN instructions", wl_ret, 2);
    run(25, "MHPMEVENT5", wl_write_25, 1);
    run(20, "Integer ADDI", wl_addi, 2);
    run(20, "Integer MUL", wl_mul, 2);
*/
  //  uint32_t before = csr_read(CSR_MINSTRET);
    //asm volatile("addi x0, x0, 0");
    //printf("\n[minstret delta] = %u\n", csr_read(CSR_MINSTRET) - before);

    return 0;
}
	