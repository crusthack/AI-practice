# Frame Layout Notes

## Ethernet II

```text
0               5 6              11 12    13 14 ...
+----------------+----------------+--------+----------------
| Destination MAC | Source MAC     | Type   | Payload
+----------------+----------------+--------+----------------
| 6 bytes         | 6 bytes        | 2 bytes| 46-1500 bytes
```

이 실습은 FCS(Frame Check Sequence) 4바이트를 다루지 않습니다. 일반적인 packet capture 도구도 OS/NIC 처리 이후의 frame을 보여주기 때문에 FCS가 빠져 있는 경우가 많습니다.

## EtherType

| 값 | 의미 |
| --- | --- |
| `0x0800` | IPv4 |
| `0x0806` | ARP |
| `0x86DD` | IPv6 |

## ARP for Ethernet/IPv4

```text
0      1 2      3 4 5 6      7 8             13 14         17 18            23 24         27
+--------+--------+-+-+--------+----------------+-------------+---------------+-------------+
| HTYPE  | PTYPE  |H|P| OPER   | Sender MAC     | Sender IP   | Target MAC    | Target IP   |
+--------+--------+-+-+--------+----------------+-------------+---------------+-------------+
| 2 bytes| 2 bytes|1|1| 2 bytes| 6 bytes        | 4 bytes     | 6 bytes       | 4 bytes     |
```

Ethernet/IPv4 ARP에서 일반적으로 사용되는 값은 다음과 같습니다.

| 필드 | 값 | 의미 |
| --- | --- | --- |
| HTYPE | `1` | Ethernet |
| PTYPE | `0x0800` | IPv4 |
| HLEN | `6` | MAC address length |
| PLEN | `4` | IPv4 address length |
| OPER | `1` | request |
| OPER | `2` | reply |

## 주소 구분

| 주소 | 계층 | 범위 |
| --- | --- | --- |
| MAC address | Data Link | 같은 local link 안에서 frame 전달 |
| IP address | Network | 다른 network까지 포함한 host 식별 |
| Port number | Transport | host 안의 process/application 식별 |

ARP는 IPv4 주소를 알고 있을 때 같은 local link에서 사용할 MAC 주소를 찾기 위한 프로토콜입니다.
