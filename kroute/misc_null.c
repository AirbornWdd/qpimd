#include <kroute.h>

#include "prefix.h"
#include "kroute/rtadv.h"
#include "kroute/irdp.h"
#include "kroute/interface.h"

void ifstat_update_proc (void) { return; }
#pragma weak rtadv_config_write = ifstat_update_proc
#pragma weak irdp_config_write = ifstat_update_proc
#pragma weak ifstat_update_sysctl = ifstat_update_proc
