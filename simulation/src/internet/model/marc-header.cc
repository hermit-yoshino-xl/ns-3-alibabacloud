/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#include "marc-header.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MarcHeader");

NS_OBJECT_ENSURE_REGISTERED (MarcHeader);

MarcHeader::MarcHeader ()
  : m_prefix (0),
    m_length (0)
{
}

MarcHeader::~MarcHeader ()
{
}

TypeId
MarcHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MarcHeader")
    .SetParent<Header> ()
    .AddConstructor<MarcHeader> ();
  return tid;
}

TypeId
MarcHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
MarcHeader::Print (std::ostream &os) const
{
  os << "prefix=" << m_prefix << " len=" << (uint32_t)m_length;
}

uint32_t
MarcHeader::GetSerializedSize (void) const
{
  // 32-bit prefix + 8-bit length = 5 bytes
  return 4 + 1; 
}

void
MarcHeader::Serialize (Buffer::Iterator start) const
{
  start.WriteHtonU32 (m_prefix); // 网络字节序写入
  start.WriteU8 (m_length);
}

uint32_t
MarcHeader::Deserialize (Buffer::Iterator start)
{
  m_prefix = start.ReadNtohU32 (); // 网络字节序读取
  m_length = start.ReadU8 ();
  return GetSerializedSize ();
}

void
MarcHeader::SetPrefix (uint32_t prefix)
{
  m_prefix = prefix;
}

uint32_t
MarcHeader::GetPrefix (void) const
{
  return m_prefix;
}

void
MarcHeader::SetLength (uint8_t length)
{
  m_length = length;
}

uint8_t
MarcHeader::GetLength (void) const
{
  return m_length;
}

} // namespace ns3
