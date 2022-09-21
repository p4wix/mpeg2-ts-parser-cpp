#include "tsCommon.h"
#include "tsTransportStream.h"

int main(int argc, char *argv[ ], char *envp[ ]) {
    FILE* TransportStreamFile = fopen("/Users/example_new.ts", "rb");
    FILE* AudioMP2 = fopen("/Users/pawel.biniak/studies/parser2/PID136.mp2", "wb");

    if (TransportStreamFile == NULL) {
        printf("bad file name\n");
        return EXIT_FAILURE;
    }
    uint8_t TS_PacketBuffer[xTS::TS_PacketLength];
    xTS_PacketHeader TS_PacketHeader;
    xTS_AdaptationField TS_AdaptationField;

    xPES_Assembler PES_Assembler136;

    PES_Assembler136.Init(136);

    int32_t TS_PacketId = 0;


    while (!feof(TransportStreamFile)) {
        size_t NumRead = fread(TS_PacketBuffer, 1, xTS::TS_PacketLength, TransportStreamFile);

        if (NumRead != xTS::TS_PacketLength) { break; }

        TS_PacketHeader.Reset();
        TS_PacketHeader.Parse(TS_PacketBuffer);

        TS_AdaptationField.Reset();

        if (TS_PacketHeader.getSyncByte() == 'G' && (TS_PacketHeader.getPID() == 136)) {

            if (TS_PacketHeader.hasAdaptationField()) {
                TS_AdaptationField.Parse(TS_PacketBuffer, TS_PacketHeader.getAdaptionField());
            }
            printf("%010d ", TS_PacketId);
            TS_PacketHeader.Print();
            printf("\n");

            if (TS_PacketHeader.hasAdaptationField()) {
                TS_AdaptationField.Print();
            }
            if (TS_PacketHeader.getPID() == 136) {
                PES_Assembler136.assemblerPes(TS_PacketBuffer, &TS_PacketHeader, &TS_AdaptationField, AudioMP2);
            }
        }
        if(TS_PacketId == 2000) { break; }
        ++TS_PacketId;
    }
    fclose(TransportStreamFile);
    fclose(AudioMP2);
    return 0;
}