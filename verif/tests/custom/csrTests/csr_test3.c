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

static void enable_hpm3_and_hpm4(void) {
    uint32_t mci = csr_read(CSR_MCOUNTINHIBIT);
    mci &= ~(1u << 3);  // Enable MHPMCOUNTER3
    mci &= ~(1u << 4);  // Enable MHPMCOUNTER4
    csr_write(CSR_MCOUNTINHIBIT, mci);
}

// Active bit 23 dans MHPMEVENT3 → active la contremesure
void write_bit_23_in_mhpm3(void) {
    uint32_t bit23 = 1 << 23;
    asm volatile("csrw 0x323, %0" :: "r"(bit23));
}

// Désactive MHPMEVENT3
void write_0_mhpm3(void) {
    asm volatile("li t0, 0\n\t"
                 "csrw 0x323, t0" ::: "t0");
}

// Désactive MHPMEVENT4
void write_0_mhpm4(void) {
    asm volatile("li t0, 0\n\t"
                 "csrw 0x324, t0" ::: "t0");
}

// Active bits 23–25 dans MHPMEVENT4 (filtrage par enclave_id)
void write_bit_23_24_25_in_mhpm4(void) {
    uint32_t bits = 0b101 << 23;  // bits 23, 24, 25
    asm volatile("csrw 0x324, %0" :: "r"(bits));
}

// Données partagées pour tester les accès mémoire
int shared_array[4] = {1, 2, 3, 4};

int read_shared(int index) {
    return shared_array[index];
}

void write_shared(int index, int value) {
    shared_array[index] = value;
}

int main(void) {
    enable_hpm3_and_hpm4();

    printf("\n=== Phase 1: Avant activation (pas d'isolation) ===\n");
    write_shared(0, 10);
    int x1 = read_shared(0);
    printf("Read shared_array[0] = %d\n", x1);

    int x2 = read_shared(1);
    printf("Read shared_array[1] = %d\n", x2);
    
    printf("\n=== Phase 2: Activation bits 23:25 dans MHPMEVENT4 (enclave_id filtering) + Activation bit 23 (contre-mesure activée) ===\n");
    write_bit_23_24_25_in_mhpm4();
    write_bit_23_in_mhpm3();
   
    write_shared(2, 77);
    int z = read_shared(2);
    printf("Read shared_array[2] (avec isolation par ID) = %d\n", z);

    printf("\n=== Phase 3: Désactivation de MHPMEVENT3et4 ===\n");
    write_0_mhpm3();
    write_0_mhpm4();
    
    int final_read = read_shared(0);
    printf("Final read shared_array[0] (après désactivation) = %d\n", final_read);

    return 0;
}
