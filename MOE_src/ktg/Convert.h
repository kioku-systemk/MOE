int RemoveCPPCommentFromString(char* szSrc, int* ctrf = NULL);
int RemoveCCommentFromString(char* szSrc, int* ctrf = NULL);
void RemoveCharFromString(char* szSrc, char Target, char Exception = -1);
int RemoveCaridgeReturnFromString(char* szSrc);
//void MakeEscapeSequenceFromString(char* szSrc);
int UpdateTexture(const char* szTexture);
void InitConverter();
void DeInitConverter();