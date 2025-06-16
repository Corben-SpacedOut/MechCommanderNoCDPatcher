#include "patch.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define PUFFER_SIZE (2048)
#define ERROR_SIZE (256)

#define MAIN_BASE (0x00702909)

static uint8_t MAIN_CHECK_SIGNATURE[]={
    0x8A  ,0x45 ,0xFC ,0xFE ,0xC0 ,0x3C ,0x5A ,0x88 ,0x45 ,0xFC 
    ,0x7E ,0xAB ,0x3B
};

static const long MAIN_PATCH_1_OFFSET=0x007028C2-MAIN_BASE;
static uint8_t MAIN_PATCH_1_SIG[] = { 0x75, 0x59 };
static uint8_t MAIN_PATCH_1[] = { 0xeb, 0x59 };

static const long MAIN_PATCH_2_OFFSET=0x00702A44-MAIN_BASE;
static uint8_t MAIN_PATCH_2_SIG[]=
    { 0x0F, 0x84, 0x72, 0xFE, 0xFF, 0xFF };
static uint8_t MAIN_PATCH_2[]=
    { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };

static const uint8_t STARTUP_CHECK_SIGNATURE[]={
    0x56,0x57,0xB0,0x43,0x33,0xFF,0x3B,0xCB,0x88,0x45,
    0xFC,0xC6,0x45,0xFD,0x3A,0x88,0x5D,0xFE,0xC6,0x45,
    0xFF,0x01,0x89,0x5D,0xF0,0x89,0x5D,0xEC
};
static const long STARTUP_PATCH_OFFSET=0x00758DB1-0x00758DC0;
static const uint8_t STARTUP_PATCH_SIG[]={ 0x75, 0x07 };
static const uint8_t STARTUP_PATCH[]={ 0x90, 0x90 };

static char epuf[PUFFER_SIZE + ERROR_SIZE];

static const char *EXE_BASE = "MCX";
static const char *EXE_SUFFIX = ".EXE";

typedef enum {
    PATCH_TYPE_MAIN=1,
    PATCH_TYPE_STARTUP=2,
} patch_type;

typedef struct {
    long offset;
    patch_type type;
} patch_info;
static patch_info patches[20];
static const int max_patches=(sizeof(patches)/sizeof(patches[0]));
static int npatches=0;

static const char * is_patchable(uint8_t *data, long size);
static const char * patch_game(uint8_t *data, long size);

const char *patch(const char *game_path)
{
    FILE *fin = NULL;
    FILE *fout = NULL;
    uint8_t *data = NULL;
    long size = 0;
    const char *errmsg = "Unforseen error!";

    char puffer[PUFFER_SIZE];
    char exe_puffer[PUFFER_SIZE];

    sprintf_s(exe_puffer, sizeof(exe_puffer), "%s\\%s%s", game_path, EXE_BASE, EXE_SUFFIX);
    fopen_s(&fin, exe_puffer, "rb");

    // Read input file.
    if (!fin) {
        sprintf_s(epuf, sizeof(epuf), "Failed to open '%s' for reading.", exe_puffer);
        errmsg = epuf;
        goto exit;
    }
    if (fseek(fin, 0, SEEK_END)) {
        errmsg = "Random access failure on executable!";
        goto exit;
    }
    size = ftell(fin);
    data = malloc(size);
    if (!data) {
        errmsg = "Memory allocation error!";
        goto exit;
    }
    if (fseek(fin, 0, SEEK_SET)) {
        errmsg = "Random access failure on executable!";
        goto exit;
    }
    if (fread(data, 1, size, fin) != size) {
        errmsg = "Failed to read executable!";
        goto exit;
    }
    fclose(fin);
    fin=NULL;

    // Find patch locations.
    errmsg=is_patchable(data,size);
    if (errmsg) goto exit;
    errmsg="Generic patching failure.";

    // Patch it.
    errmsg=patch_game(data, size);
    if (errmsg) goto exit;
    errmsg="Miserable failure!";

    // Write patch
    sprintf_s(puffer,sizeof(puffer),"%s\\%s-nocd%s",game_path,EXE_BASE,EXE_SUFFIX);
    int e=fopen_s(&fout, puffer, "wb");
    if (!fout) {
        char tbuf[200];
        strerror_s(tbuf,sizeof(tbuf),e);
        sprintf_s(epuf,sizeof(epuf),"Couldn't open %s for writing: %s",puffer,tbuf);
        errmsg=epuf;
        goto exit;
    }
    if (fwrite(data,1,size,fout)!=size) {
        errmsg="Writing patched executable failed!";
        goto exit;
    }

    errmsg=NULL;

exit:
    if (fin) fclose(fin);
    if (fout) fclose(fout);
    if (data) free(data);
    return errmsg;
}

long find_sig(const uint8_t *data,long size, long off, const uint8_t *sig, long sigsize)
{
    for(long i=off;i<=size-sigsize;i++) {
        for(long j=0;j<sigsize;j++)
            if (data[i+j]!=sig[j]) goto skip;

        return i;

    skip:;
    }

    return -1;
}

static const char * is_patchable(uint8_t *data,long size)
{
    long i=find_sig(data,size,0,MAIN_CHECK_SIGNATURE,sizeof(MAIN_CHECK_SIGNATURE));
    while(i>=0) {
        if (memcmp(data+i+MAIN_PATCH_1_OFFSET,MAIN_PATCH_1_SIG,sizeof(MAIN_PATCH_1_SIG))==0
        && memcmp(data+i+MAIN_PATCH_2_OFFSET,MAIN_PATCH_2_SIG,sizeof(MAIN_PATCH_2_SIG))==0) {
            if (npatches==max_patches) return "Found too many patch sites!";
            patch_info *pi=patches+(npatches++);
            pi->offset=i;
            pi->type=PATCH_TYPE_MAIN;
        } else if (memcmp(data+i+MAIN_PATCH_1_OFFSET,MAIN_PATCH_1,sizeof(MAIN_PATCH_1))==0
        || memcmp(data+i+MAIN_PATCH_2_OFFSET,MAIN_PATCH_2,sizeof(MAIN_PATCH_2))==0)
            return "Looks like file might already be patched.";

        i=find_sig(data,size,i+1,MAIN_CHECK_SIGNATURE,sizeof(MAIN_CHECK_SIGNATURE));
    }

    i=find_sig(data,size,0,STARTUP_CHECK_SIGNATURE,sizeof(STARTUP_CHECK_SIGNATURE));
    if (i<0) return "Unable to locate patch site.";
    if (memcmp(data+i+STARTUP_PATCH_OFFSET,STARTUP_PATCH,sizeof(STARTUP_PATCH))==0)
        return "File is probably already patched.";
    if (memcmp(data+i+STARTUP_PATCH_OFFSET,STARTUP_PATCH_SIG,sizeof(STARTUP_PATCH_SIG))!=0)
        return "Problem locating patch site.";
    patch_info *pi=patches+(npatches++);
    pi->offset=i;
    pi->type=PATCH_TYPE_STARTUP;

    if (npatches>9) return "Found more patch sites than expected.";
    if (npatches<9) return "Not all patch sites were located.";

    return NULL;
}

static const char * patch_game(uint8_t *data,long size)
{
    for(int i=0;i<npatches;i++) {
        patch_info *pi=patches+i;
        if (pi->type == PATCH_TYPE_MAIN) {
            memcpy(data+pi->offset+MAIN_PATCH_1_OFFSET,MAIN_PATCH_1,sizeof(MAIN_PATCH_1));
            memcpy(data+pi->offset+MAIN_PATCH_2_OFFSET,MAIN_PATCH_2,sizeof(MAIN_PATCH_2));
        } else if (pi->type == PATCH_TYPE_STARTUP) {
            memcpy(data+pi->offset+STARTUP_PATCH_OFFSET,STARTUP_PATCH,sizeof(STARTUP_PATCH));
        } else return "Unexpected patch type.";
    }

    return NULL;
}
