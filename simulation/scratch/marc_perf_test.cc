/* marc_perf_test.cc - Final Robust Version */
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/marc-header.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("MarcPerfTest");

uint64_t g_receivedBytes = 0;
Time g_startTime;
Time g_lastRxTime;

// 接收回调：从 Socket 读取数据
void ReceivePacket (Ptr<Socket> socket)
{
  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom (from)))
    {
      if (packet->GetSize () > 0)
        {
          g_receivedBytes += packet->GetSize ();
          g_lastRxTime = Simulator::Now ();
        }
    }
}

// 定义分包大小 (1000 字节)，防止超过 MTU 被丢弃
const uint32_t PACKET_SIZE = 1000; 

// ---------------------------------------------------
// 场景 1: 传统单播 (Baseline)
// ---------------------------------------------------
double RunBaselineTest (uint32_t totalDataSize, uint32_t numReceivers)
{
  g_receivedBytes = 0;
  g_lastRxTime = Seconds(0);
  
  // 计算需要分多少个包
  uint32_t numPackets = totalDataSize / PACKET_SIZE;
  if (numPackets == 0) numPackets = 1;

  NodeContainer nodes;
  nodes.Create (numReceivers + 1); // Node 0 是源

  // 100Gbps 链路
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Gbps"));
  csma.SetChannelAttribute ("Delay", StringValue ("1us"));
  NetDeviceContainer devices = csma.Install (nodes);

  InternetStackHelper stack;
  stack.Install (nodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = address.Assign (devices);

  // 接收端安装 Socket
  for (uint32_t i = 1; i <= numReceivers; ++i) {
      Ptr<Socket> sink = Socket::CreateSocket (nodes.Get (i), TypeId::LookupByName ("ns3::UdpSocketFactory"));
      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 8080);
      sink->Bind (local);
      sink->SetRecvCallback (MakeCallback (&ReceivePacket));
  }

  // 发送端
  Ptr<Socket> source = Socket::CreateSocket (nodes.Get (0), TypeId::LookupByName ("ns3::UdpSocketFactory"));
  g_startTime = Seconds (1.0);
  
  // 单播逻辑：对每个接收者，发送所有分片 (N * M 次发送)
  for (uint32_t r = 1; r <= numReceivers; ++r) {
      for (uint32_t p_idx = 0; p_idx < numPackets; ++p_idx) {
          // 错峰发送，模拟序列化延迟
          Time t = Seconds(1.0) + MicroSeconds(1) * (r * numPackets + p_idx);
          Simulator::Schedule (t, [=]() {
              Ptr<Packet> p = Create<Packet> (PACKET_SIZE);
              source->SendTo (p, 0, InetSocketAddress (interfaces.GetAddress (r), 8080));
          });
      }
  }

  Simulator::Run ();
  Simulator::Destroy ();

  if (g_lastRxTime < g_startTime) return 0;
  return (g_lastRxTime - g_startTime).GetSeconds();
}

// ---------------------------------------------------
// 场景 2: MARC 组播 (Our Method)
// ---------------------------------------------------
double RunMarcTest (uint32_t totalDataSize, uint32_t numReceivers)
{
  g_receivedBytes = 0;
  g_lastRxTime = Seconds(0);
  
  uint32_t numPackets = totalDataSize / PACKET_SIZE;
  if (numPackets == 0) numPackets = 1;

  NodeContainer nodes;
  nodes.Create (numReceivers + 1); 

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("100Gbps"));
  csma.SetChannelAttribute ("Delay", StringValue ("1us"));
  NetDeviceContainer devices = csma.Install (nodes);

  InternetStackHelper stack;
  stack.Install (nodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = address.Assign (devices);

  for (uint32_t i = 1; i <= numReceivers; ++i) {
      Ptr<Socket> sink = Socket::CreateSocket (nodes.Get (i), TypeId::LookupByName ("ns3::UdpSocketFactory"));
      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 8080);
      sink->Bind (local);
      sink->SetRecvCallback (MakeCallback (&ReceivePacket));
  }

  Ptr<Socket> source = Socket::CreateSocket (nodes.Get (0), TypeId::LookupByName ("ns3::UdpSocketFactory"));
  g_startTime = Seconds (1.0);

  // MARC 逻辑：只发送 1 轮分片 (1 * M 次发送)
  // 交换机会负责复制给 8 个接收者
  for (uint32_t p_idx = 0; p_idx < numPackets; ++p_idx) {
      Time t = Seconds(1.0) + NanoSeconds(80 * p_idx); // 100G 线速发包
      
      Simulator::Schedule (t, [=]() {
          Ptr<Packet> p = Create<Packet> (PACKET_SIZE);
          
          // 核心：贴上 MARC Header
          MarcHeader h;
          h.SetPrefix(999); 
          h.SetLength(2);
          p->AddHeader(h);

          // 发给任意一个目标，触发底层复制
          source->SendTo (p, 0, InetSocketAddress (interfaces.GetAddress (1), 8080));
      });
  }

  Simulator::Run ();
  Simulator::Destroy ();

  if (g_lastRxTime < g_startTime) return 0;
  return (g_lastRxTime - g_startTime).GetSeconds();
}

int main (int argc, char *argv[])
{
  uint32_t numReceivers = 8; 
  
  // 测试从小到大的包
  std::vector<uint32_t> sizes = {
      64*1024,       // 64KB
      1024*1024,     // 1MB
      4*1024*1024,   // 4MB
      16*1024*1024,  // 16MB
      64*1024*1024   // 64MB
  };

  std::cout << "Size_Bytes,Baseline_Time,MARC_Time" << std::endl;

  for (uint32_t size : sizes) {
      double t_base = RunBaselineTest(size, numReceivers);
      double t_marc = RunMarcTest(size, numReceivers);
      std::cout << size << "," << t_base << "," << t_marc << std::endl;
  }

  return 0;
}
