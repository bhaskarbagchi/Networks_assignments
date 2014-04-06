#include <cnet.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

#define LEN_ETHERHEADER (2*sizeof(CnetNICaddr) + 2*sizeof(char))
#define LEN_BPDU (2*sizeof(CnetAddr) + sizeof(int) + sizeof(float))

#define is_host(s) (s >= 0 && s <= 1)
#define is_bridge(s) (s >= 2 && s <= 6)

#define disable_port(s) linkinfo[s].linkup = false;ports[s].kind = BLOCKED_PORT
#define enable_port(s)  linkinfo[s].linkup = true;ports[s].kind = DESIGNATED_PORT

#define TIMEOUT     2000000
#define SYNCHRONIZATION_TIME     5000
typedef struct {
    CnetNICaddr    dest;
    CnetNICaddr    src;
    char           type[2];
    char           data[LAN_MAXDATA];
} ETHERPACKET;

typedef struct {
    CnetAddr    macRoot;
    CnetAddr    macSender;
    int            numPortSender;
    float          pathCostUpToNow;
} BPDU;


typedef enum {
    HOST,
    BRIDGE,
    OTHER
} NODE_TYPE;

typedef enum {
    ROOT_PORT,
    DESIGNATED_PORT,
    BLOCKED_PORT,

} PORT_STATE;

typedef enum {
    ROOT,
    NORMAL
} BRIDGE_STATE;

typedef struct {
    PORT_STATE kind;
    float rootPathCost;
    CnetAddr remoteDesignate;

} PORT;

NODE_TYPE nodetype;

BRIDGE_STATE bridgestate;
CnetAddr macAddress;
CnetAddr rootAddress;

PORT* ports;
int rootPort;

BPDU b;

CnetNICaddr broadcastAddress;
// CnetTime synchronization_time;

int compare_mac_addcess(CnetAddr o1, CnetAddr o2) {
    if (o1 < o2) {
        return 1;
    }
    else if (o1 > o2) {
        return -1;
    }
    return 0;
}

void send_data_to_lan(CnetNICaddr dest, int num, char *buf, size_t length)
{
    ETHERPACKET    packet;
    short int      twobytes;

    memcpy(packet.dest, dest,                   sizeof(CnetNICaddr));
    memcpy(packet.src,  linkinfo[num].nicaddr, sizeof(CnetNICaddr));

    if (length > LAN_MAXDATA) {
        length = LAN_MAXDATA;
    }

    twobytes = length;           // type carries the data's true length
    memcpy(packet.type, &twobytes, 2);
    memcpy(packet.data, buf, length);

    length  += LEN_ETHERHEADER;
    if(length < LAN_MINPACKET)   // pad short packets to minimum length
        length = LAN_MINPACKET;
    CHECK( CNET_write_physical_reliable(num, (char *)&packet, &length) );
}

void send_bpdu_to_lan(CnetNICaddr dest, int num, BPDU* bpdu, size_t length)
{
    ETHERPACKET    packet;
    short int      twobytes;

    memcpy(packet.dest, dest,                   sizeof(CnetNICaddr));
    memcpy(packet.src,  linkinfo[num].nicaddr, sizeof(CnetNICaddr));

    if (length > LAN_MAXDATA) {
        length = LAN_MAXDATA;
    }

    twobytes = length;           // type carries the data's true length
    memcpy(packet.type, &twobytes, 2);
    memcpy(packet.data, bpdu, length);

    length  += LEN_ETHERHEADER;
    if(length < LAN_MINPACKET)   // pad short packets to minimum length
        length = LAN_MINPACKET;
    CHECK( CNET_write_physical_reliable(num, (char *)&packet, &length) );
}

static EVENT_HANDLER(physical_ready) {
    int num;
    size_t len;
    ETHERPACKET f;

    CHECK(CNET_read_physical(&num, (char*) &f, &len));
    memcpy(&b, f.data, LEN_BPDU);

    int i, compare_root_macAddress;
    int compare_root_rootAddress;
    // Configuration bpdu (we do not simulate other frames)
    if (nodetype == BRIDGE) {
        if (bridgestate == ROOT) {
            // Determine if we are still the root bridge
            compare_root_macAddress = compare_mac_addcess(b.macRoot, macAddress);


            if (compare_root_macAddress == 1) {
                // We are no longer the root!
                bridgestate = NORMAL;

                printf("Lost root status - detected better root: mac %d\n attainable on port %d\n", b.macRoot, num);
                printf("Broadcasting new root information on all other ports...\n");

                // Set the root port information
                rootAddress = b.macRoot;
                rootPort = num;

                // Forward a configuration BPDU on all ports (except
                // receiver)
                // Reset root data
                for (i = 1; i <= nodeinfo.nlinks; i++) {
                    if (i != num) {
                        enable_port(i);
                        ports[i].kind = DESIGNATED_PORT;
                        ports[i].rootPathCost = 0;  
                        ports[i].remoteDesignate = -1;

                        BPDU bpdu;
                        bpdu.macRoot = b.macRoot;
                        bpdu.macSender = macAddress;

                        bpdu.numPortSender = i;
                        bpdu.pathCostUpToNow = b.pathCostUpToNow + linkinfo[num].costperbyte;
                        send_bpdu_to_lan(broadcastAddress, i, &bpdu, LEN_BPDU);
                        // ports[i].send(bpdu);
                    } else {
                        enable_port(i);
                        ports[i].kind = ROOT_PORT;
                        ports[i].remoteDesignate = b.macSender;
                        ports[i].rootPathCost = b.pathCostUpToNow + linkinfo[num].costperbyte;
                    }
                }
            } else if (compare_root_macAddress == 0) {
                if (compare_mac_addcess(b.macSender, macAddress) == 0 && compare_mac_addcess(b.macSender, b.macRoot) == 0) {
                    if (b.numPortSender < num) {
                        printf("Disabling port %d: connected to same segment as port %d\n", num, b.numPortSender);
                        disable_port(num);
                    }
                }
            } else {
                // We are still the root
                // In that case, squelch the frame
                printf("Squelching root announcement from bridge mac ");
                printf("%d who thinks mac \n", b.macSender);
                printf("%d is the root (we know root to be mac ", b.macRoot);
                printf("%d).\n", rootAddress);
            }
        } else {
            // We were not the root
            compare_root_rootAddress = compare_mac_addcess(b.macRoot, rootAddress);
            if (compare_root_rootAddress == 1) {
                // There is a new root!
                printf("Updating root info: root now mac ");
                printf("%d attainable on port ", b.macRoot);
                printf("%d at cost \n", num);
                printf("%f", (b.pathCostUpToNow + linkinfo[num].costperbyte));
                printf(" (root was mac %d).\n", rootAddress);
                rootAddress = b.macRoot;
                rootPort = num;

                // Reset all costs on other ports;
                for (i = 1; i <= nodeinfo.nlinks; i++) {
                    if (i != num) {
                        enable_port(i);
                        ports[i].kind = DESIGNATED_PORT;
                        ports[i].rootPathCost = 0;
                        ports[i].remoteDesignate = -1;
                    } else {
                        enable_port(i);
                        ports[i].kind = ROOT_PORT;
                        ports[i].rootPathCost = b.pathCostUpToNow + linkinfo[num].costperbyte;
                        ports[i].remoteDesignate = b.macSender;
                    }
                }

                // Forward a configuration BPDU on all ports (except
                // receiver)
                for (i = 1; i <= nodeinfo.nlinks; i++) {
                    if (i != num) {
                        BPDU bpdu;
                        bpdu.macRoot = b.macRoot;
                        bpdu.macSender = macAddress;
                        bpdu.numPortSender = i;
                        bpdu.pathCostUpToNow = b.pathCostUpToNow + linkinfo[num].costperbyte;
                        send_bpdu_to_lan(broadcastAddress, i, &bpdu, LEN_BPDU);
                    }
                }
            } else if (compare_root_rootAddress == 0) {
                // This case only happens if there is a loop on our network.
                if ( (b.pathCostUpToNow + linkinfo[num].costperbyte) < ports[rootPort].rootPathCost
                        || (b.pathCostUpToNow + linkinfo[num].costperbyte == ports[rootPort].rootPathCost 
                            && compare_mac_addcess(ports[rootPort].remoteDesignate, b.macSender) == -1)) {
                    printf("Found better route to root mac %d:\n", rootAddress);
                    printf(" - Old route on port %d", rootPort);
                    printf(" had cost %f thru mac ", ports[rootPort].rootPathCost);
                    printf("%d\n", ports[rootPort].remoteDesignate);
                    printf(" - New route on port %d has cost ", num);
                    printf("%f", (b.pathCostUpToNow + linkinfo[num].costperbyte));
                    printf(" thru mac %d\n", b.macSender);

                    // Found a better route to the root bridge
                    ports[num].rootPathCost = b.pathCostUpToNow + linkinfo[num].costperbyte;
                    ports[num].remoteDesignate = b.macSender;
                    

                    if (num == rootPort) {
                        // The better route is thru the same port as before
                        // Update the info
                        printf(" - The old and new root ports are the same\n");

                    } else {
                        // The better route is thru another port
                        // Update the info
                        printf(" - The old and new root ports are different\n");
                    }

                    rootPort = num;

                    for (i = 1; i <= nodeinfo.nlinks; i++) {
                        if (i == num) {
                            enable_port(i);
                            printf(" -> Port %d is now the root port\n", i);
                            ports[i].kind = ROOT_PORT;
                        } else {
                            if (ports[i].rootPathCost == 0) {
                                // We are alone for this port
                                enable_port(i);
                                printf(" -> Port %d is designated: there is no other known route to the root for this segment\n", i);
                                ports[i].remoteDesignate = -1;
                                ports[i].kind = DESIGNATED_PORT;
                            } else if ((ports[i].rootPathCost - linkinfo[i].costperbyte) > (b.pathCostUpToNow + linkinfo[num].costperbyte)) {
                                // We are designated for this port
                                enable_port(i);
                                printf(" -> Port %d", i);
                                printf(" is designated: this bridge has the best known route to the root for this\n segment (cost ");
                                printf("%f\n", ports[i].rootPathCost);
                                ports[i].kind = DESIGNATED_PORT;
                                ports[i].remoteDesignate = -1;
                            } else if ((ports[i].rootPathCost - linkinfo[i].costperbyte) == (b.pathCostUpToNow + linkinfo[num].costperbyte)) {
                                if (compare_mac_addcess(ports[i].remoteDesignate, -1) == 0
                                        || compare_mac_addcess(ports[i].remoteDesignate, macAddress) == -1) {
                                    // We are designated for this port
                                    printf(" -> Port %d is designated: this bridge has one of the best known\n route to the root for this segment and a lower mac than the other best known routes\n", i);
                                    enable_port(i);
                                    ports[i].kind = DESIGNATED_PORT;
                                    ports[i].remoteDesignate = -1;
                                } else {
                                    // Tie: we lose and are no longer the
                                    // designate
                                    printf(" -> Port %d is not designated this bridge has one of the best known\n route to the root for this segment but a higher mac than the other best known routes\n", i);
                                    disable_port(i);
                                }
                            } else {
                                printf(" -> Port %d is not designated this bridge has a worse route for this\n segment than the current known best route thru mac %d\n", i, ports[i].remoteDesignate);
                                disable_port(i);
                            }
                        }
                    }

                    // Update children
                    for (i = 1; i <= nodeinfo.nlinks; i++) {
                        if (i != num) {
                            BPDU bpdu;
                            bpdu.macRoot = b.macRoot;
                            bpdu.macSender = macAddress;
                            bpdu.numPortSender = i;
                            bpdu.pathCostUpToNow = ports[rootPort].rootPathCost;
                            send_bpdu_to_lan(broadcastAddress, i, &bpdu, LEN_BPDU);
                        }
                    }

                } else {
                    // The bpdu relayed to us has a higher cost than our
                    // current
                    // root path, but it may come from one of our neighbors
                    // and
                    // indicate it is in a better position than we are to
                    // service a particular segment.
                    if (ports[rootPort].rootPathCost > b.pathCostUpToNow
                            || (ports[rootPort].rootPathCost == b.pathCostUpToNow && compare_mac_addcess(macAddress, b.macSender) == -1)) {
                        if (compare_mac_addcess(ports[num].remoteDesignate, -1) == 0
                                || compare_mac_addcess(ports[num].remoteDesignate, b.macSender) == -1) {
                        ports[num].remoteDesignate = b.macSender;
                            printf("Received indication from mac %d", b.macSender);
                            printf(" that it is a better designate than our previous\n-ly known best route for segments atteched to port %d\n", num);
                        }

                        if (rootPort != num) {
                            printf("Received indication from mac %d", b.macSender);
                            printf("  that it has a better route than ours for segment\n attached to port %d\n", num);
                            disable_port(num);
                        }
                    } else {
                        printf("Received a frame from mac %d", b.macSender);
                        printf(" for current root (mac %d)\n", rootAddress);
                        printf(" with a path cost higher than ours. Not doing anything...\n");
                        if ( (ports[rootPort].rootPathCost - linkinfo[rootPort].costperbyte) < b.pathCostUpToNow) {
                            printf("- Our cost: %f; their cost: %f\n", ports[rootPort].rootPathCost, b.pathCostUpToNow);
                        } else {
                            printf("- Our mac: %d", macAddress);
                            printf("; their mac: %d\n",b.macSender);
                        }
                    }
                }

            } else {
                // This is from a bridge that thinks it is the root
                // Squelch the frame
                printf("Squelching root announcement from bridge mac ");
                printf("%d who thinks mac \n", b.macSender);
                printf("%d is the root (we know root to be mac ", b.macRoot);
                printf("%d).\n", rootAddress);
            }
        }
    }
}

static EVENT_HANDLER(start_STP) {
    int i;
    printf("Status:\n");
    for (int i = 1; i <= nodeinfo.nlinks; ++i)
    {
        switch(ports[i].kind) {
            case ROOT_PORT:
                printf("%d: ROOT\n", i);
                break;
            case  DESIGNATED_PORT:
                printf("%d: DESIGNATED\n", i);
                break;
            case BLOCKED_PORT:
                printf("%d: BLOCKED\n", i);
                break;
            default:
                break;
        }
    }
    for (i = 1; i <= nodeinfo.nlinks; i++) {
            BPDU bpdu;
            bpdu.macRoot = rootAddress;
            bpdu.macSender = macAddress;
            bpdu.numPortSender = i;
            bpdu.pathCostUpToNow = 0;
            send_bpdu_to_lan(broadcastAddress, i, &bpdu, LEN_BPDU);
        }
    CNET_start_timer(EV_TIMER1, TIMEOUT, 0);
}

static EVENT_HANDLER(free_space) {
    if (ports != NULL) {
        free(ports);
    }
}

EVENT_HANDLER(reboot_node) {

    CHECK(CNET_set_handler(EV_PHYSICALREADY, physical_ready, 0));
    CHECK(CNET_set_handler(EV_TIMER1, start_STP, 0));
    CHECK(CNET_set_handler(EV_SHUTDOWN, free_space, 0));

    CNET_parse_nicaddr(broadcastAddress, "ff:ff:ff:ff:ff:ff");

    if (is_host(nodeinfo.nodenumber)) {
        nodetype = HOST;
    }
    else if (is_bridge(nodeinfo.nodenumber)) {
        nodetype = BRIDGE;
        bridgestate = ROOT;

        ports = (PORT*) malloc((nodeinfo.nlinks + 1) * sizeof(PORT));
        int i;

        for (i = 1; i <= nodeinfo.nlinks; ++i)
        {
            ports[i].kind = DESIGNATED_PORT;
            ports[i].rootPathCost = 0;
            ports[i].remoteDesignate = -1;
        }

        macAddress = rootAddress = nodeinfo.address;

        // CNET_srand(nodeinfo.time_of_day.sec + nodeinfo.nodenumber);
        // synchronization_time = SYNCHRONIZATION_TIME;
        // synchronization_time *= ( (double ) CNET_rand() / LONG_MAX );

        printf("MAC Address: %d\n", macAddress);
        CNET_start_timer(EV_TIMER1, TIMEOUT, 0);

    }
    else {
        nodetype = OTHER;
    }
}