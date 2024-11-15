# Ground Station - Satelite comm

The setup illustrates the usage of a redundant CAN bus network with multiple modules, a gateway, and a ground station. The network is designed to ensure reliable communication by using two separate CAN buses (CAN Bus 0 and CAN Bus 1), providing redundancy in case one of the buses fails.

<link rel="stylesheet" type="text/css" href="./_static/css/custom.css" media="screen" />
<div  class='img-container'>
<img src="./_static/gs_gw_bus.png" class='img-source'>
</div>
<br>
<br>

## Components and Network Topology:

### Ground Station:

The ground station communicates with the network via RF0.
 * Address: 1
 * Netmask: 12
 * Routing: 0/0 RF0 (indicating that all traffic from the ground station is routed through RF0).

### Gateway:
The gateway acts as an intermediary between the ground station and the CAN bus network.
It has two interfaces:

**RF1**: Communicates with the ground station.
 * Address: 2
 * Netmask: 12

**CAN0 and CAN1:** These are the gateway's interfaces to the CAN buses.
 * Address (on both CAN0 and CAN1): 129
 * Netmask: 8

### CAN Buses:
There are two CAN buses in this network:
 * CAN Bus 0 (yellow lines)
 * CAN Bus 1 (green lines)

These buses provide redundant communication paths between the gateway and the modules.

### Modules (Module 0, Module 1, Module 2):
Each module has two CAN interfaces connected to both CAN Bus 0 and CAN Bus 1 for redundancy.
Each module has its own address and routing configuration.

**Module 0:** Connected to both CAN Bus 0 and CAN Bus 1 via CAN2 and CAN3, respectively.
 * Address on CAN2/CAN3: 130
 * Netmask: 8
 * Routing: Traffic is routed through both buses (1/12).

**Module 1:** Connected to both buses via CAN3 (Bus 0) and CAN4 (Bus 1).
 * Address on CAN3/CAN4: 131
 * Netmask: 8
 * Routing: Similar to Module 0, traffic is routed through both buses (1/12).

**Module 2:** Connected via CAN5 (Bus 0) and CAN6 (Bus 1).
 * Address on CAN5/CAN6: 132
 * Netmask: 8
 * Routing: Traffic is routed through both buses (1/12).

The network uses two separate CAN buses to provide redundancy. If one bus fails, communication can continue over the other bus.
Each module has two interfaces connected to different buses, ensuring that they can communicate even if one of the buses is down. The gateway connects the ground station to the redundant CAN bus system, allowing external communication with the modules.

## Bus redundancy and deduplication

### Bus redundancy

The redundancy ensures that if one of the CAN buses fails (due to a physical fault or other issues), the communication can continue uninterrupted via the other bus. When a packet is sent from the Ground Station to a specific module (e.g., Module 0), the packet is forwarded by the Gateway over both CAN Bus 0 and CAN Bus 1 simultaneously. This means that:

* The packet travels over both buses, reaching the destination through two separate paths.
* In this case, Module 0 receives the packet on both its interfaces (CAN2 and CAN3), ensuring that at least one copy of the packet will be delivered even if one of the buses fails.

### Packet Deduplication

Since the packet is transmitted over both CAN buses, it will arrive at the destination node twice—once from each bus. To avoid processing the same packet multiple times, a packet deduplication mechanism is employed at the receiving node (e.g., Module 0). Here’s how deduplication works:

* When Module 0 receives the first copy of the packet from CAN Bus 0 via CAN2 it calculates its crc32 unique identifier
and processes it.
* When it receives the second copy of the same packet from CAN Bus 1 via CAN3, it recognizes that this packet has already been processed (based on its crc32 checksum) and discards it.

<div  class='img-container'>
<img src="./_static/packet_send.gif" class='img-source'>
</div>
<br>
<br>

