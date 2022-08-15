#ifndef _MD5_HPP_
#define _MD5_HPP_

#include <iostream>
#include <openssl/md5.h>
#include <fstream>
#include <string>
#include <iomanip>
#include <string.h>

std::string get_file_md5(std::string lpcstrFilePath)
{
    char *pbymd5Buff [100]= {0};
	unsigned char achbyBuff[17] = {0};
	unsigned char *pbyBuff = achbyBuff;
	
    FILE *inFile = fopen (lpcstrFilePath.c_str(), "rb");
	MD5_CTX mdContext;
	int bytes;
	unsigned char data[1024];
	
	if (inFile == NULL) {
		printf( "%s can't be opened.\n", lpcstrFilePath.c_str());
        throw "file can't be opened : " ;
	}
	
	MD5_Init (&mdContext);
	while ((bytes = fread (data, 1, 1024, inFile)) != 0)
		MD5_Update (&mdContext, data, bytes);
	MD5_Final (pbyBuff,&mdContext);
	
    char szBuf[1024] = {0};
	for(int i = 0; i < MD5_DIGEST_LENGTH; i++) 
	{
		sprintf(&szBuf[i*2], "%02x", pbyBuff[i]);
	}
	
    strcpy((char*)pbymd5Buff, (char*)szBuf);

    std::string md5_str = szBuf;
	fclose (inFile);
    return szBuf;
}


#endif