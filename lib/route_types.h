/* Auto-generated from route_types.txt by . */
/* Do not edit! */

#ifndef _BANE_ROUTE_TYPES_H
#define _BANE_ROUTE_TYPES_H

/* Kroute route's types. */
#define KROUTE_ROUTE_SYSTEM               0
#define KROUTE_ROUTE_KERNEL               1
#define KROUTE_ROUTE_CONNECT              2
#define KROUTE_ROUTE_STATIC               3
#define KROUTE_ROUTE_RIP                  4
#define KROUTE_ROUTE_RIPNG                5
#define KROUTE_ROUTE_OSPF                 6
#define KROUTE_ROUTE_OSPF6                7
#define KROUTE_ROUTE_ISIS                 8
#define KROUTE_ROUTE_BGP                  9
#define KROUTE_ROUTE_HSLS                 10
#define KROUTE_ROUTE_OLSR                 11
#define KROUTE_ROUTE_BABEL                12
#define KROUTE_ROUTE_PIM                  13
#define KROUTE_ROUTE_MAX                  14

#define SHOW_ROUTE_V4_HEADER \
  "Codes: K - kernel route, C - connected, S - static, R - RIP,%s" \
  "       O - OSPF, I - IS-IS, B - BGP, A - Babel,%s" \
  "       > - selected route, * - FIB route%s%s", \
  VTY_NEWLINE, VTY_NEWLINE, VTY_NEWLINE, VTY_NEWLINE
#define SHOW_ROUTE_V6_HEADER \
  "Codes: K - kernel route, C - connected, S - static, R - RIPng,%s" \
  "       O - OSPFv6, I - IS-IS, B - BGP, A - Babel,%s" \
  "       > - selected route, * - FIB route%s%s", \
  VTY_NEWLINE, VTY_NEWLINE, VTY_NEWLINE, VTY_NEWLINE

/* babeld */
#define BANE_REDIST_STR_BABELD \
  "(kernel|connected|static|rip|ripng|ospf|ospf6|isis|bgp)"
#define BANE_REDIST_HELP_STR_BABELD \
  "Kernel routes (not installed via the kroute RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol (RIP)\n" \
  "Routing Information Protocol next-generation (IPv6) (RIPng)\n" \
  "Open Shortest Path First (OSPFv2)\n" \
  "Open Shortest Path First (IPv6) (OSPFv3)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n"
#define BANE_IP_REDIST_STR_BABELD \
  "(kernel|connected|static|rip|ospf|isis|bgp)"
#define BANE_IP_REDIST_HELP_STR_BABELD \
  "Kernel routes (not installed via the kroute RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol (RIP)\n" \
  "Open Shortest Path First (OSPFv2)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n"
#define BANE_IP6_REDIST_STR_BABELD \
  "(kernel|connected|static|ripng|ospf6|isis|bgp)"
#define BANE_IP6_REDIST_HELP_STR_BABELD \
  "Kernel routes (not installed via the kroute RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol next-generation (IPv6) (RIPng)\n" \
  "Open Shortest Path First (IPv6) (OSPFv3)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n"

/* bgpd */
#define BANE_REDIST_STR_BGPD \
  "(kernel|connected|static|rip|ripng|ospf|ospf6|isis|babel)"
#define BANE_REDIST_HELP_STR_BGPD \
  "Kernel routes (not installed via the kroute RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol (RIP)\n" \
  "Routing Information Protocol next-generation (IPv6) (RIPng)\n" \
  "Open Shortest Path First (OSPFv2)\n" \
  "Open Shortest Path First (IPv6) (OSPFv3)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Babel routing protocol (Babel)\n"
#define BANE_IP_REDIST_STR_BGPD \
  "(kernel|connected|static|rip|ospf|isis|babel)"
#define BANE_IP_REDIST_HELP_STR_BGPD \
  "Kernel routes (not installed via the kroute RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol (RIP)\n" \
  "Open Shortest Path First (OSPFv2)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Babel routing protocol (Babel)\n"
#define BANE_IP6_REDIST_STR_BGPD \
  "(kernel|connected|static|ripng|ospf6|isis|babel)"
#define BANE_IP6_REDIST_HELP_STR_BGPD \
  "Kernel routes (not installed via the kroute RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol next-generation (IPv6) (RIPng)\n" \
  "Open Shortest Path First (IPv6) (OSPFv3)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Babel routing protocol (Babel)\n"

/* isisd */
#define BANE_REDIST_STR_ISISD \
  "(kernel|connected|static|rip|ripng|ospf|ospf6|bgp|babel)"
#define BANE_REDIST_HELP_STR_ISISD \
  "Kernel routes (not installed via the kroute RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol (RIP)\n" \
  "Routing Information Protocol next-generation (IPv6) (RIPng)\n" \
  "Open Shortest Path First (OSPFv2)\n" \
  "Open Shortest Path First (IPv6) (OSPFv3)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Babel routing protocol (Babel)\n"
#define BANE_IP_REDIST_STR_ISISD \
  "(kernel|connected|static|rip|ospf|bgp|babel)"
#define BANE_IP_REDIST_HELP_STR_ISISD \
  "Kernel routes (not installed via the kroute RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol (RIP)\n" \
  "Open Shortest Path First (OSPFv2)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Babel routing protocol (Babel)\n"
#define BANE_IP6_REDIST_STR_ISISD \
  "(kernel|connected|static|ripng|ospf6|bgp|babel)"
#define BANE_IP6_REDIST_HELP_STR_ISISD \
  "Kernel routes (not installed via the kroute RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol next-generation (IPv6) (RIPng)\n" \
  "Open Shortest Path First (IPv6) (OSPFv3)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Babel routing protocol (Babel)\n"

/* ospf6d */
#define BANE_REDIST_STR_OSPF6D \
  "(kernel|connected|static|ripng|isis|bgp|babel)"
#define BANE_REDIST_HELP_STR_OSPF6D \
  "Kernel routes (not installed via the kroute RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol next-generation (IPv6) (RIPng)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Babel routing protocol (Babel)\n"

/* ospfd */
#define BANE_REDIST_STR_OSPFD \
  "(kernel|connected|static|rip|isis|bgp|babel)"
#define BANE_REDIST_HELP_STR_OSPFD \
  "Kernel routes (not installed via the kroute RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol (RIP)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Babel routing protocol (Babel)\n"

/* ripd */
#define BANE_REDIST_STR_RIPD \
  "(kernel|connected|static|ospf|isis|bgp|babel)"
#define BANE_REDIST_HELP_STR_RIPD \
  "Kernel routes (not installed via the kroute RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Open Shortest Path First (OSPFv2)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Babel routing protocol (Babel)\n"

/* ripngd */
#define BANE_REDIST_STR_RIPNGD \
  "(kernel|connected|static|ospf6|isis|bgp|babel)"
#define BANE_REDIST_HELP_STR_RIPNGD \
  "Kernel routes (not installed via the kroute RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Open Shortest Path First (IPv6) (OSPFv3)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Babel routing protocol (Babel)\n"

/* kroute */
#define BANE_REDIST_STR_KROUTE \
  "(kernel|connected|static|rip|ripng|ospf|ospf6|isis|bgp|babel)"
#define BANE_REDIST_HELP_STR_KROUTE \
  "Kernel routes (not installed via the kroute RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol (RIP)\n" \
  "Routing Information Protocol next-generation (IPv6) (RIPng)\n" \
  "Open Shortest Path First (OSPFv2)\n" \
  "Open Shortest Path First (IPv6) (OSPFv3)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Babel routing protocol (Babel)\n"
#define BANE_IP_REDIST_STR_KROUTE \
  "(kernel|connected|static|rip|ospf|isis|bgp|babel)"
#define BANE_IP_REDIST_HELP_STR_KROUTE \
  "Kernel routes (not installed via the kroute RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol (RIP)\n" \
  "Open Shortest Path First (OSPFv2)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Babel routing protocol (Babel)\n"
#define BANE_IP6_REDIST_STR_KROUTE \
  "(kernel|connected|static|ripng|ospf6|isis|bgp|babel)"
#define BANE_IP6_REDIST_HELP_STR_KROUTE \
  "Kernel routes (not installed via the kroute RIB)\n" \
  "Connected routes (directly attached subnet or host)\n" \
  "Statically configured routes\n" \
  "Routing Information Protocol next-generation (IPv6) (RIPng)\n" \
  "Open Shortest Path First (IPv6) (OSPFv3)\n" \
  "Intermediate System to Intermediate System (IS-IS)\n" \
  "Border Gateway Protocol (BGP)\n" \
  "Babel routing protocol (Babel)\n"


#ifdef BANE_DEFINE_DESC_TABLE

struct kroute_desc_table
{
  unsigned int type;
  const char *string;
  char chr;
};

#define DESC_ENTRY(T,S,C) [(T)] = { (T), (S), (C) }
static const struct kroute_desc_table route_types[] = {
  DESC_ENTRY	(KROUTE_ROUTE_SYSTEM,	 "system",	'X' ),
  DESC_ENTRY	(KROUTE_ROUTE_KERNEL,	 "kernel",	'K' ),
  DESC_ENTRY	(KROUTE_ROUTE_CONNECT,	 "connected",	'C' ),
  DESC_ENTRY	(KROUTE_ROUTE_STATIC,	 "static",	'S' ),
  DESC_ENTRY	(KROUTE_ROUTE_RIP,	 "rip",	'R' ),
  DESC_ENTRY	(KROUTE_ROUTE_RIPNG,	 "ripng",	'R' ),
  DESC_ENTRY	(KROUTE_ROUTE_OSPF,	 "ospf",	'O' ),
  DESC_ENTRY	(KROUTE_ROUTE_OSPF6,	 "ospf6",	'O' ),
  DESC_ENTRY	(KROUTE_ROUTE_ISIS,	 "isis",	'I' ),
  DESC_ENTRY	(KROUTE_ROUTE_BGP,	 "bgp",	'B' ),
  DESC_ENTRY	(KROUTE_ROUTE_HSLS,	 "hsls",	'H' ),
  DESC_ENTRY	(KROUTE_ROUTE_OLSR,	 "olsr",	'o' ),
  DESC_ENTRY	(KROUTE_ROUTE_BABEL,	 "babel",	'A' ),
};
#undef DESC_ENTRY

#endif /* BANE_DEFINE_DESC_TABLE */

#endif /* _BANE_ROUTE_TYPES_H */
