#pragma once
#include "tsCommon.h"
#include <string>
/*
MPEG-TS packet:
`        3                   2                   1                   0  `
`      1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0  `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `
`   0 |                             Header                            | `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `
`   4 |                  Adaptation field + Payload                   | `
`     |                                                               | `
` 184 |                                                               | `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `
MPEG-TS packet header:
`        3                   2                   1                   0  `
`      1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0  `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `
`   0 |       SB      |E|S|T|           PID           |TSC|AFC|   CC  | `
`     +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ `
Sync byte                    (SB ) :  8 bits
Transport error indicator    (E  ) :  1 bit
Payload unit start indicator (S  ) :  1 bit
Transport priority           (T  ) :  1 bit
Packet Identifier            (PID) : 13 bits
Transport scrambling control (TSC) :  2 bits
Adaptation field control     (AFC) :  2 bits
 Continuity counter           (CC ) :  4 bits
*/
//=============================================================================================================================================================================
class xTS
{
public:
  static constexpr uint32_t TS_PacketLength = 188;
  static constexpr uint32_t TS_HeaderLength = 4;
    
  static constexpr uint32_t PES_HeaderLength = 6;

  static constexpr uint32_t BaseClockFrequency_Hz         =    90000; //Hz
  static constexpr uint32_t ExtendedClockFrequency_Hz     = 27000000; //Hz
  static constexpr uint32_t BaseClockFrequency_kHz        =       90; //kHz
  static constexpr uint32_t ExtendedClockFrequency_kHz    =    27000; //kHz
  static constexpr uint32_t BaseToExtendedClockMultiplier =      300;
};
//=============================================================================================================================================================================
class xTS_PacketHeader
{
public:
    enum class ePID : uint16_t
    {
        PAT  = 0x0000,
        CAT  = 0x0001,
        TSDT = 0x0002,
        IPMT = 0x0003,
        NIT  = 0x0010, //DVB specific PID
        SDT  = 0x0011, //DVB specific PID
        NuLL = 0x1FFF,
    };
protected:
    uint32_t header;
    uint8_t SB;
    uint8_t E;
    uint8_t S;
    uint8_t T;
    uint16_t PID;
    uint8_t TSC;
    uint8_t AFC;
    uint8_t CC;
public:
    void     Reset();
    int32_t  Parse(const uint8_t* Input);
    void     Print() const;
public:
  //TODO
    bool hasAdaptationField() const;
    uint8_t getAdaptionField() const { return this->AFC; }
    uint8_t getSyncByte() const { return this->SB; }
    uint16_t getPID() const { return this->PID; }
    uint8_t getPayloadUnitStartIndicator() const { return this->S; }
    uint8_t getContinuityCounter() const { return this->CC; }
    //bool     hasPayload        () const { /*TODO*/ }
};
//=============================================================================================================================================================================
class xTS_AdaptationField
{
protected:
    //AF length
    uint8_t AF;
    uint8_t Stuffing;
    //mandatory fields
    uint8_t AFC;
    uint8_t DC;
    uint8_t RA;
    uint8_t SP;
    uint8_t PCR;
    uint8_t OR;
    uint8_t SF;
    uint8_t TP;
    uint8_t EX;

    uint64_t PCR_base;
    uint8_t PCR_reserved;
    uint16_t PCR_extension;

    uint64_t OPCR_base;
    uint8_t OPCR_reserved;
    uint16_t OPCR_extension;
public:
    void Reset();
    int32_t Parse(const uint8_t* Input, uint8_t AdaptationFieldControl);
    void Print() const;
public:
    //derrived values
    uint32_t getNumBytes() const {
        return (!AF)? 0 : AF + 1;
    }
};
//=============================================================================================================================================================================
class xPES_PacketHeader
{
public:
    enum eStreamId : uint8_t
    {
        eStreamId_program_stream_map = 0xBC,
        eStreamId_padding_stream = 0xBE,
        eStreamId_private_stream_2 = 0xBF,
        eStreamId_ECM = 0xF0,
        eStreamId_EMM = 0xF1,
        eStreamId_program_stream_directory = 0xFF,
        eStreamId_DSMCC_stream = 0xF2,
        eStreamId_ITUT_H222_1_type_E = 0xF8,
    };
protected:
    //PES packet header
    uint32_t m_PacketStartCodePrefix;
    uint8_t m_StreamId;
    uint16_t m_PacketLength;
    uint8_t m_HeaderLength;

    uint8_t PTS_DTS_flags;
    uint64_t PTS;
    uint64_t DTS;
public:
    void Reset();
    int32_t Parse(const uint8_t* Input);
    //int32_t Parse(const std::vector<uint32_t>& Input);
    void Print() const;
public:
    //PES packet header
    uint32_t getPacketStartCodePrefix() const { return m_PacketStartCodePrefix; }
    uint8_t getStreamId() const { return m_StreamId; }
    uint16_t getPacketLength() const { return m_PacketLength; }
    uint8_t getHeaderLength() const { return m_HeaderLength;  }
};
//=============================================================================================================================================================================
class xPES_Assembler
{
public:
    enum class eResult : int32_t
    {
        UnexpectedPID = 1,
        StreamPackedLost,
        AssemblingStarted,
        AssemblingContinue,
        AssemblingFinished,
        BufferFull,
    };
protected:
    //setup
    int32_t m_PID;
    //buffer
    uint8_t* m_Buffer;
    uint32_t m_BufferSize;
    //operation
    int8_t m_LastContinuityCounter;
    bool m_Started;
    xPES_PacketHeader m_PESH;
public:
    xPES_Assembler();
    ~xPES_Assembler();
    void Init(int32_t PID);
    eResult AbsorbPacket(const uint8_t* TransportStreamPacket, const xTS_PacketHeader* PacketHeader, const xTS_AdaptationField* AdaptationField);
    void PrintPESH() const { m_PESH.Print(); }
    uint8_t* getPacket() { return m_Buffer; }
    int32_t getNumPacketBytes() const { return m_BufferSize; }
    uint8_t getHeaderLenght() const { return m_PESH.getHeaderLength(); }
    void assemblerPes(const uint8_t* TS_PacketBuffer, const xTS_PacketHeader* TS_PacketHeader, const xTS_AdaptationField* TS_AdaptationField, FILE* AudioMP2);
    void saveBufferToFile(FILE* AudioMP2);
protected:
    void xBufferReset();
    void xBufferAppend(const uint8_t* data, uint32_t size);
    
};
//=============================================================================================================================================================================
