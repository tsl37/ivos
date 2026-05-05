#include <cstdint>
void idt_set_gate(int n, uint32_t handler);
void idt_load();