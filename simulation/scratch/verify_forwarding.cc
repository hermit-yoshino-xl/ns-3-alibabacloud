/* verify_forwarding.cc */
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"      // <--- 1. Use CSMA module instead of P2P
#include "ns3/marc-header.h"      // Include MARC header

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("VerifyForwarding");

int main (int argc, char *argv[])
{
  // 1. Create 3 nodes (source -> router -> destination)
  NodeContainer nodes;
  nodes.Create (3);

  // 2. Create physical links (use CSMA Ethernet to avoid P2P dependencies)
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("10Gbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

  // For CSMA we usually have a bus, but by installing CSMA on only two nodes
  // per container, we effectively emulate point-to-point links.

  // Link n0 <-> n1
  NodeContainer n0n1 = NodeContainer (nodes.Get (0), nodes.Get (1));
  NetDeviceContainer d0d1 = csma.Install (n0n1);

  // Link n1 <-> n2
  NodeContainer n1n2 = NodeContainer (nodes.Get (1), nodes.Get (2));
  NetDeviceContainer d1d2 = csma.Install (n1n2);

  // 3. Install Internet stack
  InternetStackHelper stack;
  stack.Install (nodes);

  // 4. Assign IP addresses
  Ipv4AddressHelper address;
  
  // Subnet A: 10.1.1.0/24
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i0i1 = address.Assign (d0d1);

  // Subnet B: 10.1.2.0/24
  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer i1i2 = address.Assign (d1d2);

  // 5. Populate global routing tables
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  // 6. Construct a MARC packet
  Ptr<Packet> packet = Create<Packet> (1024); 
  
  MarcHeader marc;
  marc.SetPrefix (999);      // Example prefix ID
  marc.SetLength (2);        // Example length
  packet->AddHeader (marc);  // Attach MARC header

  // 7. Send the packet
  // Send from n0 (10.1.1.1) to n2 (10.1.2.2)
  Ptr<Ipv4> ipv4_n0 = nodes.Get (0)->GetObject<Ipv4> ();
  Ipv4Address dest_addr = i1i2.GetAddress (1); 
  
  std::cout << "--------------------------------------------------------" << std::endl;
  std::cout << "Sending MARC Packet via CSMA..." << std::endl;
  std::cout << "Expect to see [MARC-SWITCH] log from Node 1." << std::endl;
  std::cout << "--------------------------------------------------------" << std::endl;

  // Here protocol number 0 is the one we fixed earlier
  ipv4_n0->Send (packet, "10.1.1.1", dest_addr, 0, 0);

  // 8. Run simulation
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}

