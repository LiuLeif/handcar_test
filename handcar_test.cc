#include <alloca.h>

#include <algorithm>
#include <cstring>
#include <iostream>

#include "SimLoader.h"

const char* handcar_path = "./handcar_cosim.so";
uint64_t pc = 0;
extern "C" {
void update_generator_register(
    uint32_t cpuid, const char* pRegName, uint64_t rval, uint64_t mask,
    const char* pAccessType) {
    // printf("reg:%s:%lx\n", pRegName, rval);
    if (strcmp(pRegName, "PC") == 0) {
        pc = rval;
        // printf("pc -> %lx\n", pc);
    }
}

void update_generator_memory(
    uint32_t cpuid, uint64_t virtualAddress, uint32_t memBank,
    uint64_t physicalAddress, uint32_t size, const char* pBytes,
    const char* pAccessType) {}

void update_vector_element(
    uint32_t cpuid, const char* pRegName, uint32_t vRegIndex, uint32_t eltIndex,
    uint32_t eltByteWidth, const uint8_t* value, uint32_t byteLength,
    const char* pAccessType) {}
}

void disass(SimDllApi& sim, uint64_t pc) {
    char* opcode = (char*)alloca(100);
    char* disassembly = (char*)alloca(100);
    memset(opcode, '-', 100);
    memset(disassembly, '-', 100);
    opcode[99] = '\0';
    disassembly[99] = '\0';

    sim.get_disassembly(&pc, &opcode, &disassembly);
    std::cout << "D: " << disassembly << std::endl;
}

void test_step() {
    printf("----------------- test step ----------------\n");
    SimDllApi sim_api;
    open_sim_dll(handcar_path, &sim_api);
    std::string options = "--auto-init-mem";
    sim_api.initialize_simulator(options.c_str());

    uint8_t target_addr[8] = {0x0, 0x80, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
    int status = sim_api.write_simulator_register(0, "x1", target_addr, 8);

    uint8_t memory_data[8] = {0x01, 0x1, 0x1, 0x1, 0x2, 0x2, 0x2, 0x2};
    sim_api.write_simulator_memory(0, (uint64_t*)target_addr, 8, memory_data);

    // NOTE: 0x1000 是 spike bootrom 的地址 (DEFAULT_RSTVEC), pc 默认指向这里
    uint64_t instr_addr = 0x1000;
    uint8_t instr_data[4] = {0x03, 0xB1, 0x0, 0x0};  // LD x2 0(x1)
    status = sim_api.write_simulator_memory(0, &instr_addr, 4, instr_data);
    disass(sim_api, instr_addr);

    uint8_t x[8] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
    sim_api.read_simulator_register(0, "x2", x, 8);
    printf("before: x2 = ");
    for (int i = 0; i < 8; i++) {
        printf("0x%x ", x[i]);
    }
    printf("\n");

    status = sim_api.step_simulator(0, 1, 0);

    sim_api.read_simulator_register(0, "x2", x, 8);
    printf("after : x2 = ");
    for (int i = 0; i < 8; i++) {
        printf("0x%x ", x[i]);
    }
    printf("\n");

    sim_api.terminate_simulator();
    close_sim_dll(&sim_api);
}

void test_step_elf() {
    printf("----------------- test step elf ----------------\n");
    SimDllApi sim_api;
    open_sim_dll(handcar_path, &sim_api);
    std::string options = "--auto-init-mem";
    sim_api.initialize_simulator(options.c_str());
    sim_api.simulator_load_elf(0, "test.elf");

    pc = 0x1000;

    for (int i = 0; i < 10; i++) {
        disass(sim_api, pc);
        sim_api.step_simulator(0, 1, 0);
    }

    sim_api.terminate_simulator();
    close_sim_dll(&sim_api);
}

int main(int argc, char* argv[]) {
    test_step();
    test_step_elf();
    return 0;
}
