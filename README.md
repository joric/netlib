netlib
======

This a crazy mess of different network code I wrote over time.

Ideally there should be only 4 public interfaces, all non-blocking, all called in the main thread:

* netOpen("protocol://host:port/info_hash");
* netSend(buf, buf_size)
* netRecv(buf, buf_size)
* netClose()

To filter out traffic one should reconnect with new info_hash.
For example, one location for a lobby, another for a game.
There's only one network connection per app (i.e. a static one).

The library utilizes ip publishing via bittorrent tracker and NAT punch-through via STUN.
Routing traffic using TURN server is in beta stage.

To test icmp client, use `tcpdump -i eth1 -nl icmp`.

STUN servers
------------

* stun.xten.com
* stun1.noc.ams-ix.net
* stun.fwd.org // not working
* stun01.sipphone.com
* stun.ekiga.net (alias for stun01.sipphone.com)
* stun.fwdnet.net (no XOR_MAPPED_ADDRESS support) // not working
* stun.ideasip.com (no XOR_MAPPED_ADDRESS support)
* stun.softjoys.com (no DNS SRV record) (no XOR_MAPPED_ADDRESS support)
* stun.voipbuster.com (no DNS SRV record) (no XOR_MAPPED_ADDRESS support)
* stun.voxgratia.org (no DNS SRV record) (no XOR_MAPPED_ADDRESS support)
* stunserver.org (see their usage policy)
* stun.sipgate.net:10000
* numb.viagenie.ca (http://numb.viagenie.ca) (XOR_MAPPED_ADDRESS only with rfc3489bis magic number in transaction ID)
* stun.ipshka.com inside UA-IX zone russsian explanation at http://www.ipshka.com/main/help/hlp_stun.php 

TURN servers
------------

* numb.viagenie.ca
* does anyone know other servers?


Open Bittorent Trackers (for IP propagation)
--------------------------------------------

* http://tracker.openbittorrent.com/announce
* http://tracker.publicbt.com:80/announce
* http://tracker.istole.it:80/announce
* http://tracker.hexagon.cc:2710/announce
* http://z6gw6skubmo2pj43.onion:8080/announce
* http://z6gw6skubmo2pj43.tor2web.com:8080/announce
* udp://tracker.openbittorrent.com:80/announce
* udp://tracker.publicbt.com:80/announce
* udp://tracker.istole.it:80/announce

