1. When programming each Slave (signal modules) it is important to change both the "location" and "ip" variable in order for the module to behave as the part you wish it to be.

2. Keep in mid that there is only one remote but four slave modules. There is no configuration required for the remote. ONLY FOR SLAVE MODULES.

3. IP/Location table.

Module Type	Location	location_var	IP
-----------------------------------------------------------
SLAVE		LeftFront	1		192.168.0.1
SLAVE		RightFront	2		192.168.0.2
SLAVE		RightRear	3		192.168.0.3
SLAVE		LeftRear	4		192.168.0.4
MASTER		Remote		5		192.168.0.5
