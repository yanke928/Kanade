#include <stdio.h>
#include <string.h>

#include <windows.h>

#define BLOCK_SIZE 2048

bool Write_Binary_Length(FILE* dst, int length);
bool Write_Moha_Tag(FILE* dst);
unsigned int  GetCRC32(unsigned char *buf, int len);
unsigned char Write_A_Block(FILE* dst, FILE* src);

static unsigned int   CRC32[256];
static char   Init = 0;


enum { File_Read_Failed, File_Write_Failed, File_RW_OK };

int main(int argc, char *argv[])
{
	FILE* src;
	FILE* dst;
	char dstPath[200];
	char srcPath[200];
	char* pointAddr;
	unsigned char errCode;
	int binaryFileLength;
	int blockCount;
	int i;
	bool success;

	if (argv[1] == NULL)
	{
		printf("Invalid argument\n");
		system("pause");
		return 1;
	}

	strcpy(srcPath, argv[1]);
	src = fopen(srcPath, "rb");

	if (src == NULL)
	{
		printf("Open file failed\n");
		system("pause");
		return 1;
	}

	if (argv[2] == NULL)
	{
		pointAddr = strchr(srcPath, '.');
		*(pointAddr + 1) = 0;
		strcat(srcPath, "moha");
		strcpy(dstPath, srcPath);
		dst = fopen(dstPath, "wb+");
	}
	else
	{
		strcpy(dstPath, argv[2]);
		dst = fopen(dstPath, "wb+");
	}

	if (src == NULL)
	{
		printf("Create target file failed\n");
		system("pause");
		return 1;
	}

	fseek(src, 0, SEEK_END);
	binaryFileLength = ftell(src);

	printf("Binary file length: %d bytes\n", binaryFileLength);

	success = Write_Moha_Tag(dst);

	if (!success)
	{
		printf("File tag write failed\n");
		system("pause");
		return 1;
	}

	success = Write_Binary_Length(dst, binaryFileLength);

	if (!success)
	{
		printf("File length write failed\n");
		system("pause");
		return 1;
	}

	fseek(src, 0, SEEK_SET);

	blockCount = binaryFileLength / BLOCK_SIZE + 1;

	printf("Block Count: %d\n", blockCount);

	for (i = 0; i < blockCount; i++)
	{
		errCode = Write_A_Block(dst, src);
		if (errCode != File_RW_OK)
		{
			switch (errCode)
			{
			case File_Read_Failed:
				printf("Read binary file failed at block %d\n", i); break;
			case File_Write_Failed:
				printf("Write target file failed at block %d\n", i); break;
			}
			fclose(src);
			fclose(dst);
			return 1;
		}
	}

	fclose(src);
	fclose(dst);
	printf("Bin2Moha success!!\n");
	return 0;
}

unsigned char Write_A_Block(FILE* dst, FILE* src)
{
	int byteRead;
	int i;
	unsigned int crc;
	unsigned char buff[BLOCK_SIZE + 4];
	byteRead = fread(buff, 1, BLOCK_SIZE, src);
	if (byteRead == 0) return File_Read_Failed;
	if (byteRead != BLOCK_SIZE)
	{
		for (i = byteRead; i < BLOCK_SIZE; i++)
		{
			buff[i] = 0;
		}
	}
	crc = GetCRC32(buff, BLOCK_SIZE);
	*(unsigned int*)(buff + BLOCK_SIZE) = crc;
	if (fwrite(&buff, sizeof(buff), 1, dst) != 1) return File_Write_Failed;
	return File_RW_OK;
}

static void Init_table()
{
	int   i, j;
	unsigned int  crc;
	for (i = 0; i < 256; i++)
	{
		crc = i;
		for (j = 0; j < 8; j++)
		{
			if (crc & 1)
			{
				crc = (crc >> 1) ^ 0xEDB88320;
			}
			else
			{
				crc = crc >> 1;
			}
		}
		CRC32[i] = crc;
	}
}

unsigned int  GetCRC32(unsigned char *buf, int len)
{
	unsigned int  ret = 0xFFFFFFFF;
	int   i;
	if (!Init)
	{
		Init_table();
		Init = 1;
	}
	for (i = 0; i < len; i++)
	{
		ret = CRC32[((ret & 0xFF) ^ buf[i])] ^ (ret >> 8);
	}
	ret = ~ret;
	return ret;
}

bool Write_Binary_Length(FILE* dst, int length)
{
	int written;
	written = fwrite(&length, sizeof(length), 1, dst);
	if (written != 1) return false;
	return true;
}

bool Write_Moha_Tag(FILE* dst)
{
	if (fwrite("moha", sizeof("moha") - 1, 1, dst) != 1) return false;
	return true;
}
