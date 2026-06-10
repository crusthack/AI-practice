#include <array>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

struct ByteView
{
    const std::uint8_t* data;
    std::size_t length;

    ByteView(const std::uint8_t* bytes, std::size_t byteCount)
        : data(bytes), length(byteCount)
    {
    }

    const std::uint8_t& operator[](std::size_t index) const
    {
        return data[index];
    }

    std::size_t size() const
    {
        return length;
    }
};

std::uint16_t Read16(ByteView bytes, std::size_t offset)
{
    return static_cast<std::uint16_t>((bytes[offset] << 8) | bytes[offset + 1]);
}

std::string MacToString(ByteView bytes, std::size_t offset)
{
    std::ostringstream out;
    out << std::hex << std::setfill('0');
    for (std::size_t i = 0; i < 6; ++i)
    {
        if (i != 0)
            out << ':';
        out << std::setw(2) << static_cast<int>(bytes[offset + i]);
    }
    return out.str();
}

std::string Ipv4ToString(ByteView bytes, std::size_t offset)
{
    std::ostringstream out;
    out << static_cast<int>(bytes[offset]) << '.'
        << static_cast<int>(bytes[offset + 1]) << '.'
        << static_cast<int>(bytes[offset + 2]) << '.'
        << static_cast<int>(bytes[offset + 3]);
    return out.str();
}

std::string EtherTypeName(std::uint16_t etherType)
{
    switch (etherType)
    {
    case 0x0800:
        return "IPv4";
    case 0x0806:
        return "ARP";
    case 0x86DD:
        return "IPv6";
    default:
        return "unknown";
    }
}

std::string ArpOperationName(std::uint16_t operation)
{
    switch (operation)
    {
    case 1:
        return "request";
    case 2:
        return "reply";
    default:
        return "unknown";
    }
}

std::string DestinationKind(ByteView frame)
{
    bool broadcast = true;
    for (std::size_t i = 0; i < 6; ++i)
        broadcast = broadcast && frame[i] == 0xff;

    if (broadcast)
        return "broadcast";
    if ((frame[0] & 0x01) != 0)
        return "multicast";
    return "unicast";
}

void PrintHexdump(ByteView bytes)
{
    for (std::size_t i = 0; i < bytes.size(); i += 16)
    {
        std::cout << "  " << std::hex << std::setw(4) << std::setfill('0') << i << "  ";
        for (std::size_t j = 0; j < 16; ++j)
        {
            if (i + j < bytes.size())
                std::cout << std::setw(2) << static_cast<int>(bytes[i + j]) << ' ';
            else
                std::cout << "   ";
        }
        std::cout << std::dec << '\n';
    }
}

void ParseArp(ByteView frame)
{
    constexpr std::size_t arp = 14;
    if (frame.size() < arp + 28)
    {
        std::cout << "ARP payload is too short.\n";
        return;
    }

    const std::uint16_t hardwareType = Read16(frame, arp + 0);
    const std::uint16_t protocolType = Read16(frame, arp + 2);
    const std::uint8_t hardwareLength = frame[arp + 4];
    const std::uint8_t protocolLength = frame[arp + 5];
    const std::uint16_t operation = Read16(frame, arp + 6);

    std::cout << "ARP\n";
    std::cout << "  hardware type : " << hardwareType << " (Ethernet is 1)\n";
    std::cout << "  protocol type : 0x" << std::hex << protocolType << std::dec << " (IPv4 is 0x0800)\n";
    std::cout << "  address sizes : MAC " << static_cast<int>(hardwareLength)
              << " bytes, IPv4 " << static_cast<int>(protocolLength) << " bytes\n";
    std::cout << "  operation     : " << operation << ' ' << ArpOperationName(operation) << '\n';
    std::cout << "  sender        : " << MacToString(frame, arp + 8)
              << " / " << Ipv4ToString(frame, arp + 14) << '\n';
    std::cout << "  target        : " << MacToString(frame, arp + 18)
              << " / " << Ipv4ToString(frame, arp + 24) << '\n';

    if (operation == 1)
        std::cout << "  meaning       : sender asks who owns target IP on this local link\n";
    else if (operation == 2)
        std::cout << "  meaning       : sender announces the MAC address for its IPv4 address\n";
}

void ParseIpv4(ByteView frame)
{
    constexpr std::size_t ip = 14;
    if (frame.size() < ip + 20)
    {
        std::cout << "IPv4 payload is too short.\n";
        return;
    }

    const std::uint8_t version = frame[ip] >> 4;
    const std::uint8_t ihlBytes = static_cast<std::uint8_t>((frame[ip] & 0x0f) * 4);
    const std::uint16_t totalLength = Read16(frame, ip + 2);
    const std::uint8_t ttl = frame[ip + 8];
    const std::uint8_t protocol = frame[ip + 9];

    std::cout << "IPv4\n";
    std::cout << "  version       : " << static_cast<int>(version) << '\n';
    std::cout << "  header length : " << static_cast<int>(ihlBytes) << " bytes\n";
    std::cout << "  total length  : " << totalLength << " bytes\n";
    std::cout << "  ttl           : " << static_cast<int>(ttl) << '\n';
    std::cout << "  protocol      : " << static_cast<int>(protocol)
              << (protocol == 1 ? " ICMP" : protocol == 6 ? " TCP" : protocol == 17 ? " UDP" : "") << '\n';
    std::cout << "  source        : " << Ipv4ToString(frame, ip + 12) << '\n';
    std::cout << "  destination   : " << Ipv4ToString(frame, ip + 16) << '\n';
}

void ParseEthernetFrame(std::string_view title, ByteView frame)
{
    std::cout << "== " << title << " ==\n";
    PrintHexdump(frame);

    if (frame.size() < 14)
    {
        std::cout << "Frame is too short for Ethernet II.\n\n";
        return;
    }

    const std::uint16_t etherType = Read16(frame, 12);
    std::cout << "Ethernet II\n";
    std::cout << "  destination   : " << MacToString(frame, 0) << " (" << DestinationKind(frame) << ")\n";
    std::cout << "  source        : " << MacToString(frame, 6) << '\n';
    std::cout << "  ether type    : 0x" << std::hex << etherType << std::dec
              << " (" << EtherTypeName(etherType) << ")\n";
    std::cout << "  payload bytes : " << (frame.size() - 14) << "\n\n";

    if (etherType == 0x0806)
        ParseArp(frame);
    else if (etherType == 0x0800)
        ParseIpv4(frame);

    std::cout << '\n';
}

int main()
{
    std::cout << "Data Link Lab: Ethernet II and ARP\n\n";

    const std::array<std::uint8_t, 42> arpRequest = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x10, 0x22, 0x33, 0x44, 0x55, 0x66,
        0x08, 0x06,
        0x00, 0x01,
        0x08, 0x00,
        0x06,
        0x04,
        0x00, 0x01,
        0x10, 0x22, 0x33, 0x44, 0x55, 0x66,
        0xc0, 0xa8, 0x00, 0x0a,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xc0, 0xa8, 0x00, 0x01
    };

    const std::array<std::uint8_t, 42> arpReply = {
        0x10, 0x22, 0x33, 0x44, 0x55, 0x66,
        0x24, 0x6e, 0x96, 0xaa, 0xbb, 0xcc,
        0x08, 0x06,
        0x00, 0x01,
        0x08, 0x00,
        0x06,
        0x04,
        0x00, 0x02,
        0x24, 0x6e, 0x96, 0xaa, 0xbb, 0xcc,
        0xc0, 0xa8, 0x00, 0x01,
        0x10, 0x22, 0x33, 0x44, 0x55, 0x66,
        0xc0, 0xa8, 0x00, 0x0a
    };

    const std::array<std::uint8_t, 34> ipv4Packet = {
        0x24, 0x6e, 0x96, 0xaa, 0xbb, 0xcc,
        0x10, 0x22, 0x33, 0x44, 0x55, 0x66,
        0x08, 0x00,
        0x45, 0x00,
        0x00, 0x14,
        0x12, 0x34,
        0x40, 0x00,
        0x40,
        0x01,
        0x00, 0x00,
        0xc0, 0xa8, 0x00, 0x0a,
        0x08, 0x08, 0x08, 0x08
    };

    ParseEthernetFrame("ARP request: who has 192.168.0.1?", ByteView(arpRequest.data(), arpRequest.size()));
    ParseEthernetFrame("ARP reply: 192.168.0.1 is at 24:6e:96:aa:bb:cc", ByteView(arpReply.data(), arpReply.size()));
    ParseEthernetFrame("IPv4 packet inside Ethernet", ByteView(ipv4Packet.data(), ipv4Packet.size()));

    std::cout << "Learning points\n";
    std::cout << "  - Ethernet uses MAC addresses on the local link.\n";
    std::cout << "  - EtherType selects the next protocol: ARP, IPv4, IPv6, and so on.\n";
    std::cout << "  - ARP maps an IPv4 address to a MAC address before local delivery.\n";
    std::cout << "  - IP routing starts after the Ethernet frame payload is interpreted as IPv4.\n";
}
