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

void test_write_and_read_mhpm23(void) {
    printf("\n[Test Write MHPMEVENT3(23) directly]\n");

    asm volatile("li t0, 1\n\t"
                 "csrw 0x323, t0" ::: "t0");

    uint32_t val;
    asm volatile("csrr %0, 0x323" : "=r"(val));
    printf("  MHPM3(23) = %u\n", val);
}


void test_write_bit_23_in_mhpm3(void) {
    printf("\n[Test: Set bit 23 in MHPMEVENT3 ]\n");
    
    uint32_t bit23 = 1 << 23;

    asm volatile("csrw 0x323, %0" :: "r"(bit23));

    uint32_t val;
    asm volatile("csrr %0, 0x323" : "=r"(val));
    printf("  MHPMEVENT3 = %08x\n", val);
}

void test_write_bit_24_25_26_in_mhpm3(void) {
    printf("\n[Test: Set bits 24:26 in MHPMEVENT3]\n");
    
    uint32_t bit2456 = 7 << 24;

    asm volatile("csrw 0x323, %0" :: "r"(bit2456));

    uint32_t val;
    asm volatile("csrr %0, 0x323" : "=r"(val));
    printf("  MHPMEVENT3 = %08x\n", val);
}


int main(void) {
    enable_hpm3();
    test_write_bit_23_in_mhpm3();
    test_write_and_read_mhpm23();
    test_write_bit_24_25_26_in_mhpm3();
    test_write_and_read_mhpm23();
    return 0;
}
	