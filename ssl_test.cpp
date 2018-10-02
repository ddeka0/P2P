#include <openssl/sha.h>
#include <bits/stdc++.h>
using namespace std;
int main()
{  
  const unsigned char str[] = "original String";
  const unsigned char str1[] = "Original String";
  unsigned char hash[SHA_DIGEST_LENGTH]; // == 20
  unsigned char hash1[SHA_DIGEST_LENGTH];

  
  SHA1(str, sizeof(str) - 1, hash);
  SHA1(str1, sizeof(str1) - 1, hash1);

  cout<<hash<<endl;
  cout<<hash1<<endl;
  // do some stuff with the hash

  return 0;
}