#include <switch.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>

constexpr const char out_path[]= "/titles.csv";
constexpr u32 MaxTitleCount = 64000;

std::string formatApplicationId(u64 ApplicationId)
{
    std::stringstream strm;
    strm << std::uppercase << std::setfill('0') << std::setw(16) << std::hex << ApplicationId;
    return strm.str();
}

int writeTitlesToFile()
{
    NsApplicationRecord *records = new NsApplicationRecord[MaxTitleCount]();
    uint64_t tid;
    NsApplicationControlData controlData;
    NacpLanguageEntry* langEntry = NULL;

    Result rc;
    int recordCount     = 0;
    size_t controlSize  = 0;

    std::ofstream file(out_path);

    if(file.is_open())
    {
        file << "Title ID|Title Name\n";
        rc = nsListApplicationRecord(records, MaxTitleCount, 0, &recordCount);
        for (s32 i = 0; i < recordCount; i++)
        {
            tid = records[i].application_id;
            rc = nsGetApplicationControlData(NsApplicationControlSource_Storage, tid, &controlData, sizeof(controlData), &controlSize);
            if(R_FAILED(rc)) break;
            rc = nacpGetLanguageEntry(&controlData.nacp, &langEntry);
            if(R_FAILED(rc)) break;
            
            if(!langEntry->name)
                continue;

            file << formatApplicationId(tid) + "|" + langEntry->name + "\n";
        }
        file.close();
        delete[] records;
        return 1;
    }
    return 0;
}

int main(int argc, char* argv[])
{
    consoleInit(NULL);
    nsInitialize();

    padConfigureInput(1, HidNpadStyleSet_NpadStandard);

    PadState pad;
    padInitializeDefault(&pad);

    int res = -1;

    while (appletMainLoop())
    {
        padUpdate(&pad);

        u64 kDown = padGetButtonsDown(&pad);

        consoleClear();

        printf("\033[31mTitles list dumper\033[0m\n\n");
        if(res == -1)
            printf("Press [A] to dump a list of titles to `%s`\n", out_path);
        if(res == 1)
            printf("Done\n");
        if(res == 0)
            printf("Failed\n");
        printf("Press [+] to exit\n");

        if (kDown & HidNpadButton_A) 
        {
            printf("Dumping...\n");
            consoleUpdate(NULL);
            res = writeTitlesToFile();
        }

        if (kDown & HidNpadButton_Plus)
            break;

        consoleUpdate(NULL);
    }

    nsExit();
    consoleExit(NULL);
    return 0;
}
