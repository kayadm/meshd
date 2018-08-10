# meshd
Meshd is a program where we use the functionality of Bluetooth Mesh Network to increase the communication distance of a Bluetooth device. Plus it can provide the location of the bluetooth device.
In development...

dependencies:
- lbluetooth
- glib
- Request (for now) crypto kernel UAPI with following config:
CRYPTO_USER_API_SKCIPHER, CRYPTO_ECB
CRYPTO_USER_API_HASH, CRYPTO_CMAC
CRYPTO_USER_API_AEAD, CRYPTO_CCM


build meshd:
- make 
(in the meshd directory)

Basic interactive interface

1. run the app on device 1
- sudo ./meshd -i

2. run the app on device 2
- sudo ./meshd -i

3. Create network on device 1
- net-create

4. Scan for unprovisioned nodes on device 1 and  auto-provision discovered node with random address
- scan on

5. Waiting for provisioning complete
- No command "Wait For Release Message"

6. Send Message to Nodes in Network
- net-send  "0xNid-Number"  "Destination-Address" "data"
- example:  net-send  0x5b  0x1234  hello

7. Special Message Formats:
- data= 00,"Device-Name-N1",  =>Request for RSSI value of device name N1
- Response of  Message=>   01,"SourceAddressN2","RSSI",=> RSSI value of N1 from N2 to all nodes.
- EXAMPLE:  net-send  0x5b  0x1234  00,SamsungS7,
