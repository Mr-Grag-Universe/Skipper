#define X86

#include "dr_api.h"

#include "Config.h"
#include "Tracer.h"

static Configurator config(std::string("input/settings.json"));
static Tracer tracer(config);

void dr_client_main(client_id_t id, int argc, const char *argv[])
{
    dr_printf("DR configured\n");

    std::vector <CodeSegmentDescriber> code_segment_describers;
}