#include <kroute.h>
#include "kroute/rib.h"
#include "kroute/zserv.h"

#include "kroute/redistribute.h"

void kroute_redistribute_add (int a, struct zserv *b, int c)
{ return; }
#pragma weak kroute_redistribute_delete = kroute_redistribute_add
#pragma weak kroute_redistribute_default_add = kroute_redistribute_add
#pragma weak kroute_redistribute_default_delete = kroute_redistribute_add

void redistribute_add (struct prefix *a, struct rib *b)
{ return; }
#pragma weak redistribute_delete = redistribute_add

void kroute_interface_up_update (struct interface *a)
{ return; }
#pragma weak kroute_interface_down_update = kroute_interface_up_update
#pragma weak kroute_interface_add_update = kroute_interface_up_update
#pragma weak kroute_interface_delete_update = kroute_interface_up_update

void kroute_interface_address_add_update (struct interface *a,
					 	struct connected *b)
{ return; }
#pragma weak kroute_interface_address_delete_update = kroute_interface_address_add_update
