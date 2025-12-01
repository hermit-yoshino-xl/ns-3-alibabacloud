/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef MARC_HEADER_H
#define MARC_HEADER_H

#include "ns3/header.h"

namespace ns3 {

/**
 * @brief MARC (Multicast Adaptive Route Constructor) Packet Header
 * * 用于携带组播树的覆盖前缀信息。
 * Format:
 * |   Prefix (32 bits)   | Length (8 bits) | Padding (optional/virtual) |
 */
class MarcHeader : public Header
{
public:
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

  MarcHeader ();
  virtual ~MarcHeader ();

  // Setters
  void SetPrefix (uint32_t prefix);
  void SetLength (uint8_t length);

  // Getters
  uint32_t GetPrefix (void) const;
  uint8_t GetLength (void) const;

private:
  uint32_t m_prefix; // The multicast group prefix ID
  uint8_t m_length;  // The power-of-two length (0 to 32)
};

} // namespace ns3

#endif /* MARC_HEADER_H */
