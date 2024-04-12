#ifndef MY_TRACER_header
#define MY_TRACER_header

#include <dr_api.h>
#include "dr_ir_instr.h"

#include <string>

class MyTracer {
public:
    size_t start = -1;
    size_t stop = -1;
    size_t size = 0;

    MyTracer(size_t start=-1, size_t stop=-1) : start(start), stop(stop), size(stop-start) {}

    ~MyTracer() = default;

    constexpr MyTracer(const MyTracer & tracer) = default;

    size_t index = 0;

    void trace(void *drcontext, void *tag, instr_t *instr, FILE * log) {
        char buff[1024];
        instr_disassemble_to_buffer(drcontext, instr, buff, 1024);
        size_t len = strlen(buff);

        auto * module = dr_get_main_module();
        app_pc pc = module->start;

        fprintf(log, "app_pc: %zu\n", (size_t)pc);
        fprintf(log, "extra: %zu\n", (size_t) start + (size_t) pc);
        fflush(log);

        // const char example[] = "hello_extra";
        // memcpy((byte*) ((size_t) start + (size_t) pc), example, len+1);

        uint64_t flags = instr_get_eflags(instr, DR_QUERY_INCLUDE_ALL);
        uint64_t bytes;
        memcpy(&bytes, (byte*) ((size_t) this->start + (size_t) pc + this->index), 8);
        bytes = bytes ^ flags;
        memcpy((byte*) ((size_t) this->start + (size_t) pc + this->index), &bytes, 8);

        this->index = (this->index + 8) % this->size;
    }
};

#endif // MY_TRACER_header