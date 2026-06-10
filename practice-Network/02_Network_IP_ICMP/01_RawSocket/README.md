# 01_RawSocket

Observe raw socket behavior and IP/ICMP packet boundaries.

## Run / Build

`powershell
MSBuild 02_Network_IP_ICMP\01_RawSocket\01_RawSocket.vcxproj /p:Configuration=Debug /p:Platform=x64
`

If the program supports modes, run the server/receiver first and then run the client/sender in another console.

## Observe

- Which OSI/TCP-IP layer is being practiced.
- Which header fields are read or generated.
- Where the payload starts.
- Which identifiers are used: MAC, IP, port, host, path.
- What guarantee is provided by the protocol and what is left to the application.

## Completion Criteria

- Explain the main protocol fields without looking at the code.
- Explain the expected packet or message flow.
- Compare the output with Wireshark when the example uses real network traffic.
- Describe one failure case and how the code should react.

## Extensions

- Add stricter input validation.
- Add clearer logging for header fields.
- Add one malformed packet/message test case.