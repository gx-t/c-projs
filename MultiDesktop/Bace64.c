#include "common.h"

LPSTR Base64Encode(LPSTR szOut, LPCSTR szIn, int iInlen)
{
  static CHAR base64digits[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  for (; iInlen >= 3; iInlen -= 3)
  {
    *szOut++ = base64digits[szIn[0] >> 2];
    *szOut++ = base64digits[((szIn[0] << 4) & 0x30) | (szIn[1] >> 4)];
    *szOut++ = base64digits[((szIn[1] << 2) & 0x3c) | (szIn[2] >> 6)];
    *szOut++ = base64digits[szIn[2] & 0x3f];
    szIn += 3;
  }
  if(iInlen > 0)
  {
    CHAR fragment = szIn[0];
    *szOut++ = base64digits[szIn[0] >> 2];
    fragment <<= 4;
    fragment &= 0x30;
    if (iInlen > 1)
      fragment |= szIn[1] >> 4;
    *szOut++ = base64digits[fragment];
    *szOut++ = (CHAR)((iInlen < 2) ? '=' : base64digits[(szIn[1] << 2) & 0x3c]);
    *szOut++ = '=';
  }
  *szOut = '\0';
  return szOut;
}

