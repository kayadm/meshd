# meshd
Using Bluetooth mesh stack POC for Linux for find location of Bluetooth Device.
In development...

dependencies:
- Gtk
- lbluetooth
- glib
- Request (for now) crypto kernel UAPI with following config:
CRYPTO_USER_API_SKCIPHER, CRYPTO_ECB
CRYPTO_USER_API_HASH, CRYPTO_CMAC
CRYPTO_USER_API_AEAD, CRYPTO_CCM


build meshd:
- make

Basic interactive interface for now

1. run meshd on device 1
- sudo ./meshd -i

2. run meshd on device 2
- sudo ./meshd -i

3. Create network on device 1
- net-create

4. Scan unprovisioned nodes on device 1 and  Provision discovered node with address
- scan on

5. Wait for provisioning complete
- "Wait For Release Message"

6. Send Message to Nodes in Network
- net-send  "0xNid-Number"  "Destination-Address" "data"
- example:  net-send  0x5b  0x1234  hello

7. Special Message Formats:
- data= 00,"Device-Name-N1",  =>Request for RSSI value of device N1
- Response of  Message=>   01,"SourceAddressN2","RSSI",=> RSSI value of N1 from N2 to all nodes.
- EXAMPLE:  net-send  0x5b  0x1234  00,SamsungS7,
