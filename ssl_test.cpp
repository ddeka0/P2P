#include <openssl/sha.h>
#include <bits/stdc++.h>
using namespace std;
int main() {  
	string a = "Debashish Deka";
	string b = "Debashish Deka";
	
	unsigned char hash[SHA_DIGEST_LENGTH];
	unsigned char hash1[SHA_DIGEST_LENGTH];
	
	SHA1(reinterpret_cast<const unsigned char *>(a.c_str()), a.length(), hash);
	SHA1(reinterpret_cast<const unsigned char *>(b.c_str()), b.length(), hash1);

	cout << hash << endl;
	cout << hash1 << endl;

	std::string str3(hash,hash + SHA_DIGEST_LENGTH);
	std::string str4(hash1,hash1 + SHA_DIGEST_LENGTH);

	cout << str3 << endl;
	cout << str4 << endl;

	cout<< (str3 == str4) <<endl;
	// do some stuff with the hash

	return 0;
}
