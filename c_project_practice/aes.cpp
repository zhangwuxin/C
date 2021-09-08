#include "ldar_crypto.h"
#include <iostream>
#include <openssl/pem.h>
#include "wbl.h"
#include "charset.h"

using namespace wbl;
using namespace std;
using namespace boss;

int main(){
string en_str="";
string de_str="";
AESCrypter ldar_aes("B37vsNIiFLqM39l9", "7");
ldar_aes.Encrypt("20200909", en_str);
string encryptData = boss::base64_encode(reinterpret_cast<unsigned char const *>(en_str.c_str()), en_str.length());

cout <<encryptData<< endl;  

string encryptData2 = boss::base64_decode(encryptData);
ldar_aes.Decrypt(encryptData2, de_str);

cout << de_str << endl;
return 0;

}
