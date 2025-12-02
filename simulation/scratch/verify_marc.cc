#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/marc-header.h" 

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("VerifyMarc");

int main (int argc, char *argv[])
{
  // 1. Create a new identity
  MarcHeader marc;
  
  // 2. Set data 
  uint32_t testPrefix = 12345; // Assume a prefix ID
  uint8_t testLength = 3;      // Assume length (2^3)
  
  marc.SetPrefix (testPrefix);
  marc.SetLength (testLength);
  
  // 3. Print and check (call the Print function we implemented in marc-header.cc)
  std::cout << "Testing MarcHeader..." << std::endl;
  std::cout << "Expected: prefix=12345 len=3" << std::endl;
  std::cout << "Actual:   ";
  marc.Print (std::cout); 
  std::cout << std::endl;

  // 4. Verify serialized size (should be 4 + 1 = 5 bytes)
  std::cout << "Serialized Size: " << marc.GetSerializedSize () << " bytes (Expected: 5)" << std::endl;

  return 0;
}

