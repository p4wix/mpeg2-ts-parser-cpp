#include "tsTransportStream.h"


//=============================================================================================================================================================================
// xTS_PacketHeader
//=============================================================================================================================================================================
void xTS_PacketHeader::Reset() {
    header, SB, E, S, T, PID, TSC, AFC, CC = 0;
}

int32_t xTS_PacketHeader::Parse(const uint8_t* Input) {
    SB = Input[0];                                                        //maski
    E = (Input[1] & 0x80) ? 1 : 0;                                        //128   10000000
    S = (Input[1] & 0x40) ? 1 : 0;                                        //64    01000000
    T = (Input[1] & 0x20) ? 1 : 0;                                        //32    00100000
    PID = From8To16((Input[1] & 0x1F), Input[2]);                         //31    00011111 + inp 2
    TSC = (Input[3] & 0xC0) >> 6;                                         //192   11000000 (6)>> 3 11
    AFC = (Input[3] & 0x30) >> 4;                                         //48    00110000 (4)>> 3 11
    CC = Input[3] & 0xF;                                                  //15    00001111
    return header = From8To32(Input[0], Input[1], Input[2], Input[3]);   // inp0 + inp1 + inp2 + inp3
    
}
void xTS_PacketHeader::Print() const {
    printf("TS: SB=%d E=%d S=%d T=%d PID=%d TSC=%d AFC=%d CC=%d", SB, E, S, T, PID, TSC, AFC, CC);
}

bool xTS_PacketHeader::hasAdaptationField() const {
    return ((AFC == 2) || (AFC == 3))? true : false;
}

//=============================================================================================================================================================================
// xTS_PacketHeader
//=============================================================================================================================================================================
void xTS_AdaptationField::Reset() {
    AFC, AF, DC, RA, SP, PCR, OR, SF, TP, EX = 0;
}

int32_t xTS_AdaptationField::Parse(const uint8_t* Input, uint8_t AdaptationFieldControl) {
    
    AFC = AdaptationFieldControl;
    if (AFC == 2 || AFC == 3) {
        AF = Input[4];
        Stuffing = AF - 1;
    }
    else {
        AF = 0;
        Stuffing = 0;
    }
    DC = (Input[5] & 0x80) >> 7;
    RA = (Input[5] & 0x40) >> 6;
    SP = (Input[5] & 0x20) >> 5;
    PCR = (Input[5] & 0x10) >> 4;
    OR = (Input[5] & 0x8) >> 3;
    SF = (Input[5] & 0x4) >> 2;
    TP = (Input[5] & 0x2) >> 1;
    EX = Input[5] & 0x1;

    int pointer = 6;
    if (PCR) {
        PCR = From8To64(0,0,0, Input[pointer], Input[++pointer], Input[++pointer], Input[++pointer], Input[++pointer]);
        PCR_base >>= 7;

        PCR_reserved = (Input[pointer] & 0x7E);
        PCR_reserved >>= 1;
        PCR_extension = From8To16((Input[pointer] & 0x1),Input[++pointer]);
        Stuffing -= 6;
    }
    if (OR) {
        OPCR_base = From8To64(0, 0, 0, Input[pointer], Input[++pointer], Input[++pointer], Input[++pointer], Input[++pointer]);
        OPCR_base >>= 7;

        OPCR_reserved = (Input[pointer] & 0x7E);
        OPCR_reserved >>= 1;
        OPCR_extension = From8To16((Input[pointer] & 0x1), Input[++pointer]);
        Stuffing -= 6;
    }
    return From8To32(AF, Input[5]);
}

void xTS_AdaptationField::Print() const {
    printf("AF: AF=%d DC=%d RA=%d SP=%d PR=%d OR=%d SF=%d TP=%d EX=%d\n",
        AF, DC, RA, SP,
        PCR, OR, SF, TP, TP);


    if(PCR){ 
        printf("\n\n\nPCR=%d ", (PCR_base * xTS::BaseToExtendedClockMultiplier + PCR_extension)); }
    if (OR) { 
        printf("\n\nOPCR=%d ", (OPCR_base * xTS::BaseToExtendedClockMultiplier + OPCR_extension)); }
}

//=============================================================================================================================================================================
// xPES_PacketHeader
//=============================================================================================================================================================================
void xPES_PacketHeader::Reset() {
    m_PacketStartCodePrefix, m_StreamId, m_PacketLength, 
        PTS_DTS_flags = 0, PTS, DTS = 0;
}

int32_t xPES_PacketHeader::Parse(const uint8_t* Input) {
    m_PacketStartCodePrefix = From8To24(Input[0], Input[1], Input[2]);
    m_StreamId = Input[3];
    m_PacketLength = From8To16(Input[4], Input[5]);
    if (!m_HeaderLength) { m_HeaderLength = 6; }
    if (m_StreamId == 0xBD || (m_StreamId >= 0xC0 && m_StreamId <= 0xEF)) {
            m_HeaderLength = 9;
        
        PTS_DTS_flags = (Input[7] & 0xC0) >> 6;

        if ((Input[7] & 0xC0) == 0xC0) {
            m_HeaderLength += 10;

            PTS = Input[9] & 0xE;
            PTS <<= 7;
            PTS += Input[10];
            PTS <<= 8;
            PTS += Input[11] & 0xFE;
            PTS <<= 7;
            PTS += Input[12];
            PTS <<= 7;
            PTS += Input[13] >> 1;


            DTS = Input[14] & 0xE;
            DTS <<= 7;
            DTS += Input[15];
            DTS <<= 8;
            DTS += Input[16] & 0xFE;
            DTS <<= 7;
            DTS += Input[17];
            DTS <<= 7;
            DTS += Input[18] >> 1;
        }
        else if ((Input[7] & 0x80) == 0x80) {
            m_HeaderLength += 5;

            PTS = Input[9] & 0xE;
            PTS <<= 7;
            PTS += Input[10];
            PTS <<= 8;
            PTS += Input[11] & 0xFE;
            PTS <<= 7;
            PTS += Input[12];
            PTS <<= 7;
            PTS += Input[13] >> 1;

        }

        //ESCR_flag == '1'
        if (Input[7] & 0x20) {
            m_HeaderLength += 6;
        }
        //ES_rate_flag == '1'
        if (Input[7] & 0x10) {
            m_HeaderLength += 3;
        }
        //DSM_trick_mode_flag == '1'
        if (Input[7] & 0x8) {
            m_HeaderLength += 0;
        }
        //additional_copy_info_flag == '1'
        if (Input[7] & 0x4) {
            m_HeaderLength += 1;
        }
        //PES_CRC_flag == '1')
        if (Input[7] & 0x2) {
            m_HeaderLength += 2;
        }
        //PES_extension_flag == '1'
        if (Input[7] & 0x1) {
            int point = m_HeaderLength;
            m_HeaderLength += 1;
            //PES_private_data_flag == '1' -> 128
            if (Input[point] & 0x80) {
                m_HeaderLength += 16;
            }
            //pack_header_field_flag == '1' -> 8
            if (Input[point] & 0x40) {
                m_HeaderLength += 1;
            }
            //program packet sequence counter flag
            if (Input[point] & 0x20) {
                m_HeaderLength += 2;
            }
            //P-STD buffer flag
            if (Input[point] & 0x10) {
                m_HeaderLength += 2;
            }
            //P-STD buffer flag
            if (Input[point] & 0x1) {
                m_HeaderLength += 2;
            }
        }
    }

    return m_PacketStartCodePrefix;
}

void xPES_PacketHeader::Print() const {
    printf("PES: ");
    printf("PSCP=%d ", m_PacketStartCodePrefix);
    printf("SID=%d ", m_StreamId);
    printf("L=%d ", m_PacketLength);

    if (PTS_DTS_flags == 0b10)
        printf("PTS=%d \n", PTS);

    if (PTS_DTS_flags == 0b11) {
        printf("PTS=%d \n", PTS);
        printf("DTS=%d \n", DTS);
    }
}

//=============================================================================================================================================================================
// xPES_Assembler
//=============================================================================================================================================================================
xPES_Assembler::xPES_Assembler() { }

xPES_Assembler::~xPES_Assembler() { delete[] m_Buffer; }

void xPES_Assembler::Init(int32_t PID) {
    m_PID = PID;
    m_Buffer = new uint8_t[100000];
    m_BufferSize = 0;
    m_LastContinuityCounter = 0;
    m_Started = false;
}
xPES_Assembler::eResult xPES_Assembler::AbsorbPacket(const uint8_t* TransportStreamPacket, const xTS_PacketHeader* PacketHeader, const xTS_AdaptationField* AdaptationField) {
    uint8_t TS_AdaptationLenght = 0;
    if (PacketHeader->hasAdaptationField()) {
        TS_AdaptationLenght = AdaptationField->getNumBytes();
    }

    uint32_t tempSize = xTS::TS_PacketLength - xTS::TS_HeaderLength - TS_AdaptationLenght;


    if (PacketHeader->getPayloadUnitStartIndicator()) {
        tempSize = xTS::TS_PacketLength - xTS::TS_HeaderLength - TS_AdaptationLenght;
        uint32_t sizeToSkip = xTS::TS_PacketLength - tempSize;
        uint8_t* firstPacketBuffer = new uint8_t[188];

        for (int i = 0; i < tempSize; i++) {
            firstPacketBuffer[i] = TransportStreamPacket[sizeToSkip + i];
        }
        m_PESH.Parse(firstPacketBuffer);

        tempSize -= getHeaderLenght();

        xPES_Assembler::xBufferAppend(TransportStreamPacket, tempSize);
        return eResult::AssemblingStarted;
    }
    else if (PacketHeader->hasAdaptationField()) {
        xPES_Assembler::xBufferAppend(TransportStreamPacket, tempSize);
        return eResult::AssemblingFinished;
    }
    else {
        xPES_Assembler::xBufferAppend(TransportStreamPacket, tempSize);
        return eResult::AssemblingContinue;
    }
}

void xPES_Assembler::xBufferReset() {
    m_PESH.Reset();
    memset(m_Buffer, 0, sizeof(m_Buffer));
    m_BufferSize = 0;
}

void xPES_Assembler::xBufferAppend(const uint8_t* data, uint32_t size) {
    uint32_t sizeToSkip = xTS::TS_PacketLength - size;

    for (int i = 0; i < size; i++) {
        m_Buffer[m_BufferSize + i] = data[sizeToSkip + i];
    }
    m_BufferSize += size;
}

void xPES_Assembler::assemblerPes(const uint8_t* TS_PacketBuffer, const xTS_PacketHeader* TS_PacketHeader, 
    const xTS_AdaptationField* TS_AdaptationField, FILE* File) {

    xPES_Assembler::eResult result = AbsorbPacket(TS_PacketBuffer, TS_PacketHeader, TS_AdaptationField);
    switch (result)
    {
        case xPES_Assembler::eResult::AssemblingStarted: {
            printf("Assembling Started: \n"); 
            PrintPESH(); 
            break;
        }
        case xPES_Assembler::eResult::AssemblingContinue: {
            printf("Assembling Continue: "); 
            break; 
        }
        case xPES_Assembler::eResult::AssemblingFinished: {
            printf("Assembling Finished: \n"); 
            printf("PES: PcktLen=%d HeadLen=%d DataLen=%d\n", getNumPacketBytes(), getHeaderLenght(), getNumPacketBytes() - getHeaderLenght());
            saveBufferToFile(File);
            break;
        }
        case xPES_Assembler::eResult::StreamPackedLost: {
            printf("Packet Lost");
            break;
        }
        case xPES_Assembler::eResult::UnexpectedPID: {
            printf("Unexpected PID");
            break;
        }
        default: break;
    }
}

void xPES_Assembler::saveBufferToFile(FILE* AudioMP2) {
    fwrite(getPacket(), 1, getNumPacketBytes(), AudioMP2);
    xBufferReset();
}