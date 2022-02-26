/*
WinHex
查找差异

1. MPUI.ori.exe: 296,960 字节
2. MPUI.exe.vir: 354,816 字节
Offsets: 16 进制

section #:
  106:	03	04

EntryPoint:
  128:	40	00
  129:	1A	90

Image size:
  151:	90	80
  152:	0E	0F

section Name:
  270:	00	2E
  271:	00	72
  272:	00	6D
  273:	00	6E
  274:	00	65
  275:	00	74

  279:	00	F0

  27D:	00	90
  27E:	00	0E

  281:	00	E2

  285:	00	88
  286:	00	04

  294:	00	20
  297:	00	E0

19 差异 找到。

=====
X64dbg:

004E9309 | 8B 85 4C 72 01 20        | mov eax,dword ptr ss:[ebp+2001724C]
004E930F | 2B 85 54 72 01 20        | sub eax,dword ptr ss:[ebp+20017254]
004E9315 | 89 44 24 1C              | mov dword ptr ss:[esp+1C],eax
004E9319 | 61                       | popal
004E931A | FF E0                    | jmp eax

EP - [+ 3 * 4]
*/

#include <stdio.h>
#include <stdlib.h>
#include <io.h>

#include <windows.h>

int main(int argc, char *argv[])
{
    int i;
    FILE *fp;
    IMAGE_DOS_HEADER *dos_hdr;
    IMAGE_NT_HEADERS *pe_hdr;
    IMAGE_OPTIONAL_HEADER *p_optional_hdr;
    WORD section_n;
    IMAGE_SECTION_HEADER *section_hdr_last;
    //
    if (argc < 2) {
        printf("%s <pe-file> ..\n", argv[0]);
        printf("Caution: backup yourself!\n");
        return 1;
    }
    //
    for (i = 1; i < argc; i++) {
        WORD e_magic = 0;
        int fd;
        long file_size;
        BYTE *pe_file, *section;
        int not_pe = 1, good = 1;
        long j, n;
        //
        printf("\"%s\"\t", argv[i]);
        //
        fp = fopen(argv[i], "r+b");
        if (fp == NULL) {
            printf("fail-read\n");
            continue;
        }
        // pre-test, consider not infected (large) files
        fread(&e_magic, 1, sizeof(WORD), fp);
        if (e_magic != 0x5A4D) {
            printf("not-pe-file, ");
            goto access_decision;
        }
        // main-test
        /* whether infected?
           0. manually specified!
        */
        fd = fileno(fp);
        file_size = _filelength(fd);
        pe_file = malloc(file_size);
        rewind(fp);
        fread(pe_file, 1, file_size, fp);
        //
        dos_hdr = (IMAGE_DOS_HEADER*)pe_file;
        pe_hdr  = (IMAGE_NT_HEADERS*)(pe_file + dos_hdr->e_lfanew);
        if (pe_hdr->Signature != 0x4550) {
            printf("not-pe-file, ");
            goto access_decision;
        }
        not_pe = 0;
        //
        section_n = pe_hdr->FileHeader.NumberOfSections;
        p_optional_hdr = &(pe_hdr->OptionalHeader);
        //
        section_hdr_last = (IMAGE_SECTION_HEADER*)((BYTE*)p_optional_hdr + sizeof(IMAGE_OPTIONAL_HEADER) + (section_n - 1) * sizeof(IMAGE_SECTION_HEADER));
        // 1. EP == VA(section_last)
        if (section_hdr_last->VirtualAddress != p_optional_hdr->AddressOfEntryPoint) {
            goto access_decision;
        }
        //
        /* 2. launch OEP
004E9309 | 8B 85 4C 72 01 20        | mov eax,dword ptr ss:[ebp+2001724C]
004E930F | 2B 85 54 72 01 20        | sub eax,dword ptr ss:[ebp+20017254]
004E9315 | 89 44 24 1C              | mov dword ptr ss:[esp+1C],eax
004E9319 | 61                       | popal
004E931A | FF E0                    | jmp eax
        */
        n = file_size - 31; // 19 + 12
        for (j = section_hdr_last->PointerToRawData; j < n && good; j++) {
            if (*(WORD*)(pe_file + j)      != 0x858B   ) {continue;}
            if (*(WORD*)(pe_file + j + 6)  != 0x852B   ) {continue;}
            if (       *(pe_file + j + 16) != 0x61     ) {continue;}
            if (*(WORD*)(pe_file + j + 17) != 0xE0FF   ) {continue;}
            good = 0;
            break;
        }
        //
access_decision:
        if (not_pe) {
            printf("html-file-could-be-infected\n");
        }
        else if (good) {
            printf("not-infected\n");
        }
        else {
            p_optional_hdr->AddressOfEntryPoint -= *(unsigned int*)(pe_file + j + 31); // 19 + 12
            pe_hdr->FileHeader.NumberOfSections--;
            p_optional_hdr->SizeOfImage = section_hdr_last->VirtualAddress;
            file_size = section_hdr_last->PointerToRawData;
            memset(pe_file + section_hdr_last->PointerToRawData, 0, section_hdr_last->SizeOfRawData);
            memset(section_hdr_last, 0, sizeof(IMAGE_SECTION_HEADER));
            //
            fp = freopen(argv[i], "w+b", fp);
            if (fp == NULL) {
                printf("fail-write\n");
                goto cleanup;
            }
            fwrite(pe_file, 1, file_size, fp);
            fflush(fp);
            printf("succeed\n");
        }
        //
cleanup:
        free(pe_file);
        fclose(fp);
    }
    //
    return 0;
}